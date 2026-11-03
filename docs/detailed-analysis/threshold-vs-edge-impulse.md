> **AI-generated document — not the author's own work.** Written by Claude (Anthropic, an AI assistant) on 2 Nov 2026 while recovering this project from the author's PC, before the v4 sketch was found. It is kept for reference only. Statements about who wrote which file are superseded by `docs/ai-contributions.md`; current status is in `README.md` and `docs/testing.md`.

# Threshold vs Edge Impulse motion interpretation

No benchmark numbers in this document were measured. Figures in the "known" column come either from source code or from compile output (memory sizes from the AI-reported build). Everything else is marked **not measured**.

| Aspect | Threshold detector (firmware/03) | Edge Impulse classifier (MAGIC v1) |
|---|---|---|
| **Sensor signals used** | 3-axis gyro magnitude (max \|g\|) and the change in 3-axis acceleration between samples | All 6 raw axes (accX/Y/Z in **m/s²**, gyroX/Y/Z) as a time series |
| **Sampling** | Every 25 ms (≈ 40 Hz, `millis()`-scheduled); sensor DLPF 42–44 Hz | Must be **exactly 50 Hz** (20 ms) to match training; 100-sample, 2 s window |
| **Units and scaling contract** | Internal only (g, °/s); thresholds assume ±2 g / ±250 °/s | Must reproduce the training pipeline: m/s² for accel, same axis order, same ranges and filter. The ESP32 collector outputs **g**, which is 9.81× off if fed to this model. |
| **Output** | Binary: "magic" or nothing | Probabilities over 6 spell labels (plus the need for a confidence threshold) |
| **Processing per decision** | About 10 float operations per sample; O(1) memory (3 previous accel values) | Spectral analysis (FFT, 16 points) per axis → 78 features → int8 neural network |
| **Memory** | Not measured (sketch not built with the Arduino toolchain) | Example build: 90,716 B flash, 5,976 B static RAM; 600-float window ≈ 2.4 KB; tensor arena 3,020 B (from metadata/report). Runtime stack and heap not measured. |
| **Latency, sensing → decision** | At most one sample period (≤ 25 ms) plus the I²C read. **Not measured.** | At least the window fill (up to 2 s, unless a sliding window is used) plus DSP and inference time. **Not measured.** The known-sample test would print DSP and inference ms once run on the board. |
| **Latency, decision → light** | MQTT publish plus broker plus Vespera. **Not measured.** | Same path, not yet implemented |
| **Robustness** | False positives from any sharp knock or flick; can't separate gestures; device-dependent thresholds | Can separate trained gestures. Sensitive to data drift (new board, new mounting, new units, new user); needs representative training data and a confidence threshold. Accuracy on the device: **not measured**. |
| **Tunability and explainability** | Two constants, easy to explain and tune live | Needs re-collection, re-training and re-export; the logic is opaque |
| **Toolchain risk** | Standard libraries only | Real, demonstrated: the export did not compile for SAMD/Cortex-M0+ without patching the bundled CMSIS (see `debugging-log.md`). Re-exporting overwrites the patch. |
| **Testability** | Pure functions; host unit tests exist (10 checks) | Known-input test sketch exists (compile-only); needs an on-board run and comparison with Studio's result for the same sample |
| **Current status** | Code complete; hardware run **unverified** | Compiles for MKR1010; **no on-device inference observed**; no live-IMU integration exists |

## Discussion points for critical reflection
- The threshold detector answers "did the wand move sharply?" The classifier answers "which spell was cast?". They solve different problems. A sensible design is to use the threshold (or a button) as a **trigger** that starts a 2 s capture, then classify that window. This reduces false triggers and wasted inference. (This design has not been implemented.)
- The most important engineering lesson visible in the files is the **data contract**: sample rate, axis order, units and filtering must match between the collector used for training and the firmware used for inference. The two collectors on this PC disagree on units (m/s² vs g) and on filter settings.
- Fair comparison needs the same recorded gestures fed to both methods. A next step is to log raw 50 Hz CSV during real casting, replay it through the threshold logic on a PC (the host test harness already exists), and compare that with the Edge Impulse classification of the same windows.
