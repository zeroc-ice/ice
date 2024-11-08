//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { LocalException } from "./LocalException.js";
import {
    ObjectAdapterDestroyedException,
    CommunicatorDestroyedException,
    CloseConnectionException,
    ConnectionAbortedException,
    ConnectionClosedException,
    ConnectTimeoutException,
    ConnectionLostException,
    CloseTimeoutException,
    FeatureNotSupportedException,
    UnknownException,
    ObjectNotExistException,
} from "./LocalExceptions.js";

import { BatchRequestQueue } from "./BatchRequestQueue.js";
import { InputStream } from "./InputStream.js";
import { OutputStream } from "./OutputStream.js";
import { Protocol } from "./Protocol.js";
import { Ice as Ice_Version } from "./Version.js";
const { ProtocolVersion, EncodingVersion } = Ice_Version;
import { throwMemoryLimitException } from "./ExUtil.js";
import { Timer } from "./Timer.js";
import { Promise } from "./Promise.js";
import { SocketOperation } from "./SocketOperation.js";
import { TraceUtil } from "./TraceUtil.js";
import { AsyncStatus } from "./AsyncStatus.js";
import { RetryException } from "./RetryException.js";
import { ConnectionFlushBatch, OutgoingAsync } from "./OutgoingAsync.js";
import { Debug } from "./Debug.js";
import { IdleTimeoutTransceiverDecorator } from "./IdleTimeoutTransceiverDecorator.js";
import { ObjectAdapter } from "./ObjectAdapter.js";
import { ObjectPrx } from "./ObjectPrx.js";
import { IncomingRequest } from "./IncomingRequest.js";
import "./CurrentExtensions.js";

const StateNotInitialized = 0;
const StateNotValidated = 1;
const StateActive = 2;
// StateHolding is a very transient state in JS: the connection is in StateHolding once the connection establishment
// has succeeded and until the outgoing connection factory activates the connection - which is essentially immediate.
const StateHolding = 3;
const StateClosing = 4;
const StateClosingPending = 5;
const StateClosed = 6;
const StateFinished = 7;

function scheduleCloseTimeout(connection) {
    if (connection._closeTimeout > 0 && connection._closeTimeoutId === undefined) {
        // Schedules a one-time check.
        connection._closeTimeoutId = connection._timer.schedule(
            () => connection.closeTimedOut(),
            connection._closeTimeout,
        );
    }
}

export class ConnectionI {
    constructor(communicator, instance, transceiver, endpoint, adapter, removeFromFactory, options) {
        this._communicator = communicator;
        this._instance = instance;
        this._desc = transceiver.toString();
        this._type = transceiver.type();
        this._endpoint = endpoint;
        this._adapter = adapter;
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

        this._warn = initData.properties.getIcePropertyAsInt("Ice.Warn.Connections") > 0;
        this._nextRequestId = 1;
        this._messageSizeMax = instance.messageSizeMax();
        this._batchRequestQueue = new BatchRequestQueue(instance);

        this._sendStreams = [];

        this._readStream = new InputStream(instance, Protocol.currentProtocolEncoding);
        this._readHeader = false;
        this._writeStream = new OutputStream(); // temporary stream

        this._readStreamPos = -1;
        this._writeStreamPos = -1;

        // The number of user calls currently executed by the event-loop (servant dispatch, invocation response, etc.).
        this._upcallCount = 0;

        // The number of outstanding dispatches. Maintained only while state is StateActive.
        this._dispatchCount = 0;

        this._state = StateNotInitialized;
        this._shutdownInitiated = false;
        this._initialized = false;
        this._validated = false;

        this._asyncRequests = new Map(); // Map<int, OutgoingAsync>

        this._exception = null;

        this._startPromise = null;
        this._closed = undefined;
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

    destroy() {
        // Called by Communicator.destroy
        this.setState(StateClosing, new CommunicatorDestroyedException());
    }

    abort() {
        this.setState(
            StateClosed,
            new ConnectionAbortedException("The connection was aborted by the application.", true),
        );
    }

    close() {
        if (this._closed === undefined) {
            this._closed = new Promise();
            scheduleCloseTimeout(this);
            if (this._asyncRequests.size === 0) {
                this.doApplicationClose();
            }
        }
        return this._closed;
    }

    doApplicationClose() {
        Debug.assert(this._state < StateClosing);
        this.setState(
            StateClosing,
            new ConnectionClosedException("The connection was closed gracefully by the application.", true),
        );
    }

    isActiveOrHolding() {
        return this._state > StateNotValidated && this._state < StateClosing;
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

                if (this._closed !== undefined && this._state < StateClosing && this._asyncRequests.size === 0) {
                    this.doApplicationClose();
                }
                return; // We're done.
            }
        }

