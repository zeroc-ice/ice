// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/OptionalFormat").Ice;

const defineProperty = Object.defineProperty;
const OptionalFormat = Ice.OptionalFormat;

const StreamHelpers = {};

StreamHelpers.FSizeOptHelper = function()
{
    this.writeOptional = function(os, tag, v)
    {
        if(v !== undefined && os.writeOptional(tag, OptionalFormat.FSize))
        {
            const pos = os.startSize();
            this.write(os, v);
            os.endSize(pos);
        }
    };

    this.readOptional = function(is, tag)
    {
        let v;
        if(is.readOptional(tag, OptionalFormat.FSize))
        {
            is.skip(4);
            v = this.read(is);
        }
        return v;
    };
};

StreamHelpers.VSizeOptHelper = function()
{
    this.writeOptional = function(os, tag, v)
    {
        if(v !== undefined && os.writeOptional(tag, OptionalFormat.VSize))
        {
            os.writeSize(this.minWireSize);
            this.write(os, v);
        }
    };

    this.readOptional = function(is, tag)
    {
        let v;
        if(is.readOptional(tag, OptionalFormat.VSize))
        {
            is.skipSize();
            v = this.read(is);
        }
        return v;
    };
};

StreamHelpers.VSizeContainerOptHelper = function(elementSize)
{
    this.writeOptional = function(os, tag, v)
    {
        if(v !== undefined && os.writeOptional(tag, OptionalFormat.VSize))
        {
            const sz = this.size(v);
            os.writeSize(sz > 254 ? sz * elementSize + 5 : sz * elementSize + 1);
            this.write(os, v);
        }
    };

    this.readOptional = function(is, tag)
    {
        let v;
        if(is.readOptional(tag, OptionalFormat.VSize))
        {
            is.skipSize();
            v = this.read(is);
        }
        return v;
    };
};

StreamHelpers.VSizeContainer1OptHelper = function()
{
    this.writeOptional = function(os, tag, v)
    {
        if(v !== undefined && os.writeOptional(tag, OptionalFormat.VSize))
        {
            this.write(os, v);
        }
    };

    this.readOptional = function(is, tag)
    {
        let v;
        if(is.readOptional(tag, OptionalFormat.VSize))
        {
            v = this.read(is);
        }
        return v;
    };
};

//
// Sequence helper to write sequences
//
class SequenceHelper
{
    write(os, v)
    {
        if(v === null || v.length === 0)
        {
            os.writeSize(0);
        }
        else
        {
            const helper = this.elementHelper;
            os.writeSize(v.length);
            for(let i = 0; i < v.length; ++i)
            {
                helper.write(os, v[i]);
            }
        }
    }

    read(is)
    {
        const helper = this.elementHelper; // Cache the element helper.
        const sz = is.readAndCheckSeqSize(helper.minWireSize);
        const v = [];
        v.length = sz;
        for(let i = 0; i < sz; ++i)
        {
            v[i] = helper.read(is);
        }
        return v;
    }

    size(v)
    {
        return (v === null || v === undefined) ? 0 : v.length;
    }
    
    get minWireSize()
    {
        return 1;
    }
}



// Speacialization optimized for ByteSeq
const byteSeqHelper = new SequenceHelper();
byteSeqHelper.write = function(os, v) { return os.writeByteSeq(v); };
byteSeqHelper.read = function(is) { return is.readByteSeq(); };
defineProperty(byteSeqHelper, "elementHelper", {
    get: function(){ return Ice.ByteHelper; }
});
StreamHelpers.VSizeContainer1OptHelper.call(byteSeqHelper);

// Read method for value sequences
const valueSequenceHelperRead = function(is)
{
    const sz = is.readAndCheckSeqSize(1);
    const v = [];
    v.length = sz;
    const elementType = this.elementType;
    const readValueAtIndex = function(idx)
    {
        is.readValue(obj => v[idx] = obj, elementType);
    };

    for(let i = 0; i < sz; ++i)
    {
        readValueAtIndex(i);
    }
    return v;
};

StreamHelpers.generateSeqHelper = function(elementHelper, fixed, elementType)
{
    if(elementHelper === Ice.ByteHelper)
    {
        return byteSeqHelper;
    }

    const helper = new SequenceHelper();
    if(fixed)
    {
        if(elementHelper.minWireSize === 1)
        {
            StreamHelpers.VSizeContainer1OptHelper.call(helper);
        }
        else
        {
            StreamHelpers.VSizeContainerOptHelper.call(helper, elementHelper.minWireSize);
        }
    }
    else
    {
        StreamHelpers.FSizeOptHelper.call(helper);
    }

    defineProperty(helper, "elementHelper", {
        get: function(){ return elementHelper; }
    });

    if(elementHelper == Ice.ObjectHelper)
    {
        defineProperty(helper, "elementType", {
            get: function(){ return elementType; }
        });
        helper.read = valueSequenceHelperRead;
    }

    return helper;
};

//
// Dictionary helper to write dictionaries
//
class DictionaryHelper
{
    write(os, v)
    {
        if(v === null || v.size === 0)
        {
            os.writeSize(0);
        }
        else
        {
            const keyHelper = this.keyHelper;
            const valueHelper = this.valueHelper;
            os.writeSize(v.size);
            for(let [key, value] of v)
            {
                keyHelper.write(os, key);
                valueHelper.write(os, value);
            }
        }
    }

    read(is)
    {
        const mapType = this.mapType;
        const v = new mapType();
        const sz = is.readSize();
        const keyHelper = this.keyHelper;
        const valueHelper = this.valueHelper;
        for(let i = 0; i < sz; ++i)
        {
            v.set(keyHelper.read(is), valueHelper.read(is));
        }
        return v;
    }

    size(v)
    {
        return (v === null || v === undefined) ? 0 : v.size;
    }
    
    get minWireSize()
    {
        return 1;
    }
}

// Read method for dictionaries of values
const valueDictionaryHelperRead = function(is)
{
    const sz = is.readSize();
    const mapType = this.mapType;
    const v = new mapType();
    const valueType = this.valueType;

    const readValueForKey = function(key)
    {
        is.readValue(function(obj) { v.set(key, obj); }, valueType);
    };

    const keyHelper = this.keyHelper;
    for(let i = 0; i < sz; ++i)
    {
        readValueForKey(keyHelper.read(is));
    }
    return v;
};

StreamHelpers.generateDictHelper = function(keyHelper, valueHelper, fixed, valueType, mapType)
{
    const helper = new DictionaryHelper();
    if(fixed)
    {
        StreamHelpers.VSizeContainerOptHelper.call(helper, keyHelper.minWireSize + valueHelper.minWireSize);
    }
    else
    {
        StreamHelpers.FSizeOptHelper.call(helper);
    }

    defineProperty(helper, "mapType", {
        get: function(){ return mapType; }
    });

    defineProperty(helper, "keyHelper", {
        get: function(){ return keyHelper; }
    });

    defineProperty(helper, "valueHelper", {
        get: function(){ return valueHelper; }
    });

    if(valueHelper == Ice.ObjectHelper)
    {
        defineProperty(helper, "valueType", {
            get: function(){ return valueType; }
        });

        helper.read = valueDictionaryHelperRead;
    }

    return helper;
};

Ice.StreamHelpers = StreamHelpers;
module.exports.Ice = Ice;
