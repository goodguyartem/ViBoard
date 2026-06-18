#pragma once

#include "Soundboard.hpp"
#include "hotkeys/HotkeyRegistry.hpp"

#include <GLFW/glfw3.h>

#include <yaml-cpp/yaml.h>

#include <imgui.h>

#include <cstddef>
#include <memory>

namespace vi {
// Indices for ImGui::GetIO().Fonts->Fonts
namespace font {
enum : size_t {
	normal,
	bold,
	larger,
	h1,
	h2
};
} // namespace font

class App {
public:
	App() = default;

	App(const App&) = delete;
	App(App&&) = delete;

	~App();

	App& operator=(const App&) = delete;
	App& operator=(App&&) = delete;

	void run();

	GLFWwindow* getWindow() const noexcept { return window.get(); }
	HotkeyRegistry& getHotkeyRegistry() const noexcept { return *hotkeys; }

private:
	void init();

	void update();
	void render() const;

	void startFrame() const noexcept;
	ImGuiWindowFlags getWindowFlags() const noexcept;

	std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window{nullptr, glfwDestroyWindow};
	std::unique_ptr<HotkeyRegistry> hotkeys = makeHotkeyRegistry();

	Soundboard soundboard{*this};
};

void loadTheme(const YAML::Node& theme, ImGuiStyle& style);

} // namespace vi