/*
 * SPELLBOUND - ESP32 Feather + MPU6050 + Edge Impulse (MAGIC) + Vespera #11
 *
 * 模型：MAGIC_inferencing（Edge Impulse 项目 "MAGIC"）
 * 输入：6 轴 IMU，50 Hz，100 个样本（2 秒）
 *       accX, accY, accZ 单位 m/s^2 | gyroX, gyroY, gyroZ 单位 deg/s
 *       （对着 MAGIC 训练导出核过，227 个样本全是这个格式）
 *
 * 操作：
 *   按钮（GPIO27 -> GND）按一下施法：全灯青色准备 350ms，随后 12 组灯
 *   在 2 秒采样期间逐组熄灭；全灭后开始识别。
 *   串口 'g'              施法
 *   串口 '1'..'6'         预览灯光效果（2 = ci）
 *   串口 'o'              全部关灯
 *
 * Vespera #11 上的灯光提示：
 *   青色   = 先亮 350ms 准备；采样期间 12 组灯从高像素编号向低编号逐组熄灭
 *   紫色   = 识别到了，然后播放咒语效果
 *   红色   = 没识别出来，再试一次
 *
 * 注意：只能装一个 Edge Impulse 库！编译前把 Arduino/libraries 里的
 *       MAGIC2_inferencing 删掉，不然 IDE 可能选错 SDK。
 *
 * IMPORTANT: Only install ONE Edge Impulse library. Remove MAGIC2_inferencing
 * from Arduino/libraries before compiling, or the IDE may pick the wrong SDK.
 *
 * 这一版整合了准备阶段、灯带倒计时和传感器诊断。诊断用于观察实际表现，
 * 不代表识别准确率已经提高；动作过滤阈值仍需结合每个手势实测。
 * Combined preparation, countdown and sensor diagnostics. Diagnostics help inspect
 * the run; they do not prove a higher recognition accuracy.
 *
 * 这一版保留的技术设置 / Other retained settings:
 *   - I2C 使用 400kHz
 *   - 采样间隔和传感器饱和诊断
 *   - 可选的低动作过滤和自动重试（重试默认关闭）
 *   - I2C at 400kHz; sample-interval and saturation diagnostics
 *   - Optional motion gate and retry (retry is disabled by default)
 */

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <MAGIC_inferencing.h>
#include <math.h>
#include <string.h>
#include "arduino_secrets.h"

// ==================================================
// 0. 编译开关 / Compile-time switches
// ==================================================
// crit 演示前可以把 DIAG_ENABLED 改成 0，少刷点串口
// Set DIAG_ENABLED to 0 before the crit to reduce serial spam
#define DIAG_ENABLED 1
// 低置信度自动重试，默认关，开了最坏情况一次施法 4 秒
// Auto-retry on low confidence, off by default; worst case 4s per cast if on
#define ENABLE_RETRY 0
// 静止误触发过滤，如果某个轻动作被误杀，改成 0 或者调低阈值
// Motion gate; set to 0 or lower the threshold if a gentle gesture gets rejected
#define MOTION_GATE_ENABLED 1

// ==================================================
// 1. 硬件接线 / Hardware
// ==================================================
// 这几个引脚是对着板子一根根试出来的，MPU6050 先占了 GND，按钮就接 27
// Checked these pin by pin on the board; MPU6050 already uses GND so the button goes on 27
constexpr int SDA_PIN = 23;
constexpr int SCL_PIN = 22;
constexpr uint8_t MPU_ADDRESS = 0x68;
// 按钮一边接 GPIO27 一边接 GND，用内部上拉，按下就是 LOW
// Button: one side GPIO27, other side GND, internal pull-up, pressed = LOW
constexpr int BUTTON_PIN = 27;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 35;

// ==================================================
// 2. 采样参数（必须和训练数据一致！） / Sampling (must match training)
// ==================================================
// 这里千万别乱改，模型就是按 50Hz、100 个样本、6 轴训练的
// Don't touch these casually, the model was trained on 50Hz, 100 samples, 6 axes
constexpr int SAMPLE_RATE = 50;
constexpr int SAMPLE_COUNT = 100;
constexpr int AXES = 6;
constexpr size_t FEATURE_COUNT = SAMPLE_COUNT * AXES;
constexpr uint32_t SAMPLE_INTERVAL_US = 1000000UL / SAMPLE_RATE;

