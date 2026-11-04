#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <MAGIC_inferencing.h>  // 如果你当前能正常工作的草稿使用 MAGIC2_inferencing.h，就只改这一行。 / If your WORKING sketch uses MAGIC2_inferencing.h, change ONLY this line.
#include <math.h>
#include <string.h>
#include <esp_system.h>
#include "arduino_secrets.h"

// SPELLBOUND：ESP32 Feather、MPU6050、Edge Impulse 和 Vespera #11。
// SPELLBOUND: ESP32 Feather + MPU6050 + Edge Impulse + Vespera #11
// IMU 设置、输入单位和轴顺序要和成功的手势测试保持一致。
// Keep IMU settings, input units, and axis order identical to the successful gesture test.
constexpr int SDA_PIN = 23;
constexpr int SCL_PIN = 22;
// 这是自锁开关：GPIO27 接到 GND，程序用 INPUT_PULLUP。
// Toggle (latching) switch: GPIO27 to GND (INPUT_PULLUP).
// 开关从 OFF 拨到 ON 时施放一次；想再施放，就先拨回 OFF，再拨到 ON。
// Switching OFF -> ON starts one cast. To cast again: switch OFF, then ON.
// 把这里改成实际接线使用的 GPIO。
// Change this to the actual GPIO you wired.
constexpr int BUTTON_PIN = 27;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 50;
// 开关打开时引脚的电平。GPIO27 接开关再接 GND 时，ON 对应 LOW。
// Pin level when the switch is ON. Wired GPIO27 -> switch -> GND: ON = LOW.
// 如果实际是在拨到 OFF 时触发，就把这里改成 HIGH。
// If casting starts when you switch OFF instead, change this to HIGH.
constexpr bool SWITCH_ON_LEVEL = LOW;
// false 表示只在自锁开关拨到 ON 时施放；拨回 OFF 不施放。
// false = cast only when the latching button turns ON (press again = OFF, no cast)
// true 表示每次状态变化都施放，ON 和 OFF 都会触发。
// true  = cast on EVERY press (both ON and OFF)
constexpr bool CAST_ON_EVERY_PRESS = false;

// Vespera 上的施法提示按这个顺序显示：
// Cast cue on Vespera:
// 1）先闪两次，提示开始。
// 1) flash twice (start warning)
// 2）72 个灯先全部亮起，再在两秒采样期间逐个熄灭。
// 2) all 72 LEDs on, then they go out one by one over the 2 s capture
// 3）灯全灭表示采样结束，接着显示识别到的咒语或备用演示。
// 3) all off = input finished; recognised spell or fallback demo follows
constexpr uint8_t CUE_R = 10, CUE_G = 185, CUE_B = 230;   // 青色。 / cyan
constexpr int COUNTDOWN_EVERY_N_SAMPLES = 4;             // 每 80 毫秒更新一次，共 25 步。 / update every 80 ms (25 steps)
constexpr int SAMPLE_RATE = 50;
constexpr int SAMPLE_COUNT = 100;
constexpr int AXES = 6;
constexpr uint32_t SAMPLE_INTERVAL_US = 1000000UL / SAMPLE_RATE;
constexpr float MIN_CONFIDENCE = 0.65f;
// 根据之前的训练样本，重力约为 9.8；这里的加速度单位是 m/s²。
// Based on the earlier training samples (gravity approximately 9.8): acceleration in m/s^2.
// 注意：如果能正常识别的草稿把这个值设为 true，先核对训练数据的单位再改。
// IMPORTANT: If your working inference sketch used true, check training units before changing.
constexpr bool ACCEL_IN_G = false;

const char* MQTT_HOST = "mqtt.cetools.org";
constexpr uint16_t MQTT_PORT = 1884;
const char* MQTT_TOPIC = "student/CASA0014/luminaire/11";
constexpr int LED_COUNT = 72;
constexpr int PAYLOAD_SIZE = LED_COUNT * 3;
constexpr uint32_t FRAME_INTERVAL_MS = 140;
constexpr uint32_t EFFECT_DURATION_MS = 3500;

