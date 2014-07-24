// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

    public sealed class ConnectionI : IceInternal.EventHandler, IceInternal.ResponseHandler, Connection
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
                lock(this)
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
                            System.Threading.Monitor.Wait(this);
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
            lock(this)
            {
                if(_state <= StateNotValidated)
                {
                    return;
                }

                if(_acmLastActivity > 0)
                {
                    _acmLastActivity = IceInternal.Time.currentMonotonicTimeMillis();
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
                        System.Threading.Monitor.Wait(this);
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
            // We can use TryLock here, because as long as there are still
            // threads operating in this connection object, connection
            // destruction is considered as not yet finished.
            //
            if(!System.Threading.Monitor.TryEnter(this))
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
                return true;
            }
            finally
            {
                System.Threading.Monitor.Exit(this);
            }
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
                    System.Threading.Monitor.Wait(this);
                }
            }
        }

        public void waitUntilFinished()
        {
            lock(this)
            {
                //
                // We wait indefinitely until the connection is finished and all
                // outstanding requests are completed. Otherwise we couldn't
                // guarantee that there are no outstanding calls when deactivate()
                // is called on the servant locators.
                //
                while(_state < StateFinished || _dispatchCount > 0)
                {
                    System.Threading.Monitor.Wait(this);
                }

                Debug.Assert(_state == StateFinished);

                //
                // Clear the OA. See bug 1673 for the details of why this is necessary.
                //
                _adapter = null;
            }
        }

        public void updateObserver()
        {
            lock(this)
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
        }

        public void monitor(long now, IceInternal.ACMConfig acm)
        {
            lock(this)
            {
                if(_state != StateActive)
                {
                    return;
                }

                if(_readStream.size() > IceInternal.Protocol.headerSize || !_writeStream.isEmpty())
                {
                    //
                    // If writing or reading, nothing to do, the connection
                    // timeout will kick-in if writes or reads don't progress. 
                    // This check is necessary because the actitivy timer is 
                    // only set when a message is fully read/written.
                    //
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
                
                if(acm.heartbeat == ACMHeartbeat.HeartbeatAlways ||
                   (acm.heartbeat != ACMHeartbeat.HeartbeatOff && now >= (_acmLastActivity + acm.timeout / 4)))
                {
                    if(acm.heartbeat != ACMHeartbeat.HeartbeatOnInvocation || _dispatchCount > 0)
                    {
                        heartbeat();
                    }
                }
    
                if(acm.close != ACMClose.CloseOff && now >= (_acmLastActivity + acm.timeout))
                {
                    if(acm.close == ACMClose.CloseOnIdleForceful || 
                       (acm.close != ACMClose.CloseOnIdle && (_requests.Count > 0 || _asyncRequests.Count > 0)))
                    {
                        //
                        // Close the connection if we didn't receive a heartbeat in
                        // the last period.
                        //
                        setState(StateClosed, new ConnectionTimeoutException());
                    }
                    else if(acm.close != ACMClose.CloseOnInvocation && 
                            _dispatchCount == 0 && _batchStream.isEmpty()  && 
                            _requests.Count == 0 && _asyncRequests.Count == 0)
                    {
                        //
                        // The connection is idle, close it.
                        //
                        setState(StateClosing, new ConnectionTimeoutException());
                    }
                }
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
                    throw new IceInternal.RetryException(_exception);
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
        }

        public bool sendAsyncRequest(IceInternal.OutgoingAsync og, bool compress, bool response,
                                     out Ice.AsyncCallback sentCallback)
        {
            IceInternal.BasicStream os = og.ostr__;

            lock(this)
            {
                if(_exception != null)
                {
                    //
                    // If the connection is closed before we even have a chance
                    // to send our request, we always try to send the request
                    // again.
                    //
                    throw new IceInternal.RetryException(_exception);
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
                    OutgoingMessage msg = new OutgoingMessage(og, os, compress, requestId);
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
                    System.Threading.Monitor.Wait(this);
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
                        throw new IceInternal.RetryException(_exception);
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
                        return;
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
                    System.Threading.Monitor.PulseAll(this);
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
                _batchStream = new IceInternal.BasicStream(_instance, Util.currentProtocolEncoding, _batchAutoFlush);
                _batchRequestNum = 0;
                _batchRequestCompress = false;
                _batchMarker = 0;

                Debug.Assert(_batchStreamInUse);
                _batchStreamInUse = false;
                System.Threading.Monitor.PulseAll(this);
            }
        }

        public void flushBatchRequests()
        {
            IceInternal.BatchOutgoing @out = new IceInternal.BatchOutgoing(this, _instance, __flushBatchRequests_name);
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
                result.invoke__();
            }
            catch(LocalException ex)
            {
                result.invokeExceptionAsync__(ex);
            }

            return result;
        }

        public bool flushBatchRequests(IceInternal.BatchOutgoing @out)
        {
            lock(this)
            {
                while(_batchStreamInUse && _exception == null)
                {
                    System.Threading.Monitor.Wait(this);
                }

                if(_exception != null)
                {
                    throw _exception;
                }

                if(_batchRequestNum == 0)
                {
                    @out.sent();
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

                //
                // Send the batch stream.
                //
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
        }

        public bool flushAsyncBatchRequests(IceInternal.BatchOutgoingAsync outAsync, out Ice.AsyncCallback sentCallback)
        {
            lock(this)
            {
                while(_batchStreamInUse && _exception == null)
                {
                    System.Threading.Monitor.Wait(this);
                }

                if(_exception != null)
                {
                    throw _exception;
                }
                
                if(_batchRequestNum == 0)
                {
                    sentCallback = outAsync.sent__();
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
        }

        public void setCallback(ConnectionCallback callback)
        {
            lock(this)
            {
                if(_state > StateClosing)
                {
                    return;
                }
                _callback = callback;
            }
        }

        public void setACM(Optional<int> timeout, Optional<ACMClose> close, Optional<ACMHeartbeat> heartbeat)
        {
            lock(this)
            {
                if(_monitor != null)
                {
                    if(_state == StateActive)
                    {
                        _monitor.remove(this);
                    }
                    _monitor = _monitor.acm(timeout, close, heartbeat);
                    if(_state == StateActive)
                    {
                        _monitor.add(this);
                    }

                    if(_monitor.getACM().timeout <= 0)
                    {
                        _acmLastActivity = -1; // Disable the recording of last activity.
                    }
                    else if(_state == StateActive && _acmLastActivity == -1)
                    {
                        _acmLastActivity = IceInternal.Time.currentMonotonicTimeMillis();
                    }
                }
            }
        }

        public ACM getACM()
        {
            lock(this)
            {
                return _monitor != null ? _monitor.getACM() : new ACM(0, ACMClose.CloseOff, ACMHeartbeat.HeartbeatOff);
            }
        }

        public void requestTimedOut(IceInternal.OutgoingMessageCallback @out)
        {
            lock(this)
            {
                LinkedListNode<OutgoingMessage> p;
                for(p = _sendStreams.First; p != null; p = p.Next)
                {
                    OutgoingMessage o = p.Value;
                    if(o.@out == @out)
                    {
                        if(o.requestId > 0)
                        {
                            _requests.Remove(o.requestId);
                        }
                
                        //
                        // If the request is being sent, don't remove it from the send streams, 
                        // it will be removed once the sending is finished.
                        //
                        o.timedOut();
                        if(p != _sendStreams.First)
                        {
                            _sendStreams.Remove(p);
                        }
                        @out.finished(new InvocationTimeoutException());
                        return; // We're done.
                    }
                }

                if(@out is IceInternal.Outgoing)
                {
                    IceInternal.Outgoing o = (IceInternal.Outgoing)@out;
                    foreach(KeyValuePair<int, IceInternal.Outgoing> kvp in _requests)
                    {
                        if(kvp.Value == o)
                        {
                            o.finished(new InvocationTimeoutException());
                            _requests.Remove(kvp.Key);
                            return; // We're done.
                        }
                    }
                }
            }
        }

        public void asyncRequestTimedOut(IceInternal.OutgoingAsyncMessageCallback outAsync)
        {
            lock(this)
            {
                LinkedListNode<OutgoingMessage> p;
                for(p = _sendStreams.First; p != null; p = p.Next)
                {
                    OutgoingMessage o = p.Value;
                    if(o.outAsync == outAsync)
                    {
                        if(o.requestId > 0)
                        {
                            _asyncRequests.Remove(o.requestId);
                        }
                
                        //
                        // If the request is being sent, don't remove it from the send streams, 
                        // it will be removed once the sending is finished.
                        //
                        o.timedOut();
                        if(o != _sendStreams.First.Value)
                        {
                            _sendStreams.Remove(p);
                        }
                        outAsync.dispatchInvocationTimeout__(_threadPool, this);
                        return; // We're done.
                    }
                }

                if(outAsync is IceInternal.OutgoingAsync)
                {
                    IceInternal.OutgoingAsync o = (IceInternal.OutgoingAsync)outAsync;
                    foreach(KeyValuePair<int, IceInternal.OutgoingAsync> kvp in _asyncRequests)
                    {
                        if(kvp.Value == o)
                        {
                            _asyncRequests.Remove(kvp.Key);
                            outAsync.dispatchInvocationTimeout__(_threadPool, this);
                            return; // We're done.
                        }
                    }
                }
            }
        }

        public void sendResponse(int requestId, IceInternal.BasicStream os, byte compressFlag)
        {
            lock(this)
            {
                Debug.Assert(_state > StateNotValidated);

                try
                {
                    if(--_dispatchCount == 0)
                    {
                        if(_state == StateFinished)
                        {
                            reap();
                        }
                        System.Threading.Monitor.PulseAll(this);
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
                        if(_state == StateFinished)
                        {
                            reap();
                        }
                        System.Threading.Monitor.PulseAll(this);
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
        }

        public bool systemException(int requestId, Ice.SystemException ex)
        {
            return false; // System exceptions aren't marshalled.
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
            lock(this)
            {
                if(_state <= StateNotValidated || _state >= StateClosing)
                {
                    return;
                }

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
                        observerStartWrite(_writeStream.getBuffer());
                    }

                    bool completed;
                    completedSynchronously = _transceiver.startWrite(_writeStream.getBuffer(), cb, this, out completed);
                    if(completed && _sendStreams.Count > 0)
                    {
                        // The whole message is written, assume it's sent now for at-most-once semantics.
                        _sendStreams.First.Value.isSent = true;
                    }
                }
                else if((operation & IceInternal.SocketOperation.Read) != 0)
                {
                    if(_observer != null && !_readHeader)
                    {
                        observerStartRead(_readStream.getBuffer());
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
                        observerFinishWrite(_writeStream.getBuffer());
                    }
                }
                else if((operation & IceInternal.SocketOperation.Read) != 0)
                {
                    _transceiver.finishRead(_readStream.getBuffer());
                    if(_observer != null && !_readHeader)
                    {
                        observerFinishRead(_readStream.getBuffer());
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

            IceInternal.ThreadPoolMessage msg = new IceInternal.ThreadPoolMessage(this);
            lock(this)
            {
                if(!msg.startIOScope(ref current))
                {
                    return;
                }

                if(_state >= StateClosed)
                {
                    return;
                }

                int readyOp = current.operation;
                try
                {
                    unscheduleTimeout(current.operation);

                    int writeOp = IceInternal.SocketOperation.None;
                    int readOp = IceInternal.SocketOperation.None;
                    if((readyOp & IceInternal.SocketOperation.Write) != 0)
                    {
                        if(_observer != null)
                        {
                            observerStartWrite(_writeStream.getBuffer());
                        }
                        writeOp = _transceiver.write(_writeStream.getBuffer());
                        if(_observer != null && (writeOp & IceInternal.SocketOperation.Write) == 0)
                        {
                            observerFinishWrite(_writeStream.getBuffer());
                        }
                    }

                    while((readyOp & IceInternal.SocketOperation.Read) != 0)
                    {
                        IceInternal.Buffer buf = _readStream.getBuffer();

                        if(_observer != null && !_readHeader)
                        {
                            observerStartRead(buf);
                        }

                        readOp = _transceiver.read(buf, ref _hasMoreData);
                        if((readOp & IceInternal.SocketOperation.Read) != 0)
                        {
                            break;
                        }
                        if(_observer != null && !_readHeader)
                        {
                            Debug.Assert(!buf.b.hasRemaining());
                            observerFinishRead(buf);
                        }

                        if(_readHeader) // Read header if necessary.
                        {
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

                        if(buf.b.hasRemaining())
                        {
                            if(_endpoint.datagram())
                            {
                                throw new Ice.DatagramLimitException(); // The message was truncated.
                            }
                            continue;
                        }
                        break;
                    }

                    int newOp = readOp | writeOp;
                    readyOp &= ~newOp;
                    Debug.Assert(readyOp != 0 || newOp != 0);

                    if(_state <= StateNotValidated)
                    {
                        if(newOp != 0)
                        {
                            //
                            // Wait for all the transceiver conditions to be
                            // satisfied before continuing.
                            //
                            scheduleTimeout(newOp);
                            _threadPool.update(this, current.operation, newOp);
                            return;
                        }

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
                        if(_startCallback != null)
                        {
                            startCB = _startCallback;
                            _startCallback = null;
                            if(startCB != null)
                            {
                                ++_dispatchCount;
                            }
                        }
                    }
                    else
                    {
                        Debug.Assert(_state <= StateClosingPending);

                        //
                        // We parse messages first, if we receive a close
                        // connection message we won't send more messages.
                        // 
                        if((readyOp & IceInternal.SocketOperation.Read) != 0)
                        {
                            newOp |= parseMessage(ref info);
                        }

                        if((readyOp & IceInternal.SocketOperation.Write) != 0)
                        {
                            sentCBs = new Queue<OutgoingMessage>();
                            newOp |= sendNextMessage(sentCBs);
                            if(sentCBs.Count > 0)
                            {
                                ++_dispatchCount;
                            }
                            else
                            {
                                sentCBs = null;
                            }
                        }

                        if(_state < StateClosed)
                        {
                            scheduleTimeout(newOp);
                            _threadPool.update(this, current.operation, newOp);
                        }

                        if(readyOp == 0)
                        {
                            return;
                        }

                        msg.completed(ref current);
                    }
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

                if(_acmLastActivity > 0)
                {
                    _acmLastActivity = IceInternal.Time.currentMonotonicTimeMillis();
                }

                //
                // Unlike C++/Java, this method is called from an IO thread of the .NET thread
                // pool or from the communicator async IO thread. While it's fine to handle the
                // non-blocking activity of the connection from these threads, the dispatching
                // of the message must be taken care of by the Ice thread pool.
                //
                IceInternal.ThreadPoolCurrent c = current;
                _threadPool.dispatch(() =>
                    {
                        dispatch(startCB, sentCBs, info);
                        msg.destroy(ref c);
                    }, this);
            }
        }

        private void dispatch(StartCallback startCB, Queue<OutgoingMessage> sentCBs, MessageInfo info)
        {
            int count = 0;

            //
            // Notify the factory that the connection establishment and
            // validation has completed.
            //
            if(startCB != null)
            {
                startCB.connectionStartCompleted(this);
                ++count;
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
                        m.outAsync.invokeSent__(m.sentCallback);
                    }
                    if(m.receivedReply)
                    {
                        ((IceInternal.OutgoingAsync)m.outAsync).finished__();
                    }
                }
                ++count;
            }

            //
            // Asynchronous replies must be handled outside the thread
            // synchronization, so that nested calls are possible.
            //
            if(info.outAsync != null)
            {
                info.outAsync.finished__();
                ++count;
            }

            if(info.heartbeatCallback != null)
            {
                try
                {
                    info.heartbeatCallback.heartbeat(this);
                }
                catch(System.Exception ex)
                {
                    _logger.error("connection callback exception:\n" + ex + '\n' + _desc);
                }
                ++count;
            }

            //
            // Method invocation (or multiple invocations for batch messages)
            // must be done outside the thread synchronization, so that nested
            // calls are possible.
            //
            if(info.invokeNum > 0)
            {
                invokeAll(info.stream, info.invokeNum, info.requestId, info.compress, info.servantManager,
                          info.adapter);

                //
                // Don't increase count, the dispatch count is
                // decreased when the incoming reply is sent.
                //
            }

            //
            // Decrease dispatch count.
            //
            if(count > 0)
            {
                lock(this)
                {
                    _dispatchCount -= count;
                    if(_dispatchCount == 0)
                    {
                        //
                        // Only initiate shutdown if not already done. It
                        // might have already been done if the sent callback
                        // or AMI callback was dispatched when the connection
                        // was already in the closing state.
                        //
                        if(_state == StateClosing)
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
                            reap();
                        }
                        System.Threading.Monitor.PulseAll(this);
                    }
                }
            }
        }

        public override void finished(ref IceInternal.ThreadPoolCurrent current)
        {
            lock(this)
            {
                Debug.Assert(_state == StateClosed);
                unscheduleTimeout(IceInternal.SocketOperation.Read | IceInternal.SocketOperation.Write);
            }

            //
            // If there are no callbacks to call, we don't call ioCompleted() since we're not going
            // to call code that will potentially block (this avoids promoting a new leader and
            // unecessary thread creation, especially if this is called on shutdown).
            //
            if(_startCallback == null && _sendStreams.Count == 0 && _asyncRequests.Count == 0 && _callback == null)
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
            _threadPool.dispatch(finish, this);
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
                    OutgoingMessage message = _sendStreams.First.Value;
                    _writeStream.swap(message.stream);

                    //
                    // The current message might be sent but not yet removed from _sendStreams. If
                    // the response has been received in the meantime, we remove the message from
                    // _sendStreams to not call finished on a message which is already done.
                    //
                    if(message.isSent || message.receivedReply)
                    {
                        if(message.sent() && message.sentCallback != null)
                        {
                            message.outAsync.invokeSent__(message.sentCallback);
                        }
                        if(message.receivedReply)
                        {
                            ((IceInternal.OutgoingAsync)message.outAsync).finished__();
                        }
                        _sendStreams.RemoveFirst();
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
                o.finished(_exception);
            }
            _requests.Clear();

            foreach(IceInternal.OutgoingAsync o in _asyncRequests.Values)
            {
                o.finished__(_exception);
            }
            _asyncRequests.Clear();

            if(_callback != null)
            {
                try
                {
                    _callback.closed(this);
                }
                catch(System.Exception ex)
                {
                    _logger.error("connection callback exception:\n" + ex + '\n' + _desc);
                }
                _callback = null;
            }

            //
            // This must be done last as this will cause waitUntilFinished() to return (and communicator
            // objects such as the timer might be destroyed too).
            //
            lock(this)
            {
                setState(StateFinished);
                if(_dispatchCount == 0)
                {
                    reap();
                }
            }
        }

        public override string ToString()
        {
            return _desc; // No mutex lock, _desc is immutable.
        }

        public void timedOut()
        {
            lock(this)
            {
                if(_state <= StateNotValidated)
                {
                    setState(StateClosed, new ConnectTimeoutException());
                }
                else if(_state < StateClosing)
                {
                    setState(StateClosed, new TimeoutException());
                }
                else if(_state < StateClosed)
                {
                    setState(StateClosed, new CloseTimeoutException());
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

        public ConnectionInfo getInfo()
        {
            lock(this)
            {
                if(_state >= StateClosed)
                {
                    throw _exception;
                }
                return initConnectionInfo();
            }
        }

        public string ice_toString_()
        {
            return ToString();
        }

        public void exception(LocalException ex)
        {
            lock(this)
            {
                setState(StateClosed, ex);
            }
        }

        public void invokeException(int requestId, LocalException ex, int invokeNum)
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
                    Debug.Assert(_dispatchCount >= invokeNum);
                    _dispatchCount -= invokeNum;
                    if(_dispatchCount == 0)
                    {
                        if(_state == StateFinished)
                        {
                            reap();
                        }
                        System.Threading.Monitor.PulseAll(this);
                    }
                }
            }
        }

        static ConnectionI()
        {
            _compressionSupported = IceInternal.BasicStream.compressible();
        }

        internal ConnectionI(Communicator communicator, IceInternal.Instance instance,
                             IceInternal.ACMMonitor monitor, IceInternal.Transceiver transceiver,
                             IceInternal.Connector connector, IceInternal.EndpointI endpoint, ObjectAdapter adapter)
        {
            _communicator = communicator;
            _instance = instance;
            _monitor = monitor;
            _transceiver = transceiver;
            _desc = transceiver.ToString();
            _type = transceiver.protocol();
            _connector = connector;
            _endpoint = endpoint;
            _adapter = adapter;
            InitializationData initData = instance.initializationData();
            _logger = initData.logger; // Cached for better performance.
            _traceLevels = instance.traceLevels(); // Cached for better performance.
            _timer = instance.timer();
            _writeTimeout = new TimeoutCallback(this);
            _writeTimeoutScheduled = false;
            _readTimeout = new TimeoutCallback(this);
            _readTimeoutScheduled = false;
            _warn = initData.properties.getPropertyAsInt("Ice.Warn.Connections") > 0;
            _warnUdp = initData.properties.getPropertyAsInt("Ice.Warn.Datagrams") > 0;
            _cacheBuffers = instance.cacheMessageBuffers() > 0;
            if(_monitor != null && _monitor.getACM().timeout > 0)
            {
                _acmLastActivity = IceInternal.Time.currentMonotonicTimeMillis();
            }
            else
            {
                _acmLastActivity = -1;
            }
            _nextRequestId = 1;
            _batchAutoFlush = initData.properties.getPropertyAsIntWithDefault("Ice.BatchAutoFlush", 1) > 0;
            _batchStream = new IceInternal.BasicStream(instance, Util.currentProtocolEncoding, _batchAutoFlush);
            _batchStreamInUse = false;
            _batchRequestNum = 0;
            _batchRequestCompress = false;
            _batchMarker = 0;
            _readStream = new IceInternal.BasicStream(instance, Util.currentProtocolEncoding);
            _readHeader = false;
            _readStreamPos = -1;
            _writeStream = new IceInternal.BasicStream(instance, Util.currentProtocolEncoding);
            _writeStreamPos = -1;
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
        private const int StateClosingPending = 5;
        private const int StateClosed = 6;
        private const int StateFinished = 7;

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
                //
                // If we are in closed state, an exception must be set.
                //
                Debug.Assert(_state != StateClosed);

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
                         (_exception is ConnectionLostException && _state >= StateClosing)))
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
                case StateClosingPending:
                {
                    //
                    // Can't change back from closing pending.
                    //
                    if(_state >= StateClosingPending)
                    {
                        return;
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
                    _transceiver.destroy();
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
            if(_monitor != null)
            {
                if(state == StateActive)
                {
                    _monitor.add(this);
                    if(_acmLastActivity > 0)
                    {
                        _acmLastActivity = IceInternal.Time.currentMonotonicTimeMillis();
                    }
                }
                else if(_state == StateActive)
                {
                    _monitor.remove(this);
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
                         (_exception is ConnectionLostException && _state >= StateClosing)))
                    {
                        _observer.failed(_exception.ice_name());
                    }
                }
            }
            _state = state;

            System.Threading.Monitor.PulseAll(this);

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

            if(_shutdownInitiated)
            {
                return;
            }
            _shutdownInitiated = true;

            if(!_endpoint.datagram())
            {
                //
                // Before we shut down, we send a close connection message.
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
                    setState(StateClosingPending);

                    //
                    // Notify the the transceiver of the graceful connection closure.
                    //
                    int op = _transceiver.closing(true, _exception);
                    if(op != 0)
                    {
                        scheduleTimeout(op);
                        _threadPool.register(this, op);
                    }
                }
            }
        }

        private void heartbeat()
        {
            Debug.Assert(_state == StateActive);
            
            if(!_endpoint.datagram())
            {
                IceInternal.BasicStream os = new IceInternal.BasicStream(_instance, Util.currentProtocolEncoding);
                os.writeBlob(IceInternal.Protocol.magic);
                Ice.Util.currentProtocol.write__(os);
                Ice.Util.currentProtocolEncoding.write__(os);
                os.writeByte(IceInternal.Protocol.validateConnectionMsg);
                os.writeByte((byte)0);
                os.writeInt(IceInternal.Protocol.headerSize); // Message size.
                try
                {
                    OutgoingMessage message = new OutgoingMessage(os, false, false);
                    sendMessage(message);
                }
                catch(Ice.LocalException ex)
                {
                    setState(StateClosed, ex);
                    Debug.Assert(_exception != null);
                }
            }
        }

        private bool initialize(int operation)
        {
            int s = _transceiver.initialize(_readStream.getBuffer(), _writeStream.getBuffer(), ref _hasMoreData);
            if(s != IceInternal.SocketOperation.None)
            {
                scheduleTimeout(s);
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
                        observerStartWrite(_writeStream.getBuffer());
                    }

                    if(_writeStream.pos() != _writeStream.size())
                    {
                        int op = _transceiver.write(_writeStream.getBuffer());
                        if(op != 0)
                        {
                            scheduleTimeout(op);
                            _threadPool.update(this, operation, op);
                            return false;
                        }
                    }

                    if(_observer != null)
                    {
                        observerFinishWrite(_writeStream.getBuffer());
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
                        observerStartRead(_readStream.getBuffer());
                    }

                    if(_readStream.pos() != _readStream.size())
                    {
                        int op = _transceiver.read(_readStream.getBuffer(), ref _hasMoreData);
                        if(op != 0)
                        {
                            scheduleTimeout(op);
                            _threadPool.update(this, operation, op);
                            return false;
                        }
                    }

                    if(_observer != null)
                    {
                        observerFinishRead(_readStream.getBuffer());
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

        private int sendNextMessage(Queue<OutgoingMessage> callbacks)
        {
            if(_sendStreams.Count == 0)
            {
                return IceInternal.SocketOperation.None;
            }
            else if(_state == StateClosingPending && _writeStream.pos() == 0)
            {
                // Message wasn't sent, empty the _writeStream, we're not going to send more data.
                OutgoingMessage message = _sendStreams.First.Value;
                _writeStream.swap(message.stream);
                return IceInternal.SocketOperation.None;
            }

            Debug.Assert(!_writeStream.isEmpty() && _writeStream.pos() == _writeStream.size());
            try
            {
                while(true)
                {
                    //
                    // Notify the message that it was sent.
                    //
                    OutgoingMessage message = _sendStreams.First.Value;
                    _writeStream.swap(message.stream);
                    if(message.sent())
                    {
                        callbacks.Enqueue(message);
                    }
                    _sendStreams.RemoveFirst();

                    //
                    // If there's nothing left to send, we're done.
                    //
                    if(_sendStreams.Count == 0)
                    {
                        break;
                    }

                    //
                    // If we are in the closed state or if the close is
                    // pending, don't continue sending.
                    //
                    // This can occur if parseMessage (called before
                    // sendNextMessage by message()) closes the connection.
                    //
                    if(_state >= StateClosingPending)
                    {
                        return IceInternal.SocketOperation.None;
                    }

                    //
                    // Otherwise, prepare the next message stream for writing.
                    //
                    message = _sendStreams.First.Value;
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
                        observerStartWrite(_writeStream.getBuffer());
                    }
                    if(_writeStream.pos() != _writeStream.size())
                    {
                        int op = _transceiver.write(_writeStream.getBuffer());
                        if(op != 0)
                        {
                            return op;
                        }
                    }
                    if(_observer != null)
                    {
                        observerFinishWrite(_writeStream.getBuffer());
                    }
                }
            }
            catch(Ice.LocalException ex)
            {
                setState(StateClosed, ex);
                return IceInternal.SocketOperation.None;
            }

            //
            // If all the messages were sent and we are in the closing state, we schedule
            // the close timeout to wait for the peer to close the connection.
            //
            if(_state == StateClosing && _dispatchCount == 0)
            {
                setState(StateClosingPending);
                int op = _transceiver.closing(true, _exception);
                if(op != 0)
                {
                    return op;
                }
            }

            return IceInternal.SocketOperation.None;
        }

        private bool sendMessage(OutgoingMessage message)
        {
            Debug.Assert(_state < StateClosed);

            if(_sendStreams.Count > 0)
            {
                message.adopt();
                _sendStreams.AddLast(message);
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

            //
            // Send the message without blocking.
            //
            if(_observer != null)
            {
                observerStartWrite(message.stream.getBuffer());
            }
            int op = _transceiver.write(message.stream.getBuffer());
            if(op == 0)
            {
                if(_observer != null)
                {
                    observerFinishWrite(message.stream.getBuffer());
                }

                message.sent();

                if(_acmLastActivity > 0)
                {
                    _acmLastActivity = IceInternal.Time.currentMonotonicTimeMillis();
                }
                return true;
            }

            message.adopt();

            _writeStream.swap(message.stream);
            _sendStreams.AddLast(message);
            scheduleTimeout(op);
            _threadPool.register(this, op);
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
            public ConnectionCallback heartbeatCallback;
        }

        private int parseMessage(ref MessageInfo info)
        {
            Debug.Assert(_state > StateNotValidated && _state < StateClosed);

            info.stream = new IceInternal.BasicStream(_instance, Util.currentProtocolEncoding);
            _readStream.swap(info.stream);
            _readStream.resize(IceInternal.Protocol.headerSize, true);
            _readStream.pos(0);
            _readHeader = true;

            Debug.Assert(info.stream.pos() == info.stream.size());

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
                            setState(StateClosingPending, new CloseConnectionException());

                            //
                            // Notify the the transceiver of the graceful connection closure.
                            //
                            int op = _transceiver.closing(false, _exception);
                            if(op != 0)
                            {
                                return op;
                            }
                            setState(StateClosed);
                        }
                        break;
                    }

                    case IceInternal.Protocol.requestMsg:
                    {
                        if(_state >= StateClosing)
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
                        if(_state >= StateClosing)
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
                            System.Threading.Monitor.PulseAll(this); // Notify threads blocked in close(false)
                        }
                        else if(_asyncRequests.TryGetValue(info.requestId, out info.outAsync))
                        {
                            _asyncRequests.Remove(info.requestId);

                            info.outAsync.istr__.swap(info.stream);

                            //
                            // If we just received the reply for a request which isn't acknowledge as 
                            // sent yet, we queue the reply instead of processing it right away. It 
                            // will be processed once the write callback is invoked for the message.
                            //
                            OutgoingMessage message = _sendStreams.Count > 0 ? _sendStreams.First.Value : null;
                            if(message != null && message.outAsync == info.outAsync)
                            {
                                message.receivedReply = true;
                                info.outAsync = null;
                            }
                            else
                            {
                                ++_dispatchCount;
                            }
                            System.Threading.Monitor.PulseAll(this); // Notify threads blocked in close(false)
                        }
                        break;
                    }

                    case IceInternal.Protocol.validateConnectionMsg:
                    {
                        IceInternal.TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                        if(_callback != null)
                        {
                            info.heartbeatCallback = _callback;
                            ++_dispatchCount;
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

            return _state == StateHolding ? IceInternal.SocketOperation.None : IceInternal.SocketOperation.Read;
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
                    Debug.Assert(!response || invokeNum == 1);

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
                invokeException(requestId, ex, invokeNum);
            }
            finally
            {
                if(inc != null)
                {
                    reclaimIncoming(inc);
                }
            }
        }

        private void scheduleTimeout(int status)
        {
            int timeout;
            if(_state < StateActive)
            {
                IceInternal.DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
                if(defaultsAndOverrides.overrideConnectTimeout)
                {
                    timeout = defaultsAndOverrides.overrideConnectTimeoutValue;
                }
                else
                {
                    timeout = _endpoint.timeout();
                }
            }
            else if(_state < StateClosingPending)
            {
                if(_readHeader) // No timeout for reading the header.
                {
                    status &= ~IceInternal.SocketOperation.Read;
                }
                timeout = _endpoint.timeout();
            }
            else
            {
                IceInternal.DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
                if(defaultsAndOverrides.overrideCloseTimeout)
                {
                    timeout = defaultsAndOverrides.overrideCloseTimeoutValue;
                }
                else
                {
                    timeout = _endpoint.timeout();
                }
            }

            if(timeout < 0)
            {
                return;
            }

            if((status & IceInternal.SocketOperation.Read) != 0)
            {
                if(_readTimeoutScheduled)
                {
                    _timer.cancel(_readTimeout);
                }
                _timer.schedule(_readTimeout, timeout);
                _readTimeoutScheduled = true;
            }
            if((status & (IceInternal.SocketOperation.Write | IceInternal.SocketOperation.Connect)) != 0)
            {
                if(_writeTimeoutScheduled)
                {
                    _timer.cancel(_writeTimeout);
                }
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

        private void reap()
        {
            if(_monitor != null)
            {
                _monitor.reap(this);
            }
            if(_observer != null)
            {
                _observer.detach();
            }
        }
        
        ConnectionState toConnectionState(int state)
        {
            return connectionStateMap[state];
        }

        private void warning(string msg, System.Exception ex)
        {
            _logger.warning(msg + ":\n" + ex + "\n" + _transceiver.ToString());
        }

        private void observerStartRead(IceInternal.Buffer buf)
        {
            if(_readStreamPos >= 0)
            {
                Debug.Assert(!buf.empty());
                _observer.receivedBytes(buf.b.position() - _readStreamPos);
            }
            _readStreamPos = buf.empty() ? -1 : buf.b.position();
        }

        private void observerFinishRead(IceInternal.Buffer buf)
        {
            if(_readStreamPos == -1)
            {
                return;
            }
            Debug.Assert(buf.b.position() >= _readStreamPos);
            _observer.receivedBytes(buf.b.position() - _readStreamPos);
            _readStreamPos = -1;
        }

        private void observerStartWrite(IceInternal.Buffer buf)
        {
            if(_writeStreamPos >= 0)
            {
                Debug.Assert(!buf.empty());
                _observer.sentBytes(buf.b.position() - _writeStreamPos);
            }
            _writeStreamPos = buf.empty() ? -1 : buf.b.position();
        }

        private void observerFinishWrite(IceInternal.Buffer buf)
        {
            if(_writeStreamPos == -1)
            {
                return;
            }
            if(buf.b.position() > _writeStreamPos)
            {
                _observer.sentBytes(buf.b.position() - _writeStreamPos);
            }
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
                        inc = new IceInternal.Incoming(_instance, this, this, adapter, response, compress, requestId);
                    }
                    else
                    {
                        inc = _incomingCache;
                        _incomingCache = _incomingCache.next;
                        inc.reset(_instance, this, this, adapter, response, compress, requestId);
                        inc.next = null;
                    }
                }
            }
            else
            {
                inc = new IceInternal.Incoming(_instance, this, this, adapter, response, compress, requestId);
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

            internal void timedOut()
            {
                Debug.Assert((@out != null || outAsync != null));
                @out = null;
                outAsync = null;
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

            internal bool sent()
            {
                if(@out != null)
                {
                    @out.sent();
                }
                else if(outAsync != null)
                {
                    sentCallback = outAsync.sent__();
                }
                return sentCallback != null || receivedReply;
            }

            internal void finished(LocalException ex)
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

            internal IceInternal.BasicStream stream;
            internal IceInternal.OutgoingMessageCallback @out;
            internal IceInternal.OutgoingAsyncMessageCallback outAsync;
            internal bool receivedReply;
            internal bool compress;
            internal int requestId;
            internal bool _adopt;
            internal bool prepared;
            internal bool isSent;
            internal Ice.AsyncCallback sentCallback = null;
        }

        private Communicator _communicator;
        private IceInternal.Instance _instance;
        private IceInternal.ACMMonitor _monitor;
        private IceInternal.Transceiver _transceiver;
        private string _desc;
        private string _type;
        private IceInternal.Connector _connector;
        private IceInternal.EndpointI _endpoint;

        private ObjectAdapter _adapter;
        private IceInternal.ServantManager _servantManager;

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

        private long _acmLastActivity;

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

        private static bool _compressionSupported;

        private bool _cacheBuffers;

        private Ice.ConnectionInfo _info;

        private Ice.ConnectionCallback _callback;

        private static ConnectionState[] connectionStateMap = new ConnectionState[] {
            ConnectionState.ConnectionStateValidating,   // StateNotInitialized
            ConnectionState.ConnectionStateValidating,   // StateNotValidated
            ConnectionState.ConnectionStateActive,       // StateActive
            ConnectionState.ConnectionStateHolding,      // StateHolding
            ConnectionState.ConnectionStateClosing,      // StateClosing
            ConnectionState.ConnectionStateClosing,      // StateClosingPending
            ConnectionState.ConnectionStateClosed,       // StateClosed
            ConnectionState.ConnectionStateClosed,       // StateFinished
        };
    }
}
