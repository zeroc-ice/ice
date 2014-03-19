// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/Class");
    require("Ice/Exception");
    
    var Ice = global.Ice || {};
    
    var Exception = Ice.Exception;
    
    var AssertionFailedException = Ice.Class(Error, {
        __init__: function(message)
        {
            Error.call(this);
            Exception.captureStackTrace(this);
            this.message = message;
        }
    });
    var Debug = {};

    Debug.AssertionFailedException = AssertionFailedException;

    Debug.assert = function(b, msg)
    {
        if(!b)
        {
            console.log(msg === undefined ? "assertion failed" : msg);
            console.log(Error().stack);
            throw new AssertionFailedException(msg === undefined ? "assertion failed" : msg);
        }
    };
    Ice.Debug = Debug;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
