//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { LocalException } from "./Exception.js";
import {
    IllegalMessageSizeException,
    ObjectAdapterDeactivatedException,
    CommunicatorDestroyedException,
    CloseConnectionException,
    ConnectionManuallyClosedException,
    ConnectionClosedException,
    ConnectTimeoutException,
    ConnectionIdleException,
    ConnectionLostException,
    CloseTimeoutException,
    SocketException,
    FeatureNotSupportedException,
    UnmarshalOutOfBoundsException,
    BadMagicException,
    ConnectionNotValidatedException,
    UnknownMessageException,
    UnknownException,
} from "./LocalException.js";

import { ConnectionClose } from "./Connection.js";

import { BatchRequestQueue } from "./BatchRequestQueue.js";
import { InputStream, OutputStream } from "./Stream.js";
import { Protocol } from "./Protocol.js";
import { Ice as Ice_Version } from "./Version.js";
const { ProtocolVersion, EncodingVersion } = Ice_Version;
import { throwMemoryLimitException } from "./ExUtil.js";
import { Timer } from "./Timer.js";
import { Promise } from "./Promise.js";
import { HashMap } from "./HashMap.js";
import { SocketOperation } from "./SocketOperation.js";
import { TraceUtil } from "./TraceUtil.js";
import { AsyncStatus } from "./AsyncStatus.js";
import { AsyncResultBase } from "./AsyncResultBase.js";
import { RetryException } from "./RetryException.js";
import { ConnectionFlushBatch, OutgoingAsync } from "./OutgoingAsync.js";
import { IncomingAsync } from "./IncomingAsync.js";
import { Debug } from "./Debug.js";
import { IdleTimeoutTransceiverDecorator } from "./IdleTimeoutTransceiverDecorator.js";
import { ObjectAdapter } from "./ObjectAdapter.js";
import { ObjectPrx } from "./ObjectPrx.js";

const StateNotInitialized = 0;
const StateNotValidated = 1;
const StateActive = 2;
const StateHolding = 3;
const StateClosing = 4;
const StateClosed = 5;
const StateFinished = 6;

class MessageInfo {
    constructor(instance) {
        this.stream = new InputStream(instance, Protocol.currentProtocolEncoding);
        this.invokeNum = 0;
        this.requestId = 0;
        this.servantManager = null;
        this.adapter = null;
        this.outAsync = null;
    }
}

export class ConnectionI {
    constructor(communicator, instance, transceiver, endpoint, removeFromFactory, options) {
        this._communicator = communicator;
        this._instance = instance;
        this._desc = transceiver.toString();
        this._type = transceiver.type();
        this._endpoint = endpoint;
        this._adapter = null;
        this._removeFromFactory = removeFromFactory;

        this._connectTimeout = options.connectTimeout * 1000; // Seconds to milliseconds
        this._connectTimeoutId = undefined;

        this._closeTimeout = options.closeTimeout * 1000; // Seconds to milliseconds.
        this._closeTimeoutId = undefined;

        this._inactivityTimeout = options.inactivityTimeout;
        this._inactivityTimer = undefined;

        const initData = instance.initializationData();
        this._logger = initData.logger; // Cached for better performance.
        this._traceLevels = instance.traceLevels(); // Cached for better performance.
        this._timer = instance.timer();

        this._hasMoreData = { value: false };

        this._warn = initData.properties.getPropertyAsInt("Ice.Warn.Connections") > 0;
        this._nextRequestId = 1;
        this._messageSizeMax = instance.messageSizeMax();
        this._batchRequestQueue = new BatchRequestQueue(instance);

        this._sendStreams = [];

        this._readStream = new InputStream(instance, Protocol.currentProtocolEncoding);
        this._readHeader = false;
        this._writeStream = new OutputStream(instance, Protocol.currentProtocolEncoding);

        this._readStreamPos = -1;
        this._writeStreamPos = -1;

        // The number of user calls currently executed by the event-loop (servant dispatch, invocation response, etc.).
        this._upcallCount = 0;

        // The number of outstanding dispatches. Maintained only while state is StateActive or StateHolding.
        this._dispatchCount = 0;

        this._state = StateNotInitialized;
        this._shutdownInitiated = false;
        this._initialized = false;
        this._validated = false;

        this._readProtocol = new ProtocolVersion();
        this._readProtocolEncoding = new EncodingVersion();

        this._asyncRequests = new HashMap(); // Map<int, OutgoingAsync>

        this._exception = null;

        this._startPromise = null;
        this._closePromises = [];
        this._finishedPromises = [];

        if (options.idleTimeout > 0) {
            transceiver = new IdleTimeoutTransceiverDecorator(
                transceiver,
                this,
                this._timer,
                options.idleTimeout,
                options.enableIdleCheck,
            );
        }
        this._transceiver = transceiver;

        this._servantManager = null;
        this._closeCallback = null;
    }

    start() {
        Debug.assert(this._startPromise === null);

        try {
            // The connection might already be closed if the communicator was destroyed.
            if (this._state >= StateClosed) {
                Debug.assert(this._exception !== null);
                return Promise.reject(this._exception);
            }

            this._startPromise = new Promise();
            this._transceiver.setCallbacks(
                () => this.message(SocketOperation.Write), // connected callback
                () => this.message(SocketOperation.Read), // read callback
                () => this.message(SocketOperation.Write), // write callback
            );

            if (!this.initialize()) {
                if (this._connectTimeout > 0) {
                    this._connectTimeoutId = this._timer.schedule(() => {
                        this.connectTimedOut();
                    }, this._connectTimeout);
                }
            }
        } catch (ex) {
            const startPromise = this._startPromise;
            this.exception(ex);
            return startPromise;
        }
        return this._startPromise;
    }

