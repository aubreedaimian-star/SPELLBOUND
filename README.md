# SPELLBOUND

A gesture-controlled wand for the CASA0014 Vespera luminaire (UCL Connected Environments, 2026).

The wand uses an MPU6050 motion sensor on an ESP32 Feather. Pressing the button starts a two-second capture. An Edge Impulse model classifies the movement and the ESP32 sends RGB animation frames to Vespera #11 over MQTT.

## Final adopted source

The author supplied [`SPELLBOUND_Final.ino`](firmware/final/SPELLBOUND_Final.ino) as the final adopted sketch on 4 Nov 2026. It uses the MAGIC Edge Impulse export (`MAGIC_inferencing.h`) and acceleration in m/s². The retained serial screenshots show a different build: they include fallback-demo messages that do not exist in this final sketch. They are kept as development evidence, not presented as test logs from this exact file.

The author reports that their prototype demonstrations worked, including gesture recognition and the light effects. A full run-by-run count was not recorded. The recent Oppugno excerpt at [serial evidence](evidence/serial_logs/2026-11-04_oppugno_AUTHOR_REPORTED.txt) is one inference at 98.4% confidence, not a model accuracy result; its exact firmware build was not recorded.

## What the final sketch does

1. A button press captures 100 IMU samples at 50 Hz (two seconds).
2. The MAGIC model returns probabilities for six spell labels.
3. A recognised spell runs its matching light effect. With `CI_TURNS_OFF = true`, `ci` turns the lights off.
4. If recognition is below the 0.65 threshold, the code flashes the red retry cue; it does not play the 2, 3, 5 fallback demo used in the earlier candidate.
5. The ESP32 sends 72 RGB pixels (216 bytes) to Vespera #11 over MQTT. The final animation frame remains lit until the next cast or an explicit off command.

## Hardware and wiring

- ESP32 Feather (HUZZAH32)
- MPU6050 over I²C: SDA GPIO23, SCL GPIO22; sensor address 0x68
- Momentary button: GPIO27 to GND, using `INPUT_PULLUP`; one press triggers a cast
- Vespera #11 over MQTT: `mqtt.cetools.org:1884`, topic `student/CASA0014/luminaire/11`

## Build and run

1. Install the ESP32 Arduino core, Adafruit MPU6050, Adafruit Unified Sensor and PubSubClient libraries.
2. Install the Edge Impulse Arduino library exported for the MAGIC project. This sketch includes `MAGIC_inferencing.h`; keep only one Edge Impulse Arduino library installed so Arduino IDE cannot select a different SDK by mistake. The full library is not vendored here; see [models/README.md](models/README.md).
3. Create a local `arduino_secrets.h` with Wi-Fi and MQTT credentials. Keep the real secrets file out of Git.
4. Open the sketch in Arduino IDE, select the ESP32 Feather and the correct port, then open Serial Monitor at 115200 baud.
5. Wait for `[MQTT] Connected!`, then press the button once. Serial commands `g`, `1`–`6` and `o` are also available for casting, previewing and switching the lights off.

The final sketch sets `ACCEL_IN_G = false`, so the accelerometer values are passed in m/s². The source header records that this was checked against 227 MAGIC samples. Earlier MAGIC2 sketches are separate versions and use a different Edge Impulse export.

## Project documentation

- [Design process and system overview](docs/design-process.md)
- [Testing and evidence](docs/testing.md)
- [Known issues and setup notes](docs/known-issues.md)
- [Code inventory and provenance](docs/code-inventory.md)
- [Development log](docs/development-log.md)
- [AI contributions](docs/ai-contributions.md)
- [Edge Impulse training records](evidence/edge_impulse/README.md)
- [Development notes and serial evidence](evidence/development_notes/README.md)
- [Model package notes](models/README.md)

## Development history

| Date | Step |
|---|---|
| 12 Oct | Wi-Fi web page toggling the MKR WiFi 1010 LED |
| 19 Oct | Gesture ideas and model iterations |
| 26 Oct | MQTT colour test, motion threshold and MKR compile problems |
| 29 Oct | 50 Hz data collector and first Edge Impulse model |
| 1 Nov | Data collector moved to ESP32 Feather |
| 2 Nov | MAGIC2 model, gesture tests, MQTT test and integrated button sketches |
| 3 Nov | Visual-effects candidate with countdown cue and fallback demos |
| 4 Nov | Author supplied the final adopted MAGIC sketch |

The earlier visual-effects candidate and v4 are retained as development versions. See [development-log.md](docs/development-log.md).

## Contributions and AI use

The author reports doing the hardware setup, gesture-data collection, Edge Impulse training, integration and debugging. AI tools also contributed to code and documentation. The [AI contributions record](docs/ai-contributions.md) lists known assistance and items still awaiting confirmation.
