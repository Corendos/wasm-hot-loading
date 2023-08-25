// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace whl {

/// Very basic structure representing an HTTP response.
struct HttpResponse {
  std::unordered_map<std::string, std::string> headers{};
  uint32_t status{0};
  std::string body{};
};

/// Executes an HTTP request to the specified URL and returns the result. This
/// call is blocking.
HttpResponse make_http_request(const std::string &url);

} // namespace whl