    activate() {
        if (this._state <= StateNotValidated) {
            return;
        }
        this.setState(StateActive);
    }

    hold() {
        if (this._state <= StateNotValidated) {
            return;
        }

        this.setState(StateHolding);
    }

    destroy(reason) {
        switch (reason) {
            case ConnectionI.ObjectAdapterDeactivated: {
                this.setState(StateClosing, new ObjectAdapterDeactivatedException());
                break;
            }

            case ConnectionI.CommunicatorDestroyed: {
                this.setState(StateClosing, new CommunicatorDestroyedException());
                break;
            }

            default: {
                Debug.assert(false);
                break;
            }
        }
    }

    close(mode) {
        const r = new AsyncResultBase(this._communicator, "close", this, null, null);

        if (mode == ConnectionClose.Forcefully) {
            this.setState(StateClosed, new ConnectionManuallyClosedException(false));
            r.resolve();
        } else if (mode == ConnectionClose.Gracefully) {
            this.setState(StateClosing, new ConnectionManuallyClosedException(true));
            r.resolve();
        } else {
            Debug.assert(mode == ConnectionClose.GracefullyWithWait);

            //
            // Wait until all outstanding requests have been completed.
            //
            this._closePromises.push(r);
            this.checkClose();
        }

        return r;
    }

    checkClose() {
        //
        // If close(GracefullyWithWait) has been called, then we need to check if all
        // requests have completed and we can transition to StateClosing. We also
        // complete outstanding promises.
        //
        if (this._asyncRequests.size === 0 && this._closePromises.length > 0) {
            //
            // The caller doesn't expect the state of the connection to change when this is called so we queue the
            // check in the event loop an return control to the caller. This is consistent with other implementations
            // as well.
            //
            Timer.setImmediate(() => {
                this.setState(StateClosing, new ConnectionManuallyClosedException(true));
                this._closePromises.forEach(p => p.resolve());
                this._closePromises = [];
            });
        }
    }

    isActiveOrHolding() {
        return this._state > StateNotValidated && this._state < StateClosing;
    }

    isFinished() {
        if (this._state !== StateFinished || this._upcallCount !== 0) {
            return false;
        }

        Debug.assert(this._state === StateFinished);
        return true;
    }

    throwException() {
        if (this._exception !== null) {
            Debug.assert(this._state >= StateClosing);
            throw this._exception;
        }
    }

    waitUntilFinished() {
        const promise = new Promise();
        this._finishedPromises.push(promise);
        this.checkState();
        return promise;
    }

    sendAsyncRequest(out, response, batchRequestNum) {
        let requestId = 0;
        const ostr = out.getOs();

        if (this._exception !== null) {
            //
            // If the connection is closed before we even have a chance
            // to send our request, we always try to send the request
            // again.
            //
            throw new RetryException(this._exception);
        }

        Debug.assert(this._state > StateNotValidated);
        Debug.assert(this._state < StateClosing);

        //
        // Notify the request that it's cancelable with this connection.
        // This will throw if the request is canceled.
        //
        out.cancelable(this); // Notify the request that it's cancelable

        if (response) {
            //
            // Create a new unique request ID.
            //
            requestId = this._nextRequestId++;
            if (requestId <= 0) {
                this._nextRequestId = 1;
                requestId = this._nextRequestId++;
            }

            //
            // Fill in the request ID.
            //
            ostr.pos = Protocol.headerSize;
            ostr.writeInt(requestId);
        } else if (batchRequestNum > 0) {
            ostr.pos = Protocol.headerSize;
            ostr.writeInt(batchRequestNum);
        }

        // We're just about to send a request, so we are not inactive anymore.
        this.cancelInactivityTimer();

        let status;
        try {
            status = this.sendMessage(OutgoingMessage.create(out, out.getOs(), requestId));
        } catch (ex) {
            if (ex instanceof LocalException) {
                this.setState(StateClosed, ex);
                Debug.assert(this._exception !== null);
                throw this._exception;
            } else {
                throw ex;
            }
        }

        if (response) {
            //
            // Add to the async requests map.
            //
            this._asyncRequests.set(requestId, out);
        }

        return status;
    }

    getBatchRequestQueue() {
        return this._batchRequestQueue;
    }

    flushBatchRequests() {
        const result = new ConnectionFlushBatch(this, this._communicator, "flushBatchRequests");
        result.invoke();
        return result;
    }

    setCloseCallback(callback) {
        if (this._state >= StateClosed) {
            if (callback !== null) {
                Timer.setImmediate(() => {
                    try {
                        callback(this);
                    } catch (ex) {
                        this._logger.error("connection callback exception:\n" + ex + "\n" + this._desc);
                    }
                });
            }
        } else {
            this._closeCallback = callback;
        }
    }

    asyncRequestCanceled(outAsync, ex) {
        for (let i = 0; i < this._sendStreams.length; i++) {
            const o = this._sendStreams[i];
            if (o.outAsync === outAsync) {
                if (o.requestId > 0) {
                    this._asyncRequests.delete(o.requestId);
                }

                //
                // If the request is being sent, don't remove it from the send streams,
                // it will be removed once the sending is finished.
                //
                o.canceled();
                if (i !== 0) {
                    this._sendStreams.splice(i, 1);
                }
                outAsync.completedEx(ex);
                this.checkClose();
                return; // We're done.
            }
        }

        if (outAsync instanceof OutgoingAsync) {
            for (const [key, value] of this._asyncRequests) {
                if (value === outAsync) {
                    this._asyncRequests.delete(key);
                    outAsync.completedEx(ex);
                    this.checkClose();
                    return; // We're done.
                }
            }
        }
    }

