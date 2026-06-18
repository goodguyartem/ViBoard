#include "Soundboard.hpp"
#include "App.hpp"
#include "Audio.hpp"
#include "Exceptions.hpp"
#include "GLFW/glfw3.h"
#include "Paths.hpp"
#include "Project.hpp"
#include "Util.hpp"
#include "Window.hpp"
#include "YamlTypes.hpp"
#include "hotkeys/HotkeyRegistry.hpp"
#include "platforms/Platform.hpp"

#include <cstdlib>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include <yaml-cpp/null.h>

#include <spdlog/spdlog.h>

#include <cassert>
#include <exception>
#include <format>
#include <fstream>

namespace fs = std::filesystem;

namespace vi {
Soundboard::Soundboard(const App& app)
	: app(&app) {

	setAudioDevices();
}

void Soundboard::loadTheme(fs::path path) {
	vi::loadTheme(YAML::LoadFile(path), ImGui::GetStyle());
	theme = std::move(path);
}

void Soundboard::serialize(const fs::path& path) noexcept {
	try {
		YAML::Node config;

		ImVec2i windowSize;
		glfwGetWindowSize(app->getWindow(), &windowSize.x, &windowSize.y);
		config["windowSize"] = windowSize;

		ImVec2i windowPos;
		glfwGetWindowPos(app->getWindow(), &windowPos.x, &windowPos.y);
		config["windowPos"] = windowPos;

		const int maximized = glfwGetWindowAttrib(app->getWindow(), GLFW_MAXIMIZED);
		config["maximized"] = maximized == GLFW_TRUE;

		auto tabsNode = config["tabs"];
		for (const SoundboardTab& tab : tabs) {
			YAML::Node node;
			serializeTab(node, tab);
			tabsNode.push_back(node);
		}

		YAML::Node dev0;
		serializeDevice(dev0, selectedDevices.first);
		config["dev0"] = dev0;

		YAML::Node dev1;
		serializeDevice(dev1, selectedDevices.second);
		config["dev1"] = dev1;

		config["dualPlayback"] = dualPlayback;
		config["sendPushToTalk"] = sendPushToTalk;
		config["pushToTalkKey"] = pushToTalkKey;

		if (theme.empty()) {
			config["theme"] = YAML::Null;
		} else {
			config["theme"] = theme.string();
		}
		config["scale"] = scale;
		config["helpOpen"] = helpOpen;

		if (stopHotkey) {
			const Hotkey& hotkey = app->getHotkeyRegistry().getHotkey(*stopHotkey);
			YAML::Node node = config["stopHotkey"];
			node["key"] = hotkey.key;
			node["mods"] = uint32_t(hotkey.mods);
		}

		std::ofstream stream(path, std::ofstream::trunc);
		stream.exceptions(std::ofstream::failbit);

		stream << config;
	} catch (std::exception& e) {
		setUnexpectedError(std::format("Failed to serialize: {}", e.what()));
	}
}

void Soundboard::deserialize(const std::filesystem::path& path) noexcept {
	try {
		YAML::Node config = YAML::LoadFile(path);

		bool maximize = false;
		if (const auto node = config["maximized"]) {
			maximize = node.as<bool>();
		}

		if (const auto node = config["windowPos"]) {
			const ImVec2i windowPos = node.as<ImVec2i>();
			glfwSetWindowPos(app->getWindow(), windowPos.x, windowPos.y);
		}

		if (maximize) {
			glfwMaximizeWindow(app->getWindow());
		} else if (const auto node = config["windowSize"]) {
			const ImVec2i windowSize = node.as<ImVec2i>();
			glfwSetWindowSize(app->getWindow(), windowSize.x, windowSize.y);
		}

		if (const auto node = config["tabs"]) {
			for (size_t i = 0; i < node.size(); i++) {
				SoundboardTab& tab = tabs.emplace_back();
				deserializeTab(node[i], tab);
			}
		}

		if (const auto node = config["dev0"]) {
			deserializeDevice(node, selectedDevices.first);
		}
		if (const auto node = config["dev1"]) {
			deserializeDevice(node, selectedDevices.second);
		}

		if (const auto node = config["dualPlayback"]) {
			dualPlayback = node.as<bool>();
		}
		if (const auto node = config["sendPushToTalk"]) {
			sendPushToTalk = node.as<bool>();
		}
		if (const auto node = config["pushToTalkKey"]) {
			pushToTalkKey = node.as<int>();
		}

		if (const auto node = config["theme"]) {
			if (node.IsNull()) {
				theme.clear();
			} else {
				loadTheme(node.as<std::string>());
			}
		}
		if (const auto node = config["scale"]) {
			scale = node.as<int>();
			applyScale();
		}

		if (const auto node = config["helpOpen"]) {
			helpOpen = node.as<bool>();
		}

		if (const auto node = config["stopHotkey"]) {
			Hotkey hotkey;
			hotkey.key = node["key"].as<uint8_t>();
			hotkey.mods = node["mods"].as<int>();
			hotkey.callback = [this] { stop(); };

			stopHotkey = app->getHotkeyRegistry().registerHotkey(hotkey);
		}
	} catch (std::exception& e) {
		setUnexpectedError(std::format("Failed to deserialize: {}", e.what()));
	}
}

void Soundboard::update() {
	showProperties();
	showTabs();

	if (bindHotkeyOpen) {
		showHotkeyBind();
	}
	if (pushToTalkBindOpen) {
		showPushToTalkBind();
	}
	if (helpOpen) {
		showHelp();
	}

	if (!popups.unexpected.empty()) {
		showUnexpectedErrorPopup();
	}
	if (!popups.error.empty()) {
		showBasicMessagePopup("Error!", popups.error);
	}
	if (!popups.info.empty()) {
		showBasicMessagePopup("Info", popups.info);
	}

	updateDevice(selectedDevices.first);
	updateDevice(selectedDevices.second);
}

void Soundboard::onKeyPress(int key, int action, int mods) noexcept {
	if (pendingBind.listening) {
		static Hotkey& newHotkey = pendingBind.newHotkey;
		newHotkey.key = key;
		newHotkey.mods = mods;

		if ((key >= GLFW_KEY_SPACE && (key < GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_MENU)) || action == GLFW_RELEASE) {
			if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) {
				newHotkey.mods &= ~GLFW_MOD_CONTROL;
			}
			if (key == GLFW_KEY_LEFT_SUPER || key == GLFW_KEY_RIGHT_SUPER) {
				newHotkey.mods &= ~GLFW_MOD_SUPER;
			}
			if (key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT) {
				newHotkey.mods &= ~GLFW_MOD_ALT;
			}
			if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
				newHotkey.mods &= ~GLFW_MOD_SHIFT;
			}

			pendingBind.listening = false;
			pendingBind.ready = true;
		}
	} else if (pushToTalkKeyListening) {
		pushToTalkKey = key;
		pushToTalkKeyListening = false;
	}
}

