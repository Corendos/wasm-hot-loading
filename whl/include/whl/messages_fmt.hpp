// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#pragma once

#include <whl/messages.hpp>

#include <fmt/format-inl.h>

template <> struct fmt::formatter<whl::SampleMessageType> {
  constexpr auto parse(format_parse_context &ctx)
      -> format_parse_context::iterator {
    return ctx.begin();
  }

  auto format(whl::SampleMessageType message_type, fmt::format_context &ctx)
      -> fmt::format_context::iterator {
    switch (message_type) {
    case whl::SampleMessageType::NOOP:
      return fmt::format_to(ctx.out(), "noop");
      break;
    case whl::SampleMessageType::UPDATE:
      return fmt::format_to(ctx.out(), "update");
      break;
    case whl::SampleMessageType::GET_STATE:
      return fmt::format_to(ctx.out(), "get_state");
      break;
    default:
      break;
    }

    return fmt::format_to(ctx.out(), "<unknown>");
  }
};

template <> struct fmt::formatter<whl::SampleMessage> {
  constexpr auto parse(format_parse_context &ctx)
      -> format_parse_context::iterator {
    return ctx.begin();
  }

  auto format(whl::SampleMessage message, fmt::format_context &ctx)
      -> fmt::format_context::iterator {
    switch (message.type) {
    case whl::SampleMessageType::NOOP:
      return fmt::format_to(ctx.out(), "{} {{ }}", message.type);
      break;
    case whl::SampleMessageType::UPDATE:
      return fmt::format_to(ctx.out(), "{} {{ .name = \"{}\", .value = {} }}",
                            message.type,
                            std::string_view{message.payload.update.name_ptr,
                                             message.payload.update.name_len},
                            message.payload.update.value);
      break;
    case whl::SampleMessageType::GET_STATE:
      return fmt::format_to(ctx.out(), "{} {{ }}", message.type);
      break;
    default:
      break;
    }

    return fmt::format_to(ctx.out(), "{} {{ ? }}", message.type);
  }
};

template <> struct fmt::formatter<whl::GlobalMessageType> {
  constexpr auto parse(format_parse_context &ctx)
      -> format_parse_context::iterator {
    return ctx.begin();
  }

  auto format(whl::GlobalMessageType message_type, fmt::format_context &ctx)
      -> fmt::format_context::iterator {
    switch (message_type) {
    case whl::GlobalMessageType::NOOP:
      return fmt::format_to(ctx.out(), "noop");
      break;
    case whl::GlobalMessageType::STATE:
      return fmt::format_to(ctx.out(), "state");
      break;
    default:
      break;
    }

    return fmt::format_to(ctx.out(), "<unknown>");
  }
};

template <> struct fmt::formatter<whl::GlobalMessage> {
  constexpr auto parse(format_parse_context &ctx)
      -> format_parse_context::iterator {
    return ctx.begin();
  }

  auto format(whl::GlobalMessage message, fmt::format_context &ctx)
      -> fmt::format_context::iterator {
    switch (message.type) {
    case whl::GlobalMessageType::NOOP:
      return fmt::format_to(ctx.out(), "{} {{ }}", message.type);
      break;
    case whl::GlobalMessageType::STATE: {
      std::string_view name{message.payload.state.name_ptr,
                            message.payload.state.name_len};
      return fmt::format_to(ctx.out(), "{} {{ .name = \"{}\", .value = {} }}",
                            message.type, name, message.payload.state.value);
    } break;
    default:
      break;
    }

    return fmt::format_to(ctx.out(), "{} {{ ? }}", message.type);
  }
};