    sendResponse(os) {
        Debug.assert(this._state > StateNotValidated);

        try {
            if (--this._upcallCount === 0) {
                if (this._state === StateFinished) {
                    this._removeFromFactory(this);
                }
                this.checkState();
            }

            if (this._state >= StateClosed) {
                Debug.assert(this._exception !== null);
                throw this._exception;
            }

            this.sendMessage(OutgoingMessage.createForStream(os, true));

            --this._dispatchCount;

            if (this._state === StateClosing && this._upcallCount === 0) {
                this.initiateShutdown();
            }
        } catch (ex) {
            if (ex instanceof LocalException) {
                this.setState(StateClosed, ex);
            } else {
                throw ex;
            }
        }
    }

    sendNoResponse() {
        Debug.assert(this._state > StateNotValidated);
        try {
            if (--this._upcallCount === 0) {
                if (this._state === StateFinished) {
                    this._removeFromFactory(this);
                }
                this.checkState();
            }

            if (this._state >= StateClosed) {
                Debug.assert(this._exception !== null);
                throw this._exception;
            }

            --this._dispatchCount;

            if (this._state === StateClosing && this._upcallCount === 0) {
                this.initiateShutdown();
            }
        } catch (ex) {
            if (ex instanceof LocalException) {
                this.setState(StateClosed, ex);
            } else {
                throw ex;
            }
        }
    }

    endpoint() {
        return this._endpoint;
    }

    setAdapter(adapter) {
        if (adapter !== null) {
            adapter.checkForDeactivation();
            if (this._state <= StateNotValidated || this._state >= StateClosing) {
                return;
            }
            this._adapter = adapter;
            this._servantManager = adapter.getServantManager(); // The ObjectAdapter's servant manager is immutable.
        } else {
            if (this._state <= StateNotValidated || this._state >= StateClosing) {
                return;
            }
            this._adapter = null;
            this._servantManager = null;
        }
    }

    getAdapter() {
        return this._adapter;
    }

    getEndpoint() {
        return this._endpoint;
    }

    createProxy(ident) {
        ObjectAdapter.checkIdentity(ident);
        return new ObjectPrx(this._instance.referenceFactory().createFixed(ident, this));
    }

    message(operation) {
        if (this._state >= StateClosed) {
            return;
        }

        //
        // Keep reading until no more data is available.
        //
        this._hasMoreData.value = (operation & SocketOperation.Read) !== 0;

        let info = null;
        try {
            if ((operation & SocketOperation.Write) !== 0 && this._writeStream.buffer.remaining > 0) {
                if (!this.write(this._writeStream.buffer)) {
                    Debug.assert(!this._writeStream.isEmpty());
                    return;
                }
                Debug.assert(this._writeStream.buffer.remaining === 0);
            }
            if ((operation & SocketOperation.Read) !== 0 && !this._readStream.isEmpty()) {
                if (this._readHeader) {
                    // Read header if necessary.
                    if (!this.read(this._readStream.buffer)) {
                        //
                        // We didn't get enough data to complete the header.
                        //
                        return;
                    }

                    Debug.assert(this._readStream.buffer.remaining === 0);
                    this._readHeader = false;

                    //
                    // Connection is validated on first message. This is only used by
                    // setState() to check wether or not we can print a connection
                    // warning (a client might close the connection forcefully if the
                    // connection isn't validated, we don't want to print a warning
                    // in this case).
                    //
                    this._validated = true;

                    const pos = this._readStream.pos;
                    Debug.assert(pos >= Protocol.headerSize);

                    this._readStream.pos = 0;
                    const magic0 = this._readStream.readByte();
                    const magic1 = this._readStream.readByte();
                    const magic2 = this._readStream.readByte();
                    const magic3 = this._readStream.readByte();
                    if (
                        magic0 !== Protocol.magic[0] ||
                        magic1 !== Protocol.magic[1] ||
                        magic2 !== Protocol.magic[2] ||
                        magic3 !== Protocol.magic[3]
                    ) {
                        throw new BadMagicException("", new Uint8Array([magic0, magic1, magic2, magic3]));
                    }

                    this._readProtocol._read(this._readStream);
                    Protocol.checkSupportedProtocol(this._readProtocol);

                    this._readProtocolEncoding._read(this._readStream);
                    Protocol.checkSupportedProtocolEncoding(this._readProtocolEncoding);

                    this._readStream.readByte(); // messageType
                    this._readStream.readByte(); // compress
                    const size = this._readStream.readInt();
                    if (size < Protocol.headerSize) {
                        throw new IllegalMessageSizeException();
                    }

                    if (size > this._messageSizeMax) {
                        throwMemoryLimitException(size, this._messageSizeMax);
                    }
                    if (size > this._readStream.size) {
                        this._readStream.resize(size);
                    }
                    this._readStream.pos = pos;
                }

                if (this._readStream.pos != this._readStream.size) {
                    if (!this.read(this._readStream.buffer)) {
                        Debug.assert(!this._readStream.isEmpty());
                        return;
                    }
                    Debug.assert(this._readStream.buffer.remaining === 0);
                }
            }

            if (this._state <= StateNotValidated) {
                if (this._state === StateNotInitialized && !this.initialize()) {
                    return;
                }

                if (this._state <= StateNotValidated && !this.validate()) {
                    return;
                }

                this._transceiver.unregister();

                //
                // We start out in holding state.
                //
                this.setState(StateHolding);
                if (this._startPromise !== null) {
                    ++this._upcallCount;
                }
            } else {
                Debug.assert(this._state <= StateClosing);

                //
                // We parse messages first, if we receive a close
                // connection message we won't send more messages.
                //
                if ((operation & SocketOperation.Read) !== 0) {
                    info = this.parseMessage();
                }

                if ((operation & SocketOperation.Write) !== 0) {
                    this.sendNextMessage();
                }
            }
        } catch (ex) {
            if (ex instanceof SocketException) {
                this.setState(StateClosed, ex);
                return;
            } else if (ex instanceof LocalException) {
                this.setState(StateClosed, ex);
                return;
            } else {
                throw ex;
            }
        }

        this.dispatch(info);

        if (this._hasMoreData.value) {
            Timer.setImmediate(() => this.message(SocketOperation.Read)); // Don't tie up the thread.
        }
    }

