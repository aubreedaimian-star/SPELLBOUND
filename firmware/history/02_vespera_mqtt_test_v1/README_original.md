# SPELLBOUND – Vespera connectivity test

Goal: test Arduino MKR WiFi 1010 → Wi-Fi → MQTT → Lumi/Vespera before adding the IMU.

## Requirements
- Arduino MKR WiFi 1010, USB data cable, Arduino IDE
- Arduino SAMD Boards package
- Library Manager: `WiFiNINA` and `PubSubClient`
- Lab Wi-Fi and MQTT credentials. Do not commit passwords.

## Setup
1. Open `SPELLBOUND_Vespera_Test/SPELLBOUND_Vespera_Test.ino` using Arduino IDE.
2. In `arduino_secrets.h`, enter your local Wi-Fi and MQTT passwords (and username if course materials say otherwise).
3. Choose **Arduino MKR WiFi 1010**, correct COM port, upload.
4. Open Serial Monitor at **115200**.
5. Open https://www.iot.io/projects/lumi/ and select `student/CASA0014/luminaire/11`.
6. Watch colours cycle: gold → blue → off every ~3 seconds.

MQTT host: `mqtt.cetools.org`, port `1884`; binary payload: 72 pixels × RGB = 216 bytes. Publishes to `student/CASA0014/luminaire/11`.

## Notes
- Wi-Fi and MQTT passwords are NOT included.
- `arduino_secrets.h` is gitignored. Keep `arduino_secrets.example.h` as the public template.
- This version sends fixed test patterns. It **does not read IMU data**; that comes next.
- The code has not been compiled against a real MKR1010 in this environment. Verify in Arduino IDE.
- Confirm the assigned topic before publishing; this folder is tailored to Vespera ID 11.

Official tutorial: https://workshops.cetools.org/codelabs/CASA0014-L5-vespera/index.html
