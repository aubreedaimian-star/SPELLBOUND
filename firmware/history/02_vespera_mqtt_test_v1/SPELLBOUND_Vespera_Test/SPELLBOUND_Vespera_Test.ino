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

const char WIFI_SSID[] = SECRET_SSID;
const char WIFI_PASSWORD[] = SECRET_PASS;
const char MQTT_USERNAME[] = SECRET_MQTTUSER;
const char MQTT_PASSWORD[] = SECRET_MQTTPASS;

const char MQTT_SERVER[] = "mqtt.cetools.org";
const uint16_t MQTT_PORT = 1884;
const char MQTT_TOPIC[] = "student/CASA0014/luminaire/11";

const size_t LED_COUNT = 72;
const size_t PAYLOAD_SIZE = LED_COUNT * 3;
uint8_t rgbPayload[PAYLOAD_SIZE] = {0};

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

uint32_t lastWiFiAttempt = 0;
uint32_t lastMQTTAttempt = 0;
uint32_t lastPublish = 0;
bool triedWiFi = false;
bool triedMQTT = false;
bool firstPublish = true;
uint8_t colourStep = 0;

void fillAllPixels(uint8_t red, uint8_t green, uint8_t blue) {
  for (size_t pixel = 0; pixel < LED_COUNT; pixel++) {
    rgbPayload[pixel * 3 + 0] = red;
    rgbPayload[pixel * 3 + 1] = green;
    rgbPayload[pixel * 3 + 2] = blue;
  }
}

void attemptWiFi() {
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void attemptMQTT() {
  String clientId = "spellbound-11-";
  clientId += String(millis(), HEX);  // unique-ish across reconnects
  Serial.print("Connecting to MQTT broker...");
  bool ok = mqtt.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD);
  if (ok) {
    Serial.println(" connected!");
    Serial.print("Publishing to topic: ");
    Serial.println(MQTT_TOPIC);
    firstPublish = true;
    colourStep = 0;
  } else {
    Serial.print(" failed. MQTT state: ");
    Serial.println(mqtt.state());
  }
}

void setup() {
  Serial.begin(115200);
  uint32_t serialStart = millis();
  while (!Serial && millis() - serialStart < 2500UL) {
    // Do not block forever when not connected to a computer.
  }

  Serial.println("\nSPELLBOUND: Vespera MQTT test");
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("ERROR: WiFiNINA module not found. Check board/firmware.");
  }

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  if (!mqtt.setBufferSize(512)) {
    Serial.println("ERROR: Cannot allocate MQTT buffer.");
  }
}

void loop() {
  const uint32_t now = millis();

  if (WiFi.status() != WL_CONNECTED) {
    if (!triedWiFi || now - lastWiFiAttempt >= 10000UL) {
      lastWiFiAttempt = now;
      triedWiFi = true;
      triedMQTT = false;
      attemptWiFi();
    }
    return;
  }

  if (!mqtt.connected()) {
    if (!triedMQTT || now - lastMQTTAttempt >= 5000UL) {
      lastMQTTAttempt = now;
      triedMQTT = true;
      attemptMQTT();
    }
    return;
  }

  mqtt.loop();  // keep MQTT session alive

  if (firstPublish || now - lastPublish >= 3000UL) {
    firstPublish = false;
    lastPublish = now;

    switch (colourStep) {
      case 0:
        fillAllPixels(60, 30, 5);
        Serial.print("GOLD: ");
        break;
      case 1:
        fillAllPixels(0, 10, 70);
        Serial.print("BLUE: ");
        break;
      default:
        fillAllPixels(0, 0, 0);
        Serial.print("OFF: ");
        break;
    }

    bool sent = mqtt.publish(MQTT_TOPIC, rgbPayload, PAYLOAD_SIZE);
    Serial.println(sent ? "published 216 bytes" : "publish failed");

    if (sent) {
      colourStep = (colourStep + 1) % 3;
    }
  }
}
