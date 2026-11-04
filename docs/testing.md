# Testing and evidence

This page distinguishes the final adopted sketch, earlier candidate screenshots, the author's demonstration report and computer-based checks.

## Final adopted sketch

The author supplied `firmware/final/SPELLBOUND_Final.ino` as the final adopted version on 4 Nov 2026. It uses MAGIC, 100 samples at 50 Hz, a 0.65 recognition threshold, purple feedback for recognition, red retry feedback for low confidence, and turns the lights off for `ci` when `CI_TURNS_OFF` is true.

The author reports that their prototype demonstrations worked, including gesture recognition and the light effects. No run-by-run count was recorded. The retained serial screenshots include fallback-demo messages from the earlier candidate, so they do not independently verify the final adopted source.

The Oppugno excerpt at `evidence/serial_logs/2026-11-04_oppugno_AUTHOR_REPORTED.txt` records one prediction at 98.4% confidence. Its firmware version is not identified. A confidence value is not overall model accuracy.

## Final-version check sheet

Use this short list for one representative final-build run. It is not necessary to log every attempt.

| Check | Expected result | Record |
|---|---|---|
| Boot | MPU6050 detected; MAGIC project and six classes printed | Not yet linked to a final-build log |
| MQTT | Serial Monitor prints `[MQTT] Connected!` | Save a log excerpt if available |
| Recognised gesture | Class label and confidence print; purple cue; matching effect runs | Record the gesture and visible response |
| `ci` | All Vespera pixels turn off | Record the visible response |
| Low-confidence gesture | Red retry cue; no spell animation | Record if observed |
| Serial preview | `1`–`6` selects a light effect; `o` turns lights off | Useful backup demo; identify it as preview mode |

Example record: date/time · sketch file · model/project · gesture · serial label/confidence · MQTT connected? · visible Vespera response · evidence filename.

## Earlier visual-effects candidate

`firmware/candidate/SPELLBOUND_Visual_Effects.ino` is an earlier version. It has a countdown cue and plays demo keys 2, 3 and 5 after successive unrecognised gestures. The screenshots in `evidence/serial_logs/2026-11-04-serial-*.png` show those fallback messages. Some runs report MQTT disconnected, so those screenshots alone do not show the luminaire receiving the effects.

## Computer-based checks recorded during archive recovery

- `tests/host_syntax_check`: five syntax checks passed for older MKR and collector sketches; the corrupted collector copy failed as expected.
- `tests/host_logic_tests`: the older threshold-wand logic passed 10/10 checks.

These use simplified stand-in libraries. They are not Arduino builds or physical hardware tests, and they do not cover the final adopted sketch.

## Reproduction

1. Install the ESP32 core, Adafruit MPU6050, Adafruit Unified Sensor and PubSubClient.
2. Install the MAGIC Edge Impulse Arduino export. Do not install MAGIC2 alongside it for this sketch.
3. Copy `firmware/final/arduino_secrets.example.h` to `arduino_secrets.h` in the sketch folder and fill it in locally.
4. Upload `firmware/final/SPELLBOUND_Final.ino` to the ESP32 Feather and open Serial Monitor at 115200 baud.
5. Wait for the MQTT connected message, then press the button or send `g`. Use `1`–`6` for preview mode and `o` to turn the lights off.
6. For a final-build evidence record, capture one representative serial run and a short video or photo of the Vespera response. Keep preview tests separate from classifier tests.