    dispatch(info) {
        let count = 0;
        //
        // Notify the factory that the connection establishment and
        // validation has completed.
        //
        if (this._startPromise !== null) {
            this._startPromise.resolve();

            this._startPromise = null;
            ++count;
        }

        if (info !== null) {
            if (info.outAsync !== null) {
                info.outAsync.completed(info.stream);
                ++count;
            }

            if (info.invokeNum > 0) {
                this.invokeAll(info.stream, info.invokeNum, info.requestId, info.servantManager, info.adapter);

                //
                // Don't increase count, the dispatch count is
                // decreased when the incoming reply is sent.
                //
            }
        }

        //
        // Decrease the upcall count.
        //
        if (count > 0) {
            this._upcallCount -= count;
            if (this._upcallCount === 0) {
                if (this._state === StateClosing) {
                    try {
                        this.initiateShutdown();
                    } catch (ex) {
                        if (ex instanceof LocalException) {
                            this.setState(StateClosed, ex);
                        } else {
                            throw ex;
                        }
                    }
                } else if (this._state === StateFinished) {
                    this._removeFromFactory(this);
                }
                this.checkState();
            }
        }
    }

    finish() {
        Debug.assert(this._state === StateClosed);

        // Cancel the timers to ensure they don't keep the event loop alive.
        if (this._connectTimeoutId !== undefined) {
            this._timer.cancel(this._connectTimeoutId);
        }

        if (this._closeTimeoutId !== undefined) {
            this._timer.cancel(this._closeTimeoutId);
        }

        const traceLevels = this._traceLevels;
        if (!this._initialized) {
            if (traceLevels.network >= 2) {
                const s = [];
                s.push("failed to establish ");
                s.push(this._endpoint.protocol());
                s.push(" connection\n");
                s.push(this.toString());
                s.push("\n");
                s.push(this._exception.toString());
                this._logger.trace(traceLevels.networkCat, s.join(""));
            }
        } else if (traceLevels.network >= 1) {
            const s = [];
            s.push("closed ");
            s.push(this._endpoint.protocol());
            s.push(" connection\n");
            s.push(this.toString());

            //
            // Trace the cause of unexpected connection closures
            //
            if (
                !(
                    this._exception instanceof CloseConnectionException ||
                    this._exception instanceof ConnectionManuallyClosedException ||
                    this._exception instanceof ConnectionClosedException ||
                    this._exception instanceof ConnectionIdleException ||
                    this._exception instanceof CommunicatorDestroyedException ||
                    this._exception instanceof ObjectAdapterDeactivatedException
                )
            ) {
                s.push("\n");
                s.push(this._exception.toString());
            }

            this._logger.trace(traceLevels.networkCat, s.join(""));
        }

        if (this._startPromise !== null) {
            this._startPromise.reject(this._exception);
            this._startPromise = null;
        }

        if (this._sendStreams.length > 0) {
            if (!this._writeStream.isEmpty()) {
                //
                // Return the stream to the outgoing call. This is important for
                // retriable AMI calls which are not marshaled again.
                //
                this._writeStream.swap(this._sendStreams[0].stream);
            }

            //
            // NOTE: for twoway requests which are not sent, finished can be called twice: the
            // first time because the outgoing is in the _sendStreams set and the second time
            // because it's either in the _requests/_asyncRequests set. This is fine, only the
            // first call should be taken into account by the implementation of finished.
            //
            for (let i = 0; i < this._sendStreams.length; ++i) {
                const p = this._sendStreams[i];
                if (p.requestId > 0) {
                    this._asyncRequests.delete(p.requestId);
                }
                p.completed(this._exception);
            }
            this._sendStreams = [];
        }

        for (const value of this._asyncRequests.values()) {
            value.completedEx(this._exception);
        }
        this._asyncRequests.clear();
        this.checkClose();

        //
        // Don't wait to be reaped to reclaim memory allocated by read/write streams.
        //
        this._readStream.clear();
        this._readStream.buffer.clear();
        this._writeStream.clear();
        this._writeStream.buffer.clear();

        if (this._closeCallback !== null) {
            try {
                this._closeCallback(this);
            } catch (ex) {
                this._logger.error("connection callback exception:\n" + ex + "\n" + this._desc);
            }
            this._closeCallback = null;
        }

        //
        // This must be done last as this will cause waitUntilFinished() to return (and communicator
        // objects such as the timer might be destroyed too).
        //
        if (this._upcallCount === 0) {
            this._removeFromFactory(this);
        }
        this.setState(StateFinished);
    }

