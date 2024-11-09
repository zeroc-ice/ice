//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export class Debug {
    static assert(condition, message) {
        if (!condition) {
            console.trace(message === undefined ? "Assertion failed" : `Assertion failed: ${message}`);
            if (typeof process !== "undefined") {
                process.exit(1);
            }
        }
    }
}