Adafruit_MPU6050 imu;
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
static float features[SAMPLE_COUNT * AXES];
static uint8_t pixels[PAYLOAD_SIZE];
static int activeSpell = -1;
static bool effectActive = false;
static uint8_t fallbackDemoStep = 0;
static uint32_t effectStarted = 0;
static uint32_t lastFrameAt = 0;
static uint32_t frameIndex = 0;
static uint32_t lastWifiAttempt = 0;
static uint32_t lastMqttAttempt = 0;
static bool buttonLastRaw = HIGH;
static bool buttonStable = HIGH;
static uint32_t buttonChangedAt = 0;

const char* SPELL_NAMES[] = {
  "Ascendio", "ci", "incendio", "leviosa", "oppugno", "stupefy"
};

static uint8_t limitByte(int n) {
  return (uint8_t)(n < 0 ? 0 : (n > 255 ? 255 : n));
}
static float clamp01(float x) {
  return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
}
static uint8_t blend(uint8_t a, uint8_t b, float t) {
  t = clamp01(t);
  return limitByte((int)(a + (b - a) * t));
}
static void pixel(int index, uint8_t r, uint8_t g, uint8_t b) {
  if (index < 0 || index >= LED_COUNT) return;
  pixels[index * 3] = r;
  pixels[index * 3 + 1] = g;
  pixels[index * 3 + 2] = b;
}
static void clearPixels() {
  memset(pixels, 0, sizeof(pixels));
}
static bool publishPixels() {
  if (!mqtt.connected()) return false;
  return mqtt.publish(MQTT_TOPIC, pixels, sizeof(pixels));
}

// Vespera #11 也用来显示魔杖按钮和系统状态。
// Vespera #11 doubles as the wand's button/status indicator.
// 每条消息仍是 72 × 3，也就是 216 字节的原始 RGB 数据。
// All messages remain 72 * 3 = 216 bytes of raw RGB data.
static bool showSolid(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < LED_COUNT; ++i) pixel(i, r, g, b);
  return publishPixels();
}

static void flashFeedback(uint8_t r, uint8_t g, uint8_t b) {
  for (int n = 0; n < 2; ++n) {
    showSolid(r, g, b);
    delay(150);
    mqtt.loop();
    showSolid(0, 0, 0);
    delay(110);
    mqtt.loop();
  }
}

// 倒计时灯条会点亮前 `lit` 个灯，其余熄灭。
// Countdown bar: the first `lit` LEDs on, the rest off.
// 采样开始时 lit 是 72；两秒结束时变成 0。
// lit = 72 at the start of capture, 0 when the 2 s are over.
static bool showCountdown(int lit) {
  for (int i = 0; i < LED_COUNT; ++i) {
    if (i < lit) pixel(i, CUE_R, CUE_G, CUE_B);
    else pixel(i, 0, 0, 0);
  }
  return publishPixels();
}

static void serviceNetwork() {
  uint32_t now = millis();
  if (WiFi.status() != WL_CONNECTED) {
    if ((uint32_t)(now - lastWifiAttempt) >= 10000UL) {
      lastWifiAttempt = now;
      Serial.println("[WiFi] Connecting...");
      WiFi.disconnect();
      WiFi.begin(SECRET_SSID, SECRET_PASS);
    }
    return;
  }
  if (mqtt.connected()) {
    mqtt.loop();
    return;
  }
  if ((uint32_t)(now - lastMqttAttempt) >= 5000UL) {
    lastMqttAttempt = now;
    String clientId = "SPELLBOUND-11-" + WiFi.macAddress();
    clientId.replace(":", "");
    Serial.println("[MQTT] Connecting...");
    if (mqtt.connect(clientId.c_str(), SECRET_MQTTUSER, SECRET_MQTTPASS)) {
      Serial.println("[MQTT] Connected! Vespera #11 ready.");
    } else {
      Serial.print("[MQTT] Failed, state=");
      Serial.println(mqtt.state());
    }
  }
}