    toString() {
        return this._desc;
    }

    type() {
        return this._type;
    }

    getInfo() {
        if (this._state >= StateClosed) {
            throw this._exception;
        }
        const info = this._transceiver.getInfo();
        for (let p = info; p !== null; p = p.underlying) {
            p.adapterName = this._adapter !== null ? this._adapter.getName() : "";
            p.incoming = false;
        }
        return info;
    }

    setBufferSize(rcvSize, sndSize) {
        if (this._state >= StateClosed) {
            throw this._exception;
        }
        this._transceiver.setBufferSize(rcvSize, sndSize);
    }

    exception(ex) {
        this.setState(StateClosed, ex);
    }

    dispatchException(ex, invokeNum) {
        //
        // Fatal exception while invoking a request. Since sendResponse/sendNoResponse isn't
        // called in case of a fatal exception we decrement this._upcallCount here.
        //

        this.setState(StateClosed, ex);

        if (invokeNum > 0) {
            Debug.assert(this._upcallCount > 0);
            this._upcallCount -= invokeNum;
            Debug.assert(this._upcallCount >= 0);
            if (this._upcallCount === 0) {
                if (this._state === StateFinished) {
                    this._removeFromFactory(this);
                }
                this.checkState();
            }
        }
    }

    inactivityCheck(inactivityTimer) {
        // If the timers are different, it means this inactivityTimer is no longer current.
        if (inactivityTimer == this._inactivityTimer) {
            this._inactivityTimer = undefined;
            inactivityTimer.destroy();

            if (this._state == StateActive) {
                this.setState(
                    StateClosing,
                    new ConnectionClosedException(
                        "connection closed because it remained inactive for longer than the inactivity timeout",
                    ),
                );
            }
        }
        // Else this timer was already canceled and disposed. Nothing to do.
    }

    setState(state, ex) {
        if (ex !== undefined) {
            Debug.assert(ex instanceof LocalException, ex);

            //
            // If setState() is called with an exception, then only closed
            // and closing states are permissible.
            //
            Debug.assert(state >= StateClosing);

            if (this._state === state) {
                // Don't switch twice.
                return;
            }

            if (this._exception === null) {
                this._exception = ex;

                //
                // We don't warn if we are not validated.
                //
                if (this._warn && this._validated) {
                    //
                    // Don't warn about certain expected exceptions.
                    //
                    if (
                        !(
                            this._exception instanceof CloseConnectionException ||
                            this._exception instanceof ConnectionManuallyClosedException ||
                            this._exception instanceof ConnectionClosedException ||
                            this._exception instanceof ConnectionIdleException ||
                            this._exception instanceof CommunicatorDestroyedException ||
                            this._exception instanceof ObjectAdapterDeactivatedException ||
                            (this._exception instanceof ConnectionLostException && this._state === StateClosing)
                        )
                    ) {
                        this.warning("connection exception", this._exception);
                    }
                }
            }

            //
            // We must set the new state before we notify requests of any
            // exceptions. Otherwise new requests may retry on a
            // connection that is not yet marked as closed or closing.
            //
        }

        //
        // Skip graceful shutdown if we are destroyed before validation.
        //
        if (this._state <= StateNotValidated && state === StateClosing) {
            state = StateClosed;
        }

        if (this._state === state) {
            // Don't switch twice.
            return;
        }

        if (state > StateActive) {
            // Cancel the inactivity timer, if not null.
            this.cancelInactivityTimer();
        }

        try {
            switch (state) {
                case StateNotInitialized: {
                    Debug.assert(false);
                    break;
                }

                case StateNotValidated: {
                    if (this._state !== StateNotInitialized) {
                        Debug.assert(this._state === StateClosed);
                        return;
                    }
                    //
                    // Register to receive validation message.
                    //
                    // Once validation is complete, a new connection starts out in the
                    // Holding state. We only want to register the transceiver now if we
                    // need to receive data in order to validate the connection.
                    //
                    this._transceiver.register();
                    break;
                }

                case StateActive: {
                    //
                    // Can only switch from holding or not validated to
                    // active.
                    //
                    if (this._state !== StateHolding && this._state !== StateNotValidated) {
                        return;
                    }
                    this._transceiver.register();
                    break;
                }

                case StateHolding: {
                    //
                    // Can only switch from active or not validated to
                    // holding.
                    //
                    if (this._state !== StateActive && this._state !== StateNotValidated) {
                        return;
                    }
                    if (this._state === StateActive) {
                        this._transceiver.unregister();
                    }
                    break;
                }

                case StateClosing: {
                    //
                    // Can't change back from closed.
                    //
                    if (this._state >= StateClosed) {
                        return;
                    }
                    if (this._state === StateHolding) {
                        // We need to continue to read in closing state.
                        this._transceiver.register();
                    }
                    break;
                }

                case StateClosed: {
                    if (this._state === StateFinished) {
                        return;
                    }
                    this._batchRequestQueue.destroy(this._exception);
                    this._transceiver.unregister();
                    break;
                }

                case StateFinished: {
                    Debug.assert(this._state === StateClosed);
                    this._transceiver.close();
                    this._communicator = null;
                    break;
                }

                default: {
                    Debug.assert(false);
                    break;
                }
            }
        } catch (ex) {
            if (ex instanceof LocalException) {
                this._logger.error(`unexpected connection exception:\n${this._desc}\n${ex.toString()}`);
            } else {
                throw ex;
            }
        }

        this._state = state;

        if (this._state === StateClosing && this._upcallCount === 0) {
            try {
                this.initiateShutdown();
            } catch (ex) {
                if (ex instanceof LocalException) {
                    this.setState(StateClosed, ex);
                } else {
                    throw ex;
                }
            }
        } else if (this._state === StateClosed) {
            this.finish();
        }

        this.checkState();
    }

