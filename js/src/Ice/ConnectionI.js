// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/AsyncStatus",
        "../Ice/Stream",
        "../Ice/OutgoingAsync",
        "../Ice/Debug",
        "../Ice/ExUtil",
        "../Ice/HashMap",
        "../Ice/IncomingAsync",
        "../Ice/RetryException",
        "../Ice/Promise",
        "../Ice/Protocol",
        "../Ice/SocketOperation",
        "../Ice/Timer",
        "../Ice/TraceUtil",
        "../Ice/Version",
        "../Ice/Exception",
        "../Ice/LocalException",
        "../Ice/BatchRequestQueue",
    ]);

var AsyncStatus = Ice.AsyncStatus;
var AsyncResultBase = Ice.AsyncResultBase;
var InputStream = Ice.InputStream;
var OutputStream = Ice.OutputStream;
var BatchRequestQueue = Ice.BatchRequestQueue;
var ConnectionFlushBatch = Ice.ConnectionFlushBatch;
var Debug = Ice.Debug;
var ExUtil = Ice.ExUtil;
var HashMap = Ice.HashMap;
var IncomingAsync = Ice.IncomingAsync;
var RetryException = Ice.RetryException;
var Promise = Ice.Promise;
var Protocol = Ice.Protocol;
var SocketOperation = Ice.SocketOperation;
var Timer = Ice.Timer;
var TraceUtil = Ice.TraceUtil;
var ProtocolVersion = Ice.ProtocolVersion;
var EncodingVersion = Ice.EncodingVersion;
var ACM = Ice.ACM;
var ACMClose = Ice.ACMClose;
var ACMHeartbeat = Ice.ACMHeartbeat;

var StateNotInitialized = 0;
var StateNotValidated = 1;
var StateActive = 2;
var StateHolding = 3;
var StateClosing = 4;
var StateClosed = 5;
var StateFinished = 6;

var MessageInfo = function(instance)
{
    this.stream = new InputStream(instance, Protocol.currentProtocolEncoding);

    this.invokeNum = 0;
    this.requestId = 0;
    this.compress = false;
    this.servantManager = null;
    this.adapter = null;
    this.outAsync = null;
    this.heartbeatCallback = null;
};

var Class = Ice.Class;

