import { Instance } from "./Instance.js";
import { Communicator } from "./Communicator.js";
import { Ice as Ice_Version } from "./Version.js";
const { EncodingVersion } = Ice_Version;
import { FormatType } from "./FormatType.js";
import { InitializationException, MarshalException } from "./LocalExceptions.js";
import { Buffer } from "./Buffer.js";
import { Ice as Ice_Identity } from "./Identity.js";
const { Identity } = Ice_Identity;
import { Encoding_1_0, Protocol } from "./Protocol.js";
import { SliceType } from "./SliceType.js";
import { Value } from "./Value.js";
import { OptionalFormat } from "./OptionalFormat.js";

class WriteEncaps {
    constructor() {
        this.start = 0;
        this.format = null;
        this.encoding = null;
        this.encoding_1_0 = false;
        this.encoder = null;
        this.next = null;
    }

    reset() {
        this.encoder = null;
    }

    setEncoding(encoding) {
        this.encoding = encoding;
        this.encoding_1_0 = encoding.equals(Encoding_1_0);
    }
}

class EncapsEncoder {
    constructor(stream, encaps) {
        this._stream = stream;
        this._encaps = encaps;
        this._marshaledMap = new Map(); // Map<Value, int>;
        this._typeIdMap = null; // Lazy initialized. Map<String, int>
        this._typeIdIndex = 0;
    }

    writeOptional() {
        return false;
    }

    writePendingValues() {
        return undefined;
    }

    registerTypeId(typeId) {
        if (this._typeIdMap === null) {
            // Lazy initialization
            this._typeIdMap = new Map(); // Map<String, int>
        }

        const p = this._typeIdMap.get(typeId);
        if (p !== undefined) {
            return p;
        } else {
            this._typeIdMap.set(typeId, ++this._typeIdIndex);
            return -1;
        }
    }
}

class EncapsEncoder10 extends EncapsEncoder {
    constructor(stream, encaps) {
        super(stream, encaps);
        this._sliceType = SliceType.NoSlice;
        this._writeSlice = 0; // Position of the slice data members
        this._valueIdIndex = 0;
        this._toBeMarshaledMap = new Map(); // Map<Value, Integer>();
    }

    writeValue(v) {
        DEV: console.assert(v !== undefined);
        //
        // Object references are encoded as a negative integer in 1.0.
        //
        if (v !== null && v !== undefined) {
            this._stream.writeInt(-this.registerValue(v));
        } else {
            this._stream.writeInt(0);
        }
    }

    writeException(v) {
        DEV: console.assert(v !== null && v !== undefined);
        //
        // User exception with the 1.0 encoding start with a boolean
        // flag that indicates whether or not the exception uses
        // classes.
        //
        // This allows reading the pending instances even if some part of
        // the exception was sliced.
        //
        const usesClasses = v._usesClasses();
        this._stream.writeBool(usesClasses);
        v._write(this._stream);
        if (usesClasses) {
            this.writePendingValues();
        }
    }

    startInstance(sliceType) {
        this._sliceType = sliceType;
    }

    endInstance() {
        if (this._sliceType === SliceType.ValueSlice) {
            //
            // Write the Object slice.
            //
            this.startSlice(Value.ice_staticId(), -1, true);
            this._stream.writeSize(0); // For compatibility with the old AFM.
            this.endSlice();
        }
        this._sliceType = SliceType.NoSlice;
    }

    startSlice(typeId) {
        //
        // For instance slices, encode a boolean to indicate how the type ID
        // is encoded and the type ID either as a string or index. For
        // exception slices, always encode the type ID as a string.
        //
        if (this._sliceType === SliceType.ValueSlice) {
            const index = this.registerTypeId(typeId);
            if (index < 0) {
                this._stream.writeBool(false);
                this._stream.writeString(typeId);
            } else {
                this._stream.writeBool(true);
                this._stream.writeSize(index);
            }
        } else {
            this._stream.writeString(typeId);
        }

        this._stream.writeInt(0); // Placeholder for the slice length.

        this._writeSlice = this._stream.pos;
    }

