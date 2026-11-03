> **AI-generated document — not the author's own work.** Written by Claude (Anthropic, an AI assistant) on 2 Nov 2026 while recovering this project from the author's PC, before the v4 sketch was found. It is kept for reference only. Statements about who wrote which file are superseded by `docs/ai-contributions.md`; current status is in `README.md` and `docs/testing.md`.

# Development log (reconstructed)

This timeline was rebuilt on 2026-11-02 from files that survive on the development PC. **File modification times are clues, not proof**: a file is re-stamped every time it is saved, copied or extracted. Every entry carries an evidence label.

| Label | Meaning |
|---|---|
| **VERIFIED** | The file exists and its content shows this directly (hash in `code-inventory.md`). |
| **REPORTED (AI)** | Stated only in a report written by an AI coding agent (Codex) on the PC. The agent's build folders and binaries exist, but its console output was not saved. |
| **INFERRED** | Deduced from timestamps, filenames or indirect evidence. Could be wrong. |
| **NEEDS CONFIRMATION** | Only the author can confirm this, ideally with a photo, video or serial log. |

All times are Europe/London local time.

---

## Phase A: Wi-Fi and a web-controlled LED (12 Oct 2026)

| When | What | Evidence |
|---|---|---|
| 12 Oct 12:20 | `arduino_secrets.h` created for the web-server sketch (it held a real Wi-Fi password, which is excluded from this archive). | VERIFIED (mtime, content) |
| 12 Oct 12:36 | `SimpleWebServerWiFi.ino` last saved. It is a heavily extended version of the WiFiNINA *SimpleWebServerWiFi* example: an embedded "LUMEN" dashboard (HTML/CSS/JS) plus a small JSON API (`/api/on`, `/api/off`, `/api/status`) that toggles `LED_BUILTIN` on the MKR WiFi 1010. | VERIFIED (content) |
| | Whether it ran and the dashboard toggled the LED. | NEEDS CONFIRMATION |

## Phase B: MQTT to the Vespera luminaire (26 Oct 2026)

| When | What | Evidence |
|---|---|---|
| 26 Oct 10:58 | `SPELLBOUND_Vespera_Test.zip` (v1) packaged: MKR1010 → Wi-Fi → MQTT `mqtt.cetools.org:1884` → topic `student/CASA0014/luminaire/11`. It publishes a 216-byte binary frame (72 px × RGB) cycling gold → blue → off every 3 s. **No IMU.** Credentials come from `arduino_secrets.h`. The two zips in Downloads are byte-identical. | VERIFIED |
| 26 Oct | The v1 README says *"The code has not been compiled against a real MKR1010 in this environment."* That wording suggests v1 and its README were drafted with an AI assistant. | INFERRED (from wording) |
| 26 Oct 12:36 | PubSubClient library installed in the sketchbook. | VERIFIED (mtime); this suggests compile attempts were made but does not prove them. INFERRED |
| 26 Oct 12:55 | The zip contents appear in `D:\SPELLBOUND_Vespera_Test` (README and example secrets are identical to the zip). | VERIFIED (diff) |
| 26 Oct 13:02 | A top-level `arduino_secrets.h` with a real MQTT password is created in that folder (excluded). | VERIFIED |
| 26 Oct 13:10 | `SPELLBOUND_Vespera_Test.ino` (**v2**) last saved. It is effectively a rewrite: an MPU6050 read over I²C every 25 ms, a **threshold** motion detector (gyro > 60 °/s or Σ|Δaccel| > 0.35 g), and a rainbow animation published every 300 ms while motion is fresh (2 s hold), then one "off" frame. The header comment was not updated ("does not use an IMU yet"). **Credentials were written directly into the code** even though `arduino_secrets.h` is included, so the archived copy is sanitised. | VERIFIED |
| | Whether v2 was uploaded and the Vespera actually lit up. | **NEEDS CONFIRMATION**. No serial log, screenshot or video was found. See `threshold-implementation-analysis.md`. |

## Phase C: Data collection and Edge Impulse (≤ 29 Oct 2026)

| When | What | Evidence |
|---|---|---|
| before 29 Oct 15:41 | Motion data for six spell gestures (`Ascendio, ci, incendio, leviosa, oppugno, stupefy`) was captured at 50 Hz on 6 axes and a model trained in Edge Impulse project **MAGIC (ID 1149248)**. | INFERRED: the exported model exists, and the AI-generated known-sample test quotes an Edge Impulse *testing* sample whose values are in m/s² |
| | The data was captured with the MKR1010 `SPELLBOUND_DataCollector` through the Edge Impulse Data Forwarder. | INFERRED: same 50 Hz rate, same axis order, and the same m/s² units as the test sample. The current file was last saved *after* the export (see below), so the exact version used is unknown. NEEDS CONFIRMATION |
| 29 Oct 15:41 | Edge Impulse Arduino library `MAGIC_inferencing` v1.0.1 (Studio 1.97.1) installed: 100 samples × 6 axes, 20 ms interval, spectral-analysis DSP, int8 EON model. | VERIFIED |

