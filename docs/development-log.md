# Development log

Dates and times are the files' last-saved times (London time). They show when a file was last saved, not when the work started. **[seen]** marks something shown by a file or serial output. **[guess]** marks an inference.

## 12 Oct: Wi-Fi web LED (MKR WiFi 1010)
`history/01_webled_mkr1010`: a web page with a toggle that switches the board's LED through `/api/on` and `/api/off`. A first test of Wi-Fi.

## 26 Oct: talking to Vespera over MQTT (MKR WiFi 1010)
- `history/02_vespera_mqtt_test_v1`: sends fixed colours (gold, blue, off) as 216-byte frames to luminaire 11. No sensor yet.
- `history/03_vespera_threshold_v2`: adds the MPU6050. A quick turn (> 60 °/s) or a sudden acceleration change (> 0.35 g) counts as "magic" and shows a rainbow for 2 s. This can't tell gestures apart.
- No record of either sketch running was found. [seen]

## Up to 29 Oct: collecting gesture data, first model (MAGIC)
- `history/04_datacollector_mkr1010_repaired`: streams 6-axis CSV at 50 Hz (accel m/s², gyro °/s) to the Edge Impulse Data Forwarder.
- Six gestures were recorded and the MAGIC model (project 1149248) was trained and exported on 29 Oct. [seen: export files] Which collector and board recorded the data is awaiting the author's confirmation.
- The saved collector later got some web-page text pasted into it ("Filter your search… Type: All"), which stops it compiling. The original is kept in `evidence/original_as_found/`, and the repaired copy only removes that text. [seen]

## 29 Oct: Edge Impulse on the MKR1010 fails to compile
- The exported library failed to compile for the MKR1010 (`__STATIC_FORCEINLINE`, then `__SSAT` errors). A workaround attempt that defines the macro in the sketch is in `experiments/02` (saved 29 Oct 16:03, before the AI agent's builds; authorship awaiting confirmation). [seen]
- An AI coding agent (Codex) found the cause, a CMSIS version clash between the Arduino SAMD core and the Edge Impulse SDK, and patched one library header. After that it compiled, but nothing was uploaded. See `experiments/` and `evidence/ai_assisted_reports/`. [seen: build artefacts and report]

## 31 Oct to 1 Nov: switch to the ESP32 Feather
- CP210x USB driver downloaded [seen]; ESP32 board support installed [reported by the AI migration agent].
- `history/05_datacollector_esp32`: an ESP32 version of the collector. It outputs accel in **g** rather than m/s².

## 2 Nov: ESP32 gestures, Vespera and button (MAGIC2)
| Time | File | What changed |
|---|---|---|
| 12:28 | MAGIC2 export downloaded and installed | New model, project 1153638 deployment v3, same 6 labels, 50 Hz × 2 s |
| 13:03 | `history/06_esp32_gesture_test` | Type `g` in Serial Monitor → 2 s capture → classify (MAGIC, accel in **g**) |
| 13:04 | `history/07_esp32_gesture_test2/test2` | Same idea, accel in **m/s²**, prints DSP and inference time (MAGIC) |
| 14:39 | `history/07_…/SPELLBOUND_MAGIC2_ESP32` | test2 switched to MAGIC2, with compile-time checks of the model size and ID |
| 15:29 | `history/08_esp32_vespera_mqtt_test` | ESP32 → Vespera fixed-colour test (type r/g/b/o) |
| 15:56 | `history/09_integrated_serial_trigger` | Gesture → light effects on Vespera, triggered from Serial Monitor (its README describes it as a generated sketch; see `ai-contributions.md`) |
| 17:10 | `history/10_button_cue` | Same structure as 09 but triggered by a GPIO27 button (authorship awaiting confirmation) |
| 21:05 | **`latest/v4`** | Hold-to-capture / release-to-classify button, ring buffer, 0.70 confidence threshold, 7 s light animations (Stupefy turns the lights off). Developed with ChatGPT assistance (see `ai-contributions.md`) |

**v4 run:** one author-reported on-device classification of Ascendio with a confidence score of 0.9922, followed by `[SPELL] CAST: Ascendio`. Wi-Fi failed during this run, so MQTT delivery and lighting were not tested. This is one inference, not a measure of accuracy. [seen: author-reported serial lines, `evidence/serial_logs/`]

No serial logs or videos from the earlier 2 Nov sketches were found.
