// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
var __M = Ice.__M;
__M.require(module,
    [
        "../Ice/Class",
        "../Ice/Debug",
        "../Ice/ExUtil",
        "../Ice/FormatType",
        "../Ice/HashMap",
        "../Ice/Object",
        "../Ice/OptionalFormat",
        "../Ice/Protocol",
        "../Ice/TraceUtil",
        "../Ice/Buffer",
        "../Ice/Exception",
        "../Ice/LocalException",
        "../Ice/Version",
        "../Ice/CompactIdRegistry",
        "../Ice/ArrayUtil",
        "../Ice/UnknownSlicedValue"
    ]);

var Debug = Ice.Debug;
var ExUtil = Ice.ExUtil;
var FormatType = Ice.FormatType;
var HashMap = Ice.HashMap;
var IceObject = Ice.Object;
var OptionalFormat = Ice.OptionalFormat;
var Protocol = Ice.Protocol;
var TraceUtil = Ice.TraceUtil;
var ArrayUtil = Ice.ArrayUtil;
var SlicedData = Ice.SlicedData;
var Class = Ice.Class;

var SliceType = {};
SliceType.NoSlice = 0;
SliceType.ValueSlice = 1;
SliceType.ExceptionSlice = 2;

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

var IndirectPatchEntry = function(index, cb)
{
    this.index = index;
    this.cb = cb;
};