// MAGIC 训练数据里加速度是 m/s^2（重力约 9.8），所以这里保持 false
// MAGIC training data uses m/s^2 (gravity ~9.8), so keep this false
constexpr bool ACCEL_IN_G = false;
constexpr float MPS2_PER_G = 9.80665f;
constexpr float GYRO_RAD_TO_DEG = 57.2957795f;

// 编译时就检查，省得装错库跑半天才发现不对
// Compile-time checks so you don't waste time running with the wrong library
static_assert(FEATURE_COUNT == 600, "Expected 600 input features");
static_assert(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE == FEATURE_COUNT,
              "Model frame size mismatch - check the Edge Impulse library");
static_assert(EI_CLASSIFIER_RAW_SAMPLE_COUNT == SAMPLE_COUNT,
              "Model sample count mismatch");
static_assert(EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME == AXES,
              "Model axis count mismatch");
static_assert(EI_CLASSIFIER_FREQUENCY == SAMPLE_RATE,
              "Model sample frequency mismatch");

// ==================================================
// 3. 分类设置 / Classification
// ==================================================
// 0.65 是试出来的，太低会乱识别，太高又老是不触发
// 0.65 was tuned by trial; too low = random triggers, too high = never fires
constexpr float MIN_CONFIDENCE = 0.65f;
// true  = "ci" 直接关灯
// false = "ci" 播放香槟金闪光效果
// true  = "ci" turns all LEDs off
// false = "ci" plays the champagne-gold flash effect
constexpr bool CI_TURNS_OFF = true;

// 顺序要和 Edge Impulse 导出的标签完全一样，不然映射会错
// Order must match the Edge Impulse exported labels exactly, otherwise mapping breaks
const char* SPELL_NAMES[] = {
  "Ascendio", "ci", "incendio", "leviosa", "oppugno", "stupefy"
};
constexpr int SPELL_COUNT = 6;
constexpr int SPELL_CI = 1;
static_assert(EI_CLASSIFIER_LABEL_COUNT == SPELL_COUNT,
              "Model label count mismatch - check the MAGIC export");

// ==================================================
// 3b. 触发和门限 / Trigger and gates
// ==================================================
// 按下按钮后先亮青色 350ms，让手从按钮上拿开、把魔杖摆好
// 这个是我后来加的，不加的话前 0.3 秒全是手指动作
// After the button press, show cyan for 350ms so the hand can leave the button
// and the wand can be positioned; without this the first 0.3s is finger motion
constexpr uint32_t CAST_PREP_MS = 350;

// 陀螺仪峰值低于这个值就当成“没动”，直接拒绝，避免静止时乱触发
// 如果某个轻动作被误杀，把这个调低（比如 25）或者把 MOTION_GATE_ENABLED 设 0
// If gyro peak is below this, treat as "no motion" and reject; avoids idle triggers
// If a gentle gesture gets rejected, lower this (e.g. 25) or set MOTION_GATE_ENABLED to 0
constexpr float NO_MOTION_GYRO_PEAK_DPS = 35.0f;

// ==================================================
// 4. MQTT / Vespera
// ==================================================
// 这些是课程给的，别改成自己的，不然连不上
// These are from the course; don't change them or you won't connect
const char* MQTT_HOST = "mqtt.cetools.org";
constexpr uint16_t MQTT_PORT = 1884;
const char* MQTT_TOPIC = "student/CASA0014/luminaire/11";
constexpr int LED_COUNT = 72;
constexpr int PAYLOAD_SIZE = LED_COUNT * 3;   // 72 * 3 = 216 字节原始 RGB
constexpr uint32_t FRAME_INTERVAL_MS = 140;
constexpr uint32_t EFFECT_DURATION_MS = 3500;

// 72 pixels are displayed as 12 groups of 6; group with highest indices turns off first.
constexpr int STRIP_COUNT = 12;
constexpr int LEDS_PER_STRIP = 6;
static_assert(STRIP_COUNT * LEDS_PER_STRIP == LED_COUNT,
              "Strip layout must cover all 72 LEDs");
constexpr uint8_t CUE_R = 10, CUE_G = 185, CUE_B = 230;

// ==================================================
// 5. 全局变量 / Globals
// ==================================================
Adafruit_MPU6050 imu;
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

