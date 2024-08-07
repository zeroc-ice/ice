//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

    delay(ms) {
        return this.then(
            value => new P((resolve, reject) => Timer.setTimeout(() => resolve(value), ms)),
            reason => new P((resolve, reject) => Timer.setTimeout(() => reject(reason), ms)),
        );
    }

    static get [Symbol.species]() {
        return P;
    }

    static delay(ms, value) {
        return new P(resolve => Timer.setTimeout(() => resolve(value), ms));
    }

    static try(cb) {
        return P.resolve().then(cb);
    }
}

export { P as Promise };
