# Testing and evidence

This page separates the final adopted source, earlier candidate screenshots, and the author's demonstration report.

## Final adopted source

The author supplied `firmware/final/SPELLBOUND_Final.ino` as the final adopted sketch on 4 Nov 2026. It uses MAGIC, a two-second capture at 50 Hz, a 0.65 recognition threshold, a red retry cue on low confidence, and turns the lights off for `ci` when `CI_TURNS_OFF` is true. It has no 2, 3, 5 fallback sequence.

The author reports that their prototype demonstrations worked, including gesture recognition and the light effects. No run-by-run count was recorded. The retained screenshots do not identify this final source as the flashed build; their serial text includes the earlier candidate's fallback messages. This means the screenshots document development, but do not independently verify the final sketch.

The supplied Oppugno excerpt at `evidence/serial_logs/2026-11-04_oppugno_AUTHOR_REPORTED.txt` shows one 98.4% prediction. The exact firmware version is not identified, and a confidence score is not overall model accuracy.

## Earlier visual-effects candidate

`firmware/candidate/SPELLBOUND_Visual_Effects.ino` is an earlier version. It has a countdown cue and plays demo keys 2, 3 and 5 after successive unrecognised gestures. The screenshots in `evidence/serial_logs/2026-11-04-serial-*.png` show its fallback messages. Some visible runs report MQTT disconnected, so those screenshots alone do not prove that Vespera received the effects.

## Computer-based checks recorded during archive recovery

- `tests/host_syntax_check`: five syntax checks passed for older MKR and collector sketches; the corrupted collector copy failed as expected.
- `tests/host_logic_tests`: the older threshold-wand logic passed 10/10 checks.

These use simplified stand-in libraries. They are not Arduino builds or physical hardware tests, and they do not cover the final adopted sketch.

## Reproduction

1. Install the ESP32 core, Adafruit MPU6050, Adafruit Unified Sensor and PubSubClient.
2. Install the MAGIC Edge Impulse Arduino export. Do not install MAGIC2 alongside it for this sketch.
3. Create a local `arduino_secrets.h` from the example and add the Wi-Fi/MQTT credentials.
4. Upload `firmware/final/SPELLBOUND_Final.ino` to the ESP32 Feather and open Serial Monitor at 115200 baud.
5. Wait for the MQTT connected message, then press the button or send `g`. Use `1`–`6` to preview effects and `o` to turn lights off.
6. To evidence this exact final build, save one representative complete serial capture and a short video of Vespera responding. Do not use the earlier fallback screenshots as evidence of behavior in this final sketch.
