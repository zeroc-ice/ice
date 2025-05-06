// Copyright (c) ZeroC, Inc.

import fs from "fs";

const formatMessage = message => message.join("").replace(/\n/g, "\n   ");

export class Logger {
    constructor(prefix) {
        if (prefix !== undefined && prefix.length > 0) {
            this._prefix = `${prefix}: `;
        } else {
            this._prefix = "";
        }

        this._dateformat = {
            year: "numeric",
            month: "numeric",
            day: "numeric",
            hour: "numeric",
            minute: "numeric",
            second: "numeric",
            hour12: false,
        };
    }

    print(message) {
        this.write(message);
    }

    trace(category, message) {
        const s = [];
        s.push("-- ");
        s.push(this.timestamp());
        s.push(" ");
        s.push(this._prefix);
        s.push(category);
        s.push(": ");
        s.push(message);
        this.write(formatMessage(s));
    }

    warning(message) {
        const s = [];
        s.push("-! ");
        s.push(this.timestamp());
        s.push(" ");
        s.push(this._prefix);
        s.push("warning: ");
        s.push(message);
        this.write(formatMessage(s));
    }

    error(message) {
        const s = [];
        s.push("!! ");
        s.push(this.timestamp());
        s.push(" ");
        s.push(this._prefix);
        s.push("error: ");
        s.push(message);
        this.write(formatMessage(s));
    }

    cloneWithPrefix(prefix) {
        return new Logger(prefix);
    }

    write(message) {
        console.log(message);
    }

    timestamp() {
        const d = new Date();
        return d.toLocaleString("en-US", this._dateformat) + "." + d.getMilliseconds();
    }
}

let FileLogger = null;

if (typeof fs.open === "function") {
    FileLogger = class extends Logger {
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
    };
}

export { FileLogger };
