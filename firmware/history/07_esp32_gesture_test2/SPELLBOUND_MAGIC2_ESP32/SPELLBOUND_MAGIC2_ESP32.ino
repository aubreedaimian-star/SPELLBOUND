
/*
 * SPELLBOUND - ESP32 Gesture Recognition V1
 *
 * Board: Adafruit HUZZAH32 ESP32 Feather
 * Sensor: MPU6050 (GY-521)
 * Model: MAGIC2_inferencing (Edge Impulse, project MAGIC2 v3)
 *
 * Input: 6-axis IMU, 50 Hz, 2-second window
 * Output: Recognised spell in Serial Monitor
 *
 * Serial command:
 *   g = Capture and classify one gesture
 *
 * NOTE:
 * Acceleration uses m/s^2, based on the earlier
 * MKR1010 training data evidence.
 * WARNING: Check acceleration and gyroscope units against your actual MAGIC2 training data.
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <MAGIC2_inferencing.h>

#include <math.h>
#include <string.h>

// ==================================================
// 1. HARDWARE CONFIGURATION
// ==================================================

constexpr int SDA_PIN = 23;
constexpr int SCL_PIN = 22;

constexpr uint8_t MPU_ADDRESS = 0x68;

constexpr uint32_t SERIAL_BAUD = 115200;

// ==================================================
// 2. SAMPLING CONFIGURATION
// ==================================================

constexpr int SAMPLE_RATE = 50;
constexpr int SAMPLE_COUNT = 100;
constexpr int AXES = 6;

constexpr uint32_t SAMPLE_INTERVAL_US =
    1000000UL / SAMPLE_RATE;

constexpr size_t FEATURE_COUNT =
    SAMPLE_COUNT * AXES;

// Edge Impulse input format:
// accX, accY, accZ, gyroX, gyroY, gyroZ

// true  = acceleration in g
// false = acceleration in m/s^2
constexpr bool ACCEL_IN_G = false;

// Gyroscope input uses degrees per second.
constexpr bool GYRO_IN_DEG_PER_SEC = true;

// ==================================================
// 3. CLASSIFICATION CONFIGURATION
// ==================================================

constexpr float MIN_CONFIDENCE = 0.65f;

// Fail compilation if a different Edge Impulse project is included.
#if EI_CLASSIFIER_PROJECT_ID != 1153638
#error "Wrong Edge Impulse model! Install MAGIC2_inferencing (project ID 1153638)."
#endif

// Model must match the feature dimensions.
static_assert(FEATURE_COUNT == 600,
              "Expected 600 input features");
static_assert(EI_CLASSIFIER_RAW_SAMPLE_COUNT == SAMPLE_COUNT,
              "MAGIC2 sample count mismatch");
static_assert(EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME == AXES,
              "MAGIC2 number of axes mismatch");
static_assert(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE == FEATURE_COUNT,
              "MAGIC2 model frame size mismatch");
static_assert(EI_CLASSIFIER_FREQUENCY == SAMPLE_RATE,
              "MAGIC2 sample frequency mismatch");

// ==================================================
// 4. GLOBAL OBJECTS
// ==================================================

Adafruit_MPU6050 imu;

static float features[FEATURE_COUNT];

bool imuReady = false;

// ==================================================
// 5. EDGE IMPULSE DATA CALLBACK
// ==================================================

int getSignalData(
    size_t offset,
    size_t length,
    float *out
) {
    if (offset > FEATURE_COUNT ||
        length > FEATURE_COUNT - offset) {
        return -1;
    }

    memcpy(
        out,
        features + offset,
        length * sizeof(float)
    );

    return 0;
}

// ==================================================
// 6. INITIALISE MPU6050
// ==================================================

bool initialiseIMU() {

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);

    Serial.println("[IMU] Initialising MPU6050...");

    if (!imu.begin(MPU_ADDRESS, &Wire)) {
        Serial.println(
            "[ERROR] MPU6050 not detected."
        );

        Serial.println(
            "Check power, SDA, SCL and address."
        );

        return false;
    }

    imu.setAccelerometerRange(
        MPU6050_RANGE_2_G
    );

    imu.setGyroRange(
        MPU6050_RANGE_250_DEG
    );

    imu.setFilterBandwidth(
        MPU6050_BAND_21_HZ
    );

    Serial.println("[IMU] MPU6050 connected.");
    Serial.println("[IMU] Accelerometer: +/-2g");
    Serial.println("[IMU] Gyroscope: +/-250 deg/s");
    Serial.println("[IMU] Filter: 21 Hz");

    return true;
}

// ==================================================
// 7. CAPTURE GESTURE
// ==================================================

bool captureGesture() {

    if (!imuReady) {
        Serial.println("[ERROR] IMU not ready.");
        return false;
    }

    Serial.println();
    Serial.println("==========================");
    Serial.println(" SPELLBOUND - CASTING");
    Serial.println("==========================");

    Serial.println(
        "[CAPTURE] Move the wand now!"
    );

    uint32_t nextSample = micros();

    for (int i = 0; i < SAMPLE_COUNT; i++) {

        // Maintain 50 Hz sampling.
        while (
            (int32_t)(micros() - nextSample) < 0
        ) {
            delayMicroseconds(100);
        }

        nextSample += SAMPLE_INTERVAL_US;

        sensors_event_t a;
        sensors_event_t g;
        sensors_event_t temp;

        if (!imu.getEvent(&a, &g, &temp)) {
            Serial.println(
                "[ERROR] Sensor read failed."
            );

            return false;
        }

        const size_t n = i * AXES;

        const float accelScale =
            ACCEL_IN_G
                ? (1.0f / 9.80665f)
                : 1.0f;

        const float gyroScale =
            GYRO_IN_DEG_PER_SEC
                ? (180.0f / PI)
                : 1.0f;

        features[n + 0] =
            a.acceleration.x * accelScale;

        features[n + 1] =
            a.acceleration.y * accelScale;

        features[n + 2] =
            a.acceleration.z * accelScale;

        features[n + 3] =
            g.gyro.x * gyroScale;

        features[n + 4] =
            g.gyro.y * gyroScale;

        features[n + 5] =
            g.gyro.z * gyroScale;
    }

    Serial.println(
        "[CAPTURE] 100 samples collected."
    );

    Serial.println(
        "[CAPTURE] 600 features ready."
    );

    return true;
}

// ==================================================
// 8. MAP MODEL LABELS TO SPELL NAMES
// ==================================================

const char* getSpellName(const char* label) {

    // These are the six actual labels exported by MAGIC2 v3.
    if (strcmp(label, "Ascendio") == 0) return "ASCENDIO";
    if (strcmp(label, "ci") == 0) return "CI";
    if (strcmp(label, "incendio") == 0) return "INCENDIO";
    if (strcmp(label, "leviosa") == 0) return "LEVIOSA";
    if (strcmp(label, "oppugno") == 0) return "OPPUGNO";
    if (strcmp(label, "stupefy") == 0) return "STUPEFY";

    // Keep any unexpected label visible for debugging.
    return label;
}

// ==================================================
// 9. CLASSIFY GESTURE
// ==================================================

void classifyGesture() {

    signal_t signal;

    signal.total_length = FEATURE_COUNT;
    signal.get_data = getSignalData;

    ei_impulse_result_t result = {0};

    Serial.println(
        "[AI] Running Edge Impulse..."
    );

    EI_IMPULSE_ERROR error =
        run_classifier(
            &signal,
            &result,
            false
        );

    if (error != EI_IMPULSE_OK) {

        Serial.print(
            "[ERROR] Inference failed: "
        );

        Serial.println((int)error);

        return;
    }

    float bestScore = -1.0f;
    const char* bestLabel = nullptr;

    Serial.println();
    Serial.println(
        "------ MODEL RESULTS ------"
    );

    for (
        size_t i = 0;
        i < EI_CLASSIFIER_LABEL_COUNT;
        i++
    ) {

        const char* label =
            result.classification[i].label;

        float score =
            result.classification[i].value;

        Serial.print(label);
        Serial.print(" : ");
        Serial.print(score * 100.0f, 1);
        Serial.println("%");

        if (score > bestScore) {
            bestScore = score;
            bestLabel = label;
        }
    }

    Serial.println(
        "---------------------------"
    );

    Serial.print("[AI] DSP time: ");
    Serial.print(result.timing.dsp);
    Serial.println(" ms");

    Serial.print("[AI] Inference time: ");
    Serial.print(result.timing.classification);
    Serial.println(" ms");

    Serial.println();

    if (
        bestLabel != nullptr &&
        bestScore >= MIN_CONFIDENCE
    ) {

        const char* spell =
            getSpellName(bestLabel);

        Serial.println(
            "***************************"
        );

        Serial.print(" SPELL DETECTED: ");
        Serial.println(spell);

        Serial.print(" MODEL LABEL: ");
        Serial.println(bestLabel);

        Serial.print(" CONFIDENCE: ");
        Serial.print(bestScore * 100.0f, 1);
        Serial.println("%");

        Serial.println(
            "***************************"
        );

    } else {

        Serial.println(
            "SPELL UNKNOWN"
        );

        Serial.println(
            "Confidence below threshold."
        );

        Serial.print("Best score: ");
        Serial.print(bestScore * 100.0f, 1);
        Serial.println("%");
    }
}

// ==================================================
// 10. SETUP
// ==================================================

void setup() {

    Serial.begin(SERIAL_BAUD);

    delay(1500);

    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.println(
        "     SPELLBOUND - ESP32 V1"
    );

    Serial.println(
        "     Gesture Recognition"
    );

    Serial.println(
        "================================"
    );

    Serial.print("[MODEL] Project: ");
    Serial.println(EI_CLASSIFIER_PROJECT_NAME);
    Serial.print("[MODEL] Deployment version: ");
    Serial.println(EI_CLASSIFIER_PROJECT_DEPLOY_VERSION);
    Serial.print("[MODEL] Frequency: ");
    Serial.print(EI_CLASSIFIER_FREQUENCY);
    Serial.println(" Hz");

    // Validate model input size.

    if (
        EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE
        != FEATURE_COUNT
    ) {

        Serial.println(
            "[ERROR] Model input mismatch."
        );

        Serial.print("Expected: ");
        Serial.println(FEATURE_COUNT);

        Serial.print("Model requires: ");
        Serial.println(
            EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE
        );

        while (true) {
            delay(1000);
        }
    }

    Serial.println(
        "[MODEL] Input size verified."
    );

    Serial.print(
        "[MODEL] Number of classes: "
    );

    Serial.println(
        EI_CLASSIFIER_LABEL_COUNT
    );

    imuReady = initialiseIMU();

    if (!imuReady) {

        Serial.println(
            "[FATAL] Cannot start gesture test."
        );

        while (true) {
            delay(1000);
        }
    }

    Serial.println();
    Serial.println(
        "[SYSTEM] SPELLBOUND READY"
    );

    Serial.println(
        "Type 'g' and press Enter to cast."
    );
}

// ==================================================
// 11. MAIN LOOP
// ==================================================

void loop() {

    if (Serial.available() > 0) {

        char command = Serial.read();

        if (
            command == 'g' ||
            command == 'G'
        ) {

            bool captured =
                captureGesture();

            if (captured) {
                classifyGesture();
            }

            Serial.println();
            Serial.println(
                "[SYSTEM] Ready for next spell."
            );

            Serial.println(
                "Type 'g' to cast again."
            );
        }
    }
}
