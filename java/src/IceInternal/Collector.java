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

public class Collector extends EventHandler
{
    public void
    destroy()
    {
        _mutex.lock();
        try
        {
            setState(StateClosing);
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
    read(Ice.Stream is)
    {
        _transceiver.read(is, 0);
    }

    public void
    message(Ice.Stream stream)
    {
        Incoming in = new Incoming(_instance, _adapter);
        Stream os = in.os();
        boolean invoke = false;
        boolean batch = false;
        boolean response = false;

        _mutex.lock();
        try
        {
            _threadPool.promoteFollower();

            if (_state != StateActive && _state != StateClosing)
            {
                return;
            }

            try
            {
                // assert(stream.i == stream.b.end()); TODO
                // stream.i = stream.b.begin() + 2;
                // Byte messageType;
                // stream.read(messageType);
                // stream.i = stream.b.begin() + headerSize;

                //
                // Write partial message header
                //
                os.writeByte(Protocol.protocolVersion);
                os.writeByte(Protocol.encodingVersion);

                switch (messageType)
                {
                    case Protocol.requestMsg:
                    {
                        if (_state == StateClosing)
                        {
                            TraceUtil.traceRequest(
                                "received request during closing\n" +
                                "(ignored by server, client will retry)",
                                stream, _logger, _traceLevels);
                        }
                        else
                        {
                            TraceUtil.traceRequest(
                                "received request", stream, _logger,
                                _traceLevels);
                            invoke = true;
                            int requestId = stream.readInt();
                            if (!_endpoint.oneway() && requestId != 0)
                                // 0 means oneway
                            {
                                response = true;
                                ++_responseCount;
                                os.writeByte(Protocol.replyMsg);
                                os.writeInt(0); // Message size (placeholder)
                                os.writeInt(requestId);
                            }
                        }
                        break;
                    }

                    case Protocol.batchRequestMsg:
                    {
                        if (_state == StateClosing)
                        {
                            TraceUtil.traceBatchRequest(
                                "received batch request during closing\n" +
                                "(ignored by server, client will retry)",
                                stream, _logger, _traceLevels);
                        }
                        else
                        {
                            TraceUtil.traceBatchRequest(
                                "received batch request", stream, _logger,
                                _traceLevels);
                            invoke = true;
                            batch = true;
                        }
                        break;
                    }

                    case Protocol.replyMsg:
                    {
                        TraceUtil.traceReply(
                            "received reply on server side\n" +
                            "(invalid, closing connection)",
                            stream, _logger, _traceLevels);
                        throw new Ice.InvalidMessageException();
                        break;
                    }

                    case Protocol.closeConnectionMsg:
                    {
                        TraceUtil.traceHeader(
                            "received close connection on server side\n" +
                            "(invalid, closing connection)",
                            stream, _logger, _traceLevels);
                        throw new Ice.InvalidMessageException();
                        break;
                    }

                    default:
                    {
                        TraceUtil.traceHeader(
                            "received unknown message\n" +
                            "(invalid, closing connection)",
                            stream, _logger, _traceLevels);
                        throw new Ice.UnknownMessageException();
                        break;
                    }
                }
            }
            catch (ConnectionLostException ex)
            {
                setState(StateClosed); // Connection drop from client is ok
                return;
            }
            catch (Ice.LocalException ex)
            {
                warning(ex);
                setState(StateClosed);
                return;
            }
        }
        finally
        {
            _mutex.unlock();
        }

        if (invoke)
        {
            do
            {
                try
                {
                    if (batch)
                    {
                        stream.startReadEncaps();
                    }

                    in.invoke(stream);

                    if (batch)
                    {
                        stream.swap(in.is()); // If we're in batch mode, we
                                              // need the input stream back
                        stream.endReadEncaps();
                    }
                }
                catch (Ice.LocalException ex)
                {
                    _mutex.lock();
                    warning(ex);
                    _mutex.unlock();
                }
                catch (Exception ex)
                {
                    assert(false); // Should not happen
                }
            }
            while (batch && stream.i < stream.b.end());
        }

        if (response)
        {
            _mutex.lock();

            try
            {
                if (_state != StateActive && _state != StateClosing)
                {
                    return;
                }

                try
                {
                    // TODO
/*
                    os.i = os.b.begin();
             
                    //
                    // Fill in the message size
                    //
                    const Byte* p;
                    Int sz = os.b.size();
                    p = reinterpret_cast<Byte*>(&sz);
                    copy(p, p + sizeof(Int), os->i + 3);
*/
                    
                    TraceUtil.traceReply("sending reply", os, _logger,
                                         _traceLevels);
                    _transceiver.write(os, _endpoint.timeout());
             
                    --_responseCount;
            
                    if (_state == StateClosing && _responseCount == 0)
                    {
                        closeConnection();
                    }
                }
                catch (ConnectionLostException ex)
                {
                    setState(StateClosed); // Connection drop from client is ok
                    return;
                }
                catch (Ice.LocalException ex)
                {
                    warning(ex);
                    setState(StateClosed);
                    return;
                }
            }
            finally
            {
                _mutex.unlock();
            }
        }
    }

    public void
    exception(Ice.LocalException ex)
    {
        _mutex.lock();
        try
        {
            if (_state != StateActive && _state != StateClosing)
            {
                return;
            }

            if (ex instanceof ConnectionLostException)
            {
                warning(ex);
            }

            setState(StateClosed);
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
            //
            // We also unregister with the thread pool if we go to holding
            // state, but in this case we may not close the connection.
            //
            if (_state == StateClosed)
            {
                _transceiver.close();
            }
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
            setState(StateClosing);
        }
        finally
        {
            _mutex.unlock();
        }

        return true;
    }

    //
    // Only for use by CollectorFactory
    //
    Collector(Instance instance,
              Ice.ObjectAdapter adapter,
              Transceiver transceiver,
              Endpoint endpoint)
    {
        super(instance);
        _adapter = adapter;
        _transceiver = transceiver;
        _endpoint = endpoint;
        _traceLevels = instance.traceLevels();
        _logger = instance.logger();
        _responseCount = 0;
        _state = StateHolding;

        try
        {
            String value = instance.properties().getProperty(
                "Ice.WarnAboutServerExceptions");
            _warnAboutExceptions = Integer.parseInt(value) > 0 ? true : false;
        }
        catch(NumberFormatException ex)
        {
            _warnAboutExceptions = false;
        }

        _threadPool = instance.threadPool();
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_state == StateClosed);
    }

    private static final int StateActive = 0;
    private static final int StateHolding = 1;
    private static final int StateClosing = 2;
    private static final int StateClosed = 3;

    private void
    setState(int state)
    {
        if (_endpoint.oneway() && state == StateClosing)
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
                {                           // to holding
                    return;
                }

                _threadPool.unregister(_transceiver.fd());
                break;
            }

            case StateClosing:
            {
                if (_state == StateClosed) // Can't change back from Closed
                {
                    return;
                }

                if (_responseCount == 0)
                {
                    try
                    {
                        closeConnection();
                    }
                    catch(ConnectionLostException ex)
                    {
                        state = StateClosed;
                        setState(state); // Connection drop from client is ok
                    }
                    catch(LocalException ex)
                    {
                        warning(ex);
                        state = StateClosed;
                        setState(state);
                    }
                }

                //
                // We need to continue to read data in closing state
                //
                if (_state == StateHolding)
                {
                    _threadPool._register(_transceiver.fd(), this);
                }
                break;
            }

            case StateClosed:
            {
                //
                // If we come from holding state, we are already unregistered,
                // so we can close right away.
                //
                if (_state == StateHolding)
                {
                    _transceiver.close();
                }
                else
                {
                    _threadPool.unregister(_transceiver.fd());
                }
                break;
            }
        }

        _state = state;
    }

    private void
    closeConection()
    {
        Stream os = new StreamI(_instance);
        os.writeByte(Protocol.protocolVersion);
        os.writeByte(Protocol.encodingVersion);
        os.writeByte(Protocol.closeConnectionMsg);
        os.writeByte(Protocol.headerSize); // Message size
        //os.i = os.b.begin(); // TODO
        TraceUtil.traceHeader("sending close connection", os, _logger,
                              _traceLevels);
        _transceiver.write(os, _endpoint.timeout());
        _transceiver.shutdown();
    }

    private void
    warning(Ice.LocalException ex)
    {
        if (_warnAboutExceptions)
        {
            String s = "server exception:\n" + ex + '\n' +
                _transceiver.toString();
            // TODO: Stack trace?
            _logger.warning(s);
        }
    }

    private Ice.ObjectAdapter _adapter;
    private Transceiver _transceiver;
    private Endpoint _endpoint;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private ThreadPool _threadPool;
    private _responseCount;
    private int _state;
    private boolean _warnAboutExceptions;
    private RecursiveMutex _mutex = new RecursiveMutex();
}
