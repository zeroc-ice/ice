// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/Class", "../Ice/Exception"]);

class AssertionFailedException extends Error
{
    constructor(message)
    {
        super();
        Ice.Exception.captureStackTrace(this);
        this.message = message;
    }
}
Ice.AssertionFailedException = AssertionFailedException;

class Debug
{
    static assert(b, msg)
    {
        if(!b)
        {
            console.log(msg === undefined ? "assertion failed" : msg);
            console.log(Error().stack);
            throw new Ice.AssertionFailedException(msg === undefined ? "assertion failed" : msg);
        }
    }
}

Ice.Debug = Debug;
module.exports.Ice = Ice;
