# Known issues: network and lighting (v4)

Points 1–3 come from reading `v4.ino`. None has been tested, and none of them is confirmed as the cause of the Wi-Fi failure in the recorded run.

1. **v4 does not use `arduino_secrets.h`.** The file is included, but `WIFI_SSID` and `WIFI_PASSWORD` are set directly in the sketch (the password is `""`, so `WiFi.begin(WIFI_SSID)` is called without one), and MQTT connects without credentials (point 2). Copying `arduino_secrets.example.h` to `arduino_secrets.h` is therefore **not enough** to configure the current firmware. Sketch 08 shows the pattern that uses `SECRET_SSID` / `SECRET_PASS`. The firmware has not been changed.
2. **MQTT connects without a username or password.** v4 calls `mqtt.connect(clientId)`, while the earlier Vespera sketches pass `SECRET_MQTTUSER` / `SECRET_MQTTPASS`. If the broker requires login, MQTT will fail even once Wi-Fi works (PubSubClient state 5).
3. **No warning when frames are skipped.** `publishPixels()` returns silently when MQTT isn't connected, so the serial output can say `[SPELL] CAST` while nothing is sent. Printing a message there would make faults obvious.
4. **Lighting not checked.** A `publish()` returning true only means the client sent it. Vespera's response must be checked by eye or in the Lumi viewer.
5. **Sensor settings differ between sketches.** v4 uses ±8 g, ±500 °/s and a 21 Hz filter. The gesture tests (07, 09, 10) used ±2 g and ±250 °/s, and the MKR collector used the 44 Hz filter. Units are the same (m/s², °/s), but filter and range differences can change the model input. Which collector recorded the MAGIC2 data still needs confirming.
6. **Feedback flashes block.** The success and failure flashes wait about 0.5 s, and the button isn't read during that time.
7. `#include <arduino_secrets.h>` uses angle brackets rather than quotes (see 1).
