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
const Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/DispatchStatus",
        "../Ice/Exception",
        "../Ice/FormatType",
        "../Ice/StreamHelpers",
        "../Ice/OptionalFormat"
    ]);

let nextAddress = 0;

const Ice_Object_ids__ = ["::Ice::Object"];

Ice.Object = class
{
    constructor()
    {
        // Fake Address used as the hashCode for this object instance.
        this.__address = nextAddress++;
    }

    hashCode()
    {
        return this.__address;
    }

    ice_isA(s, current)
    {
        return this.__mostDerivedType().__ids.indexOf(s) >= 0;
    }

    ice_ping(current)
    {
    }

    ice_ids(current)
    {
        return this.__mostDerivedType().__ids;
    }

    ice_id(current)
    {
        return this.__mostDerivedType().__id;
    }

    toString()
    {
        return "[object " + this.ice_id() + "]";
    }

    ice_preMarshal()
    {
    }

    ice_postUnmarshal()
    {
    }

    __write(os)
    {
        os.startValue(null);
        __writeImpl(this, os, this.__mostDerivedType());
        os.endValue();
    }

    __read(is)
    {
        is.startValue();
        __readImpl(this, is, this.__mostDerivedType());
        is.endValue(false);
    }

    ice_instanceof(T)
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
    }

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
    __mostDerivedType()
    {
        return Ice.Object;
    }

    //
    // The default implementation of equals compare references.
    // 
    equals(other)
    {
        return this === other;
    }
    
    //
    // These methods are used for object parameters.
    //
    static write(os, v)
    {
        os.writeValue(v);
    }

    static writeOptional(os, tag, v)
    {
        os.writeOptionalValue(tag, v);
    }

    static read(is)
    {
        const v = { value: null };
        is.readValue(o => v.value = o, this);
        return v;
    }

    static readOptional(is, tag)
    {
        const v = { value: undefined };
        is.readOptionalValue(tag, o => v.value = o, this);
        return v;
    }

    static ice_staticId()
    {
        return this.__id;
    }

    static __instanceof(T)
    {
        if(T === this)
        {
            return true;
        }

        if(this.__implements.some(i => i.__instanceof(T)))
        {
            return true;
        }

        if(this.__parent)
        {
            return this.__parent.__instanceof(T);
        }
        return false;
    }

    static get __ids()
    {
        return Ice_Object_ids__;
    }

    static get __id()
    {
        return Ice_Object_ids__[0];
    }

    static get __implements()
    {
        return [];
    }
};

//
// Private methods
//

const __writeImpl = function(obj, os, type)
{
    //
    // The __writeImpl method is a recursive method that goes down the
    // class hierarchy to marshal each slice of the class using the
    // generated __writeMemberImpl method.
    //

    if(type === undefined || type === Ice.Object)
    {
        return; // Don't marshal anything for Ice.Object
    }

    os.startSlice(type.__id, 
                  Object.prototype.hasOwnProperty.call(type, '__compactId') ? type.__compactId : -1 ,
                  type.__parent === Ice.Object);
    if(type.prototype.__writeMemberImpl)
    {
        type.prototype.__writeMemberImpl.call(obj, os);
    }
    os.endSlice();
    __writeImpl(obj, os, type.__parent);
};

const __readImpl = function(obj, is, type)
{
    //
    // The __readImpl method is a recursive method that goes down the
    // class hierarchy to marshal each slice of the class using the
    // generated __readMemberImpl method.
    //

    if(type === undefined || type === Ice.Object)
    {
        return; // Don't marshal anything for Ice.Object
    }

    is.startSlice();
    if(type.prototype.__readMemberImpl)
    {
        type.prototype.__readMemberImpl.call(obj, is);
    }
    is.endSlice();
    __readImpl(obj, is, type.__parent);
};

const __writePreserved = function(os)
{
    //
    // For Slice classes which are marked "preserved", the implementation of this method
    // replaces the Ice.Object.prototype.__write method.
    //
    os.startValue(this.__slicedData);
    __writeImpl(this, os, this.__mostDerivedType());
    os.endValue();
};

const __readPreserved = function(is)
{
    //
    // For Slice classes which are marked "preserved", the implementation of this method
    // replaces the Ice.Object.prototype.__read method.
    //
    is.startValue();
    __readImpl(this, is, this.__mostDerivedType());
    this.__slicedData = is.endValue(true);
};


const Slice = Ice.Slice;

Slice.PreservedObject = function(obj)
{
    obj.prototype.__write = __writePreserved;
    obj.prototype.__read = __readPreserved;
};

module.exports.Ice = Ice;
