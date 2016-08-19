// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;

const toString = function(key, object, objectTable, ident)
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
    let s = "\n" + ident + key + ":";
    for(let k in object)
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

//
// Ice.Exception
//
class Exception extends Error
{
    constructor(cause)
    {
        super();
        if(cause)
        {
            this.ice_cause = cause;
        }
    }

    ice_name()
    {
        return "Ice::Exception";
    }

    toString()
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
        let s = this.ice_name();
        for(let key in this)
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
    
    static captureStackTrace(object)
    {
        const stack = new Error().stack;
        //
        // In IE 10 and greater the stack will be filled once the Error is throw
        // we don't need to do anything.
        //
        if(stack !== undefined)
        {
            Object.defineProperty(object, "stack", {
                get: function()
                    {
                        return stack; 
                    }
            });
        }
    }
}

Ice.Exception = Exception;

//
// Ice.LocalException
//
class LocalException extends Exception
{
    constructor(cause)
    {
        super(cause);
        Exception.captureStackTrace(this);
    }

    ice_name()
    {
        return "Ice::LocalException";
    }
}

Ice.LocalException = LocalException;

const Slice = Ice.Slice;

//
// Ice.UserException
//
class UserException extends Exception
{
    constructor(cause)
    {
        super(cause);
        Exception.captureStackTrace(this);
    }

    ice_name()
    {
        return "Ice::UserException";
    }

    __write(os)
    {
        os.startException(null);
        __writeImpl(this, os, this.__mostDerivedType());
        os.endException();
    }

    __read(is)
    {
        is.startException();
        __readImpl(this, is, this.__mostDerivedType());
        is.endException(false);
    }

    __usesClasses()
    {
        return false;
    }

    __mostDerivedType()
    {
        return Ice.UserException;
    }
}
Ice.UserException = UserException;

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

    if(type === undefined || type === UserException)
    {
        return; // Don't marshal anything for Ice.UserException
    }

    os.startSlice(type.__id, -1, type.__parent === UserException);
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

    if(type === undefined || type === UserException)
    {
        return; // Don't marshal anything for UserException
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
    // For Slice exceptions which are marked "preserved", the implementation of this method
    // replaces the Ice.Object.prototype.__write method.
    //
    os.startException(this.__slicedData);
    __writeImpl(this, os, this.__mostDerivedType());
    os.endException();
};

const __readPreserved = function(is)
{
    //
    // For Slice exceptions which are marked "preserved", the implementation of this method
    // replaces the Ice.Object.prototype.__read method.
    //
    is.startException();
    __readImpl(this, is, this.__mostDerivedType());
    this.__slicedData = is.endException(true);
};

Slice.PreservedUserException = function(ex)
{
    ex.prototype.__write = __writePreserved;
    ex.prototype.__read = __readPreserved;
};

module.exports.Ice = Ice;
