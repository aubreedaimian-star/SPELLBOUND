# Code inventory

Original SHA-256 hashes (first 16 characters) identify the files as found. Full hashes are in `evidence/SHA256SUMS_originals.txt`. `%USERPROFILE%` is the Windows user folder.

## Included

| Staged | Original | Last saved | SHA-256 (original) | Notes |
|---|---|---|---|---|
| `firmware/latest/v4/v4.ino` | `D:\CE014\v4\v4.ino` | 02 Nov 21:05 | `836e263e4379b11e` | Latest: button capture → MAGIC2 → Vespera. Unchanged (no password inside; SSID is the public lab network name) |
| `firmware/history/01_webled_mkr1010/…/SimpleWebServerWiFi.ino` | `D:\CE014\SimpleWebServerWiFi\SimpleWebServerWiFi.ino` | 12 Oct 12:36 | `f8501110d52870ff` | Unchanged |
| `firmware/history/02_vespera_mqtt_test_v1/…/SPELLBOUND_Vespera_Test.ino` | `%USERPROFILE%\Downloads\SPELLBOUND_Vespera_Test.zip` | 26 Oct 10:58 | `655644275a6ac8c1` | Unchanged |
| `firmware/history/03_vespera_threshold_v2/…/SPELLBOUND_Vespera_Test.ino` | `D:\SPELLBOUND_Vespera_Test\SPELLBOUND_Vespera_Test.ino` | 26 Oct 13:10 | `27208d6714329303` | **Lines 14–17 changed**: hard-coded passwords → SECRET_* macros |
| `firmware/history/04_datacollector_mkr1010_repaired/…/SPELLBOUND_DataCollector.ino` | `D:\CE014\SPELLBOUND_DataCollector\SPELLBOUND_DataCollector.ino` | 29 Oct 18:49 | `4e10f846955ab42a` | **Repaired**: pasted text removed (REPAIR.diff). Original in evidence/ |
| `firmware/history/05_datacollector_esp32/…/sketch_nov1a.ino` | `D:\CE014\SPELLBOUND_DataCollector\sketch_nov1a\sketch_nov1a.ino` | 01 Nov 18:29 | `cbb061edb78b6bca` | Unchanged |
| `firmware/history/06_esp32_gesture_test/…/SPELLBOUND_Gesture_Test.ino` | `%USERPROFILE%\Documents\Arduino\SPELLBOUND_Gesture_Test\SPELLBOUND_Gesture_Test.ino` | 02 Nov 13:03 | `92629518a9c4d592` | Unchanged |
| `firmware/history/07_esp32_gesture_test2/test2/test2.ino` | `%USERPROFILE%\Documents\Arduino\test2\test2.ino` | 02 Nov 13:04 | `e0377da634087f9d` | Unchanged |
| `firmware/history/07_esp32_gesture_test2/…/SPELLBOUND_MAGIC2_ESP32.ino` | `D:\CE014\SPELLBOUND_MAGIC2_ESP32_sketch.zip` | 02 Nov 14:32 (zip entry) | `76e7a1bd52c63176` | Unchanged |
| `firmware/history/08_esp32_vespera_mqtt_test/…/SPELLBOUND_Vespera_ESP32_Test.ino` | `%USERPROFILE%\Documents\Arduino\SPELLBOUND_Vespera_ESP32_Test\SPELLBOUND_Vespera_ESP32_Test.ino` | 02 Nov 15:29 | `111ac53ebc8a23c9` | Unchanged |
| `firmware/history/09_integrated_serial_trigger/…/SPELLBOUND_Integrated.ino` | `D:\CE014\SPELLBOUND_Integrated\SPELLBOUND_Integrated\SPELLBOUND_Integrated.ino` | 02 Nov 15:56 | `e44114bd173f6ea2` | Unchanged (folder version, includes MAGIC2) |
| `firmware/history/10_button_cue/…/SPELLBOUND_Button_Cue.ino` | `%USERPROFILE%\Downloads\SPELLBOUND_Button_Cue\SPELLBOUND_Button_Cue.ino` | 02 Nov 17:10 | `0485edd797e95f44` | Unchanged |
| `experiments/02_…/static_buffer.ino` | `D:\static_buffer\static_buffer.ino` | 29 Oct 16:03 | `698ac44ccfcec8ec` | Unchanged (macro workaround attempt; authorship awaiting confirmation) |
| `experiments/03_cmsis_macro_probe/…` | `%USERPROFILE%\Documents\Codex\2026-10-29\…\work\cmsis_macro_probe` | 29 Oct 16:15 | `9f515e785c008397` | Unchanged (AI agent) |
| `experiments/04_samd_cmsis_compat_patch/…` | `%USERPROFILE%\Documents\Arduino\libraries\MAGIC_inferencing\…\arm_math_types.h` | 29 Oct 16:29 | `d7f1f0b93c96ee85` | Excerpt, lines 84–131 (AI agent patch) |
| `tests/known_sample_test/…/spellbound_known_sample_test.ino` | `%USERPROFILE%\Documents\Codex\2026-10-29\…\outputs\…` | 29 Oct 18:16 | `4b9bb82b18e0936b` | Unchanged (AI agent) |
| `models/MAGIC_inferencing_v1/*` | `%USERPROFILE%\Documents\Arduino\libraries\MAGIC_inferencing` | 29 Oct 15:41 | `a85602731375cc04` | Metadata and compiled model only (hash = model .cpp) |
| `models/MAGIC2_inferencing_v3/*` | `%USERPROFILE%\Documents\Arduino\libraries\MAGIC2_inferencing` | 02 Nov 12:30 | `94c485e3578fc913` | Metadata and compiled model only (hash = model .cpp) |
| `evidence/original_as_found/SPELLBOUND_DataCollector.CORRUPTED_AS_FOUND.ino.txt` | `D:\CE014\SPELLBOUND_DataCollector\SPELLBOUND_DataCollector.ino` | 29 Oct 18:49 | `4e10f846955ab42a` | Byte-identical, renamed .txt |
| `evidence/serial_logs/2026-11-02_v4_ascendio_AUTHOR_REPORTED.txt` | `(chat message, 3 Nov)` | 03 Nov | — | Two serial lines reported by the author; no log file on the PC |

