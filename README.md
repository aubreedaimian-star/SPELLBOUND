# SPELLBOUND

A gesture-controlled wand for the CASA0014 Vespera luminaire (UCL Connected Environments, 2026).

The wand uses an MPU6050 motion sensor on an ESP32 Feather. A button starts a two-second movement capture. On release, an Edge Impulse model classifies the movement as one of six spells; the ESP32 then sends RGB animation frames to Vespera #11 over MQTT.

## Demonstration status

The author reports that the physical prototype demonstrations worked, including gesture recognition and the light effects. A full run-by-run count was not recorded, so this is a project report rather than a measured success rate.

One author-supplied serial excerpt records an Oppugno prediction at 98.4% confidence. It is one inference and must not be read as overall model accuracy: [serial excerpt](evidence/serial_logs/2026-11-04_oppugno_AUTHOR_REPORTED.txt).

The repository contains the earlier integrated sketch at `firmware/latest/v4/v4.ino` and a later visual-effects candidate at `firmware/candidate/SPELLBOUND_Visual_Effects.ino`. The candidate was copied from the author's latest saved Library sketch (3 Nov); the author has not confirmed that this exact file is the version flashed for the reported demo. It includes `MAGIC_inferencing.h`; use the header and model export that match the trained Edge Impulse project before compiling.

## How the system works

1. The button starts one capture; the user performs a gesture while the IMU records 100 samples at 50 Hz.
2. The Edge Impulse model returns class probabilities for six labels.
3. A recognised spell selects its matching animation. If recognition fails, the visual-effects candidate advances through demo keys 2, 3 and 5 on successive failures.
4. The ESP32 publishes 72 RGB pixels (216 bytes) to the Vespera MQTT topic. The final animation frame stays lit until the next cast or an explicit off command.

The installation layout assumed by the candidate is 12 strips of 6 LEDs, with pixel 0 at the top and pixel 5 at the bottom in each strip. Confirm the physical layout before relying on height-dependent effects.

## Hardware and wiring

- ESP32 Feather (HUZZAH32)
- MPU6050 over I²C: SDA GPIO23, SCL GPIO22
- Latching switch/button: GPIO27 to GND, configured with `INPUT_PULLUP`
- Vespera #11 over MQTT: `mqtt.cetools.org:1884`, topic `student/CASA0014/luminaire/11`

## Build and run

1. Install the ESP32 Arduino core, Adafruit MPU6050, Adafruit Unified Sensor and PubSubClient libraries.
2. Install the Edge Impulse Arduino library exported for the model used to collect/train the gestures. The model package is not fully vendored here; see [models/README.md](models/README.md).
3. Configure local Wi-Fi and MQTT credentials in `arduino_secrets.h` using the example header in the relevant sketch folder. Keep the real secrets file out of Git.
4. Open the sketch in Arduino IDE and select the matching board and serial port. Open Serial Monitor at 115200 baud.
5. Confirm the serial output reports MQTT connected before expecting Vespera to show an effect. Switch OFF then ON for a cast; the candidate also accepts serial preview commands 1–6 and off command `o`.

Exact library names, model header, board selection and luminaire access must match the local setup. The candidate's header currently says `MAGIC_inferencing.h`, while earlier project records refer to MAGIC2; select the correct export deliberately rather than renaming headers blindly.

## Project documentation

- [Design process and system overview](docs/design-process.md)
- [Testing and evidence](docs/testing.md)
- [Known issues and setup notes](docs/known-issues.md)
- [Code inventory and provenance](docs/code-inventory.md)
- [Development log](docs/development-log.md)
- [AI contributions](docs/ai-contributions.md)
- [Edge Impulse training records](evidence/edge_impulse/README.md)
- [Model package notes](models/README.md)

## Development history

| Date | Step |
|---|---|
| 12 Oct | Wi-Fi web page toggling the MKR WiFi 1010 LED |
| 26 Oct | MQTT colour test to Vespera; motion-threshold wand |
| 29 Oct | 50 Hz data collector and first Edge Impulse model; library compile issue on MKR1010 |
| 1 Nov | Data collector ported to ESP32 Feather |
| 2 Nov | MAGIC2 model; serial-triggered gesture tests; ESP32 to Vespera test; integrated and button sketches |
| 3 Nov | Visual-effects candidate with capture cue, spell animations and fallback sequence |

Dates follow the available files and records; see the development log for more context.

## Contributions and AI use

The author reports doing the hardware setup, gesture-data collection, Edge Impulse training, integration and debugging. AI tools also contributed to code and documentation. The [AI contributions record](docs/ai-contributions.md) describes known assistance and uncertain authorship. Historical sketches are retained as development records; comments and style are not used to infer who wrote them.
