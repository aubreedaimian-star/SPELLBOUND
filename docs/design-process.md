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

The intended data path is:

`button → 100 IMU samples at 50 Hz → classifier → spell/fallback choice → MQTT RGB frames → Vespera`

The code assumes a 72-pixel luminaire and 216-byte RGB payloads. Its height-dependent effects also rely on the documented strip orientation; check that assumption against the physical installation.

## Iterations recorded

1. A first prototype used an MKR WiFi 1010 to toggle its on-board LED from a Wi-Fi page.
2. The next sketches tested MQTT colour control and a motion-threshold trigger.
3. A data-collection sketch recorded IMU samples for Edge Impulse. The MKR1010 library build hit a CMSIS header compatibility issue.
4. The data collector was moved to an ESP32 Feather, followed by gesture tests and a Vespera MQTT test.
5. The integrated sketches combined the sensor, classifier, button and luminaire. A later visual-effects candidate adds a two-second countdown cue, spell-specific animation, a sequential fallback preview, and a final frame that remains visible.

The file-by-file record is in [development-log.md](development-log.md), and the source history is indexed in [code-inventory.md](code-inventory.md).

## What the project demonstrates

The project brings together embedded sensing, a trained gesture classifier, network messaging and a physical light output. The author reports that their demonstrations worked end to end. The retained serial examples show individual inference outputs; they are not a controlled accuracy study.

The earlier MKR attempt also exposed that a model export can depend on board-specific DSP headers. Moving to a supported board made it possible to continue integration, while keeping the inference sampling rate, axes and units consistent with the training pipeline remained an important implementation constraint.

## Next iteration

A useful next version would:

- confirm which exact sketch and model export are flashed for the demonstration;
- record one full serial capture alongside a short video of the light output;
- check the luminaire's actual pixel/strip orientation and tune the effects against it;
- evaluate each gesture over a fixed, documented number of trials before reporting recognition performance;
- keep Wi-Fi/MQTT credentials in a local ignored header and document only the setup steps.
