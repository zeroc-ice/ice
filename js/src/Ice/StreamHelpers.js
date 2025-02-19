// Copyright (c) ZeroC, Inc.

import { OptionalFormat } from "./OptionalFormat.js";
import { InputStream } from "./InputStream.js";
import { OutputStream } from "./OutputStream.js";
import { TypeRegistry } from "./TypeRegistry.js";

const defineProperty = Object.defineProperty;

function defineBuiltinHelper(write, read, sz, format, min, max) {
    const helper = class {
        static write(os, v) {
            return write.call(os, v);
        }

        static read(is) {
            return read.call(is);
        }

        static writeOptional(os, tag, v) {
            os.writeOptionalHelper(tag, format, write, v);
        }

        static readOptional(is, tag) {
            return is.readOptionalHelper(tag, format, read);
        }

        static get minWireSize() {
            return sz;
        }
    };

    if (min !== undefined && max !== undefined) {
        helper.validate = function (v) {
            return v >= min && v <= max;
        };
    }

    return helper;
}

const istr = InputStream.prototype;
const ostr = OutputStream.prototype;

//
// Constants to use in number type range checks.
//
const MIN_UINT8_VALUE = 0x0;
const MAX_UINT8_VALUE = 0xff;

const MIN_INT16_VALUE = -0x8000;
const MAX_INT16_VALUE = 0x7fff;

const MIN_UINT32_VALUE = 0x0;
const MAX_UINT32_VALUE = 0xffffffff;

const MIN_INT32_VALUE = -0x80000000;
const MAX_INT32_VALUE = 0x7fffffff;

const MIN_FLOAT32_VALUE = -3.4028234664e38;
const MAX_FLOAT32_VALUE = 3.4028234664e38;

export const ByteHelper = defineBuiltinHelper(
    ostr.writeByte,
    istr.readByte,
    1,
    OptionalFormat.F1,
    MIN_UINT8_VALUE,
    MAX_UINT8_VALUE,
);

export const ShortHelper = defineBuiltinHelper(
    ostr.writeShort,
    istr.readShort,
    2,
    OptionalFormat.F2,
    MIN_INT16_VALUE,
    MAX_INT16_VALUE,
);

export const IntHelper = defineBuiltinHelper(
    ostr.writeInt,
    istr.readInt,
    4,
    OptionalFormat.F4,
    MIN_INT32_VALUE,
    MAX_INT32_VALUE,
);

export const FloatHelper = defineBuiltinHelper(
    ostr.writeFloat,
    istr.readFloat,
    4,
    OptionalFormat.F4,
    MIN_FLOAT32_VALUE,
    MAX_FLOAT32_VALUE,
);
FloatHelper.validate = function (v) {
    return (
        Number.isNaN(v) ||
        v == Number.POSITIVE_INFINITY ||
        v == Number.NEGATIVE_INFINITY ||
        (v >= MIN_FLOAT32_VALUE && v <= MAX_FLOAT32_VALUE)
    );
};

export const DoubleHelper = defineBuiltinHelper(
    ostr.writeDouble,
    istr.readDouble,
    8,
    OptionalFormat.F8,
    -Number.MAX_VALUE,
    Number.MAX_VALUE,
);
DoubleHelper.validate = function (v) {
    return (
        Number.isNaN(v) ||
        v == Number.POSITIVE_INFINITY ||
        v == Number.NEGATIVE_INFINITY ||
        (v >= -Number.MAX_VALUE && v <= Number.MAX_VALUE)
    );
};

export const BoolHelper = defineBuiltinHelper(ostr.writeBool, istr.readBool, 1, OptionalFormat.F1);
export const LongHelper = defineBuiltinHelper(ostr.writeLong, istr.readLong, 8, OptionalFormat.F8);
LongHelper.validate = function (v) {
    return typeof(v) ==  'number' || (typeof(v) == 'bigint' && BigInt.asIntN(64, v) === v);
};

export const StringHelper = defineBuiltinHelper(ostr.writeString, istr.readString, 1, OptionalFormat.VSize);
export const ObjectHelper = class {
    static write(os, v) {
        os.writeValue(v);
    }

    static read(is) {
        let o;
        is.readValue(v => {
            o = v;
        }, Value);
        return o;
    }

    static get minWireSize() {
        return 1;
    }
};

export const StreamHelpers = {};

StreamHelpers.FSizeOptHelper = function () {
    this.writeOptional = function (os, tag, v) {
        if (v !== undefined && v !== null && os.writeOptional(tag, OptionalFormat.FSize)) {
            const pos = os.startSize();
            this.write(os, v);
            os.endSize(pos);
        }
    };

    this.readOptional = function (is, tag) {
        let v;
        if (is.readOptional(tag, OptionalFormat.FSize)) {
            is.skip(4);
            v = this.read(is);
        }
        return v;
    };
};

StreamHelpers.VSizeOptHelper = function () {
    this.writeOptional = function (os, tag, v) {
        if (v !== undefined && v !== null && os.writeOptional(tag, OptionalFormat.VSize)) {
            os.writeSize(this.minWireSize);
            this.write(os, v);
        }
    };

    this.readOptional = function (is, tag) {
        let v;
        if (is.readOptional(tag, OptionalFormat.VSize)) {
            is.skipSize();
            v = this.read(is);
        }
        return v;
    };
};

