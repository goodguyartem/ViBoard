#include "Util.hpp"

#include <algorithm>
#include <cctype>

namespace vi {
ImVec4 rgba(const YAML::Node& node) {
	return rgba(node.as<unsigned long long>());
}

void toLower(std::string& string) noexcept {
	std::ranges::transform(string, string.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
}
} // namespace vi