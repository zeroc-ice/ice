// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/Promise");
    require("Ice/Class");
    require("Ice/ReferenceMode");
    
    var Ice = global.Ice || {};
    
    var Promise = Ice.Promise;
    var ReferenceMode = Ice.ReferenceMode;

    var ConnectionRequestHandler = Ice.Class({
        __init__: function(ref, connection, compress)
        {
            this._reference = ref;
            this._response = ref.getMode() == ReferenceMode.ModeTwoway;
            this._connection = connection;
            this._compress = compress;
        },
        prepareBatchRequest: function(out)
        {
            this._connection.prepareBatchRequest(out);
        },
        finishBatchRequest: function(out)
        {
            this._connection.finishBatchRequest(out, this._compress);
        },
        abortBatchRequest: function()
        {
            this._connection.abortBatchRequest();
        },
        sendAsyncRequest: function(out)
        {
            return this._connection.sendAsyncRequest(out, this._compress, this._response);
        },
        flushAsyncBatchRequests: function(out)
        {
            return this._connection.flushAsyncBatchRequests(out);
        },
        getReference: function()
        {
            return this._reference;
        },
        getConnection: function()
        {
            return this._connection;
        },
        onConnection: function(r)
        {
            r.succeed(this._connection, r);
        }
    });
    
    Ice.ConnectionRequestHandler = ConnectionRequestHandler;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