    endSlice() {
        //
        // Write the slice length.
        //
        const sz = this._stream.pos - this._writeSlice + 4;
        this._stream.rewriteInt(sz, this._writeSlice - 4);
    }

    writePendingValues() {
        const writeCB = (value, key) => {
            //
            // Ask the instance to marshal itself. Any new class
            // instances that are triggered by the classes marshaled
            // are added to toBeMarshaledMap.
            //
            this._stream.writeInt(value);
            key.ice_preMarshal();
            key._iceWrite(this._stream);
        };

        while (this._toBeMarshaledMap.size > 0) {
            //
            // Consider the to be marshaled instances as marshaled now,
            // this is necessary to avoid adding again the "to be
            // marshaled instances" into _toBeMarshaledMap while writing
            // instances.
            //
            this._toBeMarshaledMap.forEach((value, key) => this._marshaledMap.set(key, value));

            const savedMap = this._toBeMarshaledMap;
            this._toBeMarshaledMap = new Map(); // Map<Value, int>();
            this._stream.writeSize(savedMap.size);
            savedMap.forEach(writeCB);
        }
        this._stream.writeSize(0); // Zero marker indicates end of sequence of sequences of instances.
    }

    registerValue(v) {
        DEV: console.assert(v !== null);

        //
        // Look for this instance in the to-be-marshaled map.
        //
        let p = this._toBeMarshaledMap.get(v);
        if (p !== undefined) {
            return p;
        }

        //
        // Didn't find it, try the marshaled map next.
        //
        p = this._marshaledMap.get(v);
        if (p !== undefined) {
            return p;
        }

        //
        // We haven't seen this instance previously, create a new
        // index, and insert it into the to-be-marshaled map.
        //
        this._toBeMarshaledMap.set(v, ++this._valueIdIndex);
        return this._valueIdIndex;
    }
}

class EncapsEncoder11 extends EncapsEncoder {
    constructor(stream, encaps) {
        super(stream, encaps);
        this._current = null;
        this._valueIdIndex = 1;
    }

    writeValue(v) {
        DEV: console.assert(v !== undefined);
        if (v === null || v === undefined) {
            this._stream.writeSize(0);
        } else if (this._current !== null && this._encaps.format === FormatType.SlicedFormat) {
            if (this._current.indirectionTable === null) {
                // Lazy initialization
                this._current.indirectionTable = []; // Value[]
                this._current.indirectionMap = new Map(); // Map<Value, int>
            }

            //
            // If writing an instance within a slice and using the sliced
            // format, write an index from the instance indirection
            // table. The indirect instance table is encoded at the end of
            // each slice and is always read (even if the Slice is
            // unknown).
            //
            const index = this._current.indirectionMap.get(v);
            if (index === undefined) {
                this._current.indirectionTable.push(v);
                const idx = this._current.indirectionTable.length; // Position + 1 (0 is reserved for nil)
                this._current.indirectionMap.set(v, idx);
                this._stream.writeSize(idx);
            } else {
                this._stream.writeSize(index);
            }
        } else {
            this.writeInstance(v); // Write the instance or a reference if already marshaled.
        }
    }

    writePendingValues() {
        return undefined;
    }

    writeException(v) {
        DEV: console.assert(v !== null && v !== undefined);
        v._write(this._stream);
    }

    startInstance(sliceType, data) {
        if (this._current === null) {
            this._current = new EncapsEncoder11.InstanceData(null);
        } else {
            this._current =
                this._current.next === null ? new EncapsEncoder11.InstanceData(this._current) : this._current.next;
        }
        this._current.sliceType = sliceType;
        this._current.firstSlice = true;

        if (data !== null && data !== undefined) {
            this.writeSlicedData(data);
        }
    }

    endInstance() {
        this._current = this._current.previous;
    }

