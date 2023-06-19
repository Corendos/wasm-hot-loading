// Local includes
#include <whl/configuration.hpp>
#include <whl/defer.hpp>
#include <whl/sample_module.hpp>
#include <whl/utils.hpp>
#include <whl/wasm_hot_loading.hpp>

// Dependencies includes
#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <curl/curl.h>
#include <openssl/engine.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <wasm_runtime.h>

// std includes
#include <csignal>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

using namespace std::chrono_literals;
using namespace whl;

/// CLI application arguments.
struct CLIArguments {
  /// The url of the wasm module to load.
  std::string wasm_module_url;
  /// The path to the public key used for verification
  std::string public_key_path;
  /// The size of the stack that will be allocated for each instance of the
  /// module.
  uint32_t stack_size{1024 * 1024};
  /// The size of the heap that will be allocated for each instance of the
  /// module.
  uint32_t heap_size{1024 * 1024};
};

/// Promise controlling program lifetime.
static std::promise<void> m_global_promise{};

/// Signal handler that will set the promise and let the program end.
extern "C" void signal_handler(int signal) { m_global_promise.set_value(); }

int main(int argc, char **argv) {
  CLI::App app{"Wasm hot-loading sample"};

  CLIArguments cli_args{};

  app.add_option("--module-url", cli_args.wasm_module_url,
                 "The URL of the Wasm module to load");
  app.add_option("--key", cli_args.public_key_path,
                 "The Public key to use to verify Wasm module integrity")
      ->check(CLI::ExistingFile)
      ->required();
  app.add_option(
         "--stack-size", cli_args.stack_size,
         "The size of the stack that will be allocated for the module instance")
      ->transform(CLI::AsSizeValue(false));
  app.add_option(
         "--heap-size", cli_args.heap_size,
         "The size of the heap that will be allocated for the module instance")
      ->transform(CLI::AsSizeValue(false));

  CLI11_PARSE(app, argc, argv)

  std::string public_key = fs::read_file_string(cli_args.public_key_path);
  whl::Configuration config{cli_args.wasm_module_url, public_key, MB(1),
                            KB(64)};
  wasm_runtime_init();
  DEFER({ wasm_runtime_destroy(); });

  whl::SampleModule::register_native_exports();

  auto wasm_hot_loading_instance = whl::WasmHotLoading::create(config);

  wasm_hot_loading_instance->update("Hello, world", 42);
  wasm_hot_loading_instance->get_state();

  std::signal(SIGINT, signal_handler);

  m_global_promise.get_future().get();

  return 0;
}
