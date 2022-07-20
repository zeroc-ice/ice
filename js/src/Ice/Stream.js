//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;
const _ModuleRegistry = Ice._ModuleRegistry;

require("../Ice/ArrayUtil");
require("../Ice/Buffer");
require("../Ice/CompactIdRegistry");
require("../Ice/Debug");
require("../Ice/ExUtil");
require("../Ice/Exception");
require("../Ice/FormatType");
require("../Ice/LocalException");
require("../Ice/Object");
require("../Ice/OptionalFormat");
require("../Ice/Protocol");
require("../Ice/TraceUtil");
require("../Ice/UnknownSlicedValue");
require("../Ice/Value");
require("../Ice/Version");

const ArrayUtil = Ice.ArrayUtil;
const Debug = Ice.Debug;
const ExUtil = Ice.ExUtil;
const FormatType = Ice.FormatType;
const OptionalFormat = Ice.OptionalFormat;
const Protocol = Ice.Protocol;
const SlicedData = Ice.SlicedData;
const TraceUtil = Ice.TraceUtil;

const SliceType =
{
    NoSlice: 0,
    ValueSlice: 1,
    ExceptionSlice: 2
};

//
// Number.isNaN polyfill for compatibility with IE
//
// see: https://developer.mozilla.org/en/docs/Web/JavaScript/Reference/Global_Objects/Number/isNaN
//
Number.isNaN = Number.isNaN || function(value)
{
    return typeof value === "number" && isNaN(value);
};

//
// InputStream
//

class IndirectPatchEntry
{
    constructor(index, cb)
    {
        this.index = index;
        this.cb = cb;
    }
}

class EncapsDecoder
{
    constructor(stream, encaps, sliceValues, f)
    {
        this._stream = stream;
        this._encaps = encaps;
        this._sliceValues = sliceValues;
        this._valueFactoryManager = f;
        this._patchMap = null; // Lazy initialized, Map<int, Patcher[] >()
        this._unmarshaledMap = new Map(); // Map<int, Ice.Value>()
        this._typeIdMap = null; // Lazy initialized, Map<int, String>
        this._typeIdIndex = 0;
        this._valueList = null; // Lazy initialized. Ice.Value[]
    }

    readOptional()
    {
        return false;
    }

    readPendingValues()
    {
    }

    readTypeId(isIndex)
    {
        if(this._typeIdMap === null) // Lazy initialization
        {
            this._typeIdMap = new Map(); // Map<int, String>();
        }

        let typeId;
        if(isIndex)
        {
            typeId = this._typeIdMap.get(this._stream.readSize());
            if(typeId === undefined)
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
        }
        else
        {
            typeId = this._stream.readString();
            this._typeIdMap.set(++this._typeIdIndex, typeId);
        }
        return typeId;
    }

    newInstance(typeId)
    {
        //
        // Try to find a factory registered for the specific type.
        //
        let userFactory = this._valueFactoryManager.find(typeId);
        let v = null;

        if(userFactory !== undefined)
        {
            v = userFactory(typeId);
        }

        //
        // If that fails, invoke the default factory if one has been
        // registered.
        //
        if(v === null || v === undefined)
        {
            userFactory = this._valueFactoryManager.find("");
            if(userFactory !== undefined)
            {
                v = userFactory(typeId);
            }
        }

        //
        // Last chance: try to instantiate the class dynamically.
        //
        if(v === null || v === undefined)
        {
            v = this._stream.createInstance(typeId);
        }

        return v;
    }

    addPatchEntry(index, cb)
    {
        Debug.assert(index > 0);

        //
        // Check if we have already unmarshaled the instance. If that's the case,
        // just call the callback and we're done.
        //
        const obj = this._unmarshaledMap.get(index);
        if(obj !== undefined && obj !== null)
        {
            cb(obj);
            return;
        }

        if(this._patchMap === null) // Lazy initialization
        {
            this._patchMap = new Map(); // Map<Integer, Patcher[] >();
        }

        //
        // Add a patch entry if the instance isn't unmarshaled yet,
        // the callback will be called when the instance is
        // unmarshaled.
        //
        let l = this._patchMap.get(index);
        if(l === undefined)
        {
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
        l.push(cb);
    }

    unmarshal(index, v)
    {
        //
        // Add the instance to the map of unmarshaled instances, this must
        // be done before reading the instances (for circular references).
        //
        this._unmarshaledMap.set(index, v);

        //
        // Read the instance.
        //
        v._iceRead(this._stream);

        if(this._patchMap !== null)
        {
            //
            // Patch all instances now that the instance is unmarshaled.
            //
            const l = this._patchMap.get(index);
            if(l !== undefined)
            {
                Debug.assert(l.length > 0);

                //
                // Patch all pointers that refer to the instance.
                //
                for(let i = 0; i < l.length; ++i)
                {
                    l[i](v);
                }

                //
                // Clear out the patch map for that index -- there is nothing left
                // to patch for that index for the time being.
                //
                this._patchMap.delete(index);
            }
        }

        if((this._patchMap === null || this._patchMap.size === 0) && this._valueList === null)
        {
            try
            {
                v.ice_postUnmarshal();
            }
            catch(ex)
            {
                this._stream.instance.initializationData().logger.warning("exception raised by ice_postUnmarshal:\n" +
                    ex.toString());
            }
        }
        else
        {
            if(this._valueList === null) // Lazy initialization
            {
                this._valueList = []; // Ice.Value[]
            }
            this._valueList.push(v);

            if(this._patchMap === null || this._patchMap.size === 0)
            {
                //
                // Iterate over the instance list and invoke ice_postUnmarshal on
                // each instance. We must do this after all instances have been
                // unmarshaled in order to ensure that any instance data members
                // have been properly patched.
                //
                for(let i = 0; i < this._valueList.length; i++)
                {
                    try
                    {
                        this._valueList[i].ice_postUnmarshal();
                    }
                    catch(ex)
                    {
                        this._stream.instance.initializationData().logger.warning(
                            "exception raised by ice_postUnmarshal:\n" + ex.toString());
                    }
                }
                this._valueList = [];
            }
        }
    }
}

class EncapsDecoder10 extends EncapsDecoder
{
    constructor(stream, encaps, sliceValues, f)
    {
        super(stream, encaps, sliceValues, f);
        this._sliceType = SliceType.NoSlice;
    }

    readValue(cb)
    {
        Debug.assert(cb !== null);

        //
        // Instance references are encoded as a negative integer in 1.0.
        //
        let index = this._stream.readInt();
        if(index > 0)
        {
            throw new Ice.MarshalException("invalid object id");
        }
        index = -index;

        if(index === 0)
        {
            cb(null);
        }
        else
        {
            this.addPatchEntry(index, cb);
        }
    }

    throwException()
    {
        Debug.assert(this._sliceType === SliceType.NoSlice);

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
        while(true)
        {
            const userEx = this._stream.createUserException(this._typeId);

            //
            // We found the exception.
            //
            if(userEx !== null)
            {
                userEx._read(this._stream);
                if(usesClasses)
                {
                    this.readPendingValues();
                }
                throw userEx;

                // Never reached.
            }

            //
            // Slice off what we don't understand.
            //
            this.skipSlice();
            try
            {
                this.startSlice();
            }
            catch(ex)
            {
                //
                // An oversight in the 1.0 encoding means there is no marker to indicate
                // the last slice of an exception. As a result, we just try to read the
                // next type ID, which raises UnmarshalOutOfBoundsException when the
                // input buffer underflows.
                //
                // Set the reason member to a more helpful message.
                //
                if(ex instanceof Ice.UnmarshalOutOfBoundsException)
                {
                    ex.reason = "unknown exception type `" + mostDerivedId + "'";
                }
                throw ex;
            }
        }
    }

    startInstance(sliceType)
    {
        Debug.assert(this._sliceType === sliceType);
        this._skipFirstSlice = true;
    }

    endInstance(preserve)
    {
        //
        // Read the Ice::Object slice.
        //
        if(this._sliceType === SliceType.ValueSlice)
        {
            this.startSlice();
            const sz = this._stream.readSize(); // For compatibility with the old AFM.
            if(sz !== 0)
            {
                throw new Ice.MarshalException("invalid Object slice");
            }
            this.endSlice();
        }

        this._sliceType = SliceType.NoSlice;
        return null;
    }

    startSlice()
    {
        //
        // If first slice, don't read the header, it was already read in
        // readInstance or throwException to find the factory.
        //
        if(this._skipFirstSlice)
        {
            this._skipFirstSlice = false;
            return this._typeId;
        }

        //
        // For instances, first read the type ID boolean which indicates
        // whether or not the type ID is encoded as a string or as an
        // index. For exceptions, the type ID is always encoded as a
        // string.
        //
        if(this._sliceType === SliceType.ValueSlice) // For exceptions, the type ID is always encoded as a string
        {
            const isIndex = this._stream.readBool();
            this._typeId = this.readTypeId(isIndex);
        }
        else
        {
            this._typeId = this._stream.readString();
        }

        this._sliceSize = this._stream.readInt();
        if(this._sliceSize < 4)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }

