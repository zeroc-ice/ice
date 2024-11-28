//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { throwUOE } from "./ExUtil.js";
import { ArrayUtil } from "./ArrayUtil.js";
import { Buffer } from "./Buffer.js";
import { CompactIdRegistry } from "./CompactIdRegistry.js";
import { OptionalFormat } from "./OptionalFormat.js";
import { Encoding_1_0, Protocol } from "./Protocol.js";
import { SlicedData, SliceInfo, UnknownSlicedValue } from "./UnknownSlicedValue.js";
import { Value } from "./Value.js";
import { InitializationException, MarshalException } from "./LocalExceptions.js";
import { Ice as Ice_Identity } from "./Identity.js";
const { Identity } = Ice_Identity;
import { Ice as Ice_Version } from "./Version.js";
const { EncodingVersion } = Ice_Version;
import { Instance } from "./Instance.js";
import { Communicator } from "./Communicator.js";
import { TypeRegistry } from "./TypeRegistry.js";
import { ObjectPrx } from "./ObjectPrx.js";
import { SliceType } from "./SliceType.js";

const endOfBufferMessage = "Attempting to unmarshal past the end of the buffer.";

const slicingIds = new Map();
function traceSlicing(kind, typeId, slicingCat, logger) {
    if (!slicingIds.has(typeId)) {
        logger.trace(slicingCat, `unknown ${kind} type \`${typeId}'`);
        slicingIds.set(typeId, 1);
    }
}

class IndirectPatchEntry {
    constructor(index, cb) {
        this.index = index;
        this.cb = cb;
    }
}

class EncapsDecoder {
    constructor(stream, encaps, classGraphDepth, f) {
        this._stream = stream;
        this._encaps = encaps;
        this._classGraphDepthMax = classGraphDepth;
        this._classGraphDepth = 0;
        this._valueFactoryManager = f;
        this._patchMap = null; // Lazy initialized, Map<int, Patcher[] >()
        this._unmarshaledMap = new Map(); // Map<int, Value>()
        this._typeIdMap = null; // Lazy initialized, Map<int, String>
        this._typeIdIndex = 0;
        this._valueList = null; // Lazy initialized. Value[]
    }

    readOptional() {
        return false;
    }

    readPendingValues() {}

    readTypeId(isIndex) {
        if (this._typeIdMap === null) {
            // Lazy initialization
            this._typeIdMap = new Map(); // Map<int, String>();
        }

        let typeId;
        if (isIndex) {
            typeId = this._typeIdMap.get(this._stream.readSize());
            if (typeId === undefined) {
                throw new MarshalException(endOfBufferMessage);
            }
        } else {
            typeId = this._stream.readString();
            this._typeIdMap.set(++this._typeIdIndex, typeId);
        }
        return typeId;
    }

    newInstance(typeId) {
        // Try to find a factory registered for the specific type.
        let userFactory = this._valueFactoryManager.find(typeId);
        let v = null;

        if (userFactory !== undefined) {
            v = userFactory(typeId);
        }

        // If that fails, invoke the default factory if one has been registered.
        if (v === null || v === undefined) {
            userFactory = this._valueFactoryManager.find("");
            if (userFactory !== undefined) {
                v = userFactory(typeId);
            }
        }

        // Last chance: try to instantiate the class dynamically.
        if (v === null || v === undefined) {
            v = this._stream.createInstance(typeId);
        }

        return v;
    }

    addPatchEntry(index, cb) {
        DEV: console.assert(index > 0);

        //
        // Check if we have already unmarshaled the instance. If that's the case,
        // just call the callback and we're done.
        //
        const obj = this._unmarshaledMap.get(index);
        if (obj !== undefined && obj !== null) {
            cb(obj);
            return;
        }

        if (this._patchMap === null) {
            // Lazy initialization
            this._patchMap = new Map(); // Map<Integer, Patcher[] >();
        }

        //
        // Add a patch entry if the instance isn't unmarshaled yet,
        // the callback will be called when the instance is
        // unmarshaled.
        //
        let l = this._patchMap.get(index);
        if (l === undefined) {
            //
            // We have no outstanding instances to be patched for this
            // index, so make a new entry in the patch map.
            //
            l = []; // ReadValueCallback[]
            this._patchMap.set(index, l);
        }

        //
        // Append a patch entry for this instance.
        //
        l.push(new PatchEntry(cb, this._classGraphDepth));
    }

    unmarshal(index, v) {
        //
        // Add the instance to the map of unmarshaled instances, this must
        // be done before reading the instances (for circular references).
        //
        this._unmarshaledMap.set(index, v);

        //
        // Read the instance.
        //
        v._iceRead(this._stream);

        if (this._patchMap !== null) {
            //
            // Patch all instances now that the instance is unmarshaled.
            //
            const l = this._patchMap.get(index);
            if (l !== undefined) {
                DEV: console.assert(l.length > 0);

                //
                // Patch all pointers that refer to the instance.
                //
                for (let i = 0; i < l.length; ++i) {
                    l[i].cb(v);
                }

                //
                // Clear out the patch map for that index -- there is nothing left
                // to patch for that index for the time being.
                //
                this._patchMap.delete(index);
            }
        }

        if ((this._patchMap === null || this._patchMap.size === 0) && this._valueList === null) {
            v.ice_postUnmarshal();
        } else {
            if (this._valueList === null) {
                // Lazy initialization
                this._valueList = []; // Value[]
            }
            this._valueList.push(v);

            if (this._patchMap === null || this._patchMap.size === 0) {
                //
                // Iterate over the instance list and invoke ice_postUnmarshal on
                // each instance. We must do this after all instances have been
                // unmarshaled in order to ensure that any instance data members
                // have been properly patched.
                //
                for (let i = 0; i < this._valueList.length; i++) {
                    this._valueList[i].ice_postUnmarshal();
                }
                this._valueList = [];
            }
        }
    }
}