    initiateShutdown() {
        Debug.assert(this._state === StateClosing && this._upcallCount === 0);

        if (this._shutdownInitiated) {
            return;
        }
        this._shutdownInitiated = true;

        //
        // Before we shut down, we send a close connection message.
        //
        const os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
        os.writeBlob(Protocol.magic);
        Protocol.currentProtocol._write(os);
        Protocol.currentProtocolEncoding._write(os);
        os.writeByte(Protocol.closeConnectionMsg);
        os.writeByte(0); // compression status: always report 0 for CloseConnection.
        os.writeInt(Protocol.headerSize); // Message size.

        if (this._closeTimeout > 0) {
            // Schedules a one-time check.
            this._closeTimeoutId = this._timer.schedule(() => this.closeTimedOut(), this._closeTimeout);
        }
        this.sendMessage(OutgoingMessage.createForStream(os, false));
    }

    idleCheck(idleTimeout) {
        if (this._state == StateActive || this._state == StateHolding) {
            if (this._traceLevels.network >= 1) {
                this._logger.trace(
                    this._traceLevels.networkCat,
                    `connection aborted by the idle check because it did not receive any bytes for ${idleTimeout}s\n${this._transceiver.toString()}`,
                );
            }
            this.setState(StateClosed, new ConnectionIdleException());
        }
        // else nothing to do
    }

    sendHeartbeat() {
        if (this._state == StateActive || this._state == StateHolding) {
            // We check if the connection has become inactive.
            if (
                this._inactivityTimer === undefined && // timer not already scheduled
                this._inactivityTimeout > 0 && // inactivity timeout is enabled
                this._state == StateActive && // only schedule the timer if the connection is active
                this._dispatchCount == 0 && // no pending dispatch
                this._asyncRequests.size == 0 && // no pending invocation
                this._readHeader && // we're not waiting for the remainder of an incoming message
                this._sendStreams.length <= 1 // there is at most one pending outgoing message
            ) {
                // We may become inactive while the peer is back-pressuring us. In this case, we only schedule the
                // inactivity timer if there is no pending outgoing message or the pending outgoing message is a
                // heartbeat.

                // The stream of the first _sendStreams message is in _writeStream.
                if (
                    this._sendStreams.length == 0 ||
                    this._writeStream.buffer.getAt(8) == Protocol.validateConnectionMsg
                ) {
                    this.scheduleInactivityTimer();
                }
            }

            // We send a heartbeat to the peer to generate a "write" on the connection. This write in turns creates
            // a read on the peer, and resets the peer's idle check timer. When _sendStream is not empty, there is
            // already an outstanding write, so we don't need to send a heartbeat. It's possible the first message
            // of _sendStreams was already sent but not yet removed from _sendStreams: it means the last write
            // occurred very recently, which is good enough with respect to the idle check.
            // As a result of this optimization, the only possible heartbeat in _sendStreams is the first
            // _sendStreams message.
            if (this._sendStreams.length == 0) {
                const os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
                os.writeBlob(Protocol.magic);
                Protocol.currentProtocol._write(os);
                Protocol.currentProtocolEncoding._write(os);
                os.writeByte(Protocol.validateConnectionMsg);
                os.writeByte(0);
                os.writeInt(Protocol.headerSize); // Message size.
                try {
                    this.sendMessage(OutgoingMessage.createForStream(os, false));
                } catch (ex) {
                    this.setState(StateClosed, ex);
                }
            }
        }
    }

    initialize() {
        const s = this._transceiver.initialize(this._readStream.buffer, this._writeStream.buffer);
        if (s != SocketOperation.None) {
            return false;
        }

        //
        // Update the connection description once the transceiver is initialized.
        //
        this._desc = this._transceiver.toString();
        this._initialized = true;
        this.setState(StateNotValidated);
        return true;
    }

    validate() {
        if (this._readStream.size === 0) {
            this._readStream.resize(Protocol.headerSize);
            this._readStream.pos = 0;
        }

        if (this._readStream.pos !== this._readStream.size && !this.read(this._readStream.buffer)) {
            return false;
        }

        this._validated = true;

        Debug.assert(this._readStream.pos === Protocol.headerSize);
        this._readStream.pos = 0;
        const m = this._readStream.readBlob(4);
        if (
            m[0] !== Protocol.magic[0] ||
            m[1] !== Protocol.magic[1] ||
            m[2] !== Protocol.magic[2] ||
            m[3] !== Protocol.magic[3]
        ) {
            throw new BadMagicException("", m);
        }

        this._readProtocol._read(this._readStream);
        Protocol.checkSupportedProtocol(this._readProtocol);

        this._readProtocolEncoding._read(this._readStream);
        Protocol.checkSupportedProtocolEncoding(this._readProtocolEncoding);

        const messageType = this._readStream.readByte();
        if (messageType !== Protocol.validateConnectionMsg) {
            throw new ConnectionNotValidatedException();
        }
        this._readStream.readByte(); // Ignore compression status for validate connection.
        if (this._readStream.readInt() !== Protocol.headerSize) {
            throw new IllegalMessageSizeException();
        }
        TraceUtil.traceRecv(this._readStream, this._logger, this._traceLevels);

        this._writeStream.resize(0);
        this._writeStream.pos = 0;

        this._readStream.resize(Protocol.headerSize);
        this._readHeader = true;
        this._readStream.pos = 0;

        const traceLevels = this._traceLevels;
        if (traceLevels.network >= 1) {
            const s = [];
            s.push("established ");
            s.push(this._endpoint.protocol());
            s.push(" connection\n");
            s.push(this.toString());
            this._logger.trace(traceLevels.networkCat, s.join(""));
        }

        return true;
    }

