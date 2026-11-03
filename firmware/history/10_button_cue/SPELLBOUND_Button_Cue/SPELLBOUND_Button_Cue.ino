#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <MAGIC_inferencing.h>  // If your WORKING sketch uses MAGIC2_inferencing.h, change ONLY this line.
#include <math.h>
#include <string.h>
#include "arduino_secrets.h"

// SPELLBOUND: ESP32 Feather + MPU6050 + Edge Impulse + Vespera #11
// Keep IMU settings, input units, and axis order identical to the successful gesture test.
constexpr int SDA_PIN = 23;
constexpr int SCL_PIN = 22;
// Physical momentary button: GPIO27 to GND (INPUT_PULLUP).
// Change this to the actual GPIO you wired.
constexpr int BUTTON_PIN = 27;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 35;
constexpr int SAMPLE_RATE = 50;
constexpr int SAMPLE_COUNT = 100;
constexpr int AXES = 6;
constexpr uint32_t SAMPLE_INTERVAL_US = 1000000UL / SAMPLE_RATE;
constexpr float MIN_CONFIDENCE = 0.65f;
// Based on the earlier training samples (gravity approximately 9.8): acceleration in m/s^2.
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

// Vespera #11 doubles as the wand's button/status indicator.
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

// 6 effects. Animated frames use only 216 raw RGB bytes per MQTT message.
static void renderEffect(int spell, uint32_t frame) {
  float t = frame * 0.38f;
  for (int i = 0; i < LED_COUNT; ++i) {
    float x = (float)i / (float)(LED_COUNT - 1);
    float w = 0.5f + 0.5f * sinf(x * 11.0f - t);
    int r = 0, g = 0, b = 0;
    switch (spell) {
      case 0: { // Ascendio: rising turquoise / mint glints
        int travelling = (int)(frame * 3UL % LED_COUNT);
        int distance = abs(i - travelling);
        float glint = clamp01(1.0f - distance / 8.0f);
        r = 8 + (int)(95 * glint);
        g = 62 + (int)(100 * w) + (int)(45 * glint);
        b = 92 + (int)(70 * (1.0f - x)) + (int)(35 * glint);
        break;
      }
      case 1: { // Ci: champagne-gold impact flash
        float flash = ((frame % 9UL) < 2UL) ? 1.0f : (0.38f + 0.20f * w);
        r = (int)(220 * flash);
        g = (int)((126 + 48 * x) * flash);
        b = (int)((25 + 105 * x) * flash);
        break;
      }
      case 2: { // Incendio: ember-red / orange flicker
        int noise = (int)((i * 47UL + frame * 71UL + (i * frame * 7UL)) % 101UL);
        float flame = 0.35f + 0.65f * noise / 100.0f;
        r = 105 + (int)(125 * flame);
        g = 9 + (int)(112 * flame * (0.45f + 0.55f * x));
        b = 4 + (int)(21 * (1.0f - flame));
        break;
      }
      case 3: { // Leviosa: blue/lavender moonlight wave
        float breath = 0.5f + 0.5f * sinf(t * 0.55f + x * 9.0f);
        r = 35 + (int)(105 * breath);
        g = 60 + (int)(95 * (1.0f - breath));
        b = 125 + (int)(100 * breath);
        break;
      }
      case 4: { // Oppugno: chasing emerald streaks
        int travelling = (int)(frame * 5UL % LED_COUNT);
        int distance = abs(i - travelling);
        float streak = clamp01(1.0f - distance / 11.0f);
        r = 3 + (int)(125 * streak);
        g = 48 + (int)(168 * streak) + (int)(18 * w);
        b = 34 + (int)(85 * (1.0f - streak));
        break;
      }
      case 5: { // Stupefy: violet / rose electric shockwaves
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
    effectActive = false;
    Serial.println("[SPELL] Effect complete; last colour retained.");
  }
}

static int captureAndClassify() {
  if (EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE != SAMPLE_COUNT * AXES) {
    Serial.println("[ERROR] Model input size != 600. Check model export.");
    return -1;
  }
  Serial.println("[IMU] Capturing 2 seconds. Cast NOW!");
  uint32_t nextSample = micros();
  for (int n = 0; n < SAMPLE_COUNT; ++n) {
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
  }
  Serial.println("[IMU] 100 samples captured. Running inference...");
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

// Triggered by either the physical button or serial 'g'.
static void castSpell() {
  // Prevent an older effect from overwriting the cyan capture cue.
  effectActive = false;
  Serial.println("[BUTTON] Spell triggered; cyan = capturing");
  if (!showSolid(10, 185, 230)) {
    Serial.println("[LIGHT] Cue unavailable (MQTT disconnected)");
  }

  int spell = captureAndClassify();
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
}

// Debounced falling-edge detection: one cast per press, no repeat while held.
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
  Serial.print("[BUTTON] INPUT_PULLUP on GPIO ");
  Serial.println(BUTTON_PIN);
  WiFi.mode(WIFI_STA);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(512);
  lastWifiAttempt = millis() - 10000UL;
  lastMqttAttempt = millis() - 5000UL;
  Serial.println("[SYSTEM] Button press or g=gesture; 1..6=preview; o=off");
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
