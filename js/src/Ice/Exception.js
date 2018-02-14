// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/Class").Ice;
var Class = Ice.Class;

var toString = function(key, object, objectTable, ident)
{
    ident += "  ";
    if(object === null)
    {
        return "\n" + ident + key + ": (null)";
    }
    if(object === undefined)
    {
        return "\n" + ident + key + ": (undefined)";
    }
    if(key == "stack" || typeof object == "function")
    {
        return "";
    }
    if(typeof object != "object")
    {
        return "\n" + ident + key + ": \"" + object + "\"";
    }
    if(objectTable.indexOf(object) != -1)
    {
        return "\n" + ident + key + ": (recursive)";
    }

    objectTable.push(object);
    var s = "\n" + ident + key + ":";
    for(var k in object)
    {
        if(key.indexOf("_") === 0)
        {
            continue;
        }

        if(typeof object[k] == "function")
        {
            continue;
        }
        s += ident + toString(k, object[k], objectTable, ident);
    }
    return s;
};

var ServantError = Class(Error, {
    __init__: function(cause)
    {
        this.cause = cause;
    }
});

Ice.ServantError = ServantError;

//
// Ice.Exception
//
var Exception = Class(Error, {
    __init__: function(cause)
    {
        if(cause)
        {
            this.ice_cause = cause;
        }
    },
    ice_name: function()
    {
        return "Ice::Exception";
    },
    toString: function()
    {
        //
        // We have a guard here to prevent being re-entered. With some browsers (IE), accessing
        // the stack property ends up calling toString on the exception to print it out with the
        // stack.
        //
        if(this._inToStringAlready)
        {
            return "";
        }

        this._inToStringAlready = true;
        var s = this.ice_name();
        for(var key in this)
        {
            if(key != "_inToStringAlready")
            {
                s += toString(key, this[key], [], "");
            }
        }

        if(Ice.__printStackTraces === true && this.stack)
        {
            s += "\n" + this.stack;
        }
        this._inToStringAlready = false;
        return s;
    }
});

Exception.captureStackTrace = function(object)
{
    var stack = new Error().stack;
    //
    // In IE 10 and greater the stack will be filled once the Error is throw
    // we don't need to do anything.
    //
    if(stack !== undefined)
    {
        Object.defineProperty(object, "stack", {
            get: function(){
                return stack;
            }
        });
    }
};

Ice.Exception = Exception;

//
// Ice.LocalException
//
var LocalException = Class(Exception, {
    __init__: function(cause)
    {
        Exception.call(this, cause);
        Exception.captureStackTrace(this);
    },
    ice_name: function()
    {
        return "Ice::LocalException";
    }
});

Ice.LocalException = LocalException;

var Slice = Ice.Slice;
Slice.defineLocalException = function(constructor, base, name)
{
    var ex = constructor;
    ex.prototype = new base();
    ex.prototype.constructor = ex;
    ex.prototype.ice_name = function()
    {
        return name;
    };
    return ex;
};

//
// Ice.UserException
//
var UserException = Class(Exception, {
    __init__: function(cause)
    {
        Exception.call(this, cause);
        Exception.captureStackTrace(this);
    },
    ice_name: function()
    {
        return "Ice::UserException";
    },
    __write: function(os)
    {
        os.startWriteException(null);
        __writeImpl(this, os, this.__mostDerivedType());
        os.endWriteException();
    },
    __read: function(is)
    {
        is.startReadException();
        __readImpl(this, is, this.__mostDerivedType());
        is.endReadException(false);
    },
    __usesClasses: function()
    {
        return false;
    }
});
Ice.UserException = UserException;

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

    if(type === undefined || type === UserException)
    {
        return; // Don't marshal anything for Ice.UserException
    }

    os.startWriteSlice(type.__id, -1, type.__parent === UserException);
    if(type.prototype.__writeMemberImpl)
    {
        type.prototype.__writeMemberImpl.call(obj, os);
    }
    os.endWriteSlice();
    __writeImpl(obj, os, type.__parent);
};

var __readImpl = function(obj, is, type)
{
    //
    // The __readImpl method is a recursive method that goes down the
    // class hierarchy to marshal each slice of the class using the
    // generated __readMemberImpl method.
    //

    if(type === undefined || type === UserException)
    {
        return; // Don't marshal anything for UserException
    }

    is.startReadSlice();
    if(type.prototype.__readMemberImpl)
    {
        type.prototype.__readMemberImpl.call(obj, is);
    }
    is.endReadSlice();
    __readImpl(obj, is, type.__parent);
};

var __writePreserved = function(os)
{
    //
    // For Slice exceptions which are marked "preserved", the implementation of this method
    // replaces the Ice.Object.prototype.__write method.
    //
    os.startWriteException(this.__slicedData);
    __writeImpl(this, os, this.__mostDerivedType());
    os.endWriteException();
};

var __readPreserved = function(is)
{
    //
    // For Slice exceptions which are marked "preserved", the implementation of this method
    // replaces the Ice.Object.prototype.__read method.
    //
    is.startReadException();
    __readImpl(this, is, this.__mostDerivedType());
    this.__slicedData = is.endReadException(true);
};

Slice.defineUserException = function(constructor, base, name, writeImpl, readImpl, preserved, usesClasses)
{
    var ex = constructor;
    ex.__parent = base;
    ex.prototype = new base();
    ex.__id = "::" + name;
    ex.prototype.ice_name = function()
    {
        return name;
    };

    ex.prototype.constructor = ex;
    ex.prototype.__mostDerivedType = function() { return ex; };
    if(preserved)
    {
        ex.prototype.__write = __writePreserved;
        ex.prototype.__read = __readPreserved;
    }
    ex.prototype.__writeMemberImpl = writeImpl;
    ex.prototype.__readMemberImpl = readImpl;

    if(usesClasses)
    {
        ex.prototype.__usesClasses = function()
        {
            return true;
        };
    }

    return ex;
};
module.exports.Ice = Ice;