// 这里有 6 种效果；每帧 MQTT 消息只发送 216 字节的 RGB 数据。
// 6 effects. Animated frames use only 216 raw RGB bytes per MQTT message.
// 这里按 12 条灯带、每条 6 颗灯来计算；每条灯带从顶部 0 编到底部 5。
// Layout assumption: 12 strips x 6 LEDs, indexed top=0 to bottom=5 in each strip.
static void renderEffect(int spell, uint32_t frame) {
  float t = frame * 0.38f;
  for (int i = 0; i < LED_COUNT; ++i) {
    float x = (float)i / (float)(LED_COUNT - 1);
    float w = 0.5f + 0.5f * sinf(x * 11.0f - t);
    int r = 0, g = 0, b = 0;
    switch (spell) {
      case 0: {
        // Ascendio：柔和的青色光波向上移动，后面留下一段渐隐的光。
        // Ascendio: a soft cyan spell wave travels upward with a fading tail.
        // 每条灯带的编号是顶部 0、底部 5；所以这里把高度 0 当作底部。
        // Each strip is indexed top=0 to bottom=5; height 0 is the bottom.
        constexpr int LEDS_PER_STRIP = 6;
        const int rowInStrip = i % LEDS_PER_STRIP;
        const float height = (float)(LEDS_PER_STRIP - 1 - rowInStrip);
        float glow = 0.0f;

        if (frame < 3UL) {
          // 光先在底部聚起来，再开始上升。
          // A small pool of light gathers at the bottom before rising.
          if (rowInStrip == 5) {
            glow = 0.16f + 0.12f * frame;
          }
        } else if (frame <= 15UL) {
          // 较亮的光头从底部往上走，后面跟着较柔和的光尾。
          // The bright head moves bottom-to-top; a softer trail follows it.
          const float progress = (float)(frame - 3UL) / 12.0f;
          const float head = progress * (LEDS_PER_STRIP - 1);
          const float distance = height - head;
          const float core = clamp01(1.0f - fabsf(distance) / 1.25f);
          const float tail = (distance < 0.0f)
                                 ? 0.48f * clamp01(1.0f + distance / 3.5f)
                                 : 0.0f;
          glow = max(core, tail);
        } else {
          // 光波到顶后稍微闪动一下，然后慢慢淡出。
          // Brief shimmer after the wave reaches the top, then fade away.
          const float fade = clamp01(1.0f - (float)(frame - 16UL) / 9.0f);
          const float shimmer = 0.70f + 0.30f * sinf(t * 2.0f + height);
          glow = 0.28f * fade * shimmer;
        }

        const float breathe = 0.88f + 0.12f * sinf(t * 0.8f);
        r = (int)(35.0f * glow * breathe);
        g = (int)(175.0f * glow * breathe);
        b = (int)(255.0f * glow * breathe);
        break;
      }
      case 1: {
        // ci：一次性的阿瓦达风格效果，使用克制的翠绿和薄荷绿。
        // ci: one-shot Avada-style cast in a restrained emerald/mint palette.
        if (frame <= 2UL) {
          // 较深的玉绿色能量先在装置底部聚集。
          // Deep jade charge gathers at the bottom of the installation.
          const float charge = 0.25f + 0.15f * frame;
          const float baseGlow = (i < 7) ? (1.0f - i / 8.0f) : 0.0f;
          r = (int)(8.0f + 26.0f * charge * baseGlow);
          g = (int)(32.0f + 105.0f * charge * baseGlow);
          b = (int)(24.0f + 64.0f * charge * baseGlow);
        } else if (frame <= 10UL) {
          // 薄荷绿光束向上穿过灯带，后面带着较柔和的玉绿色光尾。
          // Mint-green bolt travels upward with a softer jade tail.
          const float travel = (float)(frame - 3UL) / 7.0f;
          const float head = travel * (LED_COUNT - 1);
          const float distance = (float)i - head;
          const float core = clamp01(1.0f - fabsf(distance) / 2.3f);
          const float tail = (distance < 0.0f)
                                 ? clamp01(1.0f + distance / 19.0f)
                                 : 0.0f;
          r = (int)(18.0f + 82.0f * core + 20.0f * tail);
          g = (int)(32.0f + 176.0f * core + 112.0f * tail);
          b = (int)(22.0f + 108.0f * core + 58.0f * tail);
        } else if (frame <= 13UL) {
          // 撞击时闪出柔和的薄荷绿，亮度够明显但不使用荧光纯绿。
          // Soft mint impact flash, bright but not pure neon green.
          const float flash = (frame == 11UL) ? 1.0f : (frame == 12UL ? 0.72f : 0.42f);
          r = (int)(118.0f * flash);
          g = (int)(232.0f * flash);
          b = (int)(166.0f * flash);
        } else {
          // 偏暗的翠绿余光逐渐淡出。
          // Muted emerald afterglow fades away.
          const float fade = clamp01(1.0f - (float)(frame - 14UL) / 11.0f);
          const float residual = 0.25f + 0.75f * (0.5f + 0.5f * sinf(x * 14.0f + t));
          r = (int)(10.0f * fade * residual);
          g = (int)(76.0f * fade * residual);
          b = (int)(48.0f * fade * residual);
        }
        break;
      }
      case 2: { // Incendio：用暖琥珀色和柔和金色做火焰，并让亮度轻轻变化。 / Incendio: warm amber and soft gold flame, with gentle variation
        int noise = (int)((i * 47UL + frame * 71UL + (i * frame * 7UL)) % 101UL);
        float turbulence = 0.35f + 0.65f * noise / 100.0f;
        float flame = turbulence * (0.55f + 0.45f * (0.5f + 0.5f * sinf(t * 1.35f + x * 12.0f)));
        r = 128 + (int)(105 * flame);
        g = 58 + (int)((68 + 88 * flame) * (0.55f + 0.45f * x));
        b = 24 + (int)(32 * flame);
        break;
      }
      case 3: { // Leviosa：蓝色和淡紫色交替的月光波纹。 / Leviosa: blue/lavender moonlight wave
        float breath = 0.5f + 0.5f * sinf(t * 0.55f + x * 9.0f);
        r = 35 + (int)(105 * breath);
        g = 60 + (int)(95 * (1.0f - breath));
        b = 125 + (int)(100 * breath);
        break;
      }
      case 4: { // Oppugno：几道翠绿色光带依次追逐移动。 / Oppugno: chasing emerald streaks
        int travelling = (int)(frame * 5UL % LED_COUNT);
        int distance = abs(i - travelling);
        float streak = clamp01(1.0f - distance / 11.0f);
        r = 3 + (int)(125 * streak);
        g = 48 + (int)(168 * streak) + (int)(18 * w);
        b = 34 + (int)(85 * (1.0f - streak));
        break;
      }
      case 5: { // Stupefy：紫色和玫瑰色的电击波纹。 / Stupefy: violet / rose electric shockwaves
        float shock = ((frame % 8UL) < 2UL) ? 1.0f : 0.42f;
        float ripple = 0.5f + 0.5f * sinf(x * 19.0f - t * 1.8f);
        r = (int)((84 + 140 * ripple) * shock);
        g = (int)((22 + 55 * (1.0f - ripple)) * shock);
        b = (int)((135 + 90 * ripple) * shock);
        break;
      }
      default:
        break;
    }
    pixel(i, limitByte(r), limitByte(g), limitByte(b));
  }
}

