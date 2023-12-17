// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#include <whl/messages.hpp>

#include <cassert>

namespace whl {

UpdatePayload UpdatePayload::alloc(const std::string &name, uint32_t value) {
  char *name_ptr = (char *)malloc(name.size());
  std::copy(name.begin(), name.end(), name_ptr);

  assert(name_ptr);

  return UpdatePayload{
      .name_ptr = name_ptr,
      .name_len = static_cast<uint32_t>(name.size()),
      .value = value,
  };
}

void UpdatePayload::free(UpdatePayload &payload) {
  ::free((void *)payload.name_ptr);
  payload.name_ptr = 0;
  payload.name_len = 0;
}

std::string UpdatePayload::name() const {
  return std::string(name_ptr, name_len);
}

} // namespace whl
