#pragma once

#include <ios>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#define KB(x) (x * 1024ull)
#define MB(x) (x * 1024ull * 1024ull)
#define GB(x) (x * 1024ull * 1024ull * 1024ull)

namespace whl {
namespace fs {

/// Open and read a file using its path. Returns a string with the content of
/// the file.
std::string read_file_string(std::string_view path,
                             std::ios_base::openmode mode = std::ios_base::in);

/// Open and read a file using its path. Returns a vector of bytes with the
/// content of the file.
std::vector<uint8_t>
read_file(std::string_view path,
          std::ios_base::openmode mode = std::ios_base::in);

/// Write the given string to a file using its path.
void write_file_string(std::string_view path, const std::string &content);

/// Write the given vector of bytes to a file using its path.
void write_file(std::string_view path, const std::vector<uint8_t> &content);

} // namespace fs

namespace crypto {

/// Verify the signature of the given file using the provided public key.
/// Returns true if computed signature matches, false otherwise.
bool verify_signature(const std::string &content, const std::string &signature,
                      const std::string &public_key);

/// Returns a string containing the base64 encoding of the input.
std::string base64_encode(const std::vector<uint8_t> &data);

/// Returns a vector of bytes containing the base64 decoding of the input, or
/// std::nullopt in case of failure.
std::optional<std::vector<uint8_t>> base64_decode(const std::string &input);

} // namespace crypto
} // namespace whl