    startSlice(typeId, compactId, last) {
        DEV: console.assert(
            (this._current.indirectionTable === null || this._current.indirectionTable.length === 0) &&
                (this._current.indirectionMap === null || this._current.indirectionMap.size === 0),
        );

        this._current.sliceFlagsPos = this._stream.pos;

        this._current.sliceFlags = 0;
        if (this._encaps.format === FormatType.SlicedFormat) {
            // Encode the slice size if using the sliced format.
            this._current.sliceFlags |= Protocol.FLAG_HAS_SLICE_SIZE;
        }
        if (last) {
            this._current.sliceFlags |= Protocol.FLAG_IS_LAST_SLICE; // This is the last slice.
        }

        this._stream.writeByte(0); // Placeholder for the slice flags

        //
        // For instance slices, encode the flag and the type ID either as a
        // string or index. For exception slices, always encode the type
        // ID a string.
        //
        if (this._current.sliceType === SliceType.ValueSlice) {
            //
            // Encode the type ID (only in the first slice for the compact
            // encoding).
            //
            if (this._encaps.format === FormatType.SlicedFormat || this._current.firstSlice) {
                if (compactId >= 0) {
                    this._current.sliceFlags |= Protocol.FLAG_HAS_TYPE_ID_COMPACT;
                    this._stream.writeSize(compactId);
                } else {
                    const index = this.registerTypeId(typeId);
                    if (index < 0) {
                        this._current.sliceFlags |= Protocol.FLAG_HAS_TYPE_ID_STRING;
                        this._stream.writeString(typeId);
                    } else {
                        this._current.sliceFlags |= Protocol.FLAG_HAS_TYPE_ID_INDEX;
                        this._stream.writeSize(index);
                    }
                }
            }
        } else {
            this._stream.writeString(typeId);
        }

        if ((this._current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) !== 0) {
            this._stream.writeInt(0); // Placeholder for the slice length.
        }

        this._current.writeSlice = this._stream.pos;
        this._current.firstSlice = false;
    }

    endSlice() {
        //
        // Write the optional member end marker if some optional members
        // were encoded. Note that the optional members are encoded before
        // the indirection table and are included in the slice size.
        //
        if ((this._current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) !== 0) {
            this._stream.writeByte(Protocol.OPTIONAL_END_MARKER);
        }

        //
        // Write the slice length if necessary.
        //
        if ((this._current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) !== 0) {
            const sz = this._stream.pos - this._current.writeSlice + 4;
            this._stream.rewriteInt(sz, this._current.writeSlice - 4);
        }

        //
        // Only write the indirection table if it contains entries.
        //
        if (this._current.indirectionTable !== null && this._current.indirectionTable.length !== 0) {
            DEV: console.assert(this._encaps.format === FormatType.SlicedFormat);
            this._current.sliceFlags |= Protocol.FLAG_HAS_INDIRECTION_TABLE;

            //
            // Write the indirection instance table.
            //
            this._stream.writeSize(this._current.indirectionTable.length);
            this._current.indirectionTable.forEach(o => this.writeInstance(o));
            this._current.indirectionTable.length = 0; // Faster way to clean array in JavaScript
            this._current.indirectionMap.clear();
        }

        //
        // Finally, update the slice flags.
        //
        this._stream.rewriteByte(this._current.sliceFlags, this._current.sliceFlagsPos);
    }

    writeOptional(tag, format) {
        if (this._current === null) {
            return this._stream.writeOptImpl(tag, format);
        }

        if (this._stream.writeOptImpl(tag, format)) {
            this._current.sliceFlags |= Protocol.FLAG_HAS_OPTIONAL_MEMBERS;
            return true;
        }

        return false;
    }

    writeSlicedData(slicedData) {
        DEV: console.assert(slicedData !== null && slicedData !== undefined);

        //
        // We only marshal preserved slices if we are using the sliced
        // format. Otherwise, we ignore the preserved slices, which
        // essentially "slices" the instance into the most-derived type
        // known by the sender.
        //
        if (this._encaps.format !== FormatType.SlicedFormat) {
            return;
        }

        slicedData.slices.forEach(info => {
            this.startSlice(info.typeId, info.compactId, info.isLastSlice);

            //
            // Write the bytes associated with this slice.
            //
            this._stream.writeBlob(info.bytes);

            if (info.hasOptionalMembers) {
                this._current.sliceFlags |= Protocol.FLAG_HAS_OPTIONAL_MEMBERS;
            }

            //
            // Make sure to also re-write the instance indirection table.
            //
            if (info.instances !== null && info.instances.length > 0) {
                if (this._current.indirectionTable === null) {
                    // Lazy initialization
                    this._current.indirectionTable = []; // Value[]
                    this._current.indirectionMap = new Map(); // Map<Value, int>
                }

                info.instances.forEach(instance => this._current.indirectionTable.push(instance));
            }

            this.endSlice();
        });
    }