class EncapsDecoder10 extends EncapsDecoder {
    constructor(stream, encaps, classGraphDepth, f) {
        super(stream, encaps, classGraphDepth, f);
        this._sliceType = SliceType.NoSlice;
    }

    readValue(cb) {
        //
        // Instance references are encoded as a negative integer in 1.0.
        //
        let index = this._stream.readInt();
        if (index > 0) {
            throw new MarshalException("invalid object id");
        }
        index = -index;

        if (index === 0) {
            cb(null);
        } else {
            this.addPatchEntry(index, cb);
        }
    }

    throwException() {
        DEV: console.assert(this._sliceType === SliceType.NoSlice);

        //
        // User exceptions with the 1.0 encoding start with a boolean flag
        // that indicates whether or not the exception has classes.
        //
        // This allows reading the pending instances even if some part of
        // the exception was sliced.
        //
        const usesClasses = this._stream.readBool();

        this._sliceType = SliceType.ExceptionSlice;
        this._skipFirstSlice = false;

        //
        // Read the first slice header.
        //
        this.startSlice();
        const mostDerivedId = this._typeId;
        while (true) {
            const userEx = this._stream.createUserException(this._typeId);

            //
            // We found the exception.
            //
            if (userEx !== null) {
                userEx._read(this._stream);
                if (usesClasses) {
                    this.readPendingValues();
                }
                throw userEx;

                // Never reached.
            }

            //
            // Slice off what we don't understand.
            //
            this.skipSlice();
            try {
                this.startSlice();
            } catch (ex) {
                //
                // An oversight in the 1.0 encoding means there is no marker to indicate
                // the last slice of an exception. As a result, we just try to read the
                // next type ID, which raises MarshalException when the input buffer underflow.
                if (ex instanceof MarshalException) {
                    throw new MarshalException(`unknown exception type '${mostDerivedId}'`);
                }
                throw ex;
            }
        }
    }

    startInstance(sliceType) {
        DEV: console.assert(this._sliceType === sliceType);
        this._skipFirstSlice = true;
    }

    endInstance() {
        //
        // Read the Ice::Object slice.
        //
        if (this._sliceType === SliceType.ValueSlice) {
            this.startSlice();
            const sz = this._stream.readSize(); // For compatibility with the old AFM.
            if (sz !== 0) {
                throw new MarshalException("invalid Object slice");
            }
            this.endSlice();
        }

        this._sliceType = SliceType.NoSlice;
        return null;
    }

    startSlice() {
        //
        // If first slice, don't read the header, it was already read in
        // readInstance or throwException to find the factory.
        //
        if (this._skipFirstSlice) {
            this._skipFirstSlice = false;
            return this._typeId;
        }

        //
        // For instances, first read the type ID boolean which indicates
        // whether or not the type ID is encoded as a string or as an
        // index. For exceptions, the type ID is always encoded as a
        // string.
        //
        if (this._sliceType === SliceType.ValueSlice) {
            // For exceptions, the type ID is always encoded as a string
            const isIndex = this._stream.readBool();
            this._typeId = this.readTypeId(isIndex);
        } else {
            this._typeId = this._stream.readString();
        }

        this._sliceSize = this._stream.readInt();
        if (this._sliceSize < 4) {
            throw new MarshalException(endOfBufferMessage);
        }

        return this._typeId;
    }

    endSlice() {}

    skipSlice() {
        this._stream.traceSkipSlice(this._typeId, this._sliceType);
        DEV: console.assert(this._sliceSize >= 4);
        this._stream.skip(this._sliceSize - 4);
    }

    readPendingValues() {
        let num;
        do {
            num = this._stream.readSize();
            for (let k = num; k > 0; --k) {
                this.readInstance();
            }
        } while (num > 0);

        if (this._patchMap !== null && this._patchMap.size !== 0) {
            //
            // If any entries remain in the patch map, the sender has sent an index for an instance, but failed
            // to supply the instance.
            //
            throw new MarshalException("index for class received, but no instance");
        }
    }

    readInstance() {
        const index = this._stream.readInt();
        let v = null;

        if (index <= 0) {
            throw new MarshalException("invalid object id");
        }

        this._sliceType = SliceType.ValueSlice;
        this._skipFirstSlice = false;

        //
        // Read the first slice header.
        //
        this.startSlice();
        const mostDerivedId = this._typeId;
        while (true) {
            //
            // For the 1.0 encoding, the type ID for the base Object class
            // marks the last slice.
            //
            if (this._typeId == Value.ice_staticId()) {
                throw new MarshalException(`Cannot find value factory for type ID '${mostDerivedId}'.`);
            }

            v = this.newInstance(this._typeId);

            //
            // We found a factory, we get out of this loop.
            //
            if (v) {
                break;
            }

            //
            // Slice off what we don't understand.
            //
            this.skipSlice();
            this.startSlice(); // Read next Slice header for next iteration.
        }

        //
        // Compute the biggest class graph depth of this object. To compute this,
        // we get the class graph depth of each ancestor from the patch map and
        // keep the biggest one.
        //
        this._classGraphDepth = 0;
        const l = this._patchMap === null ? null : this._patchMap.get(index);
        if (l !== undefined) {
            DEV: console.assert(l.length > 0);
            for (const entry of l) {
                if (entry.classGraphDepth > this._classGraphDepth) {
                    this._classGraphDepth = entry.classGraphDepth;
                }
            }
        }

        if (++this._classGraphDepth > this._classGraphDepthMax) {
            throw new MarshalException("maximum class graph depth reached");
        }

        //
        // Unmarshal the instance and add it to the map of unmarshaled instances.
        //
        this.unmarshal(index, v);
    }
}

