#include "HotkeyRegistry.hpp"
#include "../Window.hpp"

namespace vi {
std::string toString(const Hotkey& hotkey) noexcept {
	return modsToString(hotkey.mods) + keyToString(hotkey.key);
}
} // namespace vi