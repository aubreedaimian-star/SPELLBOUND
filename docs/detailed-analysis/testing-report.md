> **AI-generated document — not the author's own work.** Written by Claude (Anthropic, an AI assistant) on 2 Nov 2026 while recovering this project from the author's PC, before the v4 sketch was found. It is kept for reference only. Statements about who wrote which file are superseded by `docs/ai-contributions.md`; current status is in `README.md` and `docs/testing.md`.

# Testing report and test matrix

Status values:

| Status | Meaning |
|---|---|
| **PASS** | Run and observed. The evidence file is named. |
| **PASS (AI-reported)** | An AI agent reports a pass and a matching artefact exists, but the console output wasn't kept. |
| **FAIL** | Run and observed (or reproduced) failing. |
| **FAIL (AI-reported)** | Failure stated only in the AI report. |
| **UNVERIFIED** | No evidence either way. Not tested, or the test wasn't recorded. |
| **INDIRECT** | Circumstantial evidence only. |
| **N/A** | Doesn't apply. |

"Host" checks were run on a Linux PC on 2 Nov 2026 with **stub** Arduino headers. They show the C++ parses and that pure logic behaves as written. They are **not** Arduino builds and **not** hardware tests.

## Test matrix

| Sketch | Arduino compile | Host check | Upload | Sensor output | MQTT delivery | Lighting response |
|---|---|---|---|---|---|---|
| 01 WebLED `SimpleWebServerWiFi` | UNVERIFIED | Syntax PASS | UNVERIFIED | N/A | N/A (HTTP API: UNVERIFIED) | Built-in LED: UNVERIFIED |
| 02 Vespera v1 (fixed colours) | UNVERIFIED (its README says not compiled) | Syntax PASS | UNVERIFIED | N/A | UNVERIFIED | UNVERIFIED |
| 03 Vespera v2 (threshold) | UNVERIFIED | Syntax PASS; **logic 10/10 PASS** | UNVERIFIED | UNVERIFIED | UNVERIFIED | UNVERIFIED |
| 04 DataCollector MKR, *as found* | **FAIL** (AI-reported 31 Oct; reproduced by host syntax check 2 Nov) | Syntax FAIL (expected) | n/a | INDIRECT (an earlier, uncorrupted version probably produced the training data) | N/A | N/A |
| 04 DataCollector MKR, *repaired* | UNVERIFIED | Syntax PASS | UNVERIFIED | UNVERIFIED | N/A | N/A |
| 05 DataCollector ESP32 `sketch_nov1a` | UNVERIFIED | Syntax PASS | UNVERIFIED | UNVERIFIED | N/A | N/A |
| EI `static_buffer`, original library, MKR1010 | **FAIL (AI-reported)**; the author's own macro attempt also shows the error | n/a | n/a | N/A | N/A | N/A |
| EI `static_buffer`, `-DEIDSP_USE_CMSIS_DSP=0` | FAIL (AI-reported) | n/a | n/a | N/A | N/A | N/A |
| EI `static_buffer`, patched library, MKR1010 | **PASS (AI-reported)**; `.bin` of 90,716 B exists | n/a | UNVERIFIED (AI report: not uploaded) | N/A | N/A | N/A |
| EI `static_buffer`, ESP32 Feather | PASS (AI-reported, 31 Oct); **no artefact found** | n/a | UNVERIFIED | N/A | N/A | N/A |
| `cmsis_macro_probe` | Ran (AI-reported); result: macros absent | n/a | N/A (compile-time probe) | N/A | N/A | N/A |
| `spellbound_known_sample_test` | **PASS (AI-reported)**; `.bin` of 99,132 B exists | n/a | UNVERIFIED (AI report: not uploaded) | N/A (fixed input) | N/A | N/A |
| Known-sample **inference result** | n/a | n/a | n/a | Predictions: **UNVERIFIED (no output exists)** | N/A | N/A |
| Edge Impulse model accuracy (Studio) | n/a | n/a | n/a | UNVERIFIED (no Studio export or screenshot saved) | N/A | N/A |
| Live IMU inference, ML → Vespera, button, NeoPixel | **No code found** | | | | | |

## Tests actually run during recovery (2 Nov 2026)

1. `tests/host_syntax_check/run.sh` → `RESULT_2026-11-02.txt`: 5 PASS, plus 1 expected FAIL (the corrupted original). The failure reproduces the `'Filter' does not name a type` / `MPU_ADDR` errors.
2. `tests/host_logic_tests/run.sh` → `RESULT_2026-11-02.txt`: 10/10 PASS. It covers payload size 216, the brightness cap, the OFF frame, the gyro threshold at 59 vs 61 °/s, the accel-delta threshold, ignoring a steady tilt, and I²C NACK being treated as no movement.

## Hardware tests still to run (procedure)

Record **each** step with a Serial Monitor screenshot or text copy and, for the lamp, a short phone video. Note the board, date and library versions.

| ID | Test | Pass criterion |
|---|---|---|
| H1 | Upload `04` repaired DataCollector; hold the wand still, flat | 50 lines/s (count over 10 s); accZ ≈ +9.8 m/s² or the axis that is vertical; gyro ≈ 0 ± a few °/s |
| H2 | H1 plus `edge-impulse-data-forwarder` | Forwarder detects 6 axes at ~50 Hz (screenshot) |
| H3 | Upload `02` Vespera v1 | Serial `published 216 bytes`; Lumi viewer for luminaire 11 cycles gold → blue → off |
| H4 | Upload `03` Vespera v2; still for 10 s, then flick | No `MAGIC DETECTED!` while still; flick → `MAGIC DETECTED!` + `RAINBOW ON!` and the lamp shows a moving rainbow, then `LIGHTS OFF` about 2 s after stopping |
| H5 | H4 false-positive check: tap the table, set the wand down, walk | Count triggers per minute |
| H6 | Upload `spellbound_known_sample_test` | `run_classifier returned: 0`; top label `incendio`; record DSP and inference ms; compare with Studio's Live classification of the same sample |
| H7 | Repeat H1 with `05` ESP32 | Note the units (g) and the line rate |
