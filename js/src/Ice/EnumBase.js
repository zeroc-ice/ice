// Copyright (c) ZeroC, Inc.

// Declared here to avoid circular dependencies when importing OptionalFormat enum.
const OptionalFormat_Size = {};
Object.defineProperty(OptionalFormat_Size, "value", { value: 4 });

//
// Ice.EnumBase
//
class EnumBase {
    constructor(name, value) {
        this._name = name;
        this._value = value;
    }

    toString() {
        return this._name;
    }

    get name() {
        return this._name;
    }

    get value() {
        return this._value;
    }
}

class EnumHelper {
    constructor(enumType) {
        this._enumType = enumType;
    }

    write(os, v) {
        this._enumType._write(os, v);
    }

    writeOptional(os, tag, v) {
        this._enumType._writeOpt(os, tag, v);
    }

    read(is) {
        return this._enumType._read(is);
    }

    readOptional(is, tag) {
        return this._enumType._readOpt(is, tag);
    }
}

export function defineEnum(enumerators) {
    const type = class extends EnumBase {};

    const enums = [];
    let maxValue = 0;
    let firstEnum = null;

    for (const idx in enumerators) {
        const e = enumerators[idx][0];
        const value = enumerators[idx][1];
        const enumerator = new type(e, value);
        enums[value] = enumerator;
        if (!firstEnum) {
            firstEnum = enumerator;
        }
        Object.defineProperty(type, e, {
            enumerable: true,
            value: enumerator,
        });
        if (value > maxValue) {
            maxValue = value;
        }
    }

    Object.defineProperty(type, "minWireSize", { get: () => 1 });

    type._write = function (os, v) {
        if (v) {
            os.writeEnum(v);
        } else {
            os.writeEnum(firstEnum);
        }
    };

    type._read = function (is) {
        return is.readEnum(type);
    };

    type._writeOpt = function (os, tag, v) {
        if (v !== undefined && v !== null) {
            if (os.writeOptional(tag, OptionalFormat_Size)) {
                type._write(os, v);
            }
        }
    };

    type._readOpt = function (is, tag) {
        return is.readOptionalEnum(tag, type);
    };

    type._helper = new EnumHelper(type);

    Object.defineProperty(type, "valueOf", {
        value: function (v) {
            let enumerator = enums[v];
            if (enumerator === undefined) {
                enumerator = new type(`${v}`, v);
            }
            return enumerator;
        },
    });

    Object.defineProperty(type, "maxValue", {
        value: maxValue,
    });

    Object.defineProperty(type.prototype, "maxValue", {
        value: maxValue,
    });

    return type;
}
