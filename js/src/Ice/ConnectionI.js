//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;
require("../Ice/AsyncStatus");
require("../Ice/Stream");
require("../Ice/OutgoingAsync");
require("../Ice/Debug");
require("../Ice/ExUtil");
require("../Ice/HashMap");
require("../Ice/IncomingAsync");
require("../Ice/RetryException");
require("../Ice/Promise");
require("../Ice/Protocol");
require("../Ice/SocketOperation");
require("../Ice/Timer");
require("../Ice/TraceUtil");
require("../Ice/Version");
require("../Ice/Exception");
require("../Ice/LocalException");
require("../Ice/BatchRequestQueue");

const AsyncStatus = Ice.AsyncStatus;
const AsyncResultBase = Ice.AsyncResultBase;
const InputStream = Ice.InputStream;
const OutputStream = Ice.OutputStream;
const BatchRequestQueue = Ice.BatchRequestQueue;
const ConnectionFlushBatch = Ice.ConnectionFlushBatch;
const HeartbeatAsync = Ice.HeartbeatAsync;
const Debug = Ice.Debug;
const ExUtil = Ice.ExUtil;
const HashMap = Ice.HashMap;
const IncomingAsync = Ice.IncomingAsync;
const RetryException = Ice.RetryException;
const Protocol = Ice.Protocol;
const SocketOperation = Ice.SocketOperation;
const Timer = Ice.Timer;
const TraceUtil = Ice.TraceUtil;
const ProtocolVersion = Ice.ProtocolVersion;
const EncodingVersion = Ice.EncodingVersion;
const ACM = Ice.ACM;
const ACMClose = Ice.ACMClose;
const ACMHeartbeat = Ice.ACMHeartbeat;
const ConnectionClose = Ice.ConnectionClose;

const StateNotInitialized = 0;
const StateNotValidated = 1;
const StateActive = 2;
const StateHolding = 3;
const StateClosing = 4;
const StateClosed = 5;
const StateFinished = 6;

class MessageInfo
{
    constructor(instance)
    {
        this.stream = new InputStream(instance, Protocol.currentProtocolEncoding);
        this.invokeNum = 0;
        this.requestId = 0;
        this.servantManager = null;
        this.adapter = null;
        this.outAsync = null;
        this.heartbeatCallback = null;
    }
}

class ConnectionI
{
    constructor(communicator, instance, monitor, transceiver, endpoint, incoming, adapter)
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
        const initData = instance.initializationData();
        this._logger = initData.logger; // Cached for better performance.
        this._traceLevels = instance.traceLevels(); // Cached for better performance.
        this._timer = instance.timer();
        this._writeTimeoutId = 0;
        this._writeTimeoutScheduled = false;
        this._readTimeoutId = 0;
        this._readTimeoutScheduled = false;

