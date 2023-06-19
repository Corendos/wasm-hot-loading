#include <whl/defer.hpp>
#include <whl/network/http.hpp>

#include <curl/curl.h>

#include <string_view>
#include <utility>

namespace whl {
static size_t header_callback(char *data, size_t /* size */, size_t data_size,
                              void *userdata) {
  HttpResponse *response = reinterpret_cast<HttpResponse *>(userdata);

  std::string_view header_line{data, data_size};
  size_t column_position = header_line.find_first_of(':');
  if (column_position == std::string::npos) {
    return data_size;
  }
  std::string header_name{header_line.substr(0, column_position)};
  std::string header_value{header_line.substr(column_position + 1)};

  response->headers.emplace(std::move(header_name), std::move(header_value));

  return data_size;
}

static size_t write_callback(void *raw_data, size_t /*size*/, size_t data_size,
                             void *userdata) {
  HttpResponse *response = reinterpret_cast<HttpResponse *>(userdata);

  const char *data{reinterpret_cast<const char *>(raw_data)};
  std::copy(data, data + data_size, std::back_inserter(response->body));
  return data_size;
}

HttpResponse make_http_request(const std::string &url) {
  HttpResponse response{};

  CURL *curl_handle = curl_easy_init();
  DEFER({ curl_easy_cleanup(curl_handle); });

  curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, header_callback);
  curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, &response);

  CURLcode code = curl_easy_perform(curl_handle);

  if (code != CURLE_OK) {
    return response;
  }

  long response_code{0};
  curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
  response.status = response_code;

  return response;
}
} // namespace whl
