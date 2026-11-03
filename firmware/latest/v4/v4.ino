
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <MAGIC2_inferencing.h>
#include <math.h>
#include <string.h>
#include <arduino_secrets.h>
// =====================================================
// SPELLBOUND v4
// ESP32 Feather + MPU6050 + Edge Impulse + MQTT
// Hold button -> capture gesture
// Release button -> classify -> cast spell
// =====================================================

// ---------- Wi-Fi / MQTT ----------
// Fill in your actual Wi-Fi details.
const char* WIFI_SSID = "CE-Hub-Student";
const char* WIFI_PASSWORD = "";

const char* MQTT_SERVER = "mqtt.cetools.org";
const uint16_t MQTT_PORT = 1884;

const char* MQTT_TOPIC =
    "student/CASA0014/luminaire/11";

// ---------- Hardware ----------
const int BUTTON_PIN = 27;

// Feather ESP32 default I2C pins.
// Change only if your working MPU6050 sketch uses others.
const int I2C_SDA_PIN = 23;
const int I2C_SCL_PIN = 22;

// ---------- Edge Impulse ----------
#if EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 6
#error "This sketch requires a 6-axis Edge Impulse model."
#endif

const size_t AXES = 6;

const size_t REQUIRED_SAMPLES =
    EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE / AXES;

const uint32_t SAMPLE_INTERVAL_MS =
    (uint32_t)(EI_CLASSIFIER_INTERVAL_MS + 0.5f);

// The gyro conversion must match your training data.
// true  = degrees/second
// false = radians/second
const bool GYRO_IN_DEGREES = true;

const float CONFIDENCE_THRESHOLD = 0.70f;

// ---------- Vespera RGB ----------
const int LED_COUNT = 72;
const int RGB_BYTES = LED_COUNT * 3;

uint8_t pixels[RGB_BYTES];

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
Adafruit_MPU6050 mpu;

// ---------- Capture buffer ----------
// Ring buffer retains the most recent model window.
float imuBuffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

size_t sampleWriteIndex = 0;
size_t collectedSamples = 0;

bool capturing = false;
uint32_t nextSampleTime = 0;

// ---------- Button debounce ----------
bool buttonStablePressed = false;
bool buttonLastReading = false;
uint32_t buttonChangedAt = 0;
const uint32_t DEBOUNCE_MS = 25;

// ---------- Animation ----------
enum Spell {
  SPELL_NONE,
  SPELL_ASCENDIO,
  SPELL_CI,
  SPELL_INCENDIO,
  SPELL_LEVIOSA,
  SPELL_OPPUGNO,
  SPELL_STUPEFY
};

Spell activeSpell = SPELL_NONE;

uint32_t spellStartedAt = 0;
uint32_t lastAnimationAt = 0;
uint32_t animationFrame = 0;

const uint32_t ANIMATION_INTERVAL_MS = 65;
const uint32_t SPELL_DURATION_MS = 7000;

// ---------- Network reconnect ----------
uint32_t lastWiFiAttempt = 0;
uint32_t lastMQTTAttempt = 0;

// =====================================================
// RGB helpers
// =====================================================

void setPixel(int index, uint8_t r,
              uint8_t g, uint8_t b) {
  if (index < 0 || index >= LED_COUNT) return;

  int offset = index * 3;

  pixels[offset] = r;
  pixels[offset + 1] = g;
  pixels[offset + 2] = b;
}

void fillColor(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < LED_COUNT; i++) {
    setPixel(i, r, g, b);
  }
}

void clearPixels() {
  memset(pixels, 0, sizeof(pixels));
}

void publishPixels() {
  if (!mqtt.connected()) return;

  bool ok = mqtt.publish(
      MQTT_TOPIC,
      pixels,
      (unsigned int)RGB_BYTES
  );

  if (!ok) {
    Serial.println("[MQTT] RGB publish failed");
  }
}

void showSolid(uint8_t r, uint8_t g, uint8_t b) {
  fillColor(r, g, b);
  publishPixels();
}

void lightsOff() {
  clearPixels();
  publishPixels();
}

// =====================================================
// Network
// =====================================================

