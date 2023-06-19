#include <whl/logger.hpp>

#include <fmt/format-inl.h>

#include <cassert>
#include <iostream>

namespace whl {
namespace logger {

static std::string_view to_string_view(Level level) {
  switch (level) {
  case Level::VERBOSE:
    return "verbose";
  case Level::DEBUG:
    return "debug";
  case Level::INFO:
    return "info";
  case Level::WARNING:
    return "warning";
  case Level::ERROR:
    return "error";
  case Level::CRITICAL:
    return "critical";
  }

  assert(false);
}

void log(Level level, std::string_view msg) {
  std::cout << fmt::format("[{}] {}", to_string_view(level), msg) << '\n'
            << std::flush;
}

} // namespace logger
} // namespace whl
