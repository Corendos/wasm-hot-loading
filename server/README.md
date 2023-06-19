# Basic HTTP server

This directory contains a very basic HTTP server that is used to serve Wasm module as well as there signature.

Configuration options can be found in the `config.json` file.

It will register a route at `/wasm/<path:path>` and, depending on the extension of the requested file, will return the requested file or a signature of this file computed using the private key given in the `config.json` file.
