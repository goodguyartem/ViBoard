#include "Paths.hpp"
#include "Project.hpp"
#include "platforms/Platform.hpp"

namespace fs = std::filesystem;

namespace vi {
namespace {
fs::path getResDir() noexcept {
	static const fs::path installResDir = ".." / fs::path(RES_SUBDIR);

	if (fs::exists(installResDir)) {
		return installResDir;
	}

	return "res";
}
} // namespace

const fs::path configDir = getConfigDir() / "viboard";
const fs::path stateDir = getStateHomeDir() / "viboard";
const fs::path resDir = getResDir();

const fs::path themesDir = configDir / "themes";
const fs::path configFilePath = configDir / "config.yaml";
const std::string imGuiIniFilePath = (configDir / "ImGui.ini").string();

} // namespace vi