
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"

// SPELLBOUND - ESP32 to Vespera #11
// Fixed-colour MQTT communication test

const char* MQTT_HOST = "mqtt.cetools.org";
const uint16_t MQTT_PORT = 1884;

const char* MQTT_TOPIC =
    "student/CASA0014/luminaire/11";

constexpr size_t LED_COUNT = 72;
constexpr size_t PAYLOAD_SIZE = LED_COUNT * 3;

uint8_t rgbPayload[PAYLOAD_SIZE];

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

unsigned long lastWifiAttempt = 0;
unsigned long lastMqttAttempt = 0;

void fillColour(uint8_t r, uint8_t g, uint8_t b) {
  for (size_t i = 0; i < LED_COUNT; i++) {
    rgbPayload[i * 3] = r;
    rgbPayload[i * 3 + 1] = g;
    rgbPayload[i * 3 + 2] = b;
  }
}

void connectNetwork() {
  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastWifiAttempt >= 10000UL) {
      lastWifiAttempt = now;
      Serial.println("[WiFi] Connecting...");
      WiFi.disconnect();
      WiFi.begin(SECRET_SSID, SECRET_PASS);
    }
    return;
  }

  if (mqtt.connected()) return;

  if (now - lastMqttAttempt >= 5000UL) {
    lastMqttAttempt = now;

    String clientId = "SPELLBOUND-11-";
    clientId += WiFi.macAddress();
    clientId.replace(":", "");

    Serial.println("[MQTT] Connecting...");

    if (mqtt.connect(
          clientId.c_str(),
          SECRET_MQTTUSER,
          SECRET_MQTTPASS
        )) {
      Serial.println("[MQTT] Connected!");
      Serial.print("[MQTT] Topic: ");
      Serial.println(MQTT_TOPIC);
    } else {
      Serial.print("[MQTT] Failed, state=");
      Serial.println(mqtt.state());
    }
  }
}

void sendColour(
  uint8_t r,
  uint8_t g,
  uint8_t b,
  const char* name
) {
  if (!mqtt.connected()) {
    Serial.println("[ERROR] MQTT not connected.");
    return;
  }

  fillColour(r, g, b);

  bool success = mqtt.publish(
    MQTT_TOPIC,
    rgbPayload,
    sizeof(rgbPayload)
  );

  Serial.print("[SPELLBOUND] ");
  Serial.println(name);

  Serial.print("[MQTT] Payload: ");
  Serial.print(sizeof(rgbPayload));
  Serial.println(" bytes");

  Serial.println(
    success
      ? "[MQTT] Publish accepted by client"
      : "[MQTT] Publish failed"
  );
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("SPELLBOUND - VESPERA #11");
  Serial.println("ESP32 MQTT Colour Test");

  WiFi.mode(WIFI_STA);

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(512);

  lastWifiAttempt = millis() - 10000UL;
  lastMqttAttempt = millis() - 5000UL;

  Serial.println("Commands:");
  Serial.println("r = RED");
  Serial.println("g = GREEN");
  Serial.println("b = BLUE");
  Serial.println("o = OFF");
}

void loop() {
  connectNetwork();

  if (mqtt.connected()) {
    mqtt.loop();
  }

  if (Serial.available()) {
    char command = Serial.read();

    switch (command) {
      case 'r':
      case 'R':
        sendColour(255, 0, 0, "RED");
        break;

      case 'g':
      case 'G':
        sendColour(0, 255, 0, "GREEN");
        break;

      case 'b':
      case 'B':
        sendColour(0, 0, 255, "BLUE");
        break;

      case 'o':
      case 'O':
        sendColour(0, 0, 0, "OFF");
        break;
    }
  }

  delay(10);
}
