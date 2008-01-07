// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class TcpTransceiver implements Transceiver
{
    public java.nio.channels.SelectableChannel
    fd()
    {
        assert(_fd != null);
        return _fd;
    }

    public SocketStatus
    initialize(int timeout)
    {
        if(_state == StateNeedConnect && timeout == 0)
        {
            _state = StateConnectPending;
            return SocketStatus.NeedConnect;
        }
        else if(_state <= StateConnectPending)
        {
            try
            {
                Network.doFinishConnect(_fd, timeout);
                _state = StateConnected;
                _desc = Network.fdToString(_fd);
            }
            catch(Ice.LocalException ex)
            {
                if(_traceLevels.network >= 2)
                {
                    String s = "failed to establish tcp connection\n" + _desc + "\n" + ex;
                    _logger.trace(_traceLevels.networkCat, s);
                }
                throw ex;
            }

            if(_traceLevels.network >= 1)
            {
                String s = "tcp connection established\n" + _desc;
                _logger.trace(_traceLevels.networkCat, s);
            }
        }
        assert(_state == StateConnected);
        return SocketStatus.Finished;
    }

    public void
    close()
    {
        if(_traceLevels.network >= 1)
        {
            String s = "closing tcp connection\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        synchronized(this)
        {
            assert(_fd != null);
            if(_readSelector != null)
            {
                try
                {
                    _readSelector.close();
                }
                catch(java.io.IOException ex)
                {
                    // Ignore.
                }
                _readSelector = null;
            }
            if(_writeSelector != null)
            {
                try
                {
                    _writeSelector.close();
                }
                catch(java.io.IOException ex)
                {
                    // Ignore.
                }
                _writeSelector = null;
            }
            try
            {
                _fd.close();
            }
            catch(java.io.IOException ex)
            {
                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }
            finally
            {
                _fd = null;
            }
        }
    }

    public void
    shutdownWrite()
    {
        if(_state < StateConnected)
        {
            return;
        }

        if(_traceLevels.network >= 2)
        {
            String s = "shutting down tcp connection for writing\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        assert(_fd != null);
        java.net.Socket socket = _fd.socket();
        try
        {
            socket.shutdownOutput(); // Shutdown socket for writing
        }
        catch(java.net.SocketException ex)
        {
            //
            // Ignore. We can't reliably figure out if the socket
            // exception is because the socket is not connected.
            //
            // if(!Network.notConnected(ex))
            // {
            //     Ice.SocketException se = new Ice.SocketException();
            //     se.initCause(ex);
            //     throw se;
            // }
        }
        catch(java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public void
    shutdownReadWrite()
    {
        if(_state < StateConnected)
        {
            return;
        }

        if(_traceLevels.network >= 2)
        {
            String s = "shutting down tcp connection for reading and writing\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        assert(_fd != null);
        java.net.Socket socket = _fd.socket();
        try
        {
            socket.shutdownInput(); // Shutdown socket for reading
            socket.shutdownOutput(); // Shutdown socket for writing
        }
        catch(java.net.SocketException ex)
        {
            //
            // Ignore. We can't reliably figure out if the socket
            // exception is because the socket is not connected.
            //
            // if(!Network.notConnected(ex))
            // {
            //     Ice.SocketException se = new Ice.SocketException();
            //     se.initCause(ex);
            //     throw se;
            // }
        }
        catch(java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public boolean
    write(Buffer buf, int timeout)
    {
        while(writeBuffer(buf.b))
        {
            //
            // There is more data to write but the socket would block; now we
            // must deal with timeouts.
            //
            assert(buf.b.hasRemaining());

            if(timeout == 0)
            {
                return false;
            }
                    
            try
            {
                if(_writeSelector == null)
                {
                    _writeSelector = java.nio.channels.Selector.open();
                    _fd.register(_writeSelector, java.nio.channels.SelectionKey.OP_WRITE, null);
                }
                
                try
                {
                    if(timeout > 0)
                    {
                        long start = IceInternal.Time.currentMonotonicTimeMillis();
                        int n = _writeSelector.select(timeout);
                        if(n == 0 && IceInternal.Time.currentMonotonicTimeMillis() >= start + timeout)
                        {
                            throw new Ice.TimeoutException();
                        }
                    }
                    else
                    {
                        _writeSelector.select();
                    }
                }
                catch(java.io.InterruptedIOException ex)
                {
                    // Ignore.
                }
            }
            catch(java.io.IOException ex)
            {
                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }
        }        
        return true;
    }

    public boolean
    read(Buffer buf, int timeout, Ice.BooleanHolder moreData)
    {
        int remaining = 0;
        if(_traceLevels.network >= 3)
        {
            remaining = buf.b.remaining();
        }
        moreData.value = false;

        while(buf.b.hasRemaining())
        {
            try
            {
                assert(_fd != null);
                int ret = _fd.read(buf.b);
                
                if(ret == -1)
                {
                    throw new Ice.ConnectionLostException();
                }
                
                if(ret == 0)
                {
                    if(timeout == 0)
                    {
                        return false;
                    }

                    if(_readSelector == null)
                    {
                        _readSelector = java.nio.channels.Selector.open();
                        _fd.register(_readSelector, java.nio.channels.SelectionKey.OP_READ, null);
                    }
                    
                    try
                    {
                        if(timeout > 0)
                        {
                            long start = IceInternal.Time.currentMonotonicTimeMillis();
                            int n = _readSelector.select(timeout);
                            if(n == 0 && IceInternal.Time.currentMonotonicTimeMillis() >= start + timeout)
                            {
                                throw new Ice.TimeoutException();
                            }
                        }
                        else
                        {
                            _readSelector.select();
                        }
                    }
                    catch(java.io.InterruptedIOException ex)
                    {
                        // Ignore.
                    }

                    continue;
                }
                
                if(ret > 0)
                {
                    if(_traceLevels.network >= 3)
                    {
                        String s = "received " + ret + " of " + remaining + " bytes via tcp\n" + toString();
                        _logger.trace(_traceLevels.networkCat, s);
                    }

                    if(_stats != null)
                    {
                        _stats.bytesReceived(type(), ret);
                    }
                }
            }
            catch(java.io.InterruptedIOException ex)
            {
                continue;
            }
            catch(java.io.IOException ex)
            {
                if(Network.connectionLost(ex))
                {
                    Ice.ConnectionLostException se = new Ice.ConnectionLostException();
                    se.initCause(ex);
                    throw se;
                }
                
                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }
        }

        return true;
    }

    public String
    type()
    {
        return "tcp";
    }

    public String
    toString()
    {
        return _desc;
    }

    public void
    checkSendSize(Buffer buf, int messageSizeMax)
    {
        if(buf.size() > messageSizeMax)
        {
            throw new Ice.MemoryLimitException();
        }
    }

    //
    // Only for use by TcpConnector, TcpAcceptor
    //
    TcpTransceiver(Instance instance, java.nio.channels.SocketChannel fd, boolean connected)
    {
        _fd = fd;
        _traceLevels = instance.traceLevels();
        _logger = instance.initializationData().logger;
        _stats = instance.initializationData().stats;
        _state = connected ? StateConnected : StateNeedConnect;
        _desc = Network.fdToString(_fd);

        _maxPacketSize = 0;
        if(System.getProperty("os.name").startsWith("Windows"))
        {
            //
            // On Windows, limiting the buffer size is important to prevent
            // poor throughput performances when transfering large amount of
            // data. See Microsoft KB article KB823764.
            //
            _maxPacketSize = Network.getSendBufferSize(_fd) / 2;
            if(_maxPacketSize < 512)
            {
                _maxPacketSize = 0;
            }
        }
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        IceUtil.Assert.FinalizerAssert(_fd == null);

        super.finalize();
    }

    private boolean
    writeBuffer(java.nio.ByteBuffer buf)
    {
        final int size = buf.limit();
        int packetSize = size - buf.position();
        if(_maxPacketSize > 0 && packetSize > _maxPacketSize)
        {
            packetSize = _maxPacketSize;
            buf.limit(buf.position() + packetSize);
        }

        while(buf.hasRemaining())
        {
            try
            {
                assert(_fd != null);
                int ret = _fd.write(buf);

                if(ret == -1)
                {
                    throw new Ice.ConnectionLostException();
                }
                else if(ret == 0)
                {
                    //
                    // Writing would block, so we reset the limit (if necessary) and return true to indicate
                    // that more data must be sent.
                    //
                    if(packetSize == _maxPacketSize)
                    {
                        buf.limit(size);
                    }
                    return true;
                }

                if(_traceLevels.network >= 3)
                {
                    String s = "sent " + ret + " of " + size + " bytes via tcp\n" + toString();
                    _logger.trace(_traceLevels.networkCat, s);
                }

                if(_stats != null)
                {
                    _stats.bytesSent(type(), ret);
                }

                if(packetSize == _maxPacketSize)
                {
                    assert(buf.position() == buf.limit());
                    packetSize = size - buf.position();
                    if(packetSize > _maxPacketSize)
                    {
                        packetSize = _maxPacketSize;
                    }
                    buf.limit(buf.position() + packetSize);
                }
            }
            catch(java.io.InterruptedIOException ex)
            {
                continue;
            }
            catch(java.io.IOException ex)
            {
                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }
        }

        return false; // No more data to send.
    }

    private java.nio.channels.SocketChannel _fd;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private Ice.Stats _stats;
    private String _desc;
    private int _state;
    private java.nio.channels.Selector _readSelector;
    private java.nio.channels.Selector _writeSelector;
    private int _maxPacketSize;
    
    private static final int StateNeedConnect = 0;
    private static final int StateConnectPending = 1;
    private static final int StateConnected = 2;
}
