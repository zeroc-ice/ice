// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class UdpTransceiver implements Transceiver
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
        //
        // Nothing to do.
        //
        return SocketOperation.None;
    }

    public void
    close()
    {
        assert(_fd != null);
        
        if(_state >= StateConnected && _traceLevels.network >= 1)
        {
            String s = "closing udp connection\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }
        
        try
        {
            _fd.close();
        }
        catch(java.io.IOException ex)
        {
        }
        _fd = null;
    }

    @SuppressWarnings("deprecation") 
    public boolean
    write(Buffer buf)
    {
        //
        // We don't want write or send to be called on android main thread as this will cause
        // NetworkOnMainThreadException to be thrown. If that is the android main thread
        // we return false and this method will be later called from the thread pool
        //
        if(Util.isAndroidMainThread(Thread.currentThread()))
        {
            return false;
        }

        assert(buf.b.position() == 0);
        assert(_fd != null && _state >= StateConnected);
                
        // The caller is supposed to check the send size before by calling checkSendSize
        assert(java.lang.Math.min(_maxPacketSize, _sndSize - _udpOverhead) >= buf.size());

        int ret = 0;
        while(true)
        {
            try
            {
                if(_state == StateConnected)
                {
                    ret = _fd.write(buf.b);
                }
                else
                {
                    if(_peerAddr == null)
                    {
                        throw new Ice.SocketException(); // No peer has sent a datagram yet.
                    }
                    ret = _fd.send(buf.b, _peerAddr);
                }
                break;
            }
            catch(java.nio.channels.AsynchronousCloseException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
            catch(java.net.PortUnreachableException ex)
            {
                throw new Ice.ConnectionLostException(ex);
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

        if(ret == 0)
        {
            return false;
        }

        if(_traceLevels.network >= 3)
        {
            String s = "sent " + ret + " bytes via udp\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        if(_stats != null)
        {
            _stats.bytesSent(type(), ret);
        }

        assert(ret == buf.b.limit());
        return true;
    }
    
    @SuppressWarnings("deprecation")
    public boolean
    read(Buffer buf, Ice.BooleanHolder moreData)
    {
        assert(buf.b.position() == 0);
        moreData.value = false;

        final int packetSize = java.lang.Math.min(_maxPacketSize, _rcvSize - _udpOverhead);
        buf.resize(packetSize, true);
        buf.b.position(0);

        int ret = 0;
        while(true)
        {
            try
            {
                java.net.SocketAddress peerAddr = _fd.receive(buf.b);
                if(peerAddr == null || buf.b.position() == 0)
                {
                    return false;
                }

                _peerAddr = (java.net.InetSocketAddress)peerAddr;
                ret = buf.b.position();
                break;
            }
            catch(java.nio.channels.AsynchronousCloseException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
            catch(java.net.PortUnreachableException ex)
            {
                throw new Ice.ConnectionLostException(ex);
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

        if(_state == StateNeedConnect)
        {
            //
            // If we must connect, we connect to the first peer that sends us a packet.
            //
            Network.doConnect(_fd, _peerAddr);
            _state = StateConnected;

            if(_traceLevels.network >= 1)
            {
                String s = "connected udp socket\n" + toString();
                _logger.trace(_traceLevels.networkCat, s);
            }
        }

        if(_traceLevels.network >= 3)
        {
            String s = "received " + ret + " bytes via udp\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        if(_stats != null)
        {
            _stats.bytesReceived(type(), ret);
        }

        buf.resize(ret, true);
        buf.b.position(ret);

        return true;
    }

    public String
    type()
    {
        return "udp";
    }

    public String
    toString()
    {
        if(_fd == null)
        {
            return "<closed>";
        }

        String s;
        if(_state == StateNotConnected)
        {
            java.net.DatagramSocket socket = ((java.nio.channels.DatagramChannel)_fd).socket();
            s = "local address = " + Network.addrToString((java.net.InetSocketAddress)socket.getLocalSocketAddress());
            if(_peerAddr != null)
            {
                s += "\nremote address = " + Network.addrToString(_peerAddr);
            }
        }
        else
        {
            s = Network.fdToString(_fd);
        }

        if(_mcastAddr != null)
        {
            s += "\nmulticast address = " + Network.addrToString(_mcastAddr);
        }
        return s;
    }

    public Ice.ConnectionInfo
    getInfo()
    {
        Ice.UDPConnectionInfo info = new Ice.UDPConnectionInfo();
        if(_fd != null)
        {
            java.net.DatagramSocket socket = _fd.socket();
            info.localAddress = socket.getLocalAddress().getHostAddress();
            info.localPort = socket.getLocalPort();
            if(_state == StateNotConnected)
            {
                if(_peerAddr != null)
                {
                    info.remoteAddress = _peerAddr.getAddress().getHostAddress();
                    info.remotePort = _peerAddr.getPort();
                }
            }
            else
            {
                if(socket.getInetAddress() != null)
                {
                    info.remoteAddress = socket.getInetAddress().getHostAddress();
                    info.remotePort = socket.getPort();
                }
            }
        }
        if(_mcastAddr != null)
        {
            info.mcastAddress = _mcastAddr.getAddress().getHostAddress();
            info.mcastPort = _mcastAddr.getPort();
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
        
        //
        // The maximum packetSize is either the maximum allowable UDP packet size, or 
        // the UDP send buffer size (which ever is smaller).
        //
        final int packetSize = java.lang.Math.min(_maxPacketSize, _sndSize - _udpOverhead);
        if(packetSize < buf.size())
        {
            throw new Ice.DatagramLimitException();
        }
    }

    public final int
    effectivePort()
    {
        return _addr.getPort();
    }

    //
    // Only for use by UdpEndpoint
    //
    @SuppressWarnings("deprecation")
    UdpTransceiver(Instance instance, java.net.InetSocketAddress addr, String mcastInterface, int mcastTtl)
    {
        _traceLevels = instance.traceLevels();
        _logger = instance.initializationData().logger;
        _stats = instance.initializationData().stats;
        _state = StateNeedConnect;
        _addr = addr;

        try
        {
            _fd = Network.createUdpSocket(_addr);
            setBufSize(instance);
            Network.setBlock(_fd, false);
            //
            // NOTE: setting the multicast interface before performing the
            // connect is important for some OS such as OS X.
            //
            if(_addr.getAddress().isMulticastAddress())
            {
                configureMulticast(null, mcastInterface, mcastTtl);
            }
            Network.doConnect(_fd, _addr);
            _state = StateConnected; // We're connected now

            if(_traceLevels.network >= 1)
            {
                String s = "starting to send udp packets\n" + toString();
                _logger.trace(_traceLevels.networkCat, s);
            }
        }
        catch(Ice.LocalException ex)
        {
            _fd = null;
            throw ex;
        }
    }

    //
    // Only for use by UdpEndpoint
    //
    @SuppressWarnings("deprecation")
    UdpTransceiver(Instance instance, String host, int port, String mcastInterface, boolean connect)
    {
        _traceLevels = instance.traceLevels();
        _logger = instance.initializationData().logger;
        _stats = instance.initializationData().stats;
        _state = connect ? StateNeedConnect : StateNotConnected;

        try
        {
            _addr = Network.getAddressForServer(host, port, instance.protocolSupport(), instance.preferIPv6());
            _fd = Network.createUdpSocket(_addr);
            setBufSize(instance);
            Network.setBlock(_fd, false);
            if(_traceLevels.network >= 2)
            {
                String s = "attempting to bind to udp socket " + Network.addrToString(_addr);
                _logger.trace(_traceLevels.networkCat, s);
            }
            if(_addr.getAddress().isMulticastAddress())
            {
                Network.setReuseAddress(_fd, true);
                _mcastAddr = _addr;
                if(System.getProperty("os.name").startsWith("Windows") ||
                   System.getProperty("java.vm.name").startsWith("OpenJDK"))
                {
                    //
                    // Windows does not allow binding to the mcast address itself
                    // so we bind to INADDR_ANY (0.0.0.0) instead. As a result,
                    // bi-directional connection won't work because the source 
                    // address won't be the multicast address and the client will
                    // therefore reject the datagram.
                    //
                    int protocol = 
                        _mcastAddr.getAddress().getAddress().length == 4 ? Network.EnableIPv4 : Network.EnableIPv6;
                    _addr = Network.getAddressForServer("", port, protocol, instance.preferIPv6());
                }
                _addr = Network.doBind(_fd, _addr);
                configureMulticast(_mcastAddr, mcastInterface, -1);

                if(port == 0)
                {
                    _mcastAddr = new java.net.InetSocketAddress(_mcastAddr.getAddress(), _addr.getPort());
                }
            }
            else
            {
                if(!System.getProperty("os.name").startsWith("Windows"))
                {
                    //
                    // Enable SO_REUSEADDR on Unix platforms to allow
                    // re-using the socket even if it's in the TIME_WAIT
                    // state. On Windows, this doesn't appear to be
                    // necessary and enabling SO_REUSEADDR would actually
                    // not be a good thing since it allows a second
                    // process to bind to an address even it's already
                    // bound by another process.
                    //
                    // TODO: using SO_EXCLUSIVEADDRUSE on Windows would
                    // probably be better but it's only supported by recent
                    // Windows versions (XP SP2, Windows Server 2003).
                    //
                    Network.setReuseAddress(_fd, true);
                }
                _addr = Network.doBind(_fd, _addr);
            }

            if(_traceLevels.network >= 1)
            {
                StringBuffer s = new StringBuffer("starting to receive udp packets\n");
                s.append(toString());

                java.util.List<String> interfaces = 
                    Network.getHostsForEndpointExpand(_addr.getAddress().getHostAddress(), instance.protocolSupport(),
                                                      true);
                if(!interfaces.isEmpty())
                {
                    s.append("\nlocal interfaces: ");
                    s.append(IceUtilInternal.StringUtil.joinString(interfaces, ", "));
                }
                _logger.trace(_traceLevels.networkCat, s.toString());
            }
        }
        catch(Ice.LocalException ex)
        {
            _fd = null;
            throw ex;
        }
    }

    private synchronized void
    setBufSize(Instance instance)
    {
        assert(_fd != null);

        for(int i = 0; i < 2; ++i)
        {
            String direction;
            String prop;
            int dfltSize;
            if(i == 0)
            {
                direction = "receive";
                prop = "Ice.UDP.RcvSize";
                dfltSize = Network.getRecvBufferSize(_fd);
                _rcvSize = dfltSize;
            }
            else
            {
                direction = "send";
                prop = "Ice.UDP.SndSize";
                dfltSize = Network.getSendBufferSize(_fd);
                _sndSize = dfltSize;
            }

            //
            // Get property for buffer size and check for sanity.
            //
            int sizeRequested = instance.initializationData().properties.getPropertyAsIntWithDefault(prop, dfltSize);
            if(sizeRequested < (_udpOverhead + IceInternal.Protocol.headerSize))
            {
                _logger.warning("Invalid " + prop + " value of " + sizeRequested + " adjusted to " + dfltSize);
                sizeRequested = dfltSize;
            }
                
            if(sizeRequested != dfltSize)
            {
                //
                // Try to set the buffer size. The kernel will silently adjust
                // the size to an acceptable value. Then read the size back to
                // get the size that was actually set.
                //
                int sizeSet;
                if(i == 0)
                {
                    Network.setRecvBufferSize(_fd, sizeRequested);
                    _rcvSize = Network.getRecvBufferSize(_fd);
                    sizeSet = _rcvSize;
                }
                else
                {
                    Network.setSendBufferSize(_fd, sizeRequested);
                    _sndSize = Network.getSendBufferSize(_fd);
                    sizeSet = _sndSize;
                }

                //
                // Warn if the size that was set is less than the requested size.
                //
                if(sizeSet < sizeRequested)
                {
                    _logger.warning("UDP " + direction + " buffer size: requested size of "
                                    + sizeRequested + " adjusted to " + sizeSet);
                }
            }
        }
    }

    //
    // The NIO classes before JDK 1.7 do not support multicast, at least not directly. 
    // This method works around that limitation by using reflection to configure the
    // file descriptor of a DatagramChannel for multicast operation. Specifically, an 
    // instance of java.net.PlainDatagramSocketImpl is use to (temporarily) wrap the 
    // channel's file descriptor.
    //
    // If using JDK >= 1.7 we use the new added MulticastChannel via reflection to allow 
    // compilation with older JDK versions.
    //
    private void
    configureMulticast(java.net.InetSocketAddress group, String interfaceAddr, int ttl)
    {
        try
        {
            Class<?> cls = Util.findClass("java.nio.channels.MulticastChannel", null);
            java.lang.reflect.Method m = null;
            java.net.DatagramSocketImpl socketImpl = null;
            java.lang.reflect.Field socketFd = null;
            java.net.NetworkInterface intf = null;
            if(cls == null || !cls.isAssignableFrom(_fd.getClass()))
            {
                cls = Util.findClass("java.net.PlainDatagramSocketImpl", null);
                if(cls == null)
                {
                    throw new Ice.SocketException();
                }
                java.lang.reflect.Constructor<?> c = cls.getDeclaredConstructor((Class<?>[])null);
                c.setAccessible(true);
                socketImpl = (java.net.DatagramSocketImpl)c.newInstance((Object[])null);

                //
                // We have to invoke the protected create() method on the PlainDatagramSocketImpl object so
                // that this hack works properly when IPv6 is enabled on Windows.
                //
                try
                {
                    m = cls.getDeclaredMethod("create", (Class<?>[])null);
                    m.setAccessible(true);
                    m.invoke(socketImpl);
                }
                catch(java.lang.NoSuchMethodException ex) // OpenJDK
                {
                }

                cls = Util.findClass("sun.nio.ch.DatagramChannelImpl", null);
                if(cls == null)
                {
                    throw new Ice.SocketException();
                }
                java.lang.reflect.Field channelFd = cls.getDeclaredField("fd");
                channelFd.setAccessible(true);

                socketFd = java.net.DatagramSocketImpl.class.getDeclaredField("fd");
                socketFd.setAccessible(true);
                socketFd.set(socketImpl, channelFd.get(_fd));
            }

            try 
            {
                if(interfaceAddr.length() != 0)
                {
                    intf = java.net.NetworkInterface.getByName(interfaceAddr);
                    if(intf == null)
                    {
                        java.net.InetSocketAddress addr = Network.getAddressForServer(interfaceAddr, 0, 
                                                                                      Network.EnableIPv4, false);
                        intf = java.net.NetworkInterface.getByInetAddress(addr.getAddress());
                    }
                }

                if(group != null)
                {
                    //
                    // Join multicast group.
                    // 
                    Class<?>[] types;
                    Object[] args;
                    if(socketImpl == null)
                    {
                        types = new Class<?>[]{ java.net.InetAddress.class, java.net.NetworkInterface.class };
                        m = _fd.getClass().getDeclaredMethod("join", types);
                        m.setAccessible(true);
                        boolean join = false;
                        if(intf != null)
                        {
                            m.invoke(_fd, new Object[] { group.getAddress(), intf });
                            join = true;
                        }
                        else
                        {
                            //
                            // If the user doesn't specify an interface, we join to the multicast group with all the
                            // interfaces that support multicast and has a configured address with the same protocol
                            // as the group address protocol.
                            //
                            int protocol = group.getAddress().getAddress().length == 4 ? Network.EnableIPv4 :
                                                                                         Network.EnableIPv6;

                            java.util.List<java.net.NetworkInterface> interfaces = 
                                        java.util.Collections.list(java.net.NetworkInterface.getNetworkInterfaces());
                            for(java.net.NetworkInterface iface : interfaces)
                            {
                                if(!iface.supportsMulticast())
                                {
                                    continue;
                                }
                                boolean hasProtocolAddress = false;
                                java.util.List<java.net.InetAddress> addresses =
                                                                java.util.Collections.list(iface.getInetAddresses());
                                for(java.net.InetAddress address : addresses)
                                {
                                    if(address.getAddress().length == 4 && protocol == Network.EnableIPv4 ||
                                       address.getAddress().length != 4 && protocol == Network.EnableIPv6)
                                    {
                                        hasProtocolAddress = true;
                                        break;
                                    }
                                }

                                if(hasProtocolAddress)
                                {
                                    m.invoke(_fd, new Object[] { group.getAddress(), iface });
                                    join = true;
                                }
                            }

                            if(!join)
                            {
                                throw new Ice.SocketException(new IllegalArgumentException(
                                                            "There aren't any interfaces that support multicast, " +
                                                            "or the interfaces that support it\n" +
                                                            "are not configured for the group protocol. " +
                                                            "Cannot join the mulitcast group."));
                            }
                        }
                    }
                    else
                    {
                        try
                        {
                            types = new Class<?>[]{ java.net.SocketAddress.class, java.net.NetworkInterface.class };
                            m = socketImpl.getClass().getDeclaredMethod("joinGroup", types);
                            args = new Object[]{ group, intf };
                        }
                        catch(java.lang.NoSuchMethodException ex) // OpenJDK
                        {                        
                            types = new Class<?>[]{ java.net.InetAddress.class, java.net.NetworkInterface.class };
                            m = socketImpl.getClass().getDeclaredMethod("join", types);
                            args = new Object[]{ group.getAddress(), intf };
                        }
                        m.setAccessible(true);
                        m.invoke(socketImpl, args);
                    }
                }
                else if(intf != null)
                {
                    //
                    // Otherwise, set the multicast interface if specified.
                    //

                    Class<?>[] types = new Class<?>[]{ Integer.TYPE, Object.class };
                    if(socketImpl == null)
                    {
                        Class<?> socketOption = Util.findClass("java.net.SocketOption", null);
                        Class<?> standardSocketOptions = Util.findClass("java.net.StandardSocketOptions", null);
                        m = _fd.getClass().getDeclaredMethod("setOption", new Class<?>[]{socketOption, Object.class});
                        m.setAccessible(true);
                        java.lang.reflect.Field ipMcastIf = standardSocketOptions.getDeclaredField("IP_MULTICAST_IF");
                        ipMcastIf.setAccessible(true);
                        m.invoke(_fd, new Object[]{ ipMcastIf.get(null), intf });
                    }
                    else
                    {
                        try
                        {
                            m = socketImpl.getClass().getDeclaredMethod("setOption", types);
                        }
                        catch(java.lang.NoSuchMethodException ex) // OpenJDK
                        {
                            m = socketImpl.getClass().getDeclaredMethod("socketSetOption", types);
                        }
                        m.setAccessible(true);
                        Object[] args = new Object[]{ Integer.valueOf(java.net.SocketOptions.IP_MULTICAST_IF2), intf };
                        m.invoke(socketImpl, args);
                    }
                }

                if(ttl != -1)
                {
                    if(socketImpl == null)
                    {
                        Class<?> socketOption = Util.findClass("java.net.SocketOption", null);
                        Class<?> standardSocketOptions = Util.findClass("java.net.StandardSocketOptions", null);
                        m = _fd.getClass().getDeclaredMethod("setOption", new Class<?>[]{socketOption, Object.class});
                        m.setAccessible(true);
                        java.lang.reflect.Field ipMcastTtl = standardSocketOptions.getDeclaredField("IP_MULTICAST_TTL");
                        ipMcastTtl.setAccessible(true);
                        m.invoke(_fd, new Object[]{ ipMcastTtl.get(null), ttl });
                    }
                    else
                    {
                        Class<?>[] types = new Class<?>[]{ Integer.TYPE };
                        m = java.net.DatagramSocketImpl.class.getDeclaredMethod("setTimeToLive", types);
                        m.setAccessible(true);
                        m.invoke(socketImpl, new Object[]{ Integer.valueOf(ttl) });
                    }
                }
            }
            finally 
            {
                if(socketFd != null && socketImpl != null)
                {
                    socketFd.set(socketImpl, null);
                }
            }
        }
        catch(Exception ex)
        {
            throw new Ice.SocketException(ex);
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

    private TraceLevels _traceLevels;
    private Ice.Logger _logger;

    @SuppressWarnings("deprecation")
    private Ice.Stats _stats;
    private int _state;
    private int _rcvSize;
    private int _sndSize;
    private java.nio.channels.DatagramChannel _fd;
    private java.net.InetSocketAddress _addr;
    private java.net.InetSocketAddress _mcastAddr = null;
    private java.net.InetSocketAddress _peerAddr = null;

    //
    // The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for the UDP header
    // to get the maximum payload.
    //
    private final static int _udpOverhead = 20 + 8;
    private final static int _maxPacketSize = 65535 - _udpOverhead;

    private static final int StateNeedConnect = 0;
    private static final int StateConnected = 1;
    private static final int StateNotConnected = 2;
}