void Soundboard::setAudioDevices() noexcept {
	deviceInfos.clear();

	ma_device_info* info;
	ma_uint32 count;

	const ma_result result = ma_context_get_devices(&context.data, &info, &count, nullptr, nullptr);
	if (result != MA_SUCCESS) {
		setUnexpectedError("Failed to retrieve audio devices: ma_context_get_devices didn't return MA_SUCCESS.");
		return;
	}

	deviceInfos.reserve(count);
	for (ma_uint32 i = 0; i < count; i++) {
		deviceInfos.emplace_back(info[i].id, info[i].name);
	}
}

void Soundboard::showTabs() {
	static constexpr ImVec2 soundButtonSize(224, 56);
	const ImFontAtlas& atlas = *ImGui::GetIO().Fonts;

	bool allowWindowContextMenu = true;
	for (auto it = tabs.begin(); it != tabs.end(); it++) {
		ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_FirstUseEver);
		bool open = true;

		if (ImGui::Begin(it->name.c_str(), &open)) {
			ImGui::PushFont(atlas.Fonts[font::h1]);
			ImGui::Text("%s", it->name.c_str());

			ImGui::Separator();
			ImGui::PopFont();

			ImGui::NewLine();

			const int columnCount =
				std::max(1, static_cast<int>((ImGui::GetWindowContentRegionMax().x - 24) / soundButtonSize.x));

			int column = 0;

			if (ImGui::BeginTable("sounds", columnCount)) {
				for (auto& [name, sound] : it->sounds) {
					const std::string hotkeyStr =
						(sound.hotkeyId ? toString(app->getHotkeyRegistry().getHotkey(*sound.hotkeyId)) : "");

					const std::string& label = (hotkeyStr.empty() ? name : std::format("{}\n({})", name, hotkeyStr));

					ImGui::TableNextColumn();

					ImGui::PushFont(atlas.Fonts[font::bold]);
					if (ImGui::Button(label.c_str(), soundButtonSize)) {
						play(sound.path.c_str());
					}
					ImGui::PopFont();

					if (ImGui::BeginPopupContextItem()) {
						allowWindowContextMenu = false;
						if (ImGui::MenuItem("Bind hotkey")) {
							bindHotkeyOpen = true;
							pendingBind.id = &sound.hotkeyId;
							pendingBind.newHotkey.callback = [this, sound]() { play(sound.path.c_str()); };
						}
						ImGui::EndPopup();
					}
					if (!open && sound.hotkeyId) {
						app->getHotkeyRegistry().unregisterHotkey(*sound.hotkeyId);
					}

					if (++column >= columnCount) {
						ImGui::TableNextRow();
						column = 0;
					}
				}

				ImGui::EndTable();
			}

			if (allowWindowContextMenu && ImGui::BeginPopupContextWindow()) {
				if (ImGui::MenuItem("Refresh")) {
					refreshSoundTab(*it, app->getHotkeyRegistry());
				}
				if (ImGui::MenuItem("Reveal in file browser")) {
					openInFileBrowser(it->path);
				}
				ImGui::EndPopup();
			}
		}
		ImGui::End();

		if (!open) {
			it = tabs.erase(it);
			if (it == tabs.end()) {
				break;
			}
		}
	}
}

