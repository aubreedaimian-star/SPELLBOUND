/*
 * SPELLBOUND known-sample hardware verification
 *
 * Source: Edge Impulse project 1149248 (MAGIC), testing sample 3199609377
 * Filename: incendio.760o197i
 * Label: incendio
 * SHA-256 reported by Edge Impulse:
 *   2c6f31bf37e41d9ced65be3e0bc46ba5331a43caac2618bfcab9b38e5d6b78a1
 *
 * The values below were retrieved unchanged through the read-only Edge Impulse
 * Studio API. Each row is ordered as:
 *   accX, accY, accZ, gyroX, gyroY, gyroZ
 * There are 100 rows sampled at 50 Hz (20 ms interval), for a 2.0 s window.
 */

#include <MAGIC_inferencing.h>

namespace {

constexpr uint32_t kSampleId = 3199609377UL;
constexpr char kSampleFilename[] = "incendio.760o197i";
constexpr char kExpectedLabel[] = "incendio";

static const float kFeatures[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = {
    -9.456f, 2.506f, -1.028f, -6.58f, -0.595f, 0.718f,
    -9.118f, 2.278f, -1.246f, -7.847f, -2.115f, -0.779f,
    -9.127f, 2.015f, -1.154f, -8.046f, -1.832f, -0.489f,
    -8.893f, 2.065f, -1.207f, -6.267f, -2.756f, 1.405f,
    -8.691f, 2.535f, -1.201f, -5.351f, -2.832f, 2.542f,
    -8.491f, 2.126f, -1.298f, -8.229f, -1.595f, 1.29f,
    -8.863f, 1.748f, -1.249f, -9.145f, 1.557f, 2.344f,
    -8.795f, 1.793f, -1.322f, -7.55f, 1.565f, 2.252f,
    -8.473f, 1.818f, -1.379f, -6.015f, 1.794f, -0.038f,
    -8.811f, 1.888f, -1.302f, -4.473f, 4.412f, 0.031f,
    -9.124f, 2.169f, -1.14f, -3.863f, 5.313f, 0.153f,
    -9.403f, 2.207f, -1.168f, -6.023f, 4.71f, -2.786f,
    -9.243f, 2.142f, -1.189f, -8.13f, 2.252f, -6.771f,
    -9.128f, 1.99f, -1.167f, -7.802f, 0.771f, -7.878f,
    -9.088f, 2.285f, -1.154f, -4.542f, 0.137f, -4.084f,
    -8.778f, 2.843f, -1.226f, -2.107f, -0.901f, 0.252f,
    -8.538f, 3.006f, -1.346f, -4.504f, 0.366f, 0.786f,
    -8.298f, 3.018f, -1.263f, -8.962f, 3.359f, 2.084f,
    -7.682f, 3.049f, -1.495f, -13.847f, 6.542f, 4.817f,
    -6.791f, 3.401f, -2.133f, -18.763f, 14.107f, 12.359f,
    -3.274f, 5.571f, -3.154f, -27.763f, 25.893f, 24.702f,
    1.708f, 8.039f, -6.035f, -57.16f, 65.137f, 41.458f,
    -8.054f, 4.158f, -6.37f, -104.511f, 183.191f, 79.962f,
    -19.613f, -2.163f, -4.83f, -133.672f, 250.046f, 89.885f,
    -19.613f, -16.33f, -2.283f, -141.008f, 207.389f, 45.26f,
    -19.613f, -17.338f, -4.003f, -46.069f, 52.542f, -19.901f,
    -19.613f, -6.369f, -6.178f, 88.542f, -37.198f, -12.374f,
    -19.613f, -2.782f, -6.444f, 134.824f, -56.748f, 6.397f,
    -19.613f, -5.096f, -7.186f, 128.527f, -70.74f, -43.427f,
    -19.613f, -4.772f, -8.106f, 140.924f, -108.122f, -112.305f,
    -19.613f, -3.752f, -10.205f, 165.038f, -161.015f, -124.87f,
    -19.613f, -1.997f, -12.268f, 182.756f, -212.031f, -139.099f,
    -19.613f, 0.456f, -14.658f, 202.252f, -250.137f, -178.817f,
    -19.613f, 2.824f, -16.069f, 224.099f, -250.137f, -208.733f,
    -16.342f, 3.183f, -16.184f, 245.038f, -250.137f, -205.618f,
    -3.111f, 11.856f, -15.773f, 250.13f, -250.137f, -169.931f,
    1.024f, 16.54f, -11.137f, 250.13f, -250.137f, -103.435f,
    4.69f, 9.887f, -5.428f, 189.863f, -249.031f, -30.679f,
    16.072f, 19.613f, -4.444f, 179.122f, -224.817f, -36.573f,
    5.465f, 19.613f, 1.207f, 95.931f, -59.901f, -27.481f,
    -2.059f, 9.822f, 1.429f, -24.16f, 17.992f, 50.802f,
    -1.182f, -0.048f, 0.071f, -64.58f, -50.336f, 72.969f,
    -0.113f, -2.602f, 0.172f, -31.107f, -69.962f, 86.748f,
    -0.178f, -1.622f, 0.335f, 8.901f, -69.45f, 60.885f,
    1.743f, 0.171f, 0.048f, 32.298f, -64.298f, 13.145f,
    2.218f, 1.417f, 0.442f, 40.901f, -37.656f, -7.718f,
    3.643f, 1.61f, 1.131f, 42.962f, -11.489f, 0.13f,
    4.337f, 1.261f, 1.763f, 44.893f, 17.053f, 22.107f,
    5.663f, 1.445f, 1.15f, 48.603f, 39.817f, 50.176f,
    3.423f, 0.141f, 0.613f, 49.183f, 82.382f, 81.328f,
    1.241f, -0.941f, -0.423f, 51.763f, 121.198f, 90.336f,
    -0.673f, -1.818f, -1.445f, 60.954f, 149.076f, 95.725f,
    -4.731f, -1.792f, -1.51f, 79.496f, 181.855f, 98.221f,
    -7.05f, -1.354f, -1.889f, 99.504f, 192.595f, 87.435f,
    -7.507f, -1.087f, -3.385f, 122.595f, 183.672f, 87.893f,
    -15.152f, 0.763f, -2.666f, 159.328f, 214.084f, 106.389f,
    -17.86f, 8.66f, -1.423f, 189.519f, 198.092f, 65.74f,
    -18.046f, 8.107f, -1.18f, 158.687f, 136.763f, 20.244f,
    -16.989f, 6.972f, -1.997f, 134.435f, 94.26f, 18.878f,
    -17.221f, 9.6f, -1.306f, 139.191f, 71.015f, 9.939f,
    -16.066f, 11.466f, -0.912f, 123.908f, 60.435f, -19.863f,
    -14.171f, 15.249f, -0.821f, 98.466f, 48.26f, -0.336f,
    -16.914f, 12.966f, -0.716f, 54.817f, 31.183f, 28.008f,
    -19.613f, 15.186f, 0.188f, 34.595f, 15.679f, 18.145f,
    -18.804f, 19.613f, 0.713f, 11.916f, -29.282f, -43.626f,
    -13.793f, 19.613f, -0.891f, -49.496f, -86.947f, -59.893f,
    -11.843f, 13.05f, -1.697f, -113.939f, -95.74f, -66.916f,
    -11.522f, 9.372f, -1.821f, -130.969f, -83.473f, -65.542f,
    -12.633f, 11.119f, -1.037f, -126.771f, -79.702f, -78.137f,
    -10.997f, 11.355f, -1.126f, -146.611f, -94.809f, -119.458f,
    -6.719f, 8.158f, -2.078f, -172.947f, -118.328f, -118.565f,
    -3.071f, 5.152f, -2.479f, -181.206f, -120.214f, -92.321f,
    -3.722f, 4.689f, -1.59f, -174.099f, -75.267f, -73.267f,
    -4.694f, 3.965f, -1.226f, -174.412f, -57.053f, -75.214f,
    -4.289f, 5.041f, -1.15f, -179.298f, -48.244f, -77.45f,
    -3.865f, 1.426f, -1.464f, -192.565f, -42.588f, -72.145f,
    -3.344f, 0.468f, -1.214f, -179.427f, -47.58f, -29.344f,
    -3.454f, 2.241f, -1.378f, -166.542f, -40.74f, -8.573f,
    -3.226f, 0.23f, -1.814f, -182.817f, -26.298f, -24.374f,
    -1.435f, -2.721f, -1.929f, -185.031f, -17.977f, -24.168f,
    0.117f, -3.437f, -2.193f, -158.527f, -9.473f, 15.573f,
    -1.494f, -3.045f, -2.377f, -120.878f, 55.206f, 47.397f,
    -3.384f, -3.089f, -2.041f, -111.282f, 90.008f, 31.237f,
    -5.635f, -1.404f, -1.759f, -97.458f, 110.695f, 51.534f,
    -10.23f, -1.14f, -1.39f, -99.71f, 128.099f, 77.176f,
    -11.735f, -2.212f, -1.629f, -114.405f, 122.427f, 54.542f,
    -12.894f, -5.637f, -1.506f, -126.031f, 118.443f, 12.275f,
    -15.353f, -8.109f, -1.183f, -110.084f, 99.374f, 19.481f,
    -17.951f, -6.667f, -0.753f, -77.969f, 78.481f, 34.504f,
    -14.309f, -6.142f, -1.171f, -61.779f, 39.45f, 18.252f,
    -13.418f, -6.796f, -0.788f, -47.672f, 30.695f, 0.26f,
    -14.785f, -6.208f, -0.42f, -25.084f, 27.664f, 3.511f,
    -14.268f, -4.014f, -0.178f, -0.565f, 12.496f, 10.229f,
    -12.893f, -2.497f, -0.041f, 6.763f, -1.847f, 8.458f,
    -11.948f, -3.038f, -0.071f, 1.099f, -7.0f, -9.901f,
    -11.37f, -2.608f, -0.195f, 5.901f, -9.565f, -16.511f,
    -11.217f, -0.508f, 0.135f, 16.626f, -13.985f, 0.611f,
    -10.806f, 0.26f, 0.233f, 9.504f, -16.221f, 0.26f,
    -9.333f, -1.499f, 0.03f, -6.489f, -17.687f, -22.328f,
    -9.494f, -1.7f, -0.04f, -2.29f, -16.26f, -21.244f
};

static_assert(EI_CLASSIFIER_RAW_SAMPLE_COUNT == 100,
              "This verification sample requires a 100-row model window");
static_assert(EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME == 6,
              "This verification sample requires six axes per row");
static_assert(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE == 600,
              "This verification sample requires exactly 600 model inputs");
static_assert(EI_CLASSIFIER_INTERVAL_MS == 20,
              "This verification sample was captured at a 20 ms interval");
static_assert(sizeof(kFeatures) / sizeof(kFeatures[0]) ==
                  EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE,
              "Known-sample feature count does not match the model input");

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    if (out_ptr == nullptr || offset > EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE ||
        length > EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - offset) {
        return -1;
    }

    memcpy(out_ptr, kFeatures + offset, length * sizeof(float));
    return 0;
}

void print_result(const ei_impulse_result_t &result) {
    ei_printf("Timing (ms): DSP %d, inference %d, anomaly %d\r\n",
              result.timing.dsp,
              result.timing.classification,
              result.timing.anomaly);

    ei_printf("Predictions:\r\n");
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; ++i) {
        ei_printf("  %s: %.6f\r\n",
                  ei_classifier_inferencing_categories[i],
                  result.classification[i].value);
    }

#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("Anomaly: %.6f\r\n", result.anomaly);
#endif
}

