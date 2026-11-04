# Testing and evidence

This page separates the author's demonstration report, the serial evidence that was retained, and checks performed on a computer. The repository does not contain a full run-by-run test tally.

## Physical demonstration

The author reports that their prototype demonstrations succeeded, including gesture recognition and the Vespera light effects. They did not record every run or count the total number of attempts.

The only recent serial excerpt supplied is [one Oppugno result](../evidence/serial_logs/2026-11-04_oppugno_AUTHOR_REPORTED.txt): the classifier printed 98.4% for Oppugno, followed by a recognised-colour message and the spell name. The excerpt ends at a Wi-Fi connection message. It is a representative inference, not a model accuracy result and not a complete log of the light response.

The exact firmware version used for that reported demonstration has not been confirmed. The saved visual-effects sketch is therefore included as a candidate, not labelled as the verified flashed build.

## What the candidate sketch is intended to do

`firmware/candidate/SPELLBOUND_Visual_Effects.ino` contains:

- 100 IMU samples at 50 Hz during a two-second capture;
- Edge Impulse classification over six spell labels;
- MQTT publishing of 72 RGB pixels (216 bytes) to Vespera #11;
- a capture cue, spell animations and a held final frame;
- fallback previews 2, 3 and 5 in order across successive unrecognised gestures.

These are code features. Their presence in the file does not independently verify that this exact sketch was uploaded or tested.

## Earlier recorded run

The repository also retains an author-reported v4 Ascendio result from 2 Nov in `evidence/serial_logs/2026-11-02_v4_ascendio_AUTHOR_REPORTED.txt`. It records one classification at 0.9922 confidence. It does not establish model accuracy or repeatability.

## Computer-based checks recorded during archive recovery

- `tests/host_syntax_check`: five syntax checks passed for older MKR and collector sketches; the corrupted collector copy failed as expected.
- `tests/host_logic_tests`: the older threshold-wand logic passed 10/10 checks.

These checks use simplified stand-in libraries. They are not Arduino builds or physical hardware tests, and they do not cover the visual-effects candidate.

## Reproduction notes

1. Install the board core and libraries listed in the main README.
2. Install the matching Edge Impulse Arduino export and select the matching inference header.
3. Create a local `arduino_secrets.h` from the example, then enter local Wi-Fi and MQTT credentials. Do not commit the real secrets file.
4. Upload the sketch to the ESP32 Feather and open Serial Monitor at 115200 baud.
5. Wait for the MQTT connected message, then switch OFF and ON to trigger a cast. Compare the serial classification with the corresponding Vespera response.
6. If recording future evidence, save one complete representative serial capture and a short video of the luminaire response. A small number of representative records is enough; the project does not require logging every attempt.

For a useful performance estimate in a future iteration, record a fixed number of attempts per gesture and report the denominator alongside correct recognitions. Do not infer an accuracy percentage from confidence scores.
