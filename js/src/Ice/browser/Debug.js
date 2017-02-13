// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/Class", "../Ice/Exception"]);

Ice.AssertionFailedException = Ice.Class(Error, 
    {
        __init__: function(message)
        {
            Error.call(this);
            Ice.Exception.captureStackTrace(this);
            this.message = message;
        }
    });

Ice.Debug =
{
    assert: function(b, msg)
    {
        if(!b)
        {
            console.log(msg === undefined ? "assertion failed" : msg);
            console.log(Error().stack);
            throw new Ice.AssertionFailedException(msg === undefined ? "assertion failed" : msg);
        }
    }
};
module.exports.Ice = Ice;