static float features[FEATURE_COUNT];
static uint8_t pixels[PAYLOAD_SIZE];

static int activeSpell = -1;
static bool effectActive = false;
static uint32_t effectStarted = 0;
static uint32_t lastFrameAt = 0;
static uint32_t frameIndex = 0;

static uint32_t lastWifiAttempt = 0;
static uint32_t lastMqttAttempt = 0;

static bool buttonLastRaw = HIGH;
static bool buttonStable = HIGH;
static uint32_t buttonChangedAt = 0;

// ==================================================
// 6. 像素小工具 / Pixel helpers
// ==================================================
// 这个就是怕 int 算出来超过 0-255，直接卡住
// Just clamps ints to 0-255 so we don't overflow the byte
static uint8_t limitByte(int n) {
  return (uint8_t)(n < 0 ? 0 : (n > 255 ? 255 : n));
}

static float clamp01(float x) {
  return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
}

// 每个 LED 占 3 个字节，index 是第几颗灯
// Each LED takes 3 bytes; index is which LED
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

// 整条灯带一个颜色
// Whole strip one colour
static bool showSolid(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < LED_COUNT; ++i) pixel(i, r, g, b);
  return publishPixels();
}

// Countdown frame: keep the lowest-index groups lit and switch off higher groups first.
static bool showCountdown(int litStrips) {
  for (int i = 0; i < LED_COUNT; ++i) {
    if (i / LEDS_PER_STRIP < litStrips) pixel(i, CUE_R, CUE_G, CUE_B);
    else pixel(i, 0, 0, 0);
  }
  return publishPixels();
}

static void lightsOff() {
  effectActive = false;
  clearPixels();
  Serial.println(publishPixels() ? "[LIGHT] OFF" : "[LIGHT] OFF publish failed");
}

// 闪两下，用来告诉用户识别成功 / 失败
// Flash twice to tell the user recognition succeeded / failed
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

// 等待一段时间，同时保持 MQTT 心跳，不然准备期里连接可能掉
// Wait for a while while keeping MQTT alive, otherwise the connection may drop
static void waitWithMqtt(uint32_t ms) {
  uint32_t start = millis();
  while ((uint32_t)(millis() - start) < ms) {
    mqtt.loop();
    delay(5);
  }
}

// ==================================================
// 7. 网络（非阻塞重连） / Network (non-blocking reconnect)
// ==================================================
// 这里刻意不用 while 死等，不然采集的时候网络会卡住
// Deliberately not blocking here, otherwise the network stalls during capture
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