void Soundboard::showProperties() {
	if (ImGui::Begin("ViBoard Properties", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::PushTextWrapPos(ImGui::GetWindowWidth() - 16.0f);

		showPropertiesHeader();
		ImGui::NewLine();

		showOutputDevices();
		ImGui::NewLine();

		showSoundOptions();
		ImGui::NewLine();

		showGeneralOptions();
	}
	ImGui::End();
}

void Soundboard::showPropertiesHeader() noexcept {
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[font::h1]);
	ImGui::SeparatorText("ViBoard");
	ImGui::PopFont();

	if (ImGui::Button("New soundboard")) {
		addSoundTab();
	}

	ImGui::SameLine();
	if (ImGui::Button("Help")) {
		helpOpen = true;
	}

	ImGui::SameLine();
	if (ImGui::Button("Git")) {
		openUrl("https://github.com/goodguyartem/ViBoard");
	}
}

void Soundboard::showOutputDevices() {
	if (!ImGui::CollapsingHeader("Output devices", ImGuiTreeNodeFlags_DefaultOpen)) {
		return;
	}

	if (deviceInfos.empty()) {
		ImGui::Text("No output devices found.");
	} else {
		showOutputDeviceDropdown(selectedDevices.first, "Device 1");
		if (dualPlayback) {
			showOutputDeviceDropdown(selectedDevices.second, "Device 2");
		}
	}

	if (ImGui::Button("Refresh")) {
		setAudioDevices();
	}
	ImGui::Checkbox("Allow playback through multiple devices", &dualPlayback);
}

