// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

package io.corendos.wasmhotloading

import android.content.Context
import android.util.Log
import com.getkeepsafe.relinker.ReLinker

internal object Relinker {
    private const val TAG = "DLLBinder"

    fun linkTo(context: Context, libName: String, callback: (error: Throwable?) -> Unit) {
        ReLinker.loadLibrary(
            context.applicationContext, libName,
            object : ReLinker.LoadListener {
                override fun success() {
                    Log.i(TAG, "Successfully loaded \"${libName}\" shared library")
                    callback(null)
                }
                override fun failure(t: Throwable) {
                    Log.i(TAG, "Failed to load \"${libName}\" shared library: ${t.message}")
                    t.printStackTrace()
                    callback(t)
                }
            }
        )
    }

    fun linkTo(context: Context, libName: String): Boolean {
        return runCatching {  ReLinker.loadLibrary(context.applicationContext, libName) }.exceptionOrNull().let { t ->
            if (t == null) {
                Log.i(TAG, "Successfully loaded \"${libName}\" shared library")
                true
            } else {
                Log.i(TAG, "Failed to load \"${libName}\" shared library: ${t.message}")
                t.printStackTrace()
                false
            }
        }
    }

}