        return this._typeId;
    }

    endSlice()
    {
    }

    skipSlice()
    {
        this._stream.traceSkipSlice(this._typeId, this._sliceType);
        Debug.assert(this._sliceSize >= 4);
        this._stream.skip(this._sliceSize - 4);
    }

    readPendingValues()
    {
        let num;
        do
        {
            num = this._stream.readSize();
            for(let k = num; k > 0; --k)
            {
                this.readInstance();
            }
        }
        while(num > 0);

        if(this._patchMap !== null && this._patchMap.size !== 0)
        {
            //
            // If any entries remain in the patch map, the sender has sent an index for an instance, but failed
            // to supply the instance.
            //
            throw new Ice.MarshalException("index for class received, but no instance");
        }
    }

    readInstance()
    {
        const index = this._stream.readInt();
        let v = null;

        if(index <= 0)
        {
            throw new Ice.MarshalException("invalid object id");
        }

        this._sliceType = SliceType.ValueSlice;
        this._skipFirstSlice = false;

        //
        // Read the first slice header.
        //
        this.startSlice();
        const mostDerivedId = this._typeId;
        while(true)
        {
            //
            // For the 1.0 encoding, the type ID for the base Object class
            // marks the last slice.
            //
            if(this._typeId == Ice.Value.ice_staticId())
            {
                throw new Ice.NoValueFactoryException("", mostDerivedId);
            }

            v = this.newInstance(this._typeId);

            //
            // We found a factory, we get out of this loop.
            //
            if(v)
            {
                break;
            }

            //
            // If slicing is disabled, stop unmarshaling.
            //
            if(!this._sliceValues)
            {
                throw new Ice.NoValueFactoryException("no value factory found and slicing is disabled",
                                                       this._typeId);
            }

            //
            // Slice off what we don't understand.
            //
            this.skipSlice();
            this.startSlice(); // Read next Slice header for next iteration.
        }

        //
        // Unmarshal the instance and add it to the map of unmarshaled instances.
        //
        this.unmarshal(index, v);
    }
}

class EncapsDecoder11 extends EncapsDecoder
{
    constructor(stream, encaps, sliceValues, f, r)
    {
        super(stream, encaps, sliceValues, f);
        this._compactIdResolver = r;
        this._current = null;
        this._valueIdIndex = 1;
    }

    readValue(cb)
    {
        const index = this._stream.readSize();
        if(index < 0)
        {
            throw new Ice.MarshalException("invalid object id");
        }
        else if(index === 0)
        {
            if(cb !== null)
            {
                cb(null);
            }
        }
        else if(this._current !== null && (this._current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) !== 0)
        {
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
            if(cb !== null)
            {
                if(this._current.indirectPatchList === null) // Lazy initialization
                {
                    this._current.indirectPatchList = []; // IndirectPatchEntry[]
                }
                this._current.indirectPatchList.push(new IndirectPatchEntry(index - 1, cb));
            }
        }
        else
        {
            this.readInstance(index, cb);
        }
    }

    throwException()
    {
        Debug.assert(this._current === null);

        this.push(SliceType.ExceptionSlice);

        //
        // Read the first slice header.
        //
        this.startSlice();
        const mostDerivedId = this._current.typeId;
        while(true)
        {

            const userEx = this._stream.createUserException(this._current.typeId);

            //
            // We found the exception.
            //
            if(userEx !== null)
            {
                userEx._read(this._stream);
                throw userEx;

                // Never reached.
            }

            //
            // Slice off what we don't understand.
            //
            this.skipSlice();

            if((this._current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) !== 0)
            {
                if(mostDerivedId.indexOf("::") === 0)
                {
                    throw new Ice.UnknownUserException(mostDerivedId.substr(2));
                }
                throw new Ice.UnknownUserException(mostDerivedId);
            }

            this.startSlice();
        }
    }

    startInstance(sliceType)
    {
        Debug.assert(sliceType !== undefined);
        Debug.assert(this._current.sliceType !== null && this._current.sliceType === sliceType);
        this._current.skipFirstSlice = true;
    }

    endInstance(preserve)
    {
        let slicedData = null;
        if(preserve)
        {
            slicedData = this.readSlicedData();
        }
        if(this._current.slices !== null)
        {
            this._current.slices.length = 0; // Clear the array.
            this._current.indirectionTables.length = 0; // Clear the array.
        }
        this._current = this._current.previous;
        return slicedData;
    }

    startSlice()
    {
        //
        // If first slice, don't read the header, it was already read in
        // readInstance or throwException to find the factory.
        //
        if(this._current.skipFirstSlice)
        {
            this._current.skipFirstSlice = false;
            return this._current.typeId;
        }

        this._current.sliceFlags = this._stream.readByte();

        //
        // Read the type ID, for instance slices the type ID is encoded as a
        // string or as an index, for exceptions it's always encoded as a
        // string.
        //
        if(this._current.sliceType === SliceType.ValueSlice)
        {
            if((this._current.sliceFlags & Protocol.FLAG_HAS_TYPE_ID_COMPACT) ===
                Protocol.FLAG_HAS_TYPE_ID_COMPACT) // Must be checked 1st!
            {
                this._current.typeId = "";
                this._current.compactId = this._stream.readSize();
            }
            else if((this._current.sliceFlags & (Protocol.FLAG_HAS_TYPE_ID_INDEX |
                        Protocol.FLAG_HAS_TYPE_ID_STRING)) !== 0)
            {
                this._current.typeId =
                    this.readTypeId((this._current.sliceFlags & Protocol.FLAG_HAS_TYPE_ID_INDEX) !== 0);
                this._current.compactId = -1;
            }
            else
            {
                //
                // Only the most derived slice encodes the type ID for the compact format.
                //
                this._current.typeId = "";
                this._current.compactId = -1;
            }
        }
        else
        {
            this._current.typeId = this._stream.readString();
            this._current.compactId = -1;
        }

        //
        // Read the slice size if necessary.
        //
        if((this._current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) !== 0)
        {
            this._current.sliceSize = this._stream.readInt();
            if(this._current.sliceSize < 4)
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
        }
        else
        {
            this._current.sliceSize = 0;
        }

        return this._current.typeId;
    }

    endSlice()
    {
        if((this._current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) !== 0)
        {
            this._stream.skipOptionals();
        }

        //
        // Read the indirection table if one is present and transform the
        // indirect patch list into patch entries with direct references.
        //
        if((this._current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) !== 0)
        {
            const indirectionTable = [];
            //
            // The table is written as a sequence<size> to conserve space.
            //
            const length = this._stream.readAndCheckSeqSize(1);
            for(let i = 0; i < length; ++i)
            {
                indirectionTable[i] = this.readInstance(this._stream.readSize(), null);
            }

            //
            // Sanity checks. If there are optional members, it's possible
            // that not all instance references were read if they are from
            // unknown optional data members.
            //
            if(indirectionTable.length === 0)
            {
                throw new Ice.MarshalException("empty indirection table");
            }
            if((this._current.indirectPatchList === null || this._current.indirectPatchList.length === 0) &&
               (this._current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) === 0)
            {
                throw new Ice.MarshalException("no references to indirection table");
            }

            //
            // Convert indirect references into direct references.
            //
            if(this._current.indirectPatchList !== null)
            {
                this._current.indirectPatchList.forEach(e =>
                    {
                        Debug.assert(e.index >= 0);
                        if(e.index >= indirectionTable.length)
                        {
                            throw new Ice.MarshalException("indirection out of range");
                        }
                        this.addPatchEntry(indirectionTable[e.index], e.cb);
                    });
                this._current.indirectPatchList.length = 0;
            }
        }
    }

    skipSlice()
    {
        this._stream.traceSkipSlice(this._current.typeId, this._current.sliceType);

        const start = this._stream.pos;

        if((this._current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) !== 0)
        {
            Debug.assert(this._current.sliceSize >= 4);
            this._stream.skip(this._current.sliceSize - 4);
        }
        else if(this._current.sliceType === SliceType.ValueSlice)
        {
            throw new Ice.NoValueFactoryException("no value factory found and compact format prevents slicing " +
                                                  "(the sender should use the sliced format instead)",
                                                  this._current.typeId);
        }
        else if(this._current.typeId.indexOf("::") === 0)
        {
            throw new Ice.UnknownUserException(this._current.typeId.substring(2));
        }
        else
        {
            throw new Ice.UnknownUserException(this._current.typeId);
        }

        //
        // Preserve this slice.
        //
        const info = new Ice.SliceInfo();
        info.typeId = this._current.typeId;
        info.compactId = this._current.compactId;
        info.hasOptionalMembers = (this._current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) !== 0;
        info.isLastSlice = (this._current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) !== 0;

        const b = this._stream._buf;
        const end = b.position;
        let dataEnd = end;
        if(info.hasOptionalMembers)
        {
            //
            // Don't include the optional member end marker. It will be re-written by
            // endSlice when the sliced data is re-written.
            //
            --dataEnd;
        }

        b.position = start;
        info.bytes = b.getArray(dataEnd - start);
        b.position = end;

        if(this._current.slices === null) // Lazy initialization
        {
            this._current.slices = []; // Ice.SliceInfo[]
            this._current.indirectionTables = []; // int[]
        }

        //
        // Read the indirect instance table. We read the instances or their
        // IDs if the instance is a reference to an already unmarshaled
        // instance.
        //

        if((this._current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE) !== 0)
        {
            const length = this._stream.readAndCheckSeqSize(1);
            const indirectionTable = [];
            for(let i = 0; i < length; ++i)
            {
                indirectionTable[i] = this.readInstance(this._stream.readSize(), null);
            }
            this._current.indirectionTables.push(indirectionTable);
        }
        else
        {
            this._current.indirectionTables.push(null);
        }

        this._current.slices.push(info);
    }