// 手势没有识别出来时，按顺序播放预览 2、3、5。
// On an unrecognised gesture, play preview keys 2 -> 3 -> 5 in order.
static const int FALLBACK_SPELLS[] = {1, 2, 4};
static constexpr uint8_t FALLBACK_SPELL_COUNT = 3;

static void startSpell(int spell) {
  if (spell < 0 || spell >= 6) return;
  activeSpell = spell;
  effectStarted = millis();
  lastFrameAt = millis() - FRAME_INTERVAL_MS;
  frameIndex = 0;
  effectActive = true;
  Serial.print("[SPELL] ");
  Serial.println(SPELL_NAMES[spell]);
}

static void updateEffect() {
  if (!effectActive || !mqtt.connected()) return;
  uint32_t now = millis();
  if ((uint32_t)(now - lastFrameAt) < FRAME_INTERVAL_MS) return;
  lastFrameAt = now;
  renderEffect(activeSpell, frameIndex++);
  if (!publishPixels()) Serial.println("[MQTT] Frame publish failed");
  if ((uint32_t)(now - effectStarted) >= EFFECT_DURATION_MS) {
    // 最后一帧会保持显示，直到下一次施法或串口发送 o。
    // Freeze the final frame until the next cast or an explicit serial 'o'.
    effectActive = false;
    Serial.println("[SPELL] Effect complete; final frame held.");
  }
}

