#pragma once

#include <spdlog/spdlog.h>

#include <cstdint>
#include <span>

#ifdef NDEBUG
#	define DEFAULT_LOG_LEVEL spdlog::level::info
#else
#	define DEFAULT_LOG_LEVEL spdlog::level::trace
#endif

namespace vi {

namespace clFlags {
enum : uint8_t {
	showHelp = 1,
	noStdout = 1 << 2,
	noFileLogging = 1 << 3,
	showVersion = 1 << 4,
};
} // namespace clFlags

struct LaunchConfig {
	uint8_t flags = 0;
	spdlog::level::level_enum logLevel = DEFAULT_LOG_LEVEL;
};

LaunchConfig parseCLArgs(std::span<char*> argv) noexcept;

void printHelp() noexcept;
void printVersion() noexcept;

} // namespace vi