// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#include <whl/configuration.hpp>
#include <whl/default_sample_module.hpp>
#include <whl/logger.hpp>
#include <whl/messages_fmt.hpp>
#include <whl/network/http.hpp>
#include <whl/sample_module.hpp>
#include <whl/state_listener.hpp>
#include <whl/thread.hpp>
#include <whl/utils.hpp>
#include <whl/wasm_hot_loading.hpp>

#include <wasm/environment.hpp>

namespace whl {

/**
 * Retrieve the WASM module from the specified URL. The public key is used to
 * check that the module is signed. Returns std::nullopt in case of failure.
 * */
std::optional<std::string>
retrieve_wasm_module(const std::string &wasm_module_url,
                     const std::string &public_key) {
  std::string wasm_module_signature_url = wasm_module_url + ".sig";

  HttpResponse signature_response =
      make_http_request(wasm_module_signature_url);
  if (signature_response.status != 200) {
    fmt::println(stderr, "Failed to retrieve module signature");
    return std::nullopt;
  }

  std::vector<uint8_t> signature =
      crypto::base64_decode(signature_response.body).value();

  HttpResponse module_response = make_http_request(wasm_module_url);

  if (module_response.status != 200) {
    fmt::println(stderr, "Failed to retrieve module");
    return std::nullopt;
  }

  bool verified = crypto::verify_signature(
      module_response.body, std::string(signature.cbegin(), signature.cend()),
      public_key);

  if (!verified) {
    fmt::println(stderr, "Module verification failed");
    return std::nullopt;
  }

  return std::move(module_response.body);
}

/**
 * Load the WASM module from the given arguments.
 * If an URL is given it will load the module from it, otherwise it falls back
 * on the embedded default module.
 * */
std::optional<wasm::Module> load_module(wasm::Environment &environment,
                                        const std::string &wasm_module_url,
                                        const std::string &public_key) {
  if (wasm_module_url.empty()) {
    logger::info("No module URL specified, fallback on embedded module");
    return environment.parse_module(
        std::string(default_wasm_module, sizeof(default_wasm_module)));
  }

  auto module_opt = retrieve_wasm_module(wasm_module_url, public_key);
  if (!module_opt.has_value()) {
    logger::info("Failed to load module with the specified URL, fallback on "
                 "embedded module");
    return environment.parse_module(
        std::string(default_wasm_module, sizeof(default_wasm_module)));
  }

  return environment.parse_module(std::move(module_opt.value()));
}

// Interface for classes that can handle message sent to a Scheduler.
class GlobalListener {
public:
  // Called when a message is received on the scheduler.
  virtual void on_message(GlobalMessage message) = 0;
};

class WasmHotLoadingImpl : public WasmHotLoading, public GlobalListener {
public:
  WasmHotLoadingImpl(wasm::Environment environment, wasm::Runtime runtime,
                     const Configuration &configuration)
      : m_environment{std::move(environment)}, m_runtime{std::move(runtime)} {
    // Create the scheduler associated with the SampleModule (which is a small
    // wrapper between the WASM module and the C++ code).
    m_sample_scheduler = std::make_unique<
        task::Scheduler<SampleMessage, task::MessageListener<SampleMessage>>>(
        "sample");

    // Create the scheduler associated with the main entry point.
    m_global_scheduler =
        std::make_unique<task::Scheduler<GlobalMessage, GlobalListener>>(
            "global");
    SampleModuleConfiguration sample_module_configuration{
        .wasm_stack_size = static_cast<uint32_t>(configuration.wasm_stack_size),
    };

    // Create the sample module.
    m_sample_module = std::make_unique<SampleModule>(
        m_runtime, sample_module_configuration, m_sample_scheduler.get(),
        m_global_scheduler.get());

    // Associate ourselves as a Listener of the GlobalScheduler and start the
    // scheduler loop in its own thread.
    m_global_scheduler->set_listener(this);
    m_global_scheduler->start();
    m_global_scheduler_thread = Thread([this] { m_global_scheduler->run(); });

    // Associate the sample module as a Listener of the SampleScheduler and
    // start the scheduler loop in its own thread.
    // NOTE(Corentin): m_sample_module must be pinned to memory due to that.
    m_sample_scheduler->set_listener(m_sample_module.get());
    m_sample_scheduler->start();
    m_sample_scheduler_thread = Thread([this] { m_sample_scheduler->run(); });
  }

