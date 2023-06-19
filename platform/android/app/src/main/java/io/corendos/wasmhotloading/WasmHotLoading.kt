package io.corendos.wasmhotloading

import io.corendos.whl.bridge.Configuration
import io.corendos.whl.bridge.StateListener

typealias InnerWasmHotLoading = io.corendos.whl.bridge.WasmHotLoading

class WasmHotLoading private constructor(val innerModule: InnerWasmHotLoading) {
    data class Builder(
        var moduleUrl: String? = null,
        var publicKey: String? = null,
        var stackSize: Int? = null,
        var heapSize: Int? = null) {

        companion object {
            const val DEFAULT_STACK_SIZE = 1024 * 1024
            const val DEFAULT_HEAP_SIZE = 64 * 1024
        }


        fun moduleUrl(url: String) = apply { this.moduleUrl = url }
        fun publicKey(publicKey: String) = apply { this.publicKey = publicKey }
        fun stackSize(size: Int) = apply { this.stackSize = size }
        fun heapSize(size: Int) = apply { this.heapSize = size }
        fun build(): WasmHotLoading {
            val config = Configuration(
                moduleUrl!!,
                publicKey!!,
                stackSize ?: DEFAULT_STACK_SIZE,
                heapSize ?: DEFAULT_HEAP_SIZE
            )

            return WasmHotLoading(InnerWasmHotLoading.create(config) ?: throw Exception("Instantiation error"))
        }
    }

    fun registerStateListener(listener: StateListener?) = innerModule.registerStateListener(listener)

    fun getState() = innerModule.getState()

    fun update(name: String?, value: Int) = innerModule.update(name, value)
}