class EncapsDecoder11 extends EncapsDecoder {
    constructor(stream, encaps, classGraphDepth, f) {
        super(stream, encaps, classGraphDepth, f);
        this._current = null;
        this._valueIdIndex = 1;
    }

    readValue(cb) {
        const index = this._stream.readSize();
        if (index < 0) {
            throw new MarshalException("invalid object id");
        } else if (index === 0) {
            cb(null);
        } else if (this._current !== null && (this._current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) !== 0) {
            //
            // When reading an instance within a slice and there's an
            // indirect instance table, always read an indirect reference
            // that points to an instance from the indirect instance table
            // marshaled at the end of the Slice.
            //
            // Maintain a list of indirect references. Note that the
            // indirect index starts at 1, so we decrement it by one to
            // derive an index into the indirection table that we'll read
            // at the end of the slice.
            //
            if (this._current.indirectPatchList === null) {
                // Lazy initialization
                this._current.indirectPatchList = []; // IndirectPatchEntry[]
            }
            this._current.indirectPatchList.push(new IndirectPatchEntry(index - 1, cb));
        } else {
            this.readInstance(index, cb);
        }
    }

    throwException() {
        DEV: console.assert(this._current === null);

        this.push(SliceType.ExceptionSlice);

        //
        // Read the first slice header.
        //
        this.startSlice();
        const mostDerivedId = this._current.typeId;
        while (true) {
            const userEx = this._stream.createUserException(this._current.typeId);

            //
            // We found the exception.
            //
            if (userEx !== null) {
                userEx._read(this._stream);
                throw userEx;

                // Never reached.
            }

            //
            // Slice off what we don't understand.
            //
            this.skipSlice();

            if ((this._current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) !== 0) {
                throw new MarshalException(`cannot unmarshal user exception with type ID '${mostDerivedId}'`);
            }

            this.startSlice();
        }
    }

    startInstance(sliceType) {
        DEV: console.assert(sliceType !== undefined);
        DEV: console.assert(this._current.sliceType !== null && this._current.sliceType === sliceType);
        this._current.skipFirstSlice = true;
    }

    endInstance() {
        let slicedData = this.readSlicedData();
        if (this._current.slices !== null) {
            this._current.slices.length = 0; // Clear the array.
            this._current.indirectionTables.length = 0; // Clear the array.
        }
        this._current = this._current.previous;
        return slicedData;
    }

    startSlice() {
        //
        // If first slice, don't read the header, it was already read in
        // readInstance or throwException to find the factory.
        //
        if (this._current.skipFirstSlice) {
            this._current.skipFirstSlice = false;
            return this._current.typeId;
        }

        this._current.sliceFlags = this._stream.readByte();

        //
        // Read the type ID, for instance slices the type ID is encoded as a
        // string or as an index, for exceptions it's always encoded as a
        // string.
        //
        if (this._current.sliceType === SliceType.ValueSlice) {
            if ((this._current.sliceFlags & Protocol.FLAG_HAS_TYPE_ID_COMPACT) === Protocol.FLAG_HAS_TYPE_ID_COMPACT) {
                // Must be checked 1st!
                this._current.typeId = "";
                this._current.compactId = this._stream.readSize();
            } else if (
                (this._current.sliceFlags & (Protocol.FLAG_HAS_TYPE_ID_INDEX | Protocol.FLAG_HAS_TYPE_ID_STRING)) !==
                0
            ) {
                this._current.typeId = this.readTypeId(
                    (this._current.sliceFlags & Protocol.FLAG_HAS_TYPE_ID_INDEX) !== 0,
                );
                this._current.compactId = -1;
            } else {
                //
                // Only the most derived slice encodes the type ID for the compact format.
                //
                this._current.typeId = "";
                this._current.compactId = -1;
            }
        } else {
            this._current.typeId = this._stream.readString();
            this._current.compactId = -1;
        }

        //
        // Read the slice size if necessary.
        //
        if ((this._current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) !== 0) {
            this._current.sliceSize = this._stream.readInt();
            if (this._current.sliceSize < 4) {
                throw new MarshalException(endOfBufferMessage);
            }
        } else {
            this._current.sliceSize = 0;
        }

        return this._current.typeId;
    }

    endSlice() {
        if ((this._current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) !== 0) {
            this._stream.skipOptionals();
        }

        //
        // Read the indirection table if one is present and transform the
        // indirect patch list into patch entries with direct references.
        //
        if ((this._current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) !== 0) {
            const indirectionTable = [];
            //
            // The table is written as a sequence<size> to conserve space.
            //
            const length = this._stream.readAndCheckSeqSize(1);
            for (let i = 0; i < length; ++i) {
                indirectionTable[i] = this.readInstance(this._stream.readSize(), null);
            }

            //
            // Sanity checks. If there are optional members, it's possible
            // that not all instance references were read if they are from
            // unknown optional data members.
            //
            if (indirectionTable.length === 0) {
                throw new MarshalException("empty indirection table");
            }
            if (
                (this._current.indirectPatchList === null || this._current.indirectPatchList.length === 0) &&
                (this._current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) === 0
            ) {
                throw new MarshalException("no references to indirection table");
            }

            //
            // Convert indirect references into direct references.
            //
            if (this._current.indirectPatchList !== null) {
                this._current.indirectPatchList.forEach(e => {
                    DEV: console.assert(e.index >= 0);
                    if (e.index >= indirectionTable.length) {
                        throw new MarshalException("indirection out of range");
                    }
                    this.addPatchEntry(indirectionTable[e.index], e.cb);
                });
                this._current.indirectPatchList.length = 0;
            }
        }
    }

