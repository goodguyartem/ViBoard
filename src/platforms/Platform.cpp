#include "Platform.hpp"
#include "../Exceptions.hpp"
#include "nfd.h"

#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include <filesystem>

namespace fs = std::filesystem;

namespace vi {
fs::path pickFolder(const nfdnchar_t* defaultPath) {
	nfdnchar_t* raw = nullptr;
	nfdresult_t result = NFD_PickFolderN(&raw, defaultPath);

	if (result == NFD_ERROR) {
		throw ExternalError(NFD_GetError());
	}

	fs::path path;
	if (raw) {
		path = raw;
		NFD_FreePathN(raw);
	}
	return path;
}

fs::path getConfigDir() noexcept {
#if defined(_WIN32)
	if (const char* path = getenv("APPDATA")) {
		return path;
	}
#else
	const char* path = getenv("XDG_CONFIG_HOME");
	if (!path) {
		path = getenv("HOME");
	}
	if (path) {
		return path / fs::path(".config");
	}
#endif

	spdlog::error(
		"Failed to get config directory! getenv was unable to locate any valid paths. "
		"Defaulting to cwd instead."
	);
	return "";
}

fs::path getStateHomeDir() noexcept {
#if defined(_WIN32)
	const char* path = getenv("LOCALAPPDATA");
	if (path) {
		return path;
	}

	path = getenv("USERPROFILE");
	if (path) {
		return fs::path(path) / "AppData" / "Local";
	}

#else
	const char* path = std::getenv("XDG_STATE_HOME");
	if (path) {
		return path;
	}

	path = std::getenv("HOME");
	if (path) {
		return fs::path(path) / ".local" / "state";
	}
#endif

	spdlog::error(
		"Failed to get state home directory! getenv was unable to locate any valid paths. "
		"Defaulting to cwd instead."
	);
	return "";
}

bool createPipeWireVirtualDevices() noexcept {
	if (system(
			"pactl "
			"load-module module-null-sink "
			"media.class=Audio/Sink "
			"sink_name=ViBoard-Virtual-Mic "
			"channel_map=front-left,front-right"
		) != 0) {
		return false;
	}
	if (system(
			"pactl "
			"load-module module-null-sink "
			"media.class=Audio/Source/Virtual "
			"sink_name=ViBoard-Virtual-Mic-Source "
			"channel_map=front-left,front-right"
		) != 0) {
		return false;
	}
	if (system(
			"pw-link "
			"Virtual-Mic:monitor_FL "
			"Virtual-Mic-Source:input_FL"
		) != 0) {
		return false;
	}
	if (system(
			"pw-link "
			"Virtual-Mic:monitor_FR "
			"Virtual-Mic-Source:input_FR"
		) != 0) {
		return false;
	}

	return true;
}

bool createPulseAudioVirtualDevices() noexcept {
	if (system(
			"pactl "
			"load-module module-null-sink "
			"sink_name=ViBoard-Virtual-Sink "
			"sink_properties=device.description=ViBoard-Virtual-Sink"
		) != 0) {
		return false;
	}
	if (system(
			"pactl "
			"load-module module-virtual-source "
			"source_name=ViBoard-Virtual-Mic "
			"master=ViBoard-Virtual-Sink.monitor "
			"source_properties=device.description=ViBoard-Virtual-Mic"
		) != 0) {
		return false;
	}

	return true;
}

const char* getPlatformName() noexcept {
	const int platform = glfwGetPlatform();
	switch (platform) {
	case GLFW_PLATFORM_WIN32:	return "Win32";
	case GLFW_PLATFORM_COCOA:	return "macOS Cocoa";
	case GLFW_PLATFORM_X11:		return "Linux X11";
	case GLFW_PLATFORM_WAYLAND: return "Linux Wayland";
	default:					return "";
	}
}
} // namespace vi