// Copyright (c) ZeroC, Inc.

import { CompactIdRegistry } from "./CompactIdRegistry.js";
import { TypeRegistry } from "./TypeRegistry.js";

//
// Ice.Value
//
export class Value {
    constructor() {
        this._iceSlicedData = null;
    }

    ice_preMarshal() {}

    ice_postUnmarshal() {}

    ice_getSlicedData() {
        return this._iceSlicedData;
    }

    _iceWrite(os) {
        os.startValue(this._iceSlicedData);
        writeImpl(this, os, this._iceMostDerivedType());
        os.endValue();
    }

    _iceRead(is) {
        is.startValue();
        readImpl(this, is, this._iceMostDerivedType());
        this._iceSlicedData = is.endValue();
    }

    //
    // These methods are used for object parameters.
    //
    static write(os, v) {
        os.writeValue(v);
    }

    static read(is) {
        const v = { value: null };
        is.readValue(o => {
            v.value = o;
        }, this);
        return v;
    }
}

export function defineValue(valueType, id, compactId = 0) {
    valueType.prototype.ice_id = function () {
        return id;
    };

    valueType.prototype._iceMostDerivedType = function () {
        return valueType;
    };

    valueType.ice_staticId = function () {
        return id;
    };

    if (compactId > 0) {
        CompactIdRegistry.set(compactId, id);
    }
}
defineValue(Value, "::Ice::Object");
TypeRegistry.declareValueType("Ice.Value", Value);

//
// Private methods
//
function writeImpl(obj, os, type) {
    //
    // The writeImpl method is a recursive method that goes down the
    // class hierarchy to marshal each slice of the class using the
    // generated _iceWriteMemberImpl method.
    //

    if (type === undefined || type === Value) {
        return; // Don't marshal anything for Ice.Value
    }

    os.startSlice(
        type.ice_staticId(),
        Object.prototype.hasOwnProperty.call(type, "_iceCompactId") ? type._iceCompactId : -1,
        Object.getPrototypeOf(type) === Value,
    );
    if (type.prototype.hasOwnProperty("_iceWriteMemberImpl")) {
        type.prototype._iceWriteMemberImpl.call(obj, os);
    }
    os.endSlice();
    writeImpl(obj, os, Object.getPrototypeOf(type));
}

function readImpl(obj, is, type) {
    //
    // The readImpl method is a recursive method that goes down the
    // class hierarchy to unmarshal each slice of the class using the
    // generated _iceReadMemberImpl method.
    //

    if (type === undefined || type === Value) {
        return; // Don't unmarshal anything for Ice.Value
    }

    is.startSlice();
    if (type.prototype.hasOwnProperty("_iceReadMemberImpl")) {
        type.prototype._iceReadMemberImpl.call(obj, is);
    }
    is.endSlice();
    readImpl(obj, is, Object.getPrototypeOf(type));
}
