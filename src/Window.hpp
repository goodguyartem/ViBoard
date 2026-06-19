#pragma once

#include <string>

namespace vi {
const char* getGlfwErrorStr() noexcept;

const char* keyToString(int key) noexcept;	 // glfwGetKeyName only supports printable keys.
std::string modsToString(int mods) noexcept; // GLFW_MOD_CAPS_LOCK and GLFW_MOD_NUM_LOCK are ignored.

} // namespace vi