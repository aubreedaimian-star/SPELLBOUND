> **AI-generated document — not the author's own work.** Written by Claude (Anthropic, an AI assistant) on 2 Nov 2026 while recovering this project from the author's PC, before the v4 sketch was found. It is kept for reference only. Statements about who wrote which file are superseded by `docs/ai-contributions.md`; current status is in `README.md` and `docs/testing.md`.

# Debugging log

Each problem lists its symptom, the evidence available, the root cause and its status. Labels follow `development-log.md`.

---

## D1: Edge Impulse export does not compile for Arduino MKR WiFi 1010 (SAMD21, Cortex-M0+)

**Symptom.** Compiling the unmodified `MAGIC_inferencing` `static_buffer` example for `arduino:samd:mkrwifi1010` (SAMD core 1.8.14) failed with errors about `__STATIC_FORCEINLINE`. After a partial workaround, it failed on `__SSAT`.

**Evidence.**
- The author's own attempt (`experiments/02_ei_static_buffer_manual_macro_attempt/`, 29 Oct 16:03) defines `__STATIC_FORCEINLINE` and `EIDSP_USE_CMSIS_DSP 0` inside the sketch. This is VERIFIED and shows the error was seen first-hand.
- Build configurations for each attempt are in `evidence/build_configs/` (VERIFIED).
- The diagnosis is in `evidence/ai_assisted_reports/SPELLBOUND-edge-impulse-fix-report.md`. **This report was written by an AI coding agent (Codex).** It is REPORTED (AI).
- The raw compiler output was **not saved**. The final error messages are known only from the report and the author's attempt.

**Hypotheses tried.**
| # | Attempt | Result |
|---|---|---|
| 1 | Define the missing macro in the sketch (author) | Outcome not recorded; INFERRED unsuccessful. Macros in a `.ino` do not affect the library's separately compiled `.cpp` files. |
| 2 | Probe sketch: does the SAMD core define `__STATIC_FORCEINLINE` / `__SSAT`? (`experiments/03_cmsis_macro_probe`) | REPORTED (AI): neither is defined by CMSIS-Core 4.5 for Cortex-M0+ |
| 3 | Disable CMSIS-DSP via build flags (`-DEIDSP_USE_CMSIS_DSP=0 -DEIDSP_LOAD_CMSIS_DSP_SOURCES=0`) | REPORTED (AI): still failed, because `numpy.hpp` includes a CMSIS statistics header unconditionally |
| 4 | Add a SAMD-only compatibility block to `CMSIS/DSP/Include/arm_math_types.h` (`experiments/04_samd_cmsis_compat_patch`) | Compiles. VERIFIED by the artefact: `.bin` of 90,716 B. Compile only. |

**Root cause (REPORTED (AI); consistent with the evidence).** Arduino SAMD 1.8.14 brings in CMSIS-Core 4.5 (`cmsis_gcc.h` v4.30) via `Arduino.h`. The Edge Impulse SDK bundles CMSIS-DSP 1.10, which expects CMSIS-Core 5 (`cmsis_gcc.h` v5.4.1). Both headers use the include guard `__CMSIS_GCC_H`, so the older one wins and the newer macros (`__STATIC_FORCEINLINE`, `__SSAT`, `__USAT`) are never defined. The Cortex-M0+ has no SSAT/USAT instructions, so the patch supplies portable C equivalents.

**Status.** Compilation fixed. **On-device inference has never been run.** The patch lives inside the installed library and **will be lost if the library is re-exported or reinstalled**. Keep `experiments/04_…` as the reproducible patch.

**What I learned (for reflection).** Toolchain or header version mismatches can look like code errors. Reading the include chain and probing macros is faster than trial-and-error defines. A macro defined in a sketch does not reach library compilation units.

---

## D2: DataCollector stops compiling (pasted text)

**Symptom.** Compile errors such as `'Filter' does not name a type` and `'MPU_ADDR' was not declared in this scope`.

**Evidence.** `evidence/original_as_found/SPELLBOUND_DataCollector.CORRUPTED_AS_FOUND.ino.txt` (VERIFIED; SHA-256 in `code-inventory.md`). Lines 6–14 contain web-UI text ("Filter your search...", "Type:", "All") pasted into the middle of the comment `// Edge Impulse Data Forwarder`. The failure was REPORTED (AI) on 31 Oct and **reproduced** on 2 Nov by the host syntax check (`tests/host_syntax_check/RESULT_2026-11-02.txt`).

**Fix.** `firmware/04_datacollector_mkr1010_repaired/` removes only those lines and restores the comment. No functional code was changed (`diff` in `code-inventory.md`). The host syntax check passes. **Arduino build and hardware run are UNVERIFIED.**

---

## D3: Long build paths on Windows

REPORTED (AI) in `tests/known_sample_test/README_ai_generated.md`: a build directory nested deep under the task folder exceeded the path length that the bundled ARM GCC dependency-file handling accepts. Short build paths (`…\Codex\ei-knownbuild`) were used instead. This was also seen during recovery: some library files were too deeply nested to copy.

---

## D4: Unit and contract mismatch between data collectors (open)

- MKR1010 collector: accel in **m/s²** (`/16384 × 9.80665`), 3 decimals, `SMPLRT_DIV = 19` and DLPF 3 → 50 Hz output rate.
- ESP32 collector (`sketch_nov1a`): accel in **g**, 5 decimals, no DLPF or sample-rate configuration, clock source PLL-X.
- The MAGIC model's test sample values (accX ≈ −9.4) are in m/s².

**Risk.** Feeding ESP32 data in g to the current model, or mixing both datasets, would scale accel features about 9.81× and change spectral content. **Status: unresolved and untested.** Choose one contract and either convert on the device or re-collect and re-train.

---

## D5: Credentials committed in code (security)

`SPELLBOUND_Vespera_Test.ino` (v2) contained the real Wi-Fi and MQTT passwords as string literals, and two `arduino_secrets.h` files contained real passwords. All of these are excluded or replaced in this archive (`docs/sanitization-report.md`). The author should consider **changing those passwords** if any copy was ever shared or uploaded.
