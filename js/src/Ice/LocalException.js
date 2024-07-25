//
// Ice.LocalException
//

import { Exception } from "./Exception.js";

export class LocalException extends Exception {
    constructor(message, cause) {
        super(message, { cause: cause });
    }
}