void Soundboard::showOutputDeviceDropdown(SelectedAudioDevice& selectedDevice, const char* label) noexcept {
	assert(selectedDevice.index < deviceInfos.size());

	const char* preview = deviceInfos[selectedDevice.index].name.c_str();
	if (ImGui::BeginCombo(label, preview)) {
		for (size_t i = 0; i < deviceInfos.size(); i++) {
			const bool selected = i == selectedDevice.index;

			if (ImGui::Selectable(deviceInfos[i].name.c_str(), selected)) {
				selectedDevice.index = i;
			}
			if (selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}
}

void Soundboard::showSoundOptions() noexcept {
	if (!ImGui::CollapsingHeader("Soundboards", ImGuiTreeNodeFlags_DefaultOpen)) {
		return;
	}

	ImGui::SliderFloat("Device 1 volume", &selectedDevices.first.playback.gain, 0.0f, 2.0f, "%.2f");
	if (dualPlayback) {
		ImGui::SliderFloat("Device 2 volume", &selectedDevices.second.playback.gain, 0.0f, 2.0f, "%.2f");
	}

	ImGui::BeginDisabled(!selectedDevices.first.device.has_value());
	const std::string hotkeyStr = stopHotkey ? toString(app->getHotkeyRegistry().getHotkey(*stopHotkey)) : "Unbound";
	const std::string stopLabel = std::format("Stop ({})", hotkeyStr);

	if (ImGui::Button(stopLabel.c_str())) {
		stop();
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	if (ImGui::Button("Bind hotkey")) {
		pendingBind.newHotkey.callback = [this]() { stop(); };
		pendingBind.id = &stopHotkey;
		bindHotkeyOpen = true;
	}
	ImGui::NewLine();

	ImGui::Checkbox("Trigger push-to-talk", &sendPushToTalk);
	ImGui::Text("Set a key to send out when a sound effect is played to trigger your game's push-to-talk.");
	ImGui::NewLine();

	if (sendPushToTalk) {
		if (ImGui::Button(pushToTalkKey != 0 ? keyToString(pushToTalkKey) : "None")) {
			pushToTalkBindOpen = true;
		}
		ImGui::SameLine();
		ImGui::Text("Push-to-talk key");
	}
}

void Soundboard::showGeneralOptions() noexcept {
	if (!ImGui::CollapsingHeader("General", ImGuiTreeNodeFlags_DefaultOpen)) {
		return;
	}

	std::string preview = !theme.empty() ? theme.stem().string() : "None";
	if (ImGui::BeginCombo("Theme", preview.c_str())) {
		if (foundThemes.empty()) {
			findThemes();
		}
		if (ImGui::Selectable("None")) {
			ImGui::GetStyle() = ImGuiStyle();
			theme.clear();
		}
		for (const fs::path& path : foundThemes) {
			if (!ImGui::Selectable(path.stem().string().c_str())) {
				continue;
			}
			loadTheme(path);
		}
		ImGui::EndCombo();
	} else {
		foundThemes.clear();
	}

	if (ImGui::Button("Open folder")) {
		openInFileBrowser(themesDir);
	}

	if (ImGui::SliderInt("UI Scale", &scale, 1, 3)) {
		applyScale();
	}
	ImGui::NewLine();

	if (ImGui::Button("Open log folder")) {
		openInFileBrowser(stateDir);
	}
}

void Soundboard::setUnexpectedError(std::string string) noexcept {
	spdlog::error("{}", string);
	popups.unexpected = std::move(string);
}

void Soundboard::setError(std::string string) noexcept {
	spdlog::error("{}", string);
	popups.error = std::move(string);
}

void Soundboard::setInfo(std::string string) noexcept {
	spdlog::info("{}", string);
	popups.info = std::move(string);
}

void Soundboard::showUnexpectedErrorPopup() noexcept {
	const char* title = "Unexpected Error!";
	if (!ImGui::IsPopupOpen(title)) {
		ImGui::OpenPopup(title);
	}

	bool open = true;
	if (ImGui::BeginPopupModal(title, &open, MessagePopups::flags)) {
		ImGui::Text("An error has occurred.");
		ImGui::Text("%s", popups.unexpected.c_str());

		ImGui::NewLine();
		ImGui::Text("If you didn't see this coming, please open an issue on the Github page.");

		if (ImGui::Button("Awww, man")) {
			open = false;
		}
		ImGui::EndPopup();
	}
	if (!open) {
		popups.unexpected.clear();
	}
}

void Soundboard::showBasicMessagePopup(const char* title, std::string& msg) noexcept {
	if (!ImGui::IsPopupOpen(title)) {
		ImGui::OpenPopup(title);
	}

	bool open = true;
	if (ImGui::BeginPopupModal(title, &open, MessagePopups::flags)) {
		ImGui::Text("%s", msg.c_str());

		ImGui::NewLine();
		if (ImGui::Button("Okay")) {
			open = false;
		}
		ImGui::EndPopup();
	}
	if (!open) {
		msg.clear();
	}
}

void Soundboard::showHotkeyBind() noexcept {
	static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking;
	if (ImGui::Begin("Hotkey Bind", &bindHotkeyOpen, flags)) {
		assert(pendingBind.id);

		ImGui::Text("Assign a hotkey to trigger the action with.");
		ImGui::NewLine();

		ImGui::Text("Note that some programs, especially games, may restrict the use of hotkeys.");
		ImGui::Text("Changing the game's input method, if allowed, may help.");
		ImGui::Text("Your OS may also have reserved certain key combinations for internal use.");
		ImGui::NewLine();

		ImGui::Text("Assigned hotkey:");

		if (pendingBind.listening) {
			if (pendingBind.newHotkey.key == 0 && pendingBind.newHotkey.mods == 0) {
				ImGui::Text("Press a key combination, or Delete to unbind.");
			} else {
				ImGui::Text("%s", toString(pendingBind.newHotkey).c_str());
			}
		} else if (*pendingBind.id) {
			const Hotkey& existing = app->getHotkeyRegistry().getHotkey(**pendingBind.id);
			ImGui::Text("%s", toString(existing).c_str());
		} else {
			ImGui::Text("Unbound");
		}
		ImGui::NewLine();

		if (ImGui::Button("Close")) {
			bindHotkeyOpen = false;
		}

		ImGui::SameLine();
		if (ImGui::Button("Set")) {
			if (*pendingBind.id) {
				app->getHotkeyRegistry().unregisterHotkey(**pendingBind.id);
				pendingBind.id->reset();
			}

			pushToTalkKeyListening = false;
			pendingBind.listening = true;
			pendingBind.ready = false;

			pendingBind.newHotkey.key = 0;
			pendingBind.newHotkey.mods = 0;
		}

		if (pendingBind.ready) {
			if (pendingBind.newHotkey.key != GLFW_KEY_DELETE) {
				*pendingBind.id = app->getHotkeyRegistry().registerHotkey(pendingBind.newHotkey);
				if (!*pendingBind.id) {
					setError("Failed to assign hotkey! It may already be in use by your OS.");
				}
			}
			pendingBind.ready = false;
		}
	}
	ImGui::End();

	if (!bindHotkeyOpen) {
		pendingBind.listening = false;
	}
}

void Soundboard::showPushToTalkBind() noexcept {
	static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize;
	if (ImGui::Begin("Push-To-Talk", &pushToTalkBindOpen, flags)) {
		ImGui::Text("Assign a key to send out everytime a sound effect is played.");
		ImGui::NewLine();

		ImGui::Text("This may not work with every program, especially games.");
		ImGui::Text("Changing the game's input method, if allowed, may help.");
		ImGui::Text("Use caution when using with anti-cheat-protected games.");
		ImGui::NewLine();

		ImGui::Text("Current key:");
		if (pushToTalkKeyListening) {
			ImGui::Text("Press a key to bind.");
		} else if (pushToTalkKey != 0) {
			ImGui::Text("%s", keyToString(pushToTalkKey));
		} else {
			ImGui::Text("None");
		}
		ImGui::NewLine();

		if (ImGui::Button("Close")) {
			pushToTalkBindOpen = false;
		}
		ImGui::SameLine();

		if (ImGui::Button("Set")) {
			pendingBind.listening = false;
			pushToTalkKeyListening = true;
		}
	}
	ImGui::End();
}

void Soundboard::showHelp() noexcept {
	static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;

	const ImFontAtlas& atlas = *ImGui::GetIO().Fonts;
	ImGui::PushFont(atlas.Fonts[font::larger]);

	if (ImGui::Begin("Help", &helpOpen, flags)) {
		ImGui::PushFont(atlas.Fonts[font::h1]);
		ImGui::Text("ViBoard");
		ImGui::PopFont();

		ImGui::Text("Fast Free & Open-Source soundboard by goodguyartem.");
		ImGui::Text("v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
		ImGui::NewLine();

		ImGui::PushFont(atlas.Fonts[font::h2]);
		ImGui::SeparatorText("Getting Started");
		ImGui::PopFont();

		ImGui::Text(
			"Use the properties menu to create a new soundboard by picking a folder that contains your sound effects."
		);
		ImGui::NewLine();

		ImGui::Text(
			"You can select up to 2 output devices to simultaneously play the sound effects with.\n"
			"If you wish to have them be played as a microphone input, you need to setup a virtual input device, and a "
			"virtual output device\n"
			"that sends all incoming output to the virtual microphone device."
		);
		ImGui::NewLine();

#if defined(__linux__)
		if (ImGui::Button("Create PipeWire devices")) {
			if (createPipeWireVirtualDevices()) {
				setInfo("Created virtual PipeWire devices. If they don't appear, please manually configure PipeWire.");
				setAudioDevices();
			} else {
				setError("Failed to create virtual PipeWire devices. Please configure PipeWire manually.");
			}
		}
		ImGui::SameLine();

		if (ImGui::Button("Create PulseAudio devices")) {
			if (createPulseAudioVirtualDevices()) {
				setInfo(
					"Created virtual PulseAudio devices. If they don't appear, please manually configure PulseAudio."
				);
				setAudioDevices();
			} else {
				setError("Failed to create virtual PulseAudio devices. Please configure PulseAudio manually.");
			}
		}
		ImGui::NewLine();

		ImGui::Text(
			"If you would like to mix your microphone's input with the virtual devices,\n"
			"you can use a tool like qpwgraph from your package manager to map your microphone to your virtual output "
			"device."
		);
		ImGui::NewLine();

		if (ImGui::Button("Get qpwgraph")) {
			openUrl("https://github.com/rncbc/qpwgraph");
		}
		ImGui::NewLine();

#elif defined(_WIN32)
		ImGui::Text(
			"Windows does not have a built-in way to do this so you must rely on third-party drivers.\n"
			"One such driver is VB-CABLE you can download for free."
		);
		ImGui::NewLine();

		if (ImGui::Button("Download VB-Cable")) {
			openUrl("https://vb-audio.com/Cable/");
		}
		ImGui::NewLine();

		ImGui::Text(
			"If you would like to mix your microphone's input with the virtual devices,\n"
			"you can go to Windows Settings > System > Sound, and then click \"More sound settings\" near the bottom.\n"
			"In the Sound pop-up window, click on the \"Recording\" tab, then double-click your preferred microphone.\n"
			"Under the \"Listen\" tab, tick \"Listen to this device\", and select VB-Audio as the output device."
		);
		ImGui::NewLine();

		if (ImGui::Button("Open sound settings")) {
			openUrl("ms-settings:sound");
		}
		ImGui::NewLine();
#endif

		ImGui::PushFont(atlas.Fonts[font::h2]);
		ImGui::SeparatorText("Customizing");
		ImGui::PopFont();

		ImGui::Text(
			"You can drag windows by the title bar to reposition or pin them to a location. Most windows can also be "
			"resized to your liking."
		);
		ImGui::NewLine();

		ImGui::Text(
			"Right-click on a sound effect to bind a hotkey to it. You can enable Trigger Push-To-Talk from the "
			"properties window to send\n"
			"your push-to-talk key everytime a sound is played.\n"
			"These features may not work in every program, especially if it's a game that uses direct input or is "
			"anti-cheat protected.\n"
			"Use with caution!"
		);
		ImGui::NewLine();

		ImGui::PushFont(atlas.Fonts[font::h2]);
		ImGui::SeparatorText("Enjoy!");
		ImGui::PopFont();

		ImGui::Text("See the Github page for latest updates and to report any bugs! Contributions are welcomed.");
		ImGui::NewLine();

		if (ImGui::Button("Got it!")) {
			helpOpen = false;
		}
		ImGui::NewLine();
	}

	ImGui::End();
	ImGui::PopFont();
}

void Soundboard::addSoundTab() {
	fs::path path = pickFolder();
	if (path.empty()) {
		return;
	}

	std::string name = path.filename().string();
	ImGuiWindow* window = ImGui::FindWindowByName(path.filename().string().c_str());
	if (window && (window->Active || window->WasActive)) {
		setInfo(std::format("A soundboard with the name \"{}\" already exists!", name));
		return;
	}

	auto& tab = tabs.emplace_back();
	tab.name = std::move(name);
	tab.path = std::move(path);

	populateSoundTab(tab);
}

void Soundboard::play(const char* path) noexcept {
	try {
		if (selectedDevices.first.index < deviceInfos.size()) {
			play(path, selectedDevices.first);
		}
		if (dualPlayback && selectedDevices.second.index < deviceInfos.size()) {
			play(path, selectedDevices.second);
		}
		setPushToTalkIfEnabled(true);
	} catch (const ExternalError& e) {
		setUnexpectedError(std::format("Playback error: {} (does the file still exist?)", e.what()));
	}
}

void Soundboard::play(const char* path, SelectedAudioDevice& selectedDevice) {
	assert(selectedDevice.index < deviceInfos.size());

	if (selectedDevice.device) {
		ma_device_stop(&selectedDevice.device->data);
		selectedDevice.device.reset();
		selectedDevice.playback.decoder.reset();

		setPushToTalkIfEnabled(false);
	}

	auto& decoder = selectedDevice.playback.decoder.emplace(path, nullptr);

	ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.pDeviceID = &deviceInfos[selectedDevice.index].id;
	deviceConfig.playback.format = decoder.data.outputFormat;
	deviceConfig.playback.channels = decoder.data.outputChannels;
	deviceConfig.sampleRate = decoder.data.outputSampleRate;

	deviceConfig.dataCallback = dataCallback;
	deviceConfig.pUserData = &selectedDevice.playback;

	auto& device = selectedDevice.device.emplace(context, deviceConfig);

	if (ma_device_start(&device.data) != MA_SUCCESS) {
		throw ExternalError("Failed to start miniaudio device playback!");
	}
}

void Soundboard::stop() noexcept {
	if (selectedDevices.first.device) {
		ma_device_stop(&selectedDevices.first.device->data);
	}
	if (selectedDevices.second.device) {
		ma_device_stop(&selectedDevices.second.device->data);
	}
}

void Soundboard::updateDevice(SelectedAudioDevice& selected) const noexcept {
	if (!selected.device) {
		return;
	}
	if (!selected.playback.finished && ma_device_get_state(&selected.device->data) != ma_device_state_stopped) {
		return;
	}
	selected.playback.decoder.reset();
	selected.device.reset();
	selected.playback.finished = false;

	setPushToTalkIfEnabled(false);
}

void Soundboard::setPushToTalkIfEnabled(bool send) const noexcept {
	if (sendPushToTalk && pushToTalkKey != 0) {
		sendKey(pushToTalkKey, send);
	}
}

void Soundboard::serializeDevice(YAML::Node& yaml, const SelectedAudioDevice& selected) {
	if (selected.index >= deviceInfos.size()) {
		yaml = YAML::Null;
	} else {
		yaml = deviceInfos[selected.index].name;
	}
}

void Soundboard::deserializeDevice(const YAML::Node& yaml, SelectedAudioDevice& selected) {
	if (yaml.IsNull()) {
		return;
	}
	const std::string name = yaml.as<std::string>();

	for (size_t i = 0; i < deviceInfos.size(); i++) {
		if (deviceInfos[i].name != name) {
			continue;
		}
		selected.index = i;
		break;
	}
}

void Soundboard::serializeTab(YAML::Node& yaml, const SoundboardTab& tab) {
	yaml["name"] = tab.name;
	yaml["path"] = tab.path.string();

	YAML::Node hotkeyListNode = yaml["hotkeys"];
	for (const auto& [name, sound] : tab.sounds) {
		if (!sound.hotkeyId) {
			continue;
		}
		const Hotkey& hotkey = app->getHotkeyRegistry().getHotkey(*sound.hotkeyId);

		YAML::Node hotkeyNode;
		hotkeyNode["for"] = name;
		hotkeyNode["key"] = hotkey.key;
		hotkeyNode["mods"] = uint32_t(hotkey.mods);

		hotkeyListNode.push_back(hotkeyNode);
	}
}

void Soundboard::deserializeTab(const YAML::Node& yaml, SoundboardTab& tab) {
	tab.name = yaml["name"].as<std::string>();
	tab.path = yaml["path"].as<std::string>();

	populateSoundTab(tab);

	YAML::Node hotkeyListNode = yaml["hotkeys"];
	for (const auto& hotkeyNode : hotkeyListNode) {
		std::string name = hotkeyNode["for"].as<std::string>();

		const auto soundIt = tab.sounds.find(name);
		if (soundIt == tab.sounds.end()) {
			continue;
		}
		Sound& sound = soundIt->second;

		Hotkey hotkey;
		hotkey.key = hotkeyNode["key"].as<int>();
		hotkey.mods = hotkeyNode["mods"].as<uint8_t>();
		hotkey.callback = [this, sound] { play(sound.path.c_str()); };

		sound.hotkeyId = app->getHotkeyRegistry().registerHotkey(hotkey);
	}
}

void Soundboard::findThemes() noexcept {
	for (const auto& it : fs::directory_iterator(themesDir)) {
		std::string ext = it.path().extension().string();
		toLower(ext);

		if (ext != ".yaml" && ext != ".yml") {
			continue;
		}
		foundThemes.push_back(it.path());
	}
}

void Soundboard::applyScale() const noexcept {
	ImGui::GetStyle().FontScaleMain = scale;
}

void populateSoundTab(SoundboardTab& tab) {
	for (auto& it : fs::directory_iterator(tab.path)) {
		std::string name = it.path().filename();
		if (tab.sounds.contains(name)) {
			continue;
		}

		std::string ext = it.path().extension().string();
		toLower(ext);

		if (ext != ".wav" && ext != ".mp3" && ext != ".flac" && ext != ".ogg") {
			continue;
		}
		tab.sounds[std::move(name)] = Sound(it.path().string());
	}
}

void refreshSoundTab(SoundboardTab& tab, HotkeyRegistry& hotkeys) {
	for (auto it = tab.sounds.begin(); it != tab.sounds.end(); it++) {
		const auto& [name, sound] = *it;
		if (fs::exists(sound.path)) {
			continue;
		}
		if (sound.hotkeyId) {
			hotkeys.unregisterHotkey(*sound.hotkeyId);
		}

		it = tab.sounds.erase(it);
	}

	populateSoundTab(tab);
}

} // namespace vi