    writeInstance(v) {
        DEV: console.assert(v !== null && v !== undefined);

        //
        // If the instance was already marshaled, just write it's ID.
        //
        const p = this._marshaledMap.get(v);
        if (p !== undefined) {
            this._stream.writeSize(p);
            return;
        }

        //
        // We haven't seen this instance previously, create a new ID,
        // insert it into the marshaled map, and write the instance.
        //
        this._marshaledMap.set(v, ++this._valueIdIndex);
        v.ice_preMarshal();

        this._stream.writeSize(1); // Object instance marker.
        v._iceWrite(this._stream);
    }
}

EncapsEncoder11.InstanceData = class {
    constructor(previous) {
        DEV: console.assert(previous !== undefined);
        if (previous !== null) {
            previous.next = this;
        }
        this.previous = previous;
        this.next = null;

        // Instance attributes
        this.sliceType = null;
        this.firstSlice = false;

        // Slice attributes
        this.sliceFlags = 0;
        this.writeSlice = 0; // Position of the slice data members
        this.sliceFlagsPos = 0; // Position of the slice flags
        this.indirectionTable = null; // Value[]
        this.indirectionMap = null; // Map<Value, int>
    }
};

export class OutputStream {
    constructor(arg1, arg2) {
        let instance = null;
        this._encoding = null;

        if (arg1 !== undefined && arg1 !== null) {
            if (arg1.constructor == Communicator) {
                instance = arg1.instance;
            } else if (arg1.constructor == Instance) {
                instance = arg1;
            } else if (arg1.constructor == EncodingVersion) {
                this._encoding = arg1;
                if (arg2 !== null && arg2 !== undefined) {
                    if (arg2.constructor == FormatType) {
                        this._format = arg2;
                    } else {
                        throw new InitializationException("unknown argument to OutputStream constructor");
                    }
                }
            } else {
                throw new InitializationException("unknown argument to OutputStream constructor");
            }
        }

        this._buf = new Buffer();

        this._closure = null;

        this._encapsStack = null;
        this._encapsCache = null;

        if (instance !== null) {
            if (this._encoding === null) {
                this._encoding = instance.defaultsAndOverrides().defaultEncoding;
            }
            this._format = instance.defaultsAndOverrides().defaultFormat;
        } else {
            if (this._encoding === null) {
                this._encoding = Protocol.currentEncoding;
            }
            this._format = FormatType.CompactFormat;
        }
    }

    clear() {
        if (this._encapsStack !== null) {
            DEV: console.assert(this._encapsStack.next);
            this._encapsStack.next = this._encapsCache;
            this._encapsCache = this._encapsStack;
            this._encapsCache.reset();
            this._encapsStack = null;
        }
    }

    finished() {
        return this.prepareWrite().getArray(this.size);
    }

    swap(other) {
        [other._buf, this._buf] = [this._buf, other._buf];
        [other._encoding, this._encoding] = [this._encoding, other._encoding];
        [other._closure, this._closure] = [this._closure, other._closure];

        //
        // Swap is never called for streams that have encapsulations being written. However,
        // encapsulations might still be set in case marshaling failed. We just
        // reset the encapsulations if there are still some set.
        //
        this.resetEncapsulation();
        other.resetEncapsulation();
    }

    resetEncapsulation() {
        this._encapsStack = null;
    }

    resize(sz) {
        this._buf.resize(sz);
        this._buf.position = sz;
    }

    prepareWrite() {
        this._buf.position = 0;
        return this._buf;
    }

    startValue(data) {
        DEV: console.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.startInstance(SliceType.ValueSlice, data);
    }

    endValue() {
        DEV: console.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.endInstance();
    }

    startException() {
        DEV: console.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.startInstance(SliceType.ExceptionSlice);
    }

    endException() {
        DEV: console.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.endInstance();
    }

