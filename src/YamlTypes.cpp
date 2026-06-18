#include "YamlTypes.hpp"

namespace YAML {
Node convert<ImVec2>::encode(const ImVec2& vec) noexcept {
	Node node;
	node["x"] = vec.x;
	node["y"] = vec.y;
	return node;
}

bool convert<ImVec2>::decode(const Node& node, ImVec2& vec) noexcept {
	if (!node.IsMap()) {
		return false;
	}
	if (!node["x"] || !node["y"]) {
		return false;
	}

	vec.x = node["x"].as<float>();
	vec.y = node["y"].as<float>();
	return true;
}

Node convert<ImVec2i>::encode(const ImVec2i& vec) noexcept {
	Node node;
	node["x"] = vec.x;
	node["y"] = vec.y;
	return node;
}

bool convert<ImVec2i>::decode(const Node& node, ImVec2i& vec) noexcept {
	if (!node.IsMap()) {
		return false;
	}
	if (!node["x"] || !node["y"]) {
		return false;
	}

	vec.x = node["x"].as<int>();
	vec.y = node["y"].as<int>();
	return true;
}

Node convert<ImVec4>::encode(const ImVec4& vec) noexcept {
	Node node;
	node["x"] = vec.x;
	node["y"] = vec.y;
	node["z"] = vec.z;
	node["w"] = vec.w;
	return node;
}

bool convert<ImVec4>::decode(const Node& node, ImVec4& vec) noexcept {
	if (!node.IsMap()) {
		return false;
	}

	if (node["x"] && node["y"] && node["z"] && node["w"]) {
		vec.x = node["x"].as<float>();
		vec.y = node["y"].as<float>();
		vec.z = node["z"].as<float>();
		vec.w = node["w"].as<float>();
		return true;
	}

	if (node["r"] && node["g"] && node["b"] && node["a"]) {
		vec.x = node["r"].as<float>();
		vec.y = node["g"].as<float>();
		vec.z = node["b"].as<float>();
		vec.w = node["a"].as<float>();
		return true;
	}

	return false;
}

} // namespace YAML
