// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class Emitter extends EventHandler
{
    public void
    destroy()
    {
        _mutex.lock();
        try
        {
            setState(StateClosed, new Ice.CommunicatorDestroyedException());
        }
        finally
        {
            _mutex.unlock();
        }
    }

    public boolean
    destroyed()
    {
        _mutex.lock();
        try
        {
            return _state >= StateClosing;
        }
        finally
        {
            _mutex.unlock();
        }
    }

    public void
    prepareRequest(Outgoing out)
    {
        BasicStream os = out.os();
        os.writeByte(Protocol.protocolVersion);
        os.writeByte(Protocol.encodingVersion);
        os.writeByte(Protocol.requestMsg);
        os.writeInt(0); // Message size (placeholder)
        os.writeInt(0); // Request ID (placeholder)
    }

    public void
    sendRequest(Outgoing out, boolean oneway)
    {
        _mutex.lock();
        try
        {
            if (_exception != null)
            {
                throw _exception;
            }
            assert(_state == StateActive);

            int requestId = 0;

            try
            {
                BasicStream os = out.os();
                os.pos(3);

                //
                // Fill in the message size and request ID
                //
                os.writeInt(os.size());
                if (!_endpoint.oneway() && !oneway)
                {
                    requestId = _nextRequestId++;
                    if (requestId <= 0)
                    {
                        _nextRequestId = 1;
                        requestId = _nextRequestId++;
                    }
                    os.writeInt(requestId);
                }
                TraceUtil.traceRequest("sending request", os, _logger,
                                       _traceLevels);
                _transceiver.write(os, _endpoint.timeout());
            }
            catch (Ice.LocalException ex)
            {
                setState(StateClosed, ex);
                throw ex;
            }

            //
            // Only add to the request map if there was no exception, and if
            // the operation is not oneway.
            //
            if (!_endpoint.oneway() && !oneway)
            {
                _requests.put(new Integer(requestId), out);
            }
        }
        finally
        {
            _mutex.unlock();
        }
    }

    public void
    prepareBatchRequest(Outgoing out)
    {
        lock();

        if (_exception != null)
        {
            unlock();
            throw _exception;
        }
        assert(_state == StateActive);

        //
        // The Emitter now belongs to `out', until finishBatchRequest() is
        // called.
        //

        if (_batchStream.size() == 0)
        {
            _batchStream.writeByte(Protocol.protocolVersion);
            _batchStream.writeByte(Protocol.encodingVersion);
            _batchStream.writeByte(Protocol.requestMsg);
            _batchStream.writeInt(0); // Message size (placeholder)
        }

        _batchStream.startWriteEncaps();

        //
        // Give the batch stream to `out', until finishBatchRequest() is
        // called.
        //
        _batchStream.swap(out.os());
    }

    public void
    finishBatchRequest(Outgoing out)
    {
        if (_exception != null)
        {
            unlock();
            throw _exception;
        }
        assert(_state == StateActive);

        _batchStream.swap(out.os()); // Get the batch stream back
        unlock(); // Give the Emitter back

        _batchStream.endWriteEncaps();
    }

    public void
    abortBatchRequest()
    {
        state(StateClosed, new Ice.AbortBatchRequestException());
        unlock();
    }

    public void
    flushBatchRequest()
    {
        _mutex.lock();
        try
        {
            if (_exception != null)
            {
                throw _exception;
            }
            assert(_state == StateActive);

            try
            {
                if (_batch.size() == 0)
                {
                    return; // Nothing to send
                }

                _batchStream.pos(3);

                //
                // Fill in the message size
                //
                _batchStream.writeInt(_batchStream.size());
                TraceUtil.traceBatchRequest("sending batch request",
                                            _batchStream, _logger,
                                            _traceLevels);
                _transceiver.write(_batchStream, _endpoint.timeout());

                //
                // Reset _batchStream so that new batch messages can be sent.
                //
                _batchStream = new BasicStream(_instance);
            }
            catch (Ice.LocalException ex)
            {
                setState(StateClosed, ex);
                throw ex;
            }
        }
        finally
        {
            _mutex.unlock();
        }
    }

    public int
    timeout()
    {
        return _endpoint.timeout();
    }

    //
    // Operations from EventHandler
    //
    public boolean
    server()
    {
        return true;
    }

    public boolean
    readable()
    {
        return true;
    }

    public void
    read(BasicStream is)
    {
        _transceiver.read(stream, 0);
    }

    public void
    message(BasicStream stream)
    {
        _mutex.lock();
        try
        {
            _threadPool.promoteFollower();

            if (_state != StateActive)
            {
                return;
            }

            try
            {
                assert(stream.pos() == stream.size());
                stream.pos(2);
                byte messageType = stream.readByte();
                stream.pos(Protocol.headerSize);

                switch (messageType)
                {
                    case Protocol.requestMsg:
                    {
                        TraceUtil.traceRequest("received request on " +
                                               "the client side\n(invalid, " +
                                               "closing connection)",
                                               stream, _logger, _traceLevels);
                        throw new Ice.InvalidMessageException();
                    }

                    case Protocol.requestBatchMsg:
                    {
                        TraceUtil.traceRequest("received batch request on " +
                                               "the client side\n(invalid, " +
                                               "closing connection)",
                                               stream, _logger, _traceLevels);
                        throw new Ice.InvalidMessageException();
                    }

                    case Protocol.replyMsg:
                    {
                        TraceUtil.traceReply("received reply", stream,
                                             _logger, _traceLevels);
                        int requestId = stream.readInt();
                        Outgoing out =
                            (Outgoing)_requests.remove(new Integer(requestId));
                        if (out == null)
                        {
                            throw new Ice.UnknownRequestIdException();
                        }
                        out.finished(stream);
                        break;
                    }

                    case Protocol.closeConnectionMsg:
                    {
                        TraceUtil.traceHeader("received close connection",
                                              stream, _logger, _traceLevels);
                        throw new Ice.CloseConnectionException();
                    }

                    default:
                    {
                        TraceUtil.traceHeader("received unknown message\n" +
                                              "(invalid, closing connection)",
                                              stream, _logger, _traceLevels);
                        throw new Ice.UnknownMessageException();
                    }
                }
            }
            catch (Ice.LocalException ex)
            {
                setState(StateClosed, ex);
                return;
            }
        }
        finally
        {
            _mutex.unlock();
        }
    }

    public void
    exception(Ice.LocalException ex)
    {
        _mutex.lock();
        try
        {
            setState(StateClosed, ex);
        }
        finally
        {
            _mutex.unlock();
        }
    }

    public void
    finished()
    {
        _mutex.lock();
        try
        {
            _transceiver.close();
        }
        finally
        {
            _mutex.unlock();
        }
    }

    public boolean
    tryDestroy()
    {
        boolean isLocked = _mutex.trylock();
        if (!isLocked)
        {
            return false;
        }

        _threadPool.promoteFollower();

        try
        {
            setState(StateClosed, new Ice.CommunicatorDestroyedException());
            return true;
        }
        finally
        {
            unlock();
        }
    }

    Emitter(Instance instance, Transceiver transceiver, Endpoint endpoint)
    {
        super(instance);
        _transceiver = transceiver;
        _endpoint = endpoint;
        _nextRequestId = 1;
        _batchStream = new BasicStream(instance);
        _state = StateActive;

        _traceLevels = _instance.traceLevels();
        _logger = _instance.logger();

        if (!_endpoint.oneway())
        {
            _threadPool = _instance.threadPool();
            _threadPool._register(_transceiver.fd(), this);
        }
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_state == StateClosed);

        super.finalize();
    }

    private static final int StateActive = 0;
    private static final int StateHolding = 1;
    private static final int StateClosing = 2;
    private static final int StateClosed = 3;

    private void
    setState(int state, Ice.LocalException ex)
    {
        if (_state == state) // Don't switch twice
        {
            return;
        }

        switch (state)
        {
            case StateActive:
            {
                return; // Can't switch back to holding state
            }

            case StateClosed:
            {
                if (_threadPool != null)
                {
                    _threadPool.unregister(_transceiver.fd());
                }
                else
                {
                    _transceiver.close();
                }
                break;
            }
        }

        if (_exception == null)
        {
            _exception = ex;
        }

        java.util.Set entries = _requests.entrySet();
        java.util.Iterator p = entries.iterator();
        while (p.hasNext())
        {
            Outgoing out = (Outgoing)p.next();
            out.finished(_exception);
        }
        _requests.clear();

        _state = state;
    }

    private Transceiver _transceiver;
    private Endpoint _endpoint;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private ThreadPool _threadPool;
    private int _nextRequestId;
    private java.util.HashMap _requests = new java.util.HashMap();
    private Ice.LocalException _exception;
    private BasicStream _batchStream;
    private int _state;
    private RecursiveMutex _mutex = new RecursiveMutex();
}
