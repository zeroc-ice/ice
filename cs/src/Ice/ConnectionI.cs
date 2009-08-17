// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Threading;

    public sealed class ConnectionI : Connection, IceInternal.TimerTask
    {
        public interface StartCallback
        {
            void connectionStartCompleted(ConnectionI connection);
            void connectionStartFailed(ConnectionI connection, LocalException ex);
        }

        public void start(StartCallback callback)
        {
            try
            {
                lock(this)
                {
                    //
                    // The connection might already be closed if the communicator was destroyed.
                    //
                    if(_state == StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }

                    //
                    // Use asynchronous I/O. We cannot begin an asynchronous I/O request from
                    // this thread if a callback is provided, so we queue it on the async IO 
                    // thread.
                    //
                    if(callback == null)
                    {
                        initializeAsync(null);
                    }
                    else
                    {
                        _instance.asyncIOThread().queue(initializeAsync);
                        _startCallback = callback;
                        return;
                    }

                    while(_state <= StateNotValidated)
                    {
                        Monitor.Wait(this);
                    }
                    
                    if(_state >= StateClosing)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }
                }
            }
            catch(LocalException ex)
            {
                exception(ex);
                if(callback != null)
                {
                    callback.connectionStartFailed(this, _exception);
                    return;
                }
                else
                {
                    waitUntilFinished();
                    throw ex;
                }
            }

            if(callback != null)
            {
                callback.connectionStartCompleted(this);
            }
        }

        public void activate()
        {
            lock(this)
            {
                if(_state <= StateNotValidated)
                {
                    return;
                }

                if(_acmTimeout > 0)
                {
                    _acmAbsoluteTimeoutMillis = IceInternal.Time.currentMonotonicTimeMillis() + _acmTimeout * 1000;
                }

                setState(StateActive);
            }
        }

        public void hold()
        {
            lock(this)
            {
                if(_state <= StateNotValidated)
                {
                    return;
                }

                setState(StateHolding);
            }
        }

        // DestructionReason.
        public const int ObjectAdapterDeactivated = 0;
        public const int CommunicatorDestroyed = 1;

        public void destroy(int reason)
        {
            lock(this)
            {
                switch(reason)
                {
                    case ObjectAdapterDeactivated:
                    {
                        setState(StateClosing, new ObjectAdapterDeactivatedException());
                        break;
                    }

                    case CommunicatorDestroyed:
                    {
                        setState(StateClosing, new CommunicatorDestroyedException());
                        break;
                    }
                }
            }
        }

        public void close(bool force)
        {
            lock(this)
            {
                if(force)
                {
                    setState(StateClosed, new ForcedCloseConnectionException());
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
                    while(_requests.Count != 0 || _asyncRequests.Count != 0)
                    {
                        Monitor.Wait(this);
                    }

                    setState(StateClosing, new CloseConnectionException());
                }
            }
        }

        public bool isActiveOrHolding()
        {
            lock(this)
            {
                return _state > StateNotValidated && _state < StateClosing;
            }
        }

        public bool isFinished()
        {
            //
            // We can use TryEnter here, because as long as there are still
            // threads operating in this connection object, connection
            // destruction is considered as not yet finished.
            //
            if(!Monitor.TryEnter(this))
            {
                return false;
            }

            try
            {
                if(_transceiver != null || _dispatchCount != 0)
                {
                    return false;
                }

                Debug.Assert(_state == StateClosed);
            }
            finally
            {
                Monitor.Exit(this);
            }
            return true;
        }

        public void throwException()
        {
            lock(this)
            {
                if(_exception != null)
                {
                    Debug.Assert(_state >= StateClosing);
                    throw _exception;
                }
            }
        }

        public void waitUntilHolding()
        {
            lock(this)
            {
                while(_state < StateHolding || _dispatchCount > 0)
                {
                    Monitor.Wait(this);
                }
            }
        }

        public void waitUntilFinished()
        {
            lock(this)
            {
                //
                // We wait indefinitely until connection closing has been
                // initiated. We also wait indefinitely until all outstanding
                // requests are completed. Otherwise we couldn't guarantee
                // that there are no outstanding calls when deactivate() is
                // called on the servant locators.
                //
                while(_state < StateClosing || _dispatchCount > 0)
                {
                    Monitor.Wait(this);
                }

                //
                // Now we must wait until close() has been called on the
                // transceiver.
                //
                while(_transceiver != null)
                {
                    if(_state != StateClosed && _endpoint.timeout() >= 0)
                    {
                        long absoluteWaitTime = _stateTime + _endpoint.timeout();
                        int waitTime = (int)(absoluteWaitTime - IceInternal.Time.currentMonotonicTimeMillis());

                        if(waitTime > 0)
                        {
                            //
                            // We must wait a bit longer until we close this
                            // connection.
                            //
                            Monitor.Wait(this, waitTime);
                            if(IceInternal.Time.currentMonotonicTimeMillis() >= absoluteWaitTime)
                            {
                                setState(StateClosed, new CloseTimeoutException());
                            }
                        }
                        else
                        {
                            //
                            // We already waited long enough, so let's close this
                            // connection!
                            //
                            setState(StateClosed, new CloseTimeoutException());
                        }

                        //
                        // No return here, we must still wait until close() is
                        // called on the _transceiver.
                        //
                    }
                    else
                    {
                        Monitor.Wait(this);
                    }
                }

                Debug.Assert(_state == StateClosed);

                //
                // Clear the OA. See bug 1673 for the details of why this is necessary.
                //
                _adapter = null;
            }
        }

        public void monitor(long now)
        {
            if(!Monitor.TryEnter(this))
            {
                return;
            }

            try
            {
                if(_state != StateActive)
                {
                    return;
                }

                //
                // Active connection management for idle connections.
                //
                //
                if(_acmTimeout <= 0 ||
                    _requests.Count > 0 || _asyncRequests.Count > 0 ||
                    _batchStreamInUse || !_batchStream.isEmpty() ||
                    _sendInProgress || _dispatchCount > 0)
                {
                    return;
                }

                if(now >= _acmAbsoluteTimeoutMillis)
                {
                    setState(StateClosing, new ConnectionTimeoutException());
                }
            }
            finally
            {
                Monitor.Exit(this);
            }
        }

        public bool sendRequest(IceInternal.Outgoing og, bool compress, bool response)
        {
            IceInternal.BasicStream os = og.ostr();

            lock(this)
            {
                if(_exception != null)
                {
                    //
                    // If the connection is closed before we even have a chance
                    // to send our request, we always try to send the request
                    // again.
                    //
                    throw new IceInternal.LocalExceptionWrapper(_exception, true);
                }

                Debug.Assert(_state > StateNotValidated);
                Debug.Assert(_state < StateClosing);

                int requestId = 0;
                if(response)
                {
                    //
                    // Create a new unique request ID.
                    //
                    requestId = _nextRequestId++;
                    if(requestId <= 0)
                    {
                        _nextRequestId = 1;
                        requestId = _nextRequestId++;
                    }

                    //
                    // Fill in the request ID.
                    //
                    os.pos(IceInternal.Protocol.headerSize);
                    os.writeInt(requestId);
                }

                //
                // Send the message. If it can't be sent without blocking the message is added
                // to _sendStreams and it will be sent by the asynchronous I/O callback.
                //
                bool sent = false;
                try
                {
                    sent = sendMessage(new OutgoingMessage(og, og.ostr(), compress, response));
                }
                catch(LocalException ex)
                {
                    setState(StateClosed, ex);
                    Debug.Assert(_exception != null);
                    throw _exception;
                }

                if(response)
                {
                    //
                    // Add to the requests map.
                    //
                    _requests[requestId] = og;
                }

                return sent;
            }
        }

        public bool sendAsyncRequest(IceInternal.OutgoingAsync og, bool compress, bool response)
        {
            IceInternal.BasicStream os = og.ostr__();

            lock(this)
            {
                if(_exception != null)
                {
                    //
                    // If the connection is closed before we even have a chance
                    // to send our request, we always try to send the request
                    // again.
                    //
                    throw new IceInternal.LocalExceptionWrapper(_exception, true);
                }

                Debug.Assert(_state > StateNotValidated);
                Debug.Assert(_state < StateClosing);

                int requestId = 0;
                if(response)
                {
                    //
                    // Create a new unique request ID.
                    //
                    requestId = _nextRequestId++;
                    if(requestId <= 0)
                    {
                        _nextRequestId = 1;
                        requestId = _nextRequestId++;
                    }

                    //
                    // Fill in the request ID.
                    //
                    os.pos(IceInternal.Protocol.headerSize);
                    os.writeInt(requestId);
                }
                
                bool sent;
                try
                {
                    sent = sendMessage(new OutgoingMessage(og, og.ostr__(), compress, response));
                }
                catch(LocalException ex)
                {
                    setState(StateClosed, ex);
                    Debug.Assert(_exception != null);
                    throw _exception;
                }

                if(response)
                {
                    //
                    // Add to the async requests map.
                    //
                    _asyncRequests[requestId] = og;
                }
                return sent;
            }
        }

        public void prepareBatchRequest(IceInternal.BasicStream os)
        {
            lock(this)
            {
                //
                // Wait if flushing is currently in progress.
                //
                while(_batchStreamInUse && _exception == null)
                {
                    Monitor.Wait(this);
                }

                if(_exception != null)
                {
                    //
                    // If there were no batch requests queued when the connection failed, we can safely 
                    // retry with a new connection. Otherwise, we must throw to notify the caller that 
                    // some previous batch requests were not sent.
                    //
                    if(_batchStream.isEmpty())
                    {
                        throw new IceInternal.LocalExceptionWrapper(_exception, true);
                    }
                    else
                    {
                        throw _exception;
                    }
                }

                Debug.Assert(_state > StateNotValidated);
                Debug.Assert(_state < StateClosing);

                if(_batchStream.isEmpty())
                {
                    try
                    {
                        _batchStream.writeBlob(IceInternal.Protocol.requestBatchHdr);
                    }
                    catch(LocalException ex)
                    {
                        setState(StateClosed, ex);
                        throw;
                    }
                }

                _batchStreamInUse = true;
                _batchMarker = _batchStream.size();
                _batchStream.swap(os);

                //
                // The batch stream now belongs to the caller, until
                // finishBatchRequest() or abortBatchRequest() is called.
                //
            }
        }

        public void finishBatchRequest(IceInternal.BasicStream os, bool compress)
        {
            try
            {
                lock(this)
                {
                    //
                    // Get the batch stream back.
                    //
                    _batchStream.swap(os);

                    if(_exception != null)
                    {
                        throw _exception;
                    }

                    bool flush = false;
                    if(_batchAutoFlush)
                    {
                        //
                        // Throw memory limit exception if the first message added causes us to
                        // go over limit. Otherwise put aside the marshalled message that caused
                        // limit to be exceeded and rollback stream to the marker.
                        //
                        try
                        {
                            _transceiver.checkSendSize(_batchStream.getBuffer(), _instance.messageSizeMax());
                        }
                        catch(LocalException)
                        {
                            if(_batchRequestNum > 0)
                            {
                                flush = true;
                            }
                            else
                            {
                                throw;
                            }
                        }
                    }

                    if(flush)
                    {
                        //
                        // Temporarily save the last request.
                        //
                        int requestSize = _batchStream.size() - _batchMarker;
                        byte[] lastRequest = new byte[requestSize];
                        Buffer.BlockCopy(_batchStream.getBuffer().b.rawBytes(), _batchMarker, lastRequest, 0,
                                         requestSize);
                        _batchStream.resize(_batchMarker, false);

                        //
                        // Send the batch stream without the last request.
                        //
                        try
                        {
                            //
                            // Fill in the number of requests in the batch.
                            //
                            _batchStream.pos(IceInternal.Protocol.headerSize);
                            _batchStream.writeInt(_batchRequestNum);

                            OutgoingMessage message = new OutgoingMessage(_batchStream, _batchRequestCompress, true);
                            sendMessage(message);
                        }
                        catch(LocalException ex)
                        {
                            setState(StateClosed, ex);
                            Debug.Assert(_exception != null);
                            throw _exception;
                        }

                        //
                        // Reset the batch stream.
                        //
                        _batchStream = new IceInternal.BasicStream(_instance, _batchAutoFlush);
                        _batchRequestNum = 0;
                        _batchRequestCompress = false;
                        _batchMarker = 0;

                        //
                        // Check again if the last request doesn't exceed the maximum message size.
                        //
                        if(IceInternal.Protocol.requestBatchHdr.Length + lastRequest.Length >
                           _instance.messageSizeMax())
                        {
                            IceInternal.Ex.throwMemoryLimitException(
                                IceInternal.Protocol.requestBatchHdr.Length + lastRequest.Length,
                                _instance.messageSizeMax());
                        }

                        //
                        // Start a new batch with the last message that caused us to go over the limit.
                        //
                        _batchStream.writeBlob(IceInternal.Protocol.requestBatchHdr);
                        _batchStream.writeBlob(lastRequest);
                    }

                    //
                    // Increment the number of requests in the batch.
                    //
                    ++_batchRequestNum;

                    //
                    // We compress the whole batch if there is at least one compressed
                    // message.
                    //
                    if(compress)
                    {
                        _batchRequestCompress = true;
                    }

                    //
                    // Notify about the batch stream not being in use anymore.
                    //
                    Debug.Assert(_batchStreamInUse);
                    _batchStreamInUse = false;
                    Monitor.PulseAll(this);
                }
            }
            catch(LocalException)
            {
                abortBatchRequest();
                throw;
            }
        }

        public void abortBatchRequest()
        {
            lock(this)
            {
                _batchStream = new IceInternal.BasicStream(_instance, _batchAutoFlush);
                _batchRequestNum = 0;
                _batchRequestCompress = false;
                _batchMarker = 0;

                Debug.Assert(_batchStreamInUse);
                _batchStreamInUse = false;
                Monitor.PulseAll(this);
            }
        }

        public void flushBatchRequests()
        {
            IceInternal.BatchOutgoing @out = new IceInternal.BatchOutgoing(this, _instance);
            @out.invoke();
        }

        public bool flushBatchRequests(IceInternal.BatchOutgoing @out)
        {
            lock(this)
            {
                while(_batchStreamInUse && _exception == null)
                {
                    Monitor.Wait(this);
                }

                if(_exception != null)
                {
                    throw _exception;
                }

                if(_batchRequestNum == 0)
                {
                    @out.sent(false);
                    return true;
                }

                //
                // Fill in the number of requests in the batch.
                //
                _batchStream.pos(IceInternal.Protocol.headerSize);
                _batchStream.writeInt(_batchRequestNum);

                _batchStream.swap(@out.ostr());

                bool sent = false;
                try
                {
                    OutgoingMessage message = new OutgoingMessage(@out, @out.ostr(), _batchRequestCompress, false);
                    sent = sendMessage(message);
                }
                catch(LocalException ex)
                {
                    setState(StateClosed, ex);
                    Debug.Assert(_exception != null);
                    throw _exception;
                }

                //
                // Reset the batch stream.
                //
                _batchStream = new IceInternal.BasicStream(_instance, _batchAutoFlush);
                _batchRequestNum = 0;
                _batchRequestCompress = false;
                _batchMarker = 0;

                return sent;
            }
        }

        public bool flushAsyncBatchRequests(IceInternal.BatchOutgoingAsync outAsync)
        {
            lock(this)
            {
                while(_batchStreamInUse && _exception == null)
                {
                    Monitor.Wait(this);
                }

                if(_exception != null)
                {
                    throw _exception;
                }

                if(_batchRequestNum == 0)
                {
                    outAsync.sent__(this);
                    return true;
                }

                //
                // Fill in the number of requests in the batch.
                //
                _batchStream.pos(IceInternal.Protocol.headerSize);
                _batchStream.writeInt(_batchRequestNum);

                _batchStream.swap(outAsync.ostr__());

                //
                // Send the batch stream.
                //
                bool sent;
                try
                {
                    OutgoingMessage message = new OutgoingMessage(outAsync, outAsync.ostr__(), _batchRequestCompress,
                                                                  false);
                    sent = sendMessage(message);
                }
                catch(LocalException ex)
                {
                    setState(StateClosed, ex);
                    Debug.Assert(_exception != null);
                    throw _exception;
                }

                //
                // Reset the batch stream.
                //
                _batchStream = new IceInternal.BasicStream(_instance, _batchAutoFlush);
                _batchRequestNum = 0;
                _batchRequestCompress = false;
                _batchMarker = 0;
                return sent;
            }
        }

        public void sendResponse(IceInternal.BasicStream os, byte compressFlag)
        {
            lock(this)
            {
                Debug.Assert(_state > StateNotValidated);

                try
                {
                    if(--_dispatchCount == 0)
                    {
                        Monitor.PulseAll(this);
                    }

                    if(_state == StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }

                    sendMessage(new OutgoingMessage(os, compressFlag != 0, true));

                    if(_state == StateClosing && _dispatchCount == 0)
                    {
                        initiateShutdown();
                    }

                    if(_acmTimeout > 0)
                    {
                        _acmAbsoluteTimeoutMillis = IceInternal.Time.currentMonotonicTimeMillis() + _acmTimeout * 1000;
                    }
                }
                catch(LocalException ex)
                {
                    setState(StateClosed, ex);
                }
            }
        }

        public void sendNoResponse()
        {
            lock(this)
            {
                Debug.Assert(_state > StateNotValidated);

                try
                {
                    if(--_dispatchCount == 0)
                    {
                        Monitor.PulseAll(this);
                    }

                    if(_state == StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }

                    if(_state == StateClosing && _dispatchCount == 0)
                    {
                        initiateShutdown();
                    }

                    if(_acmTimeout > 0)
                    {
                        _acmAbsoluteTimeoutMillis = IceInternal.Time.currentMonotonicTimeMillis() + _acmTimeout * 1000;
                    }
                }
                catch(LocalException ex)
                {
                    setState(StateClosed, ex);
                }
            }
        }

        public IceInternal.EndpointI endpoint()
        {
            return _endpoint; // No mutex protection necessary, _endpoint is immutable.
        }

        public void setAdapter(ObjectAdapter adapter)
        {
            lock(this)
            {
                if(_state <= StateNotValidated || _state >= StateClosing)
                {
                    return;
                }
                Debug.Assert(_state < StateClosing);

                _adapter = adapter;

                if(_adapter != null)
                {
                    _servantManager = ((ObjectAdapterI) _adapter).getServantManager();
                    if(_servantManager == null)
                    {
                        _adapter = null;
                    }
                }
                else
                {
                    _servantManager = null;
                }

                //
                // We never change the thread pool with which we were initially
                // registered, even if we add or remove an object adapter.
                //
            }
        }

        public ObjectAdapter getAdapter()
        {
            lock(this)
            {
                return _adapter;
            }
        }

        public ObjectPrx createProxy(Identity ident)
        {
            //
            // Create a reference and return a reverse proxy for this
            // reference.
            //
            return _instance.proxyFactory().referenceToProxy(_instance.referenceFactory().create(ident, this));
        }

        public void exception(LocalException ex)
        {
            lock(this)
            {
                setState(StateClosed, ex);
            }
        }

        public void invokeException(LocalException ex, int invokeNum)
        {
            //
            // Fatal exception while invoking a request. Since sendResponse/sendNoResponse isn't
            // called in case of a fatal exception we decrement _dispatchCount here.
            //

            lock(this)
            {
                setState(StateClosed, ex);

                if(invokeNum > 0)
                {
                    Debug.Assert(_dispatchCount > 0);
                    _dispatchCount -= invokeNum;
                    Debug.Assert(_dispatchCount >= 0);
                    if(_dispatchCount == 0)
                    {
                        Monitor.PulseAll(this);
                    }
                }
            }
        }

        public string type()
        {
            return _type; // No mutex lock, _type is immutable.
        }

        public int timeout()
        {
            return _endpoint.timeout(); // No mutex protection necessary, _endpoint is immutable.
        }

        public string ice_toString_()
        {
            return ToString();
        }

        public override string ToString()
        {
            return _desc; // No mutex lock, _desc is immutable.
        }

        //
        // Only used by the SSL plug-in.
        //
        // The external party has to synchronize the connection, since the
        // connection is the object that protects the transceiver.
        //
        public IceInternal.Transceiver getTransceiver()
        {
            return _transceiver;
        }

        static ConnectionI()
        {
            _compressionSupported = IceInternal.BasicStream.compressible();
        }

        internal ConnectionI(IceInternal.Instance instance, IceInternal.Transceiver transceiver,
                             IceInternal.EndpointI endpoint, ObjectAdapter adapter)
        {
            _instance = instance;
            InitializationData initData = instance.initializationData();
            _transceiver = transceiver;
            _restartable = transceiver.restartable();
            _desc = transceiver.ToString();
            _type = transceiver.type();
            _endpoint = endpoint;
            _adapter = adapter;
            _logger = initData.logger; // Cached for better performance.
            _traceLevels = instance.traceLevels(); // Cached for better performance.
            _stream = new IceInternal.BasicStream(instance);
            _initializeAsyncCallback = new AsyncCallback(initializeAsync);
            _validateAsyncCallback = new AsyncCallback(validateAsync);
            _writeAsyncCallback = new AsyncCallback(writeAsync);
            _readAsyncCallback = new AsyncCallback(readAsync);
            _flushSentCallbacks = delegate(bool unused) { flushSentCallbacks(); };
            _reading = false;
            _warn = initData.properties.getPropertyAsInt("Ice.Warn.Connections") > 0;
            _warnUdp = initData.properties.getPropertyAsInt("Ice.Warn.Datagrams") > 0;
            _cacheBuffers = initData.properties.getPropertyAsIntWithDefault("Ice.CacheMessageBuffers", 1) == 1;
            _acmAbsoluteTimeoutMillis = 0;
            _nextRequestId = 1;
            _batchAutoFlush = initData.properties.getPropertyAsIntWithDefault("Ice.BatchAutoFlush", 1) > 0;
            _batchStream = new IceInternal.BasicStream(instance, _batchAutoFlush);
            _batchStreamInUse = false;
            _batchRequestNum = 0;
            _batchRequestCompress = false;
            _batchMarker = 0;
            _sendInProgress = false;
            _dispatchCount = 0;
            _state = StateNotInitialized;
            _stateTime = IceInternal.Time.currentMonotonicTimeMillis();

            if(_endpoint.datagram())
            {
                _acmTimeout = 0;
            }
            else
            {
                if(_adapter != null)
                {
                    _acmTimeout = _instance.serverACM();
                }
                else
                {
                    _acmTimeout = _instance.clientACM();
                }
            }

            _compressionLevel = initData.properties.getPropertyAsIntWithDefault("Ice.Compression.Level", 1);
            if(_compressionLevel < 1)
            {
                _compressionLevel = 1;
            }
            else if(_compressionLevel > 9)
            {
                _compressionLevel = 9;
            }

            ObjectAdapterI adapterImpl = _adapter as ObjectAdapterI;
            if(adapterImpl != null)
            {
                _servantManager = adapterImpl.getServantManager();
            }

            if(adapterImpl != null)
            {
                _threadPool = adapterImpl.getThreadPool();
            }
            else
            {
                _threadPool = instance.clientThreadPool();
            }

            try
            {
                _timer = _instance.timer();

                _overrideCompress = _instance.defaultsAndOverrides().overrideCompress;
                _overrideCompressValue = _instance.defaultsAndOverrides().overrideCompressValue;
            }
            catch(LocalException)
            {
                throw;
            }
            catch(System.Exception ex)
            {
                throw new SyscallException(ex);
            }
        }

        private const int StateNotInitialized = 0;
        private const int StateNotValidated = 1;
        private const int StateActive = 2;
        private const int StateHolding = 3;
        private const int StateClosing = 4;
        private const int StateClosed = 5;

        private void setState(int state, LocalException ex)
        {
            //
            // If setState() is called with an exception, then only closed
            // and closing states are permissible.
            //
            Debug.Assert(state == StateClosing || state == StateClosed);

            if(_state == state) // Don't switch twice.
            {
                return;
            }

            if(_exception == null)
            {
                _exception = ex;

                if(_warn)
                {
                    //
                    // We don't warn if we are not validated.
                    //
                    if(_state > StateNotValidated)
                    {
                        //
                        // Don't warn about certain expected exceptions.
                        //
                        if(!(_exception is CloseConnectionException ||
                             _exception is ForcedCloseConnectionException ||
                             _exception is ConnectionTimeoutException ||
                             _exception is CommunicatorDestroyedException ||
                             _exception is ObjectAdapterDeactivatedException ||
                             (_exception is ConnectionLostException && _state == StateClosing)))
                        {
                            warning("connection exception", _exception);
                        }
                    }
                }
            }

            //
            // We must set the new state before we notify requests of any
            // exceptions. Otherwise new requests may retry on a
            // connection that is not yet marked as closed or closing.
            //
            setState(state);
        }

        private void setState(int state)
        {
            //
            // We don't want to send close connection messages if the endpoint
            // only supports oneway transmission from client to server.
            //
            if(_endpoint.datagram() && state == StateClosing)
            {
                state = StateClosed;
            }

            //
            // Skip graceful shutdown if we are destroyed before validation.
            //
            if(_state <= StateNotValidated && state == StateClosing)
            {
                state = StateClosed;
            }

            if(_state == state) // Don't switch twice.
            {
                return;
            }

            switch(state)
            {
                case StateNotInitialized:
                {
                    Debug.Assert(false);
                    break;
                }

                case StateNotValidated:
                {
                    if(_state != StateNotInitialized)
                    {
                        Debug.Assert(_state == StateClosed);
                        return;
                    }
                    break;
                }

                case StateActive:
                {
                    //
                    // Can only switch from holding or not validated to
                    // active.
                    //
                    if(_state != StateHolding && _state != StateNotValidated)
                    {
                        return;
                    }
                    if(!_reading)
                    {
                        //
                        // Start reading.
                        //
                        _reading = true;
                        startReadAsync(false);
                    }
                    break;
                }

                case StateHolding:
                {
                    //
                    // Can only switch from active or not validated to
                    // holding.
                    //
                    if(_state != StateActive && _state != StateNotValidated)
                    {
                        return;
                    }
                    break;
                }

                case StateClosing:
                {
                    //
                    // Can't change back from closed.
                    //
                    if(_state == StateClosed)
                    {
                        return;
                    }
                    if(!_reading)
                    {
                        //
                        // We need to continue to read in closing state.
                        //
                        _reading = true;
                        startReadAsync(false);
                    }
                    break;
                }

                case StateClosed:
                {
                    if(!_sendInProgress)
                    {
                        _threadPool.finish(this);
                    }

                    //
                    // Close the transceiver now to interrupt pending sends or reads. If a send was
                    // in progress, the writeAsync callback method will be called it will take care
                    // of calling _threadPool.finish(this).
                    //
                    _transceiver.close();
                    break;
                }
            }

            //
            // We only register with the connection monitor if our new state
            // is StateActive. Otherwise we unregister with the connection
            // monitor, but only if we were registered before, i.e., if our
            // old state was StateActive.
            //
            IceInternal.ConnectionMonitor connectionMonitor = _instance.connectionMonitor();
            if(connectionMonitor != null)
            {
                if(state == StateActive)
                {
                    connectionMonitor.add(this);
                }
                else if(_state == StateActive)
                {
                    connectionMonitor.remove(this);
                }
            }

            _state = state;
            _stateTime = IceInternal.Time.currentMonotonicTimeMillis();

            Monitor.PulseAll(this);

            if(_state == StateClosing && _dispatchCount == 0)
            {
                try
                {
                    initiateShutdown();
                }
                catch(LocalException ex)
                {
                    setState(StateClosed, ex);
                }
            }
        }

        private void initiateShutdown()
        {
            Debug.Assert(_state == StateClosing);
            Debug.Assert(_dispatchCount == 0);

            if(!_endpoint.datagram())
            {
                //
                // Before we shut down, we send a close connection
                // message.
                //
                IceInternal.BasicStream os = new IceInternal.BasicStream(_instance);
                os.writeBlob(IceInternal.Protocol.magic);
                os.writeByte(IceInternal.Protocol.protocolMajor);
                os.writeByte(IceInternal.Protocol.protocolMinor);
                os.writeByte(IceInternal.Protocol.encodingMajor);
                os.writeByte(IceInternal.Protocol.encodingMinor);
                os.writeByte(IceInternal.Protocol.closeConnectionMsg);
                os.writeByte(_compressionSupported ? (byte)1 : (byte)0);
                os.writeInt(IceInternal.Protocol.headerSize); // Message size.

                sendMessage(new OutgoingMessage(os, false, false));

                //
                // The CloseConnection message should be sufficient. Closing the write
                // end of the socket is probably an artifact of how things were done
                // in IIOP. In fact, shutting down the write end of the socket causes
                // problems on Windows by preventing the peer from using the socket.
                // For example, the peer is no longer able to continue writing a large
                // message after the socket is shutdown.
                //
                //_transceiver.shutdownWrite();
            }
        }

        private void initializeAsync(object state)
        {
            //
            // This method is called from the async IO thread, and as
            // an asynchronous I/O callback.
            //
            IAsyncResult result = (IAsyncResult)state;

            //
            // Don't invoke Transceiver.initialize recursively. If an I/O request completed
            // synchronously, return immediately and let the code that began the request
            // handle its completion.
            //
            if(result != null && result.CompletedSynchronously)
            {
                return;
            }

            lock(this)
            {
                if(_state == StateClosed)
                {
                    return;
                }

                //
                // An I/O request has completed, so cancel a pending timeout.
                //
                if(result != null)
                {
                    _timer.cancel(this);
                }

                try
                {
                    //
                    // Initialize the transceiver.
                    //
                    // If initialize returns false, the transceiver has begun an asynchronous
                    // I/O request and we must await the callback. Otherwise we can progress to
                    // validation.
                    //
                    if(_transceiver.initialize(_initializeAsyncCallback))
                    {
                        //
                        // Update the connection description once the transceiver is initialized.
                        //
                        _desc = _transceiver.ToString();
                        setState(StateNotValidated);
                    }
                    else
                    {
                        scheduleConnectTimeout();
                        return;
                    }
                }
                catch(LocalException ex)
                {
                    setState(StateClosed, ex);
                    return;
                }
            }

            //
            // We are already executing in a thread that is safe for beginning asynchronous
            // I/O requests, so we start the validation process immediately.
            //
            validateAsync(null);
        }

        private void validateAsync(object state)
        {
            //
            // This method is called from the async IO thread, and as
            // an asynchronous I/O callback.
            //
            IAsyncResult result = (IAsyncResult)state;
            
            //
            // If the I/O request completed synchronously, then we are in a recursive call
            // to this method. We return immediately and let the code that began the I/O
            // request handle its completion (see below).
            //
            if(result != null && result.CompletedSynchronously)
            {
                return;
            }

            StartCallback callback = null;
            lock(this)
            {
                if(_state == StateClosed)
                {
                    return;
                }

                if(!_endpoint.datagram()) // Datagram connections are always implicitly validated.
                {
                    //
                    // An I/O request may have completed.
                    //
                    if(result != null)
                    {
                        //
                        // Cancel a pending timeout (if any).
                        //
                        _timer.cancel(this);
                    }

                    try
                    {
                        if(_adapter != null) // The server side has the active role for connection validation.
                        {
                            IceInternal.BasicStream os = _stream;
                            if(os.size() == 0)
                            {
                                //
                                // We haven't tried to write yet; prepare the validate connection message.
                                //
                                os.writeBlob(IceInternal.Protocol.magic);
                                os.writeByte(IceInternal.Protocol.protocolMajor);
                                os.writeByte(IceInternal.Protocol.protocolMinor);
                                os.writeByte(IceInternal.Protocol.encodingMajor);
                                os.writeByte(IceInternal.Protocol.encodingMinor);
                                os.writeByte(IceInternal.Protocol.validateConnectionMsg);
                                os.writeByte((byte)0); // Compression status (always zero for validate connection).
                                os.writeInt(IceInternal.Protocol.headerSize); // Message size.
                                IceInternal.TraceUtil.traceSend(os, _logger, _traceLevels);
                                os.prepareWrite();
                            }

                            //
                            // Check for the completion of an earlier asynchronous write.
                            //
                            if(result != null)
                            {
                                Debug.Assert(result != null);
                                _transceiver.endWrite(os.getBuffer(), result);
                                Debug.Assert(os.pos() == os.size());
                            }
                            else
                            {
                                //
                                // Try to write without blocking. If that fails, use asynchronous I/O.
                                //
                                if(!_transceiver.write(os.getBuffer()))
                                {
                                    //
                                    // Begin an asynchronous write, which may result in a recursive call to this
                                    // method if the operation completes synchronously.
                                    //
                                    result = _transceiver.beginWrite(os.getBuffer(), _validateAsyncCallback, null);
                                    if(result.CompletedSynchronously)
                                    {
                                        _transceiver.endWrite(os.getBuffer(), result);
                                    }
                                    else
                                    {
                                        //
                                        // Wait until the I/O request completes.
                                        //
                                        scheduleConnectTimeout();
                                        return;
                                    }
                                }
                            }

                            Debug.Assert(os.pos() == os.size());
                        }
                        else // The client side has the passive role for connection validation.
                        {
                            IceInternal.BasicStream istr = _stream;
                            if(istr.size() == 0)
                            {
                                //
                                // We haven't tried to read yet; resize the stream to hold the header.
                                //
                                istr.resize(IceInternal.Protocol.headerSize, true);
                                istr.pos(0);
                            }

                            //
                            // Read nonblocking until we've read the entire header, or until an asynchronous
                            // I/O callback is required.
                            //
                            while(istr.pos() != istr.size())
                            {
                                if(result != null)
                                {
                                    //
                                    // An earlier asynchronous I/O request has completed.
                                    //
                                    Debug.Assert(result != null);
                                    _transceiver.endRead(istr.getBuffer(), result);
                                    result = null;
                                }

                                if(istr.pos() == istr.size())
                                {
                                    break;
                                }

                                //
                                // Try to read without blocking. If that fails, use asynchronous I/O.
                                //
                                if(!_transceiver.read(istr.getBuffer()))
                                {
                                    //
                                    // Begin an asynchronous write, which may result in a recursive call to this
                                    // method if the operation completes synchronously.
                                    //
                                    result = _transceiver.beginRead(istr.getBuffer(), _validateAsyncCallback, null);
                                    if(!result.CompletedSynchronously)
                                    {
                                        //
                                        // Wait until the I/O request completes.
                                        //
                                        scheduleConnectTimeout();
                                        return;
                                    }
                                }
                            }

                            Debug.Assert(istr.pos() == istr.size());
                            checkValidationMessage(_stream);
                        }
                    }
                    catch(LocalException ex)
                    {
                        setState(StateClosed, ex);
                        return;
                    }
                }

                _stream.reset();
                
                Debug.Assert(_state != StateClosed);
                
                //
                // We start out in holding state.
                //
                setState(StateHolding);
                
                callback = _startCallback;
                _startCallback = null;
            }
            
            if(callback != null)
            {
                callback.connectionStartCompleted(this);
            }
        }

        private bool sendMessage(OutgoingMessage message)
        {
            Debug.Assert(_state != StateClosed);
            if(_sendInProgress)
            {
                message.adopt();
                _sendStreams.AddLast(message);
                return false;
            }

            Debug.Assert(!_sendInProgress);

            //
            // Attempt to send the message without blocking. If the send blocks, we use
            // asynchronous I/O or we request the caller to call finishSendMessage() outside
            // the synchronization.
            //

            prepareMessage(message);

            //
            // Try to send the message without blocking.
            //
            if(_transceiver.write(message.stream.getBuffer()))
            {
                message.sent(this, false);
                if(_acmTimeout > 0)
                {
                    _acmAbsoluteTimeoutMillis = IceInternal.Time.currentMonotonicTimeMillis() + _acmTimeout * 1000;
                }
                return true;
            }

            _sendStreams.AddLast(message);
            _sendInProgress = true;
            message.adopt();

            //
            // It is not safe to begin an asynchronous I/O request in this thread: if the
            // thread terminates, it also terminates the I/O request. Therefore, we call
            // it from the async IO thread.
            //
            _instance.asyncIOThread().queue(writeAsync);
            return false;
        }

        private void prepareMessage(OutgoingMessage message)
        {
            Debug.Assert(!message.prepared);

            IceInternal.BasicStream stream = message.stream;

            bool compress = _overrideCompress ? _overrideCompressValue : message.compress;
            message.stream = doCompress(message.stream, compress);
            message.stream.prepareWrite();
            message.prepared = true;

            if(message.outAsync != null)
            {
                IceInternal.TraceUtil.trace("sending asynchronous request", stream, _logger, _traceLevels);
            }
            else
            {
                IceInternal.TraceUtil.traceSend(stream, _logger, _traceLevels);
            }
        }

        private IceInternal.BasicStream doCompress(IceInternal.BasicStream uncompressed, bool compress)
        {
            if(_compressionSupported)
            {
                if(compress && uncompressed.size() >= 100)
                {
                    //
                    // Do compression.
                    //
                    IceInternal.BasicStream cstream = null;
                    if(uncompressed.compress(ref cstream, IceInternal.Protocol.headerSize, _compressionLevel))
                    {
                        //
                        // Set compression status.
                        //
                        cstream.pos(9);
                        cstream.writeByte((byte)2);

                        //
                        // Write the size of the compressed stream into the header.
                        //
                        cstream.pos(10);
                        cstream.writeInt(cstream.size());

                        //
                        // Write the compression status and size of the compressed stream into the header of the
                        // uncompressed stream -- we need this to trace requests correctly.
                        //
                        uncompressed.pos(9);
                        uncompressed.writeByte((byte)2);
                        uncompressed.writeInt(cstream.size());

                        return cstream;
                    }
                }
            }

            uncompressed.pos(9);
            uncompressed.writeByte((byte)((_compressionSupported && compress) ? 1 : 0));

            //
            // Not compressed, fill in the message size.
            //
            uncompressed.pos(10);
            uncompressed.writeInt(uncompressed.size());

            return uncompressed;
        }

        private void warning(string msg, System.Exception ex)
        {
            _logger.warning(msg + ":\n" + ex + "\n" + _transceiver.ToString());
        }

        private void writeAsync(object state)
        {
            //
            // This method is called from the async IO thread, and as
            // an asynchronous I/O callback.
            //
            IAsyncResult result = (IAsyncResult)state;

            //
            // This callback is scheduled with the async IO thread when we need to begin a new asynchronous 
            // I/O operation from a safe thread (i.e., a thread that will not terminate while the I/O request
            // is pending). In this case, the argument is null.
            //
            // This callback is also invoked when an I/O request completes, in which case the result
            // argument will be non-null.
            //
            // This callback can also be invoked recursively if an I/O request completes synchronously.
            // We return immediately in this situation.
            //
            if(result != null && result.CompletedSynchronously)
            {
                return;
            }

            //
            // Use a lock to prevent race conditions, such as when an asynchronous callback is scheduled
            // to execute just after beginWrite completes.
            //
            lock(this)
            {
                bool flushSentCallbacks = _sentCallbacks.Count == 0;
                try
                {
                    Debug.Assert(_sendInProgress);
                    Debug.Assert(_sendStreams.Count > 0);
                    Debug.Assert(_transceiver != null);

                    //
                    // An I/O request has completed, so cancel a pending timeout.
                    //
                    if(result != null)
                    {
                        _timer.cancel(this);
                    }

                    if(_state == StateClosed)
                    {
                        //
                        // If the write completed and the connection is closed, we must check if the 
                        // message might have been completely sent. If that's the case, we have to 
                        // assume it's sent (even if it might not) to not break at-most once
                        // semantics.
                        //
                        if(result != null && result.IsCompleted)
                        {
                            OutgoingMessage message = _sendStreams.First.Value;
                            if(message.stream.pos() == message.stream.size())
                            {
                                message.sent(this, true);
                                if(message.outAsync is Ice.AMISentCallback)
                                {
                                    _sentCallbacks.AddLast(message);
                                }
                                _sendStreams.RemoveFirst();
                            }
                        }
                        _sendInProgress = false;
                        _threadPool.finish(this);
                        return;
                    }
                
                    while(_sendStreams.Count > 0)
                    {
                        OutgoingMessage message = _sendStreams.First.Value;
                        
                        //
                        // The message may have already been prepared and partially sent.
                        //
                        if(!message.prepared)
                        {
                            prepareMessage(message);
                        }

                        //
                        // If we have a result, it means we need to complete a pending I/O request.
                        //
                        if(result != null)
                        {
                            _transceiver.endWrite(message.stream.getBuffer(), result);
                            result = null;
                        }
                        
                        //
                        // If there's nothing left to send, dequeue the message and send another one.
                        //
                        if(message.stream.pos() == message.stream.size())
                        {
                            message.sent(this, true); // true indicates that this is called by the async callback.
                            if(message.outAsync is Ice.AMISentCallback)
                            {
                                _sentCallbacks.AddLast(message);
                            }
                            _sendStreams.RemoveFirst();
                            continue; // Begin another I/O request if necessary.
                        }

                        //
                        // Begin an asynchronous write to send the remainder of the message.
                        //
                        result = _transceiver.beginWrite(message.stream.getBuffer(), _writeAsyncCallback, message);
                        if(!result.CompletedSynchronously)
                        {
                            //
                            // We will be called again when the write completes. Schedule a timeout if necessary
                            // and return now.
                            //
                            scheduleTimeout();
                            return;
                        }

                        //
                        // At this point, the I/O request completed synchronously; loop again to complete it.
                        //
                    }
                }
                catch(LocalException ex)
                {
                    _sendInProgress = false;
                    setState(StateClosed, ex);
                    return;
                }
                finally
                {
                    if(flushSentCallbacks && _sentCallbacks.Count > 0)
                    {
                        _threadPool.execute(_flushSentCallbacks);
                    }
                }

                Debug.Assert(_sendStreams.Count == 0);
                _sendInProgress = false;
                if(_acmTimeout > 0)
                {
                    _acmAbsoluteTimeoutMillis = IceInternal.Time.currentMonotonicTimeMillis() + _acmTimeout * 1000;
                }
            }
        }

        private void flushSentCallbacks()
        {
            LinkedList<OutgoingMessage> sentCallbacks;
            lock(this)
            {
                Debug.Assert(_sentCallbacks != null && _sentCallbacks.Count > 0);
                sentCallbacks = _sentCallbacks;
                _sentCallbacks = new LinkedList<OutgoingMessage>();
            }
            foreach(OutgoingMessage message in sentCallbacks)
            {
                message.outAsync.sent__(_instance);
            }
        }

        private void message(bool safeThread, ref IncomingMessage message)
        {
            //
            // NOTE: Unlike C++ & Java, the thread pool doesn't parse the message. The message 
            // is parsed by the readAsync callback. This avoids doing extra context context 
            // switches when the message to read can be processed without blocking (this is for
            // example the case of responses to regular twoway requests).
            //

            Debug.Assert(message.stream != null);

            //
            // If we're not required to serialize the dispatching of the connection
            // messages, we can start a new read right now. Otherwise, a new read
            // we be started once the message is dispatched.
            //
            if(!_threadPool.serialize())
            {
                //
                // Don't restart the read if in holding or closed state. Read will be
                // restarted if connection becomes active/closing again.
                //
                lock(this)
                {
                    Debug.Assert(_reading);
                    if(_state == StateHolding || _state == StateClosed)
                    {
                        _reading = false;
                    }
                    else
                    {
                        Debug.Assert(_state == StateActive || _state == StateClosing);
                        startReadAsync(safeThread || _restartable);
                    }
                }
            }

            //
            // Asynchronous replies must be handled outside the thread
            // synchronization, so that nested calls are possible.
            //
            if(message.outAsync != null)
            {
                message.outAsync.finished__(message.stream);
            }

            if(message.invokeNum > 0)
            {
                invokeAll(message);
            }
            
            //
            // If we're serializing message dispatch over the connection, it's time
            // start a new read since all the messages have been dispatched.
            //
            if(_threadPool.serialize())
            {
                //
                // Don't restart the read if in holding or closed state. Read will be
                // restarted if connection becomes active/closing again.
                //
                lock(this)
                {
                    Debug.Assert(_reading);
                    if(_state == StateHolding || _state == StateClosed)
                    {
                        _reading = false;
                    }
                    else
                    {
                        Debug.Assert(_state == StateActive || _state == StateClosing);
                        startReadAsync(safeThread || _restartable);
                    }
                }
            }
        }

        private void startReadAsync(bool safeThread)
        {
            //
            // We need to start a new read operation before dispatching the incoming message.
            // However, it may not be safe to start the read from this thread because the thread
            // pool might reap it, which would cancel the pending read. The safeThread argument
            // indicates whether this thread is safe for starting asynchronous I/O operations.
            //
            // For the best performance, we optimistically start the read from this thread and
            // hope that the thread doesn't exit before the read completes. If the thread does
            // exit, we'll restart the read from a safe thread.
            //
            // Some transports don't support this activity very well. In SSL for example, a
            // restarted read returns a partial packet, causing Ice to close the connection and
            // forcing the peer to open a new one. It's better in this case to be conservative and
            // start the read from a safe thread.
            //
            if(safeThread)
            {
                //
                // Start a new read; if it completes asynchronously, we have nothing else to do
                // because the readAsync method will be called when the operation completes.
                // Otherwise, we need to schedule a work item to continue the read process.
                //
                try
                {
                    IAsyncResult result = _transceiver.beginRead(_stream.getBuffer(), _readAsyncCallback, null);
                    if(!result.CompletedSynchronously)
                    {
                        return;
                    }
                    _transceiver.endRead(_stream.getBuffer(), result);
                }
                catch(SocketException ex)
                {
                    setState(StateClosed, ex);
                }
                catch(LocalException ex)
                {
                    if(_endpoint.datagram())
                    {
                        if(_warn)
                        {
                            warning("datagram connection exception", ex);
                        }
                    }
                    else
                    {
                        setState(StateClosed, ex);
                    }
                }
            }
            
            //
            // Schedule a work item to start a new read operation from a safe thread.
            //
            _instance.asyncIOThread().queue(readAsync);
        }

        private void readAsync(object state)
        {
            //
            // This method is responsible for reading all messages once this connection is validated.
            // It ensures that a read is always pending until the connection is closed or an exception
            // occurs.
            //
            // Usually this method is invoked as an AsyncCallback, i.e., when an asynchronous I/O
            // operation completes. It can also be invoked from the async IO thread.
            //
            // Return immediately if called as the result of a read operation completing synchronously.
            //
            IAsyncResult result = (IAsyncResult)state;
            if(result != null && result.CompletedSynchronously)
            {
                return;
            }

            try
            {
                IceInternal.BasicStream stream = null;
                IncomingMessage message = new IncomingMessage();
                lock(this)
                {
                    //
                    // We're done if the connection has been closed.
                    //
                    if(_state == StateClosed)
                    {
                        return;
                    }

                    Debug.Assert(_transceiver != null);
                    bool parseHeader = _stream.isEmpty() || _stream.pos() <= IceInternal.Protocol.headerSize;

                    //
                    // Complete an asynchronous read operation if necessary. This may raise a SocketException
                    // if the thread that called beginRead has exited. In this case, we restart the read from
                    // the current thread (which we know is a .NET thread pool thread).
                    //
                    if(result != null)
                    {
                        Debug.Assert(!result.CompletedSynchronously);
                        _transceiver.endRead(_stream.getBuffer(), result);
                    }

                    //
                    // Check for StateHold *after* completing any pending asynchronous operation.
                    //
                    if(_state == StateHolding)
                    {
                        _reading = false;
                        return;
                    }

                    //
                    // If we're about to read a new message, resize the stream to hold the header.
                    //
                    if(_stream.size() == 0)
                    {
                        _stream.resize(IceInternal.Protocol.headerSize, true);
                        _stream.pos(0);
                    }

                    //
                    // This loop reads without blocking and stops when the message is complete or an asynchronous
                    // callback is required.
                    //
                    while(true)
                    {
                        int pos = _stream.pos();

                        //
                        // For a datagram connection, each read on the transceiver produces a new message.
                        // If we've already read once (i.e., pos > 0), we need to check the message.
                        //
                        if(_endpoint.datagram() && pos > 0 && pos < IceInternal.Protocol.headerSize)
                        {
                            //
                            // This situation is possible for small UDP packets.
                            //
                            throw new Ice.IllegalMessageSizeException();
                        }

                        //
                        // When we've read enough to fill out the header, we need to validate it. The stream
                        // will be enlarged if necessary to contain the entire message.
                        //
                        if(parseHeader && pos >= IceInternal.Protocol.headerSize)
                        {
                            validateHeader(_stream);

                            //
                            // Check for a datagram packet that was smaller than expected. If so, throw an
                            // exception. We don't continue inside the loop because a malicious client could
                            // tie up the thread indefinitely by sending bogus datagrams.
                            //
                            if(_endpoint.datagram() && pos != _stream.size())
                            {
                                if(_warnUdp)
                                {
                                    _logger.warning("DatagramLimitException: maximum size of " + _stream.pos() + 
                                                    " exceeded");
                                }
                                throw new Ice.DatagramLimitException();
                            }
                        }

                        //
                        // We're done when there's nothing left to read.
                        //
                        if(pos == _stream.size())
                        {
                            break;
                        }

                        //
                        // Try a nonblocking read first.
                        //
                        if(!_transceiver.read(_stream.getBuffer()))
                        {
                            //
                            // We couldn't read all of the data, so use an asynchronous operation.
                            //
                            result = _transceiver.beginRead(_stream.getBuffer(), _readAsyncCallback, null);
                            if(!result.CompletedSynchronously)
                            {
                                return; // This method is called again when the read completes.
                            }
                            _transceiver.endRead(_stream.getBuffer(), result);
                        }
                    }

                    Debug.Assert(_stream.pos() == _stream.size());

                    //
                    // Preserve the stream before another thread begins to read.
                    //
                    stream = new IceInternal.BasicStream(_instance);
                    stream.swap(_stream);
                    _stream.resize(IceInternal.Protocol.headerSize, true); // Make room for the next header.
                    _stream.pos(0);

                    //
                    // Parse the message. Note that this may involve decompressing the message.
                    //
                    parseMessage(stream, ref message);

                    //
                    // parseMessage() can close the connection, so we must check for closed state again.
                    //
                    if(_state == StateClosed)
                    {
                        return;
                    }

                    //
                    // At this point we have received and parsed a complete message. We have two responsibilities:
                    //
                    // 1. Dispatch the message, if necessary (parseMessage may have already handled the message).
                    // 2. Start another read.
                    //
                    if(message.stream != null)
                    {
                        _threadPool.execute(delegate(bool safeThread)
                                            {
                                                this.message(safeThread, ref message);
                                            });
                    }
                    else
                    {
                        //
                        // Note: we start another read rather than just calling readAsync(null). This is
                        // to ensure fairness and avoid tying up this thread to read indefinitely over 
                        // the connection in case data is always available over this connection.
                        //
                        startReadAsync(true);
                    }
                }                    
            }
            catch(DatagramLimitException)
            {
                //
                // Expected. Restart the read.
                //
                _stream.resize(0, true);
                readAsync(null);
            }
            catch(IceInternal.ReadAbortedException)
            {
                //
                // The thread that called beginRead has exited (e.g., it was reaped by the thread pool),
                // so we need to initiate a new read.
                //
                readAsync(null);
            }
            catch(SocketException ex)
            {
                exception(ex);
            }
            catch(LocalException ex)
            {
                if(_endpoint.datagram())
                {
                    if(_instance.initializationData().properties.getPropertyAsInt("Ice.Warn.Connections") > 0)
                    {
                        warning("datagram connection exception", ex);
                    }

                    //
                    // Restart the read.
                    //
                    _stream.resize(0, true);
                    readAsync(null);
                }
                else
                {
                    exception(ex);
                }
            }
            catch(System.Exception ex)
            {
                exception(new UnknownException(ex.ToString()));

                //
                // All relevant exceptions should have been caught. We log it here because
                // the .NET thread silently ignores exceptions.
                //
                _logger.error("exception in readAsync for connection:\n" + _desc + "\n" + ex);
            }
        }

        private void validateHeader(IceInternal.BasicStream stream)
        {
            //
            // Validate the message header.
            //

            //
            // The stream's current position can be larger than Protocol.headerSize in the case of datagrams.
            //
            int pos = stream.pos();
            Debug.Assert(pos >= IceInternal.Protocol.headerSize);

            byte[] bytes = stream.getBuffer().b.rawBytes();
            if(bytes[0] != IceInternal.Protocol.magic[0] || bytes[1] != IceInternal.Protocol.magic[1] ||
               bytes[2] != IceInternal.Protocol.magic[2] || bytes[3] != IceInternal.Protocol.magic[3])
            {
                Ice.BadMagicException ex = new Ice.BadMagicException();
                ex.badMagic = stream.getBuffer().b.toArray(0, 4);
                throw ex;
            }

            if(bytes[4] != IceInternal.Protocol.protocolMajor || bytes[5] > IceInternal.Protocol.protocolMinor)
            {
                Ice.UnsupportedProtocolException e = new Ice.UnsupportedProtocolException();
                e.badMajor = bytes[4] < 0 ? bytes[4] + 255 : bytes[4];
                e.badMinor = bytes[5] < 0 ? bytes[5] + 255 : bytes[5];
                e.major = IceInternal.Protocol.protocolMajor;
                e.minor = IceInternal.Protocol.protocolMinor;
                throw e;
            }

            if(bytes[6] != IceInternal.Protocol.encodingMajor || bytes[7] > IceInternal.Protocol.encodingMinor)
            {
                Ice.UnsupportedEncodingException e = new Ice.UnsupportedEncodingException();
                e.badMajor = bytes[6] < 0 ? bytes[6] + 255 : bytes[6];
                e.badMinor = bytes[7] < 0 ? bytes[7] + 255 : bytes[7];
                e.major = IceInternal.Protocol.encodingMajor;
                e.minor = IceInternal.Protocol.encodingMinor;
                throw e;
            }

            stream.pos(10);
            int size = stream.readInt();

            if(size < IceInternal.Protocol.headerSize)
            {
                throw new Ice.IllegalMessageSizeException();
            }
            if(size > _instance.messageSizeMax())
            {
                IceInternal.Ex.throwMemoryLimitException(size, _instance.messageSizeMax());
            }
            if(size > stream.size())
            {
                stream.resize(size, true);
            }
            stream.pos(pos);
        }

        private void checkValidationMessage(IceInternal.BasicStream stream)
        {
            Debug.Assert(stream.pos() == IceInternal.Protocol.headerSize);
            validateHeader(stream);

            stream.pos(8);
            byte messageType = stream.readByte();
            if(messageType != IceInternal.Protocol.validateConnectionMsg)
            {
                throw new ConnectionNotValidatedException();
            }

            if(stream.size() != IceInternal.Protocol.headerSize)
            {
                throw new IllegalMessageSizeException();
            }
            IceInternal.TraceUtil.traceRecv(stream, _logger, _traceLevels);
        }

        private void parseMessage(IceInternal.BasicStream stream, ref IncomingMessage message)
        {
            Debug.Assert(_state > StateNotValidated && _state < StateClosed);

            if(_acmTimeout > 0)
            {
                _acmAbsoluteTimeoutMillis = IceInternal.Time.currentMonotonicTimeMillis() + _acmTimeout * 1000;
            }

            try
            {
                //
                // The magic and version fields have already been checked.
                //
                Debug.Assert(stream.pos() == stream.size());
                stream.pos(8);
                byte messageType = stream.readByte();
                byte compress = stream.readByte();
                if(compress == (byte)2)
                {
                    if(_compressionSupported)
                    {
                        stream = stream.uncompress(IceInternal.Protocol.headerSize);
                    }
                    else
                    {
                        FeatureNotSupportedException ex = new FeatureNotSupportedException();
                        ex.unsupportedFeature = "Cannot uncompress compressed message: bzip2 DLL not found";
                        throw ex;
                    }
                }
                stream.pos(IceInternal.Protocol.headerSize);

                switch(messageType)
                {
                    case IceInternal.Protocol.closeConnectionMsg:
                    {
                        IceInternal.TraceUtil.traceRecv(stream, _logger, _traceLevels);
                        if(_endpoint.datagram())
                        {
                            if(_warn)
                            {
                                _logger.warning("ignoring close connection message for datagram connection:\n"
                                                + _desc);
                            }
                        }
                        else
                        {
                            setState(StateClosed, new CloseConnectionException());
                        }
                        break;
                    }

                    case IceInternal.Protocol.requestMsg:
                    {
                        if(_state == StateClosing)
                        {
                            IceInternal.TraceUtil.trace("received request during closing\n" +
                                                        "(ignored by server, client will retry)", stream, _logger,
                                                        _traceLevels);
                        }
                        else
                        {
                            IceInternal.TraceUtil.traceRecv(stream, _logger, _traceLevels);
                            message.stream = stream;
                            message.compress = compress;
                            message.requestId = stream.readInt();
                            message.invokeNum = 1;
                            message.servantManager = _servantManager;
                            message.adapter = _adapter;
                            ++_dispatchCount;
                        }
                        break;
                    }

                    case IceInternal.Protocol.requestBatchMsg:
                    {
                        if(_state == StateClosing)
                        {
                            IceInternal.TraceUtil.trace("received batch request during closing\n" +
                                                        "(ignored by server, client will retry)", stream, _logger,
                                                        _traceLevels);
                        }
                        else
                        {
                            IceInternal.TraceUtil.traceRecv(stream, _logger, _traceLevels);
                            message.stream = stream;
                            message.compress = compress;
                            message.invokeNum = stream.readInt();
                            if(message.invokeNum < 0)
                            {
                                message.invokeNum = 0;
                                throw new UnmarshalOutOfBoundsException();
                            }
                            message.servantManager = _servantManager;
                            message.adapter = _adapter;
                            _dispatchCount += message.invokeNum;
                        }
                        break;
                    }

                    case IceInternal.Protocol.replyMsg:
                    {
                        IceInternal.TraceUtil.traceRecv(stream, _logger, _traceLevels);
                        int requestId = stream.readInt();
                        IceInternal.Outgoing og = null;
                        if(_requests.TryGetValue(requestId, out og))
                        {
                            _requests.Remove(requestId);
                            og.finished(stream);
                        }
                        else
                        {
                            if(!_asyncRequests.ContainsKey(requestId))
                            {
                                throw new UnknownRequestIdException();
                            }
                            message.stream = stream;
                            message.requestId = requestId;
                            message.compress = compress;
                            message.outAsync = _asyncRequests[requestId];
                            _asyncRequests.Remove(requestId);
                        }
                        Monitor.PulseAll(this); // Notify threads blocked in close(false)
                        break;
                    }

                    case IceInternal.Protocol.validateConnectionMsg:
                    {
                        IceInternal.TraceUtil.traceRecv(stream, _logger, _traceLevels);
                        if(_warn)
                        {
                            _logger.warning("ignoring unexpected validate connection message:\n" + _desc);
                        }
                        break;
                    }

                    default:
                    {
                        IceInternal.TraceUtil.trace("received unknown message\n(invalid, closing connection)", stream,
                                                    _logger, _traceLevels);
                        throw new UnknownMessageException();
                    }
                }
            }
            catch(LocalException ex)
            {
                if(_endpoint.datagram())
                {
                    if(_warn)
                    {
                        _logger.warning("datagram connection exception:\n" + ex.ToString() + "\n" + _desc);
                    }
                }
                else
                {
                    setState(StateClosed, ex);
                }
            }
        }

        private void invokeAll(IncomingMessage message)
        {
            Debug.Assert(message.stream != null);

            //
            // Note: In contrast to other private or protected methods, this
            // operation must be called *without* the mutex locked.
            //

            IceInternal.Incoming inc = null;
            try
            {
                while(message.invokeNum > 0)
                {
                    //
                    // Prepare the invocation.
                    //
                    bool response = !_endpoint.datagram() && message.requestId != 0;
                    inc = getIncoming(message.adapter, response, message.compress,
                                      message.requestId);
                    IceInternal.BasicStream ins = inc.istr();
                    message.stream.swap(ins);
                    IceInternal.BasicStream os = inc.ostr();

                    //
                    // Prepare the response if necessary.
                    //
                    if(response)
                    {
                        Debug.Assert(message.invokeNum == 1); // No further invocations if a response is expected.
                        os.writeBlob(IceInternal.Protocol.replyHdr);

                        //
                        // Add the request ID.
                        //
                        os.writeInt(message.requestId);
                    }

                    inc.invoke(message.servantManager);

                    //
                    // If there are more invocations, we need the stream back.
                    //
                    if(--message.invokeNum > 0)
                    {
                        message.stream.swap(ins);
                    }

                    reclaimIncoming(inc);
                    inc = null;
                }
            }
            catch(LocalException ex)
            {
                invokeException(ex, message.invokeNum);
            }
            finally
            {
                if(inc != null)
                {
                    reclaimIncoming(inc);
                }
            }
        }

        public void finished(IceInternal.ThreadPool unused)
        {
            lock(this)
            {
                //
                // It's important to keep this synchronization here as this is called from a
                // thread from the thread pool and we want to make sure that the thread that
                // called _threadPool.finish() is finished with changing the connection state.
                //
                Debug.Assert(_state == StateClosed && !_sendInProgress);
            }

            if(_startCallback != null)
            {
                _startCallback.connectionStartFailed(this, _exception);
                _startCallback = null;
            }

            foreach(OutgoingMessage message in _sendStreams)
            {
                message.finished(_exception);
            }
            _sendStreams.Clear();

            foreach(IceInternal.Outgoing o in _requests.Values)
            {
                o.finished(_exception);
            }
            _requests.Clear();

            foreach(IceInternal.OutgoingAsync o in _asyncRequests.Values)
            {
                o.finished__(_exception);
            }
            _asyncRequests.Clear();

            //
            // This must be done last as this will cause waitUntilFinished() to return (and communicator
            // objects such as the timer might be destroyed too).
            //
            lock(this)
            {
                _transceiver = null;
                Monitor.PulseAll(this);
            }
        }

        private IceInternal.Incoming getIncoming(ObjectAdapter adapter, bool response, byte compress, int requestId)
        {
            IceInternal.Incoming inc = null;

            if(_cacheBuffers)
            {
                lock(_incomingCacheMutex)
                {
                    if(_incomingCache == null)
                    {
                        inc = new IceInternal.Incoming(_instance, this, adapter, response, compress, requestId);
                    }
                    else
                    {
                        inc = _incomingCache;
                        _incomingCache = _incomingCache.next;
                        inc.reset(_instance, this, adapter, response, compress, requestId);
                        inc.next = null;
                    }
                }
            }
            else
            {
                inc = new IceInternal.Incoming(_instance, this, adapter, response, compress, requestId);
            }

            return inc;
        }

        private void reclaimIncoming(IceInternal.Incoming inc)
        {
            if(_cacheBuffers)
            {
                lock(_incomingCacheMutex)
                {
                    inc.next = _incomingCache;
                    _incomingCache = inc;
                    //
                    // Clear references to Ice objects as soon as possible.
                    //
                    _incomingCache.reclaim();
                }
            }
        }

        public IceInternal.Outgoing getOutgoing(IceInternal.RequestHandler handler, string operation,
                                                OperationMode mode, Dictionary<string, string> context)
        {
            IceInternal.Outgoing og = null;

            if(_cacheBuffers)
            {
                lock(_outgoingCacheMutex)
                {
                    if(_outgoingCache == null)
                    {
                        og = new IceInternal.Outgoing(handler, operation, mode, context);
                    }
                    else
                    {
                        og = _outgoingCache;
                        _outgoingCache = _outgoingCache.next;
                        og.reset(handler, operation, mode, context);
                        og.next = null;
                    }
                }
            }
            else
            {
                og = new IceInternal.Outgoing(handler, operation, mode, context);
            }

            return og;
        }

        public void reclaimOutgoing(IceInternal.Outgoing og)
        {
            if(_cacheBuffers)
            {
                //
                // Clear references to Ice objects as soon as possible.
                //
                og.reclaim();

                lock(_outgoingCacheMutex)
                {
                    og.next = _outgoingCache;
                    _outgoingCache = og;
                }
            }
        }

        private void scheduleConnectTimeout()
        {
            int timeout;
            IceInternal.DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
            if(defaultsAndOverrides.overrideConnectTimeout)
            {
                timeout = defaultsAndOverrides.overrideConnectTimeoutValue;
            }
            else
            {
                timeout = _endpoint.timeout();
            }

            if(timeout >= 0)
            {
                _timer.schedule(this, timeout);
            }
        }

        private void scheduleTimeout()
        {
            if(_endpoint.timeout() >= 0)
            {
                _timer.schedule(this, _endpoint.timeout());
            }
        }

        //
        // Timer.TimerTask method.
        //
        public void runTimerTask()
        {
            lock(this)
            {
                if(_state <= StateNotValidated)
                {
                    setState(StateClosed, new ConnectTimeoutException());
                }
                else if(_state <= StateClosing)
                {
                    setState(StateClosed, new TimeoutException());
                }
            }
        }

        private struct IncomingMessage
        {
            public IceInternal.BasicStream stream;
            public int invokeNum;
            public int requestId;
            public byte compress;
            public IceInternal.ServantManager servantManager;
            public ObjectAdapter adapter;
            public IceInternal.OutgoingAsync outAsync;
        }

        private class OutgoingMessage
        {
            internal OutgoingMessage(IceInternal.BasicStream stream, bool compress, bool adopt)
            {
                this.stream = stream;
                this.compress = compress;
                this._adopt = adopt;
            }

            internal OutgoingMessage(IceInternal.OutgoingMessageCallback @out, IceInternal.BasicStream stream,
                                     bool compress, bool resp)
            {
                this.stream = stream;
                this.compress = compress;
                this.@out = @out;
                this.response = resp;
            }

            internal OutgoingMessage(IceInternal.OutgoingAsyncMessageCallback @out, IceInternal.BasicStream stream,
                                     bool compress, bool resp)
            {
                this.stream = stream;
                this.compress = compress;
                this.outAsync = @out;
                this.response = resp;
            }

            internal void adopt()
            {
                if(_adopt)
                {
                    IceInternal.BasicStream stream = new IceInternal.BasicStream(this.stream.instance());
                    stream.swap(this.stream);
                    this.stream = stream;
                    _adopt = false;
                }
            }

            internal void sent(ConnectionI connection, bool notify)
            {
                if(@out != null)
                {
                    @out.sent(notify); // true = notify the waiting thread that the request was sent.
                }
                else if(outAsync != null)
                {
                    outAsync.sent__(connection);
                }
            }

            internal void finished(LocalException ex)
            {
                //
                // Only notify oneway requests. The connection keeps track of twoway
                // requests in the _requests/_asyncRequests maps and will notify them
                // of the connection exceptions.
                //
                if(!response)
                {
                    if(@out != null)
                    {
                        @out.finished(ex);
                    }
                    else if(outAsync != null)
                    {
                        outAsync.finished__(ex);
                    }
                }
            }

            internal IceInternal.BasicStream stream;
            internal IceInternal.OutgoingMessageCallback @out;
            internal IceInternal.OutgoingAsyncMessageCallback outAsync;
            internal bool compress;
            internal bool response;
            internal bool _adopt;
            internal bool prepared;
        }

        private IceInternal.Instance _instance;

        private IceInternal.Transceiver _transceiver;
        private bool _restartable;

        private string _desc;
        private string _type;
        private IceInternal.EndpointI _endpoint;

        private ObjectAdapter _adapter;
        private IceInternal.ServantManager _servantManager;

        private Logger _logger;
        private IceInternal.TraceLevels _traceLevels;

        private IceInternal.ThreadPool _threadPool;

        private IceInternal.BasicStream _stream;

        private AsyncCallback _initializeAsyncCallback;
        private AsyncCallback _validateAsyncCallback;
        private AsyncCallback _writeAsyncCallback;
        private AsyncCallback _readAsyncCallback;
        private bool _reading;

        private IceInternal.Timer _timer;

        private StartCallback _startCallback = null;

        private bool _warn;
        private bool _warnUdp;

        private int _acmTimeout;
        private long _acmAbsoluteTimeoutMillis;

        private int _compressionLevel;

        private int _nextRequestId;

        private Dictionary<int, IceInternal.Outgoing> _requests = new Dictionary<int, IceInternal.Outgoing>();
        private Dictionary<int, IceInternal.OutgoingAsync> _asyncRequests =
            new Dictionary<int, IceInternal.OutgoingAsync>();

        private LocalException _exception;

        private bool _batchAutoFlush;
        private IceInternal.BasicStream _batchStream;
        private bool _batchStreamInUse;
        private int _batchRequestNum;
        private bool _batchRequestCompress;
        private int _batchMarker;

        private LinkedList<OutgoingMessage> _sendStreams = new LinkedList<OutgoingMessage>();
        private bool _sendInProgress;

        private LinkedList<OutgoingMessage> _sentCallbacks = new LinkedList<OutgoingMessage>();
        private IceInternal.ThreadPoolWorkItem _flushSentCallbacks;
        
        private int _dispatchCount;

        private int _state; // The current state.
        private long _stateTime; // The last time when the state was changed.

        private IceInternal.Incoming _incomingCache;
        private object _incomingCacheMutex = new object();

        private IceInternal.Outgoing _outgoingCache;
        private object _outgoingCacheMutex = new object();

        private static bool _compressionSupported;

        private bool _overrideCompress;
        private bool _overrideCompressValue;
        private bool _cacheBuffers;
    }
}