Each sketch folder also has an `arduino_secrets.example.h` with placeholders. The ZIPs `SPELLBOUND_Vespera_Test.zip` (v1) and `SPELLBOUND_MAGIC2_ESP32_sketch.zip` were unpacked and their sketches included.

## Excluded: credentials

| File | Contains | Action |
|---|---|---|
| every `arduino_secrets.h` (D:\CE014\v4, \SimpleWebServerWiFi, \SPELLBOUND_Integrated, \SPELLBOUND_Vespera_ESP32_Test_copy…, D:\SPELLBOUND_Vespera_Test, Documents\Arduino\SPELLBOUND_Vespera_ESP32_Test, Downloads\SPELLBOUND_Button_Cue) | real Wi-Fi and/or MQTT passwords | not copied |
| `Documents\Arduino\arduino_secrets\arduino_secrets.ino` | **real passwords saved as a sketch file** (.ino, not .h) | not copied. This folder is inside the sketchbook, so it should never be added to Git |
| `D:\SPELLBOUND_Vespera_Test\SPELLBOUND_Vespera_Test.ino` | hard-coded passwords | sanitised copy only (lines 14–17) |

A scan of the whole staging folder for the actual password values found none.

## Excluded: duplicates

| File | Same as |
|---|---|
| `Downloads\SPELLBOUND_Button_Cue.ino`, `(1)`, `(2)`, `(3)` | `Downloads\SPELLBOUND_Button_Cue\SPELLBOUND_Button_Cue.ino` (identical SHA-256) |
| `D:\CE014\SPELLBOUND_Vespera_ESP32_Test_copy_20261102155122\…ino` | sketch 08 (identical SHA-256) |
| `Documents\Arduino\arduino_secrets\SPELLBOUND_Vespera_ESP32_Test.ino` | sketch 08 (identical apart from line endings) |
| `D:\CE014\SPELLBOUND_Integrated.zip` | sketch 09, except line 7 includes `MAGIC_inferencing.h` instead of MAGIC2 |
| `Downloads\ei-magic2-arduino-1.0.3-impulse-#1.zip` (5 MB) | the installed MAGIC2 library; only metadata and model kept |
| `Downloads\SPELLBOUND_Vespera_Test (1).zip`, `Codex\ei-static-buffer-test`, `Codex\MAGIC_inferencing-fixed`, build-config copies | identical copies found on 2 Nov (listed in the 2 Nov review ZIP inventory) |

## Excluded: unrelated or generated

ClapDetector, training (other modules), FallDetection libraries and `D:\EDGE`, third-party libraries, Arduino build folders (`*.o`, `*.elf`, `*.bin`, caches), Codex task folders for other projects, CVs and other personal documents in Downloads (not opened).