static int captureAndClassify() {
  if (EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE != SAMPLE_COUNT * AXES) {
    Serial.println("[ERROR] Model input size != 600. Check model export.");
    return -1;
  }
  Serial.println("[IMU] Capturing 2 seconds. Cast NOW!");
  const bool cueOn = mqtt.connected();
  if (cueOn) showCountdown(LED_COUNT);   // 灯条全亮表示现在开始采样。 / full bar = capture starts now
  uint32_t nextSample = micros();
  uint32_t lateSamples = 0;
  for (int n = 0; n < SAMPLE_COUNT; ++n) {
    if ((int32_t)(micros() - nextSample) > (int32_t)(SAMPLE_INTERVAL_US / 2)) ++lateSamples;
    while ((int32_t)(micros() - nextSample) < 0) {
      delayMicroseconds(100);
    }
    nextSample += SAMPLE_INTERVAL_US;
    sensors_event_t a, g, temp;
    imu.getEvent(&a, &g, &temp);
    int offset = n * AXES;
    constexpr float MPS2_PER_G = 9.80665f;
    constexpr float GYRO_RAD_TO_DEG = 57.2957795f;
    features[offset]     = ACCEL_IN_G ? a.acceleration.x / MPS2_PER_G : a.acceleration.x;
    features[offset + 1] = ACCEL_IN_G ? a.acceleration.y / MPS2_PER_G : a.acceleration.y;
    features[offset + 2] = ACCEL_IN_G ? a.acceleration.z / MPS2_PER_G : a.acceleration.z;
    features[offset + 3] = g.gyro.x * GYRO_RAD_TO_DEG;
    features[offset + 4] = g.gyro.y * GYRO_RAD_TO_DEG;
    features[offset + 5] = g.gyro.z * GYRO_RAD_TO_DEG;

    // 倒计时灯会逐个熄灭。每次读完一个样本就发送灯光更新，
    // Countdown: LEDs go out one by one. Sent right after a sample is read,
    // 这样 MQTT 发布可以利用下一个样本到来前的空档。
    // so the publish happens in the idle time before the next sample.
    const int done = n + 1;
    if (cueOn && (done % COUNTDOWN_EVERY_N_SAMPLES == 0 || done == SAMPLE_COUNT)) {
      const int lit = LED_COUNT - (LED_COUNT * done) / SAMPLE_COUNT;
      showCountdown(lit);
    }
  }
  if (lateSamples > 0) {
    Serial.print("[IMU] Warning: ");
    Serial.print(lateSamples);
    Serial.println(" samples were late (slow MQTT). Recognition may be less accurate.");
  }
  Serial.println("[IMU] 100 samples captured. All LEDs off = input finished. Running inference...");
  signal_t signal;
  int signalError = numpy::signal_from_buffer(features, SAMPLE_COUNT * AXES, &signal);
  if (signalError != 0) {
    Serial.print("[AI] Signal error: ");
    Serial.println(signalError);
    return -1;
  }
  ei_impulse_result_t result = {0};
  EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);
  if (err != EI_IMPULSE_OK) {
    Serial.print("[AI] Classifier error: ");
    Serial.println((int)err);
    return -1;
  }
  float bestConfidence = -1.0f;
  const char* bestLabel = nullptr;
  Serial.println("[AI] Class probabilities:");
  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; ++i) {
    Serial.print("  ");
    Serial.print(result.classification[i].label);
    Serial.print(": ");
    Serial.println(result.classification[i].value, 4);
    if (result.classification[i].value > bestConfidence) {
      bestConfidence = result.classification[i].value;
      bestLabel = result.classification[i].label;
    }
  }
  if (!bestLabel || bestConfidence < MIN_CONFIDENCE) {
    Serial.println("[AI] Low confidence: no light effect.");
    return -1;
  }
  String label = String(bestLabel);
  label.toLowerCase();
  for (int i = 0; i < 6; ++i) {
    String candidate = String(SPELL_NAMES[i]);
    candidate.toLowerCase();
    if (label == candidate) {
      Serial.print("[AI] Recognised: ");
      Serial.print(bestLabel);
      Serial.print(" (");
      Serial.print(bestConfidence * 100.0f, 1);
      Serial.println("%)");
      return i;
    }
  }
  Serial.print("[AI] Unmapped class: ");
  Serial.println(bestLabel);
  return -1;
}