    startEncapsulation(encoding, format) {
        //
        // If no encoding version is specified, use the current write
        // encapsulation encoding version if there's a current write
        // encapsulation, otherwise, use the stream encoding version.
        //

        if (encoding === undefined) {
            if (this._encapsStack !== null) {
                encoding = this._encapsStack.encoding;
                format = this._encapsStack.format;
            } else {
                encoding = this._encoding;
                format = null;
            }
        }

        Protocol.checkSupportedEncoding(encoding);

        let curr = this._encapsCache;
        if (curr !== null) {
            curr.reset();
            this._encapsCache = this._encapsCache.next;
        } else {
            curr = new WriteEncaps();
        }
        curr.next = this._encapsStack;
        this._encapsStack = curr;

        this._encapsStack.format = format || this._format;
        this._encapsStack.setEncoding(encoding);
        this._encapsStack.start = this._buf.limit;

        this.writeInt(0); // Placeholder for the encapsulation length.
        this._encapsStack.encoding._write(this);
    }

    endEncapsulation() {
        DEV: console.assert(this._encapsStack);

        // Size includes size and version.
        const start = this._encapsStack.start;
        const sz = this._buf.limit - start;
        this._buf.putIntAt(start, sz);

        const curr = this._encapsStack;
        this._encapsStack = curr.next;
        curr.next = this._encapsCache;
        this._encapsCache = curr;
        this._encapsCache.reset();
    }

    writeEmptyEncapsulation(encoding) {
        Protocol.checkSupportedEncoding(encoding);
        this.writeInt(6); // Size
        encoding._write(this);
    }

    writeEncapsulation(v) {
        if (v.length < 6) {
            throw new MarshalException(`A byte sequence with ${v.length} bytes is not a valid encapsulation.`);
        }
        this.expand(v.length);
        this._buf.putArray(v);
    }

    getEncoding() {
        return this._encapsStack !== null ? this._encapsStack.encoding : this._encoding;
    }

    startSlice(typeId, compactId, last) {
        DEV: console.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.startSlice(typeId, compactId, last);
    }

    endSlice() {
        DEV: console.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.endSlice();
    }

    writePendingValues() {
        if (this._encapsStack !== null && this._encapsStack.encoder !== null) {
            this._encapsStack.encoder.writePendingValues();
        } else if (
            (this._encapsStack !== null && this._encapsStack.encoding_1_0) ||
            (this._encapsStack === null && this._encoding.equals(Encoding_1_0))
        ) {
            //
            // If using the 1.0 encoding and no instances were written, we
            // still write an empty sequence for pending instances if
            // requested (i.e.: if this is called).
            //
            // This is required by the 1.0 encoding, even if no instances
            // are written we do marshal an empty sequence if marshaled
            // data types use classes.
            //
            this.writeSize(0);
        }
    }

    writeSize(v) {
        if (v > 254) {
            this.expand(5);
            this._buf.put(255);
            this._buf.putInt(v);
        } else {
            this.expand(1);
            this._buf.put(v);
        }
    }

    startSize() {
        const pos = this._buf.position;
        this.writeInt(0); // Placeholder for 32-bit size
        return pos;
    }

    endSize(pos) {
        DEV: console.assert(pos >= 0);
        this.rewriteInt(this._buf.position - pos - 4, pos);
    }

    writeBlob(v) {
        if (v === null || v === undefined) {
            return;
        }
        this.expand(v.length);
        this._buf.putArray(v);
    }

    // Read/write format and tag for optionals
    writeOptional(tag, format) {
        DEV: console.assert(this._encapsStack !== null);
        if (this._encapsStack.encoder !== null) {
            return this._encapsStack.encoder.writeOptional(tag, format);
        }
        return this.writeOptImpl(tag, format);
    }

    writeOptionalHelper(tag, format, write, v) {
        if (v !== undefined) {
            if (this.writeOptional(tag, format)) {
                write.call(this, v);
            }
        }
    }

    writeByte(v) {
        this.expand(1);
        this._buf.put(v);
    }

    rewriteByte(v, dest) {
        this._buf.putAt(dest, v);
    }

