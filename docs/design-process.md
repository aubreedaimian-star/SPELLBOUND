# Design process

This note connects the project idea to the prototypes recorded in the repository. It describes the development choices and current limitations without treating saved drafts as proof of hardware testing.

## Concept

SPELLBOUND turns a hand movement into a named spell and a light response. The interaction takes the familiar idea of a magic wand causing a visible effect in a themed environment and makes the gesture itself the input. The author has described theme-park interactive wands as an inspiration.

The physical interaction uses a button as a deliberate “cast” action. Holding a wand and moving it without a trigger could otherwise cause accidental recognition; a press-and-release capture gives the user a clear start point.

## System design

| Part | Role |
|---|---|
| ESP32 Feather | Reads the button and IMU, runs the exported classifier, and publishes light frames |
| MPU6050 | Supplies three-axis acceleration and three-axis gyroscope readings |
| Edge Impulse model | Maps a two-second motion window to one of six spell labels |
| MQTT | Carries RGB frame data from the ESP32 to the luminaire |
| Vespera #11 | Shows the capture cue, selected animation, and final held frame |

The final adopted sketch follows this path:

`button press → 100 IMU samples at 50 Hz → MAGIC classifier → spell effect or ci off → MQTT RGB frames → Vespera`

The earlier visual-effects candidate used the 2, 3, 5 fallback preview. The final adopted sketch instead gives a red retry cue when confidence is low. The retained serial screenshots show the earlier candidate, so they are labelled separately in the evidence index.

The code assumes a 72-pixel luminaire and 216-byte RGB payloads. Its height-dependent effects also rely on the documented strip orientation; check that assumption against the physical installation.

## Iterations recorded

1. A first prototype used an MKR WiFi 1010 to toggle its on-board LED from a Wi-Fi page.
2. The next sketches tested MQTT colour control and a motion-threshold trigger.
3. A data-collection sketch recorded IMU samples for Edge Impulse. The MKR1010 library build hit a CMSIS header compatibility issue.
4. The data collector was moved to an ESP32 Feather, followed by gesture tests and a Vespera MQTT test.
5. The integrated sketches combined the sensor, classifier, button and luminaire. The 3 Nov visual-effects candidate added a countdown cue and sequential fallback preview. The final adopted 4 Nov sketch uses the MAGIC export, changes the ci action to lights-off and gives a red cue on low confidence.

The file-by-file record is in [development-log.md](development-log.md), and the source history is indexed in [code-inventory.md](code-inventory.md).

## What the project demonstrates

The project brings together embedded sensing, a trained gesture classifier, network messaging and a physical light output. The author reports that their demonstrations worked end to end. The retained serial examples show individual inference outputs; they are not a controlled accuracy study.

The earlier MKR attempt also exposed that a model export can depend on board-specific DSP headers. Moving to a supported board made it possible to continue integration, while keeping the inference sampling rate, axes and units consistent with the training pipeline remained an important implementation constraint.

## What still needs checking

- Capture one representative serial log and a short Vespera video from the final adopted sketch.
- Confirm Wi-Fi and MQTT stay connected during the two-second capture.
- Compare the observed colour cues with the intended cyan, purple and red feedback.
- If reporting model performance, state the model version, number of trials and denominator; do not treat confidence scores as accuracy.
- Keep Wi-Fi/MQTT credentials in the ignored local header.
