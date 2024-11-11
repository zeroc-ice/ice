//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Protocol } from "./Protocol.js";
import { OutputStream } from "./OutputStream.js";
import { Debug } from "./Debug.js";

export class BatchRequestQueue {
    constructor(instance) {
        this._batchRequestNum = 0;
        this._batchStream = new OutputStream(
            Protocol.currentProtocolEncoding,
            instance.defaultsAndOverrides().defaultFormat,
        );
        this._batchStream.writeBlob(Protocol.requestBatchHdr);
        this._batchMarker = this._batchStream.size;
        this._exception = null;

        this._maxSize = instance.batchAutoFlushSize();
    }

    prepareBatchRequest(os) {
        if (this._exception) {
            throw this._exception;
        }
        this._batchStream.swap(os);
    }

    finishBatchRequest(os, proxy, operation) {
        this._batchStream.swap(os);

        try {
            if (this._maxSize > 0 && this._batchStream.size >= this._maxSize) {
                proxy.ice_flushBatchRequests(); // Auto flush
            }

            DEV: console.assert(this._batchMarker < this._batchStream.size);
            this._batchMarker = this._batchStream.size;
            ++this._batchRequestNum;
        } finally {
            this._batchStream.resize(this._batchMarker);
        }
    }

    abortBatchRequest(os) {
        this._batchStream.swap(os);
        this._batchStream.resize(this._batchMarker);
    }

    swap(os) {
        if (this._batchRequestNum === 0) {
            return 0;
        }

        let lastRequest = null;
        if (this._batchMarker < this._batchStream.size) {
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
        if (lastRequest !== null) {
            this._batchStream.writeBlob(lastRequest);
        }
        return requestNum;
    }

    destroy(ex) {
        this._exception = ex;
    }

    isEmpty() {
        return this._batchStream.size === Protocol.requestBatchHdr.length;
    }
}
