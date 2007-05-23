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

    public void
    write(BasicStream stream, int timeout)
        throws LocalExceptionWrapper
    {
        java.nio.ByteBuffer buf = stream.prepareWrite();
        int size = buf.limit();
        int packetSize = 0;
        if(_maxPacketSize > 0 && size > _maxPacketSize)
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
                
                if(ret == 0)
                {
                    if(timeout == 0)
                    {
                        throw new Ice.TimeoutException();
                    }
                    
                    if(_writeSelector == null)
                    {
                        _writeSelector = java.nio.channels.Selector.open();
                        _fd.register(_writeSelector, java.nio.channels.SelectionKey.OP_WRITE, null);
                    }
                    
                    try
                    {
                        if(timeout > 0)
                        {
                            long start = System.currentTimeMillis();
                            int n = _writeSelector.select(timeout);
                            if(n == 0 && System.currentTimeMillis() >= start + timeout)
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

                    continue;
                }

                
                if(_traceLevels.network >= 3)
                {
                    String s = "sent " + ret + " of " + buf.limit() + " bytes via tcp\n" + toString();
                    _logger.trace(_traceLevels.networkCat, s);
                }

                if(_stats != null)
                {
                    _stats.bytesSent(type(), ret);
                }

                if(packetSize > 0)
                {
                    assert(buf.position() == buf.limit());
                    int position = buf.position();
                    if(size - position > packetSize)
                    {
                        buf.limit(position + packetSize);
                    }
                    else
                    {
			packetSize = 0;
                        buf.limit(size);
                    }
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
    }

    public boolean
    read(BasicStream stream, int timeout)
    {
        java.nio.ByteBuffer buf = stream.prepareRead();

        int remaining = 0;
        if(_traceLevels.network >= 3)
        {
            remaining = buf.remaining();
        }

        while(buf.hasRemaining())
        {
            try
            {
                assert(_fd != null);
                int ret = _fd.read(buf);
                
                if(ret == -1)
                {
                    throw new Ice.ConnectionLostException();
                }
                
                if(ret == 0)
                {
                    if(timeout == 0)
                    {
                        throw new Ice.TimeoutException();
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
                            long start = System.currentTimeMillis();
                            int n = _readSelector.select(timeout);
                            if(n == 0 && System.currentTimeMillis() >= start + timeout)
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

        return false;
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
    checkSendSize(BasicStream stream, int messageSizeMax)
    {
        if(stream.size() > messageSizeMax)
        {
            throw new Ice.MemoryLimitException();
        }
    }

    //
    // Only for use by TcpConnector, TcpAcceptor
    //
    TcpTransceiver(Instance instance, java.nio.channels.SocketChannel fd)
    {
        _fd = fd;
        _traceLevels = instance.traceLevels();
        _logger = instance.initializationData().logger;
        _stats = instance.initializationData().stats;
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

    private java.nio.channels.SocketChannel _fd;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private Ice.Stats _stats;
    private String _desc;
    private java.nio.channels.Selector _readSelector;
    private java.nio.channels.Selector _writeSelector;
    private int _maxPacketSize;
}
