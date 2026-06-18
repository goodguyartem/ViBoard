#include "X11.hpp"
#include "Platform.hpp"

#include <GLFW/glfw3.h>

#include <X11/extensions/XTest.h>

#include <memory>

#include <spdlog/spdlog.h>

namespace vi {
namespace {
// Kept seperate from GLFW's display so events are not faught over.
std::unique_ptr<Display, decltype(&XCloseDisplay)> display(XOpenDisplay(nullptr), XCloseDisplay);
} // namespace

Display* getX11Display() noexcept {
	return display.get();
}

void sendKey(int key, int action) noexcept {
	spdlog::info("{} {}", key, action == GLFW_PRESS ? "pressed!" : "released!");
	const KeySym keySym = glfwKeyToKeySym(key);
	XTestFakeKeyEvent(display.get(), XKeysymToKeycode(display.get(), keySym), action != GLFW_RELEASE ? True : False, 0);
	XFlush(display.get());
}

} // namespace vi