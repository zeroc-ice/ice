//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export class Exception extends Error {
    constructor(...params) {
        super(...params);
        this.name = this.constructor._id.substr(2).replace(/::/g, ".");
    }

    ice_id() {
        return this.constructor._id;
    }

    static get _id() {
        return "::Ice::Exception";
    }
}