var EncapsDecoder = Class({
    __init__: function(stream, encaps, sliceValues, f)
    {
        this._stream = stream;
        this._encaps = encaps;
        this._sliceValues = sliceValues;
        this._valueFactoryManager = f;
        this._patchMap = null; // Lazy initialized, HashMap<int, Patcher[] >()
        this._unmarshaledMap = new HashMap(); // HashMap<int, Ice.Object>()
        this._typeIdMap = null; // Lazy initialized, HashMap<int, String>
        this._typeIdIndex = 0;
        this._valueList = null; // Lazy initialized. Ice.Object[]
    },
    readOptional: function()
    {
        return false;
    },
    readPendingValues: function()
    {
    },
    readTypeId: function(isIndex)
    {
        var typeId, index;
        if(this._typeIdMap === null) // Lazy initialization
        {
            this._typeIdMap = new HashMap(); // Map<int, String>();
        }

        if(isIndex)
        {
            index = this._stream.readSize();
            typeId = this._typeIdMap.get(index);
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
    },
    newInstance: function(typeId)
    {
        //
        // Try to find a factory registered for the specific type.
        //
        var userFactory = this._valueFactoryManager.find(typeId);
        var v = null;

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
    },
    addPatchEntry: function(index, cb)
    {
        Debug.assert(index > 0);

        //
        // Check if we have already unmarshaled the instance. If that's the case,
        // just call the callback and we're done.
        //
        var obj = this._unmarshaledMap.get(index);
        if(obj !== undefined && obj !== null)
        {
            cb.call(null, obj);
            return;
        }

        if(this._patchMap === null) // Lazy initialization
        {
            this._patchMap = new HashMap(); // HashMap<Integer, Patcher[] >();
        }

        //
        // Add a patch entry if the instance isn't unmarshaled yet,
        // the callback will be called when the instance is
        // unmarshaled.
        //
        var l = this._patchMap.get(index);
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
    },
    unmarshal: function(index, v)
    {
        var i, length, l;

        //
        // Add the instance to the map of unmarshaled instances, this must
        // be done before reading the instances (for circular references).
        //
        this._unmarshaledMap.set(index, v);

        //
        // Read the instance.
        //
        v.__read(this._stream);

        if(this._patchMap !== null)
        {
            //
            // Patch all instances now that the instance is unmarshaled.
            //
            l = this._patchMap.get(index);
            if(l !== undefined)
            {
                Debug.assert(l.length > 0);

                //
                // Patch all pointers that refer to the instance.
                //
                for(i = 0, length = l.length; i < length; ++i)
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
                this._valueList = []; // Ice.Object[]
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
                for(i = 0, length = this._valueList.length; i < length; i++)
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
});

var EncapsDecoder10 = Class(EncapsDecoder, {
    __init__: function(stream, encaps, sliceValues, f)
    {
        EncapsDecoder.call(this, stream, encaps, sliceValues, f);
        this._sliceType = SliceType.NoSlice;
    },
    readValue: function(cb)
    {
        Debug.assert(cb !== null);

        //
        // Instance references are encoded as a negative integer in 1.0.
        //
        var index = this._stream.readInt();
        if(index > 0)
        {
            throw new Ice.MarshalException("invalid object id");
        }
        index = -index;

        if(index === 0)
        {
            cb.call(null, null);
        }
        else
        {
            this.addPatchEntry(index, cb);
        }
    },
    throwException: function()
    {
        Debug.assert(this._sliceType === SliceType.NoSlice);

        //
        // User exceptions with the 1.0 encoding start with a boolean flag
        // that indicates whether or not the exception has classes.
        //
        // This allows reading the pending instances even if some part of
        // the exception was sliced.
        //
        var usesClasses = this._stream.readBool();

        this._sliceType = SliceType.ExceptionSlice;
        this._skipFirstSlice = false;

        //
        // Read the first slice header.
        //
        this.startSlice();
        var mostDerivedId = this._typeId;
        while(true)
        {
            var userEx = this._stream.createUserException(this._typeId);

            //
            // We found the exception.
            //
            if(userEx !== null)
            {
                userEx.__read(this._stream);
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
    },
    startInstance: function(sliceType)
    {
        Debug.assert(this._sliceType === sliceType);
        this._skipFirstSlice = true;
    },
    endInstance: function(/*preserve*/)
    {
        var sz;
        //
        // Read the Ice::Object slice.
        //
        if(this._sliceType === SliceType.ValueSlice)
        {
            this.startSlice();
            sz = this._stream.readSize(); // For compatibility with the old AFM.
            if(sz !== 0)
            {
                throw new Ice.MarshalException("invalid Object slice");
            }
            this.endSlice();
        }

        this._sliceType = SliceType.NoSlice;
        return null;
    },
    startSlice: function()
    {
        var isIndex;
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
            isIndex = this._stream.readBool();
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
    },
    endSlice: function()
    {
    },
    skipSlice: function()
    {
        this._stream.traceSkipSlice(this._typeId, this._sliceType);
        Debug.assert(this._sliceSize >= 4);
        this._stream.skip(this._sliceSize - 4);
    },
    readPendingValues: function()
    {
        var k, num;
        do
        {
            num = this._stream.readSize();
            for(k = num; k > 0; --k)
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
    },
    readInstance: function()
    {
        var index = this._stream.readInt(),
            mostDerivedId,
            v = null;

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
        mostDerivedId = this._typeId;
        while(true)
        {
            //
            // For the 1.0 encoding, the type ID for the base Object class
            // marks the last slice.
            //
            if(this._typeId == IceObject.ice_staticId())
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
});

var EncapsDecoder11 = Class(EncapsDecoder, {
    __init__: function(stream, encaps, sliceValues, f, r)
    {
        EncapsDecoder.call(this, stream, encaps, sliceValues, f);
        this._compactIdResolver = r;
        this._current = null;
        this._valueIdIndex = 1;
    },
    readValue: function(cb)
    {
        var index = this._stream.readSize();
        if(index < 0)
        {
            throw new Ice.MarshalException("invalid object id");
        }
        else if(index === 0)
        {
            if(cb !== null)
            {
                cb.call(null, null);
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
                var e = new IndirectPatchEntry();
                e.index = index - 1;
                e.cb = cb;
                this._current.indirectPatchList.push(e);
            }
        }
        else
        {
            this.readInstance(index, cb);
        }
    },
    throwException: function()
    {
        Debug.assert(this._current === null);

        this.push(SliceType.ExceptionSlice);

        //
        // Read the first slice header.
        //
        this.startSlice();
        var mostDerivedId = this._current.typeId;
        while(true)
        {

            var userEx = this._stream.createUserException(this._current.typeId);

            //
            // We found the exception.
            //
            if(userEx !== null)
            {
                userEx.__read(this._stream);
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
    },
    startInstance: function(sliceType)
    {
        Debug.assert(sliceType !== undefined);
        Debug.assert(this._current.sliceType !== null && this._current.sliceType === sliceType);
        this._current.skipFirstSlice = true;
    },
    endInstance: function(preserve)
    {
        var slicedData = null;
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
    },
    startSlice: function()
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
    },
    endSlice: function()
    {
        var e,
            i,
            indirectionTable = [],
            length;

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
            //
            // The table is written as a sequence<size> to conserve space.
            //
            length = this._stream.readAndCheckSeqSize(1);
            for(i = 0; i < length; ++i)
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
                for(i = 0, length = this._current.indirectPatchList.length; i < length; ++i)
                {
                    e = this._current.indirectPatchList[i];
                    Debug.assert(e.index >= 0);
                    if(e.index >= indirectionTable.length)
                    {
                        throw new Ice.MarshalException("indirection out of range");
                    }
                    this.addPatchEntry(indirectionTable[e.index], e.cb);
                }
                this._current.indirectPatchList.length = 0;
            }
        }
    },
    skipSlice: function()
    {
        this._stream.traceSkipSlice(this._current.typeId, this._current.sliceType);

        var start = this._stream.pos;

        if((this._current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) !== 0)
        {
            Debug.assert(this._current.sliceSize >= 4);
            this._stream.skip(this._current.sliceSize - 4);
        }
        else
        {
            if(this._current.sliceType === SliceType.ValueSlice)
            {
                throw new Ice.NoValueFactoryException("no value factory found and compact format prevents slicing " +
                                                       "(the sender should use the sliced format instead)",
                                                       this._current.typeId);
            }
            else
            {
                if(this._current.typeId.indexOf("::") === 0)
                {
                    throw new Ice.UnknownUserException(this._current.typeId.substring(2));
                }
                else
                {
                    throw new Ice.UnknownUserException(this._current.typeId);
                }
            }
        }

        //
        // Preserve this slice.
        //
        var info = new Ice.SliceInfo();
        info.typeId = this._current.typeId;
        info.compactId = this._current.compactId;
        info.hasOptionalMembers = (this._current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) !== 0;
        info.isLastSlice = (this._current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE) !== 0;

        var b = this._stream._buf;
        var end = b.position;
        var dataEnd = end;
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
            var length = this._stream.readAndCheckSeqSize(1);
            var indirectionTable = [];
            for(var i = 0; i < length; ++i)
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
    },
    readOptional: function(readTag, expectedFormat)
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
    },
    readInstance: function(index, cb)
    {
        Debug.assert(index > 0);

        var mostDerivedId,
            v = null;

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
        mostDerivedId = this._current.typeId;
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
            cb.call(null, v);
        }

        return index;
    },
    readSlicedData: function()
    {
        var i, ii, table, info, j, jj;

        if(this._current.slices === null) // No preserved slices.
        {
            return null;
        }

        //
        // The _indirectionTables member holds the indirection table for each slice
        // in _slices.
        //
        Debug.assert(this._current.slices.length === this._current.indirectionTables.length);
        for(i = 0, ii = this._current.slices.length; i < ii; ++i)
        {
            //
            // We use the "instances" list in SliceInfo to hold references
            // to the target instances. Note that the instances might not have
            // been read yet in the case of a circular reference to an
            // enclosing instance.
            //
            table = this._current.indirectionTables[i];
            info = this._current.slices[i];
            info.instances = [];
            jj = table ? table.length : 0;
            for(j = 0; j < jj; ++j)
            {
                this.addPatchEntry(table[j], sequencePatcher(info.instances, j, IceObject));
            }
        }
        return new SlicedData(ArrayUtil.clone(this._current.slices));
    },
    push: function(sliceType)
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
});

EncapsDecoder11.InstanceData = function(previous)
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
    this.slices = null;     // Preserved slices. Ice.SliceInfo[]
    this.indirectionTables = null; // int[][]

    // Slice attributes
    this.sliceFlags = 0;
    this.sliceSize = 0;
    this.typeId = null;
    this.compactId = 0;
    this.indirectPatchList = null; // Lazy initialized, IndirectPatchEntry[]
};

var sequencePatcher = function(seq, index, T)
{
    return function(v)
        {
            if(v !== null && !(v instanceof T))
            {
                ExUtil.throwUOE(T.ice_staticId(), v);
            }
            seq[index] = v;
        };
};

var ReadEncaps = Class({
    __init__: function()
    {
        this.start = 0;
        this.sz = 0;
        this.encoding = null;
        this.encoding_1_0 = false;
        this.decoder = null;
        this.next = null;
    },
    reset: function()
    {
        this.decoder = null;
    },
    setEncoding: function(encoding)
    {
        this.encoding = encoding;
        this.encoding_1_0 = encoding.equals(Ice.Encoding_1_0);
    }
});

var InputStream = Class({
    __init__: function(arg1, arg2, arg3)
    {
        var args =
        {
            instance: null,
            encoding: null,
            bytes: null,
            buffer: null
        };
        this._checkArgs([arg1, arg2, arg3], args);
        this._initialize(args);
    },
    _checkArgs: function(arr, args)
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
        for(i = 0; i < arr.length; ++i)
        {
            var arg = arr[i];
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
                else if(arg.constructor === Array)
                {
                    args.bytes = arg;
                }
                else
                {
                    throw new Ice.InitializationException("unknown argument to InputStream constructor");
                }
            }
        }
        if(args.buffer !== null && args.bytes !== null)
        {
            throw new Ice.InitializationException("invalid argument to InputStream constructor");
        }
    },
    _initialize: function(args)
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
            this._buf = new Ice.Buffer(data);
        }
        else if(args.buffer !== null)
        {
            this._buf = args.buffer;
        }
        else
        {
            this._buf = new Ice.Buffer();
        }
    },
    //
    // This function allows this object to be reused, rather than reallocated.
    //
    reset: function()
    {
        this._buf.reset();
        this.clear();
    },
    clear: function()
    {
        if(this._encapsStack !== null)
        {
            Debug.assert(this._encapsStack.next);
            this._encapsStack.next = this._encapsCache;
            this._encapsCache = this._encapsStack;
            this._encapsCache.reset();
            this._encapsStack = null;
        }

        this._startSeq = -1;
        this._sliceValues = true;
    },
    swap: function(other)
    {
        Debug.assert(this._instance === other._instance);

        var tmpBuf, tmpEncoding, tmpTraceSlicing, tmpClosure, tmpSliceValues, tmpStartSeq, tmpMinSeqSize, tmpSizePos,
            tmpVfm, tmpLogger, tmpCompactIdResolver;

        tmpBuf = other._buf;
        other._buf = this._buf;
        this._buf = tmpBuf;

        tmpEncoding = other._encoding;
        other._encoding = this._encoding;
        this._encoding = tmpEncoding;

        tmpTraceSlicing = other._traceSlicing;
        other._traceSlicing = this._traceSlicing;
        this._traceSlicing = tmpTraceSlicing;

        tmpClosure = other._closure;
        other._closure = this._closure;
        this._closure = tmpClosure;

        tmpSliceValues = other._sliceValues;
        other._sliceValues = this._sliceValues;
        this._sliceValues = tmpSliceValues;

        //
        // Swap is never called for InputStreams that have encapsulations being read/write. However,
        // encapsulations might still be set in case marshaling or unmarshaling failed. We just
        // reset the encapsulations if there are still some set.
        //
        this.resetEncapsulation();
        other.resetEncapsulation();

        tmpStartSeq = other._startSeq;
        other._startSeq = this._startSeq;
        this._startSeq = tmpStartSeq;

        tmpMinSeqSize = other._minSeqSize;
        other._minSeqSize = this._minSeqSize;
        this._minSeqSize = tmpMinSeqSize;

        tmpSizePos = other._sizePos;
        other._sizePos = this._sizePos;
        this._sizePos = tmpSizePos;

        tmpVfm = other._valueFactoryManager;
        other._valueFactoryManager = this._valueFactoryManager;
        this._valueFactoryManager = tmpVfm;

        tmpLogger = other._logger;
        other._logger = this._logger;
        this._logger = tmpLogger;

        tmpCompactIdResolver = other._compactIdResolver;
        other._compactIdResolver = this._compactIdResolver;
        this._compactIdResolver = tmpCompactIdResolver;
    },
    resetEncapsulation: function()
    {
        this._encapsStack = null;
    },
    resize: function(sz)
    {
        this._buf.resize(sz);
        this._buf.position = sz;
    },
    startValue: function()
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        this._encapsStack.decoder.startInstance(SliceType.ValueSlice);
    },
    endValue: function(preserve)
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        return this._encapsStack.decoder.endInstance(preserve);
    },
    startException: function()
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        this._encapsStack.decoder.startInstance(SliceType.ExceptionSlice);
    },
    endException: function(preserve)
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        return this._encapsStack.decoder.endInstance(preserve);
    },
    startEncapsulation: function()
    {
        var curr = this._encapsCache;
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
        var sz = this.readInt();
        if(sz < 6)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        if(sz - 4 > this._buf.remaining)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        this._encapsStack.sz = sz;

        var encoding = new Ice.EncodingVersion();
        encoding.__read(this);
        Protocol.checkSupportedEncoding(encoding); // Make sure the encoding is supported.
        this._encapsStack.setEncoding(encoding);

        return encoding;
    },
    endEncapsulation: function()
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

        var curr = this._encapsStack;
        this._encapsStack = curr.next;
        curr.next = this._encapsCache;
        this._encapsCache = curr;
        this._encapsCache.reset();
    },
    skipEmptyEncapsulation: function()
    {
        var sz = this.readInt();
        if(sz < 6)
        {
            throw new Ice.EncapsulationException();
        }
        if(sz - 4 > this._buf.remaining)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }

        var encoding = new Ice.EncodingVersion();
        encoding.__read(this);
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
    },
    readEncapsulation: function(encoding)
    {
        Debug.assert(encoding !== undefined);
        var sz = this.readInt();
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
            encoding.__read(this);
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
    },
    getEncoding: function()
    {
        return this._encapsStack !== null ? this._encapsStack.encoding : this._encoding;
    },
    getEncapsulationSize: function()
    {
        Debug.assert(this._encapsStack !== null);
        return this._encapsStack.sz - 6;
    },
    skipEncapsulation: function()
    {
        var sz = this.readInt();
        if(sz < 6)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        var encoding = new Ice.EncodingVersion();
        encoding.__read(this);
        try
        {
            this._buf.position = this._buf.position + sz - 6;
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        return encoding;
    },
    startSlice: function() // Returns type ID of next slice
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        return this._encapsStack.decoder.startSlice();
    },
    endSlice: function()
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        this._encapsStack.decoder.endSlice();
    },
    skipSlice: function()
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.decoder !== null);
        this._encapsStack.decoder.skipSlice();
    },
    readPendingValues: function()
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
    },
    readSize: function()
    {
        try
        {
            var b = this._buf.get();
            if(b === 255)
            {
                var v = this._buf.getInt();
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
    },
    readAndCheckSeqSize: function(minSize)
    {
        var sz = this.readSize();

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
    },
    readBlob: function(sz)
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
    },
    readOptional: function(tag, expectedFormat)
    {
        Debug.assert(this._encapsStack !== null);
        if(this._encapsStack.decoder !== null)
        {
            return this._encapsStack.decoder.readOptional(tag, expectedFormat);
        }
        return this.readOptImpl(tag, expectedFormat);
    },
    readOptionalHelper: function(tag, format, read)
    {
        if(this.readOptional(tag, format))
        {
            return read.call(this);
        }
        else
        {
            return undefined;
        }
    },
    readByte: function()
    {
        try
        {
            return this._buf.get();
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    },
    readByteSeq: function()
    {
        return this._buf.getArray(this.readAndCheckSeqSize(1));
    },
    readBool: function()
    {
        try
        {
            return this._buf.get() === 1;
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    },
    readShort: function()
    {
        try
        {
            return this._buf.getShort();
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    },
    readInt: function()
    {
        try
        {
            return this._buf.getInt();
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    },
    readLong: function()
    {
        try
        {
            return this._buf.getLong();
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    },
    readFloat: function()
    {
        try
        {
            return this._buf.getFloat();
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    },
    readDouble: function()
    {
        try
        {
            return this._buf.getDouble();
        }
        catch(ex)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
    },
    readString: function()
    {
        var len = this.readSize();
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
    },
    readProxy: function(type)
    {
        return this._instance.proxyFactory().streamToProxy(this, type);
    },
    readOptionalProxy: function(tag, type)
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
    },
    readEnum: function(T)
    {
        var v;
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

        var e = T.valueOf(v);
        if(e === undefined)
        {
            throw new Ice.MarshalException("enumerator value " + v + " is out of range");
        }
        return e;
    },
    readOptionalEnum: function(tag, T)
    {
        if(this.readOptional(tag, OptionalFormat.Size))
        {
            return this.readEnum(T);
        }
        else
        {
            return undefined;
        }
    },
    readValue: function(cb, T)
    {
        this.initEncaps();
        //
        // BUGFIX:
        // With Chrome on Linux the invocation of readValue on the decoder sometimes
        // calls InputStream.readValue with the decoder object as this param.
        // Use call instead of directly invoking the method to workaround this bug.
        //
        this._encapsStack.decoder.readValue.call(
            this._encapsStack.decoder,
            function(obj)
            {
                if(obj !== null && !(obj.ice_instanceof(T)))
                {
                    ExUtil.throwUOE(T.ice_staticId(), obj);
                }
                cb(obj);
            });
    },
    readOptionalValue: function(tag, cb, T)
    {
        if(this.readOptional(tag, OptionalFormat.Class))
        {
            this.readValue(cb, T);
        }
        else
        {
            cb(undefined);
        }
    },
    throwException: function()
    {
        this.initEncaps();
        this._encapsStack.decoder.throwException();
    },
    readOptImpl: function(readTag, expectedFormat)
    {
        var b, v, format, tag, offset;

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

            v = this.readByte();

            if(v === Protocol.OPTIONAL_END_MARKER)
            {
                this._buf.position -= 1; // Rewind.
                return false;
            }

            format = OptionalFormat.valueOf(v & 0x07); // First 3 bits.
            tag = v >> 3;
            if(tag === 30)
            {
                tag = this.readSize();
            }

            if(tag > readTag)
            {
                offset = tag < 30 ? 1 : (tag < 255 ? 2 : 6); // Rewind
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
    },
    skipOptional: function(format)
    {
        switch(format)
        {
            case OptionalFormat.F1:
                this.skip(1);
                break;
            case OptionalFormat.F2:
                this.skip(2);
                break;
            case OptionalFormat.F4:
                this.skip(4);
                break;
            case OptionalFormat.F8:
                this.skip(8);
                break;
            case OptionalFormat.Size:
                this.skipSize();
                break;
            case OptionalFormat.VSize:
                this.skip(this.readSize());
                break;
            case OptionalFormat.FSize:
                this.skip(this.readInt());
                break;
            case OptionalFormat.Class:
                this.readValue(null, Ice.Object);
                break;
        }
    },
    skipOptionals: function()
    {
        var b, v, format;
        //
        // Skip remaining un-read optional members.
        //
        while(true)
        {
            if(this._buf.position >= this._encapsStack.start + this._encapsStack.sz)
            {
                return; // End of encapsulation also indicates end of optionals.
            }

            b = this.readByte();
            v = b < 0 ? b + 256 : b;
            if(v === Protocol.OPTIONAL_END_MARKER)
            {
                return;
            }

            format = OptionalFormat.valueOf(v & 0x07); // Read first 3 bits.
            if((v >> 3) === 30)
            {
                this.skipSize();
            }
            this.skipOptional(format);
        }
    },
    skip: function(size)
    {
        if(size > this._buf.remaining)
        {
            throw new Ice.UnmarshalOutOfBoundsException();
        }
        this._buf.position += size;
    },
    skipSize: function()
    {
        var b = this.readByte();
        if(b === 255)
        {
            this.skip(4);
        }
    },
    isEmpty: function()
    {
        return this._buf.empty();
    },
    expand: function(n)
    {
        this._buf.expand(n);
    },
    createInstance: function(id)
    {
        var obj = null, Class;
        try
        {
            var typeId = id.length > 2 ? id.substr(2).replace(/::/g, ".") : "";
            /*jshint -W061 */
            Class = __M.type(typeId);
            /*jshint +W061 */
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
    },
    createUserException: function(id)
    {
        var userEx = null, Class;

        try
        {
            var typeId = id.length > 2 ? id.substr(2).replace(/::/g, ".") : "";
            /*jshint -W061 */
            Class = __M.type(typeId);
            /*jshint +W061 */
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
    },
    resolveCompactId: function(compactId)
    {
        var typeId = Ice.CompactIdRegistry.get(compactId);
        return typeId === undefined ? "" : typeId;
    },
    isEncoding_1_0: function()
    {
        return this._encapsStack !== null ? this._encapsStack.encoding_1_0 : this._encoding.equals(Ice.Encoding_1_0);
    },
    initEncaps: function()
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
    },
    traceSkipSlice: function(typeId, sliceType)
    {
        if(this._traceSlicing && this._logger !== null)
        {
            TraceUtil.traceSlicing(sliceType === SliceType.ExceptionSlice ? "exception" : "object", typeId, "Slicing",
                                   this._logger);
        }
    }
});

var defineProperty = Object.defineProperty;

//
// Sets the value factory manager to use when marshaling value instances. If the stream
// was initialized with a communicator, the communicator's value factory manager will
// be used by default.
//
defineProperty(InputStream.prototype, "valueFactoryManager", {
    get: function() { return this._valueFactoryManager; },
    set: function(vfm) { this._valueFactoryManager = vfm !== undefined ? vfm : null; }
});

//
// Sets the logger to use when logging trace messages. If the stream
// was initialized with a communicator, the communicator's logger will
// be used by default.
//
defineProperty(InputStream.prototype, "logger", {
    get: function() { return this._logger; },
    set: function(l) { this._logger = l !== undefined ? l : null; }
});

//
// Sets the compact ID resolver to use when unmarshaling value and exception
// instances. If the stream was initialized with a communicator, the communicator's
// resolver will be used by default.
//
defineProperty(InputStream.prototype, "compactIdResolver", {
    get: function() { return this._compactIdResolver; },
    set: function(r) { this._compactIdResolver = r !== undefined ? r : null; }
});

//
// Determines the behavior of the stream when extracting instances of Slice classes.
// A instance is "sliced" when a factory cannot be found for a Slice type ID.
// The stream's default behavior is to slice instances.
//
// If slicing is disabled and the stream encounters a Slice type ID
// during decoding for which no value factory is installed, it raises
// NoValueFactoryException.
//
defineProperty(InputStream.prototype, "sliceValues", {
    get: function() { return this._sliceValues; },
    set: function(b) { this._sliceValues = b; }
});

//
// Determines whether the stream logs messages about slicing instances of Slice values.
//
defineProperty(InputStream.prototype, "traceSlicing", {
    get: function() { return this._traceSlicing; },
    set: function(b) { this._traceSlicing = b; }
});

defineProperty(InputStream.prototype, "pos", {
    get: function() { return this._buf.position; },
    set: function(n) { this._buf.position = n; }
});

defineProperty(InputStream.prototype, "size", {
    get: function() { return this._buf.limit; }
});

defineProperty(InputStream.prototype, "instance", {
    get: function() { return this._instance; }
});

defineProperty(InputStream.prototype, "closure", {
    get: function() { return this._type; },
    set: function(type) { this._type = type; }
});

defineProperty(InputStream.prototype, "buffer", {
    get: function() { return this._buf; }
});

//
// OutputStream
//

var EncapsEncoder = Class({
    __init__: function(stream, encaps)
    {
        this._stream = stream;
        this._encaps = encaps;
        this._marshaledMap = new HashMap(); // HashMap<Ice.Object, int>;
        this._typeIdMap = null; // Lazy initialized. HashMap<String, int>
        this._typeIdIndex = 0;
    },
    writeOptional: function()
    {
        return false;
    },
    writePendingValues: function()
    {
        return undefined;
    },
    registerTypeId: function(typeId)
    {
        if(this._typeIdMap === null) // Lazy initialization
        {
            this._typeIdMap = new HashMap(); // HashMap<String, int>
        }

        var p = this._typeIdMap.get(typeId);
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
});

var EncapsEncoder10 = Class(EncapsEncoder, {
    __init__: function(stream, encaps)
    {
        EncapsEncoder.call(this, stream, encaps);
        this._sliceType = SliceType.NoSlice;
        this._writeSlice = 0;        // Position of the slice data members
        this._valueIdIndex = 0;
        this._toBeMarshaledMap = new HashMap(); // HashMap<Ice.Object, Integer>();
    },
    writeValue: function(v)
    {
        Debug.assert(v !== undefined);
        //
        // Object references are encoded as a negative integer in 1.0.
        //
        if(v !== null)
        {
            this._stream.writeInt(-this.registerValue(v));
        }
        else
        {
            this._stream.writeInt(0);
        }
    },
    writeUserException: function(v)
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
        var usesClasses = v.__usesClasses();
        this._stream.writeBool(usesClasses);
        v.__write(this._stream);
        if(usesClasses)
        {
            this.writePendingValues();
        }
    },
    startInstance: function(sliceType)
    {
        this._sliceType = sliceType;
    },
    endInstance: function()
    {
        if(this._sliceType === SliceType.ValueSlice)
        {
            //
            // Write the Object slice.
            //
            this.startSlice(IceObject.ice_staticId(), -1, true);
            this._stream.writeSize(0); // For compatibility with the old AFM.
            this.endSlice();
        }
        this._sliceType = SliceType.NoSlice;
    },
    startSlice: function(typeId)
    {
        //
        // For instance slices, encode a boolean to indicate how the type ID
        // is encoded and the type ID either as a string or index. For
        // exception slices, always encode the type ID as a string.
        //
        if(this._sliceType === SliceType.ValueSlice)
        {
            var index = this.registerTypeId(typeId);
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
    },
    endSlice: function()
    {
        //
        // Write the slice length.
        //
        var sz = this._stream.pos - this._writeSlice + 4;
        this._stream.rewriteInt(sz, this._writeSlice - 4);
    },
    writePendingValues: function()
    {
        var self = this,
            writeCB = function(key, value)
                        {
                            //
                            // Ask the instance to marshal itself. Any new class
                            // instances that are triggered by the classes marshaled
                            // are added to toBeMarshaledMap.
                            //
                            self._stream.writeInt(value);

                            try
                            {
                                key.ice_preMarshal();
                            }
                            catch(ex)
                            {
                                self._stream.instance.initializationData().logger.warning(
                                    "exception raised by ice_preMarshal:\n" + ex.toString());
                            }

                            key.__write(self._stream);
                        },
            savedMap;

        while(this._toBeMarshaledMap.size > 0)
        {
            //
            // Consider the to be marshalled instances as marshalled now,
            // this is necessary to avoid adding again the "to be
            // marshalled instances" into _toBeMarshaledMap while writing
            // instances.
            //
            this._marshaledMap.merge(this._toBeMarshaledMap);

            savedMap = this._toBeMarshaledMap;
            this._toBeMarshaledMap = new HashMap(); // HashMap<Ice.Object, int>();
            this._stream.writeSize(savedMap.size);
            savedMap.forEach(writeCB);
        }
        this._stream.writeSize(0); // Zero marker indicates end of sequence of sequences of instances.
    },
    registerValue: function(v)
    {
        Debug.assert(v !== null);

        //
        // Look for this instance in the to-be-marshaled map.
        //
        var p = this._toBeMarshaledMap.get(v);
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
});

var EncapsEncoder11 = Class(EncapsEncoder, {
    __init__: function(stream, encaps)
    {
        EncapsEncoder.call(this, stream, encaps);
        this._current = null;
        this._valueIdIndex = 1;
    },
    writeValue: function(v)
    {
        Debug.assert(v !== undefined);
        var index, idx;
        if(v === null)
        {
            this._stream.writeSize(0);
        }
        else if(this._current !== null && this._encaps.format === FormatType.SlicedFormat)
        {
            if(this._current.indirectionTable === null) // Lazy initialization
            {
                this._current.indirectionTable = []; // Ice.Object[]
                this._current.indirectionMap = new HashMap(); // HashMap<Ice.Object, int>
            }

            //
            // If writing an instance within a slice and using the sliced
            // format, write an index from the instance indirection
            // table. The indirect instance table is encoded at the end of
            // each slice and is always read (even if the Slice is
            // unknown).
            //
            index = this._current.indirectionMap.get(v);
            if(index === undefined)
            {
                this._current.indirectionTable.push(v);
                idx = this._current.indirectionTable.length; // Position + 1 (0 is reserved for nil)
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
    },
    writePendingValues: function()
    {
        return undefined;
    },
    writeUserException: function(v)
    {
        Debug.assert(v !== null && v !== undefined);
        v.__write(this._stream);
    },
    startInstance: function(sliceType, data)
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
    },
    endInstance: function()
    {
        this._current = this._current.previous;
    },
    startSlice: function(typeId, compactId, last)
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
                    var index = this.registerTypeId(typeId);
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
    },
    endSlice: function()
    {
        var sz, i, length;

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
            sz = this._stream.pos - this._current.writeSlice + 4;
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
            for(i = 0, length = this._current.indirectionTable.length; i < length; ++i)
            {
                this.writeInstance(this._current.indirectionTable[i]);
            }
            this._current.indirectionTable.length = 0; // Faster way to clean array in JavaScript
            this._current.indirectionMap.clear();
        }

        //
        // Finally, update the slice flags.
        //
        this._stream.rewriteByte(this._current.sliceFlags, this._current.sliceFlagsPos);
    },
    writeOptional: function(tag, format)
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
    },
    writeSlicedData: function(slicedData)
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

        var i, ii, info,
            j, jj;

        for(i = 0, ii = slicedData.slices.length; i < ii; ++i)
        {
            info = slicedData.slices[i];
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
                    this._current.indirectionTable = []; // Ice.Object[]
                    this._current.indirectionMap = new HashMap(); // HashMap<Ice.Object, int>
                }

                for(j = 0, jj = info.instances.length; j < jj; ++j)
                {
                    this._current.indirectionTable.push(info.instances[j]);
                }
            }

            this.endSlice();
        }
    },
    writeInstance: function(v)
    {
        Debug.assert(v !== null && v !== undefined);

        //
        // If the instance was already marshaled, just write it's ID.
        //
        var p = this._marshaledMap.get(v);
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
        v.__write(this._stream);
    }
});

EncapsEncoder11.InstanceData = function(previous)
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
    this.writeSlice = 0;    // Position of the slice data members
    this.sliceFlagsPos = 0; // Position of the slice flags
    this.indirectionTable = null; // Ice.Object[]
    this.indirectionMap = null; // HashMap<Ice.Object, int>
};

var WriteEncaps = Class({
    __init__: function()
    {
        this.start = 0;
        this.format = FormatType.DefaultFormat;
        this.encoding = null;
        this.encoding_1_0 = false;
        this.encoder = null;
        this.next = null;
    },
    reset: function()
    {
        this.encoder = null;
    },
    setEncoding: function(encoding)
    {
        this.encoding = encoding;
        this.encoding_1_0 = encoding.equals(Ice.Encoding_1_0);
    }
});

var OutputStream = Class({
    __init__: function(arg1, arg2)
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
    },
    //
    // This function allows this object to be reused, rather than reallocated.
    //
    reset: function()
    {
        this._buf.reset();
        this.clear();
    },
    clear: function()
    {
        if(this._encapsStack !== null)
        {
            Debug.assert(this._encapsStack.next);
            this._encapsStack.next = this._encapsCache;
            this._encapsCache = this._encapsStack;
            this._encapsCache.reset();
            this._encapsStack = null;
        }
    },
    finished: function()
    {
        var buf = this.prepareWrite();
        return buf.getArray(this.size);
    },
    swap: function(other)
    {
        Debug.assert(this._instance === other._instance);

        var tmpBuf, tmpEncoding, tmpClosure;

        tmpBuf = other._buf;
        other._buf = this._buf;
        this._buf = tmpBuf;

        tmpEncoding = other._encoding;
        other._encoding = this._encoding;
        this._encoding = tmpEncoding;

        tmpClosure = other._closure;
        other._closure = this._closure;
        this._closure = tmpClosure;

        //
        // Swap is never called for streams that have encapsulations being written. However,
        // encapsulations might still be set in case marshaling failed. We just
        // reset the encapsulations if there are still some set.
        //
        this.resetEncapsulation();
        other.resetEncapsulation();
    },
    resetEncapsulation: function()
    {
        this._encapsStack = null;
    },
    resize: function(sz)
    {
        this._buf.resize(sz);
        this._buf.position = sz;
    },
    prepareWrite: function()
    {
        this._buf.position = 0;
        return this._buf;
    },
    startValue: function(data)
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.startInstance(SliceType.ValueSlice, data);
    },
    endValue: function()
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.endInstance();
    },
    startException: function(data)
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.startInstance(SliceType.ExceptionSlice, data);
    },
    endException: function()
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.endInstance();
    },
    startEncapsulation: function(encoding, format)
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

        var curr = this._encapsCache;
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
        this._encapsStack.encoding.__write(this);
    },
    endEncapsulation: function()
    {
        Debug.assert(this._encapsStack);

        // Size includes size and version.
        var start = this._encapsStack.start;
        var sz = this._buf.limit - start;
        this._buf.putIntAt(start, sz);

        var curr = this._encapsStack;
        this._encapsStack = curr.next;
        curr.next = this._encapsCache;
        this._encapsCache = curr;
        this._encapsCache.reset();
    },
    writeEmptyEncapsulation: function(encoding)
    {
        Protocol.checkSupportedEncoding(encoding);
        this.writeInt(6); // Size
        encoding.__write(this);
    },
    writeEncapsulation: function(v)
    {
        if(v.length < 6)
        {
            throw new Ice.EncapsulationException();
        }
        this.expand(v.length);
        this._buf.putArray(v);
    },
    getEncoding: function()
    {
        return this._encapsStack !== null ? this._encapsStack.encoding : this._encoding;
    },
    startSlice: function(typeId, compactId, last)
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.startSlice(typeId, compactId, last);
    },
    endSlice: function()
    {
        Debug.assert(this._encapsStack !== null && this._encapsStack.encoder !== null);
        this._encapsStack.encoder.endSlice();
    },
    writePendingValues: function()
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
    },
    writeSize: function(v)
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
    },
    startSize: function()
    {
        var pos = this._buf.position;
        this.writeInt(0); // Placeholder for 32-bit size
        return pos;
    },
    endSize: function(pos)
    {
        Debug.assert(pos >= 0);
        this.rewriteInt(this._buf.position - pos - 4, pos);
    },
    writeBlob: function(v)
    {
        if(v === null)
        {
            return;
        }
        this.expand(v.length);
        this._buf.putArray(v);
    },
    // Read/write format and tag for optionals
    writeOptional: function(tag, format)
    {
        Debug.assert(this._encapsStack !== null);
        if(this._encapsStack.encoder !== null)
        {
            return this._encapsStack.encoder.writeOptional(tag, format);
        }
        return this.writeOptImpl(tag, format);
    },
    writeOptionalHelper: function(tag, format, write, v)
    {
        if(v !== undefined)
        {
            if(this.writeOptional(tag, format))
            {
                write.call(this, v);
            }
        }
    },
    writeByte: function(v)
    {
        this.expand(1);
        this._buf.put(v);
    },
    rewriteByte: function(v, dest)
    {
        this._buf.putAt(dest, v);
    },
    writeByteSeq: function(v)
    {
        if(v === null || v.length === 0)
        {
            this.writeSize(0);
        }
        else
        {
            this.writeSize(v.length);
            this.expand(v.length);
            this._buf.putArray(v);
        }
    },
    writeBool: function(v)
    {
        this.expand(1);
        this._buf.put(v ? 1 : 0);
    },
    rewriteBool: function(v, dest)
    {
        this._buf.putAt(dest, v ? 1 : 0);
    },
    writeShort: function(v)
    {
        this.expand(2);
        this._buf.putShort(v);
    },
    writeInt: function(v)
    {
        this.expand(4);
        this._buf.putInt(v);
    },
    rewriteInt: function(v, dest)
    {
        this._buf.putIntAt(dest, v);
    },
    writeLong: function(v)
    {
        this.expand(8);
        this._buf.putLong(v);
    },
    writeFloat: function(v)
    {
        this.expand(4);
        this._buf.putFloat(v);
    },
    writeDouble: function(v)
    {
        this.expand(8);
        this._buf.putDouble(v);
    },
    writeString: function(v)
    {
        if(v === null || v.length === 0)
        {
            this.writeSize(0);
        }
        else
        {
            this._buf.writeString(this, v);
        }
    },
    writeProxy: function(v)
    {
        if(v !== null)
        {
            v.__write(this);
        }
        else
        {
            var ident = new Ice.Identity();
            ident.__write(this);
        }
    },
    writeOptionalProxy: function(tag, v)
    {
        if(v !== undefined)
        {
            if(this.writeOptional(tag, OptionalFormat.FSize))
            {
                var pos = this.startSize();
                this.writeProxy(v);
                this.endSize(pos);
            }
        }
    },
    writeEnum: function(v)
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
    },
    writeValue: function(v)
    {
        this.initEncaps();
        this._encapsStack.encoder.writeValue(v);
    },
    writeOptionalValue: function(tag, v)
    {
        if(v !== undefined)
        {
            if(this.writeOptional(tag, OptionalFormat.Class))
            {
                this.writeValue(v);
            }
        }
    },
    writeUserException: function(e)
    {
        this.initEncaps();
        this._encapsStack.encoder.writeUserException(e);
    },
    writeOptImpl: function(tag, format)
    {
        if(this.isEncoding_1_0())
        {
            return false; // Optional members aren't supported with the 1.0 encoding.
        }

        var v = format.value;
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
    },
    isEmpty: function()
    {
        return this._buf.empty();
    },
    expand: function(n)
    {
        this._buf.expand(n);
    },
    isEncoding_1_0: function()
    {
        return this._encapsStack ? this._encapsStack.encoding_1_0 : this._encoding.equals(Ice.Encoding_1_0);
    },
    initEncaps: function()
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
});

