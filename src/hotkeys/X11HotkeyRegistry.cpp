#include "X11HotkeyRegistry.hpp"
#include "../platforms/X11.hpp"

#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include <array>
#include <cassert>
#include <optional>

namespace vi {
namespace {
bool x11ErrorOccured = false;

int onError(Display*, XErrorEvent* error) noexcept {
	spdlog::info("An X11 error occured: {}", error->error_code);
	x11ErrorOccured = true;
	return 0;
}
} // namespace

constexpr unsigned int ignoredMasks = LockMask | Mod2Mask;
constexpr std::array<unsigned int, 4> locks = {0, LockMask, Mod2Mask, LockMask | Mod2Mask};

X11HotkeyRegistry::X11HotkeyRegistry() noexcept
	: rootId(DefaultRootWindow(getX11Display())) {

	XSelectInput(getX11Display(), rootId, KeyPressMask);
}

std::optional<size_t> X11HotkeyRegistry::registerHotkey(const Hotkey& hotkey) noexcept {
	assert(hotkey.callback);
	static size_t nextId = 1;

	const KeySym sym = glfwKeyToKeySym(hotkey.key);
	if (sym == NoSymbol) {
		spdlog::info("Key {} is not yet supported.", hotkey.key);
		return std::nullopt;
	}

	NativeHotkey native;
	native.keycode = XKeysymToKeycode(getX11Display(), sym);

	if (hotkey.mods & GLFW_MOD_CONTROL) native.mods |= ControlMask;
	if (hotkey.mods & GLFW_MOD_SHIFT) native.mods |= ShiftMask;
	if (hotkey.mods & GLFW_MOD_ALT) native.mods |= Mod1Mask;
	if (hotkey.mods & GLFW_MOD_SUPER) native.mods |= Mod4Mask;

	if (hotkeys.contains(native)) {
		spdlog::warn("Hotkey {} with mods 0b{:b} already exists!", hotkey.key, hotkey.mods);
		return std::nullopt;
	}

	const auto lastHandler = XSetErrorHandler(onError);
	for (const unsigned int lock : locks) {
		XGrabKey(getX11Display(), native.keycode, native.mods | lock, rootId, true, GrabModeAsync, GrabModeAsync);
	}

	XSync(getX11Display(), False);
	XSetErrorHandler(lastHandler);

	if (x11ErrorOccured) {
		x11ErrorOccured = false;
		return std::nullopt;
	}

	nativeHotkeys.emplace(nextId, native);
	hotkeys.emplace(native, hotkey);

	return nextId++;
}

bool X11HotkeyRegistry::unregisterHotkey(size_t id) noexcept {
	assert(nativeHotkeys.contains(id));
	const auto it = nativeHotkeys.find(id);

	const auto lastHandler = XSetErrorHandler(onError);
	XUngrabKey(getX11Display(), it->second.keycode, it->second.mods, rootId);
	XSync(getX11Display(), False);
	XSetErrorHandler(lastHandler);

	if (x11ErrorOccured) {
		x11ErrorOccured = false;
		return false;
	}

	hotkeys.erase(it->second);
	nativeHotkeys.erase(it);
	return true;
}

void X11HotkeyRegistry::pollEvents() noexcept {
	XEvent event;
	while (XCheckTypedEvent(getX11Display(), KeyPress, &event)) {
		const NativeHotkey native(event.xkey.keycode, event.xkey.state & ~ignoredMasks);
		if (const auto it = hotkeys.find(native); it != hotkeys.end()) {
			it->second.callback();
		}
	}
}

const Hotkey& X11HotkeyRegistry::getHotkey(size_t id) const noexcept {
	return hotkeys.at(nativeHotkeys.at(id));
}

bool X11HotkeyRegistry::NativeHotkey::operator==(NativeHotkey other) const noexcept {
	return keycode == other.keycode && mods == other.mods;
}

size_t X11HotkeyRegistry::NativeHotkeyHash::operator()(const NativeHotkey& hotkey) const noexcept {
	return std::hash<int>()(hotkey.keycode) ^ std::hash<int>()(hotkey.mods);
}

std::unique_ptr<HotkeyRegistry> makeHotkeyRegistry() noexcept {
	return std::make_unique<X11HotkeyRegistry>();
}

} // namespace vi