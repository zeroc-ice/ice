//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/Debug");
require("../Ice/Protocol");
require("../Ice/Stream");

const OutputStream = Ice.OutputStream;
const Debug = Ice.Debug;
const Protocol = Ice.Protocol;

const udpOverhead = 20 + 8;

class BatchRequestQueue
{
    constructor(instance, datagram)
    {
        this._batchStreamInUse = false;
        this._batchRequestNum = 0;
        this._batchStream = new OutputStream(instance, Protocol.currentProtocolEncoding);
        this._batchStream.writeBlob(Protocol.requestBatchHdr);
        this._batchMarker = this._batchStream.size;
        this._exception = null;

        this._maxSize = instance.batchAutoFlushSize();
        if(this._maxSize > 0 && datagram)
        {
            const udpSndSize = instance.initializationData().properties.getPropertyAsIntWithDefault(
                "Ice.UDP.SndSize", 65535 - udpOverhead);
            if(udpSndSize < this._maxSize)
            {
                this._maxSize = udpSndSize;
            }
        }
    }

    prepareBatchRequest(os)
    {
        if(this._exception)
        {
            throw this._exception;
        }
        this._batchStream.swap(os);
    }

    finishBatchRequest(os, proxy, operation)
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
    }

    abortBatchRequest(os)
    {
        this._batchStream.swap(os);
        this._batchStream.resize(this._batchMarker);
    }

    swap(os)
    {
        if(this._batchRequestNum === 0)
        {
            return 0;
        }

        let lastRequest = null;
        if(this._batchMarker < this._batchStream.size)
        {
            const length = this._batchStream.size - this._batchMarker;
            this._batchStream.pos = this._batchMarker;
            lastRequest = this._batchStream.buffer.getArray(length);
            this._batchStream.resize(this._batchMarker);
        }

        const requestNum = this._batchRequestNum;
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
    }

    destroy(ex)
    {
        this._exception = ex;
    }

    isEmpty()
    {
        return this._batchStream.size === Protocol.requestBatchHdr.length;
    }
}

Ice.BatchRequestQueue = BatchRequestQueue;
module.exports.Ice = Ice;
