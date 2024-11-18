//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { LocalException } from "./LocalException.js";

export class RetryException extends Error {
    constructor(ex) {
        super();
        if (ex instanceof LocalException) {
            this._ex = ex;
        } else {
            DEV: console.assert(ex instanceof RetryException);
            this._ex = ex._ex;
        }
    }

    get inner() {
        return this._ex;
    }
}
