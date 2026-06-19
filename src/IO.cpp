#include "IO.hpp"
#include "Paths.hpp"

#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <cassert>
#include <memory>
#include <vector>

namespace vi::io {
namespace {
std::shared_ptr<spdlog::logger> console;
std::shared_ptr<spdlog::logger> logger;

} // namespace

void initConsole() noexcept {
	auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	sink->set_color_mode(spdlog::color_mode::always);

	console = std::make_shared<spdlog::logger>("console", sink);
	console->set_level(spdlog::level::info);
	console->set_pattern("%^[%l]%$ %v");
}

void initLogger(const LaunchConfig& config) {
	std::vector<spdlog::sink_ptr> sinks;

	if (!(config.flags & clFlags::noStdout)) {
		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		consoleSink->set_color_mode(spdlog::color_mode::always);
		consoleSink->set_level(spdlog::level::trace);

		sinks.push_back(std::move(consoleSink));
	}
	if (!(config.flags & clFlags::noFileLogging)) {
		auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>((stateDir / "Log.txt").string(), true);
		fileSink->set_level(spdlog::level::trace);
		sinks.push_back(std::move(fileSink));
	}

	logger = std::make_shared<spdlog::logger>("logger", sinks.begin(), sinks.end());
	logger->set_level(config.logLevel);
	logger->set_pattern("%^[%d-%m-%Y] [%I:%M:%S %p] [%l]%$ %v");

	spdlog::set_default_logger(logger);
}

spdlog::logger& getConsole() noexcept {
	assert(console);
	return *console;
}

spdlog::logger& getLogger() noexcept {
	assert(logger);
	return *logger;
}

} // namespace vi::io