    skipSlice() {
        this._stream.traceSkipSlice(this._current.typeId, this._current.sliceType);

        const start = this._stream.pos;

        if ((this._current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) !== 0) {
            DEV: console.assert(this._current.sliceSize >= 4);
            this._stream.skip(this._current.sliceSize - 4);
        } else {
            if (this._current.sliceType == SliceType.ValueSlice) {
                throw new MarshalException(
                    `Cannot find value factory for type ID '${this._current.typeId}' and compact format prevents slicing.`,
                );
            } else {
                throw new MarshalException(`Cannot find user exception for type ID '${this._current.typeId}'`);
            }
        }

        //
        // Preserve this slice.
        //
        const info = new SliceInfo();
        info.typeId = this._current.typeId;
        info.compactId = this._current.compactId;
        info.hasOptionalMembers = (this._current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) !== 0;
        info.isLastSlice = (this._current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) !== 0;

        const b = this._stream._buf;
        const end = b.position;
        let dataEnd = end;
        if (info.hasOptionalMembers) {
            //
            // Don't include the optional member end marker. It will be re-written by
            // endSlice when the sliced data is re-written.
            //
            --dataEnd;
        }

        b.position = start;
        info.bytes = b.getArray(dataEnd - start);
        b.position = end;

        if (this._current.slices === null) {
            // Lazy initialization
            this._current.slices = []; // Ice.SliceInfo[]
            this._current.indirectionTables = []; // int[]
        }

        //
        // Read the indirect instance table. We read the instances or their
        // IDs if the instance is a reference to an already unmarshaled
        // instance.
        //

        if ((this._current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) !== 0) {
            const length = this._stream.readAndCheckSeqSize(1);
            const indirectionTable = [];
            for (let i = 0; i < length; ++i) {
                indirectionTable[i] = this.readInstance(this._stream.readSize(), null);
            }
            this._current.indirectionTables.push(indirectionTable);
        } else {
            this._current.indirectionTables.push(null);
        }

        this._current.slices.push(info);
    }

    readOptional(readTag, expectedFormat) {
        if (this._current === null) {
            return this._stream.readOptImpl(readTag, expectedFormat);
        } else if ((this._current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) !== 0) {
            return this._stream.readOptImpl(readTag, expectedFormat);
        }
        return false;
    }

    readInstance(index, cb) {
        DEV: console.assert(index > 0);

        let v = null;

        if (index > 1) {
            if (cb !== null) {
                this.addPatchEntry(index, cb);
            }
            return index;
        }

        this.push(SliceType.ValueSlice);

        //
        // Get the instance ID before we start reading slices. If some
        // slices are skipped, the indirect instance table is still read and
        // might read other instances.
        //
        index = ++this._valueIdIndex;

        //
        // Read the first slice header.
        //
        this.startSlice();
        const mostDerivedId = this._current.typeId;
        while (true) {
            if (this._current.compactId >= 0) {
                //
                // Translate a compact (numeric) type ID into a string type ID.
                //
                this._current.typeId = this._stream.resolveCompactId(this._current.compactId);
            }

            if (this._current.typeId.length > 0) {
                v = this.newInstance(this._current.typeId);
            }

            if (v !== null && v !== undefined) {
                //
                // We have an instance, we get out of this loop.
                //
                break;
            }

            //
            // Slice off what we don't understand.
            //
            this.skipSlice();

            //
            // If this is the last slice, keep the instance as an opaque
            // UnknownSlicedValue object.
            //
            if ((this._current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) !== 0) {
                v = new UnknownSlicedValue(mostDerivedId);
                break;
            }

            this.startSlice(); // Read next Slice header for next iteration.
        }

        if (++this._classGraphDepth > this._classGraphDepthMax) {
            throw new MarshalException("maximum class graph depth reached");
        }

        //
        // Unmarshal the instance.
        //
        this.unmarshal(index, v);

        --this._classGraphDepth;

        if (this._current === null && this._patchMap !== null && this._patchMap.size !== 0) {
            //
            // If any entries remain in the patch map, the sender has sent an index for an instance, but failed
            // to supply the instance.
            //
            throw new MarshalException("index for class received, but no instance");
        }

        if (cb !== null) {
            cb(v);
        }

        return index;
    }

    readSlicedData() {
        if (this._current.slices === null) {
            // No preserved slices.
            return null;
        }

        //
        // The _indirectionTables member holds the indirection table for each slice
        // in _slices.
        //
        DEV: console.assert(this._current.slices.length === this._current.indirectionTables.length);
        for (let i = 0; i < this._current.slices.length; ++i) {
            //
            // We use the "instances" list in SliceInfo to hold references
            // to the target instances. Note that the instances might not have
            // been read yet in the case of a circular reference to an
            // enclosing instance.
            //
            const table = this._current.indirectionTables[i];
            const info = this._current.slices[i];
            info.instances = [];
            if (table) {
                for (let j = 0; j < table.length; ++j) {
                    this.addPatchEntry(table[j], sequencePatcher(info.instances, j, Value));
                }
            }
        }
        return new SlicedData(ArrayUtil.clone(this._current.slices));
    }

