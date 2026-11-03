
/*
 * Arduino SAMD 1.8.x ships CMSIS-Core 4.5.0. Arduino.h loads that core
 * before this bundled CMSIS-DSP 1.10 header, and the shared cmsis_gcc.h
 * include guard prevents the bundled CMSIS-Core 5 compiler header from
 * supplying the newer definitions below. Cortex-M0+ has no SSAT/USAT
 * instructions, so provide the same portable-C behavior used by CMSIS 5.
 */
#if defined(ARDUINO_ARCH_SAMD) && defined(__CORTEX_M) && (__CORTEX_M == 0U)
#ifndef __STATIC_FORCEINLINE
#define __STATIC_FORCEINLINE static inline __attribute__((always_inline))
#endif

#ifndef __SSAT
__STATIC_FORCEINLINE int32_t ei_cmsis_compat_ssat(int32_t value, uint32_t saturation)
{
    if ((saturation >= 1U) && (saturation <= 32U)) {
        const int32_t maximum = (int32_t)((1U << (saturation - 1U)) - 1U);
        const int32_t minimum = -1 - maximum;
        if (value > maximum) {
            return maximum;
        }
        if (value < minimum) {
            return minimum;
        }
    }
    return value;
}
#define __SSAT(value, saturation) ei_cmsis_compat_ssat((value), (saturation))
#endif

#ifndef __USAT
__STATIC_FORCEINLINE uint32_t ei_cmsis_compat_usat(int32_t value, uint32_t saturation)
{
    if (saturation <= 31U) {
        const uint32_t maximum = (1U << saturation) - 1U;
        if (value > (int32_t)maximum) {
            return maximum;
        }
        if (value < 0) {
            return 0U;
        }
    }
    return (uint32_t)value;
}
#define __USAT(value, saturation) ei_cmsis_compat_usat((value), (saturation))
#endif
#endif
