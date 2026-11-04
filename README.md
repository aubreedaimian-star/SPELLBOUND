# SPELLBOUND

A gesture-controlled wand for the CASA0014 Vespera luminaire (UCL Connected Environments, 2026).

The wand uses an MPU6050 motion sensor on an ESP32 Feather. In the latest sketch, holding a button records a 2-second movement. On release, an Edge Impulse model classifies it as one of six spells, and the sketch is written to send a matching light animation to Vespera #11 over MQTT.

## Where it stands (3 Nov 2026)
- **Latest sketch:** `firmware/latest/v4/v4.ino`
- **Observed on hardware:** one author-reported on-device classification of *Ascendio* with a confidence score of 0.9922 (`evidence/serial_logs/`). This is one inference. It is not a measure of model accuracy and does not show MQTT delivery or a lighting response.
- **Network:** Wi-Fi failed during that recorded run, so MQTT delivery and the lighting response have not been tested.
- **Written but not yet tested on hardware:** MQTT publishing, the spell animations, and the feedback flashes.

## Project notes
- [Testing status and evidence](docs/testing.md) separates the single reported classification from features not verified on hardware.
- [Known issues](docs/known-issues.md) lists the current Wi-Fi and MQTT configuration gaps.
- [Code inventory](docs/code-inventory.md) records the source files and their provenance.
- [AI contributions](docs/ai-contributions.md) identifies AI-assisted code and documentation. AI-generated long-form analyses and an unused test sketch were removed from the current review tree; their use remains disclosed, and the initial Git commit preserves the earlier files.

## Development history
| Date | Step |
|---|---|
| 12 Oct | Wi-Fi web page toggling the MKR WiFi 1010's LED |
| 26 Oct | MQTT colour test to Vespera; then a motion-threshold version with the MPU6050 |
| 29 Oct | 50 Hz data collector for Edge Impulse; first model (MAGIC); the library would not compile for the MKR1010 until a CMSIS header was patched |
| 1 Nov | Data collector ported to the ESP32 Feather |
| 2 Nov | MAGIC2 model; serial-triggered gesture tests; ESP32 → Vespera colour test; integrated and button versions; v4 |

Dates are the files' last-saved times. Full notes are in `docs/development-log.md`.

## Hardware
ESP32 Feather (HUZZAH32) · MPU6050 on I²C (SDA 23, SCL 22) · push button on GPIO27 to GND · Vespera #11 via `mqtt.cetools.org:1884`, topic `student/CASA0014/luminaire/11` (72 LEDs × RGB = 216 bytes).

## Running v4
1. Use the Arduino IDE with the ESP32 core, plus the Adafruit MPU6050, Adafruit Unified Sensor and PubSubClient libraries and the `MAGIC2_inferencing` export. The full Edge Impulse library is **not included** in this repository; see `models/README.md`.
2. Open Serial Monitor at 115200. Hold the button, make the gesture, then release.

**Network configuration is incomplete.** v4 includes `arduino_secrets.h` but does not use it: the Wi-Fi settings are written directly in the sketch (with an empty password), and MQTT connects without credentials. Copying `arduino_secrets.example.h` is therefore not enough to connect. See `docs/known-issues.md`. Never commit a real `arduino_secrets.h`; it is gitignored.

## Folders
`firmware/latest` · `firmware/history` (earlier sketches, in order) · `experiments` (MKR1010 compile problem) · `models` · `tests` · `evidence` · `docs`

## Who did what
I did the practical work: hardware setup, IMU gesture data collection, Edge Impulse training, integration and debugging. AI tools also contributed. The v4 sketch was developed with ChatGPT assistance; a Codex agent diagnosed the MKR1010 compile problem; Claude helped recover and document this archive. The details, including sketches whose authorship is still being confirmed and AI-generated material removed from the current review tree, are in `docs/ai-contributions.md`.
