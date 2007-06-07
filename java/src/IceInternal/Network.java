// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class Network
{
    public static boolean
    connectionLost(java.io.IOException ex)
    {
        //
        // TODO: The JDK raises a generic IOException for certain
        // cases of connection loss. Unfortunately, our only choice is
        // to search the exception message for distinguishing phrases.
        //
        String msg = ex.getMessage();
        if(msg != null)
        {
            msg = msg.toLowerCase();

            final String[] msgs =
            {
                "connection reset by peer", // ECONNRESET
                "cannot send after socket shutdown", // ESHUTDOWN (Win32)
                "cannot send after transport endpoint shutdown", // ESHUTDOWN (Linux)
                "software caused connection abort", // ECONNABORTED
                "an existing connection was forcibly closed", // unknown
                "connection closed by remote host", // unknown
                "an established connection was aborted by the software in your host machine", // unknown (Win32)
                "broken pipe", // EPIPE
                "there is no process to read data written to a pipe", // EPIPE? (AIX JDK 1.4.2)
                "socket is closed" // unknown (AIX JDK 1.4.2)
            };

            for(int i = 0; i < msgs.length; i++)
            {
                if(msg.indexOf(msgs[i]) != -1)
                {
                    return true;
                }
            }
        }

        return false;
    }

    public static boolean
    connectionRefused(java.net.ConnectException ex)
    {
        //
        // The JDK raises a generic ConnectException when the server
        // actively refuses a connection. Unfortunately, our only
        // choice is to search the exception message for
        // distinguishing phrases.
        //

        String msg = ex.getMessage().toLowerCase();

        if(msg != null)
        {
            final String[] msgs =
            {
                "connection refused", // ECONNREFUSED
                "remote host refused an attempted connect operation" // ECONNREFUSED (AIX JDK 1.4.2)
            };

            for(int i = 0; i < msgs.length; i++)
            {
                if(msg.indexOf(msgs[i]) != -1)
                {
                    return true;
                }
            }
        }

        return false;
    }

    //
    // This method is not used anymore. See comments in
    // TcpTransceiverI.java.
    //
//     public static boolean
//     notConnected(java.net.SocketException ex)
//     {
//         String msg = ex.getMessage().toLowerCase();
//         if(msg.indexOf("transport endpoint is not connected") != -1)
//         {
//             return true;
//         }
//         //
//         // BUGFIX: We check for EINVAL because shutdown() under Mac OS
//         // X returns EINVAL if the server side is gone.
//         //
//         else if(msg.indexOf("invalid argument") != -1)
//         {
//             return true;
//         }
//         return false;
//     }

    public static java.nio.channels.SocketChannel
    createTcpSocket()
    {
        try
        {
            java.nio.channels.SocketChannel fd = java.nio.channels.SocketChannel.open();
            java.net.Socket socket = fd.socket();
            socket.setTcpNoDelay(true);
            socket.setKeepAlive(true);
            return fd;
        }
        catch(java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static java.nio.channels.ServerSocketChannel
    createTcpServerSocket()
    {
        try
        {
            java.nio.channels.ServerSocketChannel fd = java.nio.channels.ServerSocketChannel.open();
            //
            // It's not possible to set TCP_NODELAY or KEEP_ALIVE
            // on a server socket in Java
            //
            //java.net.Socket socket = fd.socket();
            //socket.setTcpNoDelay(true);
            //socket.setKeepAlive(true);
            return fd;
        }
        catch(java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static java.nio.channels.DatagramChannel
    createUdpSocket()
    {
        try
        {
            return java.nio.channels.DatagramChannel.open();
        }
        catch(java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    private static void
    closeSocketNoThrow(java.nio.channels.SelectableChannel fd)
    {
        try
        {
            fd.close();
        }
        catch(java.io.IOException ex)
        {
            // Ignore
        }
    }

    public static void
    closeSocket(java.nio.channels.SelectableChannel fd)
    {
        try
        {
            fd.close();
        }
        catch(java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static void
    setBlock(java.nio.channels.SelectableChannel fd, boolean block)
    {
        try
        {
            fd.configureBlocking(block);
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static void
    setReuseAddress(java.nio.channels.DatagramChannel fd, boolean reuse)
    {
        try
        {
            fd.socket().setReuseAddress(reuse);
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static void
    setReuseAddress(java.nio.channels.ServerSocketChannel fd, boolean reuse)
    {
        try
        {
            fd.socket().setReuseAddress(reuse);
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static java.net.InetSocketAddress
    doBind(java.nio.channels.ServerSocketChannel fd, java.net.InetSocketAddress addr)
    {
        try
        {
            java.net.ServerSocket sock = fd.socket();
            sock.bind(addr);
            return (java.net.InetSocketAddress)sock.getLocalSocketAddress();
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static java.net.InetSocketAddress
    doBind(java.nio.channels.DatagramChannel fd, java.net.InetSocketAddress addr)
    {
        try
        {
            java.net.DatagramSocket sock = fd.socket();
            sock.bind(addr);
            return (java.net.InetSocketAddress)sock.getLocalSocketAddress();
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static void
    doConnect(java.nio.channels.SocketChannel fd, java.net.InetSocketAddress addr, int timeout)
    {
        try
        {
            if(!fd.connect(addr))
            {
                java.nio.channels.Selector selector = java.nio.channels.Selector.open();
                try
                {
                    while(true)
                    {
                        try
                        {
                            java.nio.channels.SelectionKey key =
                                fd.register(selector, java.nio.channels.SelectionKey.OP_CONNECT);
                            int n;
                            if(timeout > 0)
                            {
                                n = selector.select(timeout);
                            }
                            else if(timeout == 0)
                            {
                                n = selector.selectNow();
                            }
                            else
                            {
                                n = selector.select();
                            }
                            
                            if(n == 0)
                            {
                                closeSocketNoThrow(fd);
                                throw new Ice.ConnectTimeoutException();
                            }
                            
                            break;
                        }
                        catch(java.io.IOException ex)
                        {
                            if(interrupted(ex))
                            {
                                continue;
                            }
                            Ice.SocketException se = new Ice.SocketException();
                            se.initCause(ex);
                            throw se;
                        }
                    }
                }
                finally
                {
                    try
                    {
                        selector.close();
                    }
                    catch(java.io.IOException ex)
                    {
                        // Ignore
                    }
                }

                if(!fd.finishConnect())
                {
                    throw new Ice.ConnectFailedException();
                }
            }
        }
        catch(java.net.ConnectException ex)
        {
            closeSocketNoThrow(fd);

            Ice.ConnectFailedException se;
            if(connectionRefused(ex))
            {
                se = new Ice.ConnectionRefusedException();
            }
            else
            {
                se = new Ice.ConnectFailedException();
            }
            se.initCause(ex);
            throw se;
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static void
    doConnect(java.nio.channels.DatagramChannel fd, java.net.InetSocketAddress addr, int timeout)
    {
        try
        {
            fd.connect(addr);
        }
        catch(java.net.ConnectException ex)
        {
            closeSocketNoThrow(fd);

            Ice.ConnectFailedException se;
            if(connectionRefused(ex))
            {
                se = new Ice.ConnectionRefusedException();
            }
            else
            {
                se = new Ice.ConnectFailedException();
            }
            se.initCause(ex);
            throw se;
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static java.nio.channels.SocketChannel
    doAccept(java.nio.channels.ServerSocketChannel fd, int timeout)
    {
        java.nio.channels.SocketChannel result = null;
        while(result == null)
        {
            try
            {
                result = fd.accept();
                if(result == null)
                {
                    java.nio.channels.Selector selector = java.nio.channels.Selector.open();

                    try
                    {
                        while(true)
                        {
                            try
                            {
                                java.nio.channels.SelectionKey key =
                                    fd.register(selector, java.nio.channels.SelectionKey.OP_ACCEPT);
                                int n;
                                if(timeout > 0)
                                {
                                    n = selector.select(timeout);
                                }
                                else if(timeout == 0)
                                {
                                    n = selector.selectNow();
                                }
                                else
                                {
                                    n = selector.select();
                                }

                                if(n == 0)
                                {
                                    throw new Ice.TimeoutException();
                                }

                                break;
                            }
                            catch(java.io.IOException ex)
                            {
                                if(interrupted(ex))
                                {
                                    continue;
                                }
                                Ice.SocketException se = new Ice.SocketException();
                                se.initCause(ex);
                                throw se;
                            }
                        }
                    }
                    finally
                    {
                        try
                        {
                            selector.close();
                        }
                        catch(java.io.IOException ex)
                        {
                            // Ignore
                        }
                    }
                }
            }
            catch(java.io.IOException ex)
            {
                if(interrupted(ex))
                {
                    continue;
                }
                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }
        }

        try
        {
            java.net.Socket socket = result.socket();
            socket.setTcpNoDelay(true);
            socket.setKeepAlive(true);
        }
        catch(java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }

        return result;
    }

    public static void
    setSendBufferSize(java.nio.channels.SocketChannel fd, int size)
    {
        try
        {
            java.net.Socket socket = fd.socket();
            socket.setSendBufferSize(size);
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static int
    getSendBufferSize(java.nio.channels.SocketChannel fd)
    {
        int size;
        try
        {
            java.net.Socket socket = fd.socket();
            size = socket.getSendBufferSize();
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
        return size;
    }

    public static void
    setRecvBufferSize(java.nio.channels.SocketChannel fd, int size)
    {
        try
        {
            java.net.Socket socket = fd.socket();
            socket.setReceiveBufferSize(size);
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static int
    getRecvBufferSize(java.nio.channels.SocketChannel fd)
    {
        int size;
        try
        {
            java.net.Socket socket = fd.socket();
            size = socket.getReceiveBufferSize();
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
        return size;
    }

    public static void
    setRecvBufferSize(java.nio.channels.ServerSocketChannel fd, int size)
    {
        try
        {
            java.net.ServerSocket socket = fd.socket();
            socket.setReceiveBufferSize(size);
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static int
    getRecvBufferSize(java.nio.channels.ServerSocketChannel fd)
    {
        int size;
        try
        {
            java.net.ServerSocket socket = fd.socket();
            size = socket.getReceiveBufferSize();
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
        return size;
    }

    public static void
    setSendBufferSize(java.nio.channels.DatagramChannel fd, int size)
    {
        try
        {
            java.net.DatagramSocket socket = fd.socket();
            socket.setSendBufferSize(size);
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static int
    getSendBufferSize(java.nio.channels.DatagramChannel fd)
    {
        int size;
        try
        {
            java.net.DatagramSocket socket = fd.socket();
            size = socket.getSendBufferSize();
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
        return size;
    }

    public static void
    setRecvBufferSize(java.nio.channels.DatagramChannel fd, int size)
    {
        try
        {
            java.net.DatagramSocket socket = fd.socket();
            socket.setReceiveBufferSize(size);
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static int
    getRecvBufferSize(java.nio.channels.DatagramChannel fd)
    {
        int size;
        try
        {
            java.net.DatagramSocket socket = fd.socket();
            size = socket.getReceiveBufferSize();
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
        return size;
    }

    public static java.net.InetSocketAddress
    getAddress(String host, int port)
    {
        try
        {
            java.net.InetAddress[] addrs = java.net.InetAddress.getAllByName(host);
            for(int i = 0; i < addrs.length; ++i)
            {
                if(addrs[i] instanceof java.net.Inet4Address)
                {
                    return new java.net.InetSocketAddress(addrs[i], port);
                }
            }
        }
        catch(java.net.UnknownHostException ex)
        {
            Ice.DNSException e = new Ice.DNSException();
            e.host = host;
            throw e;
        }

        //
        // No Inet4Address available.
        //
        Ice.DNSException e = new Ice.DNSException();
        e.host = host;
        throw e;
    }

    public static int
    compareAddress(java.net.InetSocketAddress addr1, java.net.InetSocketAddress addr2)
    {
        if(addr1.getPort() < addr2.getPort())
        {
            return -1;
        }
        else if(addr2.getPort() < addr1.getPort())
        {
            return 1;
        }

        byte[] larr = addr1.getAddress().getAddress();
        byte[] rarr = addr2.getAddress().getAddress();
        assert(larr.length == rarr.length);
        for(int i = 0; i < larr.length; i++)
        {
            if(larr[i] < rarr[i])
            {
                return -1;
            }
            else if(rarr[i] < larr[i])
            {
                return 1;
            }
        }

        return 0;
    }

    public static java.net.InetAddress
    getLocalAddress()
    {
        java.net.InetAddress addr = null;

        try
        {
            addr = java.net.InetAddress.getLocalHost();
        }
        catch(java.net.UnknownHostException ex)
        {
            //
            // May be raised on DHCP systems.
            //
        }
        catch(NullPointerException ex)
        {
            //
            // Workaround for bug in JDK.
            //
        }

        if(addr == null || addr instanceof java.net.Inet6Address)
        {
            //
            // Iterate over the network interfaces and pick an IP
            // address (preferably not the loopback address).
            //
            java.net.InetAddress loopback = null;
            java.util.ArrayList addrs = getLocalAddresses();
            java.util.Iterator iter = addrs.iterator();
            while(addr == null && iter.hasNext())
            {
                java.net.InetAddress a = (java.net.InetAddress)iter.next();
                if(!a.isLoopbackAddress())
                {
                    addr = a;
                }
                else
                {
                    loopback = a;
                }
            }

            if(addr == null)
            {
                addr = loopback; // Use the loopback address as the last resort.
            }
        }

        assert(addr != null);
        return addr;
    }

    public static java.util.ArrayList
    getAddresses(String host, int port)
    {
        java.util.ArrayList addresses = new java.util.ArrayList();
        try
        {
            java.net.InetAddress[] addrs = java.net.InetAddress.getAllByName(host);
            for(int i = 0; i < addrs.length; ++i)
            {
                if(addrs[i] instanceof java.net.Inet4Address)
                {
                    addresses.add(new java.net.InetSocketAddress(addrs[i], port));
                }
            }
        }
        catch(java.net.UnknownHostException ex)
        {
            Ice.DNSException e = new Ice.DNSException();
            e.host = host;
            throw e;
        }

        //
        // No Inet4Address available.
        //
        if(addresses.size() == 0)
        {
            Ice.DNSException e = new Ice.DNSException();
            e.host = host;
            throw e;
        }

        return addresses;
    }

    public static java.util.ArrayList
    getLocalHosts()
    {
        java.util.ArrayList hosts = new java.util.ArrayList();
        java.util.ArrayList addrs = getLocalAddresses();
        java.util.Iterator iter = addrs.iterator();
        while(iter.hasNext())
        {
            hosts.add(((java.net.InetAddress)iter.next()).getHostAddress());
        }
        return hosts;
    }

    public static java.util.ArrayList
    getLocalAddresses()
    {
        java.util.ArrayList result = new java.util.ArrayList();

        try
        {
            java.util.Enumeration ifaces = java.net.NetworkInterface.getNetworkInterfaces();
            while(ifaces.hasMoreElements())
            {
                java.net.NetworkInterface iface = (java.net.NetworkInterface)ifaces.nextElement();
                java.util.Enumeration addrs = iface.getInetAddresses();
                while(addrs.hasMoreElements())
                {
                    java.net.InetAddress addr = (java.net.InetAddress)addrs.nextElement();
                    if(!(addr instanceof java.net.Inet6Address))
                    {
                        result.add(addr);
                    }
                }
            }
        }
        catch(java.net.SocketException e)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(e);
            throw se;
        }

        return result;
    }

    public static final class SocketPair
    {
        public java.nio.channels.spi.AbstractSelectableChannel source;
        public java.nio.channels.WritableByteChannel sink;
    }

    public static SocketPair
    createPipe()
    {
        SocketPair fds = new SocketPair();

        //
        // BUGFIX: This method should really be very simple.
        // Unfortunately, using a pipe causes a kernel crash under
        // MacOS 10.3.9.
        //
        //try
        //{
        //   java.nio.channels.Pipe pipe = java.nio.channels.Pipe.open();
        //   fds.sink = pipe.sink();
        //   fds.source = pipe.source();
        //}
        //catch(java.io.IOException ex)
        //{
        //   Ice.SocketException se = new Ice.SocketException();
        //   se.initCause(ex);
        //   throw se;
        //}
        //
        
        java.nio.channels.ServerSocketChannel fd = createTcpServerSocket();
        
        java.net.InetSocketAddress addr = new java.net.InetSocketAddress("127.0.0.1", 0);
        
        addr = doBind(fd, addr);
        
        try
        {
            java.nio.channels.SocketChannel sink = createTcpSocket();
            fds.sink = sink;
            doConnect(sink, addr, -1);
            try
            {
                fds.source = doAccept(fd, -1);
            }
            catch(Ice.LocalException ex)
            {
                try
                {
                    fds.sink.close();
                }
                catch(java.io.IOException e)
                {
                }
                throw ex;
            }
        }
        finally
        {
            closeSocketNoThrow(fd);
        }
        
        return fds;
    }
    
    public static void
    setTcpBufSize(java.nio.channels.SocketChannel socket, Ice.Properties properties, Ice.Logger logger)
    {
        //
        // By default, on Windows we use a 128KB buffer size. On Unix
        // platforms, we use the system defaults.
        //
        int dfltBufSize = 0;
        if(System.getProperty("os.name").startsWith("Windows"))
        {
            dfltBufSize = 128 * 1024;
        }

        int sizeRequested = properties.getPropertyAsIntWithDefault("Ice.TCP.RcvSize", dfltBufSize);
        if(sizeRequested > 0)
        {
            //
            // Try to set the buffer size. The kernel will silently adjust
            // the size to an acceptable value. Then read the size back to
            // get the size that was actually set.
            //
            setRecvBufferSize(socket, sizeRequested);
            int size = getRecvBufferSize(socket);
            if(size < sizeRequested) // Warn if the size that was set is less than the requested size.
            {
                logger.warning("TCP receive buffer size: requested size of " + sizeRequested + " adjusted to " + size);
            }
        }

        sizeRequested = properties.getPropertyAsIntWithDefault("Ice.TCP.SndSize", dfltBufSize);
        if(sizeRequested > 0)
        {
            //
            // Try to set the buffer size. The kernel will silently adjust
            // the size to an acceptable value. Then read the size back to
            // get the size that was actually set.
            //
            setSendBufferSize(socket, sizeRequested);
            int size = getSendBufferSize(socket);
            if(size < sizeRequested) // Warn if the size that was set is less than the requested size.
            {
                logger.warning("TCP send buffer size: requested size of " + sizeRequested + " adjusted to " + size);
            }
        }
    }

    public static void
    setTcpBufSize(java.nio.channels.ServerSocketChannel socket, Ice.Properties properties, Ice.Logger logger)
    {
        //
        // By default, on Windows we use a 128KB buffer size. On Unix
        // platforms, we use the system defaults.
        //
        int dfltBufSize = 0;
        if(System.getProperty("os.name").startsWith("Windows"))
        {
            dfltBufSize = 128 * 1024;
        }

        //
        // Get property for buffer size.
        //
        int sizeRequested = properties.getPropertyAsIntWithDefault("Ice.TCP.RcvSize", dfltBufSize);
        if(sizeRequested > 0)
        {
            //
            // Try to set the buffer size. The kernel will silently adjust
            // the size to an acceptable value. Then read the size back to
            // get the size that was actually set.
            //
            setRecvBufferSize(socket, sizeRequested);
            int size = getRecvBufferSize(socket);
            if(size < sizeRequested) // Warn if the size that was set is less than the requested size.
            {
                logger.warning("TCP receive buffer size: requested size of " + sizeRequested + " adjusted to " + size);
            }
        }
    }

    public static String
    fdToString(java.nio.channels.SelectableChannel fd)
    {
        if(fd == null)
        {
            return "<closed>";
        }

        java.net.InetAddress localAddr = null, remoteAddr = null;
        int localPort = -1, remotePort = -1;

        if(fd instanceof java.nio.channels.SocketChannel)
        {
            java.net.Socket socket = ((java.nio.channels.SocketChannel)fd).socket();
            localAddr = socket.getLocalAddress();
            localPort = socket.getLocalPort();
            remoteAddr = socket.getInetAddress();
            remotePort = socket.getPort();
        }
        else if(fd instanceof java.nio.channels.DatagramChannel)
        {
            java.net.DatagramSocket socket = ((java.nio.channels.DatagramChannel)fd).socket();
            localAddr = socket.getLocalAddress();
            localPort = socket.getLocalPort();
            remoteAddr = socket.getInetAddress();
            remotePort = socket.getPort();
        }
        else
        {
            assert(false);
        }

        return addressesToString(localAddr, localPort, remoteAddr, remotePort);
    }

    public static String
    fdToString(java.net.Socket fd)
    {
        if(fd == null)
        {
            return "<closed>";
        }

        java.net.InetAddress localAddr = fd.getLocalAddress();
        int localPort = fd.getLocalPort();
        java.net.InetAddress remoteAddr = fd.getInetAddress();
        int remotePort = fd.getPort();

        return addressesToString(localAddr, localPort, remoteAddr, remotePort);
    }

    public static String
    addressesToString(java.net.InetAddress localAddr, int localPort, java.net.InetAddress remoteAddr, int remotePort)
    {
        StringBuffer s = new StringBuffer();
        s.append("local address = ");
        s.append(localAddr.getHostAddress());
        s.append(':');
        s.append(localPort);
        if(remoteAddr == null)
        {
            s.append("\nremote address = <not connected>");
        }
        else
        {
            s.append("\nremote address = ");
            s.append(remoteAddr.getHostAddress());
            s.append(':');
            s.append(remotePort);
        }

        return s.toString();
    }

    public static String
    addrToString(java.net.InetSocketAddress addr)
    {
        StringBuffer s = new StringBuffer();
        s.append(addr.getAddress().getHostAddress());
        s.append(':');
        s.append(addr.getPort());
        return s.toString();
    }

    public static boolean
    interrupted(java.io.IOException ex)
    {
        return ex instanceof java.io.InterruptedIOException ||
            ex.getMessage().indexOf("Interrupted system call") >= 0 ||
            ex.getMessage().indexOf("A system call received an interrupt") >= 0; // AIX JDK 1.4.2
    }
}
