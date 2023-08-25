# Copyright 2023 - Corentin Godeau
# SPDX-License-Identifier: MIT

import os
import json
import base64
from flask import Flask, abort, send_from_directory
from OpenSSL import crypto

app = Flask(__name__)
app.config.from_file("config.json", load=json.load)

# Load Private Key in memory on startup
with open(app.config["PRIVATE_KEY_FILE"], "r") as f:
    private_key=crypto.load_privatekey(crypto.FILETYPE_PEM, f.read())

@app.route("/wasm/<path:path>")
def wasm(path: str):
    filename, ext = os.path.splitext(path)
    # If the extension is not `.sig`, try to serve the file from the configured directory.
    if ext != ".sig":
        return send_from_directory(app.config["STATIC_DIRECTORY"], path)
    
    # Otherwise, try to compute the file signature using the private key and return it as a base64 payload.
    try:
        with open(os.path.join(os.fspath(app.config["STATIC_DIRECTORY"]), filename), "rb") as f:
            return base64.encodebytes(crypto.sign(private_key, f.read(), "sha256"))
    except FileNotFoundError:
        abort(404)
    except:
        abort(500)

