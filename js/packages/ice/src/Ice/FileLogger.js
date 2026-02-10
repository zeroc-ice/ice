// Copyright (c) ZeroC, Inc.

import { Logger } from "./Logger.js";
import fs from "fs";

class FileLogger extends Logger {
    constructor(prefix, filename) {
        super(prefix);
        this._filename = filename;
    }

    write(message) {
        fs.appendFileSync(this._filename, message + "\n");
    }

    cloneWithPrefix(prefix) {
        return new FileLogger(prefix, this._filename);
    }
}

export { FileLogger };
