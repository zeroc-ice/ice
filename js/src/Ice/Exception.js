//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const toString = function (key, object, objectTable, ident) {
    ident += "  ";
    if (object === null) {
        return "\n" + ident + key + ": (null)";
    }
    if (object === undefined) {
        return "\n" + ident + key + ": (undefined)";
    }
    if (key == "stack" || typeof object == "function") {
        return "";
    }
    if (typeof object != "object") {
        return "\n" + ident + key + ': "' + object + '"';
    }
    if (objectTable.indexOf(object) != -1) {
        return "\n" + ident + key + ": (recursive)";
    }

    objectTable.push(object);
    let s = "\n" + ident + key + ":";
    for (const k in object) {
        if (key.indexOf("_") === 0) {
            continue;
        }

        if (typeof object[k] == "function") {
            continue;
        }
        s += ident + toString(k, object[k], objectTable, ident);
    }
    return s;
};

//
// Ice.Exception
//
export class Exception extends Error {
    constructor(cause) {
        super();
        if (cause) {
            this.ice_cause = cause;
        }
    }

    ice_id() {
        return this.constructor._id;
    }

    static get _id() {
        return "::Ice::Exception";
    }

    toString() {
        //
        // We have a guard here to prevent being re-entered. With some browsers, accessing
        // the stack property ends up calling toString on the exception to print it out with the
        // stack.
        //
        if (this._inToStringAlready) {
            return "";
        }

        this._inToStringAlready = true;
        let s = this.ice_id();
        for (const key in this) {
            if (key != "_inToStringAlready") {
                s += toString(key, this[key], [], "");
            }
        }
        this._inToStringAlready = false;
        return s;
    }

    static captureStackTrace(object) {
        const stack = new Error().stack;
        if (stack !== undefined) {
            Object.defineProperty(object, "stack", {
                get: function () {
                    return stack;
                },
            });
        }
    }
}
