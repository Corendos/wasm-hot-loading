const std = @import("std");

/// Try to allocate a slice of memory of size `size` and returns it if successful.
export fn allocate(size: usize) ?[*]u8 {
    const result = std.heap.wasm_allocator.alloc(u8, size) catch return null;
    return result.ptr;
}

/// Free a slice of memory of size `size` starting at `ptr`.
export fn free(ptr: [*]u8, size: usize) void {
    std.heap.wasm_allocator.free(ptr[0..size]);
}

/// Signature of functions that can be called from the native side.
pub const CallFunction = *const fn (context: *anyopaque, userdata: ?*anyopaque) void;

/// The function that allows native code to call a WASM function. This is required because when we
/// register to be called for a task, we only send the index of the function to the native side.
export fn call(raw_function: *anyopaque, context: *anyopaque, userdata: ?*anyopaque) void {
    const f: CallFunction = @ptrCast(raw_function);
    f(context, userdata);
}
