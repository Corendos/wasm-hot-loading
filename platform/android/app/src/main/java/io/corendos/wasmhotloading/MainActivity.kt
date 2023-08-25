// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

package io.corendos.wasmhotloading

import android.os.Bundle
import android.webkit.URLUtil
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import io.corendos.wasmhotloading.databinding.ActivityMainBinding
import io.corendos.whl.bridge.StateListener
import java.nio.charset.StandardCharsets

class MainActivity : AppCompatActivity(){
    private lateinit var binding: ActivityMainBinding
    private var wasmHotLoading: WasmHotLoading? = null
    private lateinit var publicKey: String

    private val wasmStackSize = 1024 * 1024
    private val wasmHeapSize = 64 * 1024

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        binding.counterNumberPicker.minValue = 0;
        binding.counterNumberPicker.maxValue = 100;
        publicKey = String(resources.openRawResource(R.raw.pubkey).readBytes(), StandardCharsets.UTF_8)

        if (!Relinker.linkTo(this, BuildConfig.CPP_TARGET)) {
            return
        }

        binding.updateButton.isEnabled = false
        binding.queryButton.isEnabled = false

        binding.loadButton.setOnClickListener{
            loadModule()
            binding.updateButton.isEnabled = true
            binding.queryButton.isEnabled = true
        }
        binding.unloadButton.setOnClickListener{
            binding.updateButton.isEnabled = false
            binding.queryButton.isEnabled = false
            unloadModule()
        }
    }

    private fun loadModule() {
        if (wasmHotLoading != null) {
            unloadModule()
        }

        val moduleUrl = binding.sampleModuleUrlTextEdit.text.toString().takeIf { URLUtil.isValidUrl(it) }
        if (moduleUrl == null) {
            Toast.makeText(this@MainActivity, "Invalid Module URL", Toast.LENGTH_SHORT).show()
            return
        }
        wasmHotLoading = WasmHotLoading.Builder()
            .moduleUrl(moduleUrl)
            .publicKey(publicKey)
            .stackSize(wasmStackSize)
            .heapSize(wasmHeapSize)
            .build()

        wasmHotLoading?.registerStateListener(object : StateListener() {
            override fun onNewState(name: String?, value: Int) {
                binding.currentStateTextView.text =
                    getString(R.string.stateStringTemplate, name.orEmpty(), value)
            }
        })

        binding.updateButton.setOnClickListener {
            wasmHotLoading?.update(
                binding.nameTextEdit.text.toString(),
                binding.counterNumberPicker.value
            )
            wasmHotLoading?.getState()
        }
        binding.queryButton.setOnClickListener {
            wasmHotLoading?.getState()
        }
        wasmHotLoading?.getState()
    }

    private fun unloadModule() {
        binding.currentStateTextView.text = ""
        if (wasmHotLoading == null) return
        binding.updateButton.setOnClickListener(null)
        binding.queryButton.setOnClickListener(null)
        wasmHotLoading = null
    }

    override fun onStart() {
        super.onStart()
    }

    override fun onStop() {
        super.onStop()
    }
}
