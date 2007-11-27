// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

    public sealed class ConnectionI : IceInternal.EventHandler, Connection,
                                      IceInternal.SelectorThread.SocketReadyCallback
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
                    _startCallback = callback;

                    //
                    // The connection might already be closed if the communicator was destroyed.
                    //
                    if(_state == StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }

                    //
                    // In thread per connection mode, we create the thread for the connection. The
                    // intialization and validation of the connection is taken care of by the thread
                    // per connection. If a callback is given, no need to wait, the thread will notify
                    // the callback, otherwise wait until the connection is validated.
                    //
                    if(_threadPerConnection)
                    {
                        try
                        {
                            _thread = new Thread(new ThreadStart(RunThreadPerConnection));
                            _thread.IsBackground = true;
                            _thread.Start();
                        }
                        catch(System.Exception ex)
                        {
                            _logger.error("cannot create thread for connection:\n" + ex);

                            //
                            // Clean up.
                            //
                            _thread = null;
                            _state = StateClosed;

                            throw new SyscallException(ex);
                        }

                        if(callback == null) // Wait for the connection to be validated.
                        {
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
                        return; // We're done.
                    }
                }

                Debug.Assert(!_threadPerConnection);

                //
                // Initialize the connection transceiver and then validate the connection.
                //
                IceInternal.SocketStatus status = initialize();
                if(status == IceInternal.SocketStatus.Finished)
                {
                    status = validate();
                }

                if(status == IceInternal.SocketStatus.Finished)
                {
                    finishStart(null);
                    return; // We're done!
                }

                //
                // If the initialization or validation couldn't be completed without potentially
                // blocking, we register the connection with the selector thread and return.
                //

                lock(this)
                {
                    if(_state == StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }

                    int timeout;
                    IceInternal.DefaultsAndOverrides defaultsAndOverrides = instance_.defaultsAndOverrides();
                    if(defaultsAndOverrides.overrideConnectTimeout)
                    {
                        timeout = defaultsAndOverrides.overrideConnectTimeoutValue;
                    }
                    else
                    {
                        timeout = _endpoint.timeout();
                    }

                    _sendInProgress = true;
                    _selectorThread.register(_transceiver.fd(), this, status, timeout);
                }
            }
            catch(LocalException ex)
            {
                lock(this)
                {
                    setState(StateClosed, ex);

                    //
                    // If start is called with a callback, the callback is notified either by the
                    // thread per conncetion or the thread pool.
                    //
                    if(callback != null)
                    {
                        if(!_threadPerConnection)
                        {
                            registerWithPool();
                            unregisterWithPool(); // Let finished() do the close.
                        }
                        return;
                    }

                    //
                    // Close the transceiver if there's no thread per connection. Otherwise, wait
                    // for the thread per connection to take care of it.
                    //
                    if(_thread == null && _transceiver != null)
                    {
                        try
                        {
                            _transceiver.close();
                        }
                        catch(LocalException)
                        {
                            // Here we ignore any exceptions in close().
                        }
                        _transceiver = null;
                    }
                }

                waitUntilFinished();
                throw ex;
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
            bool send = false;
            lock(this)
            {
                switch(reason)
                {
                    case ObjectAdapterDeactivated:
                    {
                        send = setState(StateClosing, new ObjectAdapterDeactivatedException());
                        break;
                    }

                    case CommunicatorDestroyed:
                    {
                        send = setState(StateClosing, new CommunicatorDestroyedException());
                        break;
                    }
                }
            }

            if(send)
            {
                try
                {
                    finishSendMessage();
                }
                catch(LocalException)
                {
                    // Ignore.
                }
            }
        }

        public void close(bool force)
        {
            bool send = false;
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

                    send = setState(StateClosing, new CloseConnectionException());
                }
            }

            if(send)
            {
                try
                {
                    finishSendMessage();
                }
                catch(LocalException)
                {
                    // Ignore.
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
            Thread threadPerConnection;

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
                if(_transceiver != null || _dispatchCount != 0 || (_thread != null && _thread.IsAlive))
                {
                    return false;
                }

                Debug.Assert(_state == StateClosed);

                threadPerConnection = _thread;
                _thread = null;
            }
            finally
            {
                Monitor.Exit(this);
            }

            if(threadPerConnection != null)
            {
                threadPerConnection.Join();
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
            Thread threadPerConnection;

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
                        // No return here, we must still wait until
                        // close() is called on the _transceiver.
                        //
                    }
                    else
                    {
                        Monitor.Wait(this);
                    }
                }

                Debug.Assert(_state == StateClosed);

                threadPerConnection = _thread;
                _thread = null;

                //
                // Clear the OA. See bug 1673 for the details of why this is necessary.
                //
                _adapter = null;
            }

            if(threadPerConnection != null)
            {
                threadPerConnection.Join();
            }
        }

        public void monitor()
        {
            bool send = false;

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

                if(IceInternal.Time.currentMonotonicTimeMillis() >= _acmAbsoluteTimeoutMillis)
                {
                    send = setState(StateClosing, new ConnectionTimeoutException());
                }
            }
            finally
            {
                Monitor.Exit(this);
            }

            if(send)
            {
                try
                {
                    finishSendMessage();
                }
                catch(LocalException)
                {
                    // Ignore.
                }
            }
        }

        public bool sendRequest(IceInternal.Outgoing og, bool compress, bool response)
        {
            int requestId = 0;
            IceInternal.BasicStream os = og.ostr();
            bool send = false;

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
                // to _sendStreams and it will be sent by the selector thread or by this thread
                // if flush is true.
                //
                try
                {
                    send = sendMessage(new OutgoingMessage(og, og.ostr(), compress, response), false);
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

                if(!send)
                {
                    return !_sendInProgress && _queuedStreams.Count == 0; // The request was sent if it's not queued!
                }
            }

            if(send)
            {
                try
                {
                    finishSendMessage();
                }
                catch(LocalException)
                {
                    Debug.Assert(_exception != null);
                    if(!response) // Twoway calls are notified through finished()
                    {
                        throw;
                    }
                }
            }

            return true; // The request was sent.
        }

        public void sendAsyncRequest(IceInternal.OutgoingAsync og, bool compress, bool response)
        {
            int requestId = 0;
            IceInternal.BasicStream os = og.ostr__();
            bool send = false;

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

                try
                {
                    send = sendMessage(new OutgoingMessage(og, og.ostr__(), compress, response), false);
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
            }

            if(send)
            {
                try
                {
                    finishSendMessage();
                }
                catch(LocalException)
                {
                    Debug.Assert(_exception != null);
                    if(!response) // Twoway calls are notified through finished()
                    {
                        throw;
                    }
                }
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
                    throw _exception;
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
            bool send = false;
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
                        // Throw memory limit exception if the first message added causes us to go over
                        // limit. Otherwise put aside the marshalled message that caused limit to be
                        // exceeded and rollback stream to the marker.
                        //
                        try
                        {
                            _transceiver.checkSendSize(_batchStream.getBuffer(), instance_.messageSizeMax());
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

                        try
                        {
                            //
                            // Fill in the number of requests in the batch.
                            //
                            _batchStream.pos(IceInternal.Protocol.headerSize);
                            _batchStream.writeInt(_batchRequestNum);

                            OutgoingMessage message = new OutgoingMessage(_batchStream, _batchRequestCompress, true);
                            send = sendMessage(message, false);
                            if(send)
                            {
                                //
                                // If the request can't be sent immediately and this is a foreground send,
                                // we adopt the stream to be able to re-use _batchStream immediately.
                                //
                                message.adopt();
                            }
                        }
                        catch(Ice.LocalException ex)
                        {
                            setState(StateClosed, ex);
                            Debug.Assert(_exception != null);
                            throw _exception;
                        }

                        //
                        // Reset the batch stream.
                        //
                        _batchStream = new IceInternal.BasicStream(instance_, _batchAutoFlush);
                        _batchRequestNum = 0;
                        _batchRequestCompress = false;
                        _batchMarker = 0;

                        //
                        // Check again if the last request doesn't exceed the maximum message size.
                        //
                        if(IceInternal.Protocol.requestBatchHdr.Length + lastRequest.Length >
                           instance_.messageSizeMax())
                        {
                            throw new MemoryLimitException();
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
                if(send)
                {
                    finishSendMessage(); // Let exceptions go through to report auto-flush failures to the caller.
                }
                throw;
            }

            if(send)
            {
                finishSendMessage(); // Let exceptions go through to report auto-flush failures to the caller.
            }
        }

        public void abortBatchRequest()
        {
            lock(this)
            {
                _batchStream = new IceInternal.BasicStream(instance_, _batchAutoFlush);
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
            IceInternal.BatchOutgoing @out = new IceInternal.BatchOutgoing(this, instance_);
            @out.invoke();
        }

        public bool flushBatchRequests(IceInternal.BatchOutgoing @out)
        {
            bool send = false;
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
                    return true;
                }

                //
                // Fill in the number of requests in the batch.
                //
                _batchStream.pos(IceInternal.Protocol.headerSize);
                _batchStream.writeInt(_batchRequestNum);

                _batchStream.swap(@out.ostr());

                try
                {
                    OutgoingMessage message = new OutgoingMessage(@out, @out.ostr(), _batchRequestCompress, false);
                    send = sendMessage(message, false);
                }
                catch(Ice.LocalException ex)
                {
                    setState(StateClosed, ex);
                    Debug.Assert(_exception != null);
                    throw _exception;
                }

                //
                // Reset the batch stream.
                //
                _batchStream = new IceInternal.BasicStream(instance_, _batchAutoFlush);
                _batchRequestNum = 0;
                _batchRequestCompress = false;
                _batchMarker = 0;

                if(!send)
                {
                    return !_sendInProgress && _queuedStreams.Count == 0; // The request was sent if it's not queued!
                }
            }

            if(send)
            {
                finishSendMessage();
            }
            return true;
        }

        public void flushAsyncBatchRequests(IceInternal.BatchOutgoingAsync outAsync)
        {
            bool send = false;
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
                    return;
                }

                //
                // Fill in the number of requests in the batch.
                //
                _batchStream.pos(IceInternal.Protocol.headerSize);
                _batchStream.writeInt(_batchRequestNum);

                _batchStream.swap(outAsync.ostr__());

                try
                {
                    OutgoingMessage message = new OutgoingMessage(outAsync, outAsync.ostr__(), _batchRequestCompress,
                                                                  false);
                    send = sendMessage(message, false);
                }
                catch(Ice.LocalException ex)
                {
                    setState(StateClosed, ex);
                    Debug.Assert(_exception != null);
                    throw _exception;
                }

                //
                // Reset the batch stream.
                //
                _batchStream = new IceInternal.BasicStream(instance_, _batchAutoFlush);
                _batchRequestNum = 0;
                _batchRequestCompress = false;
                _batchMarker = 0;
            }

            if(send)
            {
                finishSendMessage();
            }
        }

        public void sendResponse(IceInternal.BasicStream os, byte compressFlag)
        {
            bool send = false;
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

                    send = sendMessage(new OutgoingMessage(os, compressFlag != 0, true), false);

                    if(_state == StateClosing && _dispatchCount == 0)
                    {
                        initiateShutdown(true);
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

            if(send)
            {
                try
                {
                    finishSendMessage();
                }
                catch(LocalException)
                {
                    // Ignore.
                }
            }
        }

        public void sendNoResponse()
        {
            bool send = false;
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
                        send = initiateShutdown(false);
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

            if(send)
            {
                try
                {
                    finishSendMessage();
                }
                catch(LocalException)
                {
                    // Ignore.
                }
            }
        }

        public IceInternal.EndpointI endpoint()
        {
            return _endpoint; // No mutex protection necessary, _endpoint is immutable.
        }

        public bool threadPerConnection()
        {
            return _threadPerConnection; // No mutex protection necessary, _threadPerConnection is immutable.
        }

        public void setAdapter(ObjectAdapter adapter)
        {
            lock(this)
            {
                if(_state == StateClosing || _state == StateClosed)
                {
                    Debug.Assert(_exception != null);
                    throw _exception;
                }
                else if(_state <= StateNotValidated)
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
                // We never change the thread pool with which we were
                // initially registered, even if we add or remove an
                // object adapter.
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
            ConnectionI[] connections = new ConnectionI[1];
            connections[0] = this;
            IceInternal.Reference @ref = instance_.referenceFactory().create(
                ident, instance_.getDefaultContext(), "", IceInternal.Reference.Mode.ModeTwoway,
                connections);
            return instance_.proxyFactory().referenceToProxy(@ref);
        }

        //
        // Operations from EventHandler
        //

        public override bool datagram()
        {
            Debug.Assert(!_threadPerConnection); // Only for use with a thread pool.
            return _endpoint.datagram(); // No mutex protection necessary, _endpoint is immutable.
        }

        public override bool readable()
        {
            Debug.Assert(!_threadPerConnection); // Only for use with a thread pool.
            return true;
        }

        public override bool read(IceInternal.BasicStream stream)
        {
            Debug.Assert(!_threadPerConnection); // Only for use with a thread pool.

            return _transceiver.read(stream.getBuffer(), 0);

            //
            // Updating _acmAbsoluteTimeoutMillis is too expensive here,
            // because we would have to acquire a lock just for this
            // purpose. Instead, we update _acmAbsoluteTimeoutMillis in
            // message().
            //
        }

        public override void message(IceInternal.BasicStream stream, IceInternal.ThreadPool threadPool)
        {
            Debug.Assert(!_threadPerConnection); // Only for use with a thread pool.

            byte compress = 0;
            int requestId = 0;
            int invokeNum = 0;
            IceInternal.ServantManager servantManager = null;
            ObjectAdapter adapter = null;
            IceInternal.OutgoingAsync outAsync = null;

            lock(this)
            {
                //
                // We must promote within the synchronization, otherwise
                // there could be various race conditions with close
                // connection messages and other messages.
                //
                threadPool.promoteFollower();

                if(_state != StateClosed)
                {
                    parseMessage(ref stream, ref invokeNum, ref requestId, ref compress, ref servantManager,
                                 ref adapter, ref outAsync);
                }

                //
                // parseMessage() can close the connection, so we must check
                // for closed state again.
                //
                if(_state == StateClosed)
                {
                    return;
                }
            }

            //
            // Asynchronous replies must be handled outside the thread
            // synchronization, so that nested calls are possible.
            //
            if(outAsync != null)
            {
                outAsync.finished__(stream);
            }

            //
            // Method invocation (or multiple invocations for batch messages)
            // must be done outside the thread synchronization, so that nested
            // calls are possible.
            //
            invokeAll(stream, invokeNum, requestId, compress, servantManager, adapter);
        }

        public override void finished(IceInternal.ThreadPool threadPool)
        {
            Debug.Assert(!_threadPerConnection); // Only for use with a thread pool.

            threadPool.promoteFollower();

            LocalException localEx = null;

            lock(this)
            {
                --_finishedCount;
                if(_finishedCount > 0 || _state != StateClosed || _sendInProgress)
                {
                    return;
                }

                try
                {
                    _transceiver.close();
                }
                catch(LocalException ex)
                {
                    localEx = ex;
                }

                _transceiver = null;
                Monitor.PulseAll(this);
            }

            finishStart(_exception);

            foreach(OutgoingMessage message in _queuedStreams)
            {
                message.finished(_exception);
            }
            _queuedStreams.Clear();

            foreach(IceInternal.Outgoing o in _requests.Values) // _requests is immutable at this point.
            {
                o.finished(_exception); // The exception is immutable at this point.
            }
            _requests.Clear();

            foreach(IceInternal.OutgoingAsync o in _asyncRequests.Values) // _asyncRequests is immutable at this point.
            {
                o.finished__(_exception); // The exception is immutable at this point.
            }
            _asyncRequests.Clear();

            if(localEx != null)
            {
                throw localEx;
            }
        }

        public override void exception(LocalException ex)
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
        // Operations from SocketReadyCallback
        //
        public IceInternal.SocketStatus socketReady(bool finished)
        {
            if(!finished)
            {
                try
                {
                    //
                    // First, we check if there's something to send. If that's the case, the connection
                    // must be active and the only thing to do is send the queued streams.
                    //
                    if(_sendStreams.Count > 0)
                    {
                        if(!send(0))
                        {
                            return IceInternal.SocketStatus.NeedWrite;
                        }
                        Debug.Assert(_sendStreams.Count == 0);
                    }
                    else
                    {
                        //
                        // If there's nothing to send, we're still validating the connection.
                        //
                        int state;
                        lock(this)
                        {
                            Debug.Assert(_state == StateClosed || _state <= StateNotValidated);

                            if(_state == StateClosed)
                            {
                                Debug.Assert(_exception != null);
                                throw _exception;
                            }

                            state = _state;
                        }

                        if(state == StateNotInitialized)
                        {
                            IceInternal.SocketStatus status = initialize();
                            if(status != IceInternal.SocketStatus.Finished)
                            {
                                return status;
                            }
                        }

                        if(state <= StateNotValidated)
                        {
                            IceInternal.SocketStatus status = validate();
                            if(status != IceInternal.SocketStatus.Finished)
                            {
                                return status;
                            }
                        }

                        finishStart(null);
                    }
                }
                catch(Ice.LocalException ex)
                {
                    lock(this)
                    {
                        setState(StateClosed, ex);
                    }
                }
            }

            //
            // If there's no more data to send or if connection validation is finished, we checkout
            // the connection state to figure out whether or not it's time to unregister with the
            // selector thread.
            //

            lock(this)
            {
                Debug.Assert(_sendInProgress);
                if(_state == StateClosed)
                {
                    Debug.Assert(_startCallback == null || (!_threadPerConnection && !_registeredWithPool));
                    
                    LinkedListNode<OutgoingMessage> node = _sendStreams.Last;
                    while(node != null)
                    {
                        _queuedStreams.AddFirst(node.Value);
                        node = node.Previous;
                    }
                    _sendInProgress = false;

                    if(_threadPerConnection)
                    {
                        _transceiver.shutdownReadWrite();
                    }
                    else
                    {
                        registerWithPool();
                        unregisterWithPool(); // Let finished() do the close.
                    }

                    Monitor.PulseAll(this);
                    return IceInternal.SocketStatus.Finished;
                }
                else if(_waitingForSend > 0)
                {
                    _sendInProgress = false;
                    Monitor.PulseAll(this);
                    return IceInternal.SocketStatus.Finished;
                }
                else if(_queuedStreams.Count == 0)
                {
                    _sendInProgress = false;
                    if(_acmTimeout > 0)
                    {
                        _acmAbsoluteTimeoutMillis = IceInternal.Time.currentMonotonicTimeMillis() + _acmTimeout * 1000;
                    }
                    return IceInternal.SocketStatus.Finished;
                }
                else
                {
                    LinkedList<OutgoingMessage> streams = _queuedStreams;
                    _queuedStreams = _sendStreams;
                    _sendStreams = streams;
                    return IceInternal.SocketStatus.NeedWrite; // We're not finished yet, there's more data to send!
                }
            }
        }

        public void socketTimeout()
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

        //
        // Only used by the SSL plug-in.
        //
        // The external party has to synchronize the connection, since the
        // connection is the object that protects the transceiver.
        //
        public IceInternal.Transceiver
        getTransceiver()
        {
            return _transceiver;
        }

        static ConnectionI()
        {
            _compressionSupported = IceInternal.BasicStream.compressible();
        }

        internal ConnectionI(IceInternal.Instance instance, IceInternal.Transceiver transceiver,
                             IceInternal.EndpointI endpoint, ObjectAdapter adapter, bool threadPerConnection)
            : base(instance)
        {
            InitializationData initData = instance.initializationData();
            _threadPerConnection = threadPerConnection;
            _transceiver = transceiver;
            _desc = transceiver.ToString();
            _type = transceiver.type();
            _endpoint = endpoint;
            _adapter = adapter;
            _logger = initData.logger; // Cached for better performance.
            _traceLevels = instance.traceLevels(); // Cached for better performance.
            _registeredWithPool = false;
            _finishedCount = 0;
            _warn = initData.properties.getPropertyAsInt("Ice.Warn.Connections") > 0;
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
            _waitingForSend = 0;
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
                    _acmTimeout = instance_.serverACM();
                }
                else
                {
                    _acmTimeout = instance_.clientACM();
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

            try
            {
                if(!threadPerConnection)
                {
                    //
                    // Only set _threadPool if we really need it, i.e., if we are
                    // not in thread per connection mode. Thread pools have lazy
                    // initialization in Instance, and we don't want them to be
                    // created if they are not needed.
                    //
                    if(adapterImpl != null)
                    {
                        _threadPool = adapterImpl.getThreadPool();
                    }
                    else
                    {
                        _threadPool = instance.clientThreadPool();
                    }
                }

                _selectorThread = instance_.selectorThread();

                _overrideCompress = instance_.defaultsAndOverrides().overrideCompress;
                _overrideCompressValue = instance_.defaultsAndOverrides().overrideCompressValue;
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

        private bool setState(int state, LocalException ex)
        {
            //
            // If setState() is called with an exception, then only closed
            // and closing states are permissible.
            //
            Debug.Assert(state == StateClosing || state == StateClosed);

            if(_state == state) // Don't switch twice.
            {
                return false;
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
            return setState(state);
        }

        private bool setState(int state)
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
                return false;
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
                        return false;
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
                        return false;
                    }
                    if(!_threadPerConnection)
                    {
                        registerWithPool();
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
                        return false;
                    }
                    if(!_threadPerConnection)
                    {
                        unregisterWithPool();
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
                        return false;
                    }
                    if(!_threadPerConnection)
                    {
                        registerWithPool(); // We need to continue to read in closing state.
                    }
                    break;
                }

                case StateClosed:
                {
                    if(_sendInProgress)
                    {
                        //
                        // Unregister with both the pool and the selector thread. We unregister with
                        // the pool to ensure that it stops reading on the socket (otherwise, if the
                        // socket is closed the thread pool would spin always reading 0 from the FD).
                        // The selector thread will register again the FD with the pool once it's
                        // done.
                        //
                        _selectorThread.unregister(_transceiver.fd());
                        if(!_threadPerConnection)
                        {
                            unregisterWithPool();
                        }

                        _transceiver.shutdownWrite(); // Prevent further writes.
                    }
                    else if(_state <= StateNotValidated || _threadPerConnection)
                    {
                        //
                        // If we are in thread per connection mode and the thread is started, we
                        // shutdown both for reading and writing. This will unblock the read call
                        // with an exception. The thread per connection closes the transceiver.
                        //
                        _transceiver.shutdownReadWrite();
                    }
                    else
                    {
                        registerWithPool();
                        unregisterWithPool(); // Let finished() do the close.

                        _transceiver.shutdownWrite(); // Prevent further writes.
                    }
                    break;
                }
            }

            //
            // We only register with the connection monitor if our new state
            // is StateActive. Otherwise we unregister with the connection
            // monitor, but only if we were registered before, i.e., if our
            // old state was StateActive.
            //
            IceInternal.ConnectionMonitor connectionMonitor = instance_.connectionMonitor();
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
                    return initiateShutdown(false);
                }
                catch(LocalException ex)
                {
                    setState(StateClosed, ex);
                }
            }

            return false;
        }

        private bool initiateShutdown(bool queue)
        {
            Debug.Assert(_state == StateClosing);
            Debug.Assert(_dispatchCount == 0);

            if(!_endpoint.datagram())
            {
                //
                // Before we shut down, we send a close connection
                // message.
                //
                IceInternal.BasicStream os = new IceInternal.BasicStream(instance_);
                os.writeBlob(IceInternal.Protocol.magic);
                os.writeByte(IceInternal.Protocol.protocolMajor);
                os.writeByte(IceInternal.Protocol.protocolMinor);
                os.writeByte(IceInternal.Protocol.encodingMajor);
                os.writeByte(IceInternal.Protocol.encodingMinor);
                os.writeByte(IceInternal.Protocol.closeConnectionMsg);
                os.writeByte(_compressionSupported ? (byte)1 : (byte)0);
                os.writeInt(IceInternal.Protocol.headerSize); // Message size.

                return sendMessage(new OutgoingMessage(os, false, false), queue);

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

            return false;
        }

        private IceInternal.SocketStatus initialize()
        {
            int timeout = 0;
            if(_startCallback == null || _threadPerConnection)
            {
                IceInternal.DefaultsAndOverrides defaultsAndOverrides = instance_.defaultsAndOverrides();
                if(defaultsAndOverrides.overrideConnectTimeout)
                {
                    timeout = defaultsAndOverrides.overrideConnectTimeoutValue;
                }
                else
                {
                    timeout = _endpoint.timeout();
                }
            }

            try
            {
                IceInternal.SocketStatus status = _transceiver.initialize(timeout);
                if(status != IceInternal.SocketStatus.Finished)
                {
                    if(_startCallback == null || _threadPerConnection)
                    {
                        throw new Ice.TimeoutException();
                    }
                    return status;
                }
            }
            catch(Ice.TimeoutException)
            {
                throw new Ice.ConnectTimeoutException();
            }

            lock(this)
            {
                if(_state == StateClosed)
                {
                    Debug.Assert(_exception != null);
                    throw _exception;
                }

                //
                // Update the connection description once the transceiver is initialized.
                //
                _desc = _transceiver.ToString();

                setState(StateNotValidated);
            }

            return IceInternal.SocketStatus.Finished;
        }

        private IceInternal.SocketStatus validate()
        {
            if(!_endpoint.datagram()) // Datagram connections are always implicitly validated.
            {
                int timeout = 0;
                if(_startCallback == null || _threadPerConnection)
                {
                    IceInternal.DefaultsAndOverrides defaultsAndOverrides = instance_.defaultsAndOverrides();
                    if(defaultsAndOverrides.overrideConnectTimeout)
                    {
                        timeout = defaultsAndOverrides.overrideConnectTimeoutValue;
                    }
                    else
                    {
                        timeout = _endpoint.timeout();
                    }
                }

                if(_adapter != null) // The server side has the active role for connection validation.
                {
                    IceInternal.BasicStream os = stream_;
                    if(os.size() == 0)
                    {
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
                    else
                    {
                        // The stream can only be non-empty if we're doing a non-blocking connection validation.
                        Debug.Assert(_startCallback != null && !_threadPerConnection);
                    }

                    try
                    {
                        if(!_transceiver.write(os.getBuffer(), timeout))
                        {
                            if(_startCallback == null || _threadPerConnection)
                            {
                                throw new Ice.TimeoutException();
                            }
                            return IceInternal.SocketStatus.NeedWrite;
                        }
                    }
                    catch(Ice.TimeoutException)
                    {
                        throw new Ice.ConnectTimeoutException();
                    }
                }
                else // The client side has the passive role for connection validation.
                {
                    IceInternal.BasicStream istr = stream_;
                    if(istr.size() == 0)
                    {
                        istr.resize(IceInternal.Protocol.headerSize, true);
                        istr.pos(0);
                    }
                    else
                    {
                        // The stream can only be non-empty if we're doing a non-blocking connection validation.
                        Debug.Assert(_startCallback != null && !_threadPerConnection);
                    }

                    try
                    {
                        if(!_transceiver.read(istr.getBuffer(), timeout))
                        {
                            if(_startCallback == null || _threadPerConnection)
                            {
                                throw new Ice.TimeoutException();
                            }
                            return IceInternal.SocketStatus.NeedRead;
                        }
                    }
                    catch(Ice.TimeoutException)
                    {
                        throw new Ice.ConnectTimeoutException();
                    }

                    Debug.Assert(istr.pos() == IceInternal.Protocol.headerSize);
                    istr.pos(0);
                    byte[] m = istr.readBlob(4);
                    if(m[0] != IceInternal.Protocol.magic[0] || m[1] != IceInternal.Protocol.magic[1] ||
                       m[2] != IceInternal.Protocol.magic[2] || m[3] != IceInternal.Protocol.magic[3])
                    {
                        BadMagicException ex = new BadMagicException();
                        ex.badMagic = m;
                        throw ex;
                    }
                    byte pMajor = istr.readByte();
                    byte pMinor = istr.readByte();
                    if(pMajor != IceInternal.Protocol.protocolMajor)
                    {
                        UnsupportedProtocolException e = new UnsupportedProtocolException();
                        e.badMajor = pMajor < 0 ? pMajor + 255 : pMajor;
                        e.badMinor = pMinor < 0 ? pMinor + 255 : pMinor;
                        e.major = IceInternal.Protocol.protocolMajor;
                        e.minor = IceInternal.Protocol.protocolMinor;
                        throw e;
                    }
                    byte eMajor = istr.readByte();
                    byte eMinor = istr.readByte();
                    if(eMajor != IceInternal.Protocol.encodingMajor)
                    {
                        UnsupportedEncodingException e = new UnsupportedEncodingException();
                        e.badMajor = eMajor < 0 ? eMajor + 255 : eMajor;
                        e.badMinor = eMinor < 0 ? eMinor + 255 : eMinor;
                        e.major = IceInternal.Protocol.encodingMajor;
                        e.minor = IceInternal.Protocol.encodingMinor;
                        throw e;
                    }
                    byte messageType = istr.readByte();
                    if(messageType != IceInternal.Protocol.validateConnectionMsg)
                    {
                        throw new ConnectionNotValidatedException();
                    }
                    istr.readByte(); // Ignore compression status for validate connection.
                    int size = istr.readInt();
                    if(size != IceInternal.Protocol.headerSize)
                    {
                        throw new IllegalMessageSizeException();
                    }
                    IceInternal.TraceUtil.traceRecv(istr, _logger, _traceLevels);
                }
            }

            lock(this)
            {
                stream_.reset();

                if(_state == StateClosed)
                {
                    Debug.Assert(_exception != null);
                    throw _exception;
                }

                //
                // We start out in holding state.
                //
                setState(StateHolding);
            }

            return IceInternal.SocketStatus.Finished;
        }

        private bool send(int timeout)
        {
            Debug.Assert(_transceiver != null);
            Debug.Assert(_sendStreams.Count > 0);

            while(_sendStreams.Count > 0)
            {
                OutgoingMessage message = _sendStreams.First.Value;
                if(!message.prepared)
                {
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

                if(!_transceiver.write(message.stream.getBuffer(), timeout))
                {
                    Debug.Assert(timeout == 0);
                    return false;
                }

                message.sent(this, timeout == 0); // timeout == 0 indicates that this is called by the selector thread.
                _sendStreams.RemoveFirst();
            }

            return true;
        }

        private bool sendMessage(OutgoingMessage message, bool queue)
        {
            Debug.Assert(_state != StateClosed);

            //
            // TODO: Remove support for foreground send? If set to true, messages are sent
            // by the calling thread. Foreground send might still be useful for transports
            // that don't support non-blocking send.
            //
            bool foreground = false;

            //
            // If another thread is currently sending messages, we queue the message in _queuedStreams
            // if we're not required to send the message in the foreground. If we're required to send
            // the request in the foreground we wait until no more threads send messages.
            //
            if(_sendInProgress)
            {
                if(!foreground)
                {
                    message.adopt();
                    _queuedStreams.AddLast(message);
                    return false;
                }
                else if(queue)
                {
                    //
                    // Add the message to _sendStreams if requested, this is useful for sendResponse() to
                    // send the close connection message after sending the response.
                    //
                    _sendStreams.AddLast(message);
                    return true; // The calling thread must send the messages by calling finishSendMessage()
                }
                else
                {
                    ++_waitingForSend;
                    while(_sendInProgress)
                    {
                        Monitor.Wait(this);
                    }
                    --_waitingForSend;

                    if(_state == StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }
                }
            }

            Debug.Assert(!_sendInProgress);

            //
            // Attempt to send the message without blocking. If the send blocks, we register
            // the connection with the selector thread or we request the caller to call
            // finishSendMessage() outside the synchronization.
            //

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

            if(!foreground && _transceiver.write(message.stream.getBuffer(), 0))
            {
                message.sent(this, false);
                if(_acmTimeout > 0)
                {
                    _acmAbsoluteTimeoutMillis = IceInternal.Time.currentMonotonicTimeMillis() + _acmTimeout * 1000;
                }
                return false;
            }

            _sendStreams.AddLast(message);
            _sendInProgress = true;
            if(!foreground)
            {
                message.adopt();
                _selectorThread.register(_transceiver.fd(), this, IceInternal.SocketStatus.NeedWrite,
                                         _endpoint.timeout());
                return false; // The selector thread will send the message.
            }
            else
            {
                return true; // The calling thread must send the message by calling finishSendMessage()
            }
        }

        private void finishSendMessage()
        {
            try
            {
                //
                // Send the queued messages with a blocking write().
                //
                bool finished = send(_endpoint.timeout());
                Debug.Assert(finished);
            }
            catch(Ice.LocalException ex)
            {
                lock(this)
                {
                    setState(StateClosed, ex);

                    _sendStreams.Clear();
                    _sendInProgress = false;

                    if(_threadPerConnection)
                    {
                        _transceiver.shutdownReadWrite();
                    }
                    else
                    {
                        registerWithPool();
                        unregisterWithPool(); // Let finished() do the close.
                    }

                    Monitor.PulseAll(this);

                    Debug.Assert(_exception != null);
                    throw _exception;
                }
            }

            lock(this)
            {
                Debug.Assert(_sendStreams.Count == 0);
                if(_state == StateClosed)
                {
                    _sendInProgress = false;
                    if(_threadPerConnection)
                    {
                        _transceiver.shutdownReadWrite();
                    }
                    else
                    {
                        registerWithPool();
                        unregisterWithPool(); // Let finished() do the close.
                    }

                    Monitor.PulseAll(this);
                }
                if(_waitingForSend > 0)
                {
                    _sendInProgress = false;
                    Monitor.PulseAll(this);
                }
                else if(_queuedStreams.Count == 0)
                {
                    if(_acmTimeout > 0)
                    {
                        _acmAbsoluteTimeoutMillis = IceInternal.Time.currentMonotonicTimeMillis() + _acmTimeout * 1000;
                    }
                    _sendInProgress = false;
                }
                else
                {
                    _selectorThread.register(_transceiver.fd(), this, IceInternal.SocketStatus.NeedWrite,
                                             _endpoint.timeout());
                }
            }
        }

        private void finishStart(LocalException ex)
        {
            //
            // We set _startCallback to null to break potential cyclic reference count
            // and because the finalizer checks for  it to ensure that we always invoke
            // on the callback.
            //

            StartCallback callback = null;
            lock(this)
            {
                callback = _startCallback;
                _startCallback = null;
            }

            if(callback != null)
            {
                if(ex == null)
                {
                    callback.connectionStartCompleted(this);
                }
                else
                {
                    callback.connectionStartFailed(this, ex);
                }
            }
        }

        private void registerWithPool()
        {
            Debug.Assert(!_threadPerConnection); // Only for use with a thread pool.

            if(!_registeredWithPool)
            {
                _threadPool.register(_transceiver.fd(), this);
                _registeredWithPool = true;
            }
        }

        private void unregisterWithPool()
        {
            Debug.Assert(!_threadPerConnection); // Only for use with a thread pool.

            if(_registeredWithPool)
            {
                _threadPool.unregister(_transceiver.fd());
                _registeredWithPool = false;
                ++_finishedCount; // For each unregistration, finished() is called once.
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

        private void parseMessage(ref IceInternal.BasicStream stream, ref int invokeNum, ref int requestId,
                                  ref byte compress, ref IceInternal.ServantManager servantManager,
                                  ref ObjectAdapter adapter, ref IceInternal.OutgoingAsync outAsync)
        {
            Debug.Assert(_state > StateNotValidated && _state < StateClosed);

            if(_acmTimeout > 0)
            {
                _acmAbsoluteTimeoutMillis = IceInternal.Time.currentMonotonicTimeMillis() + _acmTimeout * 1000;
            }

            try
            {
                //
                // We don't need to check magic and version here. This has
                // already been done by the ThreadPool or ThreadPerConnection,
                // which provides us with the stream.
                //
                Debug.Assert(stream.pos() == stream.size());
                stream.pos(8);
                byte messageType = stream.readByte();
                compress = stream.readByte();
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
                            requestId = stream.readInt();
                            invokeNum = 1;
                            servantManager = _servantManager;
                            adapter = _adapter;
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
                            invokeNum = stream.readInt();
                            if(invokeNum < 0)
                            {
                                invokeNum = 0;
                                throw new NegativeSizeException();
                            }
                            servantManager = _servantManager;
                            adapter = _adapter;
                            _dispatchCount += invokeNum;
                        }
                        break;
                    }

                    case IceInternal.Protocol.replyMsg:
                    {
                        IceInternal.TraceUtil.traceRecv(stream, _logger, _traceLevels);
                        requestId = stream.readInt();
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
                            outAsync = _asyncRequests[requestId];
                            _asyncRequests.Remove(requestId);
                        }
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

        private void invokeAll(IceInternal.BasicStream stream, int invokeNum, int requestId, byte compress,
                               IceInternal.ServantManager servantManager, ObjectAdapter adapter)
        {
            //
            // Note: In contrast to other private or protected methods, this
            // operation must be called *without* the mutex locked.
            //

            IceInternal.Incoming inc = null;
            try
            {
                while(invokeNum > 0)
                {
                    //
                    // Prepare the invocation.
                    //
                    bool response = !_endpoint.datagram() && requestId != 0;
                    inc = getIncoming(adapter, response, compress, requestId);
                    IceInternal.BasicStream ins = inc.istr();
                    stream.swap(ins);
                    IceInternal.BasicStream os = inc.ostr();

                    //
                    // Prepare the response if necessary.
                    //
                    if(response)
                    {
                        Debug.Assert(invokeNum == 1); // No further invocations if a response is expected.
                        os.writeBlob(IceInternal.Protocol.replyHdr);

                        //
                        // Add the request ID.
                        //
                        os.writeInt(requestId);
                    }

                    inc.invoke(servantManager);

                    //
                    // If there are more invocations, we need the stream back.
                    //
                    if(--invokeNum > 0)
                    {
                        stream.swap(ins);
                    }

                    reclaimIncoming(inc);
                    inc = null;
                }
            }
            catch(LocalException ex)
            {
                invokeException(ex, invokeNum);
            }
            finally
            {
                if(inc != null)
                {
                    reclaimIncoming(inc);
                }
            }
        }

        private void run()
        {
            try
            {
                //
                // Initialize the connection transceiver and validate the connection using
                // blocking operations.
                //

                IceInternal.SocketStatus status;

                status = initialize();
                Debug.Assert(status == IceInternal.SocketStatus.Finished);

                status = validate();
                Debug.Assert(status == IceInternal.SocketStatus.Finished);
            }
            catch(LocalException ex)
            {
                lock(this)
                {
                    setState(StateClosed, ex);

                    if(_transceiver != null)
                    {
                        try
                        {
                            _transceiver.close();
                        }
                        catch(LocalException)
                        {
                            // Here we ignore any exceptions in close().
                        }

                        _transceiver = null;
                    }
                    Monitor.PulseAll(this);
                }

                finishStart(_exception);
                return;
            }

            finishStart(null);

            bool warnUdp = instance_.initializationData().properties.getPropertyAsInt("Ice.Warn.Datagrams") > 0;

            bool closed = false;

            IceInternal.BasicStream stream = new IceInternal.BasicStream(instance_);
            while(!closed)
            {
                //
                // We must read new messages outside the thread synchronization because we use blocking reads.
                //

                try
                {
                    try
                    {
                        stream.resize(IceInternal.Protocol.headerSize, true);
                        stream.pos(0);
                        _transceiver.read(stream.getBuffer(), -1);

                        int pos = stream.pos();
                        if(pos < IceInternal.Protocol.headerSize)
                        {
                            //
                            // This situation is possible for small UDP packets.
                            //
                            throw new IllegalMessageSizeException();
                        }
                        stream.pos(0);
                        byte[] m = stream.readBlob(4);
                        if(m[0] != IceInternal.Protocol.magic[0] || m[1] != IceInternal.Protocol.magic[1] ||
                           m[2] != IceInternal.Protocol.magic[2] || m[3] != IceInternal.Protocol.magic[3])
                        {
                            BadMagicException ex = new BadMagicException();
                            ex.badMagic = m;
                            throw ex;
                        }
                        byte pMajor = stream.readByte();
                        byte pMinor = stream.readByte();
                        if(pMajor != IceInternal.Protocol.protocolMajor)
                        {
                            UnsupportedProtocolException e = new UnsupportedProtocolException();
                            e.badMajor = pMajor < 0 ? pMajor + 255 : pMajor;
                            e.badMinor = pMinor < 0 ? pMinor + 255 : pMinor;
                            e.major = IceInternal.Protocol.protocolMajor;
                            e.minor = IceInternal.Protocol.protocolMinor;
                            throw e;
                        }
                        byte eMajor = stream.readByte();
                        byte eMinor = stream.readByte();
                        if(eMajor != IceInternal.Protocol.encodingMajor)
                        {
                            UnsupportedEncodingException e = new UnsupportedEncodingException();
                            e.badMajor = eMajor < 0 ? eMajor + 255 : eMajor;
                            e.badMinor = eMinor < 0 ? eMinor + 255 : eMinor;
                            e.major = IceInternal.Protocol.encodingMajor;
                            e.minor = IceInternal.Protocol.encodingMinor;
                            throw e;
                        }
                        stream.readByte(); // Skip message type.
                        stream.readByte(); // Skip compression flag.
                        int size = stream.readInt();
                        if(size < IceInternal.Protocol.headerSize)
                        {
                            throw new IllegalMessageSizeException();
                        }
                        if(size > instance_.messageSizeMax())
                        {
                            throw new MemoryLimitException();
                        }
                        if(size > stream.size())
                        {
                            stream.resize(size, true);
                        }
                        stream.pos(pos);

                        if(pos != stream.size())
                        {
                            if(_endpoint.datagram())
                            {
                                if(warnUdp)
                                {
                                    _logger.warning("DatagramLimitException: maximum size of " + pos + " exceeded");
                                }
                                throw new DatagramLimitException();
                            }
                            else
                            {
                                _transceiver.read(stream.getBuffer(), -1);
                                Debug.Assert(stream.pos() == stream.size());
                            }
                        }
                    }
                    catch(DatagramLimitException) // Expected.
                    {
                        continue;
                    }
                    catch(SocketException ex) // Expected.
                    {
                        exception(ex);
                    }
                    catch(LocalException ex)
                    {
                        if(_endpoint.datagram())
                        {
                            if(_warn)
                            {
                                warning("datagram connection exception:", ex);
                            }
                            continue;
                        }
                        else
                        {
                            exception(ex);
                        }
                    }

                    byte compress = 0;
                    int requestId = 0;
                    int invokeNum = 0;
                    IceInternal.ServantManager servantManager = null;
                    ObjectAdapter adapter = null;
                    IceInternal.OutgoingAsync outAsync = null;

                    LocalException localEx = null;

                    lock(this)
                    {
                        while(_state == StateHolding)
                        {
                            Monitor.Wait(this);
                        }

                        if(_state != StateClosed)
                        {
                            parseMessage(ref stream, ref invokeNum, ref requestId, ref compress, ref servantManager,
                                         ref adapter, ref outAsync);
                        }

                        //
                        // parseMessage() can close the connection, so we must check
                        // for closed state again.
                        //
                        if(_state == StateClosed)
                        {
                            if(_sendInProgress)
                            {
                                _selectorThread.unregister(_transceiver.fd());
                            }

                            //
                            // Prevent further writes.
                            //
                            _transceiver.shutdownWrite();

                            while(_sendInProgress)
                            {
                                Monitor.Wait(this);
                            }

                            try
                            {
                                _transceiver.close();
                            }
                            catch(LocalException ex)
                            {
                                localEx = ex;
                            }
                            _transceiver = null;
                            Monitor.PulseAll(this);

                            //
                            // We cannot simply return here. We have to make sure
                            // that all requests (regular and async) are notified
                            // about the closed connection below.
                            //
                            closed = true;
                        }
                    }

                    //
                    // Asynchronous replies must be handled outside the thread
                    // synchronization, so that nested calls are possible.
                    //
                    if(outAsync != null)
                    {
                        outAsync.finished__(stream);
                    }

                    //
                    // Method invocation (or multiple invocations for batch messages)
                    // must be done outside the thread synchronization, so that nested
                    // calls are possible.
                    //
                    invokeAll(stream, invokeNum, requestId, compress, servantManager, adapter);

                    if(closed)
                    {
                        foreach(OutgoingMessage message in _queuedStreams)
                        {
                            message.finished(_exception); // The exception is immutable at this point.
                        }
                        _queuedStreams.Clear();

                        foreach(IceInternal.Outgoing og in _requests.Values)
                        {
                            og.finished(_exception); // The exception is immutable at this point.
                        }
                        _requests.Clear();

                        foreach(IceInternal.OutgoingAsync og in _asyncRequests.Values)
                        {
                            og.finished__(_exception); // The exception is immutable at this point.
                        }
                        _asyncRequests.Clear();
                    }

                    if(localEx != null)
                    {
                        Debug.Assert(closed);
                        throw localEx;
                    }
                }
                finally
                {
                    stream.reset();
                }
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
                        inc = new IceInternal.Incoming(instance_, this, adapter, response, compress, requestId);
                    }
                    else
                    {
                        inc = _incomingCache;
                        _incomingCache = _incomingCache.next;
                        inc.reset(instance_, this, adapter, response, compress, requestId);
                        inc.next = null;
                    }
                }
            }
            else
            {
                inc = new IceInternal.Incoming(instance_, this, adapter, response, compress, requestId);
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

        public void RunThreadPerConnection()
        {
            if(instance_.initializationData().threadHook != null)
            {
                instance_.initializationData().threadHook.start();
            }

            try
            {
                run();
            }
            catch(Exception ex)
            {
                _logger.error("exception in thread per connection:\n" + ToString() + "\n" + ex.ToString());
            }
            catch(System.Exception ex)
            {
                _logger.error("system exception in thread per connection:\n" + ToString() + "\n" + ex.ToString());
            }
            finally
            {
                if(instance_.initializationData().threadHook != null)
                {
                    instance_.initializationData().threadHook.stop();
                }
            }
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

            internal void finished(Ice.LocalException ex)
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

        private Thread _thread;
        private bool _threadPerConnection;

        private IceInternal.Transceiver _transceiver;

        private string _desc;
        private string _type;
        private IceInternal.EndpointI _endpoint;

        private ObjectAdapter _adapter;
        private IceInternal.ServantManager _servantManager;

        private Logger _logger;
        private IceInternal.TraceLevels _traceLevels;

        private bool _registeredWithPool;
        private int _finishedCount;
        private IceInternal.ThreadPool _threadPool;
        private IceInternal.SelectorThread _selectorThread;

        private StartCallback _startCallback = null;

        private bool _warn;

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

        private LinkedList<OutgoingMessage> _queuedStreams = new LinkedList<OutgoingMessage>();
        private LinkedList<OutgoingMessage> _sendStreams = new LinkedList<OutgoingMessage>();
        private bool _sendInProgress;
        private int _waitingForSend;

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
