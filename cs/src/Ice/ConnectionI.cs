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
    using System.Diagnostics;
    using System.Threading;

    public sealed class ConnectionI : IceInternal.EventHandler, Connection
    {
        public void validate()
        {
            if(!endpoint().datagram()) // Datagram connections are always implicitly validated.
            {
                bool active;

                lock(this)
                {
                    if(_thread != null && _thread != Thread.CurrentThread)
                    {
                        //
                        // In thread per connection mode, this connection's thread
                        // will take care of connection validation. Therefore all we
                        // have to do here is to wait until this thread has completed
                        // validation.
                        //
                        while(_state == StateNotValidated)
                        {
                            Monitor.Wait(this);
                        }

                        if(_state >= StateClosing)
                        {
                            Debug.Assert(_exception != null);
                            throw _exception;
                        }

                        return;
                    }

                    Debug.Assert(_state == StateNotValidated || _state == StateClosed);
                    if(_state == StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }

                    if(_adapter != null)
                    {
                        active = true; // The server side has the active role for connection validation.
                    }
                    else
                    {
                        active = false; // The client side has the passive role for connection validation.
                    }
                }

                try
                {
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
                    
                    if(active)
                    {
                        lock(_sendMutex)
                        {
                            if(_transceiver == null) // Has the transceiver already been closed?
                            {
                                Debug.Assert(_exception != null);
                                throw _exception; // The exception is immutable at this point.
                            }

                            IceInternal.BasicStream os = new IceInternal.BasicStream(instance_);
                            os.writeBlob(IceInternal.Protocol.magic);
                            os.writeByte(IceInternal.Protocol.protocolMajor);
                            os.writeByte(IceInternal.Protocol.protocolMinor);
                            os.writeByte(IceInternal.Protocol.encodingMajor);
                            os.writeByte(IceInternal.Protocol.encodingMinor);
                            os.writeByte(IceInternal.Protocol.validateConnectionMsg);
                            os.writeByte((byte)0); // Compression status (always zero for validate connection).
                            os.writeInt(IceInternal.Protocol.headerSize); // Message size.
                            IceInternal.TraceUtil.traceHeader("sending validate connection", os, _logger, _traceLevels);
                            try
                            {
                                _transceiver.initialize(timeout);
                                _transceiver.write(os, timeout);
                            }
                            catch(TimeoutException)
                            {
                                throw new ConnectTimeoutException();
                            }
                        }
                    }
                    else
                    {
                        IceInternal.BasicStream ins = new IceInternal.BasicStream(instance_);
                        ins.resize(IceInternal.Protocol.headerSize, true);
                        ins.pos(0);
                        try
                        {
                            _transceiver.initialize(timeout);
                            _transceiver.read(ins, timeout);
                        }
                        catch(TimeoutException)
                        {
                            throw new ConnectTimeoutException();
                        }
                        Debug.Assert(ins.pos() == IceInternal.Protocol.headerSize);
                        ins.pos(0);
                        byte[] m = ins.readBlob(4);
                        if(m[0] != IceInternal.Protocol.magic[0] || m[1] != IceInternal.Protocol.magic[1] ||
                           m[2] != IceInternal.Protocol.magic[2] || m[3] != IceInternal.Protocol.magic[3])
                        {
                            BadMagicException ex = new BadMagicException();
                            ex.badMagic = m;
                            throw ex;
                        }
                        byte pMajor = ins.readByte();
                        byte pMinor = ins.readByte();
                        if(pMajor != IceInternal.Protocol.protocolMajor)
                        {
                            UnsupportedProtocolException e = new UnsupportedProtocolException();
                            e.badMajor = pMajor < 0 ? pMajor + 255 : pMajor;
                            e.badMinor = pMinor < 0 ? pMinor + 255 : pMinor;
                            e.major = IceInternal.Protocol.protocolMajor;
                            e.minor = IceInternal.Protocol.protocolMinor;
                            throw e;
                        }
                        byte eMajor = ins.readByte();
                        byte eMinor = ins.readByte();
                        if(eMajor != IceInternal.Protocol.encodingMajor)
                        {
                            UnsupportedEncodingException e = new UnsupportedEncodingException();
                            e.badMajor = eMajor < 0 ? eMajor + 255 : eMajor;
                            e.badMinor = eMinor < 0 ? eMinor + 255 : eMinor;
                            e.major = IceInternal.Protocol.encodingMajor;
                            e.minor = IceInternal.Protocol.encodingMinor;
                            throw e;
                        }
                        byte messageType = ins.readByte();
                        if(messageType != IceInternal.Protocol.validateConnectionMsg)
                        {
                            throw new ConnectionNotValidatedException();
                        }
                        ins.readByte(); // Ignore compression status for validate connection.
                        int size = ins.readInt();
                        if(size != IceInternal.Protocol.headerSize)
                        {
                            throw new IllegalMessageSizeException();
                        }
                        IceInternal.TraceUtil.traceHeader("received validate connection", ins, _logger, _traceLevels);
                    }
                }
                catch(LocalException ex)
                {
                    lock(this)
                    {
                        setState(StateClosed, ex);
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }
                }
            }
                
            lock(this)
            {
                if(_acmTimeout > 0)
                {
                    _acmAbsoluteTimeoutMillis = System.DateTime.Now.Ticks / 10000 + _acmTimeout * 1000;
                }
                
                //
                // We start out in holding state.
                //
                setState(StateHolding);
            }
        }
        
        public void activate()
        {
            lock(this)
            {
                while(_state == StateNotValidated)
                {
                    Monitor.Wait(this);
                }

                setState(StateActive);
            }
        }
        
        public void hold()
        {
            lock(this)
            {
                while(_state == StateNotValidated)
                {
                    Monitor.Wait(this);
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

        public bool isDestroyed()
        {
            lock(this)
            {
                return _state >= StateClosing;
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
                        int waitTime = (int)(absoluteWaitTime - System.DateTime.Now.Ticks / 10000);
                        
                        if(waitTime > 0)
                        {
                            //
                            // We must wait a bit longer until we close
                            // this connection.
                            //
                            Monitor.Wait(this, waitTime);
                            if(System.DateTime.Now.Ticks / 10000 >= absoluteWaitTime)
                            {
                                setState(StateClosed, new CloseTimeoutException());
                            }
                        }
                        else
                        {
                            //
                            // We already waited long enough, so let's
                            // close this connection!
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
                // Check for timed out async requests.
                //
                foreach(IceInternal.OutgoingAsync og in _asyncRequests.Values)
                {
                    if(og.timedOut__())
                    {
                        setState(StateClosed, new TimeoutException());
                        return;
                    }
                }
                
                //
                // Active connection management for idle connections.
                //
                //
                if(_acmTimeout > 0 &&
                    _requests.Count == 0 && _asyncRequests.Count == 0 &&
                    !_batchStreamInUse && _batchStream.isEmpty() &&
                    _dispatchCount == 0)
                {
                    if(System.DateTime.Now.Ticks / 10000 >= _acmAbsoluteTimeoutMillis)
                    {
                        setState(StateClosing, new ConnectionTimeoutException());
                        return;
                    }
                }
            }
            finally
            {
                Monitor.Exit(this);
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

        /*
        private class MessageInfo
        {
            MessageInfo(IceInternal.BasicStream stream)
            {
                this.stream = stream;
            }

            IceInternal.BasicStream stream;
            bool destroyStream;
            int invokeNum;
            int requestId;
            byte compress;
            IceInternal.ServantManager servantManager;
            ObjectAdapter adapter;
            IceInternal.OutgoingAsync outAsync;
        }
        */

        public void sendRequest(IceInternal.BasicStream os, IceInternal.Outgoing og, bool compress)
        {
            int requestId = 0;
            IceInternal.BasicStream stream = null;

            lock(this)
            {
                Debug.Assert(!(og != null && _endpoint.datagram())); // Twoway requests cannot be datagrams.

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
                // Only add to the request map if this is a twoway call.
                //
                if(og != null)
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

                    //
                    // Add to the requests map.
                    //
                    _requests[requestId] = og;
                }

                stream = doCompress(os, _overrideCompress ? _overrideCompressValue : compress);
                
                if(_acmTimeout > 0)
                {
                    _acmAbsoluteTimeoutMillis = System.DateTime.Now.Ticks / 10000 + _acmTimeout * 1000;
                }
            }
            
            try
            {
                lock(_sendMutex)
                {
                    if(_transceiver == null) // Has the transceiver already been closed?
                    {
                        Debug.Assert(_exception != null);
                        throw _exception; // The exception is immutable at this point.
                    }

                    //
                    // Send the request.
                    //
                    IceInternal.TraceUtil.traceRequest("sending request", os, _logger, _traceLevels);
                    _transceiver.write(stream, _endpoint.timeout());
                }
            }
            catch(LocalException ex)
            {
                lock(this)
                {
                    setState(StateClosed, ex);
                    Debug.Assert(_exception != null);

                    if(og != null)
                    {
                        //
                        // If the request has already been removed from
                        // the request map, we are out of luck. It would
                        // mean that finished() has been called already,
                        // and therefore the exception has been set using
                        // the Outgoing::finished() callback. In this
                        // case, we cannot throw the exception here,
                        // because we must not both raise an exception and
                        // have Outgoing::finished() called with an
                        // exception. This means that in some rare cases,
                        // a request will not be retried even though it
                        // could. But I honestly don't know how I could
                        // avoid this, without a very elaborate and
                        // complex design, which would be bad for
                        // performance.
                        //
                        IceInternal.Outgoing o = (IceInternal.Outgoing)_requests[requestId];
                        _requests.Remove(requestId);
                        if(o != null)
                        {
                            Debug.Assert(o == og);
                            throw _exception;
                        }                               
                    }
                    else
                    {
                        throw _exception;
                    }
                }
            }
        }
        
        public void sendAsyncRequest(IceInternal.BasicStream os, IceInternal.OutgoingAsync og, bool compress)
        {
            int requestId = 0;
            IceInternal.BasicStream stream = null;

            lock(this)
            {
                Debug.Assert(!_endpoint.datagram()); // Twoway requests cannot be datagrams, and async implies twoway.

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

                //
                // Add to the async requests map.
                //
                _asyncRequests[requestId] = og;
                    
                stream = doCompress(os, _overrideCompress ? _overrideCompressValue : compress);

                if(_acmTimeout > 0)
                {
                    _acmAbsoluteTimeoutMillis = System.DateTime.Now.Ticks / 10000 + _acmTimeout * 1000;
                }
            }

            try
            {
                lock(_sendMutex)
                {
                    if(_transceiver == null) // Has the transceiver already been closed?
                    {
                        Debug.Assert(_exception != null);
                        throw _exception; // The exception is imuutable at this point.
                    }

                    //
                    // Send the request.
                    //
                    IceInternal.TraceUtil.traceRequest("sending asynchronous request", os, _logger, _traceLevels);
                    _transceiver.write(stream, _endpoint.timeout());
                }
            }
            catch(LocalException ex)
            {
                lock(this)
                {
                    setState(StateClosed, ex);
                    Debug.Assert(_exception != null);
                    
                    //
                    // If the request has already been removed from the
                    // async request map, we are out of luck. It would
                    // mean that finished() has been called already, and
                    // therefore the exception has been set using the
                    // OutgoingAsync::finished__() callback. In this case,
                    // we cannot throw the exception here, because we must
                    // not both raise an exception and have
                    // OutgoingAsync::finished__() called with an
                    // exception. This means that in some rare cases, a
                    // request will not be retried even though it
                    // could. But I honestly don't know how I could avoid
                    // this, without a very elaborate and complex design,
                    // which would be bad for performance.
                    //
                    IceInternal.OutgoingAsync o = (IceInternal.OutgoingAsync)_asyncRequests[requestId];
                    _asyncRequests.Remove(requestId);
                    if(o != null)
                    {
                        Debug.Assert(o == og);
                        throw _exception;
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
            bool autoflush = false;
            byte[] lastRequest = null;

            lock(this)
            {
                //
                // Get the batch stream back.
                //
                _batchStream.swap(os);

                if(_batchAutoFlush)
                {
                    lock(_sendMutex)
                    {
                        if(_transceiver == null)
                        {
                            Debug.Assert(_exception != null);
                            throw _exception; // The exception is immutable at this point.
                        }
                        //
                        // Throw memory limit exception if the first
                        // message added causes us to go over
                        // limit. Otherwise put aside the marshalled
                        // message that caused limit to be exceeded and
                        // rollback stream to the marker.
                        //
                        try
                        {
                            _transceiver.checkSendSize(_batchStream, instance_.messageSizeMax());
                        }
                        catch(Ice.LocalException ex)
                        {
                            if(_batchRequestNum == 0)
                            {
                                resetBatch(true);
                                throw ex;
                            }
                            int requestSize = _batchStream.size() - _batchMarker;
                            lastRequest = new byte[requestSize];
                            Buffer.BlockCopy(_batchStream.prepareRead().rawBytes(), _batchMarker, lastRequest, 0,
                                             requestSize);
                            _batchStream.resize(_batchMarker, false);
                            autoflush = true;
                        }
                    }
                }
                if(!autoflush)
                {
                    // 
                    // Increment the number of requests in the batch.
                    //
                    ++_batchRequestNum;

                    //
                    // We compress the whole batch if there is at least
                    // one compressed message.
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

            if(autoflush)
            {
                //
                // We have to keep _batchStreamInUse set until after we insert the
                // saved marshalled data into a new stream.
                //
                flushBatchRequestsInternal(true);

                lock(this)
                {
                    //
                    // Throw memory limit exception if the message that caused us to go over
                    // limit causes us to exceed the limit by itself.
                    //
                    if(IceInternal.Protocol.requestBatchHdr.Length + lastRequest.Length >  instance_.messageSizeMax())
                    {
                        resetBatch(true);
                        throw new MemoryLimitException();
                    }

                    //
                    // Start a new batch with the last message that caused us to
                    // go over the limit.
                    //
                    try
                    {
                        _batchStream.writeBlob(IceInternal.Protocol.requestBatchHdr);
                        _batchStream.writeBlob(lastRequest);
                    }
                    catch(LocalException ex)
                    {
                        setState(StateClosed, ex);
                        throw;
                    }

                    if(compress)
                    {
                        _batchRequestCompress = true;
                    }

                    //
                    // Notify that the batch stream not in use anymore.
                    //
                    ++_batchRequestNum;
                    _batchStreamInUse = false;
                    Monitor.PulseAll(this);
                }
            }
        }
        
        public void abortBatchRequest()
        {
            lock(this)
            {
                //
                // Reset the batch stream. We cannot save old requests
                // in the batch stream, as they might be corrupted due to
                // incomplete marshaling.
                //
                resetBatch(true);
            }
        }

        public void flushBatchRequests()
        {
            flushBatchRequestsInternal(false);
        }

        private void flushBatchRequestsInternal(bool ignoreInUse)
        {
            IceInternal.BasicStream stream = null;

            lock(this)
            {
                if(!ignoreInUse)
                {
                    while(_batchStreamInUse && _exception == null)
                    {
                        Monitor.Wait(this);
                    }
                }
                
                if(_exception != null)
                {
                    throw _exception;
                }

                if(_batchStream.isEmpty())
                {
                    return; // Nothing to do.
                }

                Debug.Assert(_state > StateNotValidated);
                Debug.Assert(_state < StateClosing);
                
                //
                // Fill in the message size.
                //
                _batchStream.pos(10);
                _batchStream.writeInt(_batchStream.size());
                        
                //
                // Fill in the number of requests in the batch.
                //
                _batchStream.writeInt(_batchRequestNum);

                stream = doCompress(_batchStream, _overrideCompress ? _overrideCompressValue : _batchRequestCompress);
                        
                if(_acmTimeout > 0)
                {
                    _acmAbsoluteTimeoutMillis = System.DateTime.Now.Ticks / 10000 + _acmTimeout * 1000;
                }

                //
                // Prevent that new batch requests are added while we are
                // flushing.
                //
                _batchStreamInUse = true;
            }

            try
            {
                lock(_sendMutex)
                {
                    if(_transceiver == null) // Has the transceiver already been closed?
                    {
                        Debug.Assert(_exception != null);
                        throw _exception; // The exception is immutable at this point.
                    }

                    //
                    // Send the batch request.
                    //
                    IceInternal.TraceUtil.traceBatchRequest("sending batch request", _batchStream, _logger,
                                                            _traceLevels);
                    _transceiver.write(stream, _endpoint.timeout());
                }
            }
            catch(LocalException ex)
            {
                lock(this)
                {
                    setState(StateClosed, ex);
                    Debug.Assert(_exception != null);

                    //
                    // Since batch requests area all oneways (or datarams), we
                    // must report the exception to the caller.
                    //
                    throw _exception;
                }
            }

            lock(this)
            {
                //
                // Reset the batch stream, and notify that flushing is over.
                //
                resetBatch(!ignoreInUse);
            }
        }


        private void resetBatch(bool resetInUse)
        {
            _batchStream = new IceInternal.BasicStream(instance_, _batchAutoFlush);
            _batchRequestNum = 0;
            _batchRequestCompress = false;
                
            //
            // Notify about the batch stream not being in use
            // anymore.
            //
            if(resetInUse)
            {
                Debug.Assert(_batchStreamInUse);
                _batchStreamInUse = false;
                Monitor.PulseAll(this);
            }
        }

        public void sendResponse(IceInternal.BasicStream os, byte compress)
        {
            IceInternal.BasicStream stream = null;
            try
            {
                lock(_sendMutex)
                {
                    if(_transceiver == null) // Has the transceiver already been closed?
                    {
                        Debug.Assert(_exception != null);
                        throw _exception; // The exception is immutable at this point.
                    }

                    stream = doCompress(os, compress != 0);

                    //
                    // Send the reply.
                    //
                    IceInternal.TraceUtil.traceReply("sending reply", os, _logger, _traceLevels);
                    _transceiver.write(stream, _endpoint.timeout());
                }
            }
            catch(LocalException ex)
            {
                lock(this)
                {
                    setState(StateClosed, ex);
                }
            }

            lock(this)
            {
                Debug.Assert(_state > StateNotValidated);

                try
                {
                    if(--_dispatchCount == 0)
                    {
                        Monitor.PulseAll(this);
                    }

                    if(_state == StateClosing && _dispatchCount == 0)
                    {
                        initiateShutdown();
                    }
                
                    if(_acmTimeout > 0)
                    {
                        _acmAbsoluteTimeoutMillis = System.DateTime.Now.Ticks / 10000 + _acmTimeout * 1000;
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
            // No mutex protection necessary, _endpoint is immutable.
            return _endpoint;
        }

        public bool threadPerConnection()
        {
            return _threadPerConnection; // No mutex protection necessary, _threadPerConnection is immutable.
        }

        public void setAdapter(ObjectAdapter adapter)
        {
            lock(this)
            {
                if(_exception != null)
                {
                    throw _exception;
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
        
        public override void read(IceInternal.BasicStream stream)
        {
            Debug.Assert(!_threadPerConnection); // Only for use with a thread pool.

            _transceiver.read(stream, 0);

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
                // We must promote with the synchronization, otherwise
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
            
            LocalException exception = null;

            Hashtable requests = null;
            Hashtable asyncRequests = null;

            lock(this)
            {
                --_finishedCount;
                if(_finishedCount == 0 && _state == StateClosed)
                {
                    //
                    // We must make sure that nobody is sending when we
                    // close the transeiver.
                    //
                    lock(_sendMutex)
                    {
                        try
                        {
                            _transceiver.close();
                        }
                        catch(LocalException ex)
                        {
                            exception = ex;
                        }

                        _transceiver = null;
                        Monitor.PulseAll(this);
                    }
                }

                if(_state == StateClosed || _state == StateClosing)
                {
                    requests = _requests;
                    _requests = new Hashtable();

                    asyncRequests = _asyncRequests;
                    _asyncRequests = new Hashtable();
                }
            }

            if(requests != null)
            {
                foreach(IceInternal.Outgoing og in requests.Values)
                {
                    og.finished(_exception); // The exception is immutable at this point.
                }
            }

            if(asyncRequests != null)
            {
                foreach(IceInternal.OutgoingAsync og in asyncRequests.Values)
                {
                    og.finished__(_exception); // The exception is immutable at this point.
                }
            }

            if(exception != null)
            {
                throw exception;
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
            _threadPerConnection = threadPerConnection;
            _transceiver = transceiver;
            _desc = transceiver.ToString();
            _type = transceiver.type();
            _endpoint = endpoint;
            _adapter = adapter;
            _logger = instance.initializationData().logger; // Cached for better performance.
            _traceLevels = instance.traceLevels(); // Cached for better performance.
            _registeredWithPool = false;
            _finishedCount = 0;
            _warn = instance_.initializationData().properties.getPropertyAsInt("Ice.Warn.Connections") > 0;
            _cacheBuffers = instance_.initializationData().properties.getPropertyAsIntWithDefault(
                "Ice.CacheMessageBuffers", 1) == 1;
            _acmAbsoluteTimeoutMillis = 0;
            _nextRequestId = 1;
            _batchAutoFlush = instance_.initializationData().properties.getPropertyAsIntWithDefault(
                "Ice.BatchAutoFlush", 1) > 0;
            _batchStream = new IceInternal.BasicStream(instance, _batchAutoFlush);
            _batchStreamInUse = false;
            _batchRequestNum = 0;
            _batchRequestCompress = false;
            _dispatchCount = 0;
            _state = StateNotValidated;
            _stateTime = System.DateTime.Now.Ticks / 10000;
            
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

            _compressionLevel = 
                instance_.initializationData().properties.getPropertyAsIntWithDefault("Ice.Compression.Level", 1);
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

            if(!threadPerConnection)
            {
                //
                // Only set _threadPool if we really need it, i.e., if we are
                // not in thread per connection mode. Thread pools have lazy
                // initialization in Instance, and we don't want them to be
                // created if they are not needed.
                //
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
                }
                catch(System.Exception ex)
                {
                    try
                    {
                        _transceiver.close();
                    }
                    catch(LocalException)
                    {
                        // Here we ignore any exceptions in close().
                    }

                    throw new Ice.SyscallException(ex);
                }
            }

            _overrideCompress = instance_.defaultsAndOverrides().overrideCompress;
            _overrideCompressValue = instance_.defaultsAndOverrides().overrideCompressValue;
        }

        public void start()
        {
            //
            // If we are in thread per connection mode, create the thread for this connection.
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

                    try
                    {
                        _transceiver.close();
                    }
                    catch(LocalException)
                    {
                        // Here we ignore any exceptions in close().
                    }

                    //
                    // Clean up.
                    //
                    _transceiver = null;
                    _thread = null;
                    _state = StateClosed;

                    throw new Ice.SyscallException(ex);
                }
            }
        }

        private const int StateNotValidated = 0;
        private const int StateActive = 1;
        private const int StateHolding = 2;
        private const int StateClosing = 3;
        private const int StateClosed = 4;
        
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
                //
                // If we are in closed state, an exception must be set.
                //
                Debug.Assert(_state != StateClosed);

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
            if(_state == StateNotValidated && state == StateClosing)
            {
                state = StateClosed;
            }

            if(_state == state) // Don't switch twice.
            {
                return;
            }
            
            switch(state)
            {
                case StateNotValidated: 
                {
                    Debug.Assert(false);
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
                        return;
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
                        return;
                    }
                    if(!_threadPerConnection)
                    {
                        registerWithPool(); // We need to continue to read in closing state.
                    }
                    break;
                }
                
                case StateClosed: 
                {
                    if(_threadPerConnection)
                    {
                        //
                        // If we are in thread per connection mode, we
                        // shutdown both for reading and writing. This will
                        // unblock any read call with an exception. The thread
                        // per connection then closes the transceiver.
                        //
                        _transceiver.shutdownReadWrite();
                    }
                    else if(_state == StateNotValidated)
                    {
                        //
                        // If we change from not validated, we can close right
                        // away.
                        //
                        Debug.Assert(!_registeredWithPool);

                        //
                        // We must make sure that nobidy is sending when we
                        // close the transceiver.
                        //
                        lock(_sendMutex)
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
                            //Monitor.PulseAll(); // We notify already below.
                        }
                    }
                    else
                    {
                        //
                        // Otherwise we first must make sure that we are
                        // registered, then we unregister, and let finished()
                        // do the close.
                        //
                        registerWithPool();
                        unregisterWithPool();

                        //
                        // We must prevent any further writes when _state == StateClosed.
                        // However, functions such as sendResponse cannot acquire the main
                        // mutex in order to check _state. Therefore we shut down the write
                        // end of the transceiver, which causes subsequent write attempts
                        // to fail with an exception.
                        //
                        _transceiver.shutdownWrite();
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
            _stateTime = System.DateTime.Now.Ticks / 10000;

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
                lock(_sendMutex)
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

                    //
                    // Send the message.
                    //
                    IceInternal.TraceUtil.traceHeader("sending close connection", os, _logger, _traceLevels);
                    _transceiver.write(os, _endpoint.timeout());
                    //
                    // The CloseConnection message should be sufficient. Closing the write
                    // end of the socket is probably an artifact of how things were done
                    // in IIOP. In fact, shutting down the write end of the socket causes
                    // problems on Windows by preventing the peer from using the socket.
                    // For example, the peer is no longer able to continue writing a large
                    // message after the socket is shutdown.
                    //
                    //_transceiver.shutdown();
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
                _acmAbsoluteTimeoutMillis = System.DateTime.Now.Ticks / 10000 + _acmTimeout * 1000;
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
                        IceInternal.BasicStream uncompressedStream
                            = stream.uncompress(IceInternal.Protocol.headerSize);
                        stream = uncompressedStream;
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
                        IceInternal.TraceUtil.traceHeader("received close connection", stream, _logger, 
                                                          _traceLevels);
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
                            IceInternal.TraceUtil.traceRequest("received request during closing\n"
                                                               + "(ignored by server, client will retry)",
                                                               stream, _logger, _traceLevels);
                        }
                        else
                        {
                            IceInternal.TraceUtil.traceRequest("received request", stream, _logger, _traceLevels);
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
                            IceInternal.TraceUtil.traceBatchRequest("received batch request during closing\n"
                                                                    + "(ignored by server, client will retry)",
                                                                    stream, _logger, _traceLevels);
                        }
                        else
                        {
                            IceInternal.TraceUtil.traceBatchRequest("received batch request", stream, _logger, 
                                                                    _traceLevels);
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
                        IceInternal.TraceUtil.traceReply("received reply", stream, _logger, _traceLevels);

                        requestId = stream.readInt();

                        IceInternal.Outgoing og = (IceInternal.Outgoing)_requests[requestId];
                        _requests.Remove(requestId);
                        if(og != null)
                        {
                            og.finished(stream);
                        }
                        else
                        {
                            outAsync = (IceInternal.OutgoingAsync)_asyncRequests[requestId];
                            _asyncRequests.Remove(requestId);
                            if(outAsync == null)
                            {
                                throw new UnknownRequestIdException();
                            }
                        }
                        break;
                    }
                    
                    case IceInternal.Protocol.validateConnectionMsg: 
                    {
                        IceInternal.TraceUtil.traceHeader("received validate connection", stream, _logger, 
                                                          _traceLevels);
                        if(_warn)
                        {
                            _logger.warning("ignoring unexpected validate connection message:\n" + _desc);
                        }
                        break;
                    }
                    
                    default: 
                    {
                        IceInternal.TraceUtil.traceHeader("received unknown message\n" +
                                                          "(invalid, closing connection)",
                                                          stream, _logger, _traceLevels);
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
            //
            // For non-datagram connections, the thread-per-connection must
            // validate and activate this connection, and not in the
            // connection factory. Please see the comments in the connection
            // factory for details.
            //
            if(!_endpoint.datagram())
            {
                try
                {
                    validate();
                }
                catch(LocalException)
                {
                    lock(this)
                    {
                        Debug.Assert(_state == StateClosed);

                        //
                        // We must make sure that nobody is sending when we close
                        // the transceiver.
                        //
                        lock(_sendMutex)
                        {
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
                            return;
                        }
                    }
                }

                activate();
            }

            bool warnUdp = instance_.initializationData().properties.getPropertyAsInt("Ice.Warn.Datagrams") > 0;

            bool closed = false;

            IceInternal.BasicStream stream = new IceInternal.BasicStream(instance_);

            while(!closed)
            {
                //
                // We must accept new connections outside the thread
                // synchronization, because we use blocking accept.
                //

                try
                {
                    try
                    {
                        stream.resize(IceInternal.Protocol.headerSize, true);
                        stream.pos(0);
                        _transceiver.read(stream, -1);

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
                                _transceiver.read(stream, -1);
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

                    Hashtable requests = null;
                    Hashtable asyncRequests = null;

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
                            //
                            // We must make sure that nobody is sending when we close
                            // the transceiver.
                            //
                            lock(_sendMutex)
                            {
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

                        if(_state == StateClosed || _state == StateClosing)
                        {
                            requests = _requests;
                            _requests = new Hashtable();

                            asyncRequests = _asyncRequests;
                            _asyncRequests = new Hashtable();
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

                    if(requests != null)
                    {
                        foreach(IceInternal.Outgoing og in requests.Values)
                        {
                            og.finished(_exception); // The exception is immutable at this point.
                        }
                    }

                    if(asyncRequests != null)
                    {
                        foreach(IceInternal.OutgoingAsync og in asyncRequests.Values)
                        {
                            og.finished__(_exception); // The exception is immutable at this point.
                        }
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
                _logger.error("system exception in thread per connection:\n" + ToString() + "\n" +
                                         ex.ToString());
            }
            finally
            {
                if(instance_.initializationData().threadHook != null)
                {
                    instance_.initializationData().threadHook.stop();
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

        public IceInternal.Outgoing getOutgoing(IceInternal.Reference reference, string operation, OperationMode mode,
                                                Context context, bool compress)
        {
            IceInternal.Outgoing outg = null;

            if(_cacheBuffers)
            {
                lock(_outgoingCacheMutex)
                {
                    if(_outgoingCache == null)
                    {
                        outg = new IceInternal.Outgoing(this, reference, operation, mode, context, compress);
                    }
                    else
                    {
                        outg = _outgoingCache;
                        _outgoingCache = _outgoingCache.next;
                        outg.reset(reference, operation, mode, context, compress);
                        outg.next = null;
                    }
                }
            }
            else
            {
                outg = new IceInternal.Outgoing(this, reference, operation, mode, context, compress);
            }
            
            return outg;
        }

        public void reclaimOutgoing(IceInternal.Outgoing outg)
        {
            if(_cacheBuffers)
            {
                //
                // Clear references to Ice objects as soon as possible.
                //
                outg.reclaim();

                lock(_outgoingCacheMutex)
                {
                    outg.next = _outgoingCache;
                    _outgoingCache = outg;
                }
            }
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
        
        private bool _warn;
        
        private int _acmTimeout;
        private long _acmAbsoluteTimeoutMillis;

        private int _compressionLevel;

        private int _nextRequestId;
        private Hashtable _requests = new Hashtable();
        private Hashtable _asyncRequests = new Hashtable();
        
        private LocalException _exception;

        private bool _batchAutoFlush;
        private IceInternal.BasicStream _batchStream;
        private bool _batchStreamInUse;
        private int _batchRequestNum;
        private bool _batchRequestCompress;
        private int _batchMarker;
        
        private int _dispatchCount;
        
        private int _state; // The current state.
        private long _stateTime; // The last time when the state was changed.
        
        //
        // We have a separate mutex for sending, so that we don't block
        // the whole connection when we do a blocking send.
        //
        private object _sendMutex = new object();

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
