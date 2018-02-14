// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class Network
{
    // ProtocolSupport
    public final static int EnableIPv4 = 0;
    public final static int EnableIPv6 = 1;
    public final static int EnableBoth = 2;

    private static java.util.regex.Pattern IPV4_PATTERN = null;
    private static java.util.regex.Pattern IPV6_PATTERN = null;
    private final static String ipv4Pattern =
        "(([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.){3}([01]?\\d\\d?|2[0-4]\\d|25[0-5])";
    private final static String ipv6Pattern =
        "(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-" +
        "fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1" +
        ",4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1," +
        "4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-" +
        "F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])" +
        "\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}" +
        "[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))";

    static
    {
        try
        {
            IPV4_PATTERN =
                java.util.regex.Pattern.compile(ipv4Pattern, java.util.regex.Pattern.CASE_INSENSITIVE);
            IPV6_PATTERN =
                java.util.regex.Pattern.compile(ipv6Pattern, java.util.regex.Pattern.CASE_INSENSITIVE);
        }
        catch (java.util.regex.PatternSyntaxException ex)
        {
            assert(false);
        }
    }

    public static boolean
    isNumericAddress(String ipAddress)
    {
        java.util.regex.Matcher ipv4 = IPV4_PATTERN.matcher(ipAddress);
        if(ipv4.matches())
        {
            return true;
        }
        java.util.regex.Matcher ipv6 = IPV6_PATTERN.matcher(ipAddress);
        return ipv6.matches();
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

        String msg = ex.getMessage();

        if(msg != null)
        {
            msg = msg.toLowerCase();

            final String[] msgs =
            {
                "connection refused", // ECONNREFUSED
                "remote host refused an attempted connect operation" // ECONNREFUSED (AIX JDK 1.4.2)
            };

            for(String m : msgs)
            {
                if(msg.indexOf(m) != -1)
                {
                    return true;
                }
            }
        }

        return false;
    }

    public static boolean
    noMoreFds(java.lang.Throwable ex)
    {
        String msg = ex.getMessage();
        if(msg != null)
        {
            msg = msg.toLowerCase();

            final String[] msgs =
            {
                "too many open files", // EMFILE
                "file table overflow", // ENFILE
                "too many open files in system" // ENFILE
            };

            for(String m : msgs)
            {
                if(msg.indexOf(m) != -1)
                {
                    return true;
                }
            }
        }

        return false;
    }

    public static boolean
    isIPv6Supported()
    {
        try
        {
            java.net.Socket socket = new java.net.Socket();
            socket.bind(new java.net.InetSocketAddress(java.net.InetAddress.getByName("::1"), 0));
            socket.close();
            return true;
        }
        catch(java.io.IOException ex)
        {
            return false;
        }
    }

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
            throw new Ice.SocketException(ex);
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
            throw new Ice.SocketException(ex);
        }
    }

    public static java.nio.channels.DatagramChannel
    createUdpSocket(java.net.InetSocketAddress addr)
    {
        try
        {
            //
            // Use reflection so this code still compiles with older JDK versions.
            // java.net.StandardProtocolFamily is new in JDK 1.7
            //
            Class<?> c = Util.findClass("java.net.StandardProtocolFamily", null);
            if(addr.getAddress().isMulticastAddress() && c != null)
            {
                //
                // For multicast sockets with JDK 7 we must use the open overload that accepts
                // ProtocolFamily and specify the ProtocolFamily that corresponds to the address
                // type of the multicast groups that the channel will join.
                //
                String family = "INET";
                if(addr.getAddress() instanceof java.net.Inet6Address)
                {
                    family = "INET6";
                }
                java.lang.reflect.Method valueOf = c.getDeclaredMethod("valueOf", new Class<?>[]{String.class});

                Object[] args = new Object[]{valueOf.invoke(null, new Object[]{family})};

                java.lang.reflect.Method open = java.nio.channels.DatagramChannel.class.getDeclaredMethod(
                                            "open", new Class<?>[]{Util.findClass("java.net.ProtocolFamily", null)});
                return (java.nio.channels.DatagramChannel)open.invoke(null, args);
            }
            else
            {
                return java.nio.channels.DatagramChannel.open();
            }
        }
        catch(IllegalAccessException ex)
        {
            throw new Ice.SocketException(ex);
        }
        catch(java.lang.reflect.InvocationTargetException ex)
        {
            throw new Ice.SocketException(ex);
        }
        catch(NoSuchMethodException ex)
        {
            throw new Ice.SocketException(ex);
        }
        catch(java.io.IOException ex)
        {
            throw new Ice.SocketException(ex);
        }
    }

    public static void
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
            throw new Ice.SocketException(ex);
        }
    }

    public static java.net.NetworkInterface
    getInterface(String intf)
    {
        java.net.NetworkInterface iface;
        try
        {
            iface = java.net.NetworkInterface.getByName(intf);
            if(iface != null)
            {
                return iface;
            }
        }
        catch(Exception ex)
        {
        }
        try
        {
            iface = java.net.NetworkInterface.getByInetAddress(java.net.InetAddress.getByName(intf));
            if(iface != null)
            {
                return iface;
            }
        }
        catch(Exception ex)
        {
        }
        throw new IllegalArgumentException("couldn't find interface `" + intf + "'");
    }

    public static void
    setMcastInterface(java.nio.channels.DatagramChannel fd, String intf)
    {
        try
        {
            fd.setOption(java.net.StandardSocketOptions.IP_MULTICAST_IF, getInterface(intf));
        }
        catch(Exception ex)
        {
            throw new Ice.SocketException(ex);
        }
    }

    public static void
    setMcastGroup(java.net.MulticastSocket fd, java.net.InetSocketAddress group, String intf)
    {
        try
        {
            java.util.Set<java.net.NetworkInterface> interfaces = new java.util.HashSet<>();
            for(String address : getInterfacesForMulticast(intf, getProtocolSupport(group)))
            {
                java.net.NetworkInterface intf2 = getInterface(address);
                if(!interfaces.contains(intf2))
                {
                    interfaces.add(intf2);
                    fd.joinGroup(group, intf2);
                }
            }
        }
        catch(Exception ex)
        {
            throw new Ice.SocketException(ex);
        }
    }

    public static void
    setMcastGroup(java.nio.channels.DatagramChannel fd, java.net.InetSocketAddress group, String intf)
    {
        try
        {
            java.util.Set<java.net.NetworkInterface> interfaces = new java.util.HashSet<>();
            for(String address : getInterfacesForMulticast(intf, getProtocolSupport(group)))
            {
                java.net.NetworkInterface intf2 = getInterface(address);
                if(!interfaces.contains(intf2))
                {
                    interfaces.add(intf2);
                    fd.join(group.getAddress(), intf2);
                }
            }
        }
        catch(Exception ex)
        {
            throw new Ice.SocketException(ex);
        }
    }

    public static void
    setMcastTtl(java.nio.channels.DatagramChannel fd, int ttl)
    {
        try
        {
            fd.setOption(java.net.StandardSocketOptions.IP_MULTICAST_TTL, ttl);
        }
        catch(Exception ex)
        {
            throw new Ice.SocketException(ex);
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
            throw new Ice.SocketException(ex);
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
            throw new Ice.SocketException(ex);
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
            throw new Ice.SocketException(ex);
        }
    }

    public static java.net.InetSocketAddress
    doBind(java.nio.channels.ServerSocketChannel fd, java.net.InetSocketAddress addr, int backlog)
    {
        try
        {
            java.net.ServerSocket sock = fd.socket();
            sock.bind(addr, backlog);
            return (java.net.InetSocketAddress)sock.getLocalSocketAddress();
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            throw new Ice.SocketException(ex);
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
            throw new Ice.SocketException(ex);
        }
    }

    public static java.nio.channels.SocketChannel
    doAccept(java.nio.channels.ServerSocketChannel afd)
    {
        java.nio.channels.SocketChannel fd = null;
        while(true)
        {
            try
            {
                fd = afd.accept();
                break;
            }
            catch(java.io.IOException ex)
            {
                if(interrupted(ex))
                {
                    continue;
                }

                throw new Ice.SocketException(ex);
            }
        }

        try
        {
            java.net.Socket socket = fd.socket();
            socket.setTcpNoDelay(true);
            socket.setKeepAlive(true);
        }
        catch(java.io.IOException ex)
        {
            throw new Ice.SocketException(ex);
        }

        return fd;
    }

    public static boolean
    doConnect(java.nio.channels.SocketChannel fd, java.net.InetSocketAddress addr,
              java.net.InetSocketAddress sourceAddr)
    {
        if(sourceAddr != null)
        {
            try
            {
                fd.bind(sourceAddr);
            }
            catch(java.io.IOException ex)
            {
                closeSocketNoThrow(fd);
                throw new Ice.SocketException(ex);
            }
        }

        try
        {
            if(!fd.connect(addr))
            {
                return false;
            }
        }
        catch(java.net.ConnectException ex)
        {
            closeSocketNoThrow(fd);

            if(connectionRefused(ex))
            {
                throw new Ice.ConnectionRefusedException(ex);
            }
            else
            {
                throw new Ice.ConnectFailedException(ex);
            }
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            throw new Ice.SocketException(ex);
        }
        catch(java.lang.SecurityException ex)
        {
            closeSocketNoThrow(fd);
            throw new Ice.SocketException(ex);
        }

        if(System.getProperty("os.name").equals("Linux"))
        {
            //
            // Prevent self connect (self connect happens on Linux when a client tries to connect to
            // a server which was just deactivated if the client socket re-uses the same ephemeral
            // port as the server).
            //
            if(addr.equals(fd.socket().getLocalSocketAddress()))
            {
                closeSocketNoThrow(fd);
                throw new Ice.ConnectionRefusedException();
            }
        }
        return true;
    }

    public static void
    doFinishConnect(java.nio.channels.SocketChannel fd)
    {
        //
        // Note: we don't close the socket if there's an exception. It's the responsibility
        // of the caller to do so.
        //

        try
        {
            if(!fd.finishConnect())
            {
                throw new Ice.ConnectFailedException();
            }

            if(System.getProperty("os.name").equals("Linux"))
            {
                //
                // Prevent self connect (self connect happens on Linux when a client tries to connect to
                // a server which was just deactivated if the client socket re-uses the same ephemeral
                // port as the server).
                //
                java.net.SocketAddress addr = fd.socket().getRemoteSocketAddress();
                if(addr != null && addr.equals(fd.socket().getLocalSocketAddress()))
                {
                    throw new Ice.ConnectionRefusedException();
                }
            }
        }
        catch(java.net.ConnectException ex)
        {
            if(connectionRefused(ex))
            {
                throw new Ice.ConnectionRefusedException(ex);
            }
            else
            {
                throw new Ice.ConnectFailedException(ex);
            }
        }
        catch(java.io.IOException ex)
        {
            throw new Ice.SocketException(ex);
        }
    }

    public static void
    doConnect(java.nio.channels.DatagramChannel fd, java.net.InetSocketAddress addr,
              java.net.InetSocketAddress sourceAddr)
    {
        if(sourceAddr != null)
        {
            doBind(fd, sourceAddr);
        }

        try
        {
            fd.connect(addr);
        }
        catch(java.net.ConnectException ex)
        {
            closeSocketNoThrow(fd);

            if(connectionRefused(ex))
            {
                throw new Ice.ConnectionRefusedException(ex);
            }
            else
            {
                throw new Ice.ConnectFailedException(ex);
            }
        }
        catch(java.io.IOException ex)
        {
            closeSocketNoThrow(fd);
            throw new Ice.SocketException(ex);
        }
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
            throw new Ice.SocketException(ex);
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
            throw new Ice.SocketException(ex);
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
            throw new Ice.SocketException(ex);
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
            throw new Ice.SocketException(ex);
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
            throw new Ice.SocketException(ex);
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
            throw new Ice.SocketException(ex);
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
            throw new Ice.SocketException(ex);
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
            throw new Ice.SocketException(ex);
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
            throw new Ice.SocketException(ex);
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
            throw new Ice.SocketException(ex);
        }
        return size;
    }

    public static int
    getProtocolSupport(java.net.InetSocketAddress addr)
    {
        return addr.getAddress().getAddress().length == 4 ? Network.EnableIPv4 : Network.EnableIPv6;
    }

    public static java.net.InetSocketAddress
    getAddressForServer(String host, int port, int protocol, boolean preferIPv6)
    {
        if(host == null || host.length() == 0)
        {
            try
            {
                if(protocol != EnableIPv4)
                {
                    return new java.net.InetSocketAddress(java.net.InetAddress.getByName("::0"), port);
                }
                else
                {
                    return new java.net.InetSocketAddress(java.net.InetAddress.getByName("0.0.0.0"), port);
                }
            }
            catch(java.net.UnknownHostException ex)
            {
                assert(false);
                return null;
            }
            catch(java.lang.SecurityException ex)
            {
                throw new Ice.SocketException(ex);
            }
        }
        return getAddresses(host, port, protocol, Ice.EndpointSelectionType.Ordered, preferIPv6, true).get(0);
    }

    public static int
    compareAddress(java.net.InetSocketAddress addr1, java.net.InetSocketAddress addr2)
    {
        if(addr1 == null)
        {
            if(addr2 == null)
            {
                return 0;
            }
            else
            {
                return -1;
            }
        }
        else if(addr2 == null)
        {
            return 1;
        }

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
        if(larr.length < rarr.length)
        {
            return -1;
        }
        else if(rarr.length < larr.length)
        {
            return 1;
        }
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
    getLocalAddress(int protocol)
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

        if(addr == null || !isValidAddr(addr, protocol))
        {
            //
            // Iterate over the network interfaces and pick an IP
            // address (preferably not the loopback address).
            //
            java.util.ArrayList<java.net.InetAddress> addrs = getLocalAddresses(protocol, false);
            java.util.Iterator<java.net.InetAddress> iter = addrs.iterator();
            while(addr == null && iter.hasNext())
            {
                java.net.InetAddress a = iter.next();
                if(protocol == EnableBoth || isValidAddr(a, protocol))
                {
                    addr = a;
                }
            }

            if(addr == null)
            {
                addr = getLoopbackAddresses(protocol)[0]; // Use the loopback address as the last resort.
            }
        }

        assert(addr != null);
        return addr;
    }

    public static java.util.List<java.net.InetSocketAddress>
    getAddresses(String host, int port, int protocol, Ice.EndpointSelectionType selType, boolean preferIPv6,
                 boolean blocking)
    {
        if(!blocking)
        {
            if(!isNumericAddress(host))
            {
                return null; // Can't get the address without blocking.
            }

            java.util.List<java.net.InetSocketAddress> addrs = new java.util.ArrayList<java.net.InetSocketAddress>();
            try
            {
                addrs.add(new java.net.InetSocketAddress(java.net.InetAddress.getByName(host), port));
            }
            catch(java.net.UnknownHostException ex)
            {
                assert(false);
            }
            return addrs;
        }

        java.util.List<java.net.InetSocketAddress> addresses = new java.util.ArrayList<java.net.InetSocketAddress>();
        try
        {
            java.net.InetAddress[] addrs;
            if(host == null || host.length() == 0)
            {
                addrs = getLoopbackAddresses(protocol);
            }
            else
            {
                addrs = java.net.InetAddress.getAllByName(host);
            }

            for(java.net.InetAddress addr : addrs)
            {
                if(protocol == EnableBoth || isValidAddr(addr, protocol))
                {
                    addresses.add(new java.net.InetSocketAddress(addr, port));
                }
            }

            if(selType == Ice.EndpointSelectionType.Random)
            {
                java.util.Collections.shuffle(addresses);
            }

            if(protocol == EnableBoth)
            {
                if(preferIPv6)
                {
                    java.util.Collections.sort(addresses, _preferIPv6Comparator);
                }
                else
                {
                    java.util.Collections.sort(addresses, _preferIPv4Comparator);
                }
            }
        }
        catch(java.net.UnknownHostException ex)
        {
            throw new Ice.DNSException(0, host, ex);
        }
        catch(java.lang.SecurityException ex)
        {
            throw new Ice.SocketException(ex);
        }

        //
        // No Inet4Address/Inet6Address available.
        //
        if(addresses.isEmpty())
        {
            throw new Ice.DNSException(0, host);
        }

        return addresses;
    }

    public static java.util.ArrayList<java.net.InetAddress>
    getLocalAddresses(int protocol, boolean includeLoopback)
    {
        java.util.ArrayList<java.net.InetAddress> result = new java.util.ArrayList<java.net.InetAddress>();
        try
        {
            java.util.Enumeration<java.net.NetworkInterface> ifaces = java.net.NetworkInterface.getNetworkInterfaces();
            while(ifaces.hasMoreElements())
            {
                java.net.NetworkInterface iface = ifaces.nextElement();
                java.util.Enumeration<java.net.InetAddress> addrs = iface.getInetAddresses();
                while(addrs.hasMoreElements())
                {
                    java.net.InetAddress addr = addrs.nextElement();
                    if(includeLoopback || !addr.isLoopbackAddress())
                    {
                        if(protocol == EnableBoth || isValidAddr(addr, protocol))
                        {
                            result.add(addr);
                        }
                    }
                }
            }
        }
        catch(java.net.SocketException ex)
        {
            throw new Ice.SocketException(ex);
        }
        catch(java.lang.SecurityException ex)
        {
            throw new Ice.SocketException(ex);
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
        try
        {
            java.nio.channels.Pipe pipe = java.nio.channels.Pipe.open();
            fds.sink = pipe.sink();
            fds.source = pipe.source();
        }
        catch(java.io.IOException ex)
        {
            throw new Ice.SocketException(ex);
        }
        return fds;
    }

    public static java.util.ArrayList<String>
    getHostsForEndpointExpand(String host, int protocolSupport, boolean includeLoopback)
    {
        java.util.ArrayList<String> hosts = new java.util.ArrayList<String>();
        if(isWildcard(host))
        {
            for(java.net.InetAddress addr : getLocalAddresses(protocolSupport, includeLoopback))
            {
                //
                // NOTE: We don't publish link-local IPv6 addresses as these addresses can only
                // be accessed in general with a scope-id.
                //
                if(!addr.isLinkLocalAddress())
                {
                    hosts.add(addr.getHostAddress());
                }
            }
            if(hosts.isEmpty())
            {
                // Return loopback if no other local addresses are available.
                for(java.net.InetAddress addr : getLoopbackAddresses(protocolSupport))
                {
                    hosts.add(addr.getHostAddress());
                }
            }
        }
        return hosts;
    }

    public static java.util.List<String>
    getInterfacesForMulticast(String intf, int protocolSupport)
    {
        java.util.ArrayList<String> interfaces = new java.util.ArrayList<>();
        if(isWildcard(intf))
        {
            for(java.net.InetAddress addr : getLocalAddresses(protocolSupport, true))
            {
                interfaces.add(addr.getHostAddress());
            }
        }
        if(interfaces.isEmpty())
        {
            interfaces.add(intf);
        }
        return interfaces;
    }

    public static void
    setTcpBufSize(java.nio.channels.SocketChannel socket, ProtocolInstance instance)
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

        int rcvSize = instance.properties().getPropertyAsIntWithDefault("Ice.TCP.RcvSize", dfltBufSize);
        int sndSize = instance.properties().getPropertyAsIntWithDefault("Ice.TCP.SndSize", dfltBufSize);

        setTcpBufSize(socket, rcvSize, sndSize, instance);
    }

    public static void
    setTcpBufSize(java.nio.channels.SocketChannel socket, int rcvSize, int sndSize, ProtocolInstance instance)
    {
        if(rcvSize > 0)
        {
            //
            // Try to set the buffer size. The kernel will silently adjust
            // the size to an acceptable value. Then read the size back to
            // get the size that was actually set.
            //
            setRecvBufferSize(socket, rcvSize);
            int size = getRecvBufferSize(socket);
            if(size < rcvSize)
            {
                // Warn if the size that was set is less than the requested size and
                // we have not already warned.
                BufSizeWarnInfo winfo = instance.getBufSizeWarn(Ice.TCPEndpointType.value);
                if(!winfo.rcvWarn || rcvSize != winfo.rcvSize)
                {
                    instance.logger().warning("TCP receive buffer size: requested size of " + rcvSize +
                                              " adjusted to " + size);
                    instance.setRcvBufSizeWarn(Ice.TCPEndpointType.value, rcvSize);
                }
            }
        }

        if(sndSize > 0)
        {
            //
            // Try to set the buffer size. The kernel will silently adjust
            // the size to an acceptable value. Then read the size back to
            // get the size that was actually set.
            //
            setSendBufferSize(socket, sndSize);
            int size = getSendBufferSize(socket);
            if(size < sndSize)
            {
                // Warn if the size that was set is less than the requested size and
                // we have not already warned.
                BufSizeWarnInfo winfo = instance.getBufSizeWarn(Ice.TCPEndpointType.value);
                if(!winfo.sndWarn || sndSize != winfo.sndSize)
                {
                    instance.logger().warning("TCP send buffer size: requested size of " + sndSize +
                                              " adjusted to " + size);
                    instance.setSndBufSizeWarn(Ice.TCPEndpointType.value, sndSize);
                }
            }
        }
    }

    public static void
    setTcpBufSize(java.nio.channels.ServerSocketChannel socket, ProtocolInstance instance)
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
        int sizeRequested = instance.properties().getPropertyAsIntWithDefault("Ice.TCP.RcvSize", dfltBufSize);
        if(sizeRequested > 0)
        {
            //
            // Try to set the buffer size. The kernel will silently adjust
            // the size to an acceptable value. Then read the size back to
            // get the size that was actually set.
            //
            setRecvBufferSize(socket, sizeRequested);
            int size = getRecvBufferSize(socket);
            if(size < sizeRequested)
            {
                // Warn if the size that was set is less than the requested size and
                // we have not already warned.
                BufSizeWarnInfo winfo = instance.getBufSizeWarn(Ice.TCPEndpointType.value);
                if(!winfo.rcvWarn || sizeRequested != winfo.rcvSize)
                {
                    instance.logger().warning("TCP receive buffer size: requested size of " + sizeRequested +
                                              " adjusted to " + size);
                    instance.setRcvBufSizeWarn(Ice.TCPEndpointType.value, sizeRequested);
                }
            }
        }
    }

    public static String
    fdToString(java.nio.channels.SelectableChannel fd, NetworkProxy proxy, java.net.InetSocketAddress target)
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

        return addressesToString(localAddr, localPort, remoteAddr, remotePort, proxy, target);
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
    addressesToString(java.net.InetAddress localAddr, int localPort, java.net.InetAddress remoteAddr, int remotePort,
                      NetworkProxy proxy, java.net.InetSocketAddress target)
    {
        StringBuilder s = new StringBuilder(128);
        s.append("local address = ");
        s.append(addrToString(localAddr, localPort));

        if(proxy != null)
        {
            if(remoteAddr == null)
            {
                java.net.InetSocketAddress addr = proxy.getAddress();
                remoteAddr = addr.getAddress();
                remotePort = addr.getPort();
            }
            s.append("\n");
            s.append(proxy.getName());
            s.append(" proxy address = ");
            s.append(addrToString(remoteAddr, remotePort));
            s.append("\nremote address = ");
            s.append(addrToString(target.getAddress(), target.getPort()));
        }
        else
        {
            if(remoteAddr == null && target != null)
            {
                remoteAddr = target.getAddress();
                remotePort = target.getPort();
            }

            if(remoteAddr == null)
            {
                s.append("\nremote address = <not connected>");
            }
            else
            {
                s.append("\nremote address = ");
                s.append(addrToString(remoteAddr, remotePort));
            }
        }

        return s.toString();
    }

    public static String
    addressesToString(java.net.InetAddress localAddr, int localPort, java.net.InetAddress remoteAddr, int remotePort)
    {
        return addressesToString(localAddr, localPort, remoteAddr, remotePort, null, null);
    }

    public static String
    addrToString(java.net.InetSocketAddress addr)
    {
        StringBuilder s = new StringBuilder(128);
        s.append(addr.getAddress().getHostAddress());
        s.append(':');
        s.append(addr.getPort());
        return s.toString();
    }

    public static boolean
    interrupted(java.io.IOException ex)
    {
        return ex instanceof java.io.InterruptedIOException;
    }

    private static boolean
    isValidAddr(java.net.InetAddress addr, int protocol)
    {
         byte[] bytes = null;
         if(addr != null)
         {
             bytes = addr.getAddress();
         }
         return bytes != null &&
               ((bytes.length == 16 && protocol == EnableIPv6) ||
                (bytes.length == 4 && protocol == EnableIPv4));
    }

    public static String
    addrToString(java.net.InetAddress addr, int port)
    {
        StringBuffer s = new StringBuffer();

        //
        // In early Android releases, sockets don't correctly report their address and
        // port information.
        //

        if(addr == null || addr.isAnyLocalAddress())
        {
            s.append("<not available>");
        }
        else
        {
            s.append(addr.getHostAddress());
        }

        if(port > 0)
        {
            s.append(':');
            s.append(port);
        }

        return s.toString();
    }

    private static java.net.InetAddress[]
    getLoopbackAddresses(int protocol)
    {
        try
        {
            java.net.InetAddress[] addrs = new java.net.InetAddress[protocol == EnableBoth ? 2 : 1];
            int i = 0;
            if(protocol != EnableIPv6)
            {
                addrs[i++] = java.net.InetAddress.getByName("127.0.0.1");
            }
            if(protocol != EnableIPv4)
            {
                addrs[i++] = java.net.InetAddress.getByName("::1");
            }
            return addrs;
        }
        catch(java.net.UnknownHostException ex)
        {
            assert(false);
            return null;
        }
        catch(java.lang.SecurityException ex)
        {
            throw new Ice.SocketException(ex);
        }
    }

    public static java.net.InetSocketAddress
    getNumericAddress(String address)
    {
        java.net.InetSocketAddress addr = null;
        if(!address.isEmpty() && isNumericAddress(address))
        {
            try
            {
                addr = new java.net.InetSocketAddress(java.net.InetAddress.getByName(address), 0);
            }
            catch(java.net.UnknownHostException ex)
            {
            }
        }
        return addr;
    }

    private static boolean
    isWildcard(String host)
    {
        if(host == null || host.length() == 0)
        {
            return true;
        }
        try
        {
            return java.net.InetAddress.getByName(host).isAnyLocalAddress();
        }
        catch(java.net.UnknownHostException ex)
        {
        }
        catch(java.lang.SecurityException ex)
        {
            throw new Ice.SocketException(ex);
        }
        return false;
    }

    static class IPAddressComparator implements java.util.Comparator<java.net.InetSocketAddress>
    {
        IPAddressComparator(boolean ipv6)
        {
            _ipv6 = ipv6;
        }

        @Override
        public int
        compare(java.net.InetSocketAddress lhs, java.net.InetSocketAddress rhs)
        {
            if(lhs.getAddress().getAddress().length < rhs.getAddress().getAddress().length)
            {
                return _ipv6 ? 1 : -1;
            }
            else if(lhs.getAddress().getAddress().length > rhs.getAddress().getAddress().length)
            {
                return _ipv6 ? -1 : 1;
            }
            else
            {
                return 0;
            }
        }

        final private boolean _ipv6;
    }

    private static IPAddressComparator _preferIPv4Comparator = new IPAddressComparator(false);
    private static IPAddressComparator _preferIPv6Comparator = new IPAddressComparator(true);
}
