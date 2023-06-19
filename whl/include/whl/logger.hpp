#pragma once

#include <fmt/format-inl.h>

#include <string>
#include <string_view>

namespace whl {
namespace logger {

/// Log Level
enum class Level {
  VERBOSE,
  DEBUG,
  INFO,
  WARNING,
  ERROR,
  CRITICAL,
};

/// Log a message using the given level.
void log(Level level, std::string_view msg);
inline void log(Level level, std::string msg) {
  log(level, std::string_view{msg});
}

/// Format and log a Verbose message.
template <typename... Args>
inline void verbose(std::string_view msg, Args &&...args) {
  log(Level::VERBOSE, fmt::format(msg, std::move(args)...));
}

/// Format and log a Debug message.
template <typename... Args>
inline void debug(std::string_view msg, Args &&...args) {
  log(Level::DEBUG, fmt::format(msg, std::move(args)...));
}

/// Format and log an Info message.
template <typename... Args>
inline void info(std::string_view msg, Args &&...args) {
  log(Level::INFO, fmt::format(msg, std::move(args)...));
}

/// Format and log a Warning message.
template <typename... Args>
inline void warning(std::string_view msg, Args &&...args) {
  log(Level::WARNING, fmt::format(msg, std::move(args)...));
}

/// Format and log an Error message.
template <typename... Args>
inline void error(std::string_view msg, Args &&...args) {
  log(Level::ERROR, fmt::format(msg, std::move(args)...));
}

/// Format and log a Critical message.
template <typename... Args>
inline void critical(std::string_view msg, Args &&...args) {
  log(Level::CRITICAL, fmt::format(msg, std::move(args)...));
}

} // namespace logger
} // namespace whl
