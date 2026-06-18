#include "CommandLine.hpp"
#include "IO.hpp"
#include "Project.hpp"

#include <charconv>
#include <string_view>

namespace vi {
LaunchConfig parseCLArgs(std::span<char*> argv) noexcept {
	LaunchConfig config;
	for (auto it = argv.begin() + 1; it != argv.end(); it++) {
		const std::string_view flag = *it;

		if (flag == "--help") {
			config.flags |= clFlags::showHelp;

		} else if (flag == "-S" || flag == "--silent") {
			config.flags |= clFlags::noStdout;

		} else if (flag == "-N" || flag == "--no-file-logging") {
			config.flags |= clFlags::noFileLogging;

		} else if (flag == "--version") {
			config.flags |= clFlags::showVersion;

		} else if (flag == "-L" || flag == "--log-level") {
			if (++it == argv.end()) {
				io::consoleError("No log level value provided. See --help for correct usage.");
				continue;
			}

			const std::string_view value = *it;
			int level = 0;

			const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), level);
			if (ec != std::errc()) {
				io::consoleError(
					"Log level must be an integer ranging from [0, 6]. See --help for usage instructions."
				);
				continue;
			}
			config.logLevel = static_cast<spdlog::level::level_enum>(level);

		} else {
			io::consoleError("{}: unrecognized flag. Try --help for list of flags.", flag);
		}
	}

	return config;
}

void printHelp() noexcept {
	io::consoleInfo(
		"==== ViBoard Help ====\n"
		"-S --silent:           Disable logging to stdout.\n"
		"-N --no-file-logging:  Disable logging to file.\n"
		"-L --log-level <n>:    Set a log level from 0 (trace) to 5 (critical), or 6 to disable logging entirely.\n"
		"                       Default value is 2 (info).\n"
		"--version:             Display version info."
	);
}

void printVersion() noexcept {
	io::consoleInfo(
		"ViBoard {}.{}.{}\n"
		"Copyright (c) 2026-EndOfTime goodguyartem <https://www.github.com/goodguyartem>\n"
		"This is free software licensed under GPL-3.0.\n"
		"https://github.com/goodguyartem/ViBoard",
		VERSION_MAJOR,
		VERSION_MINOR,
		VERSION_PATCH
	);
}

} // namespace vi