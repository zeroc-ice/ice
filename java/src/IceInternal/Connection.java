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

public final class Connection extends EventHandler
{
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
    hold()
    {
        _mutex.lock();
        try
        {
            setState(StateHolding);
        }
        finally
        {
            _mutex.unlock();
        }
    }

    public void
    activate()
    {
        _mutex.lock();
        try
        {
            setState(StateActive);
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
            assert(_state < StateClosing);

            int requestId = 0;

            try
            {
                BasicStream os = out.os();
                os.pos(3);

                //
                // Fill in the message size and request ID
                //
                os.writeInt(os.size());
                if (!_endpoint.datagram() && !oneway)
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
            if (!_endpoint.datagram() && !oneway)
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
        _mutex.lock();

        if (_exception != null)
        {
            _mutex.unlock();
            throw _exception;
        }
        assert(_state < StateClosing);

        //
        // The Connection now belongs to `out', until finishBatchRequest() is
        // called.
        //

        if (_batchStream.size() == 0)
        {
            _batchStream.writeByte(Protocol.protocolVersion);
            _batchStream.writeByte(Protocol.encodingVersion);
            _batchStream.writeByte(Protocol.requestBatchMsg);
            _batchStream.writeInt(0); // Message size (placeholder)
        }

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
            _mutex.unlock();
            throw _exception;
        }
        assert(_state < StateClosing);

        _batchStream.swap(out.os()); // Get the batch stream back
        _mutex.unlock(); // Give the Connection back
    }

    public void
    abortBatchRequest()
    {
        setState(StateClosed, new Ice.AbortBatchRequestException());
        _mutex.unlock(); // Give the Connection back
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
            assert(_state < StateClosing);

            try
            {
                if (_batchStream.size() == 0)
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
        // No mutex protection necessary, _endpoint is immutable.
        return _endpoint.timeout();
    }

    public Endpoint
    endpoint()
    {
        // No mutex protection necessary, _endpoint is immutable.
        return _endpoint;
    }

    public void
    setAdapter(Ice.ObjectAdapter adapter)
    {
        _mutex.lock();
        try
        {
            _adapter = adapter;
        }
        finally
        {
            _mutex.unlock();
        }
    }

    public Ice.ObjectAdapter
    getAdapter()
    {
        _mutex.lock();
        try
        {
            return _adapter;
        }
        finally
        {
            _mutex.unlock();
        }
    }

    //
    // Operations from EventHandler
    //
    public boolean
    server()
    {
        _mutex.lock();
        try
        {
            return _adapter != null;
        }
        finally
        {
            _mutex.unlock();
        }
    }

    public boolean
    readable()
    {
        return true;
    }

    public void
    read(BasicStream stream)
    {
        _transceiver.read(stream, 0);
    }

    public void
    message(BasicStream stream)
    {
        boolean invoke = false;
        boolean batch = false;

        _mutex.lock();
        try
        {
            _threadPool.promoteFollower();

            if (_state == StateClosed)
            {
                Thread.yield();
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
                        if (_state == StateClosing)
                        {
                            TraceUtil.traceRequest("received request during " +
                                                   "closing\n(ignored by " +
                                                   "server, client will " +
                                                   "retry)", stream, _logger,
                                                   _traceLevels);
                        }
                        else
                        {
                            TraceUtil.traceRequest("received request", stream,
                                                   _logger, _traceLevels);
                            invoke = true;
                        }
                        break;
                    }

                    case Protocol.requestBatchMsg:
                    {
                        if (_state == StateClosing)
                        {
                            TraceUtil.traceBatchRequest("received batch " +
                                                        "request during " +
                                                        "closing\n" +
                                                        "(ignored by server, " +
                                                        "client will retry)",
                                                        stream, _logger,
                                                        _traceLevels);
                        }
                        else
                        {
                            TraceUtil.traceBatchRequest("received batch " +
                                                        "request", stream,
                                                        _logger, _traceLevels);
                            invoke = true;
                            batch = true;
                        }
                        break;
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
                        if (_endpoint.datagram())
                        {
                            if (_warn)
                            {
                                _logger.warning("ignoring close connection " +
                                                "message for datagram " +
                                                "connection:\n" +
                                                _transceiver.toString());
                            }
                        }
                        else
                        {
                            throw new Ice.CloseConnectionException();
                        }
                        break;
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

        //
        // Method invocation must be done outside the thread
        // synchronization, so that nested callbacks are possible.
        //
        if (invoke)
        {
            Incoming in = new Incoming(_instance, _adapter);
            BasicStream is = in.is();
            BasicStream os = in.os();
            stream.swap(is);

            boolean response = false;

            try
            {
                if (!batch)
                {
                    int requestId = is.readInt();
                    if (!_endpoint.datagram() && requestId != 0) // 0 = oneway
                    {
                        response = true;
                        ++_responseCount;
                        os.writeByte(Protocol.protocolVersion);
                        os.writeByte(Protocol.encodingVersion);
                        os.writeByte(Protocol.replyMsg);
                        os.writeInt(0); // Message size (placeholder)
                        os.writeInt(requestId);
                    }
                }

                do
                {
                    try
                    {
                        in.invoke(response);
                    }
                    catch (Ice.LocalException ex)
                    {
                        _mutex.lock();
                        try
                        {
                            warning(ex);
                        }
                        finally
                        {
                            _mutex.unlock();
                        }
                    }
                    catch (Exception ex)
                    {
                        assert(false); // Should not happen
                    }
                }
                while (batch && is.pos() < is.size());
            }
            catch (Ice.LocalException ex)
            {
                _mutex.lock();
                try
                {
                    setState(StateClosed, ex);
                    return;
                }
                finally
                {
                    _mutex.unlock();
                }
            }

            if (response)
            {
                _mutex.lock();
                try
                {
                    try
                    {
                        if (_state == StateClosed)
                        {
                            return;
                        }

                        //
                        // Fill in the message size
                        //
                        os.pos(3);
                        final int sz = os.size();
                        os.writeInt(sz);

                        TraceUtil.traceReply("sending reply", os, _logger,
                                             _traceLevels);
                        _transceiver.write(os, _endpoint.timeout());

                        --_responseCount;

                        if (_state == StateClosing && _responseCount == 0 &&
                            !_endpoint.datagram())
                        {
                            closeConnection();
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
            assert(_state == StateClosed);
            _transceiver.close();
        }
        finally
        {
            _mutex.unlock();
        }
    }

    /*
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
            setState(StateClosing);
            return true;
        }
        finally
        {
            _mutex.unlock();
        }
    }
    */

    Connection(Instance instance, Transceiver transceiver, Endpoint endpoint,
               Ice.ObjectAdapter adapter)
    {
        super(instance);
        _transceiver = transceiver;
        _endpoint = endpoint;
        _adapter = adapter;
        _threadPool = instance.threadPool();
        _logger = instance.logger();
        _traceLevels = instance.traceLevels();
        _nextRequestId = 1;
        _batchStream = new BasicStream(instance);
        _responseCount = 0;
        _state = StateHolding;

        try
        {
            String val =
                _instance.properties().getProperty("Ice.ConnectionWarnings");
            _warn = Integer.parseInt(val) > 0 ? true : false;
        }
        catch (NumberFormatException ex)
        {
            _warn = false;
        }
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_state == StateClosed);

        super.finalize();
    }

    // DestructionReason
    public final static int ObjectAdapterDeactivated = 0;
    public final static int CommunicatorDestroyed = 1;

    public void
    destroy(int reason)
    {
        _mutex.lock();
        try
        {
            switch (reason)
            {
                case ObjectAdapterDeactivated:
                {
                    setState(StateClosing,
                             new Ice.ObjectAdapterDeactivatedException());
                    break;
                }

                case CommunicatorDestroyed:
                {
                    setState(StateClosing,
                             new Ice.CommunicatorDestroyedException());
                    break;
                }
            }
        }
        finally
        {
            _mutex.unlock();
        }
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

        if (_exception == null)
        {
            _exception = ex;

            //
            // Don't warn about certain expected exceptions.
            //
            if (!(ex instanceof Ice.CloseConnectionException ||
                  ex instanceof Ice.CommunicatorDestroyedException ||
                  ex instanceof Ice.ObjectAdapterDeactivatedException ||
                  (ex instanceof Ice.ConnectionLostException &&
                   _state == StateClosing)))
            {
                warning(ex);
            }
        }

        java.util.Iterator i = _requests.values().iterator();
        while (i.hasNext())
        {
            Outgoing out = (Outgoing)i.next();
            out.finished(_exception);
        }
        _requests.clear();

        setState(state);
    }

    private void
    setState(int state)
    {
        //
        // We don't want to send close connection messages if the endpoint
        // only supports oneway transmission from client to server.
        //
        if (_endpoint.datagram() && state == StateClosing)
        {
            state = StateClosed;
        }

        if (_state == state) // Don't switch twice
        {
            return;
        }

        switch (state)
        {
            case StateActive:
            {
                if (_state != StateHolding) // Can only switch from holding
                {                           // to active
                    return;
                }
                _threadPool._register(_transceiver.fd(), this);
                break;
            }

            case StateHolding:
            {
                if (_state != StateActive) // Can only switch from active
                {                          // to holding
                    return;
                }
                _threadPool.unregister(_transceiver.fd(), false);
                break;
            }

            case StateClosing:
            {
                if (_state == StateClosed) // Can't change back from closed
                {
                    return;
                }
                if (_state == StateHolding)
                {
                    //
                    // We need to continue to read data in closing state
                    //
                    _threadPool._register(_transceiver.fd(), this);
                }
                break;
            }

            case StateClosed:
            {
                if (_state == StateHolding)
                {
                    //
                    // If we come from holding state, we first need to
                    // register again before we unregister, so that
                    // finished() is called correctly.
                    //
                    _threadPool._register(_transceiver.fd(), this);
                }
                _threadPool.unregister(_transceiver.fd(), true);
                break;
            }
        }

        _state = state;


        if (_state == StateClosing && _responseCount == 0 &&
            !_endpoint.datagram())
        {
            try
            {
                closeConnection();
            }
            catch (Ice.LocalException ex)
            {
                setState(StateClosed, ex);
            }
        }
    }

    private void
    closeConnection()
    {
        BasicStream os = new BasicStream(_instance);
        os.writeByte(Protocol.protocolVersion);
        os.writeByte(Protocol.encodingVersion);
        os.writeByte(Protocol.closeConnectionMsg);
        os.writeInt(Protocol.headerSize); // Message size
        _transceiver.write(os, _endpoint.timeout());
        _transceiver.shutdown();
    }

    private void
    warning(Ice.LocalException ex)
    {
        if (_warn)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            String s = "connection exception:\n" + sw.toString() + '\n' +
                _transceiver.toString();
            _logger.warning(s);
        }
    }

    private Transceiver _transceiver;
    private Endpoint _endpoint;
    private Ice.ObjectAdapter _adapter;
    private ThreadPool _threadPool;
    private Ice.Logger _logger;
    private TraceLevels _traceLevels;
    private int _nextRequestId;
    private java.util.HashMap _requests = new java.util.HashMap();
    private Ice.LocalException _exception;
    private BasicStream _batchStream;
    private int _responseCount;
    private int _state;
    private boolean _warn;
    private RecursiveMutex _mutex = new RecursiveMutex();
}
