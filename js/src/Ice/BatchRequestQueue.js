// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/BasicStream",
        "../Ice/Debug",
        "../Ice/ExUtil",
        "../Ice/Protocol",
    ]);

var BasicStream = Ice.BasicStream;
var Debug = Ice.Debug;
var ExUtil = Ice.ExUtil;
var Class = Ice.Class;
var Protocol = Ice.Protocol;

var udpOverhead = 20 + 8;

var BatchRequestQueue = Class({
    __init__: function(instance, datagram)
    {
        this._batchStreamInUse = false;
        this._batchRequestNum = 0;
        this._batchStream = new BasicStream(instance, Protocol.currentProtocolEncoding);
        this._batchStream.writeBlob(Protocol.requestBatchHdr);
        this._batchMarker = this._batchStream.size;
        this._exception = null;

        this._maxSize = instance.batchAutoFlushSize();
        if(this._maxSize > 0 && datagram)
        {
            var props = instance.initializationData().properties;
            var udpSndSize = props.getPropertyAsIntWithDefault("Ice.UDP.SndSize", 65535 - udpOverhead);
            if(udpSndSize < this._maxSize)
            {
                this._maxSize = udpSndSize;
            }
        }
    },
    prepareBatchRequest: function(os)
    {
        if(this._exception)
        {
            throw this._exception;
        }
        this._batchStream.swap(os);
    },
    finishBatchRequest: function(os, proxy, operation)
    {
        //
        // No need for synchronization, no other threads are supposed
        // to modify the queue since we set this._batchStreamInUse to true.
        //
        this._batchStream.swap(os);

        try
        {
            if(this._maxSize > 0 && this._batchStream.size >= this._maxSize)
            {
                proxy.ice_flushBatchRequests(); // Auto flush
            }

            Debug.assert(this._batchMarker < this._batchStream.size);
            this._batchMarker = this._batchStream.size;
            ++this._batchRequestNum;
        }
        finally
        {
            this._batchStream.resize(this._batchMarker);
        }
    },
    abortBatchRequest: function(os)
    {
        this._batchStream.swap(os);
        this._batchStream.resize(this._batchMarker);
    },
    swap: function(os)
    {
        if(this._batchRequestNum === 0)
        {
            return 0;
        }

        var lastRequest = null;
        if(this._batchMarker < this._batchStream.size)
        {
            var length = this._batchStream.size - this._batchMarker;
            this._batchStream.pos = this._batchMarker;
            lastRequest = this._batchStream.buffer.getArray(length);
            this._batchStream.resize(this._batchMarker);
        }

        var requestNum = this._batchRequestNum;
        this._batchStream.swap(os);

        //
        // Reset the batch.
        //
        this._batchRequestNum = 0;
        this._batchStream.writeBlob(Protocol.requestBatchHdr);
        this._batchMarker = this._batchStream.size;
        if(lastRequest !== null)
        {
            this._batchStream.writeBlob(lastRequest);
        }
        return requestNum;
    },
    destroy: function(ex)
    {
        this._exception = ex;
    },
    isEmpty: function()
    {
        return this._batchStream.size === Protocol.requestBatchHdr.length;
    }
});

Ice.BatchRequestQueue = BatchRequestQueue;
module.exports.Ice = Ice;