    push(sliceType) {
        if (this._current === null) {
            this._current = new EncapsDecoder11.InstanceData(null);
        } else {
            this._current = !this._current.next ? new EncapsDecoder11.InstanceData(this._current) : this._current.next;
        }
        this._current.sliceType = sliceType;
        this._current.skipFirstSlice = false;
    }
}

EncapsDecoder11.InstanceData = class {
    constructor(previous) {
        if (previous !== null) {
            previous.next = this;
        }
        this.previous = previous;
        this.next = null;

        // Instance attributes
        this.sliceType = null;
        this.skipFirstSlice = false;
        this.slices = null; // Preserved slices. Ice.SliceInfo[]
        this.indirectionTables = null; // int[][]

        // Slice attributes
        this.sliceFlags = 0;
        this.sliceSize = 0;
        this.typeId = null;
        this.compactId = 0;
        this.indirectPatchList = null; // Lazy initialized, IndirectPatchEntry[]
    }
};

const sequencePatcher = function (seq, index, T) {
    return v => {
        if (v !== null && !(v instanceof T)) {
            throwUOE(T.ice_staticId(), v);
        }
        seq[index] = v;
    };
};

class ReadEncaps {
    constructor() {
        this.start = 0;
        this.sz = 0;
        this.encoding = null;
        this.encoding_1_0 = false;
        this.decoder = null;
        this.next = null;
    }

    reset() {
        this.decoder = null;
    }

    setEncoding(encoding) {
        this.encoding = encoding;
        this.encoding_1_0 = encoding.equals(Encoding_1_0);
    }
}

class PatchEntry {
    constructor(cb, classGraphDepth) {
        this._cb = cb;
        this._classGraphDepth = classGraphDepth;
    }

    get cb() {
        return this._cb;
    }

    get classGraphDepth() {
        return this._classGraphDepth;
    }
}

export class InputStream {
    constructor(arg1, arg2, arg3) {
        // The first argument must be a Communicator or an Instance.
        if (arg1 === null || arg1 === undefined) {
            throw new InitializationException("missing argument(s) for InputStream constructor");
        }

        if (arg1.constructor === Instance) {
            // (instance, encoding) or (instance, encoding, buffer)

            this._instance = arg1;
            if (arg2 === null || arg2 === undefined || arg2.constructor !== EncodingVersion) {
                throw new InitializationException(
                    "expected the encoding version as the second argument to the InputStream constructor",
                );
            }
            this._encoding = arg2;

            if (arg3 === undefined || arg3 === null) {
                this._buf = new Buffer();
            } else if (arg3.constructor === Buffer) {
                this._buf = arg3;
            } else {
                throw new InitializationException("unknown third argument to InputStream constructor");
            }
        } else if (arg1.constructor === Communicator) {
            // (communicator, encoding, buffer) or (communicator, buffer), with two flavors for buffer
            this._instance = arg1.instance;

            [arg2, arg3].forEach(arg => {
                if (arg !== null && arg !== undefined) {
                    if (arg.constructor === EncodingVersion) {
                        if (this._encoding !== undefined) {
                            throw new InitializationException(
                                "expected buffer as the third argument to InputStream constructor",
                            );
                        }
                        this._encoding = arg;
                    } else if (arg.constructor === ArrayBuffer) {
                        if (this._buf !== undefined) {
                            throw new InitializationException("duplicate buffer argument to InputStream constructor");
                        }
                        this._buf = new Buffer(arg.bytes);
                    } else if (arg.constructor === Uint8Array) {
                        if (this._buf !== undefined) {
                            throw new InitializationException("duplicate buffer argument to InputStream constructor");
                        }
                        this._buf = new Buffer(arg.buffer);
                    } else {
                        throw new InitializationException("unknown argument to InputStream constructor");
                    }
                }
            });

            if (this._buf === undefined) {
                throw new InitializationException("missing buffer argument to InputStream constructor");
            }
            this._encoding ??= this._instance.defaultsAndOverrides().defaultEncoding;
        } else {
            throw new InitializationException("unknown first argument to InputStream constructor");
        }

        this._encapsStack = null;
        this._encapsCache = null;
        this._closure = null;
        this._startSeq = -1;
        this._sizePos = -1;

        this._valueFactoryManager = this._instance.initializationData().valueFactoryManager;
        this._classGraphDepthMax = this._instance.classGraphDepthMax();
    }

    clear() {
        if (this._encapsStack !== null) {
            DEV: console.assert(this._encapsStack.next === null);
            this._encapsStack.next = this._encapsCache;
            this._encapsCache = this._encapsStack;
            this._encapsCache.reset();
            this._encapsStack = null;
        }

        this._startSeq = -1;
    }

