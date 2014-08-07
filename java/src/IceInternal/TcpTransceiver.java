// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class TcpTransceiver implements Transceiver
{
    @Override
    public java.nio.channels.SelectableChannel fd()
    {
        assert(_fd != null);
        return _fd;
    }

    @Override
    public int initialize(Buffer readBuffer, Buffer writeBuffer, Ice.Holder<Boolean> moreData)
    {
        try
        {
            if(_state == StateNeedConnect)
            {
                _state = StateConnectPending;
                return SocketOperation.Connect;
            }
            else if(_state <= StateConnectPending)
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
                    if(write(writeBuffer) == SocketOperation.None)
                    {
                        //
                        // Write completed without blocking.
                        //
                        _proxy.endWriteConnectRequest(writeBuffer);

                        //
                        // Try to read the response.
                        //
                        if(read(readBuffer, moreData) == SocketOperation.None)
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
            if(_instance.traceLevel() >= 2)
            {
                StringBuilder s = new StringBuilder(128);
                s.append("failed to establish " + _instance.protocol() + " connection\n");
                s.append(Network.fdToString(_fd, _proxy, _addr));
                _instance.logger().trace(_instance.traceCategory(), s.toString());
            }
            throw ex;
        }

        assert(_state == StateConnected);
        if(_instance.traceLevel() >= 1)
        {
            String s = _instance.protocol() + " connection established\n" + _desc;
            _instance.logger().trace(_instance.traceCategory(), s);
        }
        return SocketOperation.None;
    }

    @Override
    public int closing(boolean initiator, Ice.LocalException ex)
    {
        // If we are initiating the connection closure, wait for the peer
        // to close the TCP/IP connection. Otherwise, close immediately.
        return initiator ? SocketOperation.Read : SocketOperation.None;
    }

    @Override
    public void close()
    {
        if(_state == StateConnected && _instance.traceLevel() >= 1)
        {
            String s = "closing " + _instance.protocol() + " connection\n" + toString();
            _instance.logger().trace(_instance.traceCategory(), s);
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

    @Override
    public int write(Buffer buf)
    {
        final int size = buf.b.limit();
        int packetSize = size - buf.b.position();

        if(packetSize == 0)
        {
            return SocketOperation.None;
        }

        //
        // We don't want write to be called on Android's main thread as this will cause
        // NetworkOnMainThreadException to be thrown. If this is the Android main thread
        // we return false and this method will be called later from the thread pool.
        //
        if(Util.isAndroidMainThread(Thread.currentThread()))
        {
            return SocketOperation.Write;
        }

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
                    // Writing would block, so we reset the limit (if necessary) and indicate
                    // that more data must be sent.
                    //
                    if(packetSize == _maxSendPacketSize)
                    {
                        buf.b.limit(size);
                    }
                    return SocketOperation.Write;
                }

                if(_instance.traceLevel() >= 3)
                {
                    String s = "sent " + ret + " of " + packetSize + " bytes via " + _instance.protocol() + "\n" +
                        toString();
                    _instance.logger().trace(_instance.traceCategory(), s);
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

        return SocketOperation.None;
    }

    @Override
    public int read(Buffer buf, Ice.Holder<Boolean> moreData)
    {
        int packetSize = buf.b.remaining();
        if(packetSize == 0)
        {
            return SocketOperation.None;
        }

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
                    return SocketOperation.Read;
                }

                if(ret > 0)
                {
                    if(_instance.traceLevel() >= 3)
                    {
                        String s = "received " + ret + " of " + packetSize + " bytes via " + _instance.protocol() +
                            "\n" + toString();
                        _instance.logger().trace(_instance.traceCategory(), s);
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

        return SocketOperation.None;
    }

    @Override
    public String protocol()
    {
        return _instance.protocol();
    }

    @Override
    public String toString()
    {
        return _desc;
    }

    @Override
    public Ice.ConnectionInfo getInfo()
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

    @Override
    public void checkSendSize(Buffer buf, int messageSizeMax)
    {
        if(buf.size() > messageSizeMax)
        {
            Ex.throwMemoryLimitException(buf.size(), messageSizeMax);
        }
    }

    TcpTransceiver(ProtocolInstance instance, java.nio.channels.SocketChannel fd, NetworkProxy proxy,
                   java.net.InetSocketAddress addr)
    {
        _instance = instance;
        _fd = fd;
        _proxy = proxy;
        _addr = addr;
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

    TcpTransceiver(ProtocolInstance instance, java.nio.channels.SocketChannel fd)
    {
        _instance = instance;
        _fd = fd;
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

    @Override
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

    private ProtocolInstance _instance;
    private java.nio.channels.SocketChannel _fd;
    private NetworkProxy _proxy;
    private java.net.InetSocketAddress _addr;

    private int _state;
    private String _desc;
    private int _maxSendPacketSize;

    private static final int StateNeedConnect = 0;
    private static final int StateConnectPending = 1;
    private static final int StateProxyConnectRequest = 2;
    private static final int StateProxyConnectRequestPending = 3;
    private static final int StateConnected = 4;
}
