// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

const std = @import("std");
const ffi = @import("ffi.zig");

comptime {
    // Ensure that required function are exported. More details in the `ffi.zig` file.
    _ = ffi;
}

pub const std_options = struct {
    pub const log_level = .debug;

    // Overrides the log function to use the native logger (via the `print` extern function).
    pub fn logFn(
        comptime message_level: std.log.Level,
        comptime scope: @TypeOf(.enum_literal),
        comptime format: []const u8,
        args: anytype,
    ) void {
        const level_txt = comptime message_level.asText();
        const prefix2 = if (scope == .default) ": " else "(" ++ @tagName(scope) ++ "): ";

        var buffer: [4096]u8 = undefined;
        var stream = std.io.fixedBufferStream(&buffer);
        var writer = stream.writer();

        writer.print(level_txt ++ prefix2 ++ format, args) catch return;

        const str: []const u8 = stream.getWritten();

        print(str.ptr, str.len);
    }
};

// Function exposed by the native side.

/// Register a task to be called repeatedly every `delay_us` microseconds. The return value is the ID of the task.
extern fn repeatTask(function: *const anyopaque, userdata: ?*anyopaque, delay_us: u32) u32;

/// Cancel a task given its ID.
extern fn cancelTask(task_id: u32) void;

/// Send a message to the native scheduler.
extern fn sendMessage(scheduler_id: u32, message: *const anyopaque) void;

/// Print a message using the native logger.
extern fn print(str: [*]const u8, len: u32) void;

/// Type of message that the WASM module is able to process.
const MessageType = enum(c_int) {
    /// A noop operation.
    noop,
    /// An update operation.
    update,
    /// A request to query the state.
    query,
};

/// The message structure that the WASM module is able to process.
/// This is a hand-rolled tagged union and must be kept in sync with the corresponding C++ struct.
const Message = extern struct {
    /// Type of the message.
    type: MessageType,
    /// The payload of the message.
    payload: extern union {
        noop_value: void,
        update: UpdatePayload,
        query: void,
    },
};

/// An update payload.
const UpdatePayload = extern struct {
    /// A pointer to the new name.
    name_ptr: [*]const u8,
    /// The length of the new name.
    name_len: u32,
    /// The new value to store.
    value: u32,
};

/// Type of message that the native module is able to process.
const GlobalMessageType = enum(c_int) {
    /// A noop operation.
    noop,
    /// An answer to a query message.
    state,
};

/// The message structure that the native module is able to process.
/// This is a hand-rolled tagged union and must be kept in sync with the corresponding C++ struct.
const GlobalMessage = extern struct {
    type: GlobalMessageType,
    payload: extern union {
        noop: void,
        state: StatePayload,
    },
};

/// A query payload.
const StatePayload = UpdatePayload;

/// The WASM module context.
/// This is created when the `init` function is called and destroyed when the `deinit` function is called.
/// Passing this to the function that we call from the native side is required for book-keeping.
/// Note: A global variable would also have worked instead.
const GlobalContext = struct {
    /// The stored named.
    name: []const u8 = &.{},
    /// The stored value.
    value: u32 = 0,
    /// The ID of the native scheduler.
    global_scheduler_id: ?u32 = null,
};

/// Initialize the WASM module.
export fn init() *GlobalContext {
    std.log.debug("Init function called", .{});

    const global_context = std.heap.wasm_allocator.create(GlobalContext) catch unreachable;

    const f = struct {
        pub fn callback(inner_context: *anyopaque, userdata: ?*anyopaque) void {
            greeter(@ptrCast(@alignCast(inner_context)), userdata);
        }
    }.callback;
    _ = repeatTask(@ptrCast(&f), null, 1_000_000);

    return global_context;
}

/// Deinitialize the WASM module.
export fn deinit(context: *GlobalContext) void {
    std.heap.wasm_allocator.destroy(context);
    std.log.debug("Deinit function called", .{});
}

/// Function called when the module's scheduler receives a message.
export fn onMessage(context: *GlobalContext, message: *const Message) void {
    switch (message.type) {
        .noop => {},
        .update => {
            std.log.debug("Message payload: {{ .value = {}, .name = {s} }}", .{ message.payload.update.value, message.payload.update.name_ptr[0..message.payload.update.name_len] });
            const payload = message.payload.update;
            std.heap.wasm_allocator.free(context.name);
            context.name = std.heap.wasm_allocator.dupe(u8, payload.name_ptr[0..payload.name_len]) catch unreachable;
            context.value = message.payload.update.value * 5;
        },
        .query => {
            const answer = GlobalMessage{
                .type = GlobalMessageType.state,
                .payload = .{
                    .state = StatePayload{
                        .name_ptr = context.name.ptr,
                        .name_len = context.name.len,
                        .value = context.value,
                    },
                },
            };
            sendMessage(context.global_scheduler_id.?, &answer);
        },
    }
}

/// Function called to register a scheduler to the WASM module. This is useful when we want the module to be able to send messages to another module.
export fn registerScheduler(context: *GlobalContext, scheduler_id: u32, scheduler_name_ptr: [*]const u8, scheduler_name_len: u32) void {
    const scheduler_name = scheduler_name_ptr[0..scheduler_name_len];
    if (std.mem.eql(u8, scheduler_name, "global")) {
        context.global_scheduler_id = scheduler_id;
        std.log.debug("Global scheduler registered with id={}", .{scheduler_id});
    }
}

fn greeter(context: *GlobalContext, _: ?*anyopaque) void {
    std.log.info("Hello \"{s}\"", .{context.name});
}
