#pragma once

#include <nfd.h>

#include <filesystem>
#include <string>

namespace vi {
void sendKey(int key, int action) noexcept;

std::filesystem::path pickFolder(const nfdnchar_t* defaultPath = nullptr);
bool openInFileBrowser(const std::filesystem::path& path) noexcept;
bool openUrl(const std::string& url) noexcept;

std::filesystem::path getConfigDir() noexcept;
std::filesystem::path getStateHomeDir() noexcept;

bool createPipeWireVirtualDevices() noexcept;
bool createPulseAudioVirtualDevices() noexcept;

const char* getPlatformName() noexcept;
} // namespace vi