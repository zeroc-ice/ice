//
// Ice.LocalException
//

import { Exception } from "./Exception.js";

export class LocalException extends Exception {
    constructor(cause) {
        super(cause);
        Exception.captureStackTrace(this);
    }

    static get _id() {
        return "::Ice::LocalException";
    }
}