// ==================================================
// 8. 灯光效果（每帧 216 字节原始 RGB） / Light effects
// ==================================================
// 这些效果是我一个个对着灯带调出来的，数值都是感觉差不多就留了
// Tuned these one by one against the strip; numbers are "looks about right"
static void renderEffect(int spell, uint32_t frame) {
  float t = frame * 0.38f;
  for (int i = 0; i < LED_COUNT; ++i) {
    float x = (float)i / (float)(LED_COUNT - 1);
    float w = 0.5f + 0.5f * sinf(x * 11.0f - t);
    int r = 0, g = 0, b = 0;

    switch (spell) {
      case 0: { // Ascendio：青绿 / 薄荷色往上跑
        // Ascendio: rising turquoise / mint glints
        int travelling = (int)(frame * 3UL % LED_COUNT);
        int distance = abs(i - travelling);
        float glint = clamp01(1.0f - distance / 8.0f);
        r = 8 + (int)(95 * glint);
        g = 62 + (int)(100 * w) + (int)(45 * glint);
        b = 92 + (int)(70 * (1.0f - x)) + (int)(35 * glint);
        break;
      }
      case 1: { // Ci：香槟金冲击闪光（CI_TURNS_OFF = false 时才用）
        // Ci: champagne-gold impact flash (only when CI_TURNS_OFF = false)
        float flash = ((frame % 9UL) < 2UL) ? 1.0f : (0.38f + 0.20f * w);
        r = (int)(220 * flash);
        g = (int)((126 + 48 * x) * flash);
        b = (int)((25 + 105 * x) * flash);
        break;
      }
      case 2: { // Incendio：红橙余烬闪烁
        // Incendio: ember-red / orange flicker
        int noise = (int)((i * 47UL + frame * 71UL + (i * frame * 7UL)) % 101UL);
        float flame = 0.35f + 0.65f * noise / 100.0f;
        r = 105 + (int)(125 * flame);
        g = 9 + (int)(112 * flame * (0.45f + 0.55f * x));
        b = 4 + (int)(21 * (1.0f - flame));
        break;
      }
      case 3: { // Leviosa：蓝紫月光波
        // Leviosa: blue / lavender moonlight wave
        float breath = 0.5f + 0.5f * sinf(t * 0.55f + x * 9.0f);
        r = 35 + (int)(105 * breath);
        g = 60 + (int)(95 * (1.0f - breath));
        b = 125 + (int)(100 * breath);
        break;
      }
      case 4: { // Oppugno：翠绿追踪光条
        // Oppugno: chasing emerald streaks
        int travelling = (int)(frame * 5UL % LED_COUNT);
        int distance = abs(i - travelling);
        float streak = clamp01(1.0f - distance / 11.0f);
        r = 3 + (int)(125 * streak);
        g = 48 + (int)(168 * streak) + (int)(18 * w);
        b = 34 + (int)(85 * (1.0f - streak));
        break;
      }
      case 5: { // Stupefy：紫玫瑰色电击波纹
        // Stupefy: violet / rose electric shockwaves
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

static void startSpell(int spell) {
  if (spell < 0 || spell >= SPELL_COUNT) return;

  Serial.print("[SPELL] ");
  Serial.println(SPELL_NAMES[spell]);

  // ci 我设成直接关灯，省得每次还要按 'o'
  // I set ci to just turn things off, saves pressing 'o' every time
  if (spell == SPELL_CI && CI_TURNS_OFF) {
    lightsOff();
    return;
  }

  activeSpell = spell;
  effectStarted = millis();
  lastFrameAt = millis() - FRAME_INTERVAL_MS;
  frameIndex = 0;
  effectActive = true;
}

static void updateEffect() {
  if (!effectActive || !mqtt.connected()) return;
  uint32_t now = millis();
  if ((uint32_t)(now - lastFrameAt) < FRAME_INTERVAL_MS) return;
  lastFrameAt = now;

  renderEffect(activeSpell, frameIndex++);
  if (!publishPixels()) Serial.println("[MQTT] Frame publish failed");

  if ((uint32_t)(now - effectStarted) >= EFFECT_DURATION_MS) {
    effectActive = false;
    Serial.println("[SPELL] Effect complete; last colour retained.");
  }
}

// ==================================================
// 9. 采集 + 推理 / Capture + inference
// During the 2 s window, capture 100 samples at 50 Hz and send a countdown frame
// only when one of the 12 groups turns off. MQTT publish time is included in timing diagnostics.
// ==================================================
static int captureAndClassify() {
  Serial.println("[IMU] Capturing 2 seconds. Cast NOW! Strips go out as time passes.");

  uint32_t satAcc = 0, satGyro = 0;
  uint32_t prevUs = micros();
  uint32_t minDt = 0xFFFFFFFFUL, maxDt = 0, sumDt = 0;
  uint32_t lateSamples = 0;
  bool countdownPublishFailed = false;
  const bool cueOn = mqtt.connected();
  int litStrips = STRIP_COUNT;
  uint32_t nextSample = prevUs;

  for (int n = 0; n < SAMPLE_COUNT; ++n) {
    if (n > 0) {
      nextSample += SAMPLE_INTERVAL_US;
      while ((int32_t)(micros() - nextSample) < 0) {
        delayMicroseconds(100);
      }
      if ((int32_t)(micros() - nextSample) > (int32_t)(SAMPLE_INTERVAL_US / 2)) {
        ++lateSamples;
      }
    }

    sensors_event_t a, g, temp;
    if (!imu.getEvent(&a, &g, &temp)) {
      Serial.println("[IMU] Sensor read failed.");
      return -1;
    }

    uint32_t nowUs = micros();
    uint32_t dt = nowUs - prevUs;
    prevUs = nowUs;
    if (n > 0) {
      if (dt < minDt) minDt = dt;
      if (dt > maxDt) maxDt = dt;
      sumDt += dt;
    }

    // Check clipping in sensor units before converting to model units.
    if (fabsf(a.acceleration.x) > 19.0f ||
        fabsf(a.acceleration.y) > 19.0f ||
        fabsf(a.acceleration.z) > 19.0f) satAcc++;
    if (fabsf(g.gyro.x) > 4.2f ||
        fabsf(g.gyro.y) > 4.2f ||
        fabsf(g.gyro.z) > 4.2f) satGyro++;

    // Channel order must stay ax, ay, az, gx, gy, gz.
    const int offset = n * AXES;
    features[offset]     = ACCEL_IN_G ? a.acceleration.x / MPS2_PER_G : a.acceleration.x;
    features[offset + 1] = ACCEL_IN_G ? a.acceleration.y / MPS2_PER_G : a.acceleration.y;
    features[offset + 2] = ACCEL_IN_G ? a.acceleration.z / MPS2_PER_G : a.acceleration.z;
    features[offset + 3] = g.gyro.x * GYRO_RAD_TO_DEG;
    features[offset + 4] = g.gyro.y * GYRO_RAD_TO_DEG;
    features[offset + 5] = g.gyro.z * GYRO_RAD_TO_DEG;

    // Turn off one group at each proportional step through the capture window.
    const int done = n + 1;
    const int strips = STRIP_COUNT - (STRIP_COUNT * done) / SAMPLE_COUNT;
    if (cueOn && strips != litStrips) {
      litStrips = strips;
      if (!showCountdown(litStrips)) countdownPublishFailed = true;
    }
  }

  if (countdownPublishFailed) {
    Serial.println("[LIGHT] One or more countdown frames could not be published.");
  }
  if (lateSamples > 0) {
    Serial.print("[IMU] Warning: ");
    Serial.print(lateSamples);
    Serial.println(" samples started late; MQTT traffic may have affected timing.");
  }
  Serial.println("[IMU] 100 samples captured; all countdown groups are off. Running inference...");

#if DIAG_ENABLED
  const uint32_t meanDt = sumDt / (uint32_t)(SAMPLE_COUNT - 1);
  Serial.printf("[DIAG] dt min=%lu max=%lu mean=%lu us (target 20000)\n",
                (unsigned long)minDt, (unsigned long)maxDt, (unsigned long)meanDt);
  Serial.printf("[DIAG] satAcc=%lu satGyro=%lu / %d samples\n",
                (unsigned long)satAcc, (unsigned long)satGyro, SAMPLE_COUNT);
#endif

  float gyroPeak = 0.0f;
  for (int n = 0; n < SAMPLE_COUNT; ++n) {
    const int o = n * AXES;
    const float mag = sqrtf(features[o + 3] * features[o + 3] +
                            features[o + 4] * features[o + 4] +
                            features[o + 5] * features[o + 5]);
    if (mag > gyroPeak) gyroPeak = mag;
  }
#if DIAG_ENABLED
  Serial.printf("[DIAG] gyroPeak=%.1f dps (gate=%.1f)\n",
                gyroPeak, NO_MOTION_GYRO_PEAK_DPS);
#endif
#if MOTION_GATE_ENABLED
  if (gyroPeak < NO_MOTION_GYRO_PEAK_DPS) {
    Serial.println("[AI] No motion detected, skip classification.");
    return -1;
  }
#endif

  signal_t signal;
  int signalError = numpy::signal_from_buffer(features, FEATURE_COUNT, &signal);
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
    Serial.print("[AI] Low confidence (");
    Serial.print(bestConfidence * 100.0f, 1);
    Serial.println("%): no light effect.");
    return -1;
  }

  String label = String(bestLabel);
  label.toLowerCase();
  for (int i = 0; i < SPELL_COUNT; ++i) {
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

// Button or serial 'g' both end up here
static void castSpell() {
  effectActive = false;  // 先把正在播的效果停掉，不然会盖住提示色
                          // Stop any running effect first, otherwise it overwrites the cue
  Serial.println("[CAST] Triggered; cyan prep, then strip countdown during capture");
  if (!showSolid(CUE_R, CUE_G, CUE_B)) {
    Serial.println("[LIGHT] Cue unavailable (MQTT disconnected)");
  }

  // 关键改动：青色亮起后等 350ms，让手离开按钮、把魔杖摆好
  // Key change: wait 350ms after cyan so the hand leaves the button and
  // the wand can be positioned before the 2s window starts
  waitWithMqtt(CAST_PREP_MS);

  int spell = captureAndClassify();

#if ENABLE_RETRY
  if (spell < 0) {
    Serial.println("[CAST] First attempt failed, retrying once...");
    showSolid(CUE_R, CUE_G, CUE_B);
    waitWithMqtt(CAST_PREP_MS);
    spell = captureAndClassify();
  }
#endif

  if (spell >= 0) {
    Serial.println("[LIGHT] Purple = recognised");
    if (mqtt.connected()) flashFeedback(155, 55, 235);
    startSpell(spell);
  } else {
    Serial.println("[LIGHT] Red = retry");
    if (mqtt.connected()) {
      flashFeedback(255, 25, 65);
      showSolid(0, 0, 0);
    }
  }
  Serial.println("[SYSTEM] Ready for next spell.");
}

// ==================================================
// 10. 按钮（消抖，按下沿触发，一次按下只施法一次）
// Button (debounced falling edge, one cast per press)
// ==================================================
// 一开始没消抖，按住会疯狂触发，后来加了 35ms
// Didn't debounce at first, holding it triggered like crazy; added 35ms later
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
    if (buttonStable == LOW) {
      castSpell();
    }
  }
}

// ==================================================
// 11. setup / loop
// ==================================================
void setup() {
  Serial.begin(115200);
  delay(1500);  // 等串口监视器打开，不然前面几行看不到
                // Wait for serial monitor to open, otherwise first lines are lost
  Serial.println("\nSPELLBOUND - IMU + Edge Impulse + Vespera #11");
  Serial.print("[MODEL] Project: ");
  Serial.println(EI_CLASSIFIER_PROJECT_NAME);
  Serial.print("[MODEL] Classes: ");
  Serial.println(EI_CLASSIFIER_LABEL_COUNT);

  Wire.begin(SDA_PIN, SCL_PIN);
  // I2C 提到 400kHz，读 14 字节从约 1.4ms 降到约 0.35ms，采样抖动更小
  // I2C at 400kHz: reading 14 bytes drops from ~1.4ms to ~0.35ms, less jitter
  Wire.setClock(400000);

  if (!imu.begin(MPU_ADDRESS, &Wire)) {
    Serial.println("[FATAL] MPU6050 not found. Check SDA=23 SCL=22, power and GND.");
    while (true) delay(1000);  // 找不到就停这儿，省得后面全是错
                               // Stop here if not found, otherwise everything downstream fails
  }
  // 注意：这里保持 ±2g / ±250dps，和训练数据一致最重要
  // 如果 [DIAG] 里 satGyro 经常 > 20，说明挥动时饱和了，
  // 但改量程前一定要先确认训练 CSV 也是这个量程
  // Note: keep ±2g / ±250dps here; matching the training data matters most
  // If [DIAG] shows satGyro > 20 often, the wand is clipping during fast swings,
  // but check the training CSV used the same range before changing this
  imu.setAccelerometerRange(MPU6050_RANGE_2_G);
  imu.setGyroRange(MPU6050_RANGE_250_DEG);
  imu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("[IMU] MPU6050 connected (+/-2g, +/-250 dps, 21 Hz).");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  buttonLastRaw = digitalRead(BUTTON_PIN);
  buttonStable = buttonLastRaw;
  buttonChangedAt = millis();
  Serial.print("[BUTTON] INPUT_PULLUP on GPIO ");
  Serial.println(BUTTON_PIN);

  WiFi.mode(WIFI_STA);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(512);  // 216 字节默认够，但留点余量
                            // 216 bytes fits default, but leave some headroom
  lastWifiAttempt = millis() - 10000UL;
  lastMqttAttempt = millis() - 5000UL;

  Serial.println("[SYSTEM] Button or 'g' = cast; '1'..'6' = preview; 'o' = off");
  Serial.println("[SYSTEM] Wait for [MQTT] Connected! before testing lights.");
}

void loop() {
  serviceNetwork();
  pollButton();

  // 串口命令，调试的时候很方便
  // Serial commands, very handy while debugging
  while (Serial.available()) {
    char cmd = (char)Serial.read();
    if (cmd == 'g' || cmd == 'G') {
      castSpell();
    } else if (cmd >= '1' && cmd <= '6') {
      startSpell(cmd - '1');
    } else if (cmd == 'o' || cmd == 'O') {
      lightsOff();
    }
  }

  updateEffect();
  delay(5);
}