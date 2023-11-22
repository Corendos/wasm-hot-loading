# Wasm Hot-loading sample

This directory contains a proof-of-concept of Wasm module hot-loading in native apps.

It's composed of three main parts:
* The sample core
* The sample Wasm module
* The server serving the Wasm module

## The Sample Core

The sample core is responsible to wrap the Wasm Module into a C++ usable interface. The code for that is under the `whl` namespace and in the `whl/` sub-folder.

It also contains convenience wrapper for the Wasm runtime we use, [wasm3](https://github.com/wasm3/wasm3). It simply wraps the C interface in C++ classes.

## The Wasm module

The Wasm module is available in the `wasm/zig/` sub-folder. It's written in the [Zig](https://ziglang.org/) programming language because of its first-class support of Wasm.

## The server

In the `server/` subfolder, there is a very basic python server used to serve the Wasm module compiled from Zig sources. More details in the associated [README](./server/README.md).

# How to build

## Cloning the repository

First, when cloning this repository, don't forget to get the submodules as well. If you're first cloning the repository you can do so with:

```bash
git clone git@github.com:Corendos/wasm-hot-loading.git --recursive
```

If you forgot, you can always fetch the submodules afterwards with:

```bash
git submodules update --init --recursive
```

## Dependencies

The required dependencies are:
* [CMake](https://cmake.org/download/) - Any version >= 3.19 will work and you'll get an error message otherwise.
* [Zig](https://ziglang.org/download/) - Zig is a pre-1.0 language so it's moving fast. This sample was tested with version `0.12.0-dev.1744+f29302f91`.
* [Go](https://go.dev/doc/install) - Go language (This is a transitive dependency of BoringlSSL).

## Building the Sample Core

You can build the sample CLI with:

```bash
# Configure the project
cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug .
# Build the sample
cmake --build build --target sample --parallel
```

Alternatively, you can build all the targets with:

```bash
cmake --build build --target all --parallel
```

## Running the sample

The sample is a CLI executable and includes an usage string. Here is its content:

```
Wasm hot-loading sample
Usage: ./build/bin/sample [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  --module-url TEXT           The URL of the Wasm module to load
  --key TEXT:FILE REQUIRED    The Public key to use to verify Wasm module integrity
  --stack-size UINT:SIZE [b, kb(=1024b), ...]
                              The size of the stack that will be allocated for the module instance
```

## Building the sample Wasm module

To build the sample Wasm module, you simply have to run the following command:

```bash
# Go to the module source directory
cd wasm/zig
# Build it, optionally compiling it in ReleaseSmall mode
zig build -Drelease=<true|false>
```

This will produce the `zig-out/bin/module.wasm` artifact that will ultimately be served by the Python Flask application.

## Running the local server

If not already done, initialize a virtualenv locally and install the dependencies using the `requirements.txt` file. Then, activate this virtual environment. Alternatively, if you know what you are doing, you can install the dependencies in the global Python interpreter environment.

To run the Python Flask server, first update the `server/config.json` file if needed and then run the Flask app with:

```bash
cd server
flask --app server run
```

## Step-by-Step tutorial

If you want to see results quickly, here is a fully detailed process of everything required to test this sample:

```bash
# Assuming your current directory is the root of the repository

# Create a Python3 virtual environment
python3 -m venv venv
# Activate this environment
source ./venv/bin/activate
# Install Python deps
pip install -r requirements.txt

# Go the the Wasm sample module directory
cd wasm/zig/
# Build the module
zig build -Drelease=true
# Go back to repository root
cd ../..

# Configure C++ project
cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug .
# Build C++ sample
cmake --build build --target sample --parallel

# /!\ In another terminal /!\
cd server
# Start Flask app with default config
flask --app server run
# /!\ In antoher terminal /!\


# Launch the sample with Flask URL and default public key
./build/bin/sample --module-url http://127.0.0.1:5000/wasm/module.wasm --key certs/pubkey.pem
```

This should run the sample and print some message originating from the Wasm module.

# How it works

First of all, we need to define the `Scheduler` notion. A `Scheduler` is the main building-block of this sample design. It has two roles:
* Executing tasks once at a specific time or repeatedly at specific intervals.
* Handling messages

The `Scheduler` type is a template class with two templates, the type of message it can handle and the type of message handler it can notify. Generally speaking, the message handle is also a template class using the message type, so we are closer to having only one template type.

A `Scheduler` executes scheduled tasks and message handler on a unique thread, ensuring a mono-threaded context. This is a strong guarantee as it enable us to avoid any synchronization primitives in the part of the code we know are running in the context of a Scheduler.

In this sample, we have two separate `Scheduler`. The "global" `Scheduler` whose role is to handle all the "default" messages and the "Sample" `Scheduler` whose role is to handle all the messages and tasks that will ultimately be handled/run in Wasm code.

The `whl::GlobalMessageType` and `whl::GlobalMessage` types represent the messages that the "Global" `Scheduler` can handle and the `whl::SampleMessageType` and `whl::SampleMessage` types represent the messages that the "Sample" `Scheduler` can handle. They have their counterparts declared in the Zig code as well and they need to stay in sync.

If you take a look at the code (more specifically the [`whl/src/whl/sample_module.cpp`](./whl/src/whl/sample_module.cpp) file), you can see that the `SampleModule` class handles all the interaction with the Wasm Module and is handling messages sent to the "Sample" `Scheduler`. It's also exposing the "Global" `Scheduler` through a `Sender` interface so that the Wasm Module can send messages to it.

Finally, all of this logic is wrapped in the `whl::WasmHotLoading` class (defined in [`platform/bridge/cpp/wasm_hot_loading.cpp`](./platform/bridge/cpp/wasm_hot_loading.cpp)), so that's easy to use from any platform (more details on that later).

On top of that, a default Wasm module is embedded inside the built binary. It's generated from the Zig code in the `wasm/zig` folder and declared as an array literal in the `whl/include/whl/default_sample_module.hpp` file. To generate this file, I wrote a small Python script that can be found in `scripts/convert.py`. It's documented and pretty straightforward.

This let us fallback to a known and tested module implementation if needed.

# Running it on and Android device

Such a sample would not be complete if there wasn't something out of the ordinary. To show that this solution is multi-platform, I integrated it inside an Android app.

It relies on a fork of [djinni](https://djinni.xlcpp.dev/) to generate bindings for Java and Objective-C. In the previous section, I talked about the `whl::WasmHotLoading` class. In practice, this class is an interface exposed in Java and Objective-C with an implementation in C++. This way, we can use the class seamlessly in a CLI app or Android app.

This interface is defined using Djinni's Interface Definition Language ([IDL](https://djinni.xlcpp.dev/djinni-generator/idl/)) in the `djinni/whl.djinni` file. This file contains some comment to help you understand how it's supposed to be used.

If you want to update this schema, you will need to regenerate the bindings using the `scripts/run_djinni.sh` script. Otherwise, the default version is committed with this repository and should work out-of-the box.

To see the application in action, simply open the Android project in the `platform/android` folder and build it.