        this._hasMoreData = {value: false};

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
    }

    start()
    {
        Debug.assert(this._startPromise === null);

        try
        {
            // The connection might already be closed if the communicator was destroyed.
            if(this._state >= StateClosed)
            {
                Debug.assert(this._exception !== null);
                return Ice.Promise.reject(this._exception);
            }

            this._startPromise = new Ice.Promise();
            this._transceiver.setCallbacks(
                () => this.message(SocketOperation.Write), // connected callback
                () => this.message(SocketOperation.Read), // read callback
                () => this.message(SocketOperation.Write) // write callback
            );
            this.initialize();
        }
        catch(ex)
        {
            const startPromise = this._startPromise;
            this.exception(ex);
            return startPromise;
        }
        return this._startPromise;
    }

    activate()
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
    }

    hold()
    {
        if(this._state <= StateNotValidated)
        {
            return;
        }

        this.setState(StateHolding);
    }

    destroy(reason)
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

            default:
            {
                Debug.assert(false);
                break;
            }
        }
    }

    close(mode)
    {
        const r = new AsyncResultBase(this._communicator, "close", this, null, null);

        if(mode == ConnectionClose.Forcefully)
        {
            this.setState(StateClosed, new Ice.ConnectionManuallyClosedException(false));
            r.resolve();
        }
        else if(mode == ConnectionClose.Gracefully)
        {
            this.setState(StateClosing, new Ice.ConnectionManuallyClosedException(true));
            r.resolve();
        }
        else
        {
            Debug.assert(mode == ConnectionClose.GracefullyWithWait);

            //
            // Wait until all outstanding requests have been completed.
            //
            this._closePromises.push(r);
            this.checkClose();
        }

        return r;
    }

    checkClose()
    {
        //
        // If close(GracefullyWithWait) has been called, then we need to check if all
        // requests have completed and we can transition to StateClosing. We also
        // complete outstanding promises.
        //
        if(this._asyncRequests.size === 0 && this._closePromises.length > 0)
        {
            //
            // The caller doesn't expect the state of the connection to change when this is called so
            // we defer the check immediately after doing whather we're doing. This is consistent with
            // other implementations as well.
            //
            Timer.setImmediate(() =>
            {
                this.setState(StateClosing, new Ice.ConnectionManuallyClosedException(true));
                this._closePromises.forEach(p => p.resolve());
                this._closePromises = [];
            });
        }
    }

    isActiveOrHolding()
    {
        return this._state > StateNotValidated && this._state < StateClosing;
    }

    isFinished()
    {
        if(this._state !== StateFinished || this._dispatchCount !== 0)
        {
            return false;
        }

        Debug.assert(this._state === StateFinished);
        return true;
    }

    throwException()
    {
        if(this._exception !== null)
        {
            Debug.assert(this._state >= StateClosing);
            throw this._exception;
        }
    }

    waitUntilFinished()
    {
        const promise = new Ice.Promise();
        this._finishedPromises.push(promise);
        this.checkState();
        return promise;
    }

    monitor(now, acm)
    {
        if(this._state !== StateActive)
        {
            return;
        }

        //
        // We send a heartbeat if there was no activity in the last
        // (timeout / 4) period. Sending a heartbeat sooner than
        // really needed is safer to ensure that the receiver will
        // receive the heartbeat in time. Sending the heartbeat if
        // there was no activity in the last (timeout / 2) period
        // isn't enough since monitor() is called only every (timeout
        // / 2) period.
        //
        // Note that this doesn't imply that we are sending 4 heartbeats
        // per timeout period because the monitor() method is still only
        // called every (timeout / 2) period.
        //
        if(acm.heartbeat == Ice.ACMHeartbeat.HeartbeatAlways ||
           (acm.heartbeat != Ice.ACMHeartbeat.HeartbeatOff && this._writeStream.isEmpty() &&
            now >= (this._acmLastActivity + (acm.timeout / 4))))
        {
            if(acm.heartbeat != Ice.ACMHeartbeat.HeartbeatOnDispatch || this._dispatchCount > 0)
            {
                this.sendHeartbeatNow(); // Send heartbeat if idle in the last timeout / 2 period.
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
    }

    sendAsyncRequest(out, response, batchRequestNum)
    {
        let requestId = 0;
        const ostr = out.getOs();

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
        this._transceiver.checkSendSize(ostr);

        //
        // Notify the request that it's cancelable with this connection.
        // This will throw if the request is canceled.
        //
        out.cancelable(this); // Notify the request that it's cancelable

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
            ostr.pos = Protocol.headerSize;
            ostr.writeInt(requestId);
        }
        else if(batchRequestNum > 0)
        {
            ostr.pos = Protocol.headerSize;
            ostr.writeInt(batchRequestNum);
        }

        let status;
        try
        {
            status = this.sendMessage(OutgoingMessage.create(out, out.getOs(), requestId));
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
    }

    getBatchRequestQueue()
    {
        return this._batchRequestQueue;
    }

    flushBatchRequests()
    {
        const result = new ConnectionFlushBatch(this, this._communicator, "flushBatchRequests");
        result.invoke();
        return result;
    }

    setCloseCallback(callback)
    {
        if(this._state >= StateClosed)
        {
            if(callback !== null)
            {
                Timer.setImmediate(() =>
                {
                    try
                    {
                        callback(this);
                    }
                    catch(ex)
                    {
                        this._logger.error("connection callback exception:\n" + ex + '\n' + this._desc);
                    }
                });
            }
        }
        else
        {
            this._closeCallback = callback;
        }
    }

    setHeartbeatCallback(callback)
    {
        if(this._state >= StateClosed)
        {
            return;
        }
        this._heartbeatCallback = callback;
    }

    heartbeat()
    {
        const result = new HeartbeatAsync(this, this._communicator);
        result.invoke();
        return result;
    }

    setACM(timeout, close, heartbeat)
    {
        if(timeout !== undefined && timeout < 0)
        {
            throw new RangeError("invalid negative ACM timeout value");
        }
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
    }

    getACM()
    {
        return this._monitor !== null ? this._monitor.getACM() :
            new ACM(0, ACMClose.CloseOff, ACMHeartbeat.HeartbeatOff);
    }

    asyncRequestCanceled(outAsync, ex)
    {
        for(let i = 0; i < this._sendStreams.length; i++)
        {
            const o = this._sendStreams[i];
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
                outAsync.completedEx(ex);
                this.checkClose();
                return; // We're done.
            }
        }

        if(outAsync instanceof Ice.OutgoingAsync)
        {
            for(const [key, value] of this._asyncRequests)
            {
                if(value === outAsync)
                {
                    this._asyncRequests.delete(key);
                    outAsync.completedEx(ex);
                    this.checkClose();
                    return; // We're done.
                }
            }
        }
    }

    sendResponse(os)
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

            this.sendMessage(OutgoingMessage.createForStream(os, true));

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
    }

    sendNoResponse()
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
    }

    endpoint()
    {
        return this._endpoint;
    }

    setAdapter(adapter)
    {
        if(adapter !== null)
        {
            adapter.checkForDeactivation();
            if(this._state <= StateNotValidated || this._state >= StateClosing)
            {
                return;
            }
            this._adapter = adapter;
            this._servantManager = adapter.getServantManager(); // The OA's servant manager is immutable.
        }
        else
        {
            if(this._state <= StateNotValidated || this._state >= StateClosing)
            {
                return;
            }
            this._adapter = null;
            this._servantManager = null;
        }
    }

    getAdapter()
    {
        return this._adapter;
    }

    getEndpoint()
    {
        return this._endpoint;
    }

    createProxy(ident)
    {
        //
        // Create a reference and return a reverse proxy for this
        // reference.
        //
        return this._instance.proxyFactory().referenceToProxy(
            this._instance.referenceFactory().createFixed(ident, this));
    }

    message(operation)
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

        let info = null;
        try
        {
            if((operation & SocketOperation.Write) !== 0 && this._writeStream.buffer.remaining > 0)
            {
                if(!this.write(this._writeStream.buffer))
                {
                    Debug.assert(!this._writeStream.isEmpty());
                    this.scheduleTimeout(SocketOperation.Write);
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

                    //
                    // Connection is validated on first message. This is only used by
                    // setState() to check wether or not we can print a connection
                    // warning (a client might close the connection forcefully if the
                    // connection isn't validated, we don't want to print a warning
                    // in this case).
                    //
                    this._validated = true;

                    const pos = this._readStream.pos;
                    if(pos < Protocol.headerSize)
                    {
                        //
                        // This situation is possible for small UDP packets.
                        //
                        throw new Ice.IllegalMessageSizeException();
                    }

                    this._readStream.pos = 0;
                    const magic0 = this._readStream.readByte();
                    const magic1 = this._readStream.readByte();
                    const magic2 = this._readStream.readByte();
                    const magic3 = this._readStream.readByte();
                    if(magic0 !== Protocol.magic[0] || magic1 !== Protocol.magic[1] ||
                       magic2 !== Protocol.magic[2] || magic3 !== Protocol.magic[3])
                    {
                        throw new Ice.BadMagicException("", new Uint8Array([magic0, magic1, magic2, magic3]));
                    }

                    this._readProtocol._read(this._readStream);
                    Protocol.checkSupportedProtocol(this._readProtocol);

                    this._readProtocolEncoding._read(this._readStream);
                    Protocol.checkSupportedProtocolEncoding(this._readProtocolEncoding);

                    this._readStream.readByte(); // messageType
                    this._readStream.readByte(); // compress
                    const size = this._readStream.readInt();
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
                            this.scheduleTimeout(SocketOperation.Read);
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
            Timer.setImmediate(() => this.message(SocketOperation.Read)); // Don't tie up the thread.
        }
    }

    dispatch(info)
    {
        let count = 0;
        //
        // Notify the factory that the connection establishment and
        // validation has completed.
        //
        if(this._startPromise !== null)
        {
            this._startPromise.resolve();

            this._startPromise = null;
            ++count;
        }

        if(info !== null)
        {
            if(info.outAsync !== null)
            {
                info.outAsync.completed(info.stream);
                ++count;
            }

            if(info.invokeNum > 0)
            {
                this.invokeAll(info.stream, info.invokeNum, info.requestId, info.servantManager, info.adapter);

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
                if(this._state === StateClosing)
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
    }

    finish()
    {
        Debug.assert(this._state === StateClosed);
        this.unscheduleTimeout(SocketOperation.Read | SocketOperation.Write | SocketOperation.Connect);

        const traceLevels = this._instance.traceLevels();
        if(!this._initialized)
        {
            if(traceLevels.network >= 2)
            {
                const s = [];
                s.push("failed to establish ");
                s.push(this._endpoint.protocol());
                s.push(" connection\n");
                s.push(this.toString());
                s.push("\n");
                s.push(this._exception.toString());
                this._instance.initializationData().logger.trace(traceLevels.networkCat, s.join(""));
            }
        }
        else if(traceLevels.network >= 1)
        {
            const s = [];
            s.push("closed ");
            s.push(this._endpoint.protocol());
            s.push(" connection\n");
            s.push(this.toString());

            //
            // Trace the cause of unexpected connection closures
            //
            if(!(this._exception instanceof Ice.CloseConnectionException ||
                 this._exception instanceof Ice.ConnectionManuallyClosedException ||
                 this._exception instanceof Ice.ConnectionTimeoutException ||
                 this._exception instanceof Ice.CommunicatorDestroyedException ||
                 this._exception instanceof Ice.ObjectAdapterDeactivatedException))
            {
                s.push("\n");
                s.push(this._exception.toString());
            }

            this._instance.initializationData().logger.trace(traceLevels.networkCat, s.join(""));
        }

        if(this._startPromise !== null)
        {
            this._startPromise.reject(this._exception);
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
                this._writeStream.swap(this._sendStreams[0].stream);
            }

            //
            // NOTE: for twoway requests which are not sent, finished can be called twice: the
            // first time because the outgoing is in the _sendStreams set and the second time
            // because it's either in the _requests/_asyncRequests set. This is fine, only the
            // first call should be taken into account by the implementation of finished.
            //
            for(let i = 0; i < this._sendStreams.length; ++i)
            {
                const p = this._sendStreams[i];
                if(p.requestId > 0)
                {
                    this._asyncRequests.delete(p.requestId);
                }
                p.completed(this._exception);
            }
            this._sendStreams = [];
        }

        for(const value of this._asyncRequests.values())
        {
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
    }

    toString()
    {
        return this._desc;
    }

    timedOut(event)
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
    }

    type()
    {
        return this._type;
    }

    timeout()
    {
        return this._endpoint.timeout();
    }

    getInfo()
    {
        if(this._state >= StateClosed)
        {
            throw this._exception;
        }
        const info = this._transceiver.getInfo();
        for(let p = info; p !== null; p = p.underlying)
        {
            p.adapterName = this._adapter !== null ? this._adapter.getName() : "";
            p.incoming = this._incoming;
        }
        return info;
    }

    setBufferSize(rcvSize, sndSize)
    {
        if(this._state >= StateClosed)
        {
            throw this._exception;
        }
        this._transceiver.setBufferSize(rcvSize, sndSize);
    }

    exception(ex)
    {
        this.setState(StateClosed, ex);
    }

    invokeException(ex, invokeNum)
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
    }

    setState(state, ex)
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
                         this._exception instanceof Ice.ConnectionManuallyClosedException ||
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

                default:
                {
                    Debug.assert(false);
                    break;
                }
            }
        }
        catch(ex)
        {
            if(ex instanceof Ice.LocalException)
            {
                this._instance.initializationData().logger.error(
                    `unexpected connection exception:\n${this._desc}\n${ex.toString()}`);
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
    }

    initiateShutdown()
    {
        Debug.assert(this._state === StateClosing && this._dispatchCount === 0);

        if(this._shutdownInitiated)
        {
            return;
        }
        this._shutdownInitiated = true;

        if(!this._endpoint.datagram())
        {
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

            if((this.sendMessage(OutgoingMessage.createForStream(os, false)) & AsyncStatus.Sent) > 0)
            {
                //
                // Schedule the close timeout to wait for the peer to close the connection.
                //
                this.scheduleTimeout(SocketOperation.Read);
            }
        }
    }

    sendHeartbeatNow()
    {
        Debug.assert(this._state === StateActive);

        if(!this._endpoint.datagram())
        {
            const os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
            os.writeBlob(Protocol.magic);
            Protocol.currentProtocol._write(os);
            Protocol.currentProtocolEncoding._write(os);
            os.writeByte(Protocol.validateConnectionMsg);
            os.writeByte(0);
            os.writeInt(Protocol.headerSize); // Message size.
            try
            {
                this.sendMessage(OutgoingMessage.createForStream(os, false));
            }
            catch(ex)
            {
                this.setState(StateClosed, ex);
                Debug.assert(this._exception !== null);
            }
        }
    }

    initialize()
    {
        const s = this._transceiver.initialize(this._readStream.buffer, this._writeStream.buffer);
        if(s != SocketOperation.None)
        {
            this.scheduleTimeout(s);
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

    validate()
    {
        if(!this._endpoint.datagram()) // Datagram connections are always implicitly validated.
        {
            if(this._adapter !== null) // The server side has the active role for connection validation.
            {
                if(this._writeStream.size === 0)
                {
                    this._writeStream.writeBlob(Protocol.magic);
                    Protocol.currentProtocol._write(this._writeStream);
                    Protocol.currentProtocolEncoding._write(this._writeStream);
                    this._writeStream.writeByte(Protocol.validateConnectionMsg);
                    this._writeStream.writeByte(0); // Compression status (always zero for validate connection).
                    this._writeStream.writeInt(Protocol.headerSize); // Message size.
                    TraceUtil.traceSend(this._writeStream, this._logger, this._traceLevels);
                    this._writeStream.prepareWrite();
                }

                if(this._writeStream.pos != this._writeStream.size && !this.write(this._writeStream.buffer))
                {
                    this.scheduleTimeout(SocketOperation.Write);
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
                    this.scheduleTimeout(SocketOperation.Read);
                    return false;
                }

                this._validated = true;

                Debug.assert(this._readStream.pos === Protocol.headerSize);
                this._readStream.pos = 0;
                const m = this._readStream.readBlob(4);
                if(m[0] !== Protocol.magic[0] || m[1] !== Protocol.magic[1] ||
                    m[2] !== Protocol.magic[2] || m[3] !== Protocol.magic[3])
                {
                    throw new Ice.BadMagicException("", m);
                }

                this._readProtocol._read(this._readStream);
                Protocol.checkSupportedProtocol(this._readProtocol);

                this._readProtocolEncoding._read(this._readStream);
                Protocol.checkSupportedProtocolEncoding(this._readProtocolEncoding);

                const messageType = this._readStream.readByte();
                if(messageType !== Protocol.validateConnectionMsg)
                {
                    throw new Ice.ConnectionNotValidatedException();
                }
                this._readStream.readByte(); // Ignore compression status for validate connection.
                if(this._readStream.readInt() !== Protocol.headerSize)
                {
                    throw new Ice.IllegalMessageSizeException();
                }
                TraceUtil.traceRecv(this._readStream, this._logger, this._traceLevels);
            }
        }

        this._writeStream.resize(0);
        this._writeStream.pos = 0;

        this._readStream.resize(Protocol.headerSize);
        this._readHeader = true;
        this._readStream.pos = 0;

        const traceLevels = this._instance.traceLevels();
        if(traceLevels.network >= 1)
        {
            const s = [];
            if(this._endpoint.datagram())
            {
                s.push("starting to send ");
                s.push(this._endpoint.protocol());
                s.push(" messages\n");
                s.push(this._transceiver.toDetailedString());
            }
            else
            {
                s.push("established ");
                s.push(this._endpoint.protocol());
                s.push(" connection\n");
                s.push(this.toString());
            }
            this._instance.initializationData().logger.trace(traceLevels.networkCat, s.join(""));
        }

        return true;
    }

    sendNextMessage()
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
                let message = this._sendStreams.shift();
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
                if(this._writeStream.pos != this._writeStream.size && !this.write(this._writeStream.buffer))
                {
                    Debug.assert(!this._writeStream.isEmpty());
                    this.scheduleTimeout(SocketOperation.Write);
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
        if(this._state === StateClosing && this._shutdownInitiated)
        {
            this.scheduleTimeout(SocketOperation.Read);
        }
    }

    sendMessage(message)
    {
        if(this._sendStreams.length > 0)
        {
            message.doAdopt();
            this._sendStreams.push(message);
            return AsyncStatus.Queued;
        }
        Debug.assert(this._state < StateClosed);

        Debug.assert(!message.prepared);

        const stream = message.stream;
        stream.pos = 10;
        stream.writeInt(stream.size);
        stream.prepareWrite();
        message.prepared = true;

        TraceUtil.traceSend(stream, this._logger, this._traceLevels);

        if(this.write(stream.buffer))
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
        this.scheduleTimeout(SocketOperation.Write);

        return AsyncStatus.Queued;
    }

    parseMessage()
    {
        Debug.assert(this._state > StateNotValidated && this._state < StateClosed);

        let info = new MessageInfo(this._instance);

        this._readStream.swap(info.stream);
        this._readStream.resize(Protocol.headerSize);
        this._readStream.pos = 0;
        this._readHeader = true;

        Debug.assert(info.stream.pos === info.stream.size);

        try
        {
            //
            // We don't need to check magic and version here. This has already
            // been done by the caller.
            //
            info.stream.pos = 8;
            const messageType = info.stream.readByte();
            const compress = info.stream.readByte();
            if(compress === 2)
            {
                throw new Ice.FeatureNotSupportedException("Cannot uncompress compressed message");
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
    }

    invokeAll(stream, invokeNum, requestId, servantManager, adapter)
    {
        try
        {
            while(invokeNum > 0)
            {
                //
                // Prepare the invocation.
                //
                const inc = new IncomingAsync(this._instance, this,
                                              adapter,
                                              !this._endpoint.datagram() && requestId !== 0, // response
                                              requestId);

                //
                // Dispatch the invocation.
                //
                inc.invoke(servantManager, stream);

                --invokeNum;
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
                //
                // An Error was raised outside of servant code (i.e., by Ice code).
                // Attempt to log the error and clean up.
                //
                this._logger.error("unexpected exception:\n" + ex.toString());
                this.invokeException(new Ice.UnknownException(ex), invokeNum);
            }
        }
    }

    scheduleTimeout(op)
    {
        let timeout;
        if(this._state < StateActive)
        {
            const defaultsAndOverrides = this._instance.defaultsAndOverrides();
            if(defaultsAndOverrides.overrideConnectTimeout)
            {
                timeout = defaultsAndOverrides.overrideConnectTimeoutValue;
            }
            else
            {
                timeout = this._endpoint.timeout();
            }
        }
        else if(this._state < StateClosing)
        {
            if(this._readHeader) // No timeout for reading the header.
            {
                op &= ~SocketOperation.Read;
            }
            timeout = this._endpoint.timeout();
        }
        else
        {
            const defaultsAndOverrides = this._instance.defaultsAndOverrides();
            if(defaultsAndOverrides.overrideCloseTimeout)
            {
                timeout = defaultsAndOverrides.overrideCloseTimeoutValue;
            }
            else
            {
                timeout = this._endpoint.timeout();
            }
        }

        if(timeout < 0)
        {
            return;
        }

        if((op & SocketOperation.Read) !== 0)
        {
            if(this._readTimeoutScheduled)
            {
                this._timer.cancel(this._readTimeoutId);
            }
            this._readTimeoutId = this._timer.schedule(() => this.timedOut(), timeout);
            this._readTimeoutScheduled = true;
        }
        if((op & (SocketOperation.Write | SocketOperation.Connect)) !== 0)
        {
            if(this._writeTimeoutScheduled)
            {
                this._timer.cancel(this._writeTimeoutId);
            }
            this._writeTimeoutId = this._timer.schedule(() => this.timedOut(), timeout);
            this._writeTimeoutScheduled = true;
        }
    }

    unscheduleTimeout(op)
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
    }

    warning(msg, ex)
    {
        this._logger.warning(msg + ":\n" + this._desc + "\n" + ex.toString());
    }

    checkState()
    {
        if(this._state < StateHolding || this._dispatchCount > 0)
        {
            return;
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
            this._finishedPromises.forEach(p => p.resolve());
            this._finishedPromises = [];
        }
    }

    reap()
    {
        if(this._monitor !== null)
        {
            this._monitor.reap(this);
        }
    }

    read(buf)
    {
        const start = buf.position;
        const ret = this._transceiver.read(buf, this._hasMoreData);
        if(this._instance.traceLevels().network >= 3 && buf.position != start)
        {
            const s = [];
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
    }

    write(buf)
    {
        const start = buf.position;
        const ret = this._transceiver.write(buf);
        if(this._instance.traceLevels().network >= 3 && buf.position != start)
        {
            const s = [];
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
}

// DestructionReason.
ConnectionI.ObjectAdapterDeactivated = 0;
ConnectionI.CommunicatorDestroyed = 1;

Ice.ConnectionI = ConnectionI;

class OutgoingMessage
{
    constructor()
    {
        this.stream = null;
        this.outAsync = null;
        this.requestId = 0;
        this.prepared = false;
    }

    canceled()
    {
        Debug.assert(this.outAsync !== null);
        this.outAsync = null;
    }

    doAdopt()
    {
        if(this.adopt)
        {
            const stream = new OutputStream(this.stream.instance, Protocol.currentProtocolEncoding);
            stream.swap(this.stream);
            this.stream = stream;
            this.adopt = false;
        }
    }

    sent()
    {
        if(this.outAsync !== null)
        {
            this.outAsync.sent();
        }
    }

    completed(ex)
    {
        if(this.outAsync !== null)
        {
            this.outAsync.completedEx(ex);
        }
    }

    static createForStream(stream, adopt)
    {
        const m = new OutgoingMessage();
        m.stream = stream;
        m.adopt = adopt;
        m.isSent = false;
        m.requestId = 0;
        m.outAsync = null;
        return m;
    }

    static create(out, stream, requestId)
    {
        const m = new OutgoingMessage();
        m.stream = stream;
        m.outAsync = out;
        m.requestId = requestId;
        m.isSent = false;
        m.adopt = false;
        return m;
    }
}

module.exports.Ice = Ice;
