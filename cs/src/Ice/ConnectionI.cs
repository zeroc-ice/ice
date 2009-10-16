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
                            Monitor.Wait(this);
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
                if(_state != StateFinished || _dispatchCount != 0)
                {
                    return false;
                }

                Debug.Assert(_state == StateFinished);
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
                // We wait indefinitely until the connection is finished and all
                // outstanding requests are completed. Otherwise we couldn't
                // guarantee that there are no outstanding calls when deactivate()
                // is called on the servant locators.
                //
                while(_state < StateFinished || _dispatchCount > 0)
                {
                    Monitor.Wait(this);
                }

                Debug.Assert(_state == StateFinished && _dispatchCount == 0);

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
                        if(_state == StateFinished)
                        {
                            _reaper.add(this);
                        }
                        Monitor.PulseAll(this);
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
                            _reaper.add(this);
                        }
                        Monitor.PulseAll(this);
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

        //
        // Operations from EventHandler
        //
        public override bool startAsync(int operation, AsyncCallback callback, ref bool completedSynchronously)
        {
            if(_state >= StateClosed)
            {
                return false;
            }

            try
            {
                if((operation & IceInternal.SocketOperation.Write) != 0)
                {
                    completedSynchronously = _transceiver.startWrite(_writeStream.getBuffer(), callback, this);
                }
                else if((operation & IceInternal.SocketOperation.Read) != 0)
                {
                    completedSynchronously = _transceiver.startRead(_readStream.getBuffer(), callback, this);
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
                }
                else if((operation & IceInternal.SocketOperation.Read) != 0)
                {
                    _transceiver.finishRead(_readStream.getBuffer());
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
                
                try
                {
                    unscheduleTimeout(current.operation);
                    if((current.operation & IceInternal.SocketOperation.Write) != 0 && !_writeStream.isEmpty())
                    {
                        if(_writeStream.getBuffer().b.hasRemaining() && !_transceiver.write(_writeStream.getBuffer()))
                        {
                            Debug.Assert(!_writeStream.isEmpty());
                            scheduleTimeout(IceInternal.SocketOperation.Write, _endpoint.timeout());
                            return;
                        }
                        Debug.Assert(!_writeStream.getBuffer().b.hasRemaining());
                    }
                    if((current.operation & IceInternal.SocketOperation.Read) != 0 && !_readStream.isEmpty())
                    {
                        if(_readStream.size() == IceInternal.Protocol.headerSize) // Read header.
                        {
                            if(_readStream.getBuffer().b.hasRemaining() && !_transceiver.read(_readStream.getBuffer()))
                            {
                                return;
                            }
                            Debug.Assert(!_readStream.getBuffer().b.hasRemaining());

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
                            if(m[0] != IceInternal.Protocol.magic[0] || m[1] != IceInternal.Protocol.magic[1]
                               || m[2] != IceInternal.Protocol.magic[2] || m[3] != IceInternal.Protocol.magic[3])
                            {
                                Ice.BadMagicException ex = new Ice.BadMagicException();
                                ex.badMagic = m;
                                throw ex;
                            }

                            byte pMajor = _readStream.readByte();
                            byte pMinor = _readStream.readByte();
                            if(pMajor != IceInternal.Protocol.protocolMajor ||
                               pMinor > IceInternal.Protocol.protocolMinor)
                            {
                                Ice.UnsupportedProtocolException e = new Ice.UnsupportedProtocolException();
                                e.badMajor = pMajor < 0 ? pMajor + 255 : pMajor;
                                e.badMinor = pMinor < 0 ? pMinor + 255 : pMinor;
                                e.major = IceInternal.Protocol.protocolMajor;
                                e.minor = IceInternal.Protocol.protocolMinor;
                                throw e;
                            }

                            byte eMajor = _readStream.readByte();
                            byte eMinor = _readStream.readByte();
                            if(eMajor != IceInternal.Protocol.encodingMajor || 
                               eMinor > IceInternal.Protocol.encodingMinor)
                            {
                                Ice.UnsupportedEncodingException e = new Ice.UnsupportedEncodingException();
                                e.badMajor = eMajor < 0 ? eMajor + 255 : eMajor;
                                e.badMinor = eMinor < 0 ? eMinor + 255 : eMinor;
                                e.major = IceInternal.Protocol.encodingMajor;
                                e.minor = IceInternal.Protocol.encodingMinor;
                                throw e;
                            }

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

                        if(_readStream.pos() != _readStream.size())
                        {
                            if(_endpoint.datagram())
                            {
                                if(_warnUdp)
                                {
                                    _logger.warning("DatagramLimitException: maximum size of " + 
                                                    _readStream.pos() + " exceeded");
                                }
                                throw new Ice.DatagramLimitException();
                            }
                            else
                            {
                                if(_readStream.getBuffer().b.hasRemaining() && 
                                   !_transceiver.read(_readStream.getBuffer()))
                                {
                                    Debug.Assert(!_readStream.isEmpty());
                                    scheduleTimeout(IceInternal.SocketOperation.Read, _endpoint.timeout());
                                    return;
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
                    
                        if((current.operation & IceInternal.SocketOperation.Write) != 0)
                        {
                            sentCBs = sendNextMessage();
                        }
                    
                        if((current.operation & IceInternal.SocketOperation.Read) != 0)
                        {
                            parseMessage(new IceInternal.BasicStream(_instance), ref info);
                        }
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
                    _readStream.resize(IceInternal.Protocol.headerSize, true);
                    _readStream.pos(0);
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
                // pool of from the communicator async IO thread. While it's fine to handle the 
                // non-blocking activity of the connection from these threads, the dispatching 
                // of the message must be taken care of by the Ice thread pool.
                //
                IceInternal.ThreadPoolCurrent c = current;
                _threadPool.execute(
                    delegate()
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
                                m.outAsync.sent__(_instance);
                            }
                        }
                        
                        //
                        // Asynchronous replies must be handled outside the thread
                        // synchronization, so that nested calls are possible.
                        //
                        if(info.outAsync != null)
                        {
                            info.outAsync.finished__(info.stream);
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
        
                        msg.destroy(ref c);
                    });
            }
        }

        public override void finished(ref IceInternal.ThreadPoolCurrent current)
        {
            lock(this)
            {
                Debug.Assert(_state == StateClosed);
                unscheduleTimeout(IceInternal.SocketOperation.Read | IceInternal.SocketOperation.Write);
            }

            foreach(IceInternal.Outgoing o in _requests.Values)
            {
                o.finished(_exception);
            }
            _requests.Clear();

            //
            // If there are no callbacks to call, we don't call ioCompleted() since we're not going
            // to call code that will potentially block (this avoids promoting a new leader and 
            // unecessary thread creation, especially if this is called on shutdown).
            //
            if(_startCallback != null || _sendStreams.Count > 0 || _asyncRequests.Count > 0)
            {
                //
                // Unlike C++/Java, this method is called from an IO thread of the .NET thread
                // pool of from the communicator async IO thread. While it's fine to handle the 
                // non-blocking activity of the connection from these threads, the dispatching 
                // of the message must be taken care of by the Ice thread pool.
                //
                _threadPool.execute(
                    delegate()
                    {
                        if(_startCallback != null)
                        {
                            _startCallback.connectionStartFailed(this, _exception);
                            _startCallback = null;
                        }
                        
                        if(_sendStreams.Count > 0)
                        {
                            Debug.Assert(!_writeStream.isEmpty());

                            //
                            // The current message might be sent but not yet removed from _sendStreams if the 
                            // connection was closed shortly after. We check if that's the case here and mark
                            // the message as sent if necessary.
                            //
                            OutgoingMessage message = _sendStreams.Peek();
                            _writeStream.swap(message.stream);
                            if(!message.stream.getBuffer().b.hasRemaining())
                            {
                                message.sent(this, true);
                                if(message.outAsync is Ice.AMISentCallback)
                                {
                                    message.outAsync.sent__(_instance);
                                }
                                _sendStreams.Dequeue();
                            }

                            foreach(OutgoingMessage m in _sendStreams)
                            {
                                m.finished(_exception);
                            }
                            _sendStreams.Clear();
                        }

                        foreach(IceInternal.OutgoingAsync o in _asyncRequests.Values)
                        {
                            o.finished__(_exception);
                        }
                        _asyncRequests.Clear();
                        
                        lock(this)
                        {
                            setState(StateFinished);
                            if(_dispatchCount == 0)
                            {
                                _reaper.add(this);
                            }
                        }
                    });
            }
            else
            {
                //
                // This must be done last as this will cause waitUntilFinished() to return (and communicator
                // objects such as the timer might be destroyed too).
                //
                lock(this)
                {
                    setState(StateFinished);
                    if(_dispatchCount == 0)
                    {
                        _reaper.add(this);
                    }
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
                else if(_state == StateClosing)
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

        public ConnectionInfo
        getInfo()
        {
            lock(this)
            {
                if(_state >= StateClosed)
                {
                    throw _exception;
                }
                return _transceiver.getInfo();
            }
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
                        if(_state == StateFinished)
                        {
                            _reaper.add(this);
                        }
                        Monitor.PulseAll(this);
                    }
                }
            }
        }

        static ConnectionI()
        {
            _compressionSupported = IceInternal.BasicStream.compressible();
        }

        internal ConnectionI(IceInternal.Instance instance, IceInternal.ConnectionReaper reaper, 
                             IceInternal.Transceiver transceiver, IceInternal.Connector connector,
                             IceInternal.EndpointI endpoint, ObjectAdapter adapter)
        {
            _instance = instance;
            _reaper = reaper;
            InitializationData initData = instance.initializationData();
            _transceiver = transceiver;
            _desc = transceiver.ToString();
            _type = transceiver.type();
            _connector = connector;
            _endpoint = endpoint;
            _adapter = adapter;
            _logger = initData.logger; // Cached for better performance.
            _traceLevels = instance.traceLevels(); // Cached for better performance.
            _timer = instance.timer();
            _writeTimeout = new TimeoutCallback(this);
            _writeTimeoutScheduled = false;
            _readTimeout = new TimeoutCallback(this);
            _readTimeoutScheduled = false;
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
            _readStream = new IceInternal.BasicStream(instance);
            _writeStream = new IceInternal.BasicStream(instance);
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

            _state = state;

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
                        _writeStream.writeByte(IceInternal.Protocol.protocolMajor);
                        _writeStream.writeByte(IceInternal.Protocol.protocolMinor);
                        _writeStream.writeByte(IceInternal.Protocol.encodingMajor);
                        _writeStream.writeByte(IceInternal.Protocol.encodingMinor);
                        _writeStream.writeByte(IceInternal.Protocol.validateConnectionMsg);
                        _writeStream.writeByte((byte)0); // Compression status (always zero for validate connection).
                        _writeStream.writeInt(IceInternal.Protocol.headerSize); // Message size.
                        IceInternal.TraceUtil.traceSend(_writeStream, _logger, _traceLevels);
                        _writeStream.prepareWrite();
                    }

                    if(_writeStream.pos() != _writeStream.size() && !_transceiver.write(_writeStream.getBuffer()))
                    {
                        scheduleTimeout(IceInternal.SocketOperation.Write, connectTimeout());
                        _threadPool.update(this, operation, IceInternal.SocketOperation.Write);
                        return false;
                    }
                }
                else // The client side has the passive role for connection validation.
                {
                    if(_readStream.size() == 0)
                    {
                        _readStream.resize(IceInternal.Protocol.headerSize, true);
                        _readStream.pos(0);
                    }

                    if(_readStream.pos() != _readStream.size() && !_transceiver.read(_readStream.getBuffer()))
                    {
                        scheduleTimeout(IceInternal.SocketOperation.Read, connectTimeout());
                        _threadPool.update(this, operation, IceInternal.SocketOperation.Read);
                        return false;
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
                    byte pMajor = _readStream.readByte();
                    byte pMinor = _readStream.readByte();
                    if(pMajor != IceInternal.Protocol.protocolMajor)
                    {
                        UnsupportedProtocolException e = new UnsupportedProtocolException();
                        e.badMajor = pMajor < 0 ? pMajor + 255 : pMajor;
                        e.badMinor = pMinor < 0 ? pMinor + 255 : pMinor;
                        e.major = IceInternal.Protocol.protocolMajor;
                        e.minor = IceInternal.Protocol.protocolMinor;
                        throw e;
                    }
                    byte eMajor = _readStream.readByte();
                    byte eMinor = _readStream.readByte();
                    if(eMajor != IceInternal.Protocol.encodingMajor)
                    {
                        UnsupportedEncodingException e = new UnsupportedEncodingException();
                        e.badMajor = eMajor < 0 ? eMajor + 255 : eMajor;
                        e.badMinor = eMinor < 0 ? eMinor + 255 : eMinor;
                        e.major = IceInternal.Protocol.encodingMajor;
                        e.minor = IceInternal.Protocol.encodingMinor;
                        throw e;
                    }
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
                }
            }

            _writeStream.resize(0, false);
            _writeStream.pos(0);

            _readStream.resize(IceInternal.Protocol.headerSize, true);
            _readStream.pos(0);

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
                    message.sent(this, true);
                    if(message.outAsync is Ice.AMISentCallback)
                    {
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
                    if(_writeStream.pos() != _writeStream.size() && !_transceiver.write(_writeStream.getBuffer()))
                    {
                        Debug.Assert(!_writeStream.isEmpty());
                        scheduleTimeout(IceInternal.SocketOperation.Write, _endpoint.timeout());
                        return callbacks;
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

            if(_transceiver.write(message.stream.getBuffer()))
            {
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

        private void parseMessage(IceInternal.BasicStream stream, ref MessageInfo info)
        {
            Debug.Assert(_state > StateNotValidated && _state < StateClosed);

            info.stream = stream;
            _readStream.swap(info.stream);
            _readStream.resize(IceInternal.Protocol.headerSize, true);
            _readStream.pos(0);            

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
                        }
                        Monitor.PulseAll(this); // Notify threads blocked in close(false)
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

        private void warning(string msg, System.Exception ex)
        {
            _logger.warning(msg + ":\n" + ex + "\n" + _transceiver.ToString());
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
        private IceInternal.ConnectionReaper _reaper;
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
        private IceInternal.BasicStream _writeStream;

        private int _dispatchCount;

        private int _state; // The current state.

        private IceInternal.Incoming _incomingCache;
        private object _incomingCacheMutex = new object();

        private IceInternal.Outgoing _outgoingCache;
        private object _outgoingCacheMutex = new object();

        private static bool _compressionSupported;

        private bool _cacheBuffers;
    }
}
