//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export class Exception extends Error {
    ice_id() {
        return this.constructor._id;
    }

    static get _id() {
        return "::Ice::Exception";
    }
}
