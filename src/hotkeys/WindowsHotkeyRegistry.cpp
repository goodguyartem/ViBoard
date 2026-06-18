#include "WindowsHotkeyRegistry.hpp"
#include "../platforms/Windows.hpp"

#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include <winuser.h>

#include <utility>

namespace vi {
std::optional<size_t> WindowsHotkeyRegistry::registerHotkey(Hotkey hotkey) noexcept {
	assert(hotkey.callback);

	static size_t nextId = 0;
	const int nativeId = static_cast<int>(nextId);

	UINT mods = 0;
	if (hotkey.mods & GLFW_MOD_CONTROL) mods |= MOD_CONTROL;
	if (hotkey.mods & GLFW_MOD_SHIFT) mods |= MOD_SHIFT;
	if (hotkey.mods & GLFW_MOD_ALT) mods |= MOD_ALT;
	if (hotkey.mods & GLFW_MOD_SUPER) mods |= MOD_WIN;

	const UINT virtualKey = glfwKeyToVK(hotkey.key);
	if (!RegisterHotKey(nullptr, nativeId, mods, virtualKey)) {
		spdlog::error("Failed to register hotkey (error {}).", GetLastError());
		return std::nullopt;
	}

	hotkeys.push_back(std::move(hotkey));
	return nextId++;
}

bool WindowsHotkeyRegistry::unregisterHotkey(size_t id) noexcept {
	assert(id < hotkeys.size());
	if (!UnregisterHotKey(nullptr, id)) {
		spdlog::error("Failed to unregister hotkey ID {} (error {}).", id, GetLastError());
		return false;
	}

	hotkeys.erase(hotkeys.begin() + id);
}

const Hotkey& WindowsHotkeyRegistry::getHotkey(size_t id) const noexcept {
	assert(id < hotkeys.size());
	return hotkeys[id];
}

void WindowsHotkeyRegistry::pollEvents() noexcept {
	MSG msg;
	while (PeekMessage(&msg, nullptr, WM_HOTKEY, WM_HOTKEY, PM_REMOVE)) {
		const size_t id = static_cast<size_t>(msg.wParam);
		assert(id < hotkeys.size());

		hotkeys[id].callback();
	}
}

std::unique_ptr<HotkeyRegistry> makeHotkeyRegistry() noexcept {
	return std::make_unique<WindowsHotkeyRegistry>();
}

} // namespace vi