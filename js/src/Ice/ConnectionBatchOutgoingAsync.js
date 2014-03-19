// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_FOR_ACTIONSCRIPT_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/Class");
    require("Ice/BatchOutgoingAsync");

    var Ice = global.Ice || {};

    var BatchOutgoingAsync = Ice.BatchOutgoingAsync;

    Ice.ConnectionBatchOutgoingAsync = Ice.Class(BatchOutgoingAsync, {
        __init__: function(con, communicator, operation)
        {
            BatchOutgoingAsync.call(this, communicator, operation);
            this._connection = con;
        },
        __send: function()
        {
            this._connection.flushAsyncBatchRequests(this);
        }
    });
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
