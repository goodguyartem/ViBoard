#pragma once

#include "HotkeyRegistry.hpp"

#include <X11/Xlib.h>

#include <unordered_map>

namespace vi {
class X11HotkeyRegistry : public HotkeyRegistry {
public:
	X11HotkeyRegistry() noexcept;

	std::optional<size_t> registerHotkey(const Hotkey& hotkey) noexcept override;
	bool unregisterHotkey(size_t id) noexcept override;

	const Hotkey& getHotkey(size_t id) const noexcept override;

	void pollEvents() noexcept override;
	bool isSupported() const noexcept override { return true; }

private:
	struct NativeHotkey {
		bool operator==(NativeHotkey other) const noexcept;

		int keycode = 0;
		int mods = 0;
	};

	struct NativeHotkeyHash {
		size_t operator()(const NativeHotkey& hotkey) const noexcept;
	};

	Window rootId;

	std::unordered_map<size_t, NativeHotkey> nativeHotkeys;
	std::unordered_map<NativeHotkey, Hotkey, NativeHotkeyHash> hotkeys;
};
} // namespace vi