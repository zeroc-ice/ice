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
    require("Ice/ExUtil");
    require("Ice/Debug");
    require("Ice/LocalException");
    
    var Ice = global.Ice || {};
    
    var ExUtil = Ice.ExUtil;
    var Debug = Ice.Debug;

    var RetryException = Ice.Class(Error, {
        __init__: function(ex)
        {
            if(ex instanceof Ice.LocalException)
            {
                this._ex = ex;
            }
            else
            {
                Debug.assert(ex instanceof RetryException);
                this._ex = ex._ex;
            }
        }
    });
    
    var prototype = RetryException.prototype;
    
    Object.defineProperty(prototype, "inner", {
        get: function() { return this._ex; }
    });
    
    Ice.RetryException = RetryException;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
