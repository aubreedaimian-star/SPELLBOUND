
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <MAGIC_inferencing.h>
#include <math.h>
#include <string.h>

// Adafruit HUZZAH32 ESP32 Feather
constexpr int SDA_PIN = 23;
constexpr int SCL_PIN = 22;

constexpr int SAMPLE_RATE = 50;
constexpr int SAMPLE_INTERVAL_US = 1000000 / SAMPLE_RATE;
constexpr int SAMPLE_COUNT = 100;
constexpr int AXES = 6;
constexpr float MIN_CONFIDENCE = 0.65f;

// IMPORTANT: Match the units used during model training.
// true = acceleration in g; false = acceleration in m/s^2
constexpr bool ACCEL_IN_G = true;

Adafruit_MPU6050 imu;
static float features[SAMPLE_COUNT * AXES];

// Edge Impulse reads from our captured data buffer.
int getSignalData(size_t offset, size_t length, float *out) {
  if (offset + length > SAMPLE_COUNT * AXES) return -1;
  memcpy(out, features + offset, length * sizeof(float));
  return 0;
}

void captureGesture() {
  Serial.println("CASTING STARTED - move the wand!");

  uint32_t nextSample = micros();

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    while ((int32_t)(micros() - nextSample) < 0) {
      delayMicroseconds(100);
    }
    nextSample += SAMPLE_INTERVAL_US;

    sensors_event_t a, g, temp;
    imu.getEvent(&a, &g, &temp);

    int n = i * AXES;

    float accelScale = ACCEL_IN_G ? 1.0f / 9.80665f : 1.0f;
    float gyroScale = 180.0f / PI;

    features[n + 0] = a.acceleration.x * accelScale;
    features[n + 1] = a.acceleration.y * accelScale;
    features[n + 2] = a.acceleration.z * accelScale;

    features[n + 3] = g.gyro.x * gyroScale;
    features[n + 4] = g.gyro.y * gyroScale;
    features[n + 5] = g.gyro.z * gyroScale;
  }

  Serial.println("CAPTURE COMPLETE");
}

void classifyGesture() {
  signal_t signal;
  signal.total_length = SAMPLE_COUNT * AXES;
  signal.get_data = getSignalData;

  ei_impulse_result_t result = { 0 };

  EI_IMPULSE_ERROR error = run_classifier(&signal, &result, false);

  if (error != EI_IMPULSE_OK) {
    Serial.print("Inference failed: ");
    Serial.println((int)error);
    return;
  }

  float bestScore = -1.0f;
  const char *bestLabel = nullptr;

  Serial.println("----- CLASSIFICATION -----");

  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    const char *label = result.classification[i].label;
    float score = result.classification[i].value;

    Serial.print(label);
    Serial.print(": ");
    Serial.println(score, 3);

    if (score > bestScore) {
      bestScore = score;
      bestLabel = label;
    }
  }

  if (bestLabel && bestScore >= MIN_CONFIDENCE) {
    Serial.print("SPELL DETECTED: ");
    Serial.print(bestLabel);
    Serial.print(" (");
    Serial.print(bestScore * 100.0f, 1);
    Serial.println("%)");
  } else {
    Serial.println("SPELL: UNKNOWN / LOW CONFIDENCE");
  }

  Serial.println("--------------------------");
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println("SPELLBOUND - GESTURE TEST");

  if (EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE != SAMPLE_COUNT * AXES) {
    Serial.println("ERROR: Model input size does not match 600 features.");
    while (true) delay(1000);
  }

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!imu.begin(0x68, &Wire)) {
    Serial.println("MPU6050 NOT FOUND - check wiring/address");
    while (true) delay(1000);
  }

  Serial.println("MPU6050 CONNECTED");
  Serial.println("Type g and press Enter to cast a spell.");
}

void loop() {
  if (Serial.available()) {
    char command = Serial.read();

    if (command == 'g' || command == 'G') {
      captureGesture();
      classifyGesture();
      Serial.println("READY FOR NEXT SPELL");
    }
  }
}
