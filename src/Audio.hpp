#pragma once

#include <miniaudio.h>

#include <atomic>
#include <cassert>
#include <optional>

namespace vi {
// RAII wrappers for miniaudio types

struct MAContext {
	MAContext();

	MAContext(const MAContext&) = delete;
	MAContext(const MAContext&&) = delete;

	~MAContext();

	MAContext& operator=(const MAContext&) = delete;
	MAContext& operator=(const MAContext&&) = delete;

	ma_context data;
};

struct MADevice {
	MADevice(MAContext& context, const ma_device_config& config);

	MADevice(const MADevice&) = delete;
	MADevice(const MADevice&&) = delete;

	~MADevice();

	MADevice& operator=(const MADevice&) = delete;
	MADevice& operator=(const MADevice&&) = delete;

	ma_device data;
};

struct MADecoder {
	MADecoder(const char* path, const ma_decoder_config* config);

	MADecoder(const MADevice&) = delete;
	MADecoder(const MADecoder&&) = delete;

	~MADecoder();

	MADecoder& operator=(const MADecoder&) = delete;
	MADecoder& operator=(const MADecoder&&) = delete;

	ma_decoder data;
};

struct PlaybackContext {
	std::optional<MADecoder> decoder;
	float gain = 1.0f;
	std::atomic_bool finished = false;
};

void dataCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount) noexcept;
const char* toString(ma_format format) noexcept;

} // namespace vi