    writeByteSeq(v) {
        if (v === null || v === undefined || v.length === 0) {
            this.writeSize(0);
        } else {
            this.writeSize(v.length);
            this.expand(v.length);
            this._buf.putArray(v);
        }
    }

    writeBool(v) {
        this.expand(1);
        this._buf.put(v ? 1 : 0);
    }

    rewriteBool(v, dest) {
        this._buf.putAt(dest, v ? 1 : 0);
    }

    writeShort(v) {
        this.expand(2);
        this._buf.putShort(v);
    }

    writeInt(v) {
        this.expand(4);
        this._buf.putInt(v);
    }

    rewriteInt(v, dest) {
        this._buf.putIntAt(dest, v);
    }

    writeLong(v) {
        this.expand(8);
        this._buf.putLong(v);
    }

    writeFloat(v) {
        this.expand(4);
        this._buf.putFloat(v);
    }

    writeDouble(v) {
        this.expand(8);
        this._buf.putDouble(v);
    }

    writeString(v) {
        if (v === null || v === undefined || v.length === 0) {
            this.writeSize(0);
        } else {
            this._buf.writeString(this, v);
        }
    }

    writeProxy(v) {
        if (v === null || v === undefined) {
            const ident = new Identity();
            ident._write(this);
        } else {
            v._write(this);
        }
    }

    writeOptionalProxy(tag, v) {
        if (v !== undefined && v !== null) {
            if (this.writeOptional(tag, OptionalFormat.FSize)) {
                const pos = this.startSize();
                this.writeProxy(v);
                this.endSize(pos);
            }
        }
    }

    writeEnum(v) {
        if (this.isEncoding_1_0()) {
            if (v.maxValue < 127) {
                this.writeByte(v.value);
            } else if (v.maxValue < 32767) {
                this.writeShort(v.value);
            } else {
                this.writeInt(v.value);
            }
        } else {
            this.writeSize(v.value);
        }
    }

    writeValue(v) {
        this.initEncaps();
        this._encapsStack.encoder.writeValue(v);
    }

    writeException(e) {
        this.initEncaps();
        // Exceptions are always encoded with the sliced format.
        this._encapsStack.format = FormatType.SlicedFormat;
        this._encapsStack.encoder.writeException(e);
    }

    writeOptImpl(tag, format) {
        if (this.isEncoding_1_0()) {
            return false; // Optional members aren't supported with the 1.0 encoding.
        }

        let v = format.value;
        if (tag < 30) {
            v |= tag << 3;
            this.writeByte(v);
        } else {
            v |= 0x0f0; // tag = 30
            this.writeByte(v);
            this.writeSize(tag);
        }
        return true;
    }

    isEmpty() {
        return this._buf.empty();
    }

    expand(n) {
        this._buf.expand(n);
    }

    isEncoding_1_0() {
        return this._encapsStack ? this._encapsStack.encoding_1_0 : this._encoding.equals(Encoding_1_0);
    }

    initEncaps() {
        if (!this._encapsStack) {
            // Lazy initialization
            this._encapsStack = this._encapsCache;
            if (this._encapsStack) {
                this._encapsCache = this._encapsCache.next;
            } else {
                this._encapsStack = new WriteEncaps();
            }
            this._encapsStack.setEncoding(this._encoding);
        }

        if (this._encapsStack.format === null) {
            this._encapsStack.format = this._format;
        }

        if (!this._encapsStack.encoder) {
            // Lazy initialization.
            if (this._encapsStack.encoding_1_0) {
                this._encapsStack.encoder = new EncapsEncoder10(this, this._encapsStack);
            } else {
                this._encapsStack.encoder = new EncapsEncoder11(this, this._encapsStack);
            }
        }
    }

    //
    // Sets the encoding format for class and exception instances.
    //
    get format() {
        return this._format;
    }

    set format(value) {
        this._format = value;
    }

    get pos() {
        return this._buf.position;
    }

    set pos(value) {
        this._buf.position = value;
    }

    get size() {
        return this._buf.limit;
    }

    get closure() {
        return this._closure;
    }

    set closure(value) {
        this._closure = value;
    }

    get buffer() {
        return this._buf;
    }
}
