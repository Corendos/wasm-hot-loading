# Copyright 2023 - Corentin Godeau
# SPDX-License-Identifier: MIT

add_executable(sample sample/main.cpp)
target_link_libraries(sample PRIVATE whlstatic CLI11::CLI11)