void connectWiFiIfNeeded() {
  if (WiFi.status() == WL_CONNECTED) return;

  uint32_t now = millis();

  if (lastWiFiAttempt != 0 &&
      now - lastWiFiAttempt < 8000) {
    return;
  }

  lastWiFiAttempt = now;

  Serial.println("[WiFi] Connecting...");

  WiFi.mode(WIFI_STA);

  if (strlen(WIFI_PASSWORD) == 0) {
    WiFi.begin(WIFI_SSID);
  } else {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

void connectMQTTIfNeeded() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (mqtt.connected()) return;

  uint32_t now = millis();

  if (lastMQTTAttempt != 0 &&
      now - lastMQTTAttempt < 5000) {
    return;
  }

  lastMQTTAttempt = now;

  String clientId = "SPELLBOUND-";
  clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

  Serial.println("[MQTT] Connecting...");

  if (mqtt.connect(clientId.c_str())) {
    Serial.println("[MQTT] Connected!");
    Serial.print("[MQTT] Topic: ");
    Serial.println(MQTT_TOPIC);

    lightsOff();
  } else {
    Serial.print("[MQTT] Failed, state=");
    Serial.println(mqtt.state());
  }
}

void maintainNetwork() {
  connectWiFiIfNeeded();
  connectMQTTIfNeeded();

  if (mqtt.connected()) {
    mqtt.loop();
  }
}

// =====================================================
// Feedback
// =====================================================

void waitWithMQTT(uint32_t durationMs) {
  uint32_t start = millis();

  while (millis() - start < durationMs) {
    if (mqtt.connected()) mqtt.loop();
    delay(5);
  }
}

void flashFeedback(
    uint8_t r,
    uint8_t g,
    uint8_t b,
    int times
) {
  for (int i = 0; i < times; i++) {
    showSolid(r, g, b);
    waitWithMQTT(140);

    lightsOff();
    waitWithMQTT(100);
  }
}

void successFeedback() {
  Serial.println("[SPELL] Success - purple feedback");

  flashFeedback(160, 0, 255, 2);
}

void failureFeedback() {
  Serial.println("[SPELL] Failed - red feedback");

  flashFeedback(255, 0, 0, 2);
}

// =====================================================
// MPU6050
// =====================================================

void readIMUFrame(float* dst) {
  sensors_event_t a, g, temp;

  mpu.getEvent(&a, &g, &temp);

  // Acceleration in m/s^2
  dst[0] = a.acceleration.x;
  dst[1] = a.acceleration.y;
  dst[2] = a.acceleration.z;

  // Adafruit returns gyroscope in rad/s.
  float gyroFactor =
      GYRO_IN_DEGREES ? RAD_TO_DEG : 1.0f;

  dst[3] = g.gyro.x * gyroFactor;
  dst[4] = g.gyro.y * gyroFactor;
  dst[5] = g.gyro.z * gyroFactor;
}

// =====================================================
// Capture while button is held
// =====================================================

void startCapture() {
  capturing = true;

  sampleWriteIndex = 0;
  collectedSamples = 0;

  nextSampleTime = millis();

  activeSpell = SPELL_NONE;

  Serial.println();
  Serial.println("[BUTTON] PRESSED");
  Serial.println("[CAPTURE] START");

  // Cyan ready indicator
  showSolid(0, 170, 220);
}

void captureStep() {
  if (!capturing) return;

  uint32_t now = millis();

  if ((int32_t)(now - nextSampleTime) < 0) {
    return;
  }

  float frame[AXES];

  readIMUFrame(frame);

  size_t offset = sampleWriteIndex * AXES;

  for (size_t axis = 0; axis < AXES; axis++) {
    imuBuffer[offset + axis] = frame[axis];
  }

  sampleWriteIndex++;

  if (sampleWriteIndex >= REQUIRED_SAMPLES) {
    sampleWriteIndex = 0;
  }

  if (collectedSamples < REQUIRED_SAMPLES) {
    collectedSamples++;
  }

  nextSampleTime += SAMPLE_INTERVAL_MS;

  // Avoid catch-up bursts after scheduling delays.
  if ((int32_t)(now - nextSampleTime) >=
      (int32_t)SAMPLE_INTERVAL_MS) {
    nextSampleTime = now + SAMPLE_INTERVAL_MS;
  }
}

// =====================================================
// Edge Impulse
// =====================================================

float inferenceBuffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

void prepareInferenceBuffer() {
  // sampleWriteIndex points to the oldest sample
  // once the ring buffer is full.

  for (size_t i = 0; i < REQUIRED_SAMPLES; i++) {
    size_t sourceSample =
        (sampleWriteIndex + i) % REQUIRED_SAMPLES;

    for (size_t axis = 0; axis < AXES; axis++) {
      inferenceBuffer[i * AXES + axis] =
          imuBuffer[sourceSample * AXES + axis];
    }
  }
}

Spell labelToSpell(const char* label) {
  String name = String(label);
  name.toLowerCase();

  if (name.indexOf("ascend") >= 0) {
    return SPELL_ASCENDIO;
  }

  if (name == "ci" ||
      name == "accio") {
    return SPELL_CI;
  }

  if (name.indexOf("incend") >= 0) {
    return SPELL_INCENDIO;
  }

  if (name.indexOf("lev") >= 0) {
    return SPELL_LEVIOSA;
  }

  if (name.indexOf("oppugn") >= 0) {
    return SPELL_OPPUGNO;
  }

  if (name.indexOf("stupef") >= 0 ||
      name.indexOf("stupif") >= 0) {
    return SPELL_STUPEFY;
  }

  return SPELL_NONE;
}

const char* spellName(Spell spell) {
  switch (spell) {
    case SPELL_ASCENDIO: return "Ascendio";
    case SPELL_CI:       return "CI / Accio";
    case SPELL_INCENDIO: return "Incendio";
    case SPELL_LEVIOSA:  return "Leviosa";
    case SPELL_OPPUGNO:  return "Oppugno";
    case SPELL_STUPEFY:  return "Stupefy";
    default:             return "Unknown";
  }
}

void activateSpell(Spell spell) {
  activeSpell = spell;

  spellStartedAt = millis();
  lastAnimationAt = 0;
  animationFrame = 0;

  Serial.print("[SPELL] CAST: ");
  Serial.println(spellName(spell));

  if (spell == SPELL_STUPEFY) {
    lightsOff();
    activeSpell = SPELL_NONE;
  }
}

void classifyGesture() {
  Serial.println("[AI] Preparing inference...");

  prepareInferenceBuffer();

  signal_t signal;

  int signalError = numpy::signal_from_buffer(
      inferenceBuffer,
      EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE,
      &signal
  );

  if (signalError != 0) {
    Serial.println("[AI] Signal creation failed");
    failureFeedback();
    return;
  }

  ei_impulse_result_t result = {0};

  EI_IMPULSE_ERROR err = run_classifier(
      &signal,
      &result,
      false
  );

  if (err != EI_IMPULSE_OK) {
    Serial.print("[AI] Inference error: ");
    Serial.println((int)err);
    failureFeedback();
    return;
  }

  float bestScore = -1.0f;
  const char* bestLabel = nullptr;

  Serial.println("[AI] Classification:");

  for (size_t i = 0;
       i < EI_CLASSIFIER_LABEL_COUNT;
       i++) {

    const char* label =
        result.classification[i].label;

    float score =
        result.classification[i].value;

    Serial.print("  ");
    Serial.print(label);
    Serial.print(": ");
    Serial.println(score, 4);

    if (score > bestScore) {
      bestScore = score;
      bestLabel = label;
    }
  }

  if (bestLabel == nullptr ||
      bestScore < CONFIDENCE_THRESHOLD) {

    Serial.println("[AI] Confidence too low");
    failureFeedback();
    lightsOff();
    return;
  }

  Spell detectedSpell = labelToSpell(bestLabel);

  if (detectedSpell == SPELL_NONE) {
    Serial.print("[AI] Unknown label: ");
    Serial.println(bestLabel);

    failureFeedback();
    lightsOff();
    return;
  }

  Serial.print("[AI] Detected: ");
  Serial.print(bestLabel);
  Serial.print(" | Confidence: ");
  Serial.println(bestScore, 4);

  successFeedback();
  activateSpell(detectedSpell);
}

void finishCapture() {
  if (!capturing) return;

  capturing = false;

  Serial.println("[BUTTON] RELEASED");
  Serial.println("[CAPTURE] STOP");

  Serial.print("[CAPTURE] Samples: ");
  Serial.print(collectedSamples);
  Serial.print("/");
  Serial.println(REQUIRED_SAMPLES);

  if (collectedSamples < REQUIRED_SAMPLES) {
    Serial.println(
        "[CAPTURE] Too short - hold longer"
    );

    failureFeedback();
    lightsOff();
    return;
  }

  // Inference happens ONLY after release.
  classifyGesture();
}

// =====================================================
// Button
// =====================================================

void updateButton() {
  bool reading =
      (digitalRead(BUTTON_PIN) == LOW);

  uint32_t now = millis();

  if (reading != buttonLastReading) {
    buttonChangedAt = now;
    buttonLastReading = reading;
  }

  if ((now - buttonChangedAt) < DEBOUNCE_MS) {
    return;
  }

  if (reading == buttonStablePressed) {
    return;
  }

  buttonStablePressed = reading;

  if (buttonStablePressed) {
    startCapture();
  } else {
    finishCapture();
  }
}

// =====================================================
// Animations
// =====================================================

uint8_t clampByte(float value) {
  if (value < 0) return 0;
  if (value > 255) return 255;
  return (uint8_t)value;
}

void renderAscendio() {
  // Golden trail rising through the luminaire.
  clearPixels();

  int head = (animationFrame * 2) % LED_COUNT;

  for (int i = 0; i < LED_COUNT; i++) {
    int distance =
        (head - i + LED_COUNT) % LED_COUNT;

    if (distance < 14) {
      float strength =
          1.0f - (float)distance / 14.0f;

      setPixel(
          i,
          clampByte(255 * strength),
          clampByte(170 * strength),
          clampByte(25 * strength)
      );
    }
  }
}

void renderCI() {
  // Bright blue-white pulse.
  float phase = (animationFrame % 16) / 16.0f;

  float pulse = 1.0f - phase;
  pulse *= pulse;

  uint8_t r = clampByte(150 * pulse);
  uint8_t g = clampByte(210 * pulse);
  uint8_t b = clampByte(255 * pulse);

  fillColor(r, g, b);
}

void renderIncendio() {
  // Red / orange / yellow fire.
  for (int i = 0; i < LED_COUNT; i++) {
    int flicker = random(0, 100);

    uint8_t r = 255;
    uint8_t g = 35 + flicker;
    uint8_t b = 0;

    setPixel(i, r, g, b);
  }
}

void renderLeviosa() {
  // Floating purple wave.
  for (int i = 0; i < LED_COUNT; i++) {
    float phase =
        i * 0.19f + animationFrame * 0.16f;

    float wave =
        (sinf(phase) + 1.0f) * 0.5f;

    setPixel(
        i,
        clampByte(45 + wave * 85),
        clampByte(10 + wave * 35),
        clampByte(110 + wave * 145)
    );
  }
}

void renderOppugno() {
  // Fast red/purple chasing sparks.
  clearPixels();

  int position = (animationFrame * 5) % LED_COUNT;

  for (int k = 0; k < 5; k++) {
    int p = (position + k * 13) % LED_COUNT;

    setPixel(p, 255, 20, 100);

    setPixel(
        (p - 1 + LED_COUNT) % LED_COUNT,
        90, 0, 60
    );
  }
}

void updateAnimation() {
  if (capturing) return;
  if (activeSpell == SPELL_NONE) return;

  uint32_t now = millis();

  if (now - spellStartedAt >= SPELL_DURATION_MS) {
    activeSpell = SPELL_NONE;
    lightsOff();
    return;
  }

  if (lastAnimationAt != 0 &&
      now - lastAnimationAt < ANIMATION_INTERVAL_MS) {
    return;
  }

  lastAnimationAt = now;

  switch (activeSpell) {
    case SPELL_ASCENDIO:
      renderAscendio();
      break;

    case SPELL_CI:
      renderCI();
      break;

    case SPELL_INCENDIO:
      renderIncendio();
      break;

    case SPELL_LEVIOSA:
      renderLeviosa();
      break;

    case SPELL_OPPUGNO:
      renderOppugno();
      break;

    default:
      return;
  }

  publishPixels();
  animationFrame++;
}

// =====================================================
// Setup
// =====================================================

void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" SPELLBOUND v4");
  Serial.println(" Hold -> Capture");
  Serial.println(" Release -> Classify");
  Serial.println("==============================");

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  if (!mpu.begin()) {
    Serial.println("[IMU] MPU6050 NOT FOUND!");

    while (true) {
      delay(1000);
    }
  }

  Serial.println("[IMU] MPU6050 ready");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setBufferSize(512);
  mqtt.setKeepAlive(30);

  WiFi.mode(WIFI_STA);
  connectWiFiIfNeeded();

  randomSeed(micros());

  Serial.print("[AI] Required samples: ");
  Serial.println(REQUIRED_SAMPLES);

  Serial.print("[AI] Sample interval (ms): ");
  Serial.println(SAMPLE_INTERVAL_MS);

  Serial.println("[SYSTEM] READY");
}

// =====================================================
// Main loop
// =====================================================

void loop() {
  maintainNetwork();

  updateButton();

  if (capturing) {
    captureStep();
  } else {
    updateAnimation();
  }

  delay(1);
}
