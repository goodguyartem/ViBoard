#pragma once

#include <imgui.h>
#include <imgui_internal.h>

#include <yaml-cpp/yaml.h> // IWYU pragma: export

namespace YAML {
template <>
struct convert<ImVec2> {
	static Node encode(const ImVec2& vec) noexcept;
	static bool decode(const Node& node, ImVec2& vec) noexcept;
};

template <>
struct convert<ImVec2i> {
	static Node encode(const ImVec2i& vec) noexcept;
	static bool decode(const Node& node, ImVec2i& vec) noexcept;
};

template <>
struct convert<ImVec4> {
	static Node encode(const ImVec4& vec) noexcept;
	static bool decode(const Node& node, ImVec4& vec) noexcept;
};
} // namespace YAML