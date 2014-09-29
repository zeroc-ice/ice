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
    // connect : function()
    // {
    // This request handler is only created after connection binding.
    // }
    update: function(previousHandler, newHandler)
    {
        try
        {
            if(previousHandler === this)
            {
                return newHandler;
            }
            else if(previousHandler.getConnection() === this._connection)
            {
                //
                // If both request handlers point to the same connection, we also
                // update the request handler. See bug ICE-5489 for reasons why
                // this can be useful.
                //
                return newHandler;
            }
        }
        catch(ex)
        {
            // Ignore
        }
        return this;
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