        if (outAsync instanceof OutgoingAsync) {
            for (const [key, value] of this._asyncRequests) {
                if (value === outAsync) {
                    this._asyncRequests.delete(key);
                    outAsync.completedEx(ex);

                    if (this._closed !== undefined && this._state < StateClosing && this._asyncRequests.size === 0) {
                        this.doApplicationClose();
                    }
                    return; // We're done.
                }
            }
        }
    }

    sendResponse(response, isTwoWay) {
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

            if (isTwoWay) {
                this.sendMessage(OutgoingMessage.createForStream(response.outputStream));
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
            adapter.checkForDestruction();
            if (this._state <= StateNotValidated || this._state >= StateClosing) {
                return;
            }
            this._adapter = adapter;
        } else {
            if (this._state <= StateNotValidated || this._state >= StateClosing) {
                return;
            }
            this._adapter = null;
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
        let message = null;
        try {
            if ((operation & SocketOperation.Write) !== 0 && this._writeStream.buffer.remaining > 0) {
                Debug.assert(this._sendStreams.length > 0);
                const currentMessage = this._sendStreams[0];
                if (!this.write(this._writeStream.buffer, () => (currentMessage.isSent = true))) {
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
                        throw new ProtocolException(
                            `Bad magic in message header: ${magic0.toString(16)} ${magic1.toString(16)} ${magic2.toString(16)} ${magic3.toString(16)}`,
                        );
                    }

                    const protocolVersion = new ProtocolVersion();
                    protocolVersion._read(this._readStream);
                    if (
                        protocolVersion.major != Protocol.currentProtocol.major ||
                        protocolVersion.minor != Protocol.currentProtocol.minor
                    ) {
                        throw new MarshalException(
                            `Invalid protocol version in message header: ${protocolVersion.major}.${protocolVersion.minor}`,
                        );
                    }

                    const encodingVersion = new EncodingVersion();
                    encodingVersion._read(this._readStream);
                    if (
                        encodingVersion.major != Protocol.currentProtocolEncoding.major ||
                        protocolVersion.minor != Protocol.currentProtocolEncoding.minor
                    ) {
                        throw new MarshalException(
                            `Invalid protocol encoding version in message header: ${encodingVersion.major}.${encodingVersion.minor}`,
                        );
                    }

                    this._readStream.readByte(); // messageType
                    this._readStream.readByte(); // compress
                    const size = this._readStream.readInt();
                    if (size < Protocol.headerSize) {
                        throw new MarshalException(`Received Ice message with unexpected size ${size}.`);
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

                // We start out in holding state.
                this.setState(StateHolding);
                if (this._startPromise !== null) {
                    ++this._upcallCount;
                }
            } else {
                Debug.assert(this._state <= StateClosingPending);

                // We parse messages first, if we receive a close connection message we won't send more messages.
                if ((operation & SocketOperation.Read) !== 0) {
                    info = this.parseMessage();
                }

                if ((operation & SocketOperation.Write) !== 0) {
                    message = this.sendNextMessage();
                    if (message !== null) {
                        // The returned message contains the request for which we delayed the response until it was marked as sent.
                        ++this._upcallCount;
                    }
                }
            }
        } catch (ex) {
            this.setState(StateClosed, ex);
            return;
        }

        this.upcall(info, message);

        if (this._hasMoreData.value) {
            Timer.setImmediate(() => this.message(SocketOperation.Read)); // Don't tie up the thread.
        }
    }

    upcall(info, message) {
        let count = 0;

        // Notify the factory that the connection establishment and validation has completed.
        if (this._startPromise !== null) {
            this._startPromise.resolve();

            this._startPromise = null;
            ++count;
        }

        if (message != null) {
            // Message contains the requests for which we delayed the response until it was marked as sent.1
            Debug.assert(message.receivedReply);
            message.outAsync.completed(message.reply.stream);
            ++count;
        }

        if (info !== null) {
            if (info.outAsync !== null) {
                info.outAsync.completed(info.stream);
                ++count;
            }

            if (info.requestCount > 0) {
                this.dispatchAll(info.stream, info.requestCount, info.requestId, info.adapter);

                // Don't increase count, the dispatch count is decreased when the incoming reply is sent.
            }
        }

        // Decrease the upcall count.
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
            this._closeTimeoutId = undefined;
        }

        const traceLevels = this._traceLevels;
        if (!this._initialized) {
            if (traceLevels.network >= 2) {
                this._logger.trace(
                    traceLevels.networkCat,
                    `failed to establish ${this._endpoint.protocol()} connection\n${this}\n${this._exception}`,
                );
            }
        } else if (traceLevels.network >= 1) {
            let s = `closed ${this._endpoint.protocol()} connection\n${this}`;

            //
            // Trace the cause of unexpected connection closures
            //
            if (
                !(
                    this._exception instanceof CloseConnectionException ||
                    this._exception instanceof ConnectionAbortedException ||
                    this._exception instanceof ConnectionClosedException ||
                    this._exception instanceof CommunicatorDestroyedException ||
                    this._exception instanceof ObjectAdapterDestroyedException
                )
            ) {
                s += `\n${this._exception}`;
            }
            this._logger.trace(traceLevels.networkCat, s);
        }

        if (this._startPromise !== null) {
            this._startPromise.reject(this._exception);
            this._startPromise = null;
        }

        if (this._sendStreams.length > 0) {
            if (!this._writeStream.isEmpty()) {
                // Return the stream to the outgoing call. This is important for retriable AMI calls, which are not
                // marshaled again.
                const message = this._sendStreams[0];
                this._writeStream.swap(message.stream);

                // The current message might have been sent but not yet removed from _sendStreams. We mark it as sent
                // and remove it from _sendStreams to avoid calling finish on a message that has already been processed.
                if (message.isSent || message.reply !== null) {
                    message.sent();
                    if (message.reply !== null) {
                        // If the response has already been received, process it now.
                        message.outAsync.completed(message.reply.stream);
                    }
                    _sendStreams.shift();
                }
            }

            //
            // NOTE: for twoway requests which are not sent, finished can be called twice: the
            // first time because the outgoing is in the _sendStreams set and the second time
            // because it's either in the _asyncRequests set. This is fine, only the
            // first call should be taken into account by the implementation of finished.
            //
            for (const message of this._sendStreams) {
                if (message.requestId > 0) {
                    this._asyncRequests.delete(p.requestId);
                }
                message.completed(this._exception);
            }
            this._sendStreams = [];
        }

        for (const request of this._asyncRequests.values()) {
            request.completedEx(this._exception);
        }
        this._asyncRequests.clear();

        //
        // Don't wait to be reaped to reclaim memory allocated by read/write streams.
        //
        this._readStream.clear();
        this._readStream.buffer.clear();
        this._writeStream.clear();
        this._writeStream.buffer.clear();

        if (this._closed !== undefined) {
            if (
                this._exception instanceof ConnectionClosedException ||
                this._exception instanceof CloseConnectionException ||
                this._exception instanceof CommunicatorDestroyedException ||
                this._exception instanceof ObjectAdapterDestroyedException
            ) {
                this._closed.resolve();
            } else {
                Debug.assert(this._exception !== null);
                this._closed.reject(this._exception);
            }
        }

        if (this._closeCallback !== null) {
            try {
                this._closeCallback(this);
            } catch (ex) {
                this._logger.error(`connection callback exception:\n${ex}\n${this._desc}`);
            }
            this._closeCallback = null;
        }

        // This must be done last as this will cause waitUntilFinished() to return (and communicator objects such as
        // the timer might be destroyed too).
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

    dispatchException(ex, requestCount) {
        //
        // Fatal exception while invoking a request. Since sendResponse isn't
        // called in case of a fatal exception we decrement this._upcallCount here.
        //

        this.setState(StateClosed, ex);

        if (requestCount > 0) {
            Debug.assert(this._upcallCount > requestCount);
            this._upcallCount -= requestCount;
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

            // If setState() is called with an exception, then only closed and closing states are permissible.
            Debug.assert(state >= StateClosing);

            if (this._state === state) {
                // Don't switch twice.
                return;
            }

            if (this._exception === null) {
                this._exception = ex;

                // We don't warn if we are not validated.
                if (this._warn && this._validated) {
                    // Don't warn about certain expected exceptions.
                    if (
                        !(
                            this._exception instanceof CloseConnectionException ||
                            this._exception instanceof ConnectionAbortedException ||
                            this._exception instanceof ConnectionClosedException ||
                            this._exception instanceof CommunicatorDestroyedException ||
                            this._exception instanceof ObjectAdapterDestroyedException ||
                            (this._exception instanceof ConnectionLostException && this._state === StateClosing)
                        )
                    ) {
                        this.warning("connection exception", this._exception);
                    }
                }
            }

            // We must set the new state before we notify requests of any exceptions. Otherwise new requests may retry
            // on a connection that is not yet marked as closed or closing.
        }

        // Skip graceful shutdown if we are destroyed before validation.
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
                    // Register to receive validation message. Once validation is complete, a new connection starts out
                    // in the Holding state. We only want to register the transceiver now if we need to receive data in
                    // order to validate the connection.
                    this._transceiver.register();
                    break;
                }

                case StateActive: {
                    Debug.assert(this._state <= StateHolding);
                    this._transceiver.register();
                    break;
                }

                case StateHolding:
                    Debug.assert(this._state === StateNotValidated);
                    this._transceiver.unregister();
                    break; // see comment on StateHolding definition

                case StateClosing:
                case StateClosingPending: {
                    // Can't change back from closing pending.
                    if (this._state >= StateClosingPending) {
                        return;
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

        // Before we shut down, we send a close connection message.
        const os = new OutputStream(Protocol.currentProtocolEncoding);
        os.writeBlob(Protocol.magic);
        Protocol.currentProtocol._write(os);
        Protocol.currentProtocolEncoding._write(os);
        os.writeByte(Protocol.closeConnectionMsg);
        os.writeByte(0); // compression status: always report 0 for CloseConnection.
        os.writeInt(Protocol.headerSize); // Message size.

        scheduleCloseTimeout(this);

        if ((this.sendMessage(OutgoingMessage.createForStream(os)) & AsyncStatus.Sent) > 0) {
            this.setState(StateClosingPending);
        }
    }

    idleCheck(idleTimeout) {
        if (this._state === StateActive || this._state === StateHolding) {
            if (this._traceLevels.network >= 1) {
                this._logger.trace(
                    this._traceLevels.networkCat,
                    `connection aborted by the idle check because it did not receive any bytes for ${idleTimeout}s\n${this._transceiver.toString()}`,
                );
            }
            this.setState(
                StateClosed,
                new ConnectionAbortedException(
                    `Connection aborted by the idle check because it did not receive any bytes for ${idleTimeout}s.`,
                    false,
                ),
            );
        }
        // else nothing to do
    }

    sendHeartbeat() {
        if (this._state === StateActive || this._state === StateHolding || this._state === StateClosing) {
            // We check if the connection has become inactive.
            if (
                this._inactivityTimer === undefined && // timer not already scheduled
                this._inactivityTimeout > 0 && // inactivity timeout is enabled
                this._state === StateActive && // only schedule the timer if the connection is active
                this._dispatchCount === 0 && // no pending dispatch
                this._asyncRequests.size === 0 && // no pending invocation
                this._readHeader && // we're not waiting for the remainder of an incoming message
                this._sendStreams.length <= 1 // there is at most one pending outgoing message
            ) {
                // We may become inactive while the peer is back-pressuring us. In this case, we only schedule the
                // inactivity timer if there is no pending outgoing message or the pending outgoing message is a
                // heartbeat.

                // The stream of the first _sendStreams message is in _writeStream.
                if (
                    this._sendStreams.length === 0 ||
                    this._writeStream.buffer.getAt(8) === Protocol.validateConnectionMsg
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
            if (this._sendStreams.length === 0) {
                const os = new OutputStream(Protocol.currentProtocolEncoding);
                os.writeBlob(Protocol.magic);
                Protocol.currentProtocol._write(os);
                Protocol.currentProtocolEncoding._write(os);
                os.writeByte(Protocol.validateConnectionMsg);
                os.writeByte(0);
                os.writeInt(Protocol.headerSize); // Message size.
                try {
                    this.sendMessage(OutgoingMessage.createForStream(os));
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
            throw new ProtocolException(
                `Bad magic in message header: ${m[0].toString(16)} ${m[1].toString(16)} ${m[2].toString(16)} ${m[3].toString(16)}`,
            );
        }

        const protocolVersion = new ProtocolVersion();
        protocolVersion._read(this._readStream);
        if (
            protocolVersion.major != Protocol.currentProtocol.major ||
            protocolVersion.minor != Protocol.currentProtocol.minor
        ) {
            throw new MarshalException(
                `Invalid protocol version in message header: ${protocolVersion.major}.${protocolVersion.minor}`,
            );
        }

        const encodingVersion = new EncodingVersion();
        encodingVersion._read(this._readStream);
        if (
            encodingVersion.major != Protocol.currentProtocolEncoding.major ||
            protocolVersion.minor != Protocol.currentProtocolEncoding.minor
        ) {
            throw new MarshalException(
                `Invalid protocol encoding version in message header: ${encodingVersion.major}.${encodingVersion.minor}`,
            );
        }

        const messageType = this._readStream.readByte();
        if (messageType !== Protocol.validateConnectionMsg) {
            throw new ProtocolException(
                `Received message of type ${messageType} over a connection that is not yet validated.`,
            );
        }
        this._readStream.readByte(); // Ignore compression status for validate connection.
        if (this._readStream.readInt() !== Protocol.headerSize) {
            throw new MarshalException(`Received ValidateConnection message with unexpected size ${size}.`);
        }
        TraceUtil.traceRecv(this._readStream, this, this._logger, this._traceLevels);

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

    /**
     * Called when the connection is ready to start sending the next message in the send queue. The connection queues
     * protocol messages to be sent when the sending cannot complete synchronously or when another protocol message is
     * in the process of being sent.
     *
     * When this method is called, the first message in the send queue (the message currently being sent) has already
     * been sent and will be removed from the queue.
     *
     * This method will continue sending messages until the send queue is empty or until a protocol message cannot be
     * sent synchronously.
     *
     * If the first message in the queue is a protocol request and its reply was received before this method was called,
     * the reply should be cached in the message's reply field and will be returned as the result. The caller is then
     * responsible for processing the reply, now that the message has been marked as sent.
     *
     * @returns The reply of the first message in the send queue if it has been received; null otherwise.
     */
    sendNextMessage() {
        let completed = null;
        if (this._sendStreams.length === 0) {
            return completed;
        }

        Debug.assert(!this._writeStream.isEmpty() && this._writeStream.pos === this._writeStream.size);

        // The first message in the queue has already been sent, notify the message and remove it from the sent queue.
        let message = this._sendStreams.shift();
        this._writeStream.swap(message.stream);
        message.sent();

        // If the reply for the first message in the queue was received before this method was called, we will return
        // the reply to the caller to be processed upon return.
        if (message.reply !== null) {
            completed = message.reply;
        }

        try {
            // Continue sending messages until the send queue is empty or until a message cannot be sent synchronously.
            // or the connection is closing.
            while (this._sendStreams.length > 0 && this._state < StateClosingPending) {
                // Prepare the next message stream for writing.
                message = this._sendStreams[0];
                const stream = message.stream;
                stream.pos = 10;
                stream.writeInt(stream.size);
                stream.prepareWrite();

                TraceUtil.traceSend(stream, this, this._logger, this._traceLevels);

                this._writeStream.swap(message.stream);

                // Send the message.
                if (
                    this._writeStream.pos != this._writeStream.size &&
                    !this.write(this._writeStream.buffer, () => (message.isSent = true))
                ) {
                    Debug.assert(!this._writeStream.isEmpty());
                    return completed; // not done
                }

                // The message was sent synchronously, notify the message, remove it from the sent queue and keep going.
                this._sendStreams.shift();
                this._writeStream.swap(message.stream);
                message.sent();
            }

            // Once the CloseConnection message is sent, we transition to the StateClosingPending state.
            if (this._state === StateClosing && this._shutdownInitiated) {
                this.setState(StateClosingPending);
            }
        } catch (ex) {
            if (ex instanceof LocalException) {
                this.setState(StateClosed, ex);
                return completed;
            } else {
                throw ex;
            }
        }

        Debug.assert(this._writeStream.isEmpty());
        return completed;
    }

    sendMessage(message) {
        Debug.assert(this._state >= StateActive);
        Debug.assert(this._state < StateClosed);

        if (this._sendStreams.length > 0) {
            this._sendStreams.push(message);
            return AsyncStatus.Queued;
        }

        const stream = message.stream;
        stream.pos = 10;
        stream.writeInt(stream.size);
        stream.prepareWrite();

        TraceUtil.traceSend(stream, this, this._logger, this._traceLevels);

        if (this.write(stream.buffer, () => (message.isSent = true))) {
            // Entire buffer was written immediately.
            message.sent();
            return AsyncStatus.Sent;
        }

        this._writeStream.swap(message.stream);
        this._sendStreams.push(message);

        return AsyncStatus.Queued;
    }

    parseMessage() {
        Debug.assert(this._state > StateNotValidated && this._state < StateClosed);

        let info = new IncomingMessage(this._instance);

        this._readStream.swap(info.stream);
        this._readStream.resize(Protocol.headerSize);
        this._readStream.pos = 0;
        this._readHeader = true;

        Debug.assert(info.stream.pos === info.stream.size);

        try {
            // We don't need to check magic and version here. This has already been done by the caller.
            info.stream.pos = 8;
            const messageType = info.stream.readByte();
            const compress = info.stream.readByte();
            if (compress === 2) {
                throw new FeatureNotSupportedException("Cannot decompress compressed message");
            }
            info.stream.pos = Protocol.headerSize;

            switch (messageType) {
                case Protocol.closeConnectionMsg: {
                    TraceUtil.traceRecv(info.stream, this, this._logger, this._traceLevels);
                    // We transition directly to StateClosed, not StateClosingPending.
                    this.setState(StateClosed, new CloseConnectionException());
                    break;
                }

                case Protocol.requestMsg: {
                    if (this._state >= StateClosing) {
                        TraceUtil.traceIn(
                            "received request during closing\n(ignored by server, client will retry)",
                            info.stream,
                            this,
                            this._logger,
                            this._traceLevels,
                        );
                    } else {
                        TraceUtil.traceRecv(info.stream, this, this._logger, this._traceLevels);
                        info.requestId = info.stream.readInt();
                        info.requestCount = 1;
                        info.adapter = this._adapter;
                        ++this._upcallCount;

                        this.cancelInactivityTimer();
                        ++this._dispatchCount;
                    }
                    break;
                }

                case Protocol.requestBatchMsg: {
                    if (this._state >= StateClosing) {
                        TraceUtil.traceIn(
                            "received batch request during closing\n(ignored by server, client will retry)",
                            info.stream,
                            this,
                            this._logger,
                            this._traceLevels,
                        );
                    } else {
                        TraceUtil.traceRecv(info.stream, this, this._logger, this._traceLevels);
                        const requestCount = info.stream.readInt();
                        if (info.requestCount < 0) {
                            throw new MarshalException(`Received batch request with ${requestCount} batches.`);
                        }
                        info.requestCount = requestCount;
                        info.adapter = this._adapter;
                        this._upcallCount += info.requestCount;

                        this.cancelInactivityTimer();
                        ++this._dispatchCount;
                    }
                    break;
                }

                case Protocol.replyMsg: {
                    TraceUtil.traceRecv(info.stream, this, this._logger, this._traceLevels);
                    info.requestId = info.stream.readInt();
                    info.outAsync = this._asyncRequests.get(info.requestId);
                    if (info.outAsync !== undefined) {
                        this._asyncRequests.delete(info.requestId);

                        // If we receive a reply for a request that hasn’t been marked as sent, we store the reply in the request's
                        // reply field and delay processing until the request is marked as sent. This can occur if the request is sent
                        // asynchronously and the reply is processed before the write-ready callback has a chance to run and invoke 
                        // sendNextMessage.
                        const message = this._sendStreams.length > 0 ? this._sendStreams[0] : null;
                        if (message !== null && message.outAsync === info.outAsync) {
                            message.reply = info;
                            info = null;
                        } else {
                            Debug.assert(info.outAsync.isSent());
                        }
                        ++this._upcallCount;

                        if (
                            this._closed !== undefined &&
                            this._state < StateClosing &&
                            this._asyncRequests.size === 0
                        ) {
                            this.doApplicationClose();
                        }
                    } else {
                        info.outAsync = null;
                    }
                    break;
                }

                case Protocol.validateConnectionMsg: {
                    TraceUtil.traceRecv(info.stream, this, this._logger, this._traceLevels);
                    break;
                }

                default: {
                    TraceUtil.traceIn(
                        "received unknown message\n(invalid, closing connection)",
                        info.stream,
                        this,
                        this._logger,
                        this._traceLevels,
                    );
                    throw new ProtocolException(`Received Ice protocol message with unknown type: ${messageType}`);
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

    dispatchAll(stream, requestCount, requestId, adapter) {
        const dispatcher = adapter !== null ? adapter.dispatchPipeline : null;
        try {
            while (requestCount > 0) {
                // adapter can be null here, however the adapter set in current can't be null, and we never pass
                // a null current.adapter to the application code.
                const request = new IncomingRequest(requestId, this, adapter, stream);

                if (dispatcher !== null) {
                    // We don't and can't await the dispatchAsync: with batch requests, we want all the dispatches to
                    // execute synchronously. If we awaited the dispatchAsync, we could switch to another event loop task.
                    dispatchAsync(this, request);
                } else {
                    // Received request on a connection without an object adapter.
                    this.sendResponse(
                        request.current.createOutgoingResponseWithException(new ObjectNotExistException()),
                        !this._endpoint.datagram() && requestId != 0,
                    );
                }
                --requestCount;
            }

            stream.clear();
        } catch (ex) {
            if (ex instanceof LocalException) {
                this.dispatchException(ex, requestCount);
            } else {
                //
                // An Error was raised outside of servant code (i.e., by Ice code).
                // Attempt to log the error and clean up.
                //
                this._logger.error(`unexpected exception:\n ${ex}`);
                this.dispatchException(
                    new UnknownException("unexpected exception dispatching request", { cause: ex }),
                    requestCount,
                );
            }
        }

        async function dispatchAsync(connection, request) {
            try {
                let response;
                try {
                    response = await dispatcher.dispatch(request);
                } catch (ex) {
                    response = request.current.createOutgoingResponseWithException(ex);
                }
                connection.sendResponse(response, !connection._endpoint.datagram() && requestId != 0);
            } catch (ex) {
                connection.dispatchException(ex, 1);
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
            // We don't use setState(state, exception) because we want to overwrite the exception set by a
            // graceful closure.
            this._exception = new CloseTimeoutException();
            this.setState(StateClosed);
        }
        // else ignore since we're already closed.
    }

    warning(msg, ex) {
        this._logger.warning(msg + ":\n" + this._desc + "\n" + ex.toString());
    }

    checkState() {
        // We aren't finished until the state is finished and all outstanding requests are completed. Otherwise we
        // couldn't guarantee that there are no outstanding calls when deactivate() is called on the servant locators.
        if (this._state === StateFinished && this._upcallCount === 0) {
            for (const p of this._finishedPromises) {
                p.resolve();
            }
            this._finishedPromises = [];
        }
    }

    read(buf) {
        const start = buf.position;
        const ret = this._transceiver.read(buf, this._hasMoreData);
        if (this._traceLevels.network >= 3 && buf.position != start) {
            this._logger.trace(
                this._traceLevels.networkCat,
                `received ${buf.position - start} of ${buf.limit - start} bytes via ${this._endpoint.protocol()}\n${this}`,
            );
        }
        return ret;
    }

    write(buffer, bufferFullyWritten) {
        const start = buffer.position;
        const ret = this._transceiver.write(buffer, bufferFullyWritten);
        if (this._traceLevels.network >= 3 && buffer.position != start) {
            this._logger.trace(
                this._traceLevels.networkCat,
                `sent ${buffer.position - start} of ${buffer.limit - start} bytes via ${this._endpoint.protocol()}\n${this}`,
            );
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

/**
 * Represents an incoming protocol message received by the connection.
 */
class IncomingMessage {
    constructor(instance) {
        this.stream = new InputStream(instance, Protocol.currentProtocolEncoding);
        this.requestCount = 0;
        this.requestId = 0;
        this.adapter = null;
        this.outAsync = null;
        this.upcallCount = 0;
    }
}

/**
 * Represents an outgoing protocol message sent by the connection.
 */
class OutgoingMessage {
    // Not used directly, see createForStream and create static methods below.
    constructor(requestId, stream, outAsync) {
        // The OutputStream containing the message to be sent. The connection swaps this stream with its own write stream
        // while the message is being sent and swaps it back once the message has been sent.
        this.stream = stream;
        // The OutgoingAsync object associated with a protocol request message; it is always null for other message types.
        this.outAsync = outAsync;
        // The request ID for two-way requests; 0 for one-way requests and other message types.
        this.requestId = requestId;
        // For a request message: if the reply is received before the request is marked as sent and removed from the
        // send queue, we store the reply in this field and delay its processing until the message is marked as sent.
        this.reply = null;
        // Set to true by the transport bufferFullyWriteCallback to ensure "at most once" semantics for non-idempotent,
        // retriable requests.
        this.isSent = false;
    }

    canceled() {
        Debug.assert(this.outAsync !== null);
        this.outAsync = null;
    }

    // If the outgoing message represents an outgoing request, notify it that the request has been sent.
    sent() {
        if (this.outAsync !== null) {
            this.outAsync.sent();
        }
    }

    // If the outgoing message represents an outgoing request, notify it that the request has been completed.
    completed(ex) {
        if (this.outAsync !== null) {
            this.outAsync.completedEx(ex);
        }
    }

    // Creates an OutgoingMessage from a stream containing the encoded protocol message. This method is never used for
    // protocol requests.
    static createForStream(stream) {
        return new OutgoingMessage(0, stream, null);
    }

    // Creates an OutgoingMessage for a protocol request message.
    static create(out, stream, requestId) {
        return new OutgoingMessage(requestId, stream, out);
    }
}