    sendNextMessage() {
        if (this._sendStreams.length === 0) {
            return;
        }

        Debug.assert(!this._writeStream.isEmpty() && this._writeStream.pos === this._writeStream.size);
        try {
            while (true) {
                //
                // Notify the message that it was sent.
                //
                let message = this._sendStreams.shift();
                this._writeStream.swap(message.stream);
                message.sent();

                //
                // If there's nothing left to send, we're done.
                //
                if (this._sendStreams.length === 0) {
                    break;
                }

                //
                // If we are in the closed state, don't continue sending.
                //
                // The connection can be in the closed state if parseMessage
                // (called before sendNextMessage by message()) closes the
                // connection.
                //
                if (this._state >= StateClosed) {
                    return;
                }

                //
                // Otherwise, prepare the next message stream for writing.
                //
                message = this._sendStreams[0];
                Debug.assert(!message.prepared);

                const stream = message.stream;
                stream.pos = 10;
                stream.writeInt(stream.size);
                stream.prepareWrite();
                message.prepared = true;

                TraceUtil.traceSend(stream, this._logger, this._traceLevels);

                this._writeStream.swap(message.stream);

                //
                // Send the message.
                //
                if (this._writeStream.pos != this._writeStream.size && !this.write(this._writeStream.buffer)) {
                    Debug.assert(!this._writeStream.isEmpty());
                    return;
                }
            }
        } catch (ex) {
            if (ex instanceof LocalException) {
                this.setState(StateClosed, ex);
                return;
            } else {
                throw ex;
            }
        }

        Debug.assert(this._writeStream.isEmpty());
    }

    sendMessage(message) {
        Debug.assert(this._state >= StateActive);
        Debug.assert(this._state < StateClosed);

        if (this._sendStreams.length > 0) {
            message.doAdopt();
            this._sendStreams.push(message);
            return AsyncStatus.Queued;
        }

        Debug.assert(!message.prepared);

        const stream = message.stream;
        stream.pos = 10;
        stream.writeInt(stream.size);
        stream.prepareWrite();
        message.prepared = true;

        TraceUtil.traceSend(stream, this._logger, this._traceLevels);

        if (this.write(stream.buffer)) {
            //
            // Entire buffer was written immediately.
            //
            message.sent();
            return AsyncStatus.Sent;
        }

        message.doAdopt();

        this._writeStream.swap(message.stream);
        this._sendStreams.push(message);

        return AsyncStatus.Queued;
    }

    parseMessage() {
        Debug.assert(this._state > StateNotValidated && this._state < StateClosed);

        let info = new MessageInfo(this._instance);

        this._readStream.swap(info.stream);
        this._readStream.resize(Protocol.headerSize);
        this._readStream.pos = 0;
        this._readHeader = true;

        Debug.assert(info.stream.pos === info.stream.size);

        try {
            //
            // We don't need to check magic and version here. This has already
            // been done by the caller.
            //
            info.stream.pos = 8;
            const messageType = info.stream.readByte();
            const compress = info.stream.readByte();
            if (compress === 2) {
                throw new FeatureNotSupportedException("Cannot decompress compressed message");
            }
            info.stream.pos = Protocol.headerSize;

            switch (messageType) {
                case Protocol.closeConnectionMsg: {
                    TraceUtil.traceRecv(info.stream, this._logger, this._traceLevels);
                    this.setState(StateClosed, new CloseConnectionException());
                    break;
                }

                case Protocol.requestMsg: {
                    if (this._state === StateClosing) {
                        TraceUtil.traceIn(
                            "received request during closing\n" + "(ignored by server, client will retry)",
                            info.stream,
                            this._logger,
                            this._traceLevels,
                        );
                    } else {
                        TraceUtil.traceRecv(info.stream, this._logger, this._traceLevels);
                        info.requestId = info.stream.readInt();
                        info.invokeNum = 1;
                        info.servantManager = this._servantManager;
                        info.adapter = this._adapter;
                        ++this._upcallCount;

                        this.cancelInactivityTimer();
                        ++this._dispatchCount;
                    }
                    break;
                }

                case Protocol.requestBatchMsg: {
                    if (this._state === StateClosing) {
                        TraceUtil.traceIn(
                            "received batch request during closing\n" + "(ignored by server, client will retry)",
                            info.stream,
                            this._logger,
                            this._traceLevels,
                        );
                    } else {
                        TraceUtil.traceRecv(info.stream, this._logger, this._traceLevels);
                        info.invokeNum = info.stream.readInt();
                        if (info.invokeNum < 0) {
                            info.invokeNum = 0;
                            throw new UnmarshalOutOfBoundsException();
                        }
                        info.servantManager = this._servantManager;
                        info.adapter = this._adapter;
                        this._upcallCount += info.invokeNum;

                        this.cancelInactivityTimer();
                        ++this._dispatchCount;
                    }
                    break;
                }

                case Protocol.replyMsg: {
                    TraceUtil.traceRecv(info.stream, this._logger, this._traceLevels);
                    info.requestId = info.stream.readInt();
                    info.outAsync = this._asyncRequests.get(info.requestId);
                    if (info.outAsync) {
                        this._asyncRequests.delete(info.requestId);
                        ++this._upcallCount;
                    } else {
                        info = null;
                    }
                    this.checkClose();
                    break;
                }

                case Protocol.validateConnectionMsg: {
                    TraceUtil.traceRecv(info.stream, this._logger, this._traceLevels);
                    break;
                }

                default: {
                    TraceUtil.traceIn(
                        "received unknown message\n(invalid, closing connection)",
                        info.stream,
                        this._logger,
                        this._traceLevels,
                    );
                    throw new UnknownMessageException();
                }
            }
        } catch (ex) {
            if (ex instanceof LocalException) {
                this.setState(StateClosed, ex);
            } else {
                throw ex;
            }
        }

        return info;
    }

