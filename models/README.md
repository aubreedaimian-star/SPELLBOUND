# Models

These folders are **not complete Arduino libraries**. Each contains only the model metadata (`model_metadata.h`, `model_variables.h`), `library.properties` and the compiled model (`tflite_learn_*_compiled.cpp`) from the Edge Impulse Arduino export. They are kept as a record of which model was used.

| Folder | Edge Impulse project | Used by |
|---|---|---|
| `MAGIC_inferencing_v1` | MAGIC, ID 1149248, deployment 1 | `firmware/final/SPELLBOUND_Final.ino`, earlier candidate, `history/06`, `07/test2`, `10`, `experiments/` |
| `MAGIC2_inferencing_v3` | MAGIC2, ID 1153638, deployment 3 | `firmware/latest/v4`, `history/07/SPELLBOUND_MAGIC2_ESP32`, `history/09` |

The Edge Impulse SDK (`src/edge-impulse-sdk/`, around 1,300 files), the generated inference headers and example sketches are **not included**. To build v4, install the full `MAGIC2_inferencing` Arduino library exported from the Edge Impulse project (Deployment → Arduino library) through *Sketch → Include Library → Add .ZIP Library*. This repository does not build on its own.


The final adopted sketch sets `ACCEL_IN_G = false` and includes `MAGIC_inferencing.h`. Its header says the acceleration units were checked against 227 MAGIC samples. The full raw sample export is not included here, so this repository records the source's stated check rather than independently recalculating it.
