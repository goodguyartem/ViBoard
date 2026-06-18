// Standard since C++23. Supported on most compilers as extension on previous standards.
#warning Building on an unsupported platform! Several features will not work as expected.

#include "Platform.hpp"

#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

namespace vi {
void sendKey(int) noexcept {
	spdlog::error("sendKey() is not supported on this platform!");
}

bool openUrl(const std::string&) noexcept {
	spdlog::error("openUrl() is not supported on this platform!");
	return false;
}

bool openInFileBrowser(const std::filesystem::path&) noexcept {
	spdlog::error("openInFileBrowser() is not supported on this platform!");
	return false;
}
} // namespace vi
