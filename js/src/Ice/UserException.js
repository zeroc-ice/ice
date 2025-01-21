// Copyright (c) ZeroC, Inc.

import { Exception } from "./Exception.js";

export class UserException extends Exception {
    constructor() {
        super();
    }

    _write(os) {
        os.startException();
        writeImpl(this, os, this._mostDerivedType());
        os.endException();
    }

    _read(is) {
        is.startException();
        readImpl(this, is, this._mostDerivedType());
        is.endException();
    }

    _usesClasses() {
        return false;
    }

    _mostDerivedType() {
        return UserException;
    }
}

//
// Private methods
//

const writeImpl = function (obj, os, type) {
    //
    // The writeImpl method is a recursive method that goes down the
    // class hierarchy to marshal each slice of the class using the
    // generated _writeMemberImpl method.
    //

    if (type === undefined || type === UserException) {
        return; // Don't marshal anything for Ice.UserException
    }

    os.startSlice(type._ice_id, -1, type._parent === UserException);
    if (type.prototype.hasOwnProperty("_writeMemberImpl")) {
        type.prototype._writeMemberImpl.call(obj, os);
    }
    os.endSlice();
    writeImpl(obj, os, type._parent);
};

const readImpl = function (obj, is, type) {
    //
    // The readImpl method is a recursive method that goes down the
    // class hierarchy to marshal each slice of the class using the
    // generated _readMemberImpl method.
    //

    if (type === undefined || type === UserException) {
        return; // Don't marshal anything for UserException
    }

    is.startSlice();
    if (type.prototype.hasOwnProperty("_readMemberImpl")) {
        type.prototype._readMemberImpl.call(obj, is);
    }
    is.endSlice();
    readImpl(obj, is, type._parent);
};