//
// Sets the encoding format for class and exception instances.
//
defineProperty(OutputStream.prototype, "format", {
    get: function() { return this._format; },
    set: function(f) { this._format = f; }
});

defineProperty(OutputStream.prototype, "pos", {
    get: function() { return this._buf.position; },
    set: function(n) { this._buf.position = n; }
});

defineProperty(OutputStream.prototype, "size", {
    get: function() { return this._buf.limit; }
});

defineProperty(OutputStream.prototype, "instance", {
    get: function() { return this._instance; }
});

defineProperty(OutputStream.prototype, "closure", {
    get: function() { return this._closure; },
    set: function(closure) { this._closure = closure; }
});

defineProperty(OutputStream.prototype, "buffer", {
    get: function() { return this._buf; }
});

var defineBuiltinHelper = function(write, read, sz, format, min, max)
{
    var helper = {
        write: function(os, v) { return write.call(os, v); },
        read: function(is) { return read.call(is); },
        writeOptional: function(os, tag, v) { os.writeOptionalHelper(tag, format, write, v); },
        readOptional: function(is, tag) { return is.readOptionalHelper(tag, format, read); },
    };

    if(min !== undefined && max !== undefined)
    {
        helper.validate = function(v) {
            return v >= min && v <= max;
        };
    }
    defineProperty(helper, "minWireSize", {
        get: function() { return sz; }
    });
    return helper;
};

