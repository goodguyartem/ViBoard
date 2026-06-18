#include "Platform.hpp"

#include <cstdlib>
#include <string>

namespace vi {
bool openInFileBrowser(const std::filesystem::path& path) noexcept {
	const std::string cmd = "xdg-open \"" + path.string() + "\"";
	return system(cmd.c_str()) == 0;
}

bool openUrl(const std::string& url) noexcept {
	const std::string cmd = "xdg-open " + url;
	return system(cmd.c_str()) == 0;
}
} // namespace vi