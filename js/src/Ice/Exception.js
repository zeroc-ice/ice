//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    for(const k in object)
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
        return this.constructor._id.substr(2);
    }

    ice_id()
    {
        return this.constructor._id;
    }

    static get _id()
    {
        return "::Ice::Exception";
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
        let s = this.ice_id();
        for(const key in this)
        {
            if(key != "_inToStringAlready")
            {
                s += toString(key, this[key], [], "");
            }
        }

        if(Ice._printStackTraces === true && this.stack)
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

    static get _id()
    {
        return "::Ice::LocalException";
    }
}

Ice.LocalException = LocalException;

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

    static get _id()
    {
        return "::Ice::UserException";
    }

    ice_getSlicedData()
    {
        return null;
    }

    _write(os)
    {
        os.startException(null);
        writeImpl(this, os, this._mostDerivedType());
        os.endException();
    }

    _read(is)
    {
        is.startException();
        readImpl(this, is, this._mostDerivedType());
        is.endException(false);
    }

    _usesClasses()
    {
        return false;
    }

    _mostDerivedType()
    {
        return Ice.UserException;
    }
}
Ice.UserException = UserException;

//
// Private methods
//

const writeImpl = function(obj, os, type)
{
    //
    // The writeImpl method is a recursive method that goes down the
    // class hierarchy to marshal each slice of the class using the
    // generated _writeMemberImpl method.
    //

    if(type === undefined || type === UserException)
    {
        return; // Don't marshal anything for Ice.UserException
    }

    os.startSlice(type._id, -1, type._parent === UserException);
    if(type.prototype.hasOwnProperty('_writeMemberImpl'))
    {
        type.prototype._writeMemberImpl.call(obj, os);
    }
    os.endSlice();
    writeImpl(obj, os, type._parent);
};

const readImpl = function(obj, is, type)
{
    //
    // The readImpl method is a recursive method that goes down the
    // class hierarchy to marshal each slice of the class using the
    // generated _readMemberImpl method.
    //

    if(type === undefined || type === UserException)
    {
        return; // Don't marshal anything for UserException
    }

    is.startSlice();
    if(type.prototype.hasOwnProperty('_readMemberImpl'))
    {
        type.prototype._readMemberImpl.call(obj, is);
    }
    is.endSlice();
    readImpl(obj, is, type._parent);
};

const writePreserved = function(os)
{
    //
    // For Slice exceptions which are marked "preserved", the implementation of this method
    // replaces the Ice.UserException.prototype._write method.
    //
    os.startException(this._slicedData);
    writeImpl(this, os, this._mostDerivedType());
    os.endException();
};

const readPreserved = function(is)
{
    //
    // For Slice exceptions which are marked "preserved", the implementation of this method
    // replaces the Ice.UserException.prototype._read method.
    //
    is.startException();
    readImpl(this, is, this._mostDerivedType());
    this._slicedData = is.endException(true);
};

const ice_getSlicedData = function()
{
    return this._slicedData;
};

Ice.Slice.PreservedUserException = function(ex)
{
    ex.prototype.ice_getSlicedData = ice_getSlicedData;
    ex.prototype._write = writePreserved;
    ex.prototype._read = readPreserved;
};

module.exports.Ice = Ice;