  ~WasmHotLoadingImpl() override {
    m_sample_scheduler->stop();
    if (m_sample_scheduler_thread.joinable()) {
      m_sample_scheduler_thread.join();
    }

    m_global_scheduler->stop();
    if (m_global_scheduler_thread.joinable()) {
      m_global_scheduler_thread.join();
    }
  }

  void register_state_listener(
      const std::shared_ptr<StateListener> &listener) override {
    m_state_listener = listener;
  }

  void get_state() override {
    // Send a message to the SampleScheduler. We will receive the answer as a
    // STATE message on the GlobalScheduler.
    SampleMessage message{
        .type = SampleMessageType::QUERY,
    };
    m_sample_scheduler->send(message);
  }

  void update(const std::string &name, int32_t value) override {
    // Create an UPDATE message from the given parameters and send it to the
    // SampleScheduler.
    SampleMessage message{
        .type = SampleMessageType::UPDATE,
    };
    message.payload.update =
        UpdatePayload::alloc(name, static_cast<uint32_t>(value));
    m_sample_scheduler->send(message);
  }

  void on_message(GlobalMessage message) override {
    logger::info("Received GlobalMessage: {}", message);
    switch (message.type) {
    case GlobalMessageType::STATE:
      // On UPDATE message, update the state of our state listener if we have
      // one.
      if (m_state_listener) {
        m_state_listener->on_new_state(
            message.payload.state.name(),
            static_cast<int32_t>(message.payload.state.value));
      }
      break;
    default:
      break;
    }
  }

private:
  // WASM related variables.
  wasm::Environment m_environment;
  wasm::Runtime m_runtime;

  // Bridge related variables.
  std::shared_ptr<StateListener> m_state_listener{nullptr};

  // GlobalScheduler related variables.
  std::unique_ptr<task::Scheduler<GlobalMessage, GlobalListener>>
      m_global_scheduler;
  whl::Thread m_global_scheduler_thread;

  // SampleScheduler related variables.
  std::unique_ptr<
      task::Scheduler<SampleMessage, task::MessageListener<SampleMessage>>>
      m_sample_scheduler;
  whl::Thread m_sample_scheduler_thread;

  // The SampleModule wrapping the WASM module.
  std::unique_ptr<SampleModule> m_sample_module{nullptr};
};

/**
 * Create an instance of the WasmHotLoading interface. */
std::shared_ptr<WasmHotLoading>
WasmHotLoading::create(const Configuration &configuration) {
  // Try to create the WASM environment.
  auto maybe_environment = wasm::Environment::create();
  if (!maybe_environment.has_value()) {
    return nullptr;
  }

  // Try to create the WASM runtime.
  auto maybe_runtime =
      maybe_environment->create_runtime(configuration.wasm_stack_size);
  if (!maybe_runtime.has_value()) {
    return nullptr;
  }

  // Try to load the WASM module from the given configuration.
  std::optional<wasm::Module> maybe_module =
      load_module(*maybe_environment, configuration.sample_module_url,
                  configuration.public_key);
  if (!maybe_module.has_value()) {
    return nullptr;
  }

  // Try to load the module on the runtime.
  maybe_runtime->load_module(std::move(maybe_module.value()));

  return std::make_shared<WasmHotLoadingImpl>(
      std::move(maybe_environment.value()), std::move(maybe_runtime.value()),
      configuration);
}

} // namespace whl
