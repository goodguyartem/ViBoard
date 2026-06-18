#include "Windows.hpp"

#include <windows.h>

#include <string>

namespace vi {
void sendKey(int key, int action) noexcept {
	const UINT virtualKey = glfwKeyToVK(key);

	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = 0;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;

	// Press the key
	input.ki.wVk = virtualKey;
	input.ki.dwFlags = action == GLFW_RELEASE ? KEYEVENTF_KEYUP : 0;
	SendInput(1, &input, sizeof(INPUT));
}

bool openInFileBrowser(const std::filesystem::path& path) noexcept {
	const std::string cmd = "explorer \"" + path.string() + "\"";
	return system(cmd.c_str()) == 0;
}

bool openUrl(const std::string& url) noexcept {
	const std::string cmd = "start " + url;
	return system(cmd.c_str()) == 0;
}
} // namespace vi