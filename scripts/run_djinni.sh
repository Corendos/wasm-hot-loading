#!/usr/bin/env bash

# Copyright 2023 - Corentin Godeau
# SPDX-License-Identifier: MIT

dir=${0%/*}
root_dir=$(readlink -f "$dir/../")

set -euo pipefail

# Run djinni generator an put generated files at the correct location.
$dir/../bin/djinni \
   --idl $root_dir/djinni/whl.djinni \
   --java-out $root_dir/platform/bridge/java/io/corendos/whl/bridge \
   --java-package "io.corendos.whl.bridge" \
   --jni-out $root_dir/platform/bridge/jni \
   --jni-namespace "whl::bridge" \
   --jni-include-cpp-prefix "whl/" \
   --jni-generate-main false \
   --objc-out $root_dir/platform/bridge/objc \
   --cpp-out $root_dir/platform/bridge/cpp \
   --cpp-header-out $root_dir/platform/bridge/include/whl \
   --cpp-namespace "whl"
