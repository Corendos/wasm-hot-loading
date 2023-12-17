// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>

namespace whl {

/// Types of message that the SampleModule can receive.
enum class SampleMessageType {
  NOOP,
  UPDATE,
  QUERY,
};

/// Update payload type.
struct UpdatePayload {
  const char *name_ptr;
  uint32_t name_len;
  uint32_t value;

  static UpdatePayload alloc(const std::string &name, uint32_t value);
  static void free(UpdatePayload &payload);

  std::string name() const;
};

/// Represents a Message that needs to be handled by the SampleModule.
struct SampleMessage {
  /// Type of the message.
  SampleMessageType type;
  /// Payload depending on the type.
  union {
    /* void noop; */
    UpdatePayload update;
    /* void query; */
  } payload;
};

/// Types of message that the Global scheduler can receive.
enum class GlobalMessageType {
  NOOP,
  STATE,
};

/// State payload contains the same field as the UpdatePayload.
using StatePayload = UpdatePayload;

/// Represents a Message that needs to be handled by the Global Scheduler.
struct GlobalMessage {
  GlobalMessageType type;
  union {
    /* void noop; */
    StatePayload state;
  } payload;
};

} // namespace whl