    invokeAll(stream, invokeNum, requestId, servantManager, adapter) {
        try {
            while (invokeNum > 0) {
                //
                // Prepare the invocation.
                //
                const inc = new IncomingAsync(
                    this._instance,
                    this,
                    adapter,
                    requestId !== 0, // response
                    requestId,
                );

                //
                // Dispatch the invocation.
                //
                inc.invoke(servantManager, stream);

                --invokeNum;
            }

            stream.clear();
        } catch (ex) {
            if (ex instanceof LocalException) {
                this.dispatchException(ex, invokeNum);
            } else {
                //
                // An Error was raised outside of servant code (i.e., by Ice code).
                // Attempt to log the error and clean up.
                //
                this._logger.error("unexpected exception:\n" + ex.toString());
                this.dispatchException(new UnknownException(ex), invokeNum);
            }
        }
    }

    connectTimedOut() {
        if (this._state < StateActive) {
            this.setState(StateClosed, new ConnectTimeoutException());
        }
        // else ignore since we're already connected
    }

    closeTimedOut() {
        if (this._state < StateClosed) {
            this.setState(StateClosed, new CloseTimeoutException());
        }
        // else ignore since we're already closed.
    }

    warning(msg, ex) {
        this._logger.warning(msg + ":\n" + this._desc + "\n" + ex.toString());
    }

    checkState() {
        if (this._state < StateHolding || this._upcallCount > 0) {
            return;
        }

        //
        // We aren't finished until the state is finished and all
        // outstanding requests are completed. Otherwise we couldn't
        // guarantee that there are no outstanding calls when deactivate()
        // is called on the servant locators.
        //
        if (this._state === StateFinished && this._finishedPromises.length > 0) {
            //
            // Clear the OA. See bug 1673 for the details of why this is necessary.
            //
            this._adapter = null;
            this._finishedPromises.forEach(p => p.resolve());
            this._finishedPromises = [];
        }
    }

    read(buf) {
        const start = buf.position;
        const ret = this._transceiver.read(buf, this._hasMoreData);
        if (this._traceLevels.network >= 3 && buf.position != start) {
            const s = [];
            s.push("received ");
            s.push(buf.position - start);
            s.push(" of ");
            s.push(buf.limit - start);
            s.push(" bytes via ");
            s.push(this._endpoint.protocol());
            s.push("\n");
            s.push(this.toString());
            this._logger.trace(this._traceLevels.networkCat, s.join(""));
        }
        return ret;
    }

    write(buf) {
        const start = buf.position;
        const ret = this._transceiver.write(buf);
        if (this._traceLevels.network >= 3 && buf.position != start) {
            const s = [];
            s.push("sent ");
            s.push(buf.position - start);
            s.push(" of ");
            s.push(buf.limit - start);
            s.push(" bytes via ");
            s.push(this._endpoint.protocol());
            s.push("\n");
            s.push(this.toString());
            this._logger.trace(this._traceLevels.networkCat, s.join(""));
        }
        return ret;
    }

    scheduleInactivityTimer() {
        Debug.assert(this._inactivityTimer === undefined);
        Debug.assert(this._inactivityTimeout > 0);

        this._inactivityTimer = new Timer();
        const inactivityTimer = this._inactivityTimer;
        this._inactivityTimer.schedule(() => this.inactivityCheck(inactivityTimer), this._inactivityTimeout);
    }

    cancelInactivityTimer() {
        if (this._inactivityTimer !== undefined) {
            this._inactivityTimer.destroy();
            this._inactivityTimer = undefined;
        }
    }
}

// DestructionReason.
ConnectionI.ObjectAdapterDeactivated = 0;
ConnectionI.CommunicatorDestroyed = 1;

class OutgoingMessage {
    constructor() {
        this.stream = null;
        this.outAsync = null;
        this.requestId = 0;
        this.prepared = false;
    }

    canceled() {
        Debug.assert(this.outAsync !== null);
        this.outAsync = null;
    }

    doAdopt() {
        if (this.adopt) {
            const stream = new OutputStream(this.stream.instance, Protocol.currentProtocolEncoding);
            stream.swap(this.stream);
            this.stream = stream;
            this.adopt = false;
        }
    }

    sent() {
        if (this.outAsync !== null) {
            this.outAsync.sent();
        }
    }

    completed(ex) {
        if (this.outAsync !== null) {
            this.outAsync.completedEx(ex);
        }
    }

    static createForStream(stream, adopt) {
        const m = new OutgoingMessage();
        m.stream = stream;
        m.adopt = adopt;
        m.isSent = false;
        m.requestId = 0;
        m.outAsync = null;
        return m;
    }

    static create(out, stream, requestId) {
        const m = new OutgoingMessage();
        m.stream = stream;
        m.outAsync = out;
        m.requestId = requestId;
        m.isSent = false;
        m.adopt = false;
        return m;
    }
}
