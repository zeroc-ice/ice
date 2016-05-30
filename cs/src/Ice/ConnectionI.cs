// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
    using Ice.Instrumentation;

    public sealed class ConnectionI : IceInternal.EventHandler, Connection
    {
        public interface StartCallback
        {
            void connectionStartCompleted(ConnectionI connection);
            void connectionStartFailed(ConnectionI connection, LocalException ex);
        }

        private class TimeoutCallback : IceInternal.TimerTask
        {
            public TimeoutCallback(ConnectionI connection)
            {
                _connection = connection;
            }

            public void runTimerTask()
            {
                _connection.timedOut();
            }

            private Ice.ConnectionI _connection;
        }

        public void start(StartCallback callback)
        {
            try
            {
                _m.Lock();
                try
                {
                    //
                    // The connection might already be closed if the communicator was destroyed.
                    //
                    if(_state >= StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }

                    if(!initialize(IceInternal.SocketOperation.None) || !validate(IceInternal.SocketOperation.None))
                    {
                        if(callback != null)
                        {
                            _startCallback = callback;
                            return;
                        }

                        //
                        // Wait for the connection to be validated.
                        //
                        while(_state <= StateNotValidated)
                        {
                            _m.Wait();
                        }

                        if(_state >= StateClosing)
                        {
                            Debug.Assert(_exception != null);
                            throw _exception;
                        }
                    }

                    //
                    // We start out in holding state.
                    //
                    setState(StateHolding);
                }
                finally
                {
                    _m.Unlock();
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
                    throw;
                }
            }

            if(callback != null)
            {
                callback.connectionStartCompleted(this);
            }
        }

        public void activate()
        {
            _m.Lock();
            try
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
            finally
            {
                _m.Unlock();
            }
        }

        public void hold()
        {
            _m.Lock();
            try
            {
                if(_state <= StateNotValidated)
                {
                    return;
                }

                setState(StateHolding);
            }
            finally
            {
                _m.Unlock();
            }
        }

        // DestructionReason.
        public const int ObjectAdapterDeactivated = 0;
        public const int CommunicatorDestroyed = 1;

        public void destroy(int reason)
        {
            _m.Lock();
            try
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
            finally
            {
                _m.Unlock();
            }
        }

        public void close(bool force)
        {
            _m.Lock();
            try
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
                        _m.Wait();
                    }

                    setState(StateClosing, new CloseConnectionException());
                }
            }
            finally
            {
                _m.Unlock();
            }
        }

        public bool isActiveOrHolding()
        {
            _m.Lock();
            try
            {
                return _state > StateNotValidated && _state < StateClosing;
            }
            finally
            {
                _m.Unlock();
            }
        }

        public bool isFinished()
        {
            //
            // We can use TryEnter here, because as long as there are still
            // threads operating in this connection object, connection
            // destruction is considered as not yet finished.
            //
            if(!_m.TryLock())
            {
                return false;
            }

            try
            {
                if(_state != StateFinished || _dispatchCount != 0)
                {
                    return false;
                }

                Debug.Assert(_state == StateFinished);
            }
            finally
            {
                _m.Unlock();
            }
            return true;
        }

        public void throwException()
        {
            _m.Lock();
            try
            {
                if(_exception != null)
                {
                    Debug.Assert(_state >= StateClosing);
                    throw _exception;
                }
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void waitUntilHolding()
        {
            _m.Lock();
            try
            {
                while(_state < StateHolding || _dispatchCount > 0)
                {
                    _m.Wait();
                }
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void waitUntilFinished()
        {
            _m.Lock();
            try
            {
                //
                // We wait indefinitely until the connection is finished and all
                // outstanding requests are completed. Otherwise we couldn't
                // guarantee that there are no outstanding calls when deactivate()
                // is called on the servant locators.
                //
                while(_state < StateFinished || _dispatchCount > 0)
                {
                    _m.Wait();
                }

                Debug.Assert(_state == StateFinished && _dispatchCount == 0);

                //
                // Clear the OA. See bug 1673 for the details of why this is necessary.
                //
                _adapter = null;
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void updateObserver()
        {
            _m.Lock();
            try
            {
                if(_state < StateNotValidated || _state > StateClosed)
                {
                    return;
                }
                
                Debug.Assert(_instance.getObserver() != null);
                _observer = _instance.getObserver().getConnectionObserver(initConnectionInfo(),
                                                                          _endpoint,
                                                                          toConnectionState(_state),
                                                                          _observer);
                if(_observer != null)
                {
                    _observer.attach();
                }
                else
                {
                    _writeStreamPos = -1;
                    _readStreamPos = -1;
                }
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void monitor(long now)
        {
            if(!_m.TryLock())
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
                if(_acmTimeout <= 0 ||
                   _requests.Count > 0 || _asyncRequests.Count > 0 || _dispatchCount > 0 ||
                   _readStream.size() > IceInternal.Protocol.headerSize ||
                   !_writeStream.isEmpty() ||
                   !_batchStream.isEmpty())
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
                _m.Unlock();
            }
        }

        public bool sendRequest(IceInternal.Outgoing og, bool compress, bool response)
        {
            IceInternal.BasicStream os = og.ostr();

            _m.Lock();
            try
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

                //
                // Ensure the message isn't bigger than what we can send with the
                // transport.
                //
                _transceiver.checkSendSize(os.getBuffer(), _instance.messageSizeMax());

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

                og.attachRemoteObserver(initConnectionInfo(), _endpoint, requestId, 
                                        os.size() - IceInternal.Protocol.headerSize - 4);

                //
                // Send the message. If it can't be sent without blocking the message is added
                // to _sendStreams and it will be sent by the asynchronous I/O callback.
                //
                bool sent = false;
                try
                {
                    sent = sendMessage(new OutgoingMessage(og, os, compress, requestId));
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
            finally
            {
                _m.Unlock();
            }
        }

        public bool sendAsyncRequest(IceInternal.OutgoingAsync og, bool compress, bool response,
                                     out Ice.AsyncCallback sentCallback)
        {
            IceInternal.BasicStream os = og.ostr__;

            _m.Lock();
            try
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

                //
                // Ensure the message isn't bigger than what we can send with the
                // transport.
                //
                _transceiver.checkSendSize(os.getBuffer(), _instance.messageSizeMax());

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

                og.attachRemoteObserver__(initConnectionInfo(), _endpoint, requestId, 
                                          os.size() - IceInternal.Protocol.headerSize - 4);

                bool sent;
                try
                {
                    OutgoingMessage msg = new OutgoingMessage(og, og.ostr__, compress, requestId);
                    sent = sendMessage(msg);
                    sentCallback = msg.sentCallback;
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
            finally
            {
                _m.Unlock();
            }
        }

        public void prepareBatchRequest(IceInternal.BasicStream os)
        {
            _m.Lock();
            try
            {
                //
                // Wait if flushing is currently in progress.
                //
                while(_batchStreamInUse && _exception == null)
                {
                    _m.Wait();
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
            finally
            {
                _m.Unlock();
            }
        }

        public void finishBatchRequest(IceInternal.BasicStream os, bool compress)
        {
            try
            {
                _m.Lock();
                try
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
                        _batchStream = new IceInternal.BasicStream(_instance, Util.currentProtocolEncoding,
                                                                   _batchAutoFlush);
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
                    _m.NotifyAll();
                }
                finally
                {
                    _m.Unlock();
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
            _m.Lock();
            try
            {
                _batchStream = new IceInternal.BasicStream(_instance, Util.currentProtocolEncoding, _batchAutoFlush);
                _batchRequestNum = 0;
                _batchRequestCompress = false;
                _batchMarker = 0;

                Debug.Assert(_batchStreamInUse);
                _batchStreamInUse = false;
                _m.NotifyAll();
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void flushBatchRequests()
        {
            InvocationObserver observer = IceInternal.ObserverHelper.get(_instance, __flushBatchRequests_name);
            IceInternal.BatchOutgoing @out = new IceInternal.BatchOutgoing(this, _instance, observer);
            @out.invoke();
        }

        public AsyncResult begin_flushBatchRequests()
        {
            return begin_flushBatchRequestsInternal(null, null);
        }

        public AsyncResult begin_flushBatchRequests(AsyncCallback cb, object cookie)
        {
            return begin_flushBatchRequestsInternal(cb, cookie);
        }

        public void end_flushBatchRequests(AsyncResult r)
        {
            IceInternal.OutgoingAsyncBase outAsync = (IceInternal.OutgoingAsyncBase)r;
            IceInternal.OutgoingAsyncBase.check__(outAsync, this, __flushBatchRequests_name);
            outAsync.wait__();
        }

        private const string __flushBatchRequests_name = "flushBatchRequests";

        private AsyncResult begin_flushBatchRequestsInternal(AsyncCallback cb, object cookie)
        {
            IceInternal.ConnectionBatchOutgoingAsync result =
                new IceInternal.ConnectionBatchOutgoingAsync(this, _communicator, _instance, __flushBatchRequests_name,
                                                             cookie);

            if(cb != null)
            {
                result.whenCompletedWithAsyncCallback(cb);
            }

            try
            {
                result.send__();
            }
            catch(LocalException ex)
            {
                result.exceptionAsync__(ex);
            }

            return result;
        }

        public bool flushBatchRequests(IceInternal.BatchOutgoing @out)
        {
            _m.Lock();
            try
            {
                while(_batchStreamInUse && _exception == null)
                {
                    _m.Wait();
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

                @out.attachRemoteObserver(initConnectionInfo(), _endpoint, 
                                          _batchStream.size() - IceInternal.Protocol.headerSize);

                _batchStream.swap(@out.ostr());

                bool sent = false;
                try
                {
                    OutgoingMessage message = new OutgoingMessage(@out, @out.ostr(), _batchRequestCompress, 0);
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
                _batchStream = new IceInternal.BasicStream(_instance, Util.currentProtocolEncoding, _batchAutoFlush);
                _batchRequestNum = 0;
                _batchRequestCompress = false;
                _batchMarker = 0;

                return sent;
            }
            finally
            {
                _m.Unlock();
            }
        }

        public bool flushAsyncBatchRequests(IceInternal.BatchOutgoingAsync outAsync, out Ice.AsyncCallback sentCallback)
        {
            _m.Lock();
            try
            {
                while(_batchStreamInUse && _exception == null)
                {
                    _m.Wait();
                }

                if(_exception != null)
                {
                    throw _exception;
                }
                
                if(_batchRequestNum == 0)
                {
                    sentCallback = outAsync.sent__(this);
                    return true;
                }

                //
                // Fill in the number of requests in the batch.
                //
                _batchStream.pos(IceInternal.Protocol.headerSize);
                _batchStream.writeInt(_batchRequestNum);

                outAsync.attachRemoteObserver__(initConnectionInfo(), _endpoint, 0, 
                                                _batchStream.size() - IceInternal.Protocol.headerSize - 4);

                _batchStream.swap(outAsync.ostr__);

                //
                // Send the batch stream.
                //
                bool sent;
                try
                {
                    OutgoingMessage message = new OutgoingMessage(outAsync, outAsync.ostr__, _batchRequestCompress, 0);
                    sent = sendMessage(message);
                    sentCallback = message.sentCallback;
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
                _batchStream = new IceInternal.BasicStream(_instance, Util.currentProtocolEncoding, _batchAutoFlush);
                _batchRequestNum = 0;
                _batchRequestCompress = false;
                _batchMarker = 0;
                return sent;
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void sendResponse(IceInternal.BasicStream os, byte compressFlag)
        {
            _m.Lock();
            try
            {
                Debug.Assert(_state > StateNotValidated);

                try
                {
                    if(--_dispatchCount == 0)
                    {
                        if(_state == StateFinished)
                        {
                            _reaper.add(this, _observer);
                        }
                        _m.NotifyAll();
                    }

                    if(_state >= StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }

                    sendMessage(new OutgoingMessage(os, compressFlag != 0, true));

                    if(_state == StateClosing && _dispatchCount == 0)
                    {
                        initiateShutdown();
                    }
                }
                catch(LocalException ex)
                {
                    setState(StateClosed, ex);
                }
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void sendNoResponse()
        {
            _m.Lock();
            try
            {
                Debug.Assert(_state > StateNotValidated);

                try
                {
                    if(--_dispatchCount == 0)
                    {
                        if(_state == StateFinished)
                        {
                            _reaper.add(this, _observer);
                        }
                        _m.NotifyAll();
                    }

                    if(_state >= StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }

                    if(_state == StateClosing && _dispatchCount == 0)
                    {
                        initiateShutdown();
                    }
                }
                catch(LocalException ex)
                {
                    setState(StateClosed, ex);
                }
            }
            finally
            {
                _m.Unlock();
            }
        }

        public IceInternal.EndpointI endpoint()
        {
            return _endpoint; // No mutex protection necessary, _endpoint is immutable.
        }

        public IceInternal.Connector connector()
        {
            return _connector; // No mutex protection necessary, _endpoint is immutable.
        }

        public void setAdapter(ObjectAdapter adapter)
        {
            _m.Lock();
            try
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
            finally
            {
                _m.Unlock();
            }
        }

        public ObjectAdapter getAdapter()
        {
            _m.Lock();
            try
            {
                return _adapter;
            }
            finally
            {
                _m.Unlock();
            }
        }

        public Endpoint getEndpoint()
        {
            return _endpoint; // No mutex protection necessary, _endpoint is immutable.
        }

        public ObjectPrx createProxy(Identity ident)
        {
            //
            // Create a reference and return a reverse proxy for this
            // reference.
            //
            return _instance.proxyFactory().referenceToProxy(_instance.referenceFactory().create(ident, this));
        }

        //
        // Operations from EventHandler
        //
        public override bool startAsync(int operation, IceInternal.AsyncCallback cb, ref bool completedSynchronously)
        {
            if(_state >= StateClosed)
            {
                return false;
            }

            try
            {
                if((operation & IceInternal.SocketOperation.Write) != 0)
                {
                    if(_observer != null)
                    {
                        observerStartWrite(_writeStream.pos());
                    }

                    bool completed;
                    completedSynchronously = _transceiver.startWrite(_writeStream.getBuffer(), cb, this, out completed);
                    if(completed && _sendStreams.Count > 0)
                    {
                        // The whole message is written, assume it's sent now for at-most-once semantics.
                        _sendStreams.Peek().isSent = true;
                    }
                }
                else if((operation & IceInternal.SocketOperation.Read) != 0)
                {
                    if(_observer != null && !_readHeader)
                    {
                        observerStartRead(_readStream.pos());
                    }
                    completedSynchronously = _transceiver.startRead(_readStream.getBuffer(), cb, this);
                }
            }
            catch(Ice.LocalException ex)
            {
                setState(StateClosed, ex);
                return false;
            }
            return true;
        }

        public override bool finishAsync(int operation)
        {
            try
            {
                if((operation & IceInternal.SocketOperation.Write) != 0)
                {
                    _transceiver.finishWrite(_writeStream.getBuffer());
                    if(_observer != null)
                    {
                        observerFinishWrite(_writeStream.pos());
                    }
                }
                else if((operation & IceInternal.SocketOperation.Read) != 0)
                {
                    _transceiver.finishRead(_readStream.getBuffer());
                    if(_observer != null && !_readHeader)
                    {
                        observerFinishRead(_readStream.pos());
                    }
                }
            }
            catch(Ice.LocalException ex)
            {
                setState(StateClosed, ex);
            }
            return _state < StateClosed;
        }

        public override void message(ref IceInternal.ThreadPoolCurrent current)
        {
            StartCallback startCB = null;
            Queue<OutgoingMessage> sentCBs = null;
            MessageInfo info = new MessageInfo();

            IceInternal.ThreadPoolMessage msg = new IceInternal.ThreadPoolMessage(_m);
            _m.Lock();
            try
            {
                if(!msg.startIOScope(ref current))
                {
                    return;
                }

                if(_state >= StateClosed)
                {
                    return;
                }

                try
                {
                    unscheduleTimeout(current.operation);
                    if((current.operation & IceInternal.SocketOperation.Write) != 0 && !_writeStream.isEmpty())
                    {
                        if(_observer != null)
                        {
                            observerStartWrite(_writeStream.pos());
                        }
                        if(_writeStream.getBuffer().b.hasRemaining() && !_transceiver.write(_writeStream.getBuffer()))
                        {
                            Debug.Assert(!_writeStream.isEmpty());
                            scheduleTimeout(IceInternal.SocketOperation.Write, _endpoint.timeout());
                            return;
                        }
                        if(_observer != null)
                        {
                            observerFinishWrite(_writeStream.pos());
                        }
                        Debug.Assert(!_writeStream.getBuffer().b.hasRemaining());
                    }
                    if((current.operation & IceInternal.SocketOperation.Read) != 0 && !_readStream.isEmpty())
                    {
                        if(_readHeader) // Read header if necessary.
                        {
                            if(_readStream.getBuffer().b.hasRemaining() && !_transceiver.read(_readStream.getBuffer()))
                            {
                                return;
                            }
                            Debug.Assert(!_readStream.getBuffer().b.hasRemaining());
                            _readHeader = false;

                            if(_observer != null)
                            {
                                _observer.receivedBytes(IceInternal.Protocol.headerSize);
                            }

                            int pos = _readStream.pos();
                            if(pos < IceInternal.Protocol.headerSize)
                            {
                                //
                                // This situation is possible for small UDP packets.
                                //
                                throw new Ice.IllegalMessageSizeException();
                            }

                            _readStream.pos(0);
                            byte[] m = new byte[4];
                            m[0] = _readStream.readByte();
                            m[1] = _readStream.readByte();
                            m[2] = _readStream.readByte();
                            m[3] = _readStream.readByte();
                            if(m[0] != IceInternal.Protocol.magic[0] || m[1] != IceInternal.Protocol.magic[1] ||
                               m[2] != IceInternal.Protocol.magic[2] || m[3] != IceInternal.Protocol.magic[3])
                            {
                                Ice.BadMagicException ex = new Ice.BadMagicException();
                                ex.badMagic = m;
                                throw ex;
                            }

                            ProtocolVersion pv  = new ProtocolVersion();
                            pv.read__(_readStream);
                            IceInternal.Protocol.checkSupportedProtocol(pv);
                            EncodingVersion ev = new EncodingVersion();
                            ev.read__(_readStream);
                            IceInternal.Protocol.checkSupportedProtocolEncoding(ev);

                            _readStream.readByte(); // messageType
                            _readStream.readByte(); // compress
                            int size = _readStream.readInt();
                            if(size < IceInternal.Protocol.headerSize)
                            {
                                throw new Ice.IllegalMessageSizeException();
                            }
                            if(size > _instance.messageSizeMax())
                            {
                                IceInternal.Ex.throwMemoryLimitException(size, _instance.messageSizeMax());
                            }
                            if(size > _readStream.size())
                            {
                                _readStream.resize(size, true);
                            }
                            _readStream.pos(pos);
                        }

                        if(_readStream.getBuffer().b.hasRemaining())
                        {
                            if(_endpoint.datagram())
                            {
                                throw new Ice.DatagramLimitException(); // The message was truncated.
                            }
                            else
                            {
                                if(_observer != null)
                                {
                                    observerStartRead(_readStream.pos());
                                }
                                if(!_transceiver.read(_readStream.getBuffer()))
                                {
                                    Debug.Assert(!_readStream.isEmpty());
                                    scheduleTimeout(IceInternal.SocketOperation.Read, _endpoint.timeout());
                                    return;
                                }
                                if(_observer != null)
                                {
                                    observerFinishRead(_readStream.pos());
                                }
                                Debug.Assert(!_readStream.getBuffer().b.hasRemaining());
                            }
                        }
                    }

                    if(_state <= StateNotValidated)
                    {
                        if(_state == StateNotInitialized && !initialize(current.operation))
                        {
                            return;
                        }

                        if(_state <= StateNotValidated && !validate(current.operation))
                        {
                            return;
                        }

                        _threadPool.unregister(this, current.operation);

                        //
                        // We start out in holding state.
                        //
                        setState(StateHolding);
                        startCB = _startCallback;
                        _startCallback = null;
                    }
                    else
                    {
                        Debug.Assert(_state <= StateClosing);

                        //
                        // We parse messages first, if we receive a close
                        // connection message we won't send more messages.
                        // 
                        if((current.operation & IceInternal.SocketOperation.Read) != 0)
                        {
                            parseMessage(ref info);
                        }

                        if((current.operation & IceInternal.SocketOperation.Write) != 0)
                        {
                            sentCBs = sendNextMessage();
                        }
                    }

                    //
                    // We increment the dispatch count to prevent the
                    // communicator destruction during the callback.
                    //
                    if(sentCBs != null || info.outAsync != null)
                    {
                        ++_dispatchCount;
                    }

                    if(_acmTimeout > 0)
                    {
                        _acmAbsoluteTimeoutMillis = IceInternal.Time.currentMonotonicTimeMillis() + _acmTimeout * 1000;
                    }

                    if(startCB == null && sentCBs == null && info.invokeNum == 0 && info.outAsync == null)
                    {
                        return; // Nothing to dispatch.
                    }

                    msg.completed(ref current);
                }
                catch(DatagramLimitException) // Expected.
                {
                    if(_warnUdp)
                    {
                        _logger.warning("maximum datagram size of " + _readStream.pos() + " exceeded");
                    }
                    _readStream.resize(IceInternal.Protocol.headerSize, true);
                    _readStream.pos(0);
                    _readHeader = true;
                    return;
                }
                catch(SocketException ex)
                {
                    setState(StateClosed, ex);
                    return;
                }
                catch(LocalException ex)
                {
                    if(_endpoint.datagram())
                    {
                        if(_warn)
                        {
                            String s = "datagram connection exception:\n" + ex + '\n' + _desc;
                            _logger.warning(s);
                        }
                        _readStream.resize(IceInternal.Protocol.headerSize, true);
                        _readStream.pos(0);
                        _readHeader = true;
                    }
                    else
                    {
                        setState(StateClosed, ex);
                    }
                    return;
                }
                finally
                {
                    msg.finishIOScope(ref current);
                }

                //
                // Unlike C++/Java, this method is called from an IO thread of the .NET thread
                // pool or from the communicator async IO thread. While it's fine to handle the
                // non-blocking activity of the connection from these threads, the dispatching
                // of the message must be taken care of by the Ice thread pool.
                //
                IceInternal.ThreadPoolCurrent c = current;
                _threadPool.execute(
                    delegate()
                    {
                        if(_dispatcher != null)
                        {
                            try
                            {
                                _dispatcher(delegate()
                                            {
                                                dispatch(startCB, sentCBs, info);
                                            },
                                            this);
                            }
                            catch(System.Exception ex)
                            {
                                if(_instance.initializationData().properties.getPropertyAsIntWithDefault(
                                       "Ice.Warn.Dispatch", 1) > 1)
                                {
                                    warning("dispatch exception", ex);
                                }
                            }
                        }
                        else
                        {
                            dispatch(startCB, sentCBs, info);
                        }
                        msg.destroy(ref c);
                    });
            }
            finally
            {
                _m.Unlock();
            }
        }

        private void dispatch(StartCallback startCB, Queue<OutgoingMessage> sentCBs, MessageInfo info)
        {
            //
            // Notify the factory that the connection establishment and
            // validation has completed.
            //
            if(startCB != null)
            {
                startCB.connectionStartCompleted(this);
            }

            //
            // Notify AMI calls that the message was sent.
            //
            if(sentCBs != null)
            {
                foreach(OutgoingMessage m in sentCBs)
                {
                    if(m.sentCallback != null)
                    {
                        m.outAsync.sent__(m.sentCallback);
                    }
                    if(m.replyOutAsync != null)
                    {
                        m.replyOutAsync.finished__();
                    }
                }
            }

            //
            // Asynchronous replies must be handled outside the thread
            // synchronization, so that nested calls are possible.
            //
            if(info.outAsync != null)
            {
                info.outAsync.finished__();
            }

            if(info.invokeNum > 0)
            {
                //
                // Method invocation (or multiple invocations for batch messages)
                // must be done outside the thread synchronization, so that nested
                // calls are possible.
                //
                invokeAll(info.stream, info.invokeNum, info.requestId, info.compress, info.servantManager,
                          info.adapter);
            }

            //
            // Decrease dispatch count.
            //
            if(sentCBs != null || info.outAsync != null)
            {
                _m.Lock();
                try
                {
                    if(--_dispatchCount == 0)
                    {
                        //
                        // Only initiate shutdown if not already done. It
                        // might have already been done if the sent callback
                        // or AMI callback was dispatched when the connection
                        // was already in the closing state.
                        //
                        if(_state == StateClosing && !_shutdownInitiated)
                        {
                            try
                            {
                                initiateShutdown();
                            }
                            catch(Ice.LocalException ex)
                            {
                                setState(StateClosed, ex);
                            }
                        }
                        else if(_state == StateFinished)
                        {
                            _reaper.add(this, _observer);
                        }
                        _m.NotifyAll();
                    }
                }
                finally
                {
                    _m.Unlock();
                }
            }
        }

        public override void finished(ref IceInternal.ThreadPoolCurrent current)
        {
            _m.Lock();
            try
            {
                Debug.Assert(_state == StateClosed);
                unscheduleTimeout(IceInternal.SocketOperation.Read | IceInternal.SocketOperation.Write);
            }
            finally
            {
                _m.Unlock();
            }

            //
            // If there are no callbacks to call, we don't call ioCompleted() since we're not going
            // to call code that will potentially block (this avoids promoting a new leader and
            // unecessary thread creation, especially if this is called on shutdown).
            //
            if(_startCallback == null && _sendStreams.Count == 0 && _asyncRequests.Count == 0)
            {
                finish();
                return;
            }

            //
            // Unlike C++/Java, this method is called from an IO thread of the .NET thread
            // pool of from the communicator async IO thread. While it's fine to handle the
            // non-blocking activity of the connection from these threads, the dispatching
            // of the message must be taken care of by the Ice thread pool.
            //
            _threadPool.execute(
                delegate()
                {
                    if(_dispatcher != null)
                    {
                        try
                        {
                            _dispatcher(finish, this);
                        }
                        catch(System.Exception ex)
                        {
                            if(_instance.initializationData().properties.getPropertyAsIntWithDefault(
                                   "Ice.Warn.Dispatch", 1) > 1)
                            {
                                warning("dispatch exception", ex);
                            }
                        }
                    }
                    else
                    {
                        finish();
                    }
                });
        }

        private void finish()
        {
            if(_startCallback != null)
            {
                _startCallback.connectionStartFailed(this, _exception);
                _startCallback = null;
            }

            if(_sendStreams.Count > 0)
            {
                if(!_writeStream.isEmpty())
                {
                    //
                    // Return the stream to the outgoing call. This is important for
                    // retriable AMI calls which are not marshalled again.
                    //
                    OutgoingMessage message = _sendStreams.Peek();
                    _writeStream.swap(message.stream);
                    
                    //
                    // The current message might be sent but not yet removed from _sendStreams. If
                    // the response has been received in the meantime, we remove the message from
                    // _sendStreams to not call finished on a message which is already done.
                    //
                    if(message.requestId > 0 &&
                       (message.@out != null && !_requests.ContainsKey(message.requestId) ||
                        message.outAsync != null && !_asyncRequests.ContainsKey(message.requestId)))
                    {
                        if(message.sent(this, true))
                        {
                            Debug.Assert(message.outAsync != null);
                            message.outAsync.sent__(message.sentCallback);
                        }
                        _sendStreams.Dequeue();
                    }
                }

                foreach(OutgoingMessage m in _sendStreams)
                {
                    m.finished(_exception);
                    if(m.requestId > 0) // Make sure finished isn't called twice.
                    {
                        if(m.@out != null)
                        {
                            _requests.Remove(m.requestId);
                        }
                        else
                        {
                            _asyncRequests.Remove(m.requestId);
                        }
                    }
                }
                _sendStreams.Clear();
            }

            foreach(IceInternal.Outgoing o in _requests.Values)
            {
                o.finished(_exception, true);
            }
            _requests.Clear();

            foreach(IceInternal.OutgoingAsync o in _asyncRequests.Values)
            {
                o.finished__(_exception, true);
            }
            _asyncRequests.Clear();

            //
            // Don't wait to be reaped to reclaim memory allocated by read/write streams.
            //
            _writeStream.clear();
            _writeStream.getBuffer().clear();
            _readStream.clear();
            _readStream.getBuffer().clear();
            _incomingCache = null;

            //
            // This must be done last as this will cause waitUntilFinished() to return (and communicator
            // objects such as the timer might be destroyed too).
            //
            _m.Lock();
            try
            {
                setState(StateFinished);
                if(_dispatchCount == 0)
                {
                    _reaper.add(this, _observer);
                }
            }
            finally
            {
                _m.Unlock();
            }
        }

        public override string ToString()
        {
            return _desc; // No mutex lock, _desc is immutable.
        }

        public void timedOut()
        {
            _m.Lock();
            try
            {
                if(_state <= StateNotValidated)
                {
                    setState(StateClosed, new ConnectTimeoutException());
                }
                else if(_state < StateClosing)
                {
                    setState(StateClosed, new TimeoutException());
                }
                else if(_state == StateClosing)
                {
                    setState(StateClosed, new CloseTimeoutException());
                }
            }
            finally
            {
                _m.Unlock();
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

        public ConnectionInfo getInfo()
        {
            _m.Lock();
            try
            {
                if(_state >= StateClosed)
                {
                    throw _exception;
                }
                return initConnectionInfo();
            }
            finally
            {
                _m.Unlock();
            }
        }

        public string ice_toString_()
        {
            return ToString();
        }

        public void exception(LocalException ex)
        {
            _m.Lock();
            try
            {
                setState(StateClosed, ex);
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void invokeException(LocalException ex, int invokeNum)
        {
            //
            // Fatal exception while invoking a request. Since sendResponse/sendNoResponse isn't
            // called in case of a fatal exception we decrement _dispatchCount here.
            //

            _m.Lock();
            try
            {
                setState(StateClosed, ex);

                if(invokeNum > 0)
                {
                    Debug.Assert(_dispatchCount > 0);
                    _dispatchCount -= invokeNum;
                    Debug.Assert(_dispatchCount >= 0);
                    if(_dispatchCount == 0)
                    {
                        if(_state == StateFinished)
                        {
                            _reaper.add(this, _observer);
                        }
                        _m.NotifyAll();
                    }
                }
            }
            finally
            {
                _m.Unlock();
            }
        }

        static ConnectionI()
        {
            _compressionSupported = IceInternal.BasicStream.compressible();
        }

        internal ConnectionI(Communicator communicator, IceInternal.Instance instance,
                             IceInternal.ConnectionReaper reaper, IceInternal.Transceiver transceiver,
                             IceInternal.Connector connector, IceInternal.EndpointI endpoint, ObjectAdapter adapter)
        {
            _communicator = communicator;
            _instance = instance;
            _reaper = reaper;
            InitializationData initData = instance.initializationData();
            _transceiver = transceiver;
            _desc = transceiver.ToString();
            _type = transceiver.type();
            _connector = connector;
            _endpoint = endpoint;
            _adapter = adapter;
            _dispatcher = initData.dispatcher; // Cached for better performance.
            _logger = initData.logger; // Cached for better performance.
            _traceLevels = instance.traceLevels(); // Cached for better performance.
            _timer = instance.timer();
            _writeTimeout = new TimeoutCallback(this);
            _writeTimeoutScheduled = false;
            _writeStreamPos = -1;
            _readTimeout = new TimeoutCallback(this);
            _readTimeoutScheduled = false;
            _readStreamPos = -1;
            _warn = initData.properties.getPropertyAsInt("Ice.Warn.Connections") > 0;
            _warnUdp = initData.properties.getPropertyAsInt("Ice.Warn.Datagrams") > 0;
            _cacheBuffers = initData.properties.getPropertyAsIntWithDefault("Ice.CacheMessageBuffers", 1) == 1;
            _acmAbsoluteTimeoutMillis = 0;
            _nextRequestId = 1;
            _batchAutoFlush = initData.properties.getPropertyAsIntWithDefault("Ice.BatchAutoFlush", 1) > 0;
            _batchStream = new IceInternal.BasicStream(instance, Util.currentProtocolEncoding, _batchAutoFlush);
            _batchStreamInUse = false;
            _batchRequestNum = 0;
            _batchRequestCompress = false;
            _batchMarker = 0;
            _readStream = new IceInternal.BasicStream(instance, Util.currentProtocolEncoding);
            _readHeader = false;
            _writeStream = new IceInternal.BasicStream(instance, Util.currentProtocolEncoding);
            _dispatchCount = 0;
            _state = StateNotInitialized;

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
                if(_endpoint.datagram())
                {
                    _acmTimeout = 0;
                }
                else
                {
                    if(adapterImpl != null)
                    {
                        _acmTimeout = adapterImpl.getACM();
                    }
                    else
                    {
                        _acmTimeout = _instance.clientACM();
                    }
                }

                if(adapterImpl != null)
                {
                    _threadPool = adapterImpl.getThreadPool();
                }
                else
                {
                    _threadPool = instance.clientThreadPool();
                }
                _threadPool.initialize(this);
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
        private const int StateFinished = 6;

        private void setState(int state, LocalException ex)
        {
            //
            // If setState() is called with an exception, then only closed
            // and closing states are permissible.
            //
            Debug.Assert(state >= StateClosing);

            if(_state == state) // Don't switch twice.
            {
                return;
            }

            if(_exception == null)
            {
                _exception = ex;

                //
                // We don't warn if we are not validated.
                //
                if(_warn && _validated)
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

            try
            {
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
                    _threadPool.register(this, IceInternal.SocketOperation.Read);
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
                    if(_state == StateActive)
                    {
                        _threadPool.unregister(this, IceInternal.SocketOperation.Read);
                    }
                    break;
                }

                case StateClosing:
                {
                    //
                    // Can't change back from closed.
                    //
                    if(_state >= StateClosed)
                    {
                        return;
                    }
                    if(_state == StateHolding)
                    {
                        //
                        // We need to continue to read in closing state.
                        //
                        _threadPool.register(this, IceInternal.SocketOperation.Read);
                    }
                    break;
                }

                case StateClosed:
                {
                    if(_state == StateFinished)
                    {
                        return;
                    }
                    _threadPool.finish(this);
                    _transceiver.close();
                    break;
                }

                case StateFinished:
                {
                    Debug.Assert(_state == StateClosed);
                    _communicator = null;
                    break;
                }
                }
            }
            catch(Ice.LocalException ex)
            {
                _logger.error("unexpected connection exception:\n" + ex + "\n" + _transceiver.ToString());
            }

            //
            // We only register with the connection monitor if our new state
            // is StateActive. Otherwise we unregister with the connection
            // monitor, but only if we were registered before, i.e., if our
            // old state was StateActive.
            //
            if(_acmTimeout > 0)
            {
                if(state == StateActive)
                {
                    _instance.connectionMonitor().add(this);
                }
                else if(_state == StateActive)
                {
                    _instance.connectionMonitor().remove(this);
                }
            }

            if(_instance.getObserver() != null)
            {
                ConnectionState oldState = toConnectionState(_state);
                ConnectionState newState = toConnectionState(state);
                if(oldState != newState)
                {
                    _observer = _instance.getObserver().getConnectionObserver(initConnectionInfo(),
                                                                              _endpoint, 
                                                                              newState,
                                                                              _observer);
                    if(_observer != null)
                    {
                        _observer.attach();
                    }
                    else
                    {
                        _writeStreamPos = -1;
                        _readStreamPos = -1;
                    }
                }
                if(_observer != null && state == StateClosed && _exception != null)
                {
                    if(!(_exception is CloseConnectionException ||
                         _exception is ForcedCloseConnectionException ||
                         _exception is ConnectionTimeoutException ||
                         _exception is CommunicatorDestroyedException ||
                         _exception is ObjectAdapterDeactivatedException ||
                         (_exception is ConnectionLostException && _state == StateClosing)))
                    {
                        _observer.failed(_exception.ice_name());
                    }
                }
            }
            _state = state;

            _m.NotifyAll();

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
            Debug.Assert(!_shutdownInitiated);

            _shutdownInitiated = true;

            if(!_endpoint.datagram())
            {
                //
                // Before we shut down, we send a close connection
                // message.
                //
                IceInternal.BasicStream os = new IceInternal.BasicStream(_instance, Util.currentProtocolEncoding);
                os.writeBlob(IceInternal.Protocol.magic);
                Ice.Util.currentProtocol.write__(os);
                Ice.Util.currentProtocolEncoding.write__(os);
                os.writeByte(IceInternal.Protocol.closeConnectionMsg);
                os.writeByte(_compressionSupported ? (byte)1 : (byte)0);
                os.writeInt(IceInternal.Protocol.headerSize); // Message size.

                if(sendMessage(new OutgoingMessage(os, false, false)))
                {
                    //
                    // Schedule the close timeout to wait for the peer to close the connection. If
                    // the message was queued for sending, sendNextMessage will schedule the timeout
                    // once all messages were sent.
                    //
                    scheduleTimeout(IceInternal.SocketOperation.Write, closeTimeout());
                }

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

        private bool initialize(int operation)
        {
            int s = _transceiver.initialize();
            if(s != IceInternal.SocketOperation.None)
            {
                scheduleTimeout(s, connectTimeout());
                _threadPool.update(this, operation, s);
                return false;
            }

            //
            // Update the connection description once the transceiver is initialized.
            //
            _desc = _transceiver.ToString();
            setState(StateNotValidated);
            return true;
        }

        private bool validate(int operation)
        {
            if(!_endpoint.datagram()) // Datagram connections are always implicitly validated.
            {
                if(_adapter != null) // The server side has the active role for connection validation.
                {
                    if(_writeStream.size() == 0)
                    {
                        _writeStream.writeBlob(IceInternal.Protocol.magic);
                        Ice.Util.currentProtocol.write__(_writeStream);
                        Ice.Util.currentProtocolEncoding.write__(_writeStream);
                        _writeStream.writeByte(IceInternal.Protocol.validateConnectionMsg);
                        _writeStream.writeByte((byte)0); // Compression status (always zero for validate connection).
                        _writeStream.writeInt(IceInternal.Protocol.headerSize); // Message size.
                        IceInternal.TraceUtil.traceSend(_writeStream, _logger, _traceLevels);
                        _writeStream.prepareWrite();
                    }

                    if(_observer != null)
                    {
                        observerStartWrite(_writeStream.pos());
                    }
                    if(_writeStream.pos() != _writeStream.size() && !_transceiver.write(_writeStream.getBuffer()))
                    {
                        scheduleTimeout(IceInternal.SocketOperation.Write, connectTimeout());
                        _threadPool.update(this, operation, IceInternal.SocketOperation.Write);
                        return false;
                    }
                    if(_observer != null)
                    {
                        observerFinishWrite(_writeStream.pos());
                    }
                }
                else // The client side has the passive role for connection validation.
                {
                    if(_readStream.size() == 0)
                    {
                        _readStream.resize(IceInternal.Protocol.headerSize, true);
                        _readStream.pos(0);
                    }

                    if(_observer != null)
                    {
                        observerStartRead(_readStream.pos());
                    }
                    if(_readStream.pos() != _readStream.size() && !_transceiver.read(_readStream.getBuffer()))
                    {
                        scheduleTimeout(IceInternal.SocketOperation.Read, connectTimeout());
                        _threadPool.update(this, operation, IceInternal.SocketOperation.Read);
                        return false;
                    }
                    if(_observer != null)
                    {
                        observerFinishRead(_readStream.pos());
                    }

                    Debug.Assert(_readStream.pos() == IceInternal.Protocol.headerSize);
                    _readStream.pos(0);
                    byte[] m = _readStream.readBlob(4);
                    if(m[0] != IceInternal.Protocol.magic[0] || m[1] != IceInternal.Protocol.magic[1] ||
                       m[2] != IceInternal.Protocol.magic[2] || m[3] != IceInternal.Protocol.magic[3])
                    {
                        BadMagicException ex = new BadMagicException();
                        ex.badMagic = m;
                        throw ex;
                    }

                    ProtocolVersion pv  = new ProtocolVersion();
                    pv.read__(_readStream);
                    IceInternal.Protocol.checkSupportedProtocol(pv);
                    EncodingVersion ev = new EncodingVersion();
                    ev.read__(_readStream);
                    IceInternal.Protocol.checkSupportedProtocolEncoding(ev);

                    byte messageType = _readStream.readByte();
                    if(messageType != IceInternal.Protocol.validateConnectionMsg)
                    {
                        throw new ConnectionNotValidatedException();
                    }
                    _readStream.readByte(); // Ignore compression status for validate connection.
                    int size = _readStream.readInt();
                    if(size != IceInternal.Protocol.headerSize)
                    {
                        throw new IllegalMessageSizeException();
                    }
                    IceInternal.TraceUtil.traceRecv(_readStream, _logger, _traceLevels);

                    _validated = true;
                }
            }

            _writeStream.resize(0, false);
            _writeStream.pos(0);

            _readStream.resize(IceInternal.Protocol.headerSize, true);
            _readStream.pos(0);
            _readHeader = true;

            return true;
        }

        private Queue<OutgoingMessage> sendNextMessage()
        {
            Debug.Assert(_sendStreams.Count > 0);
            Debug.Assert(!_writeStream.isEmpty() && _writeStream.pos() == _writeStream.size());

            Queue<OutgoingMessage> callbacks = null;
            try
            {
                while(true)
                {
                    //
                    // Notify the message that it was sent.
                    //
                    OutgoingMessage message = _sendStreams.Peek();
                    _writeStream.swap(message.stream);
                    Debug.Assert(_writeStream.isEmpty());
                    if(message.sent(this, true) || message.replyOutAsync != null)
                    {
                        Debug.Assert(message.outAsync != null);
                        if(callbacks == null)
                        {
                            callbacks = new Queue<OutgoingMessage>();
                        }
                        callbacks.Enqueue(message);
                    }
                    _sendStreams.Dequeue();

                    //
                    // If there's nothing left to send, we're done.
                    //
                    if(_sendStreams.Count == 0)
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
                    if(_state >= StateClosed)
                    {
                        return callbacks;
                    }
                
                    //
                    // Otherwise, prepare the next message stream for writing.
                    //
                    message = _sendStreams.Peek();
                    Debug.Assert(!message.prepared);
                    IceInternal.BasicStream stream = message.stream;

                    message.stream = doCompress(message.stream, message.compress);
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
                    _writeStream.swap(message.stream);

                    //
                    // Send the message.
                    //
                    if(_observer != null)
                    {
                        observerStartWrite(_writeStream.pos());
                    }
                    if(_writeStream.pos() != _writeStream.size() && !_transceiver.write(_writeStream.getBuffer()))
                    {
                        Debug.Assert(!_writeStream.isEmpty());
                        scheduleTimeout(IceInternal.SocketOperation.Write, _endpoint.timeout());
                        return callbacks;
                    }
                    if(_observer != null)
                    {
                        observerFinishWrite(_writeStream.pos());
                    }
                }
            }
            catch(Ice.LocalException ex)
            {
                setState(StateClosed, ex);
                return callbacks;
            }

            Debug.Assert(_writeStream.isEmpty());
            _threadPool.unregister(this, IceInternal.SocketOperation.Write);

            //
            // If all the messages were sent and we are in the closing state, we schedule
            // the close timeout to wait for the peer to close the connection.
            //
            if(_state == StateClosing)
            {
                scheduleTimeout(IceInternal.SocketOperation.Write, closeTimeout());
            }

            return callbacks;
        }


        private bool sendMessage(OutgoingMessage message)
        {
            Debug.Assert(_state < StateClosed);
            if(_sendStreams.Count > 0)
            {
                message.adopt();
                _sendStreams.Enqueue(message);
                return false;
            }

            //
            // Attempt to send the message without blocking. If the send blocks, we use
            // asynchronous I/O or we request the caller to call finishSendMessage() outside
            // the synchronization.
            //

            Debug.Assert(!message.prepared);

            IceInternal.BasicStream stream = message.stream;

            message.stream = doCompress(stream, message.compress);
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

            if(_observer != null)
            {
                observerStartWrite(message.stream.pos());
            }
            if(_transceiver.write(message.stream.getBuffer()))
            {
                if(_observer != null)
                {
                    observerFinishWrite(message.stream.pos());
                }
                message.sent(this, false);
                if(_acmTimeout > 0)
                {
                    _acmAbsoluteTimeoutMillis = IceInternal.Time.currentMonotonicTimeMillis() + _acmTimeout * 1000;
                }
                return true;
            }
            message.adopt();

            _writeStream.swap(message.stream);
            _sendStreams.Enqueue(message);
            scheduleTimeout(IceInternal.SocketOperation.Write, _endpoint.timeout());
            _threadPool.register(this, IceInternal.SocketOperation.Write);
            return false;
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

        private struct MessageInfo
        {
            public IceInternal.BasicStream stream;
            public int invokeNum;
            public int requestId;
            public byte compress;
            public IceInternal.ServantManager servantManager;
            public ObjectAdapter adapter;
            public IceInternal.OutgoingAsync outAsync;
        }

        private void parseMessage(ref MessageInfo info)
        {
            Debug.Assert(_state > StateNotValidated && _state < StateClosed);

            info.stream = new IceInternal.BasicStream(_instance, Util.currentProtocolEncoding);
            _readStream.swap(info.stream);
            _readStream.resize(IceInternal.Protocol.headerSize, true);
            _readStream.pos(0);
            _readHeader = true;

            //
            // Connection is validated on first message. This is only used by
            // setState() to check wether or not we can print a connection
            // warning (a client might close the connection forcefully if the
            // connection isn't validated).
            //
            _validated = true;

            try
            {
                //
                // The magic and version fields have already been checked.
                //
                Debug.Assert(info.stream.pos() == info.stream.size());
                info.stream.pos(8);
                byte messageType = info.stream.readByte();
                info.compress = info.stream.readByte();
                if(info.compress == (byte)2)
                {
                    if(_compressionSupported)
                    {
                        info.stream = info.stream.uncompress(IceInternal.Protocol.headerSize);
                    }
                    else
                    {
                        FeatureNotSupportedException ex = new FeatureNotSupportedException();
                        ex.unsupportedFeature = "Cannot uncompress compressed message: bzip2 DLL not found";
                        throw ex;
                    }
                }
                info.stream.pos(IceInternal.Protocol.headerSize);

                switch(messageType)
                {
                    case IceInternal.Protocol.closeConnectionMsg:
                    {
                        IceInternal.TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                        if(_endpoint.datagram())
                        {
                            if(_warn)
                            {
                                _logger.warning("ignoring close connection message for datagram connection:\n" + _desc);
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
                                                        "(ignored by server, client will retry)", info.stream, _logger,
                                                        _traceLevels);
                        }
                        else
                        {
                            IceInternal.TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                            info.requestId = info.stream.readInt();
                            info.invokeNum = 1;
                            info.servantManager = _servantManager;
                            info.adapter = _adapter;
                            ++_dispatchCount;
                        }
                        break;
                    }

                    case IceInternal.Protocol.requestBatchMsg:
                    {
                        if(_state == StateClosing)
                        {
                            IceInternal.TraceUtil.trace("received batch request during closing\n" +
                                                        "(ignored by server, client will retry)", info.stream, _logger,
                                                        _traceLevels);
                        }
                        else
                        {
                            IceInternal.TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                            info.invokeNum = info.stream.readInt();
                            if(info.invokeNum < 0)
                            {
                                info.invokeNum = 0;
                                throw new UnmarshalOutOfBoundsException();
                            }
                            info.servantManager = _servantManager;
                            info.adapter = _adapter;
                            _dispatchCount += info.invokeNum;
                        }
                        break;
                    }

                    case IceInternal.Protocol.replyMsg:
                    {
                        IceInternal.TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                        info.requestId = info.stream.readInt();
                        IceInternal.Outgoing og = null;
                        if(_requests.TryGetValue(info.requestId, out og))
                        {
                            _requests.Remove(info.requestId);
                            og.finished(info.stream);
                        }
                        else
                        {
                            if(!_asyncRequests.TryGetValue(info.requestId, out info.outAsync))
                            {
                                throw new UnknownRequestIdException();
                            }
                            _asyncRequests.Remove(info.requestId);

                            info.outAsync.istr__.swap(info.stream);

                            //
                            // If we just received the reply for a request which isn't acknowledge as 
                            // sent yet, we queue the reply instead of processing it right away. It 
                            // will be processed once the write callback is invoked for the message.
                            //
                            OutgoingMessage message = _sendStreams.Count > 0 ? _sendStreams.Peek() : null;
                            if(message != null && message.outAsync == info.outAsync)
                            {
                                message.replyOutAsync = info.outAsync;
                                info.outAsync = null;
                            }
                        }
                        _m.NotifyAll(); // Notify threads blocked in close(false)
                        break;
                    }

                    case IceInternal.Protocol.validateConnectionMsg:
                    {
                        IceInternal.TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                        if(_warn)
                        {
                            _logger.warning("ignoring unexpected validate connection message:\n" + _desc);
                        }
                        break;
                    }

                    default:
                    {
                        IceInternal.TraceUtil.trace("received unknown message\n(invalid, closing connection)",
                                                    info.stream, _logger, _traceLevels);
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

                    //
                    // Dispatch the invocation.
                    //
                    inc.invoke(servantManager, stream);

                    --invokeNum;

                    reclaimIncoming(inc);
                    inc = null;
                }

                stream.clear();
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

        private void scheduleTimeout(int status, int timeout)
        {
            if(timeout < 0)
            {
                return;
            }

            if((status & IceInternal.SocketOperation.Read) != 0)
            {
                _timer.schedule(_readTimeout, timeout);
                _readTimeoutScheduled = true;
            }
            if((status & (IceInternal.SocketOperation.Write | IceInternal.SocketOperation.Connect)) != 0)
            {
                _timer.schedule(_writeTimeout, timeout);
                _writeTimeoutScheduled = true;
            }
        }

        private void unscheduleTimeout(int status)
        {
            if((status & IceInternal.SocketOperation.Read) != 0 && _readTimeoutScheduled)
            {
                _timer.cancel(_readTimeout);
                _readTimeoutScheduled = false;
            }
            if((status & (IceInternal.SocketOperation.Write | IceInternal.SocketOperation.Connect)) != 0 &&
                _writeTimeoutScheduled)
            {
                _timer.cancel(_writeTimeout);
                _writeTimeoutScheduled = false;
            }
        }

        private int connectTimeout()
        {
            IceInternal.DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
            if(defaultsAndOverrides.overrideConnectTimeout)
            {
                return defaultsAndOverrides.overrideConnectTimeoutValue;
            }
            else
            {
                return _endpoint.timeout();
            }
        }

        private int closeTimeout()
        {
            IceInternal.DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
            if(defaultsAndOverrides.overrideCloseTimeout)
            {
                return defaultsAndOverrides.overrideCloseTimeoutValue;
            }
            else
            {
                return _endpoint.timeout();
            }
        }

        private ConnectionInfo initConnectionInfo()
        {
            if(_info != null)
            {
                return _info;
            }

            ConnectionInfo info = _transceiver.getInfo();
            info.connectionId = _endpoint.connectionId();
            info.adapterName = _adapter != null ? _adapter.getName() : "";
            info.incoming = _connector == null;
            if(_state > StateNotInitialized)
            {
                _info = info; // Cache the connection information only if initialized.
            }
            return info;
        }

        ConnectionState toConnectionState(int state)
        {
            return connectionStateMap[state];
        }

        private void warning(string msg, System.Exception ex)
        {
            _logger.warning(msg + ":\n" + ex + "\n" + _transceiver.ToString());
        }

        private void observerStartRead(int pos)
        {
            if(_readStreamPos >= 0)
            {
                _observer.receivedBytes(pos - _readStreamPos);
            }
            _readStreamPos = pos;
        }

        private void observerFinishRead(int pos)
        {
            if(_readStreamPos == -1)
            {
                return;
            }
            Debug.Assert(pos >= _readStreamPos);
            _observer.receivedBytes(pos - _readStreamPos);
            _readStreamPos = -1;
        }

        private void observerStartWrite(int pos)
        {
            if(_writeStreamPos >= 0)
            {
                _observer.sentBytes(pos - _writeStreamPos);
            }
            _writeStreamPos = pos;
        }

        private void observerFinishWrite(int pos)
        {
            if(_writeStreamPos == -1)
            {
                return;
            }
            Debug.Assert(pos >= _writeStreamPos);
            _observer.sentBytes(pos - _writeStreamPos);
            _writeStreamPos = -1;
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
                                                OperationMode mode, Dictionary<string, string> context,
                                                InvocationObserver observer)
        {
            IceInternal.Outgoing og = null;

            if(_cacheBuffers)
            {
                lock(_outgoingCacheMutex)
                {
                    if(_outgoingCache == null)
                    {
                        og = new IceInternal.Outgoing(handler, operation, mode, context, observer);
                    }
                    else
                    {
                        og = _outgoingCache;
                        _outgoingCache = _outgoingCache.next;
                        og.reset(handler, operation, mode, context, observer);
                        og.next = null;
                    }
                }
            }
            else
            {
                og = new IceInternal.Outgoing(handler, operation, mode, context, observer);
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

        private class OutgoingMessage
        {
            internal OutgoingMessage(IceInternal.BasicStream stream, bool compress, bool adopt)
            {
                this.stream = stream;
                this.compress = compress;
                this._adopt = adopt;
                this.isSent = false;
                this.requestId = 0;
            }

            internal OutgoingMessage(IceInternal.OutgoingMessageCallback @out, IceInternal.BasicStream stream,
                                     bool compress, int requestId)
            {
                this.stream = stream;
                this.compress = compress;
                this.@out = @out;
                this.requestId = requestId;
                this.isSent = false;
            }

            internal OutgoingMessage(IceInternal.OutgoingAsyncMessageCallback @out, IceInternal.BasicStream stream,
                                     bool compress, int requestId)
            {
                this.stream = stream;
                this.compress = compress;
                this.outAsync = @out;
                this.requestId = requestId;
                this.isSent = false;
            }

            internal void adopt()
            {
                if(_adopt)
                {
                    IceInternal.BasicStream stream = new IceInternal.BasicStream(this.stream.instance(), 
                                                                                 Util.currentProtocolEncoding);
                    stream.swap(this.stream);
                    this.stream = stream;
                    _adopt = false;
                }
            }

            internal bool sent(ConnectionI connection, bool notify)
            {
                isSent = true; // The message is sent.

                if(@out != null)
                {
                    @out.sent(notify); // true = notify the waiting thread that the request was sent.
                    return false;
                }
                else if(outAsync != null)
                {
                    sentCallback = outAsync.sent__(connection);
                    return sentCallback != null;
                }
                else
                {
                    return false;
                }
            }

            internal void finished(LocalException ex)
            {
                if(@out != null)
                {
                    @out.finished(ex, isSent);
                }
                else if(outAsync != null)
                {
                    outAsync.finished__(ex, isSent);
                }
            }

            internal IceInternal.BasicStream stream;
            internal IceInternal.OutgoingMessageCallback @out;
            internal IceInternal.OutgoingAsyncMessageCallback outAsync;
            internal IceInternal.OutgoingAsync replyOutAsync;
            internal bool compress;
            internal int requestId;
            internal bool _adopt;
            internal bool prepared;
            internal bool isSent;
            internal Ice.AsyncCallback sentCallback = null;
        }

        private Communicator _communicator;
        private IceInternal.Instance _instance;
        private IceInternal.ConnectionReaper _reaper;
        private IceInternal.Transceiver _transceiver;
        private string _desc;
        private string _type;
        private IceInternal.Connector _connector;
        private IceInternal.EndpointI _endpoint;

        private ObjectAdapter _adapter;
        private IceInternal.ServantManager _servantManager;

        private Dispatcher _dispatcher;
        private Logger _logger;
        private IceInternal.TraceLevels _traceLevels;
        private IceInternal.ThreadPool _threadPool;

        private IceInternal.Timer _timer;
        private IceInternal.TimerTask _writeTimeout;
        private bool _writeTimeoutScheduled;
        private IceInternal.TimerTask _readTimeout;
        private bool _readTimeoutScheduled;

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

        private Queue<OutgoingMessage> _sendStreams = new Queue<OutgoingMessage>();

        private IceInternal.BasicStream _readStream;
        private bool _readHeader;
        private IceInternal.BasicStream _writeStream;

        private ConnectionObserver _observer;
        private int _readStreamPos;
        private int _writeStreamPos;
        
        private int _dispatchCount;

        private int _state; // The current state.
        private bool _shutdownInitiated = false;
        private bool _validated = false;

        private IceInternal.Incoming _incomingCache;
        private object _incomingCacheMutex = new object();

        private IceInternal.Outgoing _outgoingCache;
        private object _outgoingCacheMutex = new object();

        private static bool _compressionSupported;

        private bool _cacheBuffers;

        private Ice.ConnectionInfo _info;

        private static ConnectionState[] connectionStateMap = new ConnectionState[] {
            ConnectionState.ConnectionStateValidating,   // StateNotInitialized
            ConnectionState.ConnectionStateValidating,   // StateNotValidated
            ConnectionState.ConnectionStateActive,       // StateActive
            ConnectionState.ConnectionStateHolding,      // StateHolding
            ConnectionState.ConnectionStateClosing,      // StateClosing
            ConnectionState.ConnectionStateClosed,       // StateClosed
            ConnectionState.ConnectionStateClosed,       // StateFinished
        };

        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }
}
