# Testing status

Only things that were actually observed are marked as working. Everything else is **UNVERIFIED**.

## 1. Shown by serial output (v4, 2 Nov, one run)
Source: `evidence/serial_logs/2026-11-02_v4_ascendio_AUTHOR_REPORTED.txt`. These are two lines reported by the author; the full log was not saved.

| Observed | Status |
|---|---|
| v4 compiled and uploaded to the ESP32 Feather | PASS (implied by the run; assumes the run used `v4.ino` as saved, since the run time was not recorded) |
| The MPU6050 started (v4 stops at boot if it isn't found) | PASS (implied) |
| Button hold and release triggered capture, with at least 100 samples (classification is only reached through this path) | PASS (implied) |
| `run_classifier` succeeded: one author-reported on-device classification of Ascendio with a confidence score of 0.9922 (above the 0.70 threshold) | PASS, **one gesture, one run** |
| `[SPELL] CAST: Ascendio` printed | PASS |

This does **not** show model accuracy, repeatability across gestures or users, or anything about lighting.

## 2. In the code but not verified on hardware
| Feature (v4) | Status |
|---|---|
| Wi-Fi connection | **Failed during the recorded Ascendio run.** Not tested otherwise; this does not show it always fails |
| MQTT connection to `mqtt.cetools.org:1884` | UNVERIFIED |
| Publishing 216-byte frames to `luminaire/11` | UNVERIFIED |
| Vespera showing the animations (Ascendio, ci, incendio, leviosa, oppugno; Stupefy turns it off) | UNVERIFIED |
| Purple/red success and failure flashes, cyan "capturing" colour | UNVERIFIED (these need MQTT) |
| Low-confidence and "too short" handling | UNVERIFIED |
| Classification of the other five spells | UNVERIFIED |

## 3. Earlier sketches
| Sketch | Compile | Ran on hardware |
|---|---|---|
| 01 web LED | UNVERIFIED | UNVERIFIED |
| 02 Vespera v1, 03 threshold wand | UNVERIFIED (host syntax check passes) | UNVERIFIED |
| 04 MKR collector | Original as found: FAIL (pasted text). Repaired: UNVERIFIED | INDIRECT (the MAGIC training data exists) |
| 05–10 ESP32 sketches | UNVERIFIED | UNVERIFIED (no logs found) |
| Edge Impulse example on the MKR1010 | FAIL, then PASS after the CMSIS patch (AI agent, compile only) | Not uploaded |

## 4. Checks run on a PC during the recovery
- `tests/host_syntax_check`: the older MKR and collector sketches parse (5 pass); the corrupted collector fails as expected.
- `tests/host_logic_tests`: threshold-wand logic, 10/10 pass.

These use simplified stand-in libraries. They are not Arduino builds or hardware tests, and they do not cover v4.

## Next tests to record
1. Fix Wi-Fi in v4 (see `known-issues.md`) and save the full Serial Monitor output, including `[MQTT] Connected!`.
2. Cast each spell several times and record how many are recognised correctly.
3. Film Vespera responding, alongside the Serial Monitor or MQTT Explorer.
