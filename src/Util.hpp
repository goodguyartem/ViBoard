#pragma once

#include <imgui.h>

#include <yaml-cpp/yaml.h>

#include <string>

namespace vi {
constexpr ImVec4 rgba(unsigned long long hex) noexcept {
	ImVec4 col;
	col.x = ((hex >> 24) & 0xff) / 255.0f;
	col.y = ((hex >> 16) & 0xff) / 255.0f;
	col.z = ((hex >> 8) & 0xff) / 255.0f;
	col.w = (hex & 0xff) / 255.0f;

	return col;
}

ImVec4 rgba(const YAML::Node& node);

constexpr ImVec4 rgb(unsigned long hex) noexcept {
	ImVec4 col;
	col.x = ((hex >> 16) & 0xff) / 255.0f;
	col.y = ((hex >> 8) & 0xff) / 255.0f;
	col.z = (hex & 0xff) / 255.0f;
	col.w = 1.0f;

	return col;
}

void toLower(std::string& string) noexcept;
} // namespace vi