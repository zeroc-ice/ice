// Copyright (c) ZeroC, Inc.

import { ArrayUtil } from "./ArrayUtil.js";
import { HashUtil } from "./HashUtil.js";
import { StreamHelpers } from "./StreamHelpers.js";

//
// Use generic equality test from ArrayUtil.
//
const eq = ArrayUtil.eq;

function equals(other) {
    if (this === other) {
        return true;
    }

    if (other === null || other === undefined) {
        return false;
    }

    if (this.prototype !== other.prototype) {
        return false;
    }

    for (const key in this) {
        const e1 = this[key];
        const e2 = other[key];
        if (typeof e1 == "function") {
            continue; // Don't need to compare functions
        } else if (!eq(e1, e2)) {
            return false;
        }
    }
    return true;
}

function clone() {
    const other = new this.constructor();
    for (const key in this) {
        const e = this[key];
        if (e === undefined || e === null) {
            other[key] = e;
        } else if (typeof e == "function") {
            continue;
        } else if (typeof e.clone == "function") {
            other[key] = e.clone();
        } else if (e instanceof Array) {
            other[key] = ArrayUtil.clone(e);
        } else {
            other[key] = e;
        }
    }
    return other;
}

function memberHashCode(h, e) {
    if (typeof e.hashCode == "function") {
        return HashUtil.addHashable(h, e);
    } else if (e instanceof Array) {
        return HashUtil.addArray(h, e, memberHashCode);
    } else {
        const t = typeof e;
        if (e instanceof String || t == "string") {
            return HashUtil.addString(h, e);
        } else if (e instanceof Number || t == "number") {
            return HashUtil.addNumber(h, e);
        } else if (e instanceof Boolean || t == "boolean") {
            return HashUtil.addBoolean(h, e);
        }
    }
}

function hashCode() {
    let h = 5381;
    for (const key in this) {
        const e = this[key];
        if (e === undefined || e === null || typeof e == "function") {
            continue;
        }
        h = memberHashCode(h, e);
    }
    return h;
}

export function defineStruct(obj, legalKeyType, variableLength) {
    obj.prototype.clone = clone;

    obj.prototype.equals = equals;

    //
    // Only generate hashCode if this structure type is a legal dictionary key type.
    //
    if (legalKeyType) {
        obj.prototype.hashCode = hashCode;
    }

    if (obj.prototype._write && obj.prototype._read) {
        obj.write = function (os, v) {
            if (!v) {
                if (!obj.prototype._nullMarshalValue) {
                    obj.prototype._nullMarshalValue = new this();
                }
                v = obj.prototype._nullMarshalValue;
            }
            v._write(os);
        };

        obj.read = function (is, v) {
            if (!v || !(v instanceof this)) {
                v = new this();
            }
            v._read(is);
            return v;
        };

        if (variableLength) {
            StreamHelpers.FSizeOptHelper.call(obj);
        } else {
            StreamHelpers.VSizeOptHelper.call(obj);
        }
    }
    return obj;
}