    readOptional(readTag, expectedFormat)
    {
        if(this._current === null)
        {
            return this._stream.readOptImpl(readTag, expectedFormat);
        }
        else if((this._current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) !== 0)
        {
            return this._stream.readOptImpl(readTag, expectedFormat);
        }
        return false;
    }

    readInstance(index, cb)
    {
        Debug.assert(index > 0);

        let v = null;

        if(index > 1)
        {
            if(cb !== null)
            {
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
        while(true)
        {
            if(this._current.compactId >= 0)
            {
                //
                // Translate a compact (numeric) type ID into a string type ID.
                //
                this._current.typeId = "";
                if(this._compactIdResolver !== null)
                {
                    try
                    {
                        this._current.typeId = this._compactIdResolver.call(null, this._current.compactId);
                    }
                    catch(ex)
                    {
                        if(!(ex instanceof Ice.LocalException))
                        {
                            throw new Ice.MarshalException("exception in CompactIdResolver for ID " +
                                                           this._current.compactId, ex);
                        }
                        throw ex;
                    }
                }

                if(this._current.typeId.length === 0)
                {
                    this._current.typeId = this._stream.resolveCompactId(this._current.compactId);
                }
            }

            if(this._current.typeId.length > 0)
            {
                v = this.newInstance(this._current.typeId);
            }

            if(v !== null && v !== undefined)
            {
                //
                // We have an instance, we get out of this loop.
                //
                break;
            }

            //
            // If slicing is disabled, stop unmarshaling.
            //
            if(!this._sliceValues)
            {
                throw new Ice.NoValueFactoryException("no value factory found and slicing is disabled",
                                                       this._current.typeId);
            }

            //
            // Slice off what we don't understand.
            //
            this.skipSlice();

            //
            // If this is the last slice, keep the instance as an opaque
            // UnknownSlicedValue object.
            //
            if((this._current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) !== 0)
            {
                v = new Ice.UnknownSlicedValue(mostDerivedId);
                break;
            }

            this.startSlice(); // Read next Slice header for next iteration.
        }

        //
        // Unmarshal the instance.
        //
        this.unmarshal(index, v);

        if(this._current === null && this._patchMap !== null && this._patchMap.size !== 0)
        {
            //
            // If any entries remain in the patch map, the sender has sent an index for an instance, but failed
            // to supply the instance.
            //
            throw new Ice.MarshalException("index for class received, but no instance");
        }

        if(cb !== null)
        {
            cb(v);
        }

        return index;
    }

    readSlicedData()
    {
        if(this._current.slices === null) // No preserved slices.
        {
            return null;
        }

        //
        // The _indirectionTables member holds the indirection table for each slice
        // in _slices.
        //
        Debug.assert(this._current.slices.length === this._current.indirectionTables.length);
        for(let i = 0; i < this._current.slices.length; ++i)
        {
            //
            // We use the "instances" list in SliceInfo to hold references
            // to the target instances. Note that the instances might not have
            // been read yet in the case of a circular reference to an
            // enclosing instance.
            //
            const table = this._current.indirectionTables[i];
            const info = this._current.slices[i];
            info.instances = [];
            if(table)
            {
                for(let j = 0; j < table.length; ++j)
                {
                    this.addPatchEntry(table[j], sequencePatcher(info.instances, j, Ice.Value));
                }
            }
        }
        return new SlicedData(ArrayUtil.clone(this._current.slices));
    }

    push(sliceType)
    {
        if(this._current === null)
        {
            this._current = new EncapsDecoder11.InstanceData(null);
        }
        else
        {
            this._current = !this._current.next ? new EncapsDecoder11.InstanceData(this._current) : this._current.next;
        }
        this._current.sliceType = sliceType;
        this._current.skipFirstSlice = false;
    }
}

EncapsDecoder11.InstanceData = class
{
    constructor(previous)
    {
        if(previous !== null)
        {
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

const sequencePatcher = function(seq, index, T)
{
    return v =>
        {
            if(v !== null && !(v instanceof T))
            {
                ExUtil.throwUOE(T.ice_staticId(), v);
            }
            seq[index] = v;
        };
};

class ReadEncaps
{
    constructor()
    {
        this.start = 0;
        this.sz = 0;
        this.encoding = null;
        this.encoding_1_0 = false;
        this.decoder = null;
        this.next = null;
    }

    reset()
    {
        this.decoder = null;
    }

    setEncoding(encoding)
    {
        this.encoding = encoding;
        this.encoding_1_0 = encoding.equals(Ice.Encoding_1_0);
    }
}

class InputStream
{
    constructor(arg1, arg2, arg3)
    {
        const args =
        {
            instance: null,
            encoding: null,
            bytes: null,
            buffer: null
        };
        this._checkArgs([arg1, arg2, arg3], args);
        this._initialize(args);
    }

    _checkArgs(arr, args)
    {
        //
        // The constructor can accept a variety of argument combinations:
        //
        // (<empty>)
        // (communicator)
        // (instance)
        // (encoding)
        // (array)
        // (buffer)
        // (communicator, encoding)
        // (instance, encoding)
        // (communicator, array)
        // (instance, array)
        // (communicator, buffer)
        // (instance, buffer)
        // (communicator, encoding, array)
        // (instance, encoding, array)
        // (communicator, encoding, buffer)
        // (instance, encoding, buffer)
        // (encoding, array)
        // (encoding, array)
        // (encoding, buffer)
        // (encoding, buffer)
        //
        arr.forEach(arg =>
            {
                if(arg !== null && arg !== undefined)
                {
                    if(arg.constructor === Ice.Communicator)
                    {
                        args.instance = arg.instance;
                    }
                    else if(arg.constructor === Ice.Instance)
                    {
                        args.instance = arg;
                    }
                    else if(arg.constructor === Ice.EncodingVersion)
                    {
                        args.encoding = arg;
                    }
                    else if(arg.constructor === Ice.Buffer)
                    {
                        args.buffer = arg;
                    }
                    else if(arg.constructor === ArrayBuffer)
                    {
                        args.bytes = arg;
                    }
                    else if(arg.constructor === Uint8Array)
                    {
                        args.bytes = arg.buffer;
                    }
                    else
                    {
                        throw new Ice.InitializationException("unknown argument to InputStream constructor");
                    }
                }
            });
        if(args.buffer !== null && args.bytes !== null)
        {
            throw new Ice.InitializationException("invalid argument to InputStream constructor");
        }
    }

    _initialize(args)
    {
        this._instance = args.instance;
        this._encoding = args.encoding;
        this._encapsStack = null;
        this._encapsCache = null;
        this._closure = null;
        this._sliceValues = true;
        this._startSeq = -1;
        this._sizePos = -1;
        this._compactIdResolver = null;

        if(this._instance !== null)
        {
            if(this._encoding === null)
            {
                this._encoding = this._instance.defaultsAndOverrides().defaultEncoding;
            }
            this._traceSlicing = this._instance.traceLevels().slicing > 0;
            this._valueFactoryManager = this._instance.initializationData().valueFactoryManager;
            this._logger = this._instance.initializationData().logger;
        }
        else
        {
            if(this._encoding === null)
            {
                this._encoding = Protocol.currentEncoding;
            }
            this._traceSlicing = false;
            this._valueFactoryManager = null;
            this._logger = null;
        }

        if(args.bytes !== null)
        {
            this._buf = new Ice.Buffer(args.bytes);
        }
        else if(args.buffer !== null)
        {
            this._buf = args.buffer;
        }
        else
        {
            this._buf = new Ice.Buffer();
        }
    }

    //
    // This function allows this object to be reused, rather than reallocated.
    //
    reset()
    {
        this._buf.reset();
        this.clear();
    }

    clear()
    {
        if(this._encapsStack !== null)
        {
            Debug.assert(this._encapsStack.next === null);
            this._encapsStack.next = this._encapsCache;
            this._encapsCache = this._encapsStack;
            this._encapsCache.reset();
            this._encapsStack = null;
        }

        this._startSeq = -1;
        this._sliceValues = true;
    }

    swap(other)
    {
        Debug.assert(this._instance === other._instance);

        [other._buf, this._buf] = [this._buf, other._buf];
        [other._encoding, this._encoding] = [this._encoding, other._encoding];
        [other._traceSlicing, this._traceSlicing] = [this._traceSlicing, other._traceSlicing];
        [other._closure, this._closure] = [this._closure, other.closure];
        [other._sliceValues, this._sliceValues] = [this._sliceValues, other._sliceValues];

        //
        // Swap is never called for InputStreams that have encapsulations being read/write. However,
        // encapsulations might still be set in case marshaling or unmarshaling failed. We just
        // reset the encapsulations if there are still some set.
        //
        this.resetEncapsulation();
        other.resetEncapsulation();

        [other._startSeq, this._startSeq] = [this._startSeq, other._startSeq];
        [other._minSeqSize, this._minSeqSize] = [this._minSeqSize, other._minSeqSize];
        [other._sizePos, this._sizePos] = [this._sizePos, other._sizePos];
        [other._valueFactoryManager, this._valueFactoryManager] = [this._valueFactoryManager, other._valueFactoryManager];
        [other._logger, this._logger] = [this._logger, other._logger];
        [other._compactIdResolver, this._compactIdResolver] = [this._compactIdResolver, other._compactIdResolver];
    }

    resetEncapsulation()
    {
        this._encapsStack = null;
    }

    resize(sz)
    {
        this._buf.resize(sz);
        this._buf.position = sz;
    }

    startValue()
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        this._encapsStack.decoder.startInstance(SliceType.ValueSlice);
    }

    endValue(preserve)
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        return this._encapsStack.decoder.endInstance(preserve);
    }

    startException()
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        this._encapsStack.decoder.startInstance(SliceType.ExceptionSlice);
    }

    endException(preserve)
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        return this._encapsStack.decoder.endInstance(preserve);
    }

    startEncapsulation()
    {
        let curr = this._encapsCache;
        if(curr !== null)
        {
            curr.reset();
            this._encapsCache = this._encapsCache.next;
        }
        else
        {
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
        if(sz < 6)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        if(sz - 4 > this._buf.remaining)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        this._encapsStack.sz = sz;

        const encoding = new Ice.EncodingVersion();
        encoding._read(this);
        Protocol.checkSupportedEncoding(encoding); // Make sure the encoding is supported.
        this._encapsStack.setEncoding(encoding);

        return encoding;
    }

    endEncapsulation()
    {
        Debug.assert(this._encapsStack !== null);

        if(!this._encapsStack.encoding_1_0)
        {
            this.skipOptionals();
            if(this._buf.position !== this._encapsStack.start + this._encapsStack.sz)
            {
                throw new Ice.EncapsulationException();
            }
        }
        else if(this._buf.position !== this._encapsStack.start + this._encapsStack.sz)
        {
            if(this._buf.position + 1 !== this._encapsStack.start + this._encapsStack.sz)
            {
                throw new Ice.EncapsulationException();
            }

            //
            // Ice version < 3.3 had a bug where user exceptions with
            // class members could be encoded with a trailing byte
            // when dispatched with AMD. So we tolerate an extra byte
            // in the encapsulation.
            //

            try
            {
                this._buf.get();
            }
            catch(ex)
            {
                throw new Ice.UnmarshalOutOfBoundsException();
            }
        }

        const curr = this._encapsStack;
        this._encapsStack = curr.next;
        curr.next = this._encapsCache;
        this._encapsCache = curr;
        this._encapsCache.reset();
    }

    skipEmptyEncapsulation()
    {
        const sz = this.readInt();
        if(sz < 6)
        {
            throw new Ice.EncapsulationException();
        }
        if(sz - 4 > this._buf.remaining)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }

        const encoding = new Ice.EncodingVersion();
        encoding._read(this);
        Protocol.checkSupportedEncoding(encoding); // Make sure the encoding is supported.

        if(encoding.equals(Ice.Encoding_1_0))
        {
            if(sz != 6)
            {
                throw new Ice.EncapsulationException();
            }
        }
        else
        {
            // Skip the optional content of the encapsulation if we are expecting an
            // empty encapsulation.
            this._buf.position = this._buf.position + sz - 6;
        }
        return encoding;
    }

    readEncapsulation(encoding)
    {
        Debug.assert(encoding !== undefined);
        const sz = this.readInt();
        if(sz < 6)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }

        if(sz - 4 > this._buf.remaining)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }

