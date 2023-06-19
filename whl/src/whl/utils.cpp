#include <whl/defer.hpp>
#include <whl/logger.hpp>
#include <whl/utils.hpp>

#include <fmt/format-inl.h>
#include <fmt/std.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include <fstream>

namespace whl {
namespace fs {
std::string read_file_string(std::string_view path,
                             std::ios_base::openmode mode) {
  std::ifstream f{std::string(path), mode};
  if (!f.is_open())
    return {};

  f.seekg(0, std::ios_base::end);
  const size_t file_size = f.tellg();

  std::string buffer(file_size, 0);

  f.seekg(0, std::ios_base::beg);
  f.read(buffer.data(), file_size);

  return buffer;
}

std::vector<uint8_t> read_file(std::string_view path,
                               std::ios_base::openmode mode) {
  std::ifstream f{std::string(path), mode};
  if (!f.is_open())
    return {};

  f.seekg(0, std::ios_base::end);
  const size_t file_size = f.tellg();

  std::vector<uint8_t> buffer(file_size);

  f.seekg(0, std::ios_base::beg);
  f.read((char *)buffer.data(), file_size);

  return buffer;
}

void write_file_string(std::string_view path, const std::string &content) {
  std::ofstream f{std::string(path),
                  std::ios_base::binary | std::ios_base::trunc};
  if (!f.is_open())
    return;

  f.write(content.data(), content.size());
}

void write_file(std::string_view path, const std::vector<uint8_t> &content) {
  std::ofstream f{std::string(path),
                  std::ios_base::binary | std::ios_base::trunc};
  if (!f.is_open())
    return;

  f.write((const char *)content.data(), content.size());
}

} // namespace fs

namespace crypto {

std::optional<std::string> get_last_error() {
  auto err = ERR_get_error();
  if (err == 0) {
    return std::nullopt;
  }

  return std::string(ERR_reason_error_string(err));
}

bool verify_signature(const std::string &content, const std::string &signature,
                      const std::string &public_key) {
  BIO *public_key_bio = BIO_new_mem_buf(public_key.data(), public_key.size());
  EVP_PKEY *raw_public_key =
      PEM_read_bio_PUBKEY(public_key_bio, NULL, NULL, NULL);
  BIO_free(public_key_bio);
  DEFER({ EVP_PKEY_free(raw_public_key); });

  EVP_MD_CTX *md_context = EVP_MD_CTX_new();
  if (!md_context) {
    whl::logger::error("Failed to initialize EVP_MD_CTX: {}", get_last_error());
    return false;
  }
  DEFER({ EVP_MD_CTX_free(md_context); });

  if (EVP_DigestVerifyInit(md_context, NULL, EVP_sha256(), NULL,
                           raw_public_key) <= 0) {
    whl::logger::error("Failed to initialize EVP_DigestVerify: {}",
                       get_last_error());
    return false;
  }

  int result = EVP_DigestVerify(
      md_context, (unsigned char *)signature.data(), signature.size(),
      (unsigned char *)content.data(), content.size());

  if (result < 0) {
    whl::logger::error("Failed to verify signature: {}", get_last_error());
    return false;
  }

  return result == 1;
}

std::string base64_encode(const std::vector<uint8_t> &data) {
  if (data.empty()) {
    return "";
  }

  EVP_ENCODE_CTX *ctx = EVP_ENCODE_CTX_new();
  DEFER({ EVP_ENCODE_CTX_free(ctx); });

  size_t output_size{4 * (((data.size() - 1) / 3) + 1)};
  std::string output(output_size, 0);
  EVP_EncodeInit(ctx);
  int written_size =
      EVP_EncodeBlock((unsigned char *)output.data(), data.data(), data.size());
  output.resize(written_size);

  return output;
}

std::optional<std::vector<uint8_t>> base64_decode(const std::string &input) {
  if (input.empty()) {
    return {};
  }

  EVP_ENCODE_CTX *ctx = EVP_ENCODE_CTX_new();
  DEFER({ EVP_ENCODE_CTX_free(ctx); });

  EVP_DecodeInit(ctx);
  bool done{false};
  int bytes_decoded{0};
  uint8_t buffer[128];
  auto current_input = input.begin();
  auto input_end = input.end();
  std::vector<uint8_t> output;
  while (!done) {
    auto input_limit = std::min(current_input + 64, input_end);
    auto input_length = std::distance(current_input, input_limit);
    int result = EVP_DecodeUpdate(ctx, buffer, &bytes_decoded,
                                  (const unsigned char *)&current_input[0],
                                  input_length);

    std::copy(buffer, buffer + bytes_decoded, std::back_inserter(output));
    current_input = input_limit;

    if (result == 0) {
      done = true;
    } else if (current_input == input_end) {
      result = EVP_DecodeFinal(ctx, buffer, &bytes_decoded);
      if (result == 1) {
        std::copy(buffer, buffer + bytes_decoded, std::back_inserter(output));
        done = true;
      }
      break;
    } else if (result == -1) {
      break;
    }
  }

  if (!done) {
    return std::nullopt;
  }
  return output;
}

} // namespace crypto
} // namespace whl
