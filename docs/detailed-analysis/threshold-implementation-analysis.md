> **AI-generated document — not the author's own work.** Written by Claude (Anthropic, an AI assistant) on 2 Nov 2026 while recovering this project from the author's PC, before the v4 sketch was found. It is kept for reference only. Statements about who wrote which file are superseded by `docs/ai-contributions.md`; current status is in `README.md` and `docs/testing.md`.

# Threshold-based MPU6050 → MQTT → Vespera (firmware/03)

Source: `firmware/03_vespera_threshold_v2/SPELLBOUND_Vespera_Test/SPELLBOUND_Vespera_Test.ino`, last saved on 26 Oct 2026 at 13:10. The only change from the original is that the credential literals were replaced with `SECRET_*` macros.

## 1. What the code does

### Sensor set-up (`setupMPU`)
| Register | Value | Effect |
|---|---|---|
| `0x6B` PWR_MGMT_1 | `0x00` | Wakes the device; internal 8 MHz oscillator |
| `0x1C` ACCEL_CONFIG | `0x00` | ±2 g → 16384 LSB/g |
| `0x1B` GYRO_CONFIG | `0x00` | ±250 °/s → 131 LSB/(°/s) |
| `0x1A` CONFIG | `0x03` | DLPF ≈ 44 Hz (accel) / 42 Hz (gyro) bandwidth, per the MPU-6000/6050 register map |

The sample-rate divider (`0x19`) is **not** set. The sensor updates internally at about 1 kHz and the sketch simply reads the most recent value. It does not check WHO_AM_I; it only checks that the I²C address 0x68 ACKs.

### Sampling and detection (`detectMovement`, called every 25 ms ≈ 40 Hz)
1. A 14-byte burst read from `0x3B` (accel XYZ, temperature, gyro XYZ, all big-endian int16).
2. Raw counts are converted to g and °/s.
3. `gyroMotion = max(|gx|, |gy|, |gz|)`.
4. `accelMotion = |Δax| + |Δay| + |Δaz|` between consecutive 25 ms samples (an L1 "jerk-like" measure).
5. Movement is `gyroMotion > 60 °/s` **or** `accelMotion > 0.35 g`.

Gravity cancels in the difference, so a steady tilt does not trigger. A slow, smooth rotation also changes the gravity projection only slightly per 25 ms, so it triggers only through the gyro path.

### Output behaviour (`loop`)
- Any movement refreshes `lastMovement`. The effect stays **active for 2 s** after the last movement (`HOLD_TIME`), so continuous waving keeps it on.
- While active, a new rainbow frame is published **every 300 ms** (about 3.3 messages per second). Each frame shifts the hue by 6/256. Brightness is capped at 110 of 255.
- When the hold expires, **one** all-zero frame is published.
- `forceFrame` re-publishes the current state after an MQTT (re)connect.
- The payload is 216 raw bytes (72 × RGB) on `student/CASA0014/luminaire/11`, the same format as v1.

### Connectivity (`maintainConnection`)
It retries Wi-Fi every 10 s and MQTT every 5 s, and calls `mqtt.loop()` when connected. The PubSubClient buffer is raised to 512 bytes. A 216-byte payload plus a 29-character topic and the MQTT header is about 250 bytes, very close to the library's 256-byte default, so this increase matters.

## 2. Weaknesses visible in the code (not measured)
| # | Observation | Consequence |
|---|---|---|
| 1 | Credentials were hard-coded despite `#include "arduino_secrets.h"` | Security risk. Fixed in the archived copy only. |
| 2 | `WiFi.begin()` is called from the loop | WiFiNINA's `begin()` waits internally for a connection result, so IMU sampling can stall during reconnects. **The duration has not been measured.** |
| 3 | An I²C failure returns `false`, the same as "no movement" | A loose wire looks like a still wand. `imuReady` is checked only at boot. Confirmed by the host test. |
| 4 | Thresholds are fixed constants with no calibration of the gyro zero-rate offset | They are probably high enough to ignore offset, but sensitivity varies per device and per user. |
| 5 | One movement detector for every gesture | It cannot tell spells apart. Any sharp flick, bump or set-down of the wand counts as "magic". |
| 6 | Fixed MQTT client ID `SPELLBOUND_11` | A second board using the same ID would disconnect the first. |
| 7 | Header comment still says "does not use an IMU yet" | Misleading documentation. |
| 8 | A failed OFF publish leaves `lightsOn == true`, so it is retried on **every** loop iteration with no back-off | If the connection is up but publishing fails, the serial port fills with `MQTT publish failed` and the broker is hammered. If MQTT is down, `publishLights()` returns early (no message) and the OFF frame is sent after reconnect via `forceFrame`. |

## 3. Is there evidence that it actually ran?

**No direct evidence was found.**

- No serial-monitor capture containing the sketch's messages (`MPU6050 READY!`, `MQTT connected!`, `MAGIC DETECTED!`, `RAINBOW ON!`, `LIGHTS OFF`).
- No screenshot of the Lumi/Vespera web viewer or MQTT Explorer showing frames on `luminaire/11`.
- No photo or video of the lamp responding to the wand, and no Arduino build output for this sketch.

Indirect, weak clues: PubSubClient was installed about 30 minutes before v2 was saved, and MQTT Explorer and MQTTX shortcuts exist on the Desktop (MQTT Explorer dated 1 Oct). These show the tooling was available. **They do not show that this sketch ran.**

What *was* verified on 2 Nov (`tests/host_logic_tests`): the sketch's pure logic compiles on a PC with stub libraries, and the threshold, payload and colour-wheel functions behave as described above (10/10 checks). This is a logic check of the code, **not** a hardware test.

**To claim it works, the author needs to supply or re-record** a serial log plus a short video showing a wand flick, the `MAGIC DETECTED!` message and the Vespera lighting up, ideally with MQTT Explorer showing the 216-byte messages. `testing-report.md` lists the exact procedure.
