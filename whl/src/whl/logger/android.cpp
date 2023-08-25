// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#include <whl/logger.hpp>

#include <android/log.h>

#include <cassert>

namespace whl {
namespace logger {

static android_LogPriority convert(Level level) {
  switch (level) {
  case Level::VERBOSE:
    return ANDROID_LOG_VERBOSE;
  case Level::DEBUG:
    return ANDROID_LOG_DEBUG;
  case Level::INFO:
    return ANDROID_LOG_INFO;
  case Level::WARNING:
    return ANDROID_LOG_WARN;
  case Level::ERROR:
    return ANDROID_LOG_ERROR;
  case Level::CRITICAL:
    return ANDROID_LOG_FATAL;
  }

  assert(false);
}

void log(Level level, std::string_view msg) {
  __android_log_print(convert(level), "whl", "%.*s", msg.size(), msg.data());
}

} // namespace logger
} // namespace whl
