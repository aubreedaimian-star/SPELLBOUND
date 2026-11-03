/*
  SPELLBOUND - Vespera MQTT connection test
  Board: Arduino MKR WiFi 1010
  Vespera topic: student/CASA0014/luminaire/11
  Sends 72 RGB pixels (216 bytes) every 3 seconds.
  This version does not use an IMU yet.
*/

#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"
#include <Wire.h>

const char WIFI_SSID[] = SECRET_SSID;  // [SANITISED] literal replaced by macro from arduino_secrets.h
const char WIFI_PASSWORD[] = SECRET_PASS;  // [SANITISED] literal replaced by macro from arduino_secrets.h
const char MQTT_USERNAME[] = SECRET_MQTTUSER;  // [SANITISED] literal replaced by macro from arduino_secrets.h
const char MQTT_PASSWORD[] = SECRET_MQTTPASS;  // [SANITISED] literal replaced by macro from arduino_secrets.h

// 2. Vespera
const char* MQTT_SERVER = "mqtt.cetools.org";
const int MQTT_PORT = 1884;

const char* MQTT_TOPIC =
  "student/CASA0014/luminaire/11";

const int LED_COUNT = 72;
byte rgbPayload[LED_COUNT * 3];

// 3. MPU6050
const byte MPU_ADDRESS = 0x68;

// Change these if the wand is too sensitive
const float GYRO_THRESHOLD = 60.0;
const float ACCEL_THRESHOLD = 0.35;

// Rainbow stays on for 2 seconds
const unsigned long HOLD_TIME = 2000;

// Animation speed
const unsigned long FRAME_INTERVAL = 300;

// Sensor sampling
const unsigned long SAMPLE_INTERVAL = 25;

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

unsigned long lastSample = 0;
unsigned long lastMovement = 0;
unsigned long lastFrame = 0;
unsigned long lastWiFiAttempt = 0;
unsigned long lastMQTTAttempt = 0;

bool movementSeen = false;
bool lightsOn = false;
bool forceFrame = true;
bool previousSampleValid = false;
bool imuReady = false;

float previousAX = 0;
float previousAY = 0;
float previousAZ = 0;

byte rainbowShift = 0;

// ====================================
// MPU6050 functions
// ====================================

bool writeMPU(byte reg, byte value) {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

int16_t read16() {
  uint16_t high = (uint8_t)Wire.read();
  uint16_t low = (uint8_t)Wire.read();
  return (int16_t)((high << 8) | low);
}

bool setupMPU() {
  Wire.beginTransmission(MPU_ADDRESS);

  if (Wire.endTransmission() != 0) {
    return false;
  }

  // Wake up MPU6050
  if (!writeMPU(0x6B, 0x00)) return false;

  // Accelerometer: +/- 2g
  if (!writeMPU(0x1C, 0x00)) return false;

  // Gyroscope: +/- 250 deg/s
  if (!writeMPU(0x1B, 0x00)) return false;

  // Low-pass filter
  if (!writeMPU(0x1A, 0x03)) return false;

  return true;
}

bool detectMovement() {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(0x3B);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom(
        MPU_ADDRESS, (byte)14) != 14) {
    return false;
  }

  int16_t rawAX = read16();
  int16_t rawAY = read16();
  int16_t rawAZ = read16();

  read16(); // Skip temperature

  int16_t rawGX = read16();
  int16_t rawGY = read16();
  int16_t rawGZ = read16();

  // Convert accelerometer to g
  float ax = rawAX / 16384.0f;
  float ay = rawAY / 16384.0f;
  float az = rawAZ / 16384.0f;

  // Gyro rotation speed in deg/s
  float gx = rawGX / 131.0f;
  float gy = rawGY / 131.0f;
  float gz = rawGZ / 131.0f;

  float gyroMotion = max(
    fabs(gx),
    max(fabs(gy), fabs(gz))
  );

  float accelMotion = 0;

  if (previousSampleValid) {
    accelMotion =
      fabs(ax - previousAX) +
      fabs(ay - previousAY) +
      fabs(az - previousAZ);
  }

  previousAX = ax;
  previousAY = ay;
  previousAZ = az;
  previousSampleValid = true;

  // A rotation OR sudden acceleration
  return gyroMotion > GYRO_THRESHOLD ||
         accelMotion > ACCEL_THRESHOLD;
}