// 开关拨到 ON 或串口输入 g 时开始施法。
// Triggered by switching ON or serial 'g'.
static void castSpell() {
  // 先停掉上一个灯效，免得它覆盖采样提示。
  // Prevent an older effect from overwriting the capture cue.
  effectActive = false;
  Serial.println("[SWITCH] Spell triggered: flash x2, then LEDs count down 2 s");
  if (mqtt.connected()) {
    flashFeedback(CUE_R, CUE_G, CUE_B);   // 开始提示：闪两次。 / start warning: flashes twice
  } else {
    Serial.println("[LIGHT] Cue unavailable (MQTT disconnected); capturing anyway");
  }

  int spell = captureAndClassify();
  if (spell >= 0) {
    Serial.println("[LIGHT] Purple = recognised");
    if (mqtt.connected()) flashFeedback(155, 55, 235);
    startSpell(spell);
  } else {
    // 每次识别失败就前进一项：先 2，再 3，最后 5。
    // Advance once per failed recognition: first key 2, then 3, then 5.
    const int fallbackSpell = FALLBACK_SPELLS[fallbackDemoStep];
    Serial.print("[AI] Gesture not recognised; showing demo key ");
    Serial.println(fallbackSpell + 1);
    fallbackDemoStep = (fallbackDemoStep + 1) % FALLBACK_SPELL_COUNT;
    if (mqtt.connected()) flashFeedback(210, 145, 70);
    startSpell(fallbackSpell);
  }
}

// 按键消抖后检测 OFF 到 ON 的变化；每次拨到 ON 只施放一次。
// Debounced OFF -> ON detection: one cast each time the switch is turned ON.
// 开关一直保持 ON 不会重复施放，拨回 OFF 也不会触发。
// Leaving it ON does not repeat; switching OFF does nothing.
static void pollButton() {
  bool raw = digitalRead(BUTTON_PIN);
  uint32_t now = millis();
  if (raw != buttonLastRaw) {
    buttonChangedAt = now;
    buttonLastRaw = raw;
  }
  if ((uint32_t)(now - buttonChangedAt) >= BUTTON_DEBOUNCE_MS &&
      raw != buttonStable) {
    buttonStable = raw;
    if (buttonStable == SWITCH_ON_LEVEL || CAST_ON_EVERY_PRESS) {
      Serial.println(buttonStable == SWITCH_ON_LEVEL ? "[SWITCH] ON" : "[SWITCH] OFF");
      castSpell();
      // 施法过程会暂时占用程序；结束后重新读取开关状态，
      // Re-sync after the blocking cast so a switch flipped during it
      // 这样过程中拨动开关也能正确同步，不会因此又施放一次。
      // is read correctly and never triggers a second cast by itself.
      buttonLastRaw = digitalRead(BUTTON_PIN);
      buttonStable = buttonLastRaw;
      buttonChangedAt = millis();
    } else {
      Serial.println("[SWITCH] OFF (switch ON again to cast)");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\nSPELLBOUND - IMU + Edge Impulse + Vespera #11");
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!imu.begin(0x68, &Wire)) {
    Serial.println("[FATAL] MPU6050 not found. Check SDA=23 SCL=22, power and GND.");
    while (true) delay(1000);
  }
  imu.setAccelerometerRange(MPU6050_RANGE_2_G);
  imu.setGyroRange(MPU6050_RANGE_250_DEG);
  imu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("[IMU] MPU6050 connected.");
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  buttonLastRaw = digitalRead(BUTTON_PIN);
  buttonStable = buttonLastRaw;
  buttonChangedAt = millis();
  Serial.print("[SWITCH] INPUT_PULLUP on GPIO ");
  Serial.print(BUTTON_PIN);
  Serial.println(buttonStable == SWITCH_ON_LEVEL
                     ? " - switch is ON at boot: turn it OFF, then ON to cast"
                     : " - switch is OFF: turn it ON to cast");
  WiFi.mode(WIFI_STA);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(512);
  lastWifiAttempt = millis() - 10000UL;
  lastMqttAttempt = millis() - 5000UL;
  Serial.println("[SYSTEM] Switch ON or g=gesture; 1..6=preview; o=off");
  Serial.println("[SYSTEM] Wait for [MQTT] Connected! before testing lights.");
}

void loop() {
  serviceNetwork();
  pollButton();
  while (Serial.available()) {
    char cmd = (char)Serial.read();
    if (cmd == 'g' || cmd == 'G') {
      castSpell();
    } else if (cmd >= '1' && cmd <= '6') {
      startSpell(cmd - '1');
    } else if (cmd == 'o' || cmd == 'O') {
      effectActive = false;
      clearPixels();
      Serial.println(publishPixels() ? "[LIGHT] OFF" : "[LIGHT] OFF publish failed");
    }
  }
  updateEffect();
  delay(5);
}