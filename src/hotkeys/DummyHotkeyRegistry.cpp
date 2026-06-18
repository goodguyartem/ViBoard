#include "DummyHotkeyRegistry.hpp"

// Standard since C++23. Supported on most compilers as extension on previous standards.
#warning Building with no hotkey support.

namespace vi {
std::optional<size_t> DummyHotkeyRegistry::registerHotkey(const Hotkey&) noexcept {
	printError();
	return std::nullopt;
}

bool DummyHotkeyRegistry::unregisterHotkey(size_t) noexcept {
	printError();
	return false;
}

const Hotkey& DummyHotkeyRegistry::getHotkey(size_t) const noexcept {
	printError();
	static Hotkey dummy;
	return dummy;
}

void DummyHotkeyRegistry::printError() const noexcept {
	spdlog::error("Hotkeys not supported on current platform.");
}

std::unique_ptr<HotkeyRegistry> makeHotkeyRegistry() noexcept {
	return std::make_unique<DummyHotkeyRegistry>();
}
} // namespace vi