## Phase D: Deploying to the MKR WiFi 1010 fails with CMSIS errors (29 Oct 2026)

| When | What | Evidence |
|---|---|---|
| 29 Oct 16:03 | The author's own fix attempt, `D:\static_buffer\static_buffer.ino`: the upstream example plus `#define __STATIC_FORCEINLINE …` and `#define EIDSP_USE_CMSIS_DSP 0`. This shows independently that the author saw `__STATIC_FORCEINLINE` compile errors. | VERIFIED (diff in `experiments/02_…/diff_vs_upstream.diff`) |
| | The outcome of that attempt. | INFERRED unsuccessful (macros in the sketch can't reach separately compiled library files, and an AI agent was engaged eight minutes later). NEEDS CONFIRMATION |
| 29 Oct 16:11 | AI agent "red" build of the unmodified example for `arduino:samd:mkrwifi1010`: it failed. | REPORTED (AI); the build-config file is VERIFIED; the raw error text was not preserved |
| 29 Oct 16:15 | `cmsis_macro_probe`: checks whether the SAMD core defines `__STATIC_FORCEINLINE` / `__SSAT`. | VERIFIED (code); result REPORTED (AI) |
| 29 Oct 16:17 | Build with `-DEIDSP_USE_CMSIS_DSP=0 -DEIDSP_LOAD_CMSIS_DSP_SOURCES=0`: still failed, because `numpy.hpp` includes CMSIS headers unconditionally. | Config VERIFIED; outcome REPORTED (AI) |
| 29 Oct 16:21–16:29 | Compatibility block added to the library's `CMSIS/DSP/Include/arm_math_types.h`, active only for SAMD on Cortex-M0+. The patched copy and the installed copy are byte-identical, and the model files are unchanged versus the pre-patch backup. | VERIFIED (hashes) |
| 29 Oct 16:38 | Patched library compiles: `static_buffer.ino.bin` = **90,716 bytes**, which matches the report's flash figure. **Compile only. Nothing was uploaded.** | VERIFIED (artefact) and REPORTED (AI) |
| 29 Oct 18:16–18:25 | AI-written `spellbound_known_sample_test.ino` (600 floats from Edge Impulse test sample `incendio.760o197i`). It compiles: the `.bin` is 99,132 bytes and the report says 99,124 bytes of flash. **Never uploaded. No predictions exist.** | VERIFIED (artefact); "never uploaded" is REPORTED (AI) |
| 29 Oct 18:49 | `SPELLBOUND_DataCollector.ino` last saved, **with eight lines of pasted web-page text** ("Filter your search… / Type: / All") splitting a comment. This breaks compilation. | VERIFIED |

## Phase E: Moving to the ESP32 (31 Oct to 1 Nov 2026)

| When | What | Evidence |
|---|---|---|
| 31 Oct 14:49 | CP210x USB-UART driver downloaded. The Adafruit HUZZAH32 uses a CP2104 chip, so an ESP32 Feather was probably being connected. | INFERRED |
| 31 Oct | An AI agent migrated toolchain caches C:→D:, installed the `esp32:esp32` 3.3.12 core and reports that `static_buffer` compiled for both the MKR1010 and the Adafruit ESP32 Feather. It also reports the DataCollector failing at the pasted text (`MPU_ADDR` not declared). | REPORTED (AI); no ESP32 build artefact found |
| 1 Nov 18:29 | `sketch_nov1a.ino`: an ESP32 Feather Data Forwarder (SDA 23 / SCL 22, 50 Hz). **Acceleration is output in g, not m/s²**; there is no DLPF or sample-rate divider configuration, and it prints 5 decimals. | VERIFIED |
| | Whether it compiled, streamed data, or was used to collect a new dataset. | NEEDS CONFIRMATION |

## Phase F: Recovery (2 Nov 2026)

| When | What | Evidence |
|---|---|---|
| 2 Nov | Files inventoried and hashed. Credentials removed. DataCollector repaired as a separate file (comment-only change). Host-side syntax and logic checks run (see `testing-report.md`). | VERIFIED (this archive) |

## Not found anywhere on the PC (searched 2 Nov)
NeoPixel sketch · standalone MQTT test besides Vespera v1 · casting-button code · real-time (live-IMU) Edge Impulse inference · Edge Impulse inference on the ESP32 · ML → Vespera integration · Edge Impulse CSV/JSON exports · serial-monitor logs · raw compiler output · photos/videos/screenshots of the device working. See `missing-evidence.md`.
