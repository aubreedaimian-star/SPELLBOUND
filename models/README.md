# Models

These folders are **not complete Arduino libraries**. Each contains only the model metadata (`model_metadata.h`, `model_variables.h`), `library.properties` and the compiled model (`tflite_learn_*_compiled.cpp`) from the Edge Impulse Arduino export. They are kept as a record of which model was used.

| Folder | Edge Impulse project | Used by |
|---|---|---|
| `MAGIC_inferencing_v1` | MAGIC, ID 1149248, deployment 1 | `history/06`, `07/test2`, `10`, `experiments/` |
| `MAGIC2_inferencing_v3` | MAGIC2, ID 1153638, deployment 3 | `firmware/latest/v4`, `history/07/SPELLBOUND_MAGIC2_ESP32`, `history/09` |

The Edge Impulse SDK (`src/edge-impulse-sdk/`, around 1,300 files), the `MAGIC2_inferencing.h` header and the example sketches are **not included**. To build v4, install the full `MAGIC2_inferencing` Arduino library exported from the Edge Impulse project (Deployment → Arduino library) through *Sketch → Include Library → Add .ZIP Library*. This repository does not build on its own.
