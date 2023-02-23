#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

    (void)pInput;
}

int main(int argc, char** argv)
{
    ma_result result;
    ma_decoder decoder;
    ma_device_config deviceConfig;
    ma_device device;

    if (argc < 2) {
        printf("No input file.\n");
        return -1;
    }

    ma_context context;
    ma_context_config contextConfig = ma_context_config_init();
    contextConfig.threadPriority = ma_thread_priority_highest;
    context.backend = ma_backend_wasapi;

    if (ma_context_init(NULL, 0, &contextConfig, &context) != MA_SUCCESS) {
        // Error.
    }

    ma_device_info* pPlaybackInfos;
    ma_uint32 playbackCount;
    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;
    if (ma_context_get_devices(&context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) != MA_SUCCESS) {
        // Error.
    }
    
    ma_decoder_config config;
    config = ma_decoder_config_init(ma_format_f32, 2, 192000);
    printf("sample rate: %d\n", config.sampleRate);
    config.ditherMode = ma_dither_mode_none;
    config.resampling = ma_resampler_config_init(ma_format_s24, 2, 192000, 192000, ma_resample_algorithm_linear);

    result = ma_decoder_init_file(argv[1], &config, &decoder);
    if (result != MA_SUCCESS) {
        printf("Could not load file: %s\n", argv[1]);
        return -2;
    }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format            = ma_format_f32;
    deviceConfig.playback.channels          = 2;
    deviceConfig.sampleRate                 = 192000;
    deviceConfig.dataCallback               = data_callback;
    deviceConfig.wasapi.usage               = ma_wasapi_usage_pro_audio;
    deviceConfig.playback.shareMode         = ma_share_mode_exclusive;
    deviceConfig.pUserData                  = &decoder;
    deviceConfig.performanceProfile         = ma_performance_profile_low_latency;
    deviceConfig.noClip                     = MA_TRUE;
    deviceConfig.noPreSilencedOutputBuffer  = MA_TRUE;

    if (ma_device_init(&context, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&decoder);
        return -3;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return -4;
    }

    printf("Press Enter to quit...");
    getchar();

    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);

    return 0;
}
