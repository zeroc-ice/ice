// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/Promise", "../Ice/Class", "../Ice/ReferenceMode"]);

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
        return out.__send(this._connection, this._compress, this._response);
    },
    asyncRequestTimedOut: function(out)
    {
        return this._connection.asyncRequestTimedOut(out);
    },
    getReference: function()
    {
        return this._reference;
    },
    getConnection: function()
    {
        return this._connection;
    },
});

Ice.ConnectionRequestHandler = ConnectionRequestHandler;
module.exports.Ice = Ice;
