#include "App.hpp"
#include "CommandLine.hpp"
#include "Exceptions.hpp"
#include "IO.hpp"
#include "Paths.hpp"
#include "Project.hpp"
#include "Window.hpp"
#include "platforms/Platform.hpp"

#include <GLFW/glfw3.h>

#include <imgui.h>

#include <nfd.h>

#include <cstdlib>
#include <exception>

namespace vi {
namespace {
void onGlfwError(int error, const char* msg) noexcept {
	spdlog::error("GLFW error {} occurred: {}", error, msg);
}

void initGlfw() {
	glfwSetErrorCallback(onGlfwError);

	if (glfwPlatformSupported(GLFW_PLATFORM_X11)) {
		glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
	}
	if (!glfwInit()) {
		throw ExternalError(getGlfwErrorStr());
	}
}

void initImGui() noexcept {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	static const std::string filename = imGuiIniFilePath.string();
	io.IniFilename = filename.c_str();
	spdlog::debug("ImGui ini path set to: {}", io.IniFilename);
}

void quit() noexcept {
	NFD_Quit();
	ImGui::DestroyContext();
	glfwTerminate();
}

} // namespace
} // namespace vi

int main(int argc, char* argv[]) {
	using namespace vi;
	int status = EXIT_SUCCESS;

	io::initConsole();
	const LaunchConfig config = parseCLArgs(std::span(argv, argc));

	if (config.flags & clFlags::showHelp) {
		printHelp();
		return status;
	} else if (config.flags & clFlags::showVersion) {
		printVersion();
		return status;
	}

	try {
		io::initLogger(config);
		initGlfw();
		initImGui();
		NFD_Init();

		spdlog::info("ViBoard version {}.{}.{}", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
		spdlog::info("Running on {}", getPlatformName());

		App app;
		app.run();

	} catch (const std::exception& e) {
		spdlog::critical("CRASH! {}", e.what());
		status = EXIT_FAILURE;
	} catch (...) {
		spdlog::critical("CRASH! Unknown exception has occured.");
		status = EXIT_FAILURE;
	}

	quit();
	return status;
}