    swap(other) {
        DEV: console.assert(this._instance === other._instance);

        // These are cached values derived from instance.
        DEV: console.assert(this._classGraphDepthMax === other._classGraphDepthMax);
        DEV: console.assert(this._valueFactoryManager === other._valueFactoryManager);

        [other._buf, this._buf] = [this._buf, other._buf];
        [other._encoding, this._encoding] = [this._encoding, other._encoding];

        [other._startSeq, this._startSeq] = [this._startSeq, other._startSeq];
        [other._minSeqSize, this._minSeqSize] = [this._minSeqSize, other._minSeqSize];
        [other._sizePos, this._sizePos] = [this._sizePos, other._sizePos];

        // Swap is never called for InputStreams that have encapsulations being read/write. However,
        // encapsulations might still be set in case marshaling or unmarshaling failed. We just
        // reset the encapsulations if there are still some set.
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

    startValue() {
        DEV: console.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        this._encapsStack.decoder.startInstance(SliceType.ValueSlice);
    }

    endValue() {
        DEV: console.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        return this._encapsStack.decoder.endInstance();
    }

    startException() {
        DEV: console.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        this._encapsStack.decoder.startInstance(SliceType.ExceptionSlice);
    }

    endException() {
        DEV: console.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        return this._encapsStack.decoder.endInstance();
    }

    startEncapsulation() {
        let curr = this._encapsCache;
        if (curr !== null) {
            curr.reset();
            this._encapsCache = this._encapsCache.next;
        } else {
            curr = new ReadEncaps();
        }
        curr.next = this._encapsStack;
        this._encapsStack = curr;

        this._encapsStack.start = this._buf.position;

        //
        // I don't use readSize() for encapsulations, because when creating an encapsulation,
        // I must know in advance how many bytes the size information will require in the data
        // stream. If I use an Int, it is always 4 bytes. For readSize(), it could be 1 or 5 bytes.
        //
        const sz = this.readInt();
        if (sz < 6) {
            throw new MarshalException(endOfBufferMessage);
        }
        if (sz - 4 > this._buf.remaining) {
            throw new MarshalException(endOfBufferMessage);
        }
        this._encapsStack.sz = sz;

        const encoding = new EncodingVersion();
        encoding._read(this);
        Protocol.checkSupportedEncoding(encoding); // Make sure the encoding is supported.
        this._encapsStack.setEncoding(encoding);

        return encoding;
    }

    endEncapsulation() {
        DEV: console.assert(this._encapsStack !== null);

        if (!this._encapsStack.encoding_1_0) {
            this.skipOptionals();
            if (this._buf.position !== this._encapsStack.start + this._encapsStack.sz) {
                throw new MarshalException("Failed to unmarshal encapsulation.");
            }
        } else if (this._buf.position !== this._encapsStack.start + this._encapsStack.sz) {
            if (this._buf.position + 1 !== this._encapsStack.start + this._encapsStack.sz) {
                throw new MarshalException("Failed to unmarshal encapsulation.");
            }

            //
            // Ice version < 3.3 had a bug where user exceptions with
            // class members could be encoded with a trailing byte
            // when dispatched with AMD. So we tolerate an extra byte
            // in the encapsulation.
            //

            try {
                this._buf.get();
            } catch (ex) {
                throw new MarshalException(endOfBufferMessage);
            }
        }

        const curr = this._encapsStack;
        this._encapsStack = curr.next;
        curr.next = this._encapsCache;
        this._encapsCache = curr;
        this._encapsCache.reset();
    }

    skipEmptyEncapsulation() {
        const sz = this.readInt();
        if (sz < 6) {
            throw new MarshalException(`${sz} is not a valid encapsulation size.`);
        }
        if (sz - 4 > this._buf.remaining) {
            throw new MarshalException(endOfBufferMessage);
        }

        const encoding = new EncodingVersion();
        encoding._read(this);
        Protocol.checkSupportedEncoding(encoding); // Make sure the encoding is supported.

        if (encoding.equals(Encoding_1_0)) {
            if (sz != 6) {
                throw new MarshalException(`${sz} is not a valid encapsulation size for a 1.0 empty encapsulation.`);
            }
        } else {
            // Skip the optional content of the encapsulation if we are expecting an
            // empty encapsulation.
            this._buf.position = this._buf.position + sz - 6;
        }
        return encoding;
    }

    readEncapsulation() {
        const sz = this.readInt();
        if (sz < 6) {
            throw new MarshalException(endOfBufferMessage);
        }

        if (sz - 4 > this._buf.remaining) {
            throw new MarshalException(endOfBufferMessage);
        }

        const encoding = new EncodingVersion();
        encoding._read(this);
        this._buf.position = this._buf.position - 6;

        try {
            return [encoding, this._buf.getArray(sz)];
        } catch (ex) {
            throw new MarshalException(endOfBufferMessage);
        }
    }

    getEncoding() {
        return this._encapsStack !== null ? this._encapsStack.encoding : this._encoding;
    }

    getEncapsulationSize() {
        DEV: console.assert(this._encapsStack !== null);
        return this._encapsStack.sz - 6;
    }

    skipEncapsulation() {
        const sz = this.readInt();
        if (sz < 6) {
            throw new MarshalException(endOfBufferMessage);
        }
        const encoding = new EncodingVersion();
        encoding._read(this);
        try {
            this._buf.position = this._buf.position + sz - 6;
        } catch (ex) {
            throw new MarshalException(endOfBufferMessage);
        }
        return encoding;
    }

    startSlice() {
        // Returns type ID of next slice
        DEV: console.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        return this._encapsStack.decoder.startSlice();
    }

    endSlice() {
        DEV: console.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        this._encapsStack.decoder.endSlice();
    }

    skipSlice() {
        DEV: console.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        this._encapsStack.decoder.skipSlice();
    }

    readPendingValues() {
        if (this._encapsStack !== null && this._encapsStack.decoder !== null) {
            this._encapsStack.decoder.readPendingValues();
        } else if (
            (this._encapsStack !== null && this._encapsStack.encoding_1_0) ||
            (this._encapsStack === null && this._encoding.equals(Encoding_1_0))
        ) {
            //
            // If using the 1.0 encoding and no instances were read, we
            // still read an empty sequence of pending instances if
            // requested (i.e.: if this is called).
            //
            // This is required by the 1.0 encoding, even if no instances
            // are written we do marshal an empty sequence if marshaled
            // data types use classes.
            //
            this.skipSize();
        }
    }

    readSize() {
        try {
            const b = this._buf.get();
            if (b === 255) {
                const v = this._buf.getInt();
                if (v < 0) {
                    throw new MarshalException(endOfBufferMessage);
                }
                return v;
            }
            return b;
        } catch (ex) {
            throw new MarshalException(endOfBufferMessage);
        }
    }

    readAndCheckSeqSize(minSize) {
        const sz = this.readSize();

        if (sz === 0) {
            return sz;
        }

        //
        // The _startSeq variable points to the start of the sequence for which
        // we expect to read at least _minSeqSize bytes from the stream.
        //
        // If not initialized or if we already read more data than _minSeqSize,
        // we reset _startSeq and _minSeqSize for this sequence (possibly a
        // top-level sequence or enclosed sequence it doesn't really matter).
        //
        // Otherwise, we are reading an enclosed sequence and we have to bump
        // _minSeqSize by the minimum size that this sequence will require on
        // the stream.
        //
        // The goal of this check is to ensure that when we start unmarshaling
        // a new sequence, we check the minimal size of this new sequence against
        // the estimated remaining buffer size. This estimation is based on
        // the minimum size of the enclosing sequences, it's _minSeqSize.
        //
        if (this._startSeq === -1 || this._buf.position > this._startSeq + this._minSeqSize) {
            this._startSeq = this._buf.position;
            this._minSeqSize = sz * minSize;
        } else {
            this._minSeqSize += sz * minSize;
        }

        //
        // If there isn't enough data to read on the stream for the sequence (and
        // possibly enclosed sequences), something is wrong with the marshaled
        // data: it's claiming having more data that what is possible to read.
        //
        if (this._startSeq + this._minSeqSize > this._buf.limit) {
            throw new MarshalException(endOfBufferMessage);
        }

        return sz;
    }

    readBlob(sz) {
        if (this._buf.remaining < sz) {
            throw new MarshalException(endOfBufferMessage);
        }
        try {
            return this._buf.getArray(sz);
        } catch (ex) {
            throw new MarshalException(endOfBufferMessage);
        }
    }

    readOptional(tag, expectedFormat) {
        DEV: console.assert(this._encapsStack !== null);
        if (this._encapsStack.decoder !== null) {
            return this._encapsStack.decoder.readOptional(tag, expectedFormat);
        }
        return this.readOptImpl(tag, expectedFormat);
    }

    readOptionalHelper(tag, format, read) {
        if (this.readOptional(tag, format)) {
            return read.call(this);
        } else {
            return undefined;
        }
    }

    readByte() {
        try {
            return this._buf.get();
        } catch (ex) {
            throw new MarshalException(endOfBufferMessage);
        }
    }

    readByteSeq() {
        return this._buf.getArray(this.readAndCheckSeqSize(1));
    }

    readBool() {
        try {
            return this._buf.get() === 1;
        } catch (ex) {
            throw new MarshalException(endOfBufferMessage);
        }
    }

    readShort() {
        try {
            return this._buf.getShort();
        } catch (ex) {
            throw new MarshalException(endOfBufferMessage);
        }
    }

    readInt() {
        try {
            return this._buf.getInt();
        } catch (ex) {
            throw new MarshalException(endOfBufferMessage);
        }
    }

    readLong() {
        try {
            return this._buf.getLong();
        } catch (ex) {
            throw new MarshalException(endOfBufferMessage);
        }
    }

    readFloat() {
        try {
            return this._buf.getFloat();
        } catch (ex) {
            throw new MarshalException(endOfBufferMessage);
        }
    }

    readDouble() {
        try {
            return this._buf.getDouble();
        } catch (ex) {
            throw new MarshalException(endOfBufferMessage);
        }
    }

    readString() {
        const len = this.readSize();
        if (len === 0) {
            return "";
        }
        //
        // Check the buffer has enough bytes to read.
        //
        if (this._buf.remaining < len) {
            throw new MarshalException(endOfBufferMessage);
        }

        try {
            return this._buf.getString(len);
        } catch (ex) {
            throw new MarshalException(endOfBufferMessage);
        }
    }

    readProxy(type) {
        const ident = new Identity();
        ident._read(this);
        const reference = this._instance.referenceFactory().createFromStream(ident, this);
        const TPrx = type == null ? ObjectPrx : type;
        return reference == null ? null : new TPrx(reference);
    }

    readOptionalProxy(tag, type) {
        if (this.readOptional(tag, OptionalFormat.FSize)) {
            this.skip(4);
            return this.readProxy(type);
        } else {
            return undefined;
        }
    }

    readEnum(T) {
        let v;
        if (this.getEncoding().equals(Encoding_1_0)) {
            if (T.maxValue < 127) {
                v = this.readByte();
            } else if (T.maxValue < 32767) {
                v = this.readShort();
            } else {
                v = this.readInt();
            }
        } else {
            v = this.readSize();
        }

        const e = T.valueOf(v);
        if (e === undefined) {
            throw new MarshalException(`enumerator value '${v}' is out of range`);
        }
        return e;
    }

    readOptionalEnum(tag, T) {
        if (this.readOptional(tag, OptionalFormat.Size)) {
            return this.readEnum(T);
        } else {
            return undefined;
        }
    }

    readValue(cb, T) {
        this.initEncaps();
        this._encapsStack.decoder.readValue(obj => {
            if (obj !== null && !(obj instanceof T)) {
                throwUOE(T.ice_staticId(), obj);
            }
            cb(obj);
        });
    }

    throwException() {
        this.initEncaps();
        this._encapsStack.decoder.throwException();
    }

    readOptImpl(readTag, expectedFormat) {
        if (this.isEncoding_1_0()) {
            return false; // Optional members aren't supported with the 1.0 encoding.
        }

        while (true) {
            if (this._buf.position >= this._encapsStack.start + this._encapsStack.sz) {
                return false; // End of encapsulation also indicates end of optionals.
            }

            const v = this.readByte();

            if (v === Protocol.OPTIONAL_END_MARKER) {
                this._buf.position -= 1; // Rewind.
                return false;
            }

            const format = OptionalFormat.valueOf(v & 0x07); // First 3 bits.
            let tag = v >> 3;
            if (tag === 30) {
                tag = this.readSize();
            }

            if (tag > readTag) {
                const offset = tag < 30 ? 1 : tag < 255 ? 2 : 6; // Rewind
                this._buf.position -= offset;
                return false; // No optional data members with the requested tag.
            } else if (tag < readTag) {
                this.skipOptional(format); // Skip optional data members
            } else {
                if (format !== expectedFormat) {
                    throw new MarshalException(`invalid optional data member '${tag}': unexpected format`);
                }
                return true;
            }
        }
    }

    skipOptional(format) {
        switch (format) {
            case OptionalFormat.F1: {
                this.skip(1);
                break;
            }
            case OptionalFormat.F2: {
                this.skip(2);
                break;
            }
            case OptionalFormat.F4: {
                this.skip(4);
                break;
            }
            case OptionalFormat.F8: {
                this.skip(8);
                break;
            }
            case OptionalFormat.Size: {
                this.skipSize();
                break;
            }
            case OptionalFormat.VSize: {
                this.skip(this.readSize());
                break;
            }
            case OptionalFormat.FSize: {
                this.skip(this.readInt());
                break;
            }
            case OptionalFormat.Class: {
                throw new MarshalException("cannot skip an optional class");
            }
            default: {
                DEV: console.assert(false);
                break;
            }
        }
    }

    skipOptionals() {
        //
        // Skip remaining un-read optional members.
        //
        while (true) {
            if (this._buf.position >= this._encapsStack.start + this._encapsStack.sz) {
                return; // End of encapsulation also indicates end of optionals.
            }

            const b = this.readByte();
            const v = b < 0 ? b + 256 : b;
            if (v === Protocol.OPTIONAL_END_MARKER) {
                return;
            }

            const format = OptionalFormat.valueOf(v & 0x07); // Read first 3 bits.
            if (v >> 3 === 30) {
                this.skipSize();
            }
            this.skipOptional(format);
        }
    }

    skip(size) {
        if (size > this._buf.remaining) {
            throw new MarshalException(endOfBufferMessage);
        }
        this._buf.position += size;
    }

    skipSize() {
        const b = this.readByte();
        if (b === 255) {
            this.skip(4);
        }
    }

    isEmpty() {
        return this._buf.empty();
    }

    createInstance(id) {
        let obj = null;
        const typeId = id.length > 2 ? id.substr(2).replace(/::/g, ".") : "";
        try {
            const Class = TypeRegistry.getValueType(typeId);
            if (Class !== undefined) {
                obj = new Class();
            }
        } catch (ex) {
            throw new MarshalException(`Failed to create a class with type ID '${typeId}'.`, { cause: ex });
        }

        return obj;
    }

    createUserException(id) {
        let userEx = null;
        try {
            const typeId = id.length > 2 ? id.substr(2).replace(/::/g, ".") : "";
            const Class = TypeRegistry.getUserExceptionType(typeId);
            if (Class !== undefined) {
                userEx = new Class();
            }
        } catch (ex) {
            throw new MarshalException(`Failed to create user exception with type ID '${id}'.`, { cause: ex });
        }
        return userEx;
    }

    resolveCompactId(compactId) {
        const typeId = CompactIdRegistry.get(compactId);
        return typeId === undefined ? "" : typeId;
    }

    isEncoding_1_0() {
        return this._encapsStack !== null ? this._encapsStack.encoding_1_0 : this._encoding.equals(Encoding_1_0);
    }

    initEncaps() {
        if (this._encapsStack === null) {
            // Lazy initialization
            this._encapsStack = this._encapsCache;
            if (this._encapsStack !== null) {
                this._encapsCache = this._encapsCache.next;
            } else {
                this._encapsStack = new ReadEncaps();
            }
            this._encapsStack.setEncoding(this._encoding);
            this._encapsStack.sz = this._buf.limit;
        }

        if (this._encapsStack.decoder === null) {
            // Lazy initialization.
            if (this._encapsStack.encoding_1_0) {
                this._encapsStack.decoder = new EncapsDecoder10(
                    this,
                    this._encapsStack,
                    this._classGraphDepthMax,
                    this._valueFactoryManager,
                );
            } else {
                this._encapsStack.decoder = new EncapsDecoder11(
                    this,
                    this._encapsStack,
                    this._classGraphDepthMax,
                    this._valueFactoryManager,
                );
            }
        }
    }

    traceSkipSlice(typeId, sliceType) {
        if (this._instance.traceLevels().slicing > 0) {
            traceSlicing(
                sliceType === SliceType.ExceptionSlice ? "exception" : "object",
                typeId,
                "Slicing",
                this._instance.initializationData().logger,
            );
        }
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

    get instance() {
        return this._instance;
    }

    get buffer() {
        return this._buf;
    }
}
