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
    require("Ice/AsyncResult");

    var Ice = global.Ice || {};

    var AsyncResult = Ice.AsyncResult;

    var BatchOutgoingAsync = Ice.Class(AsyncResult, {
        __init__: function(communicator, operation)
        {
            AsyncResult.call(this, communicator, operation, null, null, null, null);
        },
        __sent: function(connection)
        {
            this._state |= AsyncResult.Done | AsyncResult.OK | AsyncResult.Sent;
            this._os.resize(0);
            this.succeed(this);
        },
        __finishedEx: function(exc, sent)
        {
            this.__exception(exc);
        }
    });

    Ice.BatchOutgoingAsync = BatchOutgoingAsync;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
