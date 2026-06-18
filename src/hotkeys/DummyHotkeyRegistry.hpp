#pragma once

#include "HotkeyRegistry.hpp"

#include <spdlog/spdlog.h>

namespace vi {
class DummyHotkeyRegistry : public HotkeyRegistry {
public:
	std::optional<size_t> registerHotkey(const Hotkey&) noexcept override;
	bool unregisterHotkey(size_t) noexcept override;

	const Hotkey& getHotkey(size_t) const noexcept override;

	void pollEvents() noexcept override {}
	bool isSupported() const noexcept override { return false; }

private:
	void printError() const noexcept;
};
} // namespace vi