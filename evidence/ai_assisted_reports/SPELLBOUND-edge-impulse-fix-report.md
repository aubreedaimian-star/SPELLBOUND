# SPELLBOUND Edge Impulse Arduino deployment fix

Date: 29 October 2026

## Result

The installed `MAGIC_inferencing` library now compiles successfully for `arduino:samd:mkrwifi1010` with Arduino SAMD core 1.8.14. The verification was compile-only; no firmware was uploaded.

- Flash: 90,716 / 262,144 bytes (34% used)
- Static SRAM: 5,976 / 32,768 bytes (18% used; 26,792 bytes remain)
- Final artifacts: `static_buffer.ino.elf` and `static_buffer.ino.bin`

## Root cause

`dsp/config.hpp` enables CMSIS-DSP whenever GCC reports an ARM M-profile target. The MKR WiFi 1010 is therefore routed through `ei_arm_cmsis_dsp.h` from `numpy.hpp`.

Arduino SAMD 1.8.14 loads CMSIS-Core 4.5.0 (`cmsis_gcc.h` v4.30) through the automatically inserted `Arduino.h` include. The Edge Impulse export bundles CMSIS-DSP 1.10 with a newer CMSIS-Core compiler header (`cmsis_gcc.h` v5.4.1). Both compiler headers use the `__CMSIS_GCC_H` include guard, so the old Arduino header prevents the newer bundled header from loading.

CMSIS-Core 4.5 for Cortex-M0+ defines `__STATIC_INLINE`, but not `__STATIC_FORCEINLINE`, `__SSAT`, or `__USAT`. CMSIS-DSP 1.10 expects those definitions. This caused the initial `__STATIC_FORCEINLINE` errors and, after a partial workaround, the `__SSAT` error.

Defining `EIDSP_USE_CMSIS_DSP=0` alone is insufficient because `numpy.hpp` unconditionally includes the CMSIS statistics header for downstream compatibility.

## Change

Changed one installed file:

`%USERPROFILE%\Documents\Arduino\libraries\MAGIC_inferencing\src\edge-impulse-sdk\CMSIS\DSP\Include\arm_math_types.h`

At lines 85-131, a SAMD/Cortex-M0+-only compatibility block now:

- Defines `__STATIC_FORCEINLINE` when the old core did not provide it.
- Supplies the portable-C signed and unsigned saturation behavior from newer CMSIS-Core for a target without SSAT/USAT instructions.
- Activates only when `ARDUINO_ARCH_SAMD` is defined and `__CORTEX_M == 0U`.

No ARM instruction unsupported by Cortex-M0+ was introduced. The CMSIS FFT path, EON compiled model, quantized weights, metadata, and example sketch remain intact.

## Protection and scope audit

The original 1,336-file, 21,643,679-byte library was copied before modification to:

`%USERPROFILE%\Documents\Codex\2026-10-29\spellbound-fix-edge-impulse-arduino-deployment\work\MAGIC_inferencing-original-20261029-1605`

After the fix, a complete SHA-256 comparison found exactly one changed file: `arm_math_types.h`. All five files under `src/model-parameters` and `src/tflite-model` are byte-identical to the backup. No Wi-Fi/MQTT sketch, Arduino core, toolchain package, or physical board was modified.

## Verification command

```powershell
& 'D:\Arduino\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe' `
  --config-dir '%USERPROFILE%\AppData\Local\Arduino15' `
  compile `
  --fqbn arduino:samd:mkrwifi1010 `
  --clean `
  --build-path '%USERPROFILE%\Documents\Codex\ei-finalbuild' `
  '%USERPROFILE%\Documents\Arduino\libraries\MAGIC_inferencing\examples\static_buffer\static_buffer'
```

CLI: Arduino CLI 1.5.1. Platform: Arduino SAMD 1.8.14. Compiler target flags included `-mcpu=cortex-m0plus -mthumb`.

## Model facts relevant to live MPU6050 integration

- Input window: 100 samples × 6 axes = 600 values
- Sampling interval: 20 ms (50 Hz)
- Window duration: 2 seconds
- Axis order: `accX, accY, accZ, gyroX, gyroY, gyroZ`
- DSP: spectral analysis, 16-point FFT, overlap enabled, 78 output features
- Inference: compiled EON graph, quantized int8 input/output
- Tensor arena metadata: 3,020 bytes
- Labels: Ascendio, ci, incendio, leviosa, oppugno, stupefy

## Remaining risks and next steps

1. This proves compilation and linking, not on-device numerical inference. The example currently has an empty feature array.
2. Re-exporting/reinstalling the Edge Impulse library will overwrite the local compatibility patch. Keep the backup and reapply the small block, or move the change into a reproducible patch step.
3. Capture MPU6050 data at exactly 50 Hz and preserve the trained axis order and units/scaling. A mismatch here will degrade accuracy even though the code compiles.
4. First run inference with a fixed known 600-value sample and compare probabilities with Edge Impulse Studio. Then replace the fixed sample with a 600-float circular buffer (about 2.4 KB) or an equivalent `signal_t` data source.
5. Measure stack headroom and inference latency on the board before adding Wi-Fi/MQTT. The compile reports 26,792 bytes free after static allocations, but runtime stack/heap and sensor/network buffers still consume that space.
6. Add confidence thresholding/debouncing and only then publish classified gestures to MQTT.

