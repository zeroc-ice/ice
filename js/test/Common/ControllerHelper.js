// Copyright (c) ZeroC, Inc.

import { Ice } from "ice";

export class ControllerHelper {
    constructor(exe, output) {
        if (exe === "Server.js" || exe === "ServerAMD.js") {
            this._serverReady = new Ice.Promise();
        }
        this._output = output;
    }

    serverReady(ex) {
        if (this._serverReady) {
            if (ex) {
                this._serverReady.reject(ex);
            } else {
                this._serverReady.resolve();
            }
        }
    }

    async waitReady() {
        if (this._serverReady) {
            await this._serverReady;
        }
    }

    write(msg) {
        this._output.write(msg);
    }

    writeLine(msg) {
        this._output.writeLine(msg);
    }
}
