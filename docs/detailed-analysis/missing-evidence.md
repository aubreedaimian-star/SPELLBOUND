> **AI-generated document — not the author's own work.** Written by Claude (Anthropic, an AI assistant) on 2 Nov 2026 while recovering this project from the author's PC, before the v4 sketch was found. It is kept for reference only. Statements about who wrote which file are superseded by `docs/ai-contributions.md`; current status is in `README.md` and `docs/testing.md`.

# Missing code and missing evidence

Searched on 2 Nov 2026: `Documents\Arduino`, `Documents\Codex`, `Desktop`, `Downloads`, `D:\CE014`, `D:\SPELLBOUND_Vespera_Test`, `D:\static_buffer`, `D:\EDGE`. **Not searched:** Pictures, Videos, other D: folders, OneDrive/Google Drive, the Arduino IDE temporary "unsaved sketches" folder, Edge Impulse Studio, other computers or phones.

## Code not found
| Expected component | Notes |
|---|---|
| NeoPixel sketch | No Adafruit_NeoPixel library or sketch on the PC |
| Standalone MQTT experiment | Only Vespera v1/v2 |
| Casting button | No button handling in any sketch |
| Live MPU6050 → `run_classifier` (real-time inference) | Only fixed-input tests exist |
| Edge Impulse inference on the ESP32 | Only an AI-reported compile of the generic example |
| Spell label → Vespera colour mapping (end-to-end) | Not found |

## Evidence not found
| Evidence | Why it matters | How to recover |
|---|---|---|
| Serial-monitor logs of any sketch | Proves upload and sensor output | Re-run tests H1–H7 in `testing-report.md`; save the text |
| Video of the Vespera responding | Proves lighting response | Record during H3/H4 |
| MQTT Explorer screenshot of `luminaire/11` | Proves MQTT delivery | Capture during H3/H4 |
| Edge Impulse dataset export, class counts, confusion matrix, model accuracy | Supports any accuracy claim | Studio → Dashboard → Export; screenshot the model-testing page |
| Raw compiler error text for the CMSIS failure | Primary evidence for D1 | Rebuild against an unpatched export and save the output |
| Wiring diagram and photos of the wand | Sensors and System criteria | Draw (Fritzing/draw.io) and photograph |
| Which collector version produced the training data | Data contract | Author's recollection, or the Edge Impulse sample metadata (device name) |