var istr = InputStream.prototype;
var ostr = OutputStream.prototype;

//
// Constants to use in number type range checks.
//
var MIN_UINT8_VALUE = 0x0;
var MAX_UINT8_VALUE = 0xFF;

var MIN_INT16_VALUE = -0x8000;
var MAX_INT16_VALUE = 0x7FFF;

var MIN_UINT32_VALUE = 0x0;
var MAX_UINT32_VALUE = 0xFFFFFFFF;

var MIN_INT32_VALUE = -0x80000000;
var MAX_INT32_VALUE = 0x7FFFFFFF;

var MIN_FLOAT32_VALUE = -3.4028234664e+38;
var MAX_FLOAT32_VALUE = 3.4028234664e+38;

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

Ice.ObjectHelper = {
    write: function(os, v)
    {
        os.writeValue(v);
    },
    read: function(is)
    {
        var o;
        is.readValue(function(v) { o = v; }, Ice.Object);
        return o;
    },
    writeOptional: function(os, tag, v)
    {
        os.writeOptionalValue(tag, Ice.OptionalFormat.Class, ostr.writeValue, v);
    },
    readOptional: function(is, tag)
    {
        var o;
        is.readOptionalValue(tag, function(v) { o = v; }, Ice.Object);
        return o;
    },
};

defineProperty(Ice.ObjectHelper, "minWireSize", {
    get: function() { return 1; }
});

Ice.InputStream = InputStream;
Ice.OutputStream = OutputStream;
module.exports.Ice = Ice;
