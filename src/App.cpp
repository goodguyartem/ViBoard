#include "App.hpp"
#include "Exceptions.hpp"
#include "Paths.hpp"
#include "Util.hpp"
#include "Window.hpp"
#include "YamlTypes.hpp"

#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include <spdlog/spdlog.h>

#include <array>
#include <filesystem>
#include <utility>

namespace fs = std::filesystem;

namespace vi {
void App::run() {
	init();

	while (!glfwWindowShouldClose(window.get())) {
		update();
		render();
	}

	soundboard.serialize(configFilePath);
}

App::~App() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
}

void App::init() {
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100 (WebGL 1.0)
	const char* glslVersion = "#version 100";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
	// GL ES 3.0 + GLSL 300 es (WebGL 2.0)
	const char* glslVersion = "#version 300 es";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
	// GL 3.2 + GLSL 150
	const char* glslVersion = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);		   // Required on Mac
#else
	// GL 3.2 + GLSL 150
	const char* glslVersion = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);		   // 3.0+ only
#endif

	window.reset(glfwCreateWindow(1280, 720, "ViBoard", nullptr, nullptr));
	if (!window) {
		throw ExternalError(getGlfwErrorStr());
	}

	glfwMakeContextCurrent(window.get());
	glfwSwapInterval(1); // Enable vsync

	ImGui_ImplGlfw_InitForOpenGL(window.get(), true);
#ifdef __EMSCRIPTEN__
	ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
	ImGui_ImplOpenGL3_Init(glslVersion);

	if (std::filesystem::exists(configFilePath)) {
		spdlog::debug("Found {}", configFilePath.string());
		soundboard.deserialize(configFilePath);

	} else {
		spdlog::debug("Loading default theme.");
		soundboard.loadTheme(themesDir / "Violet.yaml");
	}

	if (!fs::exists(imGuiIniFilePath)) {
		spdlog::debug("No ImGui ini found, copying default ini.");
		fs::copy(resDir / "ImGuiDefault.ini", imGuiIniFilePath);
	}

	glfwSetWindowUserPointer(window.get(), this);

	glfwSetKeyCallback(window.get(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		App& app = *static_cast<App*>(glfwGetWindowUserPointer(window));
		app.soundboard.onKeyPress(key, action, mods);

		std::ignore = scancode;
	});

	ImGuiIO& io = ImGui::GetIO();

	// -- TTF function also loads OTF files. --
	std::array<fs::path, 3> fonts;
	fonts[0] = resDir / "CreatoDisplayRegular.otf";
	fonts[1] = resDir / "CreatoDisplayBold.otf";
	fonts[2] = resDir / "CoolveticaRg.otf";

	for (const fs::path& path : fonts) {
		if (!fs::exists(path)) {
			throw IOError("Could not find font \"" + path.string() + "\"");
		}
	}

	io.Fonts->AddFontFromFileTTF(fonts[0].string().c_str(), 16.0f); // Normal
	io.Fonts->AddFontFromFileTTF(fonts[1].string().c_str(), 16.0f); // Bold
	io.Fonts->AddFontFromFileTTF(fonts[0].string().c_str(), 18.0f); // Larger
	io.Fonts->AddFontFromFileTTF(fonts[2].string().c_str(), 48.0f); // h1
	io.Fonts->AddFontFromFileTTF(fonts[1].string().c_str(), 21.0f); // h2
}

void App::update() {
	glfwWaitEventsTimeout(glfwGetWindowAttrib(window.get(), GLFW_ICONIFIED) ? 0.1 : 0.01);
	hotkeys->pollEvents();

	startFrame();
	soundboard.update();

	ImGui::End();
}

void App::render() const {
	ImGui::Render();

	ImVec2i display;
	glfwGetFramebufferSize(window.get(), &display.x, &display.y);
	glViewport(0, 0, display.x, display.y);
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		glfwMakeContextCurrent(window.get());
	}

	glfwSwapBuffers(window.get());
}

void App::startFrame() const noexcept {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	static const ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
	static const ImGuiWindowFlags windowFlags = getWindowFlags();

	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("DockSpace", nullptr, windowFlags);
	ImGui::PopStyleVar(3);

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
		ImGuiID dockspaceId = ImGui::GetID("OpenGLAppDockspace");
		ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);
	}
}

ImGuiWindowFlags App::getWindowFlags() const noexcept {
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking;

	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	flags |= ImGuiWindowFlags_NoNavFocus;
	// flags |= ImGuiWindowFlags_MenuBar;

	return flags;
}

