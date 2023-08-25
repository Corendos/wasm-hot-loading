# Copyright 2023 - Corentin Godeau
# SPDX-License-Identifier: MIT

add_subdirectory(bridge)
if (NOT ANDROID)
add_subdirectory(cli)
endif()
