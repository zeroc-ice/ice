// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// Ice.Object
//
// Using IceObject in this file to avoid collisions with the native Object.
//
var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/DispatchStatus",
        "../Ice/Exception",
        "../Ice/FormatType",
        "../Ice/StreamHelpers",
        "../Ice/OptionalFormat"
    ]);

var Class = Ice.Class;

var nextAddress = 0;

var IceObject = Class({
    __init__: function()
    {
        // Fake Address used as the hashCode for this object instance.
        this.__address = nextAddress++;
    },
    hashCode: function()
    {
        return this.__address;
    },
    ice_isA: function(s, current)
    {
        return this.__mostDerivedType().__ids.indexOf(s) >= 0;
    },
    ice_ping: function(current)
    {
    },
    ice_ids: function(current)
    {
        return this.__mostDerivedType().__ids;
    },
    ice_id: function(current)
    {
        return this.__mostDerivedType().__id;
    },
    toString: function()
    {
        return "[object " + this.ice_id() + "]";
    },
    ice_preMarshal: function()
    {
    },
    ice_postUnmarshal: function()
    {
    },
    __write: function(os)
    {
        os.startValue(null);
        __writeImpl(this, os, this.__mostDerivedType());
        os.endValue();
    },
    __read: function(is)
    {
        is.startValue();
        __readImpl(this, is, this.__mostDerivedType());
        is.endValue(false);
    },
    ice_instanceof: function(T)
    {
        if(T)
        {
            if(this instanceof T)
            {
                return true;
            }
            return this.__mostDerivedType().__instanceof(T);
        }
        return false;
    },
    //
    // __mostDerivedType returns the the most derived Ice generated class. This is
    // necessary because the user might extend Slice generated classes. The user
    // class extensions don't have __id, __ids, __instanceof etc static members so
    // the implementation of ice_id, ice_ids and ice_instanceof would fail trying
    // to access those members of the user defined class. Instead, ice_id, ice_ids
    // and ice_instanceof call __mostDerivedType to get the most derived Ice class.
    //
    // The __mostDerivedType is overriden by each Slice generated class, see the
    // Slice.defineObject method implementation for details.
    //
    __mostDerivedType: function()
    {
        return IceObject;
    },
    //
    // The default implementation of equals compare references.
    // 
    equals: function(other)
    {
        return this === other;
    }
});

//
// These methods are used for object parameters.
//
IceObject.write = function(os, v)
{
    os.writeValue(v);
};

IceObject.writeOptional = function(os, tag, v)
{
    os.writeOptionalValue(tag, v);
};

IceObject.read = function(is)
{
    var v = { value: null };
    is.readValue(function(o) { v.value = o; }, IceObject);
    return v;
};

IceObject.readOptional = function(is, tag)
{
    var v = { value: undefined };
    is.readOptionalValue(tag, function(o) { v.value = o; }, IceObject);
    return v;
};

IceObject.ice_staticId = function()
{
    return IceObject.__id;
};

IceObject.__instanceof = function(T)
{
    if(T === this)
    {
        return true;
    }

    for(var i in this.__implements)
    {
        if(this.__implements[i].__instanceof(T))
        {
            return true;
        }
    }

    if(this.__parent)
    {
        return this.__parent.__instanceof(T);
    }
    return false;
};

IceObject.__ids = ["::Ice::Object"];
IceObject.__id = IceObject.__ids[0];
IceObject.__compactId = -1;
IceObject.__preserved = false;

//
// Private methods
//

var __writeImpl = function(obj, os, type)
{
    //
    // The __writeImpl method is a recursive method that goes down the
    // class hierarchy to marshal each slice of the class using the
    // generated __writeMemberImpl method.
    //

    if(type === undefined || type === IceObject)
    {
        return; // Don't marshal anything for IceObject
    }

    os.startSlice(type.__id, type.__compactId, type.__parent === IceObject);
    if(type.prototype.__writeMemberImpl)
    {
        type.prototype.__writeMemberImpl.call(obj, os);
    }
    os.endSlice();
    __writeImpl(obj, os, type.__parent);
};

var __readImpl = function(obj, is, type)
{
    //
    // The __readImpl method is a recursive method that goes down the
    // class hierarchy to marshal each slice of the class using the
    // generated __readMemberImpl method.
    //

    if(type === undefined || type === IceObject)
    {
        return; // Don't marshal anything for IceObject
    }

    is.startSlice();
    if(type.prototype.__readMemberImpl)
    {
        type.prototype.__readMemberImpl.call(obj, is);
    }
    is.endSlice();
    __readImpl(obj, is, type.__parent);
};

var __writePreserved = function(os)
{
    //
    // For Slice classes which are marked "preserved", the implementation of this method
    // replaces the Ice.Object.prototype.__write method.
    //
    os.startValue(this.__slicedData);
    __writeImpl(this, os, this.__mostDerivedType());
    os.endValue();
};

var __readPreserved = function(is)
{
    //
    // For Slice classes which are marked "preserved", the implementation of this method
    // replaces the Ice.Object.prototype.__read method.
    //
    is.startValue();
    __readImpl(this, is, this.__mostDerivedType());
    this.__slicedData = is.endValue(true);
};

Ice.Object = IceObject;

var Slice = Ice.Slice;
Slice.defineLocalObject = function(constructor, base)
{
    var obj = constructor || function(){};

    if(base !== undefined)
    {
        obj.prototype = new base();
        obj.__parent = base;
        obj.prototype.constructor = constructor;
    }

    return obj;
};

Slice.defineObject = function(constructor, base, intfs, scope, ids, compactId, writeImpl, readImpl, preserved)
{
    var obj = constructor || function(){};

    obj.prototype = new base();
    obj.__parent = base;
    obj.__ids = ids;
    obj.__id = ids[scope];
    obj.__compactId = compactId;
    obj.__instanceof = IceObject.__instanceof;
    obj.__implements = intfs;

    //
    // These methods are used for object parameters.
    //
    obj.write = function(os, v)
    {
        os.writeValue(v);
    };
    obj.writeOptional = function(os, tag, v)
    {
        os.writeOptionalValue(tag, v);
    };
    obj.read = function(is)
    {
        var v = { value: null };
        is.readValue(function(o) { v.value = o; }, obj);
        return v;
    };
    obj.readOptional = function(is, tag)
    {
        var v = { value: undefined };
        is.readOptionalValue(tag, function(o) { v.value = o; }, obj);
        return v;
    };

    obj.ice_staticId = function()
    {
        return ids[scope];
    };

    obj.prototype.constructor = obj;
    obj.prototype.__mostDerivedType = function() { return obj; };
    if(preserved)
    {
        obj.prototype.__write = __writePreserved;
        obj.prototype.__read = __readPreserved;
    }
    obj.prototype.__writeMemberImpl = writeImpl;
    obj.prototype.__readMemberImpl = readImpl;

    return obj;
};
module.exports.Ice = Ice;