        if(encoding !== null)
        {
            encoding._read(this);
            this._buf.position = this._buf.position - 6;
        }
        else
        {
            this._buf.position = this._buf.position - 4;
        }

        try
        {
            return this._buf.getArray(sz);
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    getEncoding()
    {
        return this._encapsStack !== null ? this._encapsStack.encoding : this._encoding;
    }

    getEncapsulationSize()
    {
        Debug.assert(this._encapsStack !== null);
        return this._encapsStack.sz - 6;
    }

    skipEncapsulation()
    {
        const sz = this.readInt();
        if(sz < 6)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        const encoding = new Ice.EncodingVersion();
        encoding._read(this);
        try
        {
            this._buf.position = this._buf.position + sz - 6;
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        return encoding;
    }

    startSlice() // Returns type ID of next slice
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        return this._encapsStack.decoder.startSlice();
    }

    endSlice()
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        this._encapsStack.decoder.endSlice();
    }

    skipSlice()
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        this._encapsStack.decoder.skipSlice();
    }

    readPendingValues()
    {
        if(this._encapsStack !== null && this._encapsStack.decoder !== null)
        {
            this._encapsStack.decoder.readPendingValues();
        }
        else if((this._encapsStack !== null && this._encapsStack.encoding_1_0) ||
                (this._encapsStack === null && this._encoding.equals(Ice.Encoding_1_0)))
        {
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

    readSize()
    {
        try
        {
            const b = this._buf.get();
            if(b === 255)
            {
                const v = this._buf.getInt();
                if(v < 0)
                {
                    throw new Ice.UnmarshalOutOfBoundsException();
                }
                return v;
            }
            return b;
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    readAndCheckSeqSize(minSize)
    {
        const sz = this.readSize();

        if(sz === 0)
        {
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
        // _minSeqSize by the minimum size that this sequence will  require on
        // the stream.
        //
        // The goal of this check is to ensure that when we start unmarshaling
        // a new sequence, we check the minimal size of this new sequence against
        // the estimated remaining buffer size. This estimatation is based on
        // the minimum size of the enclosing sequences, it's _minSeqSize.
        //
        if(this._startSeq === -1 || this._buf.position > (this._startSeq + this._minSeqSize))
        {
            this._startSeq = this._buf.position;
            this._minSeqSize = sz * minSize;
        }
        else
        {
            this._minSeqSize += sz * minSize;
        }

        //
        // If there isn't enough data to read on the stream for the sequence (and
        // possibly enclosed sequences), something is wrong with the marshaled
        // data: it's claiming having more data that what is possible to read.
        //
        if(this._startSeq + this._minSeqSize > this._buf.limit)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }

        return sz;
    }

    readBlob(sz)
    {
        if(this._buf.remaining < sz)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        try
        {
            return this._buf.getArray(sz);
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    readOptional(tag, expectedFormat)
    {
        Debug.assert(this._encapsStack !== null);
        if(this._encapsStack.decoder !== null)
        {
            return this._encapsStack.decoder.readOptional(tag, expectedFormat);
        }
        return this.readOptImpl(tag, expectedFormat);
    }

    readOptionalHelper(tag, format, read)
    {
        if(this.readOptional(tag, format))
        {
            return read.call(this);
        }
        else
        {
            return undefined;
        }
    }

    readByte()
    {
        try
        {
            return this._buf.get();
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    readByteSeq()
    {
        return this._buf.getArray(this.readAndCheckSeqSize(1));
    }

    readBool()
    {
        try
        {
            return this._buf.get() === 1;
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    readShort()
    {
        try
        {
            return this._buf.getShort();
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    readInt()
    {
        try
        {
            return this._buf.getInt();
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    readLong()
    {
        try
        {
            return this._buf.getLong();
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    readFloat()
    {
        try
        {
            return this._buf.getFloat();
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    readDouble()
    {
        try
        {
            return this._buf.getDouble();
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    readString()
    {
        const len = this.readSize();
        if(len === 0)
        {
            return "";
        }
        //
        // Check the buffer has enough bytes to read.
        //
        if(this._buf.remaining < len)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }

        try
        {
            return this._buf.getString(len);
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    }

    readProxy(type)
    {
        return this._instance.proxyFactory().streamToProxy(this, type);
    }

    readOptionalProxy(tag, type)
    {
        if(this.readOptional(tag, OptionalFormat.FSize))
        {
            this.skip(4);
            return this.readProxy(type);
        }
        else
        {
            return undefined;
        }
    }

    readEnum(T)
    {
        let v;
        if(this.getEncoding().equals(Ice.Encoding_1_0))
        {
            if(T.maxValue < 127)
            {
                v = this.readByte();
            }
            else if(T.maxValue < 32767)
            {
                v = this.readShort();
            }
            else
            {
                v = this.readInt();
            }
        }
        else
        {
            v = this.readSize();
        }

        const e = T.valueOf(v);
        if(e === undefined)
        {
            throw new Ice.MarshalException("enumerator value " + v + " is out of range");
        }
        return e;
    }

    readOptionalEnum(tag, T)
    {
        if(this.readOptional(tag, OptionalFormat.Size))
        {
            return this.readEnum(T);
        }
        else
        {
            return undefined;
        }
    }

    readValue(cb, T)
    {
        this.initEncaps();
        this._encapsStack.decoder.readValue(
            cb === null ? null : obj =>
            {
                if(obj !== null && !(obj instanceof T))
                {
                    ExUtil.throwUOE(T.ice_staticId(), obj);
                }
                cb(obj);
            });
    }

    readOptionalValue(tag, cb, T)
    {
        if(this.readOptional(tag, OptionalFormat.Class))
        {
            this.readValue(cb, T);
        }
        else
        {
            cb(undefined);
        }
    }

    throwException()
    {
        this.initEncaps();
        this._encapsStack.decoder.throwException();
    }

    readOptImpl(readTag, expectedFormat)
    {
        if(this.isEncoding_1_0())
        {
            return false; // Optional members aren't supported with the 1.0 encoding.
        }

        while(true)
        {
            if(this._buf.position >= this._encapsStack.start + this._encapsStack.sz)
            {
                return false; // End of encapsulation also indicates end of optionals.
            }

            const v = this.readByte();

            if(v === Protocol.OPTIONAL_END_MARKER)
            {
                this._buf.position -= 1; // Rewind.
                return false;
            }

            const format = OptionalFormat.valueOf(v & 0x07); // First 3 bits.
            let tag = v >> 3;
            if(tag === 30)
            {
                tag = this.readSize();
            }

            if(tag > readTag)
            {
                const offset = tag < 30 ? 1 : (tag < 255 ? 2 : 6); // Rewind
                this._buf.position -= offset;
                return false; // No optional data members with the requested tag.
            }
            else if(tag < readTag)
            {
                this.skipOptional(format); // Skip optional data members
            }
            else
            {
                if(format !== expectedFormat)
                {
                    throw new Ice.MarshalException("invalid optional data member `" + tag + "': unexpected format");
                }
                return true;
            }
        }
    }

    skipOptional(format)
    {
        switch(format)
        {
            case OptionalFormat.F1:
            {
                this.skip(1);
                break;
            }
            case OptionalFormat.F2:
            {
                this.skip(2);
                break;
            }
            case OptionalFormat.F4:
            {
                this.skip(4);
                break;
            }
            case OptionalFormat.F8:
            {
                this.skip(8);
                break;
            }
            case OptionalFormat.Size:
            {
                this.skipSize();
                break;
            }
            case OptionalFormat.VSize:
            {
                this.skip(this.readSize());
                break;
            }
            case OptionalFormat.FSize:
            {
                this.skip(this.readInt());
                break;
            }
            case OptionalFormat.Class:
            {
                this.readValue(null, Ice.Value);
                break;
            }
            default:
            {
                Debug.assert(false);
                break;
            }
        }
    }

    skipOptionals()
    {
        //
        // Skip remaining un-read optional members.
        //
        while(true)
        {
            if(this._buf.position >= this._encapsStack.start + this._encapsStack.sz)
            {
                return; // End of encapsulation also indicates end of optionals.
            }

            const b = this.readByte();
            const v = b < 0 ? b + 256 : b;
            if(v === Protocol.OPTIONAL_END_MARKER)
            {
                return;
            }

            const format = OptionalFormat.valueOf(v & 0x07); // Read first 3 bits.
            if((v >> 3) === 30)
            {
                this.skipSize();
            }
            this.skipOptional(format);
        }
    }

    skip(size)
    {
        if(size > this._buf.remaining)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        this._buf.position += size;
    }

    skipSize()
    {
        const b = this.readByte();
        if(b === 255)
        {
            this.skip(4);
        }
    }

    isEmpty()
    {
        return this._buf.empty();
    }

    expand(n)
    {
        this._buf.expand(n);
    }

    createInstance(id)
    {
        let obj = null;
        try
        {
            const typeId = id.length > 2 ? id.substr(2).replace(/::/g, ".") : "";
            const Class = _ModuleRegistry.type(typeId);
            if(Class !== undefined)
            {
                obj = new Class();
            }
        }
        catch(ex)
        {
            throw new Ice.NoValueFactoryException("no value factory", id, ex);
        }

        return obj;
    }

    createUserException(id)
    {
        let userEx = null;
        try
        {
            const typeId = id.length > 2 ? id.substr(2).replace(/::/g, ".") : "";
            const Class = _ModuleRegistry.type(typeId);
            if(Class !== undefined)
            {
                userEx = new Class();
            }
        }
        catch(ex)
        {
            throw new Ice.MarshalException(ex);
        }
        return userEx;
    }

    resolveCompactId(compactId)
    {
        const typeId = Ice.CompactIdRegistry.get(compactId);
        return typeId === undefined ? "" : typeId;
    }

    isEncoding_1_0()
    {
        return this._encapsStack !== null ? this._encapsStack.encoding_1_0 : this._encoding.equals(Ice.Encoding_1_0);
    }

    initEncaps()
    {
        if(this._encapsStack === null) // Lazy initialization
        {
            this._encapsStack = this._encapsCache;
            if(this._encapsStack !== null)
            {
                this._encapsCache = this._encapsCache.next;
            }
            else
            {
                this._encapsStack = new ReadEncaps();
            }
            this._encapsStack.setEncoding(this._encoding);
            this._encapsStack.sz = this._buf.limit;
        }

        if(this._encapsStack.decoder === null) // Lazy initialization.
        {
            if(this._encapsStack.encoding_1_0)
            {
                this._encapsStack.decoder = new EncapsDecoder10(this, this._encapsStack, this._sliceValues,
                                                                this._valueFactoryManager);
            }
            else
            {
                this._encapsStack.decoder = new EncapsDecoder11(this, this._encapsStack, this._sliceValues,
                                                                this._valueFactoryManager, this._compactIdResolver);
            }
        }
    }

    traceSkipSlice(typeId, sliceType)
    {
        if(this._traceSlicing && this._logger !== null)
        {
            TraceUtil.traceSlicing(sliceType === SliceType.ExceptionSlice ? "exception" : "object", typeId, "Slicing",
                                   this._logger);
        }
    }

    //
    // Sets the value factory manager to use when marshaling value instances. If the stream
    // was initialized with a communicator, the communicator's value factory manager will
    // be used by default.
    //
    get valueFactoryManager()
    {
        return this._valueFactoryManager;
    }

    set valueFactoryManager(value)
    {
        this._valueFactoryManager = value !== undefined ? value : null;
    }

    //
    // Sets the logger to use when logging trace messages. If the stream
    // was initialized with a communicator, the communicator's logger will
    // be used by default.
    //
    get logger()
    {
        return this._logger;
    }

    set logger(value)
    {
        this._logger = value !== undefined ? value : null;
    }

    //
    // Sets the compact ID resolver to use when unmarshaling value and exception
    // instances. If the stream was initialized with a communicator, the communicator's
    // resolver will be used by default.
    //
    get compactIdResolver()
    {
        return this._compactIdResolver;
    }

    set compactIdResolver(value)
    {
        this._compactIdResolver = value !== undefined ? value : null;
    }

    //
    // Determines the behavior of the stream when extracting instances of Slice classes.
    // A instance is "sliced" when a factory cannot be found for a Slice type ID.
    // The stream's default behavior is to slice instances.
    //
    // If slicing is disabled and the stream encounters a Slice type ID
    // during decoding for which no value factory is installed, it raises
    // NoValueFactoryException.
    //
    get sliceValues()
    {
        return this._sliceValues;
    }

    set sliceValues(value)
    {
        this._sliceValues = value;
    }

    //
    // Determines whether the stream logs messages about slicing instances of Slice values.
    //
    get traceSlicing()
    {
        return this._traceSlicing;
    }

    set traceSlicing(value)
    {
        this._traceSlicing = value;
    }

    get pos()
    {
        return this._buf.position;
    }

    set pos(value)
    {
        this._buf.position = value;
    }

    get size()
    {
        return this._buf.limit;
    }

    get instance()
    {
        return this._instance;
    }

    get closure()
    {
        return this._type;
    }

    set closure(value)
    {
        this._type = value;
    }

    get buffer()
    {
        return this._buf;
    }
}

//
// OutputStream
//

class EncapsEncoder
{
    constructor(stream, encaps)
    {
        this._stream = stream;
        this._encaps = encaps;
        this._marshaledMap = new Map(); // Map<Ice.Value, int>;
        this._typeIdMap = null; // Lazy initialized. Map<String, int>
        this._typeIdIndex = 0;
    }

    writeOptional()
    {
        return false;
    }

    writePendingValues()
    {
        return undefined;
    }

    registerTypeId(typeId)
    {
        if(this._typeIdMap === null) // Lazy initialization
        {
            this._typeIdMap = new Map(); // Map<String, int>
        }

        const p = this._typeIdMap.get(typeId);
        if(p !== undefined)
        {
            return p;
        }
        else
        {
            this._typeIdMap.set(typeId, ++this._typeIdIndex);
            return -1;
        }
    }
}

class EncapsEncoder10 extends EncapsEncoder
{
    constructor(stream, encaps)
    {
        super(stream, encaps);
        this._sliceType = SliceType.NoSlice;
        this._writeSlice = 0; // Position of the slice data members
        this._valueIdIndex = 0;
        this._toBeMarshaledMap = new Map(); // Map<Ice.Value, Integer>();
    }

    writeValue(v)
    {
        Debug.assert(v !== undefined);
        //
        // Object references are encoded as a negative integer in 1.0.
        //
        if(v !== null && v !== undefined)
        {
            this._stream.writeInt(-this.registerValue(v));
        }
        else
        {
            this._stream.writeInt(0);
        }
    }

    writeException(v)
    {
        Debug.assert(v !== null && v !== undefined);
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
        if(usesClasses)
        {
            this.writePendingValues();
        }
    }

    startInstance(sliceType)
    {
        this._sliceType = sliceType;
    }

    endInstance()
    {
        if(this._sliceType === SliceType.ValueSlice)
        {
            //
            // Write the Object slice.
            //
            this.startSlice(Ice.Value.ice_staticId(), -1, true);
            this._stream.writeSize(0); // For compatibility with the old AFM.
            this.endSlice();
        }
        this._sliceType = SliceType.NoSlice;
    }

    startSlice(typeId)
    {
        //
        // For instance slices, encode a boolean to indicate how the type ID
        // is encoded and the type ID either as a string or index. For
        // exception slices, always encode the type ID as a string.
        //
        if(this._sliceType === SliceType.ValueSlice)
        {
            const index = this.registerTypeId(typeId);
            if(index < 0)
            {
                this._stream.writeBool(false);
                this._stream.writeString(typeId);
            }
            else
            {
                this._stream.writeBool(true);
                this._stream.writeSize(index);
            }
        }
        else
        {
            this._stream.writeString(typeId);
        }

        this._stream.writeInt(0); // Placeholder for the slice length.

        this._writeSlice = this._stream.pos;
    }

    endSlice()
    {
        //
        // Write the slice length.
        //
        const sz = this._stream.pos - this._writeSlice + 4;
        this._stream.rewriteInt(sz, this._writeSlice - 4);
    }

    writePendingValues()
    {
        const writeCB = (value, key) =>
            {
                //
                // Ask the instance to marshal itself. Any new class
                // instances that are triggered by the classes marshaled
                // are added to toBeMarshaledMap.
                //
                this._stream.writeInt(value);
                try
                {
                    key.ice_preMarshal();
                }
                catch(ex)
                {
                    this._stream.instance.initializationData().logger.warning(
                        "exception raised by ice_preMarshal:\n" + ex.toString());
                }
                key._iceWrite(this._stream);
            };

        while(this._toBeMarshaledMap.size > 0)
        {
            //
            // Consider the to be marshalled instances as marshalled now,
            // this is necessary to avoid adding again the "to be
            // marshalled instances" into _toBeMarshaledMap while writing
            // instances.
            //
            this._toBeMarshaledMap.forEach((value, key) => this._marshaledMap.set(key, value));

            const savedMap = this._toBeMarshaledMap;
            this._toBeMarshaledMap = new Map(); // Map<Ice.Value, int>();
            this._stream.writeSize(savedMap.size);
            savedMap.forEach(writeCB);
        }
        this._stream.writeSize(0); // Zero marker indicates end of sequence of sequences of instances.
    }

    registerValue(v)
    {
        Debug.assert(v !== null);

        //
        // Look for this instance in the to-be-marshaled map.
        //
        let p = this._toBeMarshaledMap.get(v);
        if(p !== undefined)
        {
            return p;
        }

        //
        // Didn't find it, try the marshaled map next.
        //
        p = this._marshaledMap.get(v);
        if(p !== undefined)
        {
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

class EncapsEncoder11 extends EncapsEncoder
{
    constructor(stream, encaps)
    {
        super(stream, encaps);
        this._current = null;
        this._valueIdIndex = 1;
    }

    writeValue(v)
    {
        Debug.assert(v !== undefined);
        if(v === null || v === undefined)
        {
            this._stream.writeSize(0);
        }
        else if(this._current !== null && this._encaps.format === FormatType.SlicedFormat)
        {
            if(this._current.indirectionTable === null) // Lazy initialization
            {
                this._current.indirectionTable = []; // Ice.Value[]
                this._current.indirectionMap = new Map(); // Map<Ice.Value, int>
            }

            //
            // If writing an instance within a slice and using the sliced
            // format, write an index from the instance indirection
            // table. The indirect instance table is encoded at the end of
            // each slice and is always read (even if the Slice is
            // unknown).
            //
            const index = this._current.indirectionMap.get(v);
            if(index === undefined)
            {
                this._current.indirectionTable.push(v);
                const idx = this._current.indirectionTable.length; // Position + 1 (0 is reserved for nil)
                this._current.indirectionMap.set(v, idx);
                this._stream.writeSize(idx);
            }
            else
            {
                this._stream.writeSize(index);
            }
        }
        else
        {
            this.writeInstance(v); // Write the instance or a reference if already marshaled.
        }
    }

    writePendingValues()
    {
        return undefined;
    }

    writeException(v)
    {
        Debug.assert(v !== null && v !== undefined);
        v._write(this._stream);
    }

    startInstance(sliceType, data)
    {
        if(this._current === null)
        {
            this._current = new EncapsEncoder11.InstanceData(null);
        }
        else
        {
            this._current =
                (this._current.next === null) ? new EncapsEncoder11.InstanceData(this._current) : this._current.next;
        }
        this._current.sliceType = sliceType;
        this._current.firstSlice = true;

        if(data !== null && data !== undefined)
        {
            this.writeSlicedData(data);
        }
    }

    endInstance()
    {
        this._current = this._current.previous;
    }

    startSlice(typeId, compactId, last)
    {
        Debug.assert((this._current.indirectionTable === null || this._current.indirectionTable.length === 0) &&
                     (this._current.indirectionMap === null || this._current.indirectionMap.size === 0));

        this._current.sliceFlagsPos = this._stream.pos;

        this._current.sliceFlags = 0;
        if(this._encaps.format === FormatType.SlicedFormat)
        {
            // Encode the slice size if using the sliced format.
            this._current.sliceFlags |= Protocol.FLAG_HAS_SLICE_SIZE;
        }
        if(last)
        {
            this._current.sliceFlags |= Protocol.FLAG_IS_LAST_SLICE; // This is the last slice.
        }

        this._stream.writeByte(0); // Placeholder for the slice flags

        //
        // For instance slices, encode the flag and the type ID either as a
        // string or index. For exception slices, always encode the type
        // ID a string.
        //
        if(this._current.sliceType === SliceType.ValueSlice)
        {
            //
            // Encode the type ID (only in the first slice for the compact
            // encoding).
            //
            if(this._encaps.format === FormatType.SlicedFormat || this._current.firstSlice)
            {
                if(compactId >= 0)
                {
                    this._current.sliceFlags |= Protocol.FLAG_HAS_TYPE_ID_COMPACT;
                    this._stream.writeSize(compactId);
                }
                else
                {
                    const index = this.registerTypeId(typeId);
                    if(index < 0)
                    {
                        this._current.sliceFlags |= Protocol.FLAG_HAS_TYPE_ID_STRING;
                        this._stream.writeString(typeId);
                    }
                    else
                    {
                        this._current.sliceFlags |= Protocol.FLAG_HAS_TYPE_ID_INDEX;
                        this._stream.writeSize(index);
                    }
                }
            }
        }
        else
        {
            this._stream.writeString(typeId);
        }

        if((this._current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) !== 0)
        {
            this._stream.writeInt(0); // Placeholder for the slice length.
        }

        this._current.writeSlice = this._stream.pos;
        this._current.firstSlice = false;
    }

    endSlice()
    {
        //
        // Write the optional member end marker if some optional members
        // were encoded. Note that the optional members are encoded before
        // the indirection table and are included in the slice size.
        //
        if((this._current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) !== 0)
        {
            this._stream.writeByte(Protocol.OPTIONAL_END_MARKER);
        }

        //
        // Write the slice length if necessary.
        //
        if((this._current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) !== 0)
        {
            const sz = this._stream.pos - this._current.writeSlice + 4;
            this._stream.rewriteInt(sz, this._current.writeSlice - 4);
        }

        //
        // Only write the indirection table if it contains entries.
        //
        if(this._current.indirectionTable !== null && this._current.indirectionTable.length !== 0)
        {
            Debug.assert(this._encaps.format === FormatType.SlicedFormat);
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

    writeOptional(tag, format)
    {
        if(this._current === null)
        {
            return this._stream.writeOptImpl(tag, format);
        }

        if(this._stream.writeOptImpl(tag, format))
        {
            this._current.sliceFlags |= Protocol.FLAG_HAS_OPTIONAL_MEMBERS;
            return true;
        }

        return false;
    }

    writeSlicedData(slicedData)
    {
        Debug.assert(slicedData !== null && slicedData !== undefined);

        //
        // We only remarshal preserved slices if we are using the sliced
        // format. Otherwise, we ignore the preserved slices, which
        // essentially "slices" the instance into the most-derived type
        // known by the sender.
        //
        if(this._encaps.format !== FormatType.SlicedFormat)
        {
            return;
        }

        slicedData.slices.forEach(info =>
            {
                this.startSlice(info.typeId, info.compactId, info.isLastSlice);

                //
                // Write the bytes associated with this slice.
                //
                this._stream.writeBlob(info.bytes);

                if(info.hasOptionalMembers)
                {
                    this._current.sliceFlags |= Protocol.FLAG_HAS_OPTIONAL_MEMBERS;
                }

                //
                // Make sure to also re-write the instance indirection table.
                //
                if(info.instances !== null && info.instances.length > 0)
                {
                    if(this._current.indirectionTable === null) // Lazy initialization
                    {
                        this._current.indirectionTable = []; // Ice.Value[]
                        this._current.indirectionMap = new Map(); // Map<Ice.Value, int>
                    }

                    info.instances.forEach(instance => this._current.indirectionTable.push(instance));
                }

                this.endSlice();
            });
    }

    writeInstance(v)
    {
        Debug.assert(v !== null && v !== undefined);

        //
        // If the instance was already marshaled, just write it's ID.
        //
        const p = this._marshaledMap.get(v);
        if(p !== undefined)
        {
            this._stream.writeSize(p);
            return;
        }

        //
        // We haven't seen this instance previously, create a new ID,
        // insert it into the marshaled map, and write the instance.
        //
        this._marshaledMap.set(v, ++this._valueIdIndex);

        try
        {
            v.ice_preMarshal();
        }
        catch(ex)
        {
            this._stream.instance.initializationData().logger.warning("exception raised by ice_preMarshal:\n" +
                                                                      ex.toString());
        }

        this._stream.writeSize(1); // Object instance marker.
        v._iceWrite(this._stream);
    }
}

EncapsEncoder11.InstanceData = class
{
    constructor(previous)
    {
        Debug.assert(previous !== undefined);
        if(previous !== null)
        {
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
        this.indirectionTable = null; // Ice.Value[]
        this.indirectionMap = null; // Map<Ice.Value, int>
    }
};

class WriteEncaps
{
    constructor()
    {
        this.start = 0;
        this.format = FormatType.DefaultFormat;
        this.encoding = null;
        this.encoding_1_0 = false;
        this.encoder = null;
        this.next = null;
    }

    reset()
    {
        this.encoder = null;
    }

    setEncoding(encoding)
    {
        this.encoding = encoding;
        this.encoding_1_0 = encoding.equals(Ice.Encoding_1_0);
    }
}

class OutputStream
{
    constructor(arg1, arg2)
    {
        this._instance = null;
        this._encoding = null;

        if(arg1 !== undefined && arg1 !== null)
        {
            if(arg1.constructor == Ice.Communicator)
            {
                this._instance = arg1.instance;
            }
            else if(arg1.constructor == Ice.Instance)
            {
                this._instance = arg1;
            }
            else if(arg1.constructor == Ice.EncodingVersion)
            {
                this._encoding = arg1;
            }
            else
            {
                throw new Ice.InitializationException("unknown argument to OutputStream constructor");
            }
        }

        if(arg2 !== undefined && arg2 !== null)
        {
            if(arg2.constructor == Ice.EncodingVersion)
            {
                this._encoding = arg2;
            }
            else
            {
                throw new Ice.InitializationException("unknown argument to OutputStream constructor");
            }
        }

        this._buf = new Ice.Buffer();

        this._closure = null;

        this._encapsStack = null;
        this._encapsCache = null;

        if(this._instance !== null)
        {
            if(this._encoding === null)
            {
                this._encoding = this._instance.defaultsAndOverrides().defaultEncoding;
            }
            this._format = this._instance.defaultsAndOverrides().defaultFormat;
        }
        else
        {
            if(this._encoding === null)
            {
                this._encoding = Protocol.currentEncoding;
            }
            this._format = FormatType.CompactFormat;
        }
    }

    //
    // This function allows this object to be reused, rather than reallocated.
    //
    reset()
    {
        this._buf.reset();
        this.clear();
    }

    clear()
    {
        if(this._encapsStack !== null)
        {
            Debug.assert(this._encapsStack.next);
            this._encapsStack.next = this._encapsCache;
            this._encapsCache = this._encapsStack;
            this._encapsCache.reset();
            this._encapsStack = null;
        }
    }

    finished()
    {
        return this.prepareWrite().getArray(this.size);
    }

    swap(other)
    {
        Debug.assert(this._instance === other._instance);

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

    resetEncapsulation()
    {
        this._encapsStack = null;
    }

    resize(sz)
    {
        this._buf.resize(sz);
        this._buf.position = sz;
    }

    prepareWrite()
    {
        this._buf.position = 0;
        return this._buf;
    }

    startValue(data)
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.startInstance(SliceType.ValueSlice, data);
    }

    endValue()
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.endInstance();
    }

    startException(data)
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.startInstance(SliceType.ExceptionSlice, data);
    }

    endException()
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.endInstance();
    }

    startEncapsulation(encoding, format)
    {
        //
        // If no encoding version is specified, use the current write
        // encapsulation encoding version if there's a current write
        // encapsulation, otherwise, use the stream encoding version.
        //

        if(encoding === undefined)
        {
            if(this._encapsStack !== null)
            {
                encoding = this._encapsStack.encoding;
                format = this._encapsStack.format;
            }
            else
            {
                encoding = this._encoding;
                format = FormatType.DefaultFormat;
            }
        }

        Protocol.checkSupportedEncoding(encoding);

        let curr = this._encapsCache;
        if(curr !== null)
        {
            curr.reset();
            this._encapsCache = this._encapsCache.next;
        }
        else
        {
            curr = new WriteEncaps();
        }
        curr.next = this._encapsStack;
        this._encapsStack = curr;

        this._encapsStack.format = format;
        this._encapsStack.setEncoding(encoding);
        this._encapsStack.start = this._buf.limit;

        this.writeInt(0); // Placeholder for the encapsulation length.
        this._encapsStack.encoding._write(this);
    }

    endEncapsulation()
    {
        Debug.assert(this._encapsStack);

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

    writeEmptyEncapsulation(encoding)
    {
        Protocol.checkSupportedEncoding(encoding);
        this.writeInt(6); // Size
        encoding._write(this);
    }

    writeEncapsulation(v)
    {
        if(v.length < 6)
        {
            throw new Ice.EncapsulationException();
        }
        this.expand(v.length);
        this._buf.putArray(v);
    }

    getEncoding()
    {
        return this._encapsStack !== null ? this._encapsStack.encoding : this._encoding;
    }

    startSlice(typeId, compactId, last)
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.startSlice(typeId, compactId, last);
    }

    endSlice()
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.endSlice();
    }

    writePendingValues()
    {
        if(this._encapsStack !== null && this._encapsStack.encoder !== null)
        {
            this._encapsStack.encoder.writePendingValues();
        }
        else if((this._encapsStack !== null && this._encapsStack.encoding_1_0) ||
                (this._encapsStack === null && this._encoding.equals(Ice.Encoding_1_0)))
        {
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

    writeSize(v)
    {
        if(v > 254)
        {
            this.expand(5);
            this._buf.put(255);
            this._buf.putInt(v);
        }
        else
        {
            this.expand(1);
            this._buf.put(v);
        }
    }

    startSize()
    {
        const pos = this._buf.position;
        this.writeInt(0); // Placeholder for 32-bit size
        return pos;
    }

    endSize(pos)
    {
        Debug.assert(pos >= 0);
        this.rewriteInt(this._buf.position - pos - 4, pos);
    }

    writeBlob(v)
    {
        if(v === null || v === undefined)
        {
            return;
        }
        this.expand(v.length);
        this._buf.putArray(v);
    }

    // Read/write format and tag for optionals
    writeOptional(tag, format)
    {
        Debug.assert(this._encapsStack !== null);
        if(this._encapsStack.encoder !== null)
        {
            return this._encapsStack.encoder.writeOptional(tag, format);
        }
        return this.writeOptImpl(tag, format);
    }

    writeOptionalHelper(tag, format, write, v)
    {
        if(v !== undefined)
        {
            if(this.writeOptional(tag, format))
            {
                write.call(this, v);
            }
        }
    }

    writeByte(v)
    {
        this.expand(1);
        this._buf.put(v);
    }

    rewriteByte(v, dest)
    {
        this._buf.putAt(dest, v);
    }

    writeByteSeq(v)
    {
        if(v === null || v === undefined || v.length === 0)
        {
            this.writeSize(0);
        }
        else
        {
            this.writeSize(v.length);
            this.expand(v.length);
            this._buf.putArray(v);
        }
    }

    writeBool(v)
    {
        this.expand(1);
        this._buf.put(v ? 1 : 0);
    }

    rewriteBool(v, dest)
    {
        this._buf.putAt(dest, v ? 1 : 0);
    }

    writeShort(v)
    {
        this.expand(2);
        this._buf.putShort(v);
    }

    writeInt(v)
    {
        this.expand(4);
        this._buf.putInt(v);
    }

    rewriteInt(v, dest)
    {
        this._buf.putIntAt(dest, v);
    }

    writeLong(v)
    {
        this.expand(8);
        this._buf.putLong(v);
    }

    writeFloat(v)
    {
        this.expand(4);
        this._buf.putFloat(v);
    }

    writeDouble(v)
    {
        this.expand(8);
        this._buf.putDouble(v);
    }

    writeString(v)
    {
        if(v === null || v === undefined || v.length === 0)
        {
            this.writeSize(0);
        }
        else
        {
            this._buf.writeString(this, v);
        }
    }

    writeProxy(v)
    {
        if(v === null || v === undefined)
        {
            const ident = new Ice.Identity();
            ident._write(this);
        }
        else
        {
            v._write(this);
        }
    }

    writeOptionalProxy(tag, v)
    {
        if(v !== undefined)
        {
            if(this.writeOptional(tag, OptionalFormat.FSize))
            {
                const pos = this.startSize();
                this.writeProxy(v);
                this.endSize(pos);
            }
        }
    }

    writeEnum(v)
    {
        if(this.isEncoding_1_0())
        {
            if(v.maxValue < 127)
            {
                this.writeByte(v.value);
            }
            else if(v.maxValue < 32767)
            {
                this.writeShort(v.value);
            }
            else
            {
                this.writeInt(v.value);
            }
        }
        else
        {
            this.writeSize(v.value);
        }
    }

    writeValue(v)
    {
        this.initEncaps();
        this._encapsStack.encoder.writeValue(v);
    }

    writeOptionalValue(tag, v)
    {
        if(v !== undefined)
        {
            if(this.writeOptional(tag, OptionalFormat.Class))
            {
                this.writeValue(v);
            }
        }
    }

    writeException(e)
    {
        this.initEncaps();
        this._encapsStack.encoder.writeException(e);
    }

    //
    // Keep for compatibility with 3.7.0 remove with next major version
    //
    writeUserException(e)
    {
        this.WriteException(e);
    }

    writeOptImpl(tag, format)
    {
        if(this.isEncoding_1_0())
        {
            return false; // Optional members aren't supported with the 1.0 encoding.
        }

        let v = format.value;
        if(tag < 30)
        {
            v |= tag << 3;
            this.writeByte(v);
        }
        else
        {
            v |= 0x0F0; // tag = 30
            this.writeByte(v);
            this.writeSize(tag);
        }
        return true;
    }

    isEmpty()
    {
        return this._buf.empty();
    }

    expand(n)
    {
        this._buf.expand(n);
    }

    isEncoding_1_0()
    {
        return this._encapsStack ? this._encapsStack.encoding_1_0 : this._encoding.equals(Ice.Encoding_1_0);
    }

    initEncaps()
    {
        if(!this._encapsStack) // Lazy initialization
        {
            this._encapsStack = this._encapsCache;
            if(this._encapsStack)
            {
                this._encapsCache = this._encapsCache.next;
            }
            else
            {
                this._encapsStack = new WriteEncaps();
            }
            this._encapsStack.setEncoding(this._encoding);
        }

        if(this._encapsStack.format === FormatType.DefaultFormat)
        {
            this._encapsStack.format = this._instance.defaultsAndOverrides().defaultFormat;
        }

        if(!this._encapsStack.encoder) // Lazy initialization.
        {
            if(this._encapsStack.encoding_1_0)
            {
                this._encapsStack.encoder = new EncapsEncoder10(this, this._encapsStack);
            }
            else
            {
                this._encapsStack.encoder = new EncapsEncoder11(this, this._encapsStack);
            }
        }
    }

    //
    // Sets the encoding format for class and exception instances.
    //
    get format()
    {
        return this._format;
    }

    set format(value)
    {
        this._format = value;
    }

    get pos()
    {
        return this._buf.position;
    }

    set pos(value)
    {
        this._buf.position = value;
    }

    get size()
    {
        return this._buf.limit;
    }

    get instance()
    {
        return this._instance;
    }

    get closure()
    {
        return this._closure;
    }

    set closure(value)
    {
        this._closure = value;
    }

    get buffer()
    {
        return this._buf;
    }
}

const defineBuiltinHelper = function(write, read, sz, format, min, max)
{
    const helper = class
    {
        static write(os, v)
        {
            return write.call(os, v);
        }

        static read(is)
        {
            return read.call(is);
        }

        static writeOptional(os, tag, v)
        {
            os.writeOptionalHelper(tag, format, write, v);
        }

        static readOptional(is, tag)
        {
            return is.readOptionalHelper(tag, format, read);
        }

        static get minWireSize()
        {
            return sz;
        }
    };

    if(min !== undefined && max !== undefined)
    {
        helper.validate = function(v)
        {
            return v >= min && v <= max;
        };
    }

    return helper;
};

const istr = InputStream.prototype;
const ostr = OutputStream.prototype;

//
// Constants to use in number type range checks.
//
const MIN_UINT8_VALUE = 0x0;
const MAX_UINT8_VALUE = 0xFF;

const MIN_INT16_VALUE = -0x8000;
const MAX_INT16_VALUE = 0x7FFF;

const MIN_UINT32_VALUE = 0x0;
const MAX_UINT32_VALUE = 0xFFFFFFFF;

const MIN_INT32_VALUE = -0x80000000;
const MAX_INT32_VALUE = 0x7FFFFFFF;

const MIN_FLOAT32_VALUE = -3.4028234664e+38;
const MAX_FLOAT32_VALUE = 3.4028234664e+38;

Ice.ByteHelper = defineBuiltinHelper(ostr.writeByte, istr.readByte, 1, Ice.OptionalFormat.F1,
                                     MIN_UINT8_VALUE, MAX_UINT8_VALUE);

Ice.ShortHelper = defineBuiltinHelper(ostr.writeShort, istr.readShort, 2, Ice.OptionalFormat.F2,
                                      MIN_INT16_VALUE, MAX_INT16_VALUE);

Ice.IntHelper = defineBuiltinHelper(ostr.writeInt, istr.readInt, 4, Ice.OptionalFormat.F4,
                                    MIN_INT32_VALUE, MAX_INT32_VALUE);

Ice.FloatHelper = defineBuiltinHelper(ostr.writeFloat, istr.readFloat, 4, Ice.OptionalFormat.F4,
                                      MIN_FLOAT32_VALUE, MAX_FLOAT32_VALUE);
Ice.FloatHelper.validate = function(v)
{
    return Number.isNaN(v) || v == Number.POSITIVE_INFINITY || v == Number.NEGATIVE_INFINITY ||
        (v >= MIN_FLOAT32_VALUE && v <= MAX_FLOAT32_VALUE);
};

Ice.DoubleHelper = defineBuiltinHelper(ostr.writeDouble, istr.readDouble, 8, Ice.OptionalFormat.F8,
                                       -Number.MAX_VALUE, Number.MAX_VALUE);
Ice.DoubleHelper.validate = function(v)
{
    return Number.isNaN(v) || v == Number.POSITIVE_INFINITY || v == Number.NEGATIVE_INFINITY ||
        (v >= -Number.MAX_VALUE && v <= Number.MAX_VALUE);
};

Ice.BoolHelper = defineBuiltinHelper(ostr.writeBool, istr.readBool, 1, Ice.OptionalFormat.F1);
Ice.LongHelper = defineBuiltinHelper(ostr.writeLong, istr.readLong, 8, Ice.OptionalFormat.F8);
Ice.LongHelper.validate = function(v)
{
    //
    // For a long to be valid both words must be within the range of UINT32
    //
    return v.low >= MIN_UINT32_VALUE && v.low <= MAX_UINT32_VALUE &&
           v.high >= MIN_UINT32_VALUE && v.high <= MAX_UINT32_VALUE;
};

Ice.StringHelper = defineBuiltinHelper(ostr.writeString, istr.readString, 1, Ice.OptionalFormat.VSize);

Ice.ObjectHelper = class
{
    static write(os, v)
    {
        os.writeValue(v);
    }

    static read(is)
    {
        let o;
        is.readValue(v =>
                     {
                         o = v;
                     }, Ice.Value);
        return o;
    }

    static writeOptional(os, tag, v)
    {
        os.writeOptionalValue(tag, Ice.OptionalFormat.Class, ostr.writeValue, v);
    }

    static readOptional(is, tag)
    {
        let o;
        is.readOptionalValue(tag, v =>
                             {
                                 o = v;
                             }, Ice.Value);
        return o;
    }

    static get minWireSize()
    {
        return 1;
    }
};

Ice.InputStream = InputStream;
Ice.OutputStream = OutputStream;
module.exports.Ice = Ice;
