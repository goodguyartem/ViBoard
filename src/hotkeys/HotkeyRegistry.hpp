#pragma once

#include <yaml-cpp/yaml.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

namespace vi {
struct Hotkey {
	int key = 0;
	uint8_t mods = 0;
	std::function<void()> callback;
};

class HotkeyRegistry {
public:
	virtual ~HotkeyRegistry() = default;

	// Registers a new hotkey.
	// Returns a numeric ID of the newly-registered hotkey upon success, or std::nullopt on failure.
	virtual std::optional<size_t> registerHotkey(const Hotkey& hotkey) noexcept = 0;

	// Unregisters a hotkey.
	// Returns true on success, false otherwise.
	virtual bool unregisterHotkey(size_t id) noexcept = 0;

	// Returns hotkey associated with given ID.
	virtual const Hotkey& getHotkey(size_t id) const noexcept = 0;

	// Handles hotkey events. Irrelevant events are ignored.
	virtual void pollEvents() noexcept = 0;

	// Returns true if current platform has hotkey support.
	virtual bool isSupported() const noexcept = 0;

protected:
	HotkeyRegistry() = default;
	HotkeyRegistry(const HotkeyRegistry& other) = default;
	HotkeyRegistry(HotkeyRegistry&& other) = default;

	HotkeyRegistry& operator=(const HotkeyRegistry& other) = default;
	HotkeyRegistry& operator=(HotkeyRegistry&& other) = default;
};

std::unique_ptr<HotkeyRegistry> makeHotkeyRegistry() noexcept;

std::string toString(const Hotkey& hotkey) noexcept;

} // namespace vi