void run_known_sample_test() {
    ei_printf("\r\nSPELLBOUND Edge Impulse known-sample test\r\n");
    ei_printf("Project: %s / %s (ID %lu, deployment v%lu)\r\n",
              EI_CLASSIFIER_PROJECT_OWNER,
              EI_CLASSIFIER_PROJECT_NAME,
              static_cast<unsigned long>(EI_CLASSIFIER_PROJECT_ID),
              static_cast<unsigned long>(EI_CLASSIFIER_PROJECT_DEPLOY_VERSION));
    ei_printf("Sample: %s (ID %lu, expected label: %s)\r\n",
              kSampleFilename,
              static_cast<unsigned long>(kSampleId),
              kExpectedLabel);
    ei_printf("Input: %lu values = %lu rows x %lu axes, %.1f Hz, %.0f ms\r\n",
              static_cast<unsigned long>(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE),
              static_cast<unsigned long>(EI_CLASSIFIER_RAW_SAMPLE_COUNT),
              static_cast<unsigned long>(EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME),
              static_cast<double>(EI_CLASSIFIER_FREQUENCY),
              static_cast<double>(EI_CLASSIFIER_RAW_SAMPLE_COUNT *
                                  EI_CLASSIFIER_INTERVAL_MS));
    ei_printf("Axis order: accX, accY, accZ, gyroX, gyroY, gyroZ\r\n");

    signal_t signal;
    signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    signal.get_data = &raw_feature_get_data;

    ei_impulse_result_t result = {};
    const EI_IMPULSE_ERROR error = run_classifier(&signal, &result, false);
    ei_printf("run_classifier returned: %d\r\n", error);

    if (error != EI_IMPULSE_OK) {
        ei_printf("ERROR: inference failed; no prediction values are valid.\r\n");
        return;
    }

    print_result(result);
    ei_printf("Test complete. Reset the board to run it again.\r\n");
}

}  // namespace

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    run_known_sample_test();
}

void loop() {
    // One-shot verification: deliberately do not repeat inference.
}