namespace {
void loadThemeVars(const YAML::Node& vars, ImGuiStyle& style) noexcept {
	if (const auto node = vars["frameRounding"]) {
		style.FrameRounding = node.as<int>();
	}
	if (const auto node = vars["grabRounding"]) {
		style.GrabRounding = node.as<int>();
	}
	if (const auto node = vars["popupRounding"]) {
		style.PopupRounding = node.as<int>();
	}
	if (const auto node = vars["framePadding"]) {
		style.FramePadding = node.as<ImVec2>();
	}
}

void loadThemeColours(const YAML::Node& colours, ImGuiStyle& style) noexcept {
	if (const auto node = colours["button"]) {
		style.Colors[ImGuiCol_Button] = rgba(node);
	}
	if (const auto node = colours["buttonHovered"]) {
		style.Colors[ImGuiCol_ButtonHovered] = rgba(node);
	}
	if (const auto node = colours["buttonActive"]) {
		style.Colors[ImGuiCol_ButtonActive] = rgba(node);
	}

	if (const auto node = colours["header"]) {
		style.Colors[ImGuiCol_Header] = rgba(node);
	}
	if (const auto node = colours["headerHovered"]) {
		style.Colors[ImGuiCol_HeaderHovered] = rgba(node);
	}
	if (const auto node = colours["headerActive"]) {
		style.Colors[ImGuiCol_HeaderActive] = rgba(node);
	}

	if (const auto node = colours["sliderGrab"]) {
		style.Colors[ImGuiCol_SliderGrab] = rgba(node);
	}
	if (const auto node = colours["sliderGrabActive"]) {
		style.Colors[ImGuiCol_SliderGrabActive] = rgba(node);
	}

	if (const auto node = colours["frameBg"]) {
		style.Colors[ImGuiCol_FrameBg] = rgba(node);
	}
	if (const auto node = colours["frameBgHovered"]) {
		style.Colors[ImGuiCol_FrameBgHovered] = rgba(node);
	}
	if (const auto node = colours["frameBgActive"]) {
		style.Colors[ImGuiCol_FrameBgActive] = rgba(node);
	}

	if (const auto node = colours["checkboxSelectedBg"]) {
		style.Colors[ImGuiCol_CheckboxSelectedBg] = rgba(node);
	}
	if (const auto node = colours["checkMark"]) {
		style.Colors[ImGuiCol_CheckMark] = rgba(node);
	}

	if (const auto node = colours["resizeGrip"]) {
		style.Colors[ImGuiCol_ResizeGrip] = rgba(node);
	}
	if (const auto node = colours["resizeGripHovered"]) {
		style.Colors[ImGuiCol_ResizeGripHovered] = rgba(node);
	}
	if (const auto node = colours["resizeGripActive"]) {
		style.Colors[ImGuiCol_ResizeGripActive] = rgba(node);
	}

	if (const auto node = colours["tab"]) {
		style.Colors[ImGuiCol_Tab] = rgba(node);
	}
	if (const auto node = colours["tabHovered"]) {
		style.Colors[ImGuiCol_TabHovered] = rgba(node);
	}
	if (const auto node = colours["tabActive"]) {
		style.Colors[ImGuiCol_TabActive] = rgba(node);
	}
	if (const auto node = colours["tabSelected"]) {
		style.Colors[ImGuiCol_TabSelected] = rgba(node);
	}
	if (const auto node = colours["tabSelectedOverline"]) {
		style.Colors[ImGuiCol_TabSelectedOverline] = rgba(node);
	}

	if (const auto node = colours["tabDimmed"]) {
		style.Colors[ImGuiCol_TabDimmed] = rgba(node);
	}
	if (const auto node = colours["tabDimmedSelected"]) {
		style.Colors[ImGuiCol_TabDimmedSelected] = rgba(node);
	}
	if (const auto node = colours["tabDimmedSelectedOverline"]) {
		style.Colors[ImGuiCol_TabDimmedSelectedOverline] = rgba(node);
	}
	if (const auto node = colours["tabUnfocused"]) {
		style.Colors[ImGuiCol_TabUnfocused] = rgba(node);
	}
	if (const auto node = colours["tabUnfocusedActive"]) {
		style.Colors[ImGuiCol_TabUnfocusedActive] = rgba(node);
	}

	if (const auto node = colours["titleBg"]) {
		style.Colors[ImGuiCol_TitleBg] = rgba(node);
	}
	if (const auto node = colours["titleBgActive"]) {
		style.Colors[ImGuiCol_TitleBgActive] = rgba(node);
	}
	if (const auto node = colours["titleBgCollapsed"]) {
		style.Colors[ImGuiCol_TitleBgCollapsed] = rgba(node);
	}

	if (const auto node = colours["dockingPreview"]) {
		style.Colors[ImGuiCol_DockingPreview] = rgba(node);
	}
	if (const auto node = colours["dockingEmptyBg"]) {
		style.Colors[ImGuiCol_DockingEmptyBg] = rgba(node);
	}

	if (const auto node = colours["windowBg"]) {
		style.Colors[ImGuiCol_WindowBg] = rgba(node);
	}
	if (const auto node = colours["modalWindowDimBg"]) {
		style.Colors[ImGuiCol_ModalWindowDimBg] = rgba(node);
	}

	if (const auto node = colours["text"]) {
		style.Colors[ImGuiCol_Text] = rgba(node);
	}
	if (const auto node = colours["textDisabled"]) {
		style.Colors[ImGuiCol_TextDisabled] = rgba(node);
	}
	if (const auto node = colours["textLink"]) {
		style.Colors[ImGuiCol_TextLink] = rgba(node);
	}
}
} // namespace

void loadTheme(const YAML::Node& theme, ImGuiStyle& style) {
	const auto basedOnNode = theme["basedOn"];
	const std::string basedOn = basedOnNode && !basedOnNode.IsNull() ? basedOnNode.as<std::string>() : "";

	if (basedOn.empty() || basedOn == "dark") {
		ImGui::StyleColorsDark(&style);
	} else if (basedOn == "light") {
		ImGui::StyleColorsLight(&style);
	} else if (basedOn == "classic") {
		ImGui::StyleColorsClassic(&style);
	} else {
		throw IOError(basedOn + " is not a valid template. Allowed values are dark, light, or classic.");
	}

	const auto vars = theme["styleVars"];
	if (vars) {
		loadThemeVars(vars, style);
	}

	const auto colours = theme["colours"];
	if (colours) {
		loadThemeColours(colours, style);
	}
}
} // namespace vi