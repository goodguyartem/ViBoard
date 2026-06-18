#pragma once

#include <stdexcept>

namespace vi {
class ExternalError : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

class IOError : public std::runtime_error {
	using std::runtime_error::runtime_error;
};
} // namespace vi