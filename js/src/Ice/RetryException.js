// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/Class", "../Ice/Exception", "../Ice/Debug", "../Ice/LocalException"]);

var RetryException = Ice.Class(Error, {
    __init__: function(ex)
    {
        if(ex instanceof Ice.LocalException)
        {
            this._ex = ex;
        }
        else
        {
            Ice.Debug.assert(ex instanceof RetryException);
            this._ex = ex._ex;
        }
    }
});

var prototype = RetryException.prototype;

Object.defineProperty(prototype, "inner", {
    get: function() { return this._ex; }
});

Ice.RetryException = RetryException;
module.exports.Ice = Ice;
