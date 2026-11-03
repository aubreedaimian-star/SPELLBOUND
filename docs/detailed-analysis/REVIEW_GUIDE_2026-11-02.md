> **AI-generated document — not the author's own work.** Written by Claude (Anthropic, an AI assistant) on 2 Nov 2026 while recovering this project from the author's PC, before the v4 sketch was found. It is kept for reference only. Statements about who wrote which file are superseded by `docs/ai-contributions.md`; current status is in `README.md` and `docs/testing.md`.

# Guide for an independent reviewer

This archive reconstructs the SPELLBOUND project from files found on the author's PC on 2 Nov 2026, for UCL CASA0014 (criteria: Sensors, System, Code, Testing, Understanding, Repository, Critical Reflection).

Please check in particular:
1. **Over-claiming.** Does any document claim something works without evidence? The intended position is that nothing has recorded hardware evidence (`docs/testing-report.md`).
2. **Evidence labels.** Are VERIFIED, REPORTED (AI), INFERRED and NEEDS CONFIRMATION in `docs/development-log.md` applied consistently?
3. **Sensor understanding.** MPU6050 register settings, scaling, sample rate and the m/s² vs g unit mismatch (`docs/debugging-log.md` D4, `docs/threshold-implementation-analysis.md`).
4. **Code quality and risks** in `firmware/03` (blocking reconnects, I²C error handling, hard-coded thresholds and client ID).
5. **Threshold vs ML comparison** (`docs/threshold-vs-edge-impulse.md`). No benchmark numbers were invented; flag any that look invented.
6. **Repair integrity.** `firmware/04_…/REPAIR.diff` should only remove pasted text. The v2 sanitisation should only change lines 14–17.
7. **Secrets.** There should be no real passwords anywhere. Report any you find.

Out of scope: third-party library code, build caches and other modules' projects were intentionally excluded (`docs/code-inventory.md`).
