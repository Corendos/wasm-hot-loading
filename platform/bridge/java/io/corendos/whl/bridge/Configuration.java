// AUTOGENERATED FILE - DO NOT MODIFY!
// This file was generated by Djinni from whl.djinni

package io.corendos.whl.bridge;

/**
 * Copyright 2023 - Corentin Godeau
 * SPDX-License-Identifier: MIT
 * Configuration to instantiate a wasm_hot_loading instance.
 */
public final class Configuration {


    /*package*/ final String sampleModuleUrl;

    /*package*/ final String publicKey;

    /*package*/ final int wasmStackSize;

    public Configuration(
            String sampleModuleUrl,
            String publicKey,
            int wasmStackSize) {
        this.sampleModuleUrl = sampleModuleUrl;
        this.publicKey = publicKey;
        this.wasmStackSize = wasmStackSize;
    }

    /** The URL to retrieve the Wasm module to load. */
    public String getSampleModuleUrl() {
        return sampleModuleUrl;
    }

    /** The public key used to verify module's signature. */
    public String getPublicKey() {
        return publicKey;
    }

    /** The stack size to use for the Wasm execution environment. */
    public int getWasmStackSize() {
        return wasmStackSize;
    }

    @Override
    public String toString() {
        return "Configuration{" +
                "sampleModuleUrl=" + sampleModuleUrl +
                "," + "publicKey=" + publicKey +
                "," + "wasmStackSize=" + wasmStackSize +
        "}";
    }

}