var ConnectionI = Class({
    __init__: function(communicator, instance, monitor, transceiver, endpoint, incoming, adapter)
    {
        this._communicator = communicator;
        this._instance = instance;
        this._monitor = monitor;
        this._transceiver = transceiver;
        this._desc = transceiver.toString();
        this._type = transceiver.type();
        this._endpoint = endpoint;
        this._incoming = incoming;
        this._adapter = adapter;
        var initData = instance.initializationData();
        this._logger = initData.logger; // Cached for better performance.
        this._traceLevels = instance.traceLevels(); // Cached for better performance.
        this._timer = instance.timer();
        this._writeTimeoutId = 0;
        this._writeTimeoutScheduled = false;
        this._readTimeoutId = 0;
        this._readTimeoutScheduled = false;

        this._hasMoreData = { value: false };

        this._warn = initData.properties.getPropertyAsInt("Ice.Warn.Connections") > 0;
        this._warnUdp = instance.initializationData().properties.getPropertyAsInt("Ice.Warn.Datagrams") > 0;
        this._acmLastActivity = this._monitor !== null && this._monitor.getACM().timeout > 0 ? Date.now() : -1;
        this._nextRequestId = 1;
        this._messageSizeMax = adapter ? adapter.messageSizeMax() : instance.messageSizeMax();
        this._batchRequestQueue = new BatchRequestQueue(instance, endpoint.datagram());

        this._sendStreams = [];

        this._readStream = new InputStream(instance, Protocol.currentProtocolEncoding);
        this._readHeader = false;
        this._writeStream = new OutputStream(instance, Protocol.currentProtocolEncoding);

        this._readStreamPos = -1;
        this._writeStreamPos = -1;

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
        this._holdPromises = [];
        this._finishedPromises = [];

        if(this._adapter !== null)
        {
            this._servantManager = this._adapter.getServantManager();
        }
        else
        {
            this._servantManager = null;
        }
        this._closeCallback = null;
        this._heartbeatCallback = null;
    },
    start: function()
    {
        Debug.assert(this._startPromise === null);

        try
        {
            // The connection might already be closed if the communicator was destroyed.
            if(this._state >= StateClosed)
            {
                Debug.assert(this._exception !== null);
                return new Promise().fail(this._exception);
            }

            this._startPromise = new Promise();
            var self = this;
            this._transceiver.setCallbacks(
                function() { self.message(SocketOperation.Write); }, // connected callback
                function() { self.message(SocketOperation.Read); },  // read callback
                function(bytesSent, bytesTotal) {
                    self.message(SocketOperation.Write);
                    if(self._instance.traceLevels().network >= 3 && bytesSent > 0)
                    {
                        var s = [];
                        s.push("sent ");
                        s.push(bytesSent);
                        if(!self._endpoint.datagram())
                        {
                            s.push(" of ");
                            s.push(bytesTotal);
                        }
                        s.push(" bytes via ");
                        s.push(self._endpoint.protocol());
                        s.push("\n");
                        s.push(this.toString());
                        self._instance.initializationData().logger.trace(self._instance.traceLevels().networkCat,
                                                                         s.join(""));
                    }
                }  // write callback
            );
            this.initialize();
        }
        catch(ex)
        {
            if(ex instanceof Ice.LocalException)
            {
                this.exception(ex);
            }
            return new Promise().fail(ex);
        }

        return this._startPromise;
    },
    activate: function()
    {
        if(this._state <= StateNotValidated)
        {
            return;
        }

        if(this._acmLastActivity > 0)
        {
            this._acmLastActivity = Date.now();
        }
        this.setState(StateActive);
    },
    hold: function()
    {
        if(this._state <= StateNotValidated)
        {
            return;
        }

        this.setState(StateHolding);
    },
    destroy: function(reason)
    {
        switch(reason)
        {
            case ConnectionI.ObjectAdapterDeactivated:
            {
                this.setState(StateClosing, new Ice.ObjectAdapterDeactivatedException());
                break;
            }

            case ConnectionI.CommunicatorDestroyed:
            {
                this.setState(StateClosing, new Ice.CommunicatorDestroyedException());
                break;
            }
        }
    },
    close: function(force)
    {
        var __r = new AsyncResultBase(this._communicator, "close", this, null, null);

        if(force)
        {
            this.setState(StateClosed, new Ice.ForcedCloseConnectionException());
            __r.succeed(__r);
        }
        else
        {
            //
            // If we do a graceful shutdown, then we wait until all
            // outstanding requests have been completed. Otherwise,
            // the CloseConnectionException will cause all outstanding
            // requests to be retried, regardless of whether the
            // server has processed them or not.
            //
            this._closePromises.push(__r);
            this.checkClose();
        }

        return __r;
    },
    checkClose: function()
    {
        //
        // If close(false) has been called, then we need to check if all
        // requests have completed and we can transition to StateClosing.
        // We also complete outstanding promises.
        //
        if(this._asyncRequests.size === 0 && this._closePromises.length > 0)
        {
            this.setState(StateClosing, new Ice.CloseConnectionException());
            for(var i = 0; i < this._closePromises.length; ++i)
            {
                this._closePromises[i].succeed(this._closePromises[i]);
            }
            this._closePromises = [];
        }
    },
    isActiveOrHolding: function()
    {
        return this._state > StateNotValidated && this._state < StateClosing;
    },
    isFinished: function()
    {
        if(this._state !== StateFinished || this._dispatchCount !== 0)
        {
            return false;
        }

        Debug.assert(this._state === StateFinished);
        return true;
    },
    throwException: function()
    {
        if(this._exception !== null)
        {
            Debug.assert(this._state >= StateClosing);
            throw this._exception;
        }
    },
    waitUntilHolding: function()
    {
        var promise = new Promise();
        this._holdPromises.push(promise);
        this.checkState();
        return promise;
    },
    waitUntilFinished: function()
    {
        var promise = new Promise();
        this._finishedPromises.push(promise);
        this.checkState();
        return promise;
    },
    monitor: function(now, acm)
    {
        if(this._state !== StateActive)
        {
            return;
        }

        //
        // We send a heartbeat if there was no activity in the last
        // (timeout / 4) period. Sending a heartbeat sooner than
        // really needed is safer to ensure that the receiver will
        // receive in time the heartbeat. Sending the heartbeat if
        // there was no activity in the last (timeout / 2) period
        // isn't enough since monitor() is called only every (timeout
        // / 2) period.
        //
        // Note that this doesn't imply that we are sending 4 heartbeats
        // per timeout period because the monitor() method is sill only
        // called every (timeout / 2) period.
        //
        if(acm.heartbeat == Ice.ACMHeartbeat.HeartbeatAlways ||
            (acm.heartbeat != Ice.ACMHeartbeat.HeartbeatOff && this._writeStream.isEmpty() &&
             now >= (this._acmLastActivity + acm.timeout / 4)))
        {
            if(acm.heartbeat != Ice.ACMHeartbeat.HeartbeatOnInvocation || this._dispatchCount > 0)
            {
                this.heartbeat(); // Send heartbeat if idle in the last timeout / 2 period.
            }
        }

        if(this._readStream.size > Protocol.headerSize || !this._writeStream.isEmpty())
        {
            //
            // If writing or reading, nothing to do, the connection
            // timeout will kick-in if writes or reads don't progress.
            // This check is necessary because the actitivy timer is
            // only set when a message is fully read/written.
            //
            return;
        }

        if(acm.close != Ice.ACMClose.CloseOff && now >= (this._acmLastActivity + acm.timeout))
        {
            if(acm.close == Ice.ACMClose.CloseOnIdleForceful ||
                (acm.close != Ice.ACMClose.CloseOnIdle && this._asyncRequests.size > 0))
            {
                //
                // Close the connection if we didn't receive a heartbeat in
                // the last period.
                //
                this.setState(StateClosed, new Ice.ConnectionTimeoutException());
            }
            else if(acm.close != Ice.ACMClose.CloseOnInvocation &&
                    this._dispatchCount === 0 && this._batchRequestQueue.isEmpty() && this._asyncRequests.size === 0)
            {
                //
                // The connection is idle, close it.
                //
                this.setState(StateClosing, new Ice.ConnectionTimeoutException());
            }
        }
    },
    sendAsyncRequest: function(out, compress, response, batchRequestNum)
    {
        var requestId = 0;
        var os = out.__os();

        if(this._exception !== null)
        {
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
        // Ensure the message isn't bigger than what we can send with the
        // transport.
        //
        this._transceiver.checkSendSize(os);

        //
        // Notify the request that it's cancelable with this connection.
        // This will throw if the request is canceled.
        //
        out.__cancelable(this); // Notify the request that it's cancelable

        if(response)
        {
            //
            // Create a new unique request ID.
            //
            requestId = this._nextRequestId++;
            if(requestId <= 0)
            {
                this._nextRequestId = 1;
                requestId = this._nextRequestId++;
            }

            //
            // Fill in the request ID.
            //
            os.pos = Protocol.headerSize;
            os.writeInt(requestId);
        }
        else if(batchRequestNum > 0)
        {
            os.pos = Protocol.headerSize;
            os.writeInt(batchRequestNum);
        }

        var status;
        try
        {
            status = this.sendMessage(OutgoingMessage.create(out, out.__os(), compress, requestId));
        }
        catch(ex)
        {
            if(ex instanceof Ice.LocalException)
            {
                this.setState(StateClosed, ex);
                Debug.assert(this._exception !== null);
                throw this._exception;
            }
            else
            {
                throw ex;
            }
        }

        if(response)
        {
            //
            // Add to the async requests map.
            //
            this._asyncRequests.set(requestId, out);
        }

        return status;
    },
    getBatchRequestQueue: function()
    {
        return this._batchRequestQueue;
    },
    flushBatchRequests: function()
    {
        var result = new ConnectionFlushBatch(this, this._communicator, "flushBatchRequests");
        result.__invoke();
        return result;
    },
    setCloseCallback: function(callback)
    {
        if(this._state >= StateClosed)
        {
            if(callback !== null)
            {
                var self = this;
                Timer.setImmediate(function() {
                    try
                    {
                        callback(this);
                    }
                    catch(ex)
                    {
                        self._logger.error("connection callback exception:\n" + ex + '\n' + self._desc);
                    }
                });
            }
        }
        else
        {
            this._closeCallback = callback;
        }
    },
    setHeartbeatCallback: function(callback)
    {
        this._heartbeatCallback = callback;
    },
    setACM: function(timeout, close, heartbeat)
    {
        if(this._monitor === null || this._state >= StateClosed)
        {
            return;
        }

        if(this._state == StateActive)
        {
            this._monitor.remove(this);
        }
        this._monitor = this._monitor.acm(timeout, close, heartbeat);
        if(this._state == StateActive)
        {
            this._monitor.add(this);
        }
        if(this._monitor.getACM().timeout <= 0)
        {
            this._acmLastActivity = -1; // Disable the recording of last activity.
        }
        else if(this._state == StateActive && this._acmLastActivity == -1)
        {
            this._acmLastActivity = Date.now();
        }
    },
    getACM: function()
    {
        return this._monitor !== null ? this._monitor.getACM() :
            new ACM(0, ACMClose.CloseOff, ACMHeartbeat.HeartbeatOff);
    },
    asyncRequestCanceled: function(outAsync, ex)
    {
        for(var i = 0; i < this._sendStreams.length; i++)
        {
            var o = this._sendStreams[i];
            if(o.outAsync === outAsync)
            {
                if(o.requestId > 0)
                {
                    this._asyncRequests.delete(o.requestId);
                }

                //
                // If the request is being sent, don't remove it from the send streams,
                // it will be removed once the sending is finished.
                //
                o.canceled();
                if(i !== 0)
                {
                    this._sendStreams.splice(i, 1);
                }
                outAsync.__completedEx(ex);
                return; // We're done.
            }
        }

        if(outAsync instanceof Ice.OutgoingAsync)
        {
            for(var e = this._asyncRequests.entries; e !== null; e = e.next)
            {
                if(e.value === outAsync)
                {
                    this._asyncRequests.delete(e.key);
                    outAsync.__completedEx(ex);
                    return; // We're done.
                }
            }
        }
    },
    sendResponse: function(os, compressFlag)
    {
        Debug.assert(this._state > StateNotValidated);

        try
        {
            if(--this._dispatchCount === 0)
            {
                if(this._state === StateFinished)
                {
                    this.reap();
                }
                this.checkState();
            }

            if(this._state >= StateClosed)
            {
                Debug.assert(this._exception !== null);
                throw this._exception;
            }

            this.sendMessage(OutgoingMessage.createForStream(os, compressFlag !== 0, true));

            if(this._state === StateClosing && this._dispatchCount === 0)
            {
                this.initiateShutdown();
            }
        }
        catch(ex)
        {
            if(ex instanceof Ice.LocalException)
            {
                this.setState(StateClosed, ex);
            }
            else
            {
                throw ex;
            }
        }
    },
    sendNoResponse: function()
    {
        Debug.assert(this._state > StateNotValidated);
        try
        {
            if(--this._dispatchCount === 0)
            {
                if(this._state === StateFinished)
                {
                    this.reap();
                }
                this.checkState();
            }

            if(this._state >= StateClosed)
            {
                Debug.assert(this._exception !== null);
                throw this._exception;
            }

            if(this._state === StateClosing && this._dispatchCount === 0)
            {
                this.initiateShutdown();
            }
        }
        catch(ex)
        {
            if(ex instanceof Ice.LocalException)
            {
                this.setState(StateClosed, ex);
            }
            else
            {
                throw ex;
            }
        }
    },
    endpoint: function()
    {
        return this._endpoint;
    },
    setAdapter: function(adapter)
    {
        if(this._state <= StateNotValidated || this._state >= StateClosing)
        {
            return;
        }
        Debug.assert(this._state < StateClosing);

        this._adapter = adapter;

        if(this._adapter !== null)
        {
            this._servantManager = this._adapter.getServantManager();
            if(this._servantManager === null)
            {
                this._adapter = null;
            }
        }
        else
        {
            this._servantManager = null;
        }
    },
    getAdapter: function()
    {
        return this._adapter;
    },
    getEndpoint: function()
    {
        return this._endpoint;
    },
    createProxy: function(ident)
    {
        //
        // Create a reference and return a reverse proxy for this
        // reference.
        //
        return this._instance.proxyFactory().referenceToProxy(
            this._instance.referenceFactory().createFixed(ident, this));
    },
    message: function(operation)
    {
        if(this._state >= StateClosed)
        {
            return;
        }

        this.unscheduleTimeout(operation);

        //
        // Keep reading until no more data is available.
        //
        this._hasMoreData.value = (operation & SocketOperation.Read) !== 0;

        var info = null;
        try
        {
            if((operation & SocketOperation.Write) !== 0 && this._writeStream.buffer.remaining > 0)
            {
                if(!this.write(this._writeStream.buffer))
                {
                    Debug.assert(!this._writeStream.isEmpty());
                    this.scheduleTimeout(SocketOperation.Write, this._endpoint.timeout());
                    return;
                }
                Debug.assert(this._writeStream.buffer.remaining === 0);
            }
            if((operation & SocketOperation.Read) !== 0 && !this._readStream.isEmpty())
            {
                if(this._readHeader) // Read header if necessary.
                {
                    if(!this.read(this._readStream.buffer))
                    {
                        //
                        // We didn't get enough data to complete the header.
                        //
                        return;
                    }

                    Debug.assert(this._readStream.buffer.remaining === 0);
                    this._readHeader = false;

                    var pos = this._readStream.pos;
                    if(pos < Protocol.headerSize)
                    {
                        //
                        // This situation is possible for small UDP packets.
                        //
                        throw new Ice.IllegalMessageSizeException();
                    }

                    this._readStream.pos = 0;
                    var magic0 = this._readStream.readByte();
                    var magic1 = this._readStream.readByte();
                    var magic2 = this._readStream.readByte();
                    var magic3 = this._readStream.readByte();
                    if(magic0 !== Protocol.magic[0] || magic1 !== Protocol.magic[1] ||
                        magic2 !== Protocol.magic[2] || magic3 !== Protocol.magic[3])
                    {
                        var bme = new Ice.BadMagicException();
                        bme.badMagic = Ice.Buffer.createNative([magic0, magic1, magic2, magic3]);
                        throw bme;
                    }

                    this._readProtocol.__read(this._readStream);
                    Protocol.checkSupportedProtocol(this._readProtocol);

                    this._readProtocolEncoding.__read(this._readStream);
                    Protocol.checkSupportedProtocolEncoding(this._readProtocolEncoding);

                    this._readStream.readByte(); // messageType
                    this._readStream.readByte(); // compress
                    var size = this._readStream.readInt();
                    if(size < Protocol.headerSize)
                    {
                        throw new Ice.IllegalMessageSizeException();
                    }
                    if(size > this._messageSizeMax)
                    {
                        ExUtil.throwMemoryLimitException(size, this._messageSizeMax);
                    }
                    if(size > this._readStream.size)
                    {
                        this._readStream.resize(size);
                    }
                    this._readStream.pos = pos;
                }

                if(this._readStream.pos != this._readStream.size)
                {
                    if(this._endpoint.datagram())
                    {
                        throw new Ice.DatagramLimitException(); // The message was truncated.
                    }
                    else
                    {
                        if(!this.read(this._readStream.buffer))
                        {
                            Debug.assert(!this._readStream.isEmpty());
                            this.scheduleTimeout(SocketOperation.Read, this._endpoint.timeout());
                            return;
                        }
                        Debug.assert(this._readStream.buffer.remaining === 0);
                    }
                }
            }

            if(this._state <= StateNotValidated)
            {
                if(this._state === StateNotInitialized && !this.initialize())
                {
                    return;
                }

                if(this._state <= StateNotValidated && !this.validate())
                {
                    return;
                }

                this._transceiver.unregister();

                //
                // We start out in holding state.
                //
                this.setState(StateHolding);
                if(this._startPromise !== null)
                {
                    ++this._dispatchCount;
                }
            }
            else
            {
                Debug.assert(this._state <= StateClosing);

                //
                // We parse messages first, if we receive a close
                // connection message we won't send more messages.
                //
                if((operation & SocketOperation.Read) !== 0)
                {
                    info = this.parseMessage();
                }

                if((operation & SocketOperation.Write) !== 0)
                {
                    this.sendNextMessage();
                }
            }
        }
        catch(ex)
        {
            if(ex instanceof Ice.DatagramLimitException) // Expected.
            {
                if(this._warnUdp)
                {
                    this._logger.warning("maximum datagram size of " + this._readStream.pos + " exceeded");
                }
                this._readStream.resize(Protocol.headerSize);
                this._readStream.pos = 0;
                this._readHeader = true;
                return;
            }
            else if(ex instanceof Ice.SocketException)
            {
                this.setState(StateClosed, ex);
                return;
            }
            else if(ex instanceof Ice.LocalException)
            {
                if(this._endpoint.datagram())
                {
                    if(this._warn)
                    {
                        this._logger.warning("datagram connection exception:\n" + ex + '\n' + this._desc);
                    }
                    this._readStream.resize(Protocol.headerSize);
                    this._readStream.pos = 0;
                    this._readHeader = true;
                }
                else
                {
                    this.setState(StateClosed, ex);
                }
                return;
            }
            else
            {
                throw ex;
            }
        }

        if(this._acmLastActivity > 0)
        {
            this._acmLastActivity = Date.now();
        }

        this.dispatch(info);

        if(this._hasMoreData.value)
        {
            var self = this;
            Timer.setImmediate(function() { self.message(SocketOperation.Read); }); // Don't tie up the thread.
        }
    },
    dispatch: function(info)
    {
        var count = 0;
        //
        // Notify the factory that the connection establishment and
        // validation has completed.
        //
        if(this._startPromise !== null)
        {
            this._startPromise.succeed();
            this._startPromise = null;
            ++count;
        }

        if(info !== null)
        {
            if(info.outAsync !== null)
            {
                info.outAsync.__completed(info.stream);
                ++count;
            }

            if(info.invokeNum > 0)
            {
                this.invokeAll(info.stream, info.invokeNum, info.requestId, info.compress, info.servantManager,
                            info.adapter);

                //
                // Don't increase count, the dispatch count is
                // decreased when the incoming reply is sent.
                //
            }

            if(info.heartbeatCallback)
            {
                try
                {
                    info.heartbeatCallback(this);
                }
                catch(ex)
                {
                    this._logger.error("connection callback exception:\n" + ex + '\n' + this._desc);
                }
                info.heartbeatCallback = null;
                ++count;
            }
        }

        //
        // Decrease dispatch count.
        //
        if(count > 0)
        {
            this._dispatchCount -= count;
            if(this._dispatchCount === 0)
            {
                if(this._state === StateClosing && !this._shutdownInitiated)
                {
                    try
                    {
                        this.initiateShutdown();
                    }
                    catch(ex)
                    {
                        if(ex instanceof Ice.LocalException)
                        {
                            this.setState(StateClosed, ex);
                        }
                        else
                        {
                            throw ex;
                        }
                    }
                }
                else if(this._state === StateFinished)
                {
                    this.reap();
                }
                this.checkState();
            }
        }
    },
    finish: function()
    {
        Debug.assert(this._state === StateClosed);
        this.unscheduleTimeout(SocketOperation.Read | SocketOperation.Write | SocketOperation.Connect);

        var s;
        var traceLevels = this._instance.traceLevels();
        if(!this._initialized)
        {
            if(traceLevels.network >= 2)
            {
                s = [];
                s.push("failed to establish ");
                s.push(this._endpoint.protocol());
                s.push(" connection\n");
                s.push(this.toString());
                s.push("\n");
                s.push(this._exception.toString());
                this._instance.initializationData().logger.trace(traceLevels.networkCat, s.join(""));
            }
        }
        else
        {
            if(traceLevels.network >= 1)
            {
                s = [];
                s.push("closed ");
                s.push(this._endpoint.protocol());
                s.push(" connection\n");
                s.push(this.toString());

                //
                // Trace the cause of unexpected connection closures
                //
                if(!(this._exception instanceof Ice.CloseConnectionException ||
                     this._exception instanceof Ice.ForcedCloseConnectionException ||
                     this._exception instanceof Ice.ConnectionTimeoutException ||
                     this._exception instanceof Ice.CommunicatorDestroyedException ||
                     this._exception instanceof Ice.ObjectAdapterDeactivatedException))
                {
                    s.push("\n");
                    s.push(this._exception.toString());
                }

                this._instance.initializationData().logger.trace(traceLevels.networkCat, s.join(""));
            }
        }

        if(this._startPromise !== null)
        {
            this._startPromise.fail(this._exception);
            this._startPromise = null;
        }

        if(this._sendStreams.length > 0)
        {
            if(!this._writeStream.isEmpty())
            {
                //
                // Return the stream to the outgoing call. This is important for
                // retriable AMI calls which are not marshalled again.
                //
                var message = this._sendStreams[0];
                this._writeStream.swap(message.stream);
            }

            //
            // NOTE: for twoway requests which are not sent, finished can be called twice: the
            // first time because the outgoing is in the _sendStreams set and the second time
            // because it's either in the _requests/_asyncRequests set. This is fine, only the
            // first call should be taken into account by the implementation of finished.
            //
            for(var i = 0; i < this._sendStreams.length; ++i)
            {
                var p = this._sendStreams[i];
                if(p.requestId > 0)
                {
                    this._asyncRequests.delete(p.requestId);
                }
                p.completed(this._exception);
            }
            this._sendStreams = [];
        }

        for(var e = this._asyncRequests.entries; e !== null; e = e.next)
        {
            e.value.__completedEx(this._exception);
        }
        this._asyncRequests.clear();

        //
        // Don't wait to be reaped to reclaim memory allocated by read/write streams.
        //
        this._readStream.clear();
        this._readStream.buffer.clear();
        this._writeStream.clear();
        this._writeStream.buffer.clear();

        if(this._closeCallback !== null)
        {
            try
            {
                this._closeCallback(this);
            }
            catch(ex)
            {
                this._logger.error("connection callback exception:\n" + ex + '\n' + this._desc);
            }
            this._closeCallback = null;
        }

        this._heartbeatCallback = null;

        //
        // This must be done last as this will cause waitUntilFinished() to return (and communicator
        // objects such as the timer might be destroyed too).
        //
        if(this._dispatchCount === 0)
        {
            this.reap();
        }
        this.setState(StateFinished);
    },
    toString: function()
    {
        return this._desc;
    },
    timedOut: function(event)
    {
        if(this._state <= StateNotValidated)
        {
            this.setState(StateClosed, new Ice.ConnectTimeoutException());
        }
        else if(this._state < StateClosing)
        {
            this.setState(StateClosed, new Ice.TimeoutException());
        }
        else if(this._state === StateClosing)
        {
            this.setState(StateClosed, new Ice.CloseTimeoutException());
        }
    },
    type: function()
    {
        return this._type;
    },
    timeout: function()
    {
        return this._endpoint.timeout();
    },
    getInfo: function()
    {
        if(this._state >= StateClosed)
        {
            throw this._exception;
        }
        var info = this._transceiver.getInfo();
        info.adapterName = this._adapter !== null ? this._adapter.getName() : "";
        info.incoming = this._incoming;
        return info;
    },
    setBufferSize: function(rcvSize, sndSize)
    {
        if(this._state >= StateClosed)
        {
            throw this._exception;
        }
        this._transceiver.setBufferSize(rcvSize, sndSize);
    },
    exception: function(ex)
    {
        this.setState(StateClosed, ex);
    },
    invokeException: function(ex, invokeNum)
    {
        //
        // Fatal exception while invoking a request. Since sendResponse/sendNoResponse isn't
        // called in case of a fatal exception we decrement this._dispatchCount here.
        //

        this.setState(StateClosed, ex);

        if(invokeNum > 0)
        {
            Debug.assert(this._dispatchCount > 0);
            this._dispatchCount -= invokeNum;
            Debug.assert(this._dispatchCount >= 0);
            if(this._dispatchCount === 0)
            {
                if(this._state === StateFinished)
                {
                    this.reap();
                }
                this.checkState();
            }
        }
    },
    setState: function(state, ex)
    {
        if(ex !== undefined)
        {
            Debug.assert(ex instanceof Ice.LocalException);

            //
            // If setState() is called with an exception, then only closed
            // and closing states are permissible.
            //
            Debug.assert(state >= StateClosing);

            if(this._state === state) // Don't switch twice.
            {
                return;
            }

            if(this._exception === null)
            {
                this._exception = ex;

                //
                // We don't warn if we are not validated.
                //
                if(this._warn && this._validated)
                {
                    //
                    // Don't warn about certain expected exceptions.
                    //
                    if(!(this._exception instanceof Ice.CloseConnectionException ||
                         this._exception instanceof Ice.ForcedCloseConnectionException ||
                         this._exception instanceof Ice.ConnectionTimeoutException ||
                         this._exception instanceof Ice.CommunicatorDestroyedException ||
                         this._exception instanceof Ice.ObjectAdapterDeactivatedException ||
                         (this._exception instanceof Ice.ConnectionLostException && this._state === StateClosing)))
                    {
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
        // We don't want to send close connection messages if the endpoint
        // only supports oneway transmission from client to server.
        //
        if(this._endpoint.datagram() && state === StateClosing)
        {
            state = StateClosed;
        }

        //
        // Skip graceful shutdown if we are destroyed before validation.
        //
        if(this._state <= StateNotValidated && state === StateClosing)
        {
            state = StateClosed;
        }

        if(this._state === state) // Don't switch twice.
        {
            return;
        }

        try
        {
            switch(state)
            {
            case StateNotInitialized:
            {
                Debug.assert(false);
                break;
            }

            case StateNotValidated:
            {
                if(this._state !== StateNotInitialized)
                {
                    Debug.assert(this._state === StateClosed);
                    return;
                }
                //
                // Register to receive validation message.
                //
                if(!this._endpoint.datagram() && !this._incoming)
                {
                    //
                    // Once validation is complete, a new connection starts out in the
                    // Holding state. We only want to register the transceiver now if we
                    // need to receive data in order to validate the connection.
                    //
                    this._transceiver.register();
                }
                break;
            }

            case StateActive:
            {
                //
                // Can only switch from holding or not validated to
                // active.
                //
                if(this._state !== StateHolding && this._state !== StateNotValidated)
                {
                    return;
                }
                this._transceiver.register();
                break;
            }

            case StateHolding:
            {
                //
                // Can only switch from active or not validated to
                // holding.
                //
                if(this._state !== StateActive && this._state !== StateNotValidated)
                {
                    return;
                }
                if(this._state === StateActive)
                {
                    this._transceiver.unregister();
                }
                break;
            }

            case StateClosing:
            {
                //
                // Can't change back from closed.
                //
                if(this._state >= StateClosed)
                {
                    return;
                }
                if(this._state === StateHolding)
                {
                    // We need to continue to read in closing state.
                    this._transceiver.register();
                }
                break;
            }

            case StateClosed:
            {
                if(this._state === StateFinished)
                {
                    return;
                }
                this._batchRequestQueue.destroy(this._exception);
                this._transceiver.unregister();
                break;
            }

            case StateFinished:
            {
                Debug.assert(this._state === StateClosed);
                this._transceiver.close();
                this._communicator = null;
                break;
            }
            }
        }
        catch(ex)
        {
            if(ex instanceof Ice.LocalException)
            {
                var msg = "unexpected connection exception:\n " + this._desc + "\n" + ex.toString();
                this._instance.initializationData().logger.error(msg);
            }
            else
            {
                throw ex;
            }
        }

        //
        // We only register with the connection monitor if our new state
        // is StateActive. Otherwise we unregister with the connection
        // monitor, but only if we were registered before, i.e., if our
        // old state was StateActive.
        //
        if(this._monitor !== null)
        {
            if(state === StateActive)
            {
                this._monitor.add(this);
                if(this._acmLastActivity > 0)
                {
                    this._acmLastActivity = Date.now();
                }
            }
            else if(this._state === StateActive)
            {
                this._monitor.remove(this);
            }
        }

        this._state = state;

        if(this._state === StateClosing && this._dispatchCount === 0)
        {
            try
            {
                this.initiateShutdown();
            }
            catch(ex)
            {
                if(ex instanceof Ice.LocalException)
                {
                    this.setState(StateClosed, ex);
                }
                else
                {
                    throw ex;
                }
            }
        }
        else if(this._state === StateClosed)
        {
            this.finish();
        }

        this.checkState();
    },
    initiateShutdown: function()
    {
        Debug.assert(this._state === StateClosing);
        Debug.assert(this._dispatchCount === 0);
        Debug.assert(!this._shutdownInitiated);

        if(!this._endpoint.datagram())
        {
            //
            // Before we shut down, we send a close connection
            // message.
            //
            var os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
            os.writeBlob(Protocol.magic);
            Protocol.currentProtocol.__write(os);
            Protocol.currentProtocolEncoding.__write(os);
            os.writeByte(Protocol.closeConnectionMsg);
            os.writeByte(0); // compression status: always report 0 for CloseConnection.
            os.writeInt(Protocol.headerSize); // Message size.

            var status = this.sendMessage(OutgoingMessage.createForStream(os, false, false));
            if((status & AsyncStatus.Sent) > 0)
            {
                //
                // Schedule the close timeout to wait for the peer to close the connection.
                //
                this.scheduleTimeout(SocketOperation.Write, this.closeTimeout());
            }

            //
            // The CloseConnection message should be sufficient. Closing the write
            // end of the socket is probably an artifact of how things were done
            // in IIOP. In fact, shutting down the write end of the socket causes
            // problems on Windows by preventing the peer from using the socket.
            // For example, the peer is no longer able to continue writing a large
            // message after the socket is shutdown.
            //
            //this._transceiver.shutdownWrite();
        }
    },
    heartbeat: function()
    {
        Debug.assert(this._state === StateActive);

        if(!this._endpoint.datagram())
        {
            var os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
            os.writeBlob(Protocol.magic);
            Protocol.currentProtocol.__write(os);
            Protocol.currentProtocolEncoding.__write(os);
            os.writeByte(Protocol.validateConnectionMsg);
            os.writeByte(0);
            os.writeInt(Protocol.headerSize); // Message size.
            try
            {
                this.sendMessage(OutgoingMessage.createForStream(os, false, false));
            }
            catch(ex)
            {
                this.setState(StateClosed, ex);
                Debug.assert(this._exception !== null);
            }
        }
    },
    initialize: function()
    {
        var s = this._transceiver.initialize(this._readStream.buffer, this._writeStream.buffer);
        if(s != SocketOperation.None)
        {
            this.scheduleTimeout(s, this.connectTimeout());
            return false;
        }

        //
        // Update the connection description once the transceiver is initialized.
        //
        this._desc = this._transceiver.toString();
        this._initialized = true;
        this.setState(StateNotValidated);
        return true;
    },
    validate: function()
    {
        if(!this._endpoint.datagram()) // Datagram connections are always implicitly validated.
        {
            if(this._adapter !== null) // The server side has the active role for connection validation.
            {
                if(this._writeStream.size === 0)
                {
                    this._writeStream.writeBlob(Protocol.magic);
                    Protocol.currentProtocol.__write(this._writeStream);
                    Protocol.currentProtocolEncoding.__write(this._writeStream);
                    this._writeStream.writeByte(Protocol.validateConnectionMsg);
                    this._writeStream.writeByte(0); // Compression status (always zero for validate connection).
                    this._writeStream.writeInt(Protocol.headerSize); // Message size.
                    TraceUtil.traceSend(this._writeStream, this._logger, this._traceLevels);
                    this._writeStream.prepareWrite();
                }

                if(this._writeStream.pos != this._writeStream.size && !this.write(this._writeStream.buffer))
                {
                    this.scheduleTimeout(SocketOperation.Write, this.connectTimeout());
                    return false;
                }
            }
            else // The client side has the passive role for connection validation.
            {
                if(this._readStream.size === 0)
                {
                    this._readStream.resize(Protocol.headerSize);
                    this._readStream.pos = 0;
                }

                if(this._readStream.pos !== this._readStream.size &&
                    !this.read(this._readStream.buffer))
                {
                    this.scheduleTimeout(SocketOperation.Read, this.connectTimeout());
                    return false;
                }

                Debug.assert(this._readStream.pos === Protocol.headerSize);
                this._readStream.pos = 0;
                var m = this._readStream.readBlob(4);
                if(m[0] !== Protocol.magic[0] || m[1] !== Protocol.magic[1] ||
                    m[2] !== Protocol.magic[2] || m[3] !== Protocol.magic[3])
                {
                    var bme = new Ice.BadMagicException();
                    bme.badMagic = m;
                    throw bme;
                }

                this._readProtocol.__read(this._readStream);
                Protocol.checkSupportedProtocol(this._readProtocol);

                this._readProtocolEncoding.__read(this._readStream);
                Protocol.checkSupportedProtocolEncoding(this._readProtocolEncoding);

                var messageType = this._readStream.readByte();
                if(messageType !== Protocol.validateConnectionMsg)
                {
                    throw new Ice.ConnectionNotValidatedException();
                }
                this._readStream.readByte(); // Ignore compression status for validate connection.
                var size = this._readStream.readInt();
                if(size !== Protocol.headerSize)
                {
                    throw new Ice.IllegalMessageSizeException();
                }
                TraceUtil.traceRecv(this._readStream, this._logger, this._traceLevels);
                this._validated = true;
            }
        }

        this._writeStream.resize(0);
        this._writeStream.pos = 0;

        this._readStream.resize(Protocol.headerSize);
        this._readHeader = true;
        this._readStream.pos = 0;

        var traceLevels = this._instance.traceLevels();
        if(traceLevels.network >= 1)
        {
            var s = [];
            if(this._endpoint.datagram())
            {
                s.push("starting to send ");
                s.push(this._endpoint.protocol());
                s.push(" messages\n");
                s.push(this._transceiver.toDetailedString());
            }
            else
            {
                s = [];
                s.push("established ");
                s.push(this._endpoint.protocol());
                s.push(" connection\n");
                s.push(this.toString());
            }
            this._instance.initializationData().logger.trace(traceLevels.networkCat, s.join(""));
        }

        return true;
    },
    sendNextMessage: function()
    {
        if(this._sendStreams.length === 0)
        {
            return;
        }

        Debug.assert(!this._writeStream.isEmpty() && this._writeStream.pos === this._writeStream.size);
        try
        {
            while(true)
            {
                //
                // Notify the message that it was sent.
                //
                var message = this._sendStreams.shift();
                this._writeStream.swap(message.stream);
                message.sent();

                //
                // If there's nothing left to send, we're done.
                //
                if(this._sendStreams.length === 0)
                {
                    break;
                }

                //
                // If we are in the closed state, don't continue sending.
                //
                // The connection can be in the closed state if parseMessage
                // (called before sendNextMessage by message()) closes the
                // connection.
                //
                if(this._state >= StateClosed)
                {
                    return;
                }

                //
                // Otherwise, prepare the next message stream for writing.
                //
                message = this._sendStreams[0];
                Debug.assert(!message.prepared);
                var stream = message.stream;

                stream.pos = 10;
                stream.writeInt(stream.size);
                stream.prepareWrite();
                message.prepared = true;

                if(message.outAsync !== null)
                {
                    TraceUtil.traceOut("sending asynchronous request", stream, this._logger, this._traceLevels);
                }
                else
                {
                    TraceUtil.traceSend(stream, this._logger, this._traceLevels);
                }
                this._writeStream.swap(message.stream);

                //
                // Send the message.
                //
                if(this._writeStream.pos != this._writeStream.size && !this.write(this._writeStream.buffer))
                {
                    Debug.assert(!this._writeStream.isEmpty());
                    this.scheduleTimeout(SocketOperation.Write, this._endpoint.timeout());
                    return;
                }
            }
        }
        catch(ex)
        {
            if(ex instanceof Ice.LocalException)
            {
                this.setState(StateClosed, ex);
                return;
            }
            else
            {
                throw ex;
            }
        }

        Debug.assert(this._writeStream.isEmpty());

        //
        // If all the messages were sent and we are in the closing state, we schedule
        // the close timeout to wait for the peer to close the connection.
        //
        if(this._state === StateClosing)
        {
            this.scheduleTimeout(SocketOperation.Write, this.closeTimeout());
        }
    },
    sendMessage: function(message)
    {
        if(this._sendStreams.length > 0)
        {
            message.doAdopt();
            this._sendStreams.push(message);
            return AsyncStatus.Queued;
        }
        Debug.assert(this._state < StateClosed);

        Debug.assert(!message.prepared);

        var stream = message.stream;
        stream.pos = 10;
        stream.writeInt(stream.size);
        stream.prepareWrite();
        message.prepared = true;

        if(message.outAsync)
        {
            TraceUtil.traceOut("sending asynchronous request", message.stream, this._logger, this._traceLevels);
        }
        else
        {
            TraceUtil.traceSend(message.stream, this._logger, this._traceLevels);
        }

        if(this.write(message.stream.buffer))
        {
            //
            // Entire buffer was written immediately.
            //
            message.sent();

            if(this._acmLastActivity > 0)
            {
                this._acmLastActivity = Date.now();
            }
            return AsyncStatus.Sent;
        }
        message.doAdopt();

        this._writeStream.swap(message.stream);
        this._sendStreams.push(message);
        this.scheduleTimeout(SocketOperation.Write, this._endpoint.timeout());

        return AsyncStatus.Queued;
    },
    parseMessage: function()
    {
        Debug.assert(this._state > StateNotValidated && this._state < StateClosed);

        var info = new MessageInfo(this._instance);

        this._readStream.swap(info.stream);
        this._readStream.resize(Protocol.headerSize);
        this._readStream.pos = 0;
        this._readHeader = true;

        //
        // Connection is validated on first message. This is only used by
        // setState() to check wether or not we can print a connection
        // warning (a client might close the connection forcefully if the
        // connection isn't validated).
        //
        this._validated = true;

        Debug.assert(info.stream.pos === info.stream.size);

        try
        {
            //
            // We don't need to check magic and version here. This has already
            // been done by the caller.
            //
            info.stream.pos = 8;
            var messageType = info.stream.readByte();
            info.compress = info.stream.readByte();
            if(info.compress === 2)
            {
                var ex = new Ice.FeatureNotSupportedException();
                ex.unsupportedFeature = "Cannot uncompress compressed message";
                throw ex;
            }
            info.stream.pos = Protocol.headerSize;

            switch(messageType)
            {
                case Protocol.closeConnectionMsg:
                {
                    TraceUtil.traceRecv(info.stream, this._logger, this._traceLevels);
                    if(this._endpoint.datagram())
                    {
                        if(this._warn)
                        {
                            this._logger.warning("ignoring close connection message for datagram connection:\n" +
                                                this._desc);
                        }
                    }
                    else
                    {
                        this.setState(StateClosed, new Ice.CloseConnectionException());
                    }
                    break;
                }

                case Protocol.requestMsg:
                {
                    if(this._state === StateClosing)
                    {
                        TraceUtil.traceIn("received request during closing\n" +
                                          "(ignored by server, client will retry)",
                                          info.stream, this._logger, this._traceLevels);
                    }
                    else
                    {
                        TraceUtil.traceRecv(info.stream, this._logger, this._traceLevels);
                        info.requestId = info.stream.readInt();
                        info.invokeNum = 1;
                        info.servantManager = this._servantManager;
                        info.adapter = this._adapter;
                        ++this._dispatchCount;
                    }
                    break;
                }

                case Protocol.requestBatchMsg:
                {
                    if(this._state === StateClosing)
                    {
                        TraceUtil.traceIn("received batch request during closing\n" +
                                          "(ignored by server, client will retry)",
                                          info.stream, this._logger, this._traceLevels);
                    }
                    else
                    {
                        TraceUtil.traceRecv(info.stream, this._logger, this._traceLevels);
                        info.invokeNum = info.stream.readInt();
                        if(info.invokeNum < 0)
                        {
                            info.invokeNum = 0;
                            throw new Ice.UnmarshalOutOfBoundsException();
                        }
                        info.servantManager = this._servantManager;
                        info.adapter = this._adapter;
                        this._dispatchCount += info.invokeNum;
                    }
                    break;
                }

                case Protocol.replyMsg:
                {
                    TraceUtil.traceRecv(info.stream, this._logger, this._traceLevels);
                    info.requestId = info.stream.readInt();
                    info.outAsync = this._asyncRequests.get(info.requestId);
                    if(info.outAsync)
                    {
                        this._asyncRequests.delete(info.requestId);
                        ++this._dispatchCount;
                    }
                    else
                    {
                        info = null;
                    }
                    this.checkClose();
                    break;
                }

                case Protocol.validateConnectionMsg:
                {
                    TraceUtil.traceRecv(info.stream, this._logger, this._traceLevels);
                    if(this._heartbeatCallback !== null)
                    {
                        info.heartbeatCallback = this._heartbeatCallback;
                        ++this._dispatchCount;
                    }
                    break;
                }

                default:
                {
                    TraceUtil.traceIn("received unknown message\n(invalid, closing connection)",
                                      info.stream, this._logger, this._traceLevels);
                    throw new Ice.UnknownMessageException();
                }
            }
        }
        catch(ex)
        {
            if(ex instanceof Ice.LocalException)
            {
                if(this._endpoint.datagram())
                {
                    if(this._warn)
                    {
                        this._logger.warning("datagram connection exception:\n" + ex + '\n' + this._desc);
                    }
                }
                else
                {
                    this.setState(StateClosed, ex);
                }
            }
            else
            {
                throw ex;
            }
        }

        return info;
    },
    invokeAll: function(stream, invokeNum, requestId, compress, servantManager, adapter)
    {
        var inc = null;
        try
        {
            while(invokeNum > 0)
            {
                //
                // Prepare the invocation.
                //
                var response = !this._endpoint.datagram() && requestId !== 0;
                inc = new IncomingAsync(this._instance, this, adapter, response, compress, requestId);

                //
                // Dispatch the invocation.
                //
                inc.invoke(servantManager, stream);

                --invokeNum;
                inc = null;
            }

            stream.clear();
        }
        catch(ex)
        {
            if(ex instanceof Ice.LocalException)
            {
                this.invokeException(ex, invokeNum);
            }
            else
            {
                throw ex;
            }
        }
    },
    scheduleTimeout: function(op, timeout)
    {
        if(timeout < 0)
        {
            return;
        }

        var self = this;
        if((op & SocketOperation.Read) !== 0)
        {
            this._readTimeoutId = this._timer.schedule(function() { self.timedOut(); }, timeout);
            this._readTimeoutScheduled = true;
        }
        if((op & (SocketOperation.Write | SocketOperation.Connect)) !== 0)
        {
            this._writeTimeoutId = this._timer.schedule(function() { self.timedOut(); }, timeout);
            this._writeTimeoutScheduled = true;
        }
    },
    unscheduleTimeout: function(op)
    {
        if((op & SocketOperation.Read) !== 0 && this._readTimeoutScheduled)
        {
            this._timer.cancel(this._readTimeoutId);
            this._readTimeoutScheduled = false;
        }
        if((op & (SocketOperation.Write | SocketOperation.Connect)) !== 0 && this._writeTimeoutScheduled)
        {
            this._timer.cancel(this._writeTimeoutId);
            this._writeTimeoutScheduled = false;
        }
    },
    connectTimeout: function()
    {
        var defaultsAndOverrides = this._instance.defaultsAndOverrides();
        if(defaultsAndOverrides.overrideConnectTimeout)
        {
            return defaultsAndOverrides.overrideConnectTimeoutValue;
        }
        else
        {
            return this._endpoint.timeout();
        }
    },
    closeTimeout: function()
    {
        var defaultsAndOverrides = this._instance.defaultsAndOverrides();
        if(defaultsAndOverrides.overrideCloseTimeout)
        {
            return defaultsAndOverrides.overrideCloseTimeoutValue;
        }
        else
        {
            return this._endpoint.timeout();
        }
    },
    warning: function(msg, ex)
    {
        this._logger.warning(msg + ":\n" + this._desc + "\n" + ex.toString());
    },
    checkState: function()
    {
        if(this._state < StateHolding || this._dispatchCount > 0)
        {
            return;
        }

        var i;
        if(this._holdPromises.length > 0)
        {
            for(i = 0; i < this._holdPromises.length; ++i)
            {
                this._holdPromises[i].succeed();
            }
            this._holdPromises = [];
        }

        //
        // We aren't finished until the state is finished and all
        // outstanding requests are completed. Otherwise we couldn't
        // guarantee that there are no outstanding calls when deactivate()
        // is called on the servant locators.
        //
        if(this._state === StateFinished && this._finishedPromises.length > 0)
        {
            //
            // Clear the OA. See bug 1673 for the details of why this is necessary.
            //
            this._adapter = null;

            for(i = 0; i < this._finishedPromises.length; ++i)
            {
                this._finishedPromises[i].succeed();
            }
            this._finishedPromises = [];
        }
    },
    reap: function()
    {
        if(this._monitor !== null)
        {
            this._monitor.reap(this);
        }
    },
    read: function(buf)
    {
        var start = buf.position;
        var ret = this._transceiver.read(buf, this._hasMoreData);
        if(this._instance.traceLevels().network >= 3 && buf.position != start)
        {
            var s = [];
            s.push("received ");
            if(this._endpoint.datagram())
            {
                s.push(buf.limit);
            }
            else
            {
                s.push(buf.position - start);
                s.push(" of ");
                s.push(buf.limit - start);
            }
            s.push(" bytes via ");
            s.push(this._endpoint.protocol());
            s.push("\n");
            s.push(this.toString());
            this._instance.initializationData().logger.trace(this._instance.traceLevels().networkCat, s.join(""));
        }
        return ret;
    },
    write: function(buf)
    {
        var start = buf.position;
        var ret = this._transceiver.write(buf);
        if(this._instance.traceLevels().network >= 3 && buf.position != start)
        {
            var s = [];
            s.push("sent ");
            s.push(buf.position - start);
            if(!this._endpoint.datagram())
            {
                s.push(" of ");
                s.push(buf.limit - start);
            }
            s.push(" bytes via ");
            s.push(this._endpoint.protocol());
            s.push("\n");
            s.push(this.toString());
            this._instance.initializationData().logger.trace(this._instance.traceLevels().networkCat, s.join(""));
        }
        return ret;
    }
});

// DestructionReason.
ConnectionI.ObjectAdapterDeactivated = 0;
ConnectionI.CommunicatorDestroyed = 1;

Ice.ConnectionI = ConnectionI;

var OutgoingMessage = Class({
    __init__: function()
    {
        this.stream = null;
        this.outAsync = null;
        this.compress = false;
        this.requestId = 0;
        this.prepared = false;
    },
    canceled: function()
    {
        Debug.assert(this.outAsync !== null);
        this.outAsync = null;
    },
    doAdopt: function()
    {
        if(this.adopt)
        {
            var stream = new OutputStream(this.stream.instance, Protocol.currentProtocolEncoding);
            stream.swap(this.stream);
            this.stream = stream;
            this.adopt = false;
        }
    },
    sent: function()
    {
        if(this.outAsync !== null)
        {
            this.outAsync.__sent();
        }
    },
    completed: function(ex)
    {
        if(this.outAsync !== null)
        {
            this.outAsync.__completedEx(ex);
        }
    }
});

OutgoingMessage.createForStream = function(stream, compress, adopt)
{
    var m = new OutgoingMessage();
    m.stream = stream;
    m.compress = compress;
    m.adopt = adopt;
    m.isSent = false;
    m.requestId = 0;
    m.outAsync = null;
    return m;
};

OutgoingMessage.create = function(out, stream, compress, requestId)
{
    var m = new OutgoingMessage();
    m.stream = stream;
    m.compress = compress;
    m.outAsync = out;
    m.requestId = requestId;
    m.isSent = false;
    m.adopt = false;
    return m;
};

module.exports.Ice = Ice;