// ====================================
// Rainbow functions
// ====================================

void colorWheel(byte pos,
                byte &r,
                byte &g,
                byte &b) {
  // Soft brightness for the demo
  const byte brightness = 110;

  if (pos < 85) {
    r = brightness - pos * brightness / 85;
    g = pos * brightness / 85;
    b = 0;
  } else if (pos < 170) {
    pos -= 85;
    r = 0;
    g = brightness - pos * brightness / 85;
    b = pos * brightness / 85;
  } else {
    pos -= 170;
    r = pos * brightness / 85;
    g = 0;
    b = brightness - pos * brightness / 85;
  }
}

void makeRainbow() {
  for (int i = 0; i < LED_COUNT; i++) {
    byte r, g, b;

    byte position =
      (i * 256 / LED_COUNT + rainbowShift) % 256;

    colorWheel(position, r, g, b);

    rgbPayload[i * 3] = r;
    rgbPayload[i * 3 + 1] = g;
    rgbPayload[i * 3 + 2] = b;
  }

  rainbowShift += 6;
}

void makeOff() {
  memset(rgbPayload, 0, sizeof(rgbPayload));
}

bool publishLights() {
  if (!mqtt.connected()) return false;

  bool success = mqtt.publish(
    MQTT_TOPIC,
    rgbPayload,
    sizeof(rgbPayload)
  );

  if (!success) {
    Serial.println("MQTT publish failed");
  }

  return success;
}

// ====================================
// WiFi and MQTT
// ====================================

void maintainConnection() {
  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastWiFiAttempt >= 10000) {
      lastWiFiAttempt = now;

      Serial.println("Connecting WiFi...");
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
    return;
  }

  if (!mqtt.connected()) {
    if (now - lastMQTTAttempt >= 5000) {
      lastMQTTAttempt = now;

      Serial.println("Connecting MQTT...");

      if (mqtt.connect(
            "SPELLBOUND_11",
            MQTT_USERNAME,
            MQTT_PASSWORD)) {
        Serial.println("MQTT connected!");
        forceFrame = true;
      } else {
        Serial.print("MQTT error: ");
        Serial.println(mqtt.state());
      }
    }
    return;
  }

  mqtt.loop();
}

// ====================================
// Setup
// ====================================

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println("SPELLBOUND STARTING");

  Wire.begin();
  imuReady = setupMPU();

  if (imuReady) {
    Serial.println("MPU6050 READY!");
  } else {
    Serial.println("MPU6050 NOT FOUND!");
  }

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setBufferSize(512);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWiFiAttempt = millis();
  lastMQTTAttempt = millis() - 5000UL;

  Serial.println("Connecting to Vespera...");
}

// ====================================
// Main loop
// ====================================

void loop() {
  maintainConnection();

  unsigned long now = millis();

  // Check the IMU every 25 ms
  if (imuReady &&
      now - lastSample >= SAMPLE_INTERVAL) {

    lastSample = now;

    if (detectMovement()) {
      if (!movementSeen ||
          now - lastMovement >= HOLD_TIME) {
        Serial.println("MAGIC DETECTED!");
      }

      movementSeen = true;
      lastMovement = now;
    }
  }

  bool active = movementSeen &&
                (now - lastMovement < HOLD_TIME);

  // Send rainbow frames while active
  if (active) {
    if (!lightsOn || forceFrame ||
        now - lastFrame >= FRAME_INTERVAL) {

      makeRainbow();

      if (publishLights()) {
        if (!lightsOn) {
          Serial.println("RAINBOW ON!");
        }

        lightsOn = true;
        forceFrame = false;
      }

      lastFrame = now;
    }
  } else {
    // Switch lights off once after activity
    if (lightsOn || forceFrame) {
      makeOff();

      if (publishLights()) {
        if (lightsOn) {
          Serial.println("LIGHTS OFF");
        }

        lightsOn = false;
        forceFrame = false;
      }
    }
  }
}