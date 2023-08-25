// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

const std = @import("std");

const wasm_arch = .wasm32;
const wasm_os = .{
    .tag = .freestanding,
    .version_range = .{ .none = {} },
};

const wasm_target = std.Target{
    .cpu = std.Target.wasm.cpu.generic.toCpu(wasm_arch),
    .os = wasm_os,
    .abi = .none,
    .ofmt = std.Target.ObjectFormat.default(wasm_os.tag, wasm_arch),
};

pub fn build(b: *std.Build) !void {
    const optimize = b.standardOptimizeOption(std.Build.StandardOptimizeOptionOptions{ .preferred_optimize_mode = .ReleaseSmall });
    const target = std.zig.CrossTarget.fromTarget(wasm_target);

    const wasm_module = b.addSharedLibrary(std.Build.SharedLibraryOptions{
        .name = "module",
        .optimize = optimize,
        .target = target,
        .root_source_file = std.Build.FileSource.relative("src/main.zig"),
    });
    wasm_module.rdynamic = true;

    const wasm_module_install = b.addInstallArtifact(wasm_module, std.Build.Step.InstallArtifact.Options{
        .dest_dir = .{ .override = .{ .lib = {} } },
    });

    b.getInstallStep().dependOn(&wasm_module_install.step);
}

inline fn thisDir() [:0]const u8 {
    return comptime std.fs.path.dirname(@src().file) orelse ".";
}