StreamHelpers.VSizeContainerOptHelper = function (elementSize) {
    this.writeOptional = function (os, tag, v) {
        if (v !== undefined && v !== null && os.writeOptional(tag, OptionalFormat.VSize)) {
            const sz = this.size(v);
            os.writeSize(sz > 254 ? sz * elementSize + 5 : sz * elementSize + 1);
            this.write(os, v);
        }
    };

    this.readOptional = function (is, tag) {
        let v;
        if (is.readOptional(tag, OptionalFormat.VSize)) {
            is.skipSize();
            v = this.read(is);
        }
        return v;
    };
};

StreamHelpers.VSizeContainer1OptHelper = function () {
    this.writeOptional = function (os, tag, v) {
        if (v !== undefined && v !== null && os.writeOptional(tag, OptionalFormat.VSize)) {
            this.write(os, v);
        }
    };

    this.readOptional = function (is, tag) {
        let v;
        if (is.readOptional(tag, OptionalFormat.VSize)) {
            v = this.read(is);
        }
        return v;
    };
};

//
// Sequence helper to write sequences
//
class SequenceHelper {
    write(os, v) {
        if (v === null || v === undefined || v.length === 0) {
            os.writeSize(0);
        } else {
            const helper = this.elementHelper;
            os.writeSize(v.length);
            for (let i = 0; i < v.length; ++i) {
                helper.write(os, v[i]);
            }
        }
    }

    read(is) {
        const helper = this.elementHelper; // Cache the element helper.
        const sz = is.readAndCheckSeqSize(helper.minWireSize);
        const v = [];
        v.length = sz;
        for (let i = 0; i < sz; ++i) {
            v[i] = helper.read(is);
        }
        return v;
    }

    size(v) {
        return v === null || v === undefined ? 0 : v.length;
    }

    get minWireSize() {
        return 1;
    }
}

// Specialization optimized for ByteSeq
const byteSeqHelper = new SequenceHelper();
byteSeqHelper.write = (os, v) => os.writeByteSeq(v);
byteSeqHelper.read = is => is.readByteSeq();

defineProperty(byteSeqHelper, "elementHelper", { get: () => ByteHelper });
StreamHelpers.VSizeContainer1OptHelper.call(byteSeqHelper);

// Read method for value sequences
const valueSequenceHelperRead = function (is) {
    const sz = is.readAndCheckSeqSize(1);
    const v = [];
    v.length = sz;
    const elementType = this.elementType;
    const readValueAtIndex = function (idx) {
        is.readValue(obj => {
            v[idx] = obj;
        }, elementType);
    };

    for (let i = 0; i < sz; ++i) {
        readValueAtIndex(i);
    }
    return v;
};

StreamHelpers.generateSeqHelper = function (elementHelper, fixed, elementType) {
    if (elementHelper === ByteHelper) {
        return byteSeqHelper;
    }

    const helper = new SequenceHelper();
    if (fixed) {
        if (elementHelper.minWireSize === 1) {
            StreamHelpers.VSizeContainer1OptHelper.call(helper);
        } else {
            StreamHelpers.VSizeContainerOptHelper.call(helper, elementHelper.minWireSize);
        }
    } else {
        StreamHelpers.FSizeOptHelper.call(helper);
    }

    defineProperty(helper, "elementHelper", { get: () => elementHelper });

    if (elementHelper == ObjectHelper) {
        defineProperty(helper, "elementType", {
            get: () => TypeRegistry.getValueType(elementType),
        });
        helper.read = valueSequenceHelperRead;
    }

    return helper;
};

//
// Dictionary helper to write dictionaries
//
class DictionaryHelper {
    write(os, v) {
        if (v === null || v == undefined || v.size === 0) {
            os.writeSize(0);
        } else {
            const keyHelper = this.keyHelper;
            const valueHelper = this.valueHelper;
            os.writeSize(v.size);
            for (const [key, value] of v) {
                keyHelper.write(os, key);
                valueHelper.write(os, value);
            }
        }
    }

    read(is) {
        const mapType = this.mapType;
        const v = new mapType();
        const sz = is.readSize();
        const keyHelper = this.keyHelper;
        const valueHelper = this.valueHelper;
        for (let i = 0; i < sz; ++i) {
            v.set(keyHelper.read(is), valueHelper.read(is));
        }
        return v;
    }

    size(v) {
        return v === null || v === undefined ? 0 : v.size;
    }

    get minWireSize() {
        return 1;
    }
}

// Read method for dictionaries of values
function valueDictionaryHelperRead(is) {
    const sz = is.readSize();
    const mapType = this.mapType;
    const v = new mapType();
    const valueType = this.valueType;

    const readValueForKey = function (key) {
        is.readValue(obj => v.set(key, obj), valueType);
    };

    const keyHelper = this.keyHelper;
    for (let i = 0; i < sz; ++i) {
        readValueForKey(keyHelper.read(is));
    }
    return v;
}

StreamHelpers.generateDictHelper = function (keyHelper, valueHelper, fixed, valueType, mapType) {
    const helper = new DictionaryHelper();
    if (fixed) {
        StreamHelpers.VSizeContainerOptHelper.call(helper, keyHelper.minWireSize + valueHelper.minWireSize);
    } else {
        StreamHelpers.FSizeOptHelper.call(helper);
    }

    defineProperty(helper, "mapType", {
        get: () => mapType,
    });

    defineProperty(helper, "keyHelper", {
        get: () => keyHelper,
    });

    defineProperty(helper, "valueHelper", {
        get: () => valueHelper,
    });

    if (valueHelper == ObjectHelper) {
        defineProperty(helper, "valueType", {
            get: () => TypeRegistry.getValueType(valueType),
        });

        helper.read = valueDictionaryHelperRead;
    }

    return helper;
};
