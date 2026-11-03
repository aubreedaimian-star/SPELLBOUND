
#include <Arduino.h>
#include <Wire.h>

// SPELLBOUND - Edge Impulse Data Forwarder
// Board: Adafruit ESP32 Feather (HUZZAH32)
// Sensor: GY-521 MPU6050

constexpr uint8_t MPU_ADDR = 0x68;
constexpr int SDA_PIN = 23;
constexpr int SCL_PIN = 22;

constexpr uint32_t BAUD_RATE = 115200;
constexpr uint32_t SAMPLE_PERIOD_US = 20000; // 50 Hz

uint32_t nextSampleUs = 0;

bool writeMPU(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readMPU(float values[6]) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // ACCEL_XOUT_H

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom(MPU_ADDR, (uint8_t)14, (uint8_t)true) != 14) {
    return false;
  }

  int16_t raw[7];

  for (int i = 0; i < 7; i++) {
    raw[i] = (int16_t)((Wire.read() << 8) | Wire.read());
  }

  // MPU6050 configuration:
  // Acceleration: +/-2g
  // Gyroscope: +/-250 degrees/second

  values[0] = raw[0] / 16384.0f; // accX (g)
  values[1] = raw[1] / 16384.0f; // accY (g)
  values[2] = raw[2] / 16384.0f; // accZ (g)

  values[3] = raw[4] / 131.0f;   // gyroX (deg/s)
  values[4] = raw[5] / 131.0f;   // gyroY (deg/s)
  values[5] = raw[6] / 131.0f;   // gyroZ (deg/s)

  return true;
}

void setup() {
  Serial.begin(BAUD_RATE);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  delay(100);

  // Wake MPU6050 and select X-axis gyro clock
  if (!writeMPU(0x6B, 0x01)) {
    // No text output: Data Forwarder expects CSV only
    while (true) {
      delay(1000);
    }
  }

  // Explicitly configure sensor ranges
  writeMPU(0x1C, 0x00); // +/-2g
  writeMPU(0x1B, 0x00); // +/-250 deg/s

  delay(200);
  nextSampleUs = micros();
}

void loop() {
  uint32_t now = micros();

  if ((int32_t)(now - nextSampleUs) < 0) {
    return;
  }

  nextSampleUs += SAMPLE_PERIOD_US;

  // Recover if sampling falls significantly behind
  if ((int32_t)(micros() - nextSampleUs) > (int32_t)SAMPLE_PERIOD_US) {
    nextSampleUs = micros() + SAMPLE_PERIOD_US;
  }

  float values[6];

  if (!readMPU(values)) {
    return; // Skip failed reads; never print invalid data
  }

  for (int i = 0; i < 6; i++) {
    Serial.print(values[i], 5);
    Serial.print(i == 5 ? '\n' : ',');
  }
}
