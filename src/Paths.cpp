#include "Paths.hpp"
#include "Project.hpp"
#include "platforms/Platform.hpp"

namespace fs = std::filesystem;

namespace vi {
namespace {
fs::path getResDir() noexcept {
	if (fs::exists(INSTALL_RES_DIR)) {
		return INSTALL_RES_DIR;
	}
	return "res";
}
} // namespace

const fs::path configDir = getConfigDir() / PROJECT_SUBDIR;
const fs::path stateDir = getStateHomeDir() / PROJECT_SUBDIR;
const fs::path resDir = getResDir();

const fs::path themesDir = resDir / "themes";
const fs::path configFilePath = configDir / "config.yaml";
const fs::path imGuiIniFilePath = configDir / "ImGui.ini";

} // namespace vi