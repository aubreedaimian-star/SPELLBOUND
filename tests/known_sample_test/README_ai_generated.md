# SPELLBOUND known-sample verification

`spellbound_known_sample_test.ino` is a standalone, one-shot hardware test for
the installed `MAGIC_inferencing` Edge Impulse library. It does not read the
MPU6050, connect to Wi-Fi/MQTT, or modify the existing library example.

## Model input contract

- Project: `DAIMIAN / MAGIC`, project ID `1149248`, deployment version `1`
- Input to `run_classifier()`: 600 raw `float` values
- Shape: 100 time rows x 6 axes
- Sampling: 50 Hz / 20 ms interval
- Window: 2,000 ms
- Row-interleaved axis order: `accX, accY, accZ, gyroX, gyroY, gyroZ`
- Axis units in the Edge Impulse record: accelerometer `m/s2`; gyroscope `N/A`
- DSP: spectral analysis; the generated EON network is int8 quantized, but the
  public classifier signal still receives the raw floating-point sensor values
- Labels, in model order: `Ascendio`, `ci`, `incendio`, `leviosa`, `oppugno`,
  `stupefy`

## Sample provenance

- Edge Impulse category: `testing`
- Label: `incendio`
- Sample ID: `3199609377`
- Filename: `incendio.760o197i`
- Captured: 100 rows at 50 Hz
- Edge Impulse signature validation: valid
- Edge Impulse payload SHA-256:
  `2c6f31bf37e41d9ced65be3e0bc46ba5331a43caac2618bfcab9b38e5d6b78a1`
- Verification performed: the 600 sketch literals were compared numerically
  against a fresh read-only Studio API response; mismatch count was zero.

## Compile verification

Board FQBN: `arduino:samd:mkrwifi1010`

```powershell
& 'D:\Arduino\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe' `
  --config-dir '%USERPROFILE%\AppData\Local\Arduino15' `
  compile `
  --fqbn arduino:samd:mkrwifi1010 `
  --clean `
  --build-path '%USERPROFILE%\Documents\Codex\ei-knownbuild' `
  '%USERPROFILE%\Documents\Codex\2026-10-29\spellbound-fix-edge-impulse-arduino-deployment\outputs\spellbound_known_sample_test'
```

Result with Arduino SAMD core 1.8.14:

- Flash: 99,124 / 262,144 bytes (37%)
- SRAM: 5,980 / 32,768 bytes (18%)
- SRAM remaining for stack and locals: 26,788 bytes

Use a short build path on Windows. A build directory nested under the full task
path exceeds the path length accepted by the bundled ARM GCC dependency-file
handling.

## Hardware run

No firmware was uploaded during preparation of this test. To run it in Arduino
IDE, open the `.ino`, select **Arduino MKR WiFi 1010**, select the correct port,
upload, then open Serial Monitor at **115200 baud**. The sketch runs inference
once after the native USB Serial connection opens; press Reset to repeat it.

The real board output will have this form. Values marked `measured on board`
are intentionally not predicted or fabricated here:

```text
SPELLBOUND Edge Impulse known-sample test
Project: DAIMIAN / MAGIC (ID 1149248, deployment v1)
Sample: incendio.760o197i (ID 3199609377, expected label: incendio)
Input: 600 values = 100 rows x 6 axes, 50.0 Hz, 2000 ms
Axis order: accX, accY, accZ, gyroX, gyroY, gyroZ
run_classifier returned: <measured on board; 0 means success>
Timing (ms): DSP <measured>, inference <measured>, anomaly <measured>
Predictions:
  Ascendio: <measured on board>
  ci: <measured on board>
  incendio: <measured on board>
  leviosa: <measured on board>
  oppugno: <measured on board>
  stupefy: <measured on board>
Test complete. Reset the board to run it again.
```

If `run_classifier returned` is nonzero, the sketch prints an explicit error
and does not present the prediction fields as valid.
