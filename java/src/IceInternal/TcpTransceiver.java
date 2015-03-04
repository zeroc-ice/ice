// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
    initialize(Buffer readBuffer, Buffer writeBuffer)
    {
        try
        {
            if(_state == StateNeedConnect)
            {
                _state = StateConnectPending;
                return SocketOperation.Connect;
            }
            else if(_state == StateConnectPending)
            {
                Network.doFinishConnect(_fd);
                _desc = Network.fdToString(_fd, _proxy, _addr);

                if(_proxy != null)
                {
                    //
                    // Prepare the read & write buffers in advance.
                    //
                    _proxy.beginWriteConnectRequest(_addr, writeBuffer);
                    _proxy.beginReadConnectRequestResponse(readBuffer);

                    //
                    // Write the proxy connection message.
                    //
                    if(write(writeBuffer))
                    {
                        //
                        // Write completed without blocking.
                        //
                        _proxy.endWriteConnectRequest(writeBuffer);

                        //
                        // Try to read the response.
                        //
                        Ice.BooleanHolder dummy = new Ice.BooleanHolder();
                        if(read(readBuffer, dummy))
                        {
                            //
                            // Read completed without blocking - fall through.
                            //
                            _proxy.endReadConnectRequestResponse(readBuffer);
                        }
                        else
                        {
                            //
                            // Return SocketOperationRead to indicate we need to complete the read.
                            //
                            _state = StateProxyConnectRequestPending; // Wait for proxy response
                            return SocketOperation.Read;
                        }
                    }
                    else
                    {
                        //
                        // Return SocketOperationWrite to indicate we need to complete the write.
                        //
                        _state = StateProxyConnectRequest; // Send proxy connect request
                        return SocketOperation.Write;
                    }
                }

                _state = StateConnected;
            }
            else if(_state == StateProxyConnectRequest)
            {
                //
                // Write completed.
                //
                _proxy.endWriteConnectRequest(writeBuffer);
                _state = StateProxyConnectRequestPending; // Wait for proxy response
                return SocketOperation.Read;
            }
            else if(_state == StateProxyConnectRequestPending)
            {
                //
                // Read completed.
                //
                _proxy.endReadConnectRequestResponse(readBuffer);
                _state = StateConnected;
            }
        }
        catch(Ice.LocalException ex)
        {
            if(_traceLevels.network >= 2)
            {
                StringBuilder s = new StringBuilder(128);
                s.append("failed to establish tcp connection\n");
                s.append(Network.fdToString(_fd, _proxy, _addr));
                _logger.trace(_traceLevels.networkCat, s.toString());
            }
            throw ex;
        }

        assert(_state == StateConnected);
        if(_traceLevels.network >= 1)
        {
            String s = "tcp connection established\n" + _desc;
            _logger.trace(_traceLevels.networkCat, s);
        }
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

    @SuppressWarnings("deprecation")
    public boolean
    write(Buffer buf)
    {
        //
        // We don't want write to be called on android main thread as this will cause
        // NetworkOnMainThreadException to be thrown. If that is the android main thread
        // we return false and this method will be later called from the thread pool.
        //
        if(Util.isAndroidMainThread(Thread.currentThread()))
        {
            return false;
        }

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
                    // Writing would block, so we reset the limit (if necessary) and return false to indicate
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

    @SuppressWarnings("deprecation")
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
        Ice.TCPConnectionInfo info = new Ice.TCPConnectionInfo();
        if(_fd != null)
        {
            java.net.Socket socket = _fd.socket();
            info.localAddress = socket.getLocalAddress().getHostAddress();
            info.localPort = socket.getLocalPort();
            if(socket.getInetAddress() != null)
            {
                info.remoteAddress = socket.getInetAddress().getHostAddress();
                info.remotePort = socket.getPort();
            }
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

    @SuppressWarnings("deprecation")
    TcpTransceiver(Instance instance, java.nio.channels.SocketChannel fd, NetworkProxy proxy,
                   java.net.InetSocketAddress addr)
    {
        _fd = fd;
        _proxy = proxy;
        _addr = addr;
        _traceLevels = instance.traceLevels();
        _logger = instance.initializationData().logger;
        _stats = instance.initializationData().stats;
        _state = StateNeedConnect;
        _desc = "";

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

    @SuppressWarnings("deprecation")
    TcpTransceiver(Instance instance, java.nio.channels.SocketChannel fd)
    {
        _fd = fd;
        _traceLevels = instance.traceLevels();
        _logger = instance.initializationData().logger;
        _stats = instance.initializationData().stats;
        _state = StateConnected;
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
        try
        {
            IceUtilInternal.Assert.FinalizerAssert(_fd == null);
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    private java.nio.channels.SocketChannel _fd;
    private NetworkProxy _proxy;
    private java.net.InetSocketAddress _addr;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;

    @SuppressWarnings("deprecation")
    private Ice.Stats _stats;
    private String _desc;
    private int _state;
    private int _maxSendPacketSize;

    private static final int StateNeedConnect = 0;
    private static final int StateConnectPending = 1;
    private static final int StateProxyConnectRequest = 2;
    private static final int StateProxyConnectRequestPending = 3;
    private static final int StateConnected = 4;
}
