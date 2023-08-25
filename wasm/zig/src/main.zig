// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

const std = @import("std");

extern fn repeat_task(function_index: i32, userdata: ?*anyopaque, delay_us: u32) u32;
extern fn cancel_task(task_id: u32) void;
extern fn send_message(scheduler_id: u32, message: *const anyopaque) void;
extern fn print(str: [*]const u8, len: u32) void;

pub const std_options = struct {
    pub const log_level = .debug;
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

        var str: []const u8 = stream.getWritten();

        print(str.ptr, str.len);
    }
};

const MessageType = enum(c_int) {
    noop,
    update,
    get_state,
};

const UpdatePayload = extern struct {
    name_ptr: [*]const u8,
    name_len: u32,
    value: u32,
};

const Message = extern struct {
    type: MessageType,
    payload: extern union {
        noop_value: void,
        update: UpdatePayload,
        get_state: void,
    },
};

const StatePayload = UpdatePayload;

const GlobalMessageType = enum(c_int) {
    noop,
    state,
};

const GlobalMessage = extern struct {
    type: GlobalMessageType,
    payload: extern union {
        noop: void,
        state: StatePayload,
    },
};

const GlobalContext = struct {
    name: []const u8 = &.{},
    counter: u32 = 0,
    global_scheduler_id: ?u32 = null,
};

export fn init() *GlobalContext {
    std.log.info("Init function called", .{});

    const global_context = std.heap.wasm_allocator.create(GlobalContext) catch unreachable;

    return global_context;
}

export fn deinit(context: *GlobalContext) void {
    std.heap.wasm_allocator.destroy(context);
    std.log.debug("Deinit function called", .{});
}

export fn on_message(context: *GlobalContext, message: *const Message) void {
    std.log.debug("Received message: {}", .{message.*});
    switch (message.type) {
        .noop => {},
        .update => {
            const payload = message.payload.update;
            std.heap.wasm_allocator.free(context.name);
            context.name = std.heap.wasm_allocator.dupe(u8, payload.name_ptr[0..payload.name_len]) catch unreachable;
            context.counter = message.payload.update.value * 10;
        },
        .get_state => {
            const answer = GlobalMessage{
                .type = GlobalMessageType.state,
                .payload = .{
                    .state = StatePayload{
                        .name_ptr = context.name.ptr,
                        .name_len = context.name.len,
                        .value = context.counter,
                    },
                },
            };
            send_message(context.global_scheduler_id.?, &answer);
        },
    }
}

export fn register_scheduler(context: *GlobalContext, scheduler_id: u32, scheduler_name_ptr: [*]const u8, scheduler_name_len: u32) void {
    const scheduler_name = scheduler_name_ptr[0..scheduler_name_len];
    if (std.mem.eql(u8, scheduler_name, "global")) {
        context.global_scheduler_id = scheduler_id;
        std.log.debug("Global scheduler registered with id={}", .{scheduler_id});
    }
}
