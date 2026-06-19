#define MINIAUDIO_IMPLEMENTATION
#include "Audio.hpp"
#include "Exceptions.hpp"

#include <spdlog/spdlog.h>

#define STB_VORBIS_IMPLEMENTATION
#include <stb_vorbis.c>

namespace vi {
MAContext::MAContext() {
	if (ma_context_init(nullptr, 0, nullptr, &data) != MA_SUCCESS) {
		throw ExternalError("Failed to create miniaudio context.");
	}
}

MAContext::~MAContext() {
	ma_context_uninit(&data);
}

MADevice::MADevice(MAContext& context, const ma_device_config& config) {
	if (ma_device_init(&context.data, &config, &data) != MA_SUCCESS) {
		throw ExternalError("Failed to create miniaudio device.");
	}
}

MADevice::~MADevice() {
	ma_device_uninit(&data);
}

MADecoder::MADecoder(const char* path, const ma_decoder_config* config) {
	if (ma_decoder_init_file(path, config, &data) != MA_SUCCESS) {
		throw ExternalError("Failed to create miniaudio decoder.");
	}
}

MADecoder::~MADecoder() {
	ma_decoder_uninit(&data);
}

void dataCallback(ma_device* device, void* output, const void* /*input*/, ma_uint32 frameCount) noexcept {
	PlaybackContext* playback = static_cast<PlaybackContext*>(device->pUserData);
	assert(playback && playback->decoder);

	ma_uint64 framesRead;
	ma_decoder_read_pcm_frames(&playback->decoder->data, output, frameCount, &framesRead);

	if (framesRead == 0) {
		playback->finished = true;
	}

	if (playback->gain != 1.0f) {
		float* samples = static_cast<float*>(output);
		const ma_uint64 sampleCount = framesRead * device->playback.channels;

		for (ma_uint64 i = 0; i < sampleCount; i++) {
			samples[i] *= playback->gain;
		}
	}

	spdlog::debug(
		"Read {} PCM frames: format: {}, channels: {}, sample rate: {}",
		framesRead,
		toString(device->playback.format),
		device->playback.channels,
		device->sampleRate
	);
}

const char* toString(ma_format format) noexcept {
	switch (format) {
	case ma_format_unknown:
		return "unknown";
	case ma_format_u8:
		return "u8";
	case ma_format_s16:
		return "s16";
	case ma_format_s24:
		return "s24";
	case ma_format_s32:
		return "s32";
	case ma_format_f32:
		return "f32";
	default:
		assert(false);
		break;
	}
}
} // namespace vi