#pragma once

#include "CommandLine.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace vi::io {
void initConsole() noexcept;
void initLogger(const LaunchConfig& config);

// Returns stdout/stderr logger for user interaction.
// This logger is always available.
spdlog::logger& getConsole() noexcept;

// Returns logger for diagnostics.
// This logger follows settings set by LaunchConfig. By default, it outputs to console and file.
spdlog::logger& getLogger() noexcept;

template <typename T>
void consoleInfo(const T& msg) noexcept {
	getConsole().info(msg);
}

template <typename T>
void consoleError(const T& msg) noexcept {
	getConsole().error(msg);
}

template <typename T>
void consoleCritical(const T& msg) noexcept {
	getConsole().critical(msg);
}

template <typename... Args>
void consoleInfo(spdlog::format_string_t<Args...> fmt, Args&&... args) noexcept {
	getConsole().info(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void consoleError(spdlog::format_string_t<Args...> fmt, Args&&... args) noexcept {
	getConsole().error(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void consoleCritical(spdlog::format_string_t<Args...> fmt, Args&&... args) noexcept {
	getConsole().critical(fmt, std::forward<Args>(args)...);
}
} // namespace vi::io