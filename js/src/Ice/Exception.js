//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export class Exception extends Error {
    constructor(...params) {
        super(...params);
        this.name = this.constructor._ice_id.substr(2).replace(/::/g, ".");
    }

    ice_id() {
        return this.constructor._ice_id;
    }
}
