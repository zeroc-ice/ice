// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
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

    public int
    initialize()
    {
        if(_state == StateNeedConnect)
        {
            _state = StateConnectPending;
            return SocketOperation.Connect;
        }
        else if(_state <= StateConnectPending)
        {
            try
            {
                Network.doFinishConnect(_fd);
                _state = StateConnected;
                _desc = Network.fdToString(_fd);
            }
            catch(Ice.LocalException ex)
            {
                if(_traceLevels.network >= 2)
                {
                    java.net.Socket fd = (java.net.Socket)_fd.socket();
                    StringBuilder s = new StringBuilder(128);
                    s.append("failed to establish tcp connection\n");
                    s.append("local address = ");
                    s.append(Network.addrToString(fd.getLocalAddress(), fd.getLocalPort()));
                    s.append("\nremote address = ");
                    assert(_connectAddr != null);
                    s.append(Network.addrToString(_connectAddr));
                    _logger.trace(_traceLevels.networkCat, s.toString());
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
        return SocketOperation.None;
    }

    public void
    close()
    {
        if(_state == StateConnected && _traceLevels.network >= 1)
        {
            String s = "closing tcp connection\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        assert(_fd != null);
        try
        {
                _fd.close();
        }
        catch(java.io.IOException ex)
        {
            throw new Ice.SocketException(ex);
        }
        finally
        {
            _fd = null;
        }
    }
    
    public boolean
    write(Buffer buf)
    {
        final int size = buf.b.limit();
        int packetSize = size - buf.b.position();

        //
        // Limit packet size to avoid performance problems on WIN32
        //
        if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize)
        {
            packetSize = _maxSendPacketSize;
            buf.b.limit(buf.b.position() + packetSize);
        }

        while(buf.b.hasRemaining())
        {
            try
            {
                assert(_fd != null);
                int ret = _fd.write(buf.b);

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
                    if(packetSize == _maxSendPacketSize)
                    {
                        buf.b.limit(size);
                    }
                    return false;
                }

                if(_traceLevels.network >= 3)
                {
                    String s = "sent " + ret + " of " + packetSize + " bytes via tcp\n" + toString();
                    _logger.trace(_traceLevels.networkCat, s);
                }

                if(_stats != null)
                {
                    _stats.bytesSent(type(), ret);
                }

                if(packetSize == _maxSendPacketSize)
                {
                    assert(buf.b.position() == buf.b.limit());
                    packetSize = size - buf.b.position();
                    if(packetSize > _maxSendPacketSize)
                    {
                        packetSize = _maxSendPacketSize;
                    }
                    buf.b.limit(buf.b.position() + packetSize);
                }
            }
            catch(java.io.InterruptedIOException ex)
            {
                continue;
            }
            catch(java.io.IOException ex)
            {
                throw new Ice.SocketException(ex);
            }
        }
        return true;
    }

    public boolean
    read(Buffer buf, Ice.BooleanHolder moreData)
    {
        int packetSize = buf.b.remaining();
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
                    return false;
                }
                
                if(ret > 0)
                {
                    if(_traceLevels.network >= 3)
                    {
                        String s = "received " + ret + " of " + packetSize + " bytes via tcp\n" + toString();
                        _logger.trace(_traceLevels.networkCat, s);
                    }

                    if(_stats != null)
                    {
                        _stats.bytesReceived(type(), ret);
                    }
                }

                packetSize = buf.b.remaining();
            }
            catch(java.io.InterruptedIOException ex)
            {
                continue;
            }
            catch(java.io.IOException ex)
            {
                throw new Ice.ConnectionLostException(ex);
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

    public Ice.ConnectionInfo
    getInfo()
    {
        assert(_fd != null);
        Ice.TCPConnectionInfo info = new Ice.TCPConnectionInfo();
        java.net.Socket socket = _fd.socket();
        info.localAddress = socket.getLocalAddress().getHostAddress();
        info.localPort = socket.getLocalPort();
        if(socket.getInetAddress() != null)
        {
            info.remoteAddress = socket.getInetAddress().getHostAddress();
            info.remotePort = socket.getPort();
        }
        else
        {
            info.remoteAddress = "";
            info.remotePort = -1;
        }
        return info;
    }

    public void
    checkSendSize(Buffer buf, int messageSizeMax)
    {
        if(buf.size() > messageSizeMax)
        {
            Ex.throwMemoryLimitException(buf.size(), messageSizeMax);
        }
    }

    //
    // Only for use by TcpConnector, TcpAcceptor
    //
    TcpTransceiver(Instance instance, java.nio.channels.SocketChannel fd, boolean connected,
                   java.net.InetSocketAddress connectAddr)
    {
        _fd = fd;
        _connectAddr = connectAddr;
        _traceLevels = instance.traceLevels();
        _logger = instance.initializationData().logger;
        _stats = instance.initializationData().stats;
        _state = connected ? StateConnected : StateNeedConnect;
        _desc = Network.fdToString(_fd);

        _maxSendPacketSize = 0;
        if(System.getProperty("os.name").startsWith("Windows"))
        {
            //
            // On Windows, limiting the buffer size is important to prevent
            // poor throughput performances when transfering large amount of
            // data. See Microsoft KB article KB823764.
            //
            _maxSendPacketSize = Network.getSendBufferSize(_fd) / 2;
            if(_maxSendPacketSize < 512)
            {
                _maxSendPacketSize = 0;
            }
        }
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        IceUtilInternal.Assert.FinalizerAssert(_fd == null);

        super.finalize();
    }

    private java.nio.channels.SocketChannel _fd;
    private java.net.InetSocketAddress _connectAddr;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private Ice.Stats _stats;
    private String _desc;
    private int _state;
    private int _maxSendPacketSize;
    
    private static final int StateNeedConnect = 0;
    private static final int StateConnectPending = 1;
    private static final int StateConnected = 2;
}
