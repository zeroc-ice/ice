// Copyright (c) ZeroC, Inc.

import { Timer } from "./Timer.js";

class P extends Promise {
    constructor(cb) {
        let res;
        let rej;
        super((resolve, reject) => {
            res = resolve;
            rej = reject;

            if (cb) {
                cb(resolve, reject);
            }
        });

        this.resolve = res;
        this.reject = rej;
    }

    static get [Symbol.species]() {
        return P;
    }

    static delay(ms, value) {
        return new P(resolve => Timer.setTimeout(() => resolve(value), ms));
    }
}

export { P as Promise };
