#pragma once

#include "HotkeyRegistry.hpp"

#include <cassert>

namespace vi {
class WindowsHotkeyRegistry : public HotkeyRegistry {
public:
	std::optional<size_t> registerHotkey(const Hotkey& hotkey) noexcept override;
	bool unregisterHotkey(size_t id) noexcept override;

	const Hotkey& getHotkey(size_t id) const noexcept override;

	void pollEvents() noexcept override;
	bool isSupported() const noexcept override { return true; }

private:
	std::vector<Hotkey> hotkeys;
};

} // namespace vi