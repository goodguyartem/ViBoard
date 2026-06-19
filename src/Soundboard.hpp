#pragma once

#include "Audio.hpp"
#include "hotkeys/HotkeyRegistry.hpp"

#include <GLFW/glfw3.h>

#include <yaml-cpp/yaml.h>

#include <imgui.h>

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace vi {
class App;

struct Sound {
	std::string path;
	std::optional<size_t> hotkeyId;
};

struct SoundboardTab {
	std::string name;
	std::filesystem::path path;
	std::unordered_map<std::string, Sound> sounds;
};

struct AudioDeviceInfo {
	ma_device_id id{};
	std::string name;
};

struct SelectedAudioDevice {
	size_t index = 0;
	std::optional<MADevice> device;
	PlaybackContext playback;
};

struct MessagePopups {
	static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
	std::string unexpected;
	std::string error;
	std::string info;
};

struct PendingBind {
	bool listening = false;
	bool ready = false;

	Hotkey newHotkey;
	std::optional<size_t>* id = nullptr;
};

class Soundboard {
public:
	Soundboard(const App& app);

	void loadTheme(std::filesystem::path path);
	void serialize(const std::filesystem::path& path) noexcept;
	void deserialize(const std::filesystem::path& path) noexcept;

	void update();
	void onKeyPress(int key, int action, int mods) noexcept;

private:
	void setAudioDevices() noexcept;

	void showTabs();
	void showProperties();

	void showPropertiesHeader() noexcept;
	void showOutputDevices();
	void showOutputDeviceDropdown(SelectedAudioDevice& selectedDevice, const char* label) noexcept;

	void showSoundOptions() noexcept;
	void showGeneralOptions() noexcept;

	void setUnexpectedError(std::string string) noexcept;
	void setError(std::string string) noexcept;
	void setInfo(std::string string) noexcept;

	void showUnexpectedErrorPopup() noexcept;
	void showBasicMessagePopup(const char* title, std::string& msg) noexcept;

	void showHotkeyBind() noexcept;
	void showPushToTalkBind() noexcept;

	void showHelp() noexcept;

	void addSoundTab();

	void play(const char* path) noexcept;
	void play(const char* path, SelectedAudioDevice& selectedDevice);
	void stop() noexcept;

	void updateDevice(SelectedAudioDevice& selected) const noexcept;

	void setPushToTalkIfEnabled(bool send) const noexcept;

	void serializeDevice(YAML::Node& yaml, const SelectedAudioDevice& selected);
	void deserializeDevice(const YAML::Node& yaml, SelectedAudioDevice& selected);

	void serializeTab(YAML::Node& yaml, const SoundboardTab& tab);
	void deserializeTab(const YAML::Node& yaml, SoundboardTab& tab);

	void findThemes() noexcept;
	void applyScale() const noexcept;

	const App* app;

	MAContext context;
	std::vector<AudioDeviceInfo> deviceInfos;
	std::pair<SelectedAudioDevice, SelectedAudioDevice> selectedDevices;
	std::optional<size_t> stopHotkey;
	bool dualPlayback = false;

	std::vector<SoundboardTab> tabs;
	bool sendPushToTalk = false;
	int pushToTalkKey = 0;
	bool pushToTalkKeyListening = false;

	std::filesystem::path theme;
	std::vector<std::filesystem::path> foundThemes;
	float scale = 1.0f;

	MessagePopups popups;
	bool bindHotkeyOpen = false;
	bool pushToTalkBindOpen = false;
	bool helpOpen = true;

	PendingBind pendingBind;
};

void populateSoundTab(SoundboardTab& tab);
void refreshSoundTab(SoundboardTab& tab, HotkeyRegistry& hotkeys);

} // namespace vi