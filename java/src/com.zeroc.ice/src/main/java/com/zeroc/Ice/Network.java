// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.IOException;
import java.net.ConnectException;
import java.net.DatagramSocket;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.MulticastSocket;
import java.net.NetworkInterface;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.StandardProtocolFamily;
import java.net.StandardSocketOptions;
import java.net.UnknownHostException;
import java.nio.channels.DatagramChannel;
import java.nio.channels.SelectableChannel;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;

/**
 * @hidden Public because it's used by IceDiscovery and IceLocatorDiscovery.
 */
public final class Network {
    // ProtocolSupport
    public static final int EnableIPv4 = 0;
    public static final int EnableIPv6 = 1;
    public static final int EnableBoth = 2;

    private static Pattern IPV4_PATTERN;
    private static Pattern IPV6_PATTERN;
    private static final String ipv4Pattern =
        "(([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.){3}([01]?\\d\\d?|2[0-4]\\d|25[0-5])";
    private static final String ipv6Pattern =
        "(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-"
            + "fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1"
            + ",4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,"
            + "4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-"
            + "F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])"
            + "\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}"
            + "[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))";

    static {
        try {
            IPV4_PATTERN =
                Pattern.compile(
                    ipv4Pattern, Pattern.CASE_INSENSITIVE);
            IPV6_PATTERN =
                Pattern.compile(
                    ipv6Pattern, Pattern.CASE_INSENSITIVE);
        } catch (PatternSyntaxException ex) {
            assert false;
        }
    }

    public static boolean isNumericAddress(String ipAddress) {
        Matcher ipv4 = IPV4_PATTERN.matcher(ipAddress);
        if (ipv4.matches()) {
            return true;
        }
        Matcher ipv6 = IPV6_PATTERN.matcher(ipAddress);
        return ipv6.matches();
    }

    public static boolean connectionRefused(ConnectException ex) {
        //
        // The JDK raises a generic ConnectException when the server
        // actively refuses a connection. Unfortunately, our only choice is to search the exception
        // message for distinguishing phrases.
        //

        String msg = ex.getMessage();

        if (msg != null) {
            msg = msg.toLowerCase();

            return msg.indexOf("connection refused") != -1;
        }

        return false;
    }

    public static boolean noMoreFds(Throwable ex) {
        String msg = ex.getMessage();
        if (msg != null) {
            msg = msg.toLowerCase();

            final String[] msgs = {
                "too many open files", // EMFILE
                "file table overflow", // ENFILE
                "too many open files in system" // ENFILE
            };

            for (String m : msgs) {
                if (msg.indexOf(m) != -1) {
                    return true;
                }
            }
        }

        return false;
    }

    public static boolean isIPv6Supported() {
        try {
            Socket socket = new Socket();
            socket.bind(new InetSocketAddress(InetAddress.getByName("::1"), 0));
            socket.close();
            return true;
        } catch (IOException ex) {
            return false;
        }
    }

    public static SocketChannel createTcpSocket() {
        try {
            SocketChannel fd = SocketChannel.open();
            Socket socket = fd.socket();
            socket.setTcpNoDelay(true);
            socket.setKeepAlive(true);
            return fd;
        } catch (IOException ex) {
            throw new SocketException(ex);
        }
    }

    public static ServerSocketChannel createTcpServerSocket() {
        try {
            ServerSocketChannel fd = ServerSocketChannel.open();
            // It's not possible to set TCP_NODELAY or KEEP_ALIVE on a server socket in Java
            //
            // java.net.Socket socket = fd.socket();
            // socket.setTcpNoDelay(true); socket.setKeepAlive(true);
            return fd;
        } catch (IOException ex) {
            throw new SocketException(ex);
        }
    }

    public static DatagramChannel createUdpSocket(
            InetSocketAddress addr) {
        try {
            if (addr.getAddress().isMulticastAddress()) {
                var familyStr =
                    addr.getAddress() instanceof Inet6Address ? "INET6" : "INET";
                var family = StandardProtocolFamily.valueOf(familyStr);
                return DatagramChannel.open(family);
            } else {
                return DatagramChannel.open();
            }
        } catch (IOException ex) {
            throw new SocketException(ex);
        }
    }

    public static void closeSocketNoThrow(SelectableChannel fd) {
        try {
            fd.close();
        } catch (IOException ex) {
            // Ignore
        }
    }

    public static NetworkInterface getInterface(String intf) {
        NetworkInterface iface;
        try {
            iface = NetworkInterface.getByName(intf);
            if (iface != null) {
                return iface;
            }
        } catch (Exception ex) {}
        try {
            iface =
                NetworkInterface.getByInetAddress(
                    InetAddress.getByName(intf));
            if (iface != null) {
                return iface;
            }
        } catch (Exception ex) {}
        throw new IllegalArgumentException("couldn't find interface `" + intf + "'");
    }

    public static void setMcastInterface(DatagramChannel fd, String intf) {
        try {
            fd.setOption(StandardSocketOptions.IP_MULTICAST_IF, getInterface(intf));
        } catch (Exception ex) {
            throw new SocketException(ex);
        }
    }

    public static void setMcastGroup(
            MulticastSocket fd, InetSocketAddress group, String intf) {
        try {
            Set<NetworkInterface> interfaces = new HashSet<>();
            for (String address : getInterfacesForMulticast(intf, getProtocolSupport(group))) {
                NetworkInterface intf2 = getInterface(address);
                if (!interfaces.contains(intf2)) {
                    interfaces.add(intf2);
                    fd.joinGroup(group, intf2);
                }
            }
        } catch (Exception ex) {
            throw new SocketException(ex);
        }
    }

    public static void setMcastGroup(
            DatagramChannel fd, InetSocketAddress group, String intf) {
        try {
            Set<NetworkInterface> interfaces = new HashSet<>();
            for (String address : getInterfacesForMulticast(intf, getProtocolSupport(group))) {
                NetworkInterface intf2 = getInterface(address);
                if (!interfaces.contains(intf2)) {
                    interfaces.add(intf2);
                    fd.join(group.getAddress(), intf2);
                }
            }
        } catch (Exception ex) {
            throw new SocketException(ex);
        }
    }

    public static void setMcastTtl(DatagramChannel fd, int ttl) {
        try {
            fd.setOption(StandardSocketOptions.IP_MULTICAST_TTL, ttl);
        } catch (Exception ex) {
            throw new SocketException(ex);
        }
    }

    public static void setBlock(SelectableChannel fd, boolean block) {
        try {
            fd.configureBlocking(block);
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
    }

    public static void setReuseAddress(DatagramChannel fd, boolean reuse) {
        try {
            fd.socket().setReuseAddress(reuse);
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
    }

    public static void setReuseAddress(ServerSocketChannel fd, boolean reuse) {
        try {
            fd.socket().setReuseAddress(reuse);
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
    }

    public static InetSocketAddress doBind(
            ServerSocketChannel fd,
            InetSocketAddress addr,
            int backlog) {
        try {
            ServerSocket sock = fd.socket();
            sock.bind(addr, backlog);
            return (InetSocketAddress) sock.getLocalSocketAddress();
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
    }

    public static InetSocketAddress doBind(
            DatagramChannel fd, InetSocketAddress addr) {
        try {
            DatagramSocket sock = fd.socket();
            sock.bind(addr);
            return (InetSocketAddress) sock.getLocalSocketAddress();
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
    }

    public static SocketChannel doAccept(
            ServerSocketChannel socketChannel) {
        SocketChannel fd = null;
        while (true) {
            try {
                fd = socketChannel.accept();
                break;
            } catch (IOException ex) {
                throw new SocketException(ex);
            }
        }

        try {
            Socket socket = fd.socket();
            socket.setTcpNoDelay(true);
            socket.setKeepAlive(true);
        } catch (IOException ex) {
            throw new SocketException(ex);
        }

        return fd;
    }

    public static boolean doConnect(
            SocketChannel fd,
            InetSocketAddress addr,
            InetSocketAddress sourceAddr) {
        if (sourceAddr != null) {
            try {
                fd.bind(sourceAddr);
            } catch (IOException ex) {
                closeSocketNoThrow(fd);
                throw new SocketException(ex);
            }
        }

        try {
            if (!fd.connect(addr)) {
                return false;
            }
        } catch (ConnectException ex) {
            closeSocketNoThrow(fd);

            if (connectionRefused(ex)) {
                throw new ConnectionRefusedException(ex);
            } else {
                throw new ConnectFailedException(ex);
            }
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        } catch (java.lang.SecurityException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }

        if ("Linux".equals(System.getProperty("os.name"))) {
            //
            // Prevent self connect (self connect happens on Linux when a client tries to connect to
            // a server which was just deactivated if the client socket re-uses the same ephemeral
            // port as the server).
            //
            if (addr.equals(fd.socket().getLocalSocketAddress())) {
                closeSocketNoThrow(fd);
                throw new ConnectionRefusedException();
            }
        }
        return true;
    }

    public static void doFinishConnect(SocketChannel fd) {
        //
        // Note: we don't close the socket if there's an exception. It's the responsibility of the
        // caller to do so.
        //

        try {
            if (!fd.finishConnect()) {
                throw new ConnectFailedException();
            }

            if ("Linux".equals(System.getProperty("os.name"))) {
                //
                // Prevent self connect (self connect happens on Linux when a client tries to
                // connect to a server which was just deactivated if the client socket re-uses the
                // same ephemeral port as the server).
                //
                SocketAddress addr = fd.socket().getRemoteSocketAddress();
                if (addr != null && addr.equals(fd.socket().getLocalSocketAddress())) {
                    throw new ConnectionRefusedException();
                }
            }
        } catch (ConnectException ex) {
            if (connectionRefused(ex)) {
                throw new ConnectionRefusedException(ex);
            } else {
                throw new ConnectFailedException(ex);
            }
        } catch (IOException ex) {
            throw new SocketException(ex);
        }
    }

    public static void doConnect(
            DatagramChannel fd,
            InetSocketAddress addr,
            InetSocketAddress sourceAddr) {
        if (sourceAddr != null) {
            doBind(fd, sourceAddr);
        }

        try {
            fd.connect(addr);
        } catch (ConnectException ex) {
            closeSocketNoThrow(fd);

            if (connectionRefused(ex)) {
                throw new ConnectionRefusedException(ex);
            } else {
                throw new ConnectFailedException(ex);
            }
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
    }

    public static void setSendBufferSize(SocketChannel fd, int size) {
        try {
            Socket socket = fd.socket();
            socket.setSendBufferSize(size);
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
    }

    public static int getSendBufferSize(SocketChannel fd) {
        int size;
        try {
            Socket socket = fd.socket();
            size = socket.getSendBufferSize();
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
        return size;
    }

    public static void setRecvBufferSize(SocketChannel fd, int size) {
        try {
            Socket socket = fd.socket();
            socket.setReceiveBufferSize(size);
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
    }

    public static int getRecvBufferSize(SocketChannel fd) {
        int size;
        try {
            Socket socket = fd.socket();
            size = socket.getReceiveBufferSize();
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
        return size;
    }

    public static void setRecvBufferSize(ServerSocketChannel fd, int size) {
        try {
            ServerSocket socket = fd.socket();
            socket.setReceiveBufferSize(size);
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
    }

    public static int getRecvBufferSize(ServerSocketChannel fd) {
        int size;
        try {
            ServerSocket socket = fd.socket();
            size = socket.getReceiveBufferSize();
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
        return size;
    }

    public static void setSendBufferSize(DatagramChannel fd, int size) {
        try {
            DatagramSocket socket = fd.socket();
            socket.setSendBufferSize(size);
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
    }

    public static int getSendBufferSize(DatagramChannel fd) {
        int size;
        try {
            DatagramSocket socket = fd.socket();
            size = socket.getSendBufferSize();
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
        return size;
    }

    public static void setRecvBufferSize(DatagramChannel fd, int size) {
        try {
            DatagramSocket socket = fd.socket();
            socket.setReceiveBufferSize(size);
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
    }

    public static int getRecvBufferSize(DatagramChannel fd) {
        int size;
        try {
            DatagramSocket socket = fd.socket();
            size = socket.getReceiveBufferSize();
        } catch (IOException ex) {
            closeSocketNoThrow(fd);
            throw new SocketException(ex);
        }
        return size;
    }

    public static int getProtocolSupport(InetSocketAddress addr) {
        return addr.getAddress().getAddress().length == 4 ? Network.EnableIPv4 : Network.EnableIPv6;
    }

    public static InetSocketAddress getAddressForServer(
            String host, int port, int protocol, boolean preferIPv6) {
        if (host == null || host.isEmpty()) {
            try {
                if (protocol != EnableIPv4) {
                    return new InetSocketAddress(
                        InetAddress.getByName("::0"), port);
                } else {
                    return new InetSocketAddress(
                        InetAddress.getByName("0.0.0.0"), port);
                }
            } catch (UnknownHostException ex) {
                assert false;
                return null;
            } catch (java.lang.SecurityException ex) {
                throw new SocketException(ex);
            }
        }
        return getAddresses(host, port, protocol, preferIPv6, true).get(0);
    }

    public static int compareAddress(
            InetSocketAddress addr1, InetSocketAddress addr2) {
        if (addr1 == null) {
            return addr2 == null ? 0 : -1;
        } else if (addr2 == null) {
            return 1;
        }

        int v = Integer.compare(addr1.getPort(), addr2.getPort());
        if (v != 0) {
            return v;
        }

        byte[] larr = addr1.getAddress().getAddress();
        byte[] rarr = addr2.getAddress().getAddress();
        v = larr.length - rarr.length;
        if (v != 0) {
            return v;
        }
        return Arrays.compare(larr, rarr);
    }

    public static List<InetSocketAddress> getAddresses(
            String host, int port, int protocol, boolean preferIPv6, boolean blocking) {
        if (!blocking) {
            if (!isNumericAddress(host)) {
                return null; // Can't get the address without blocking.
            }

            List<InetSocketAddress> addrs = new ArrayList<>();
            try {
                addrs.add(
                    new InetSocketAddress(InetAddress.getByName(host), port));
            } catch (UnknownHostException ex) {
                assert false;
            }
            return addrs;
        }

        List<InetSocketAddress> addresses = new ArrayList<>();
        try {
            InetAddress[] addrs;
            if (host == null || host.isEmpty()) {
                addrs = getLoopbackAddresses(protocol);
            } else {
                addrs = InetAddress.getAllByName(host);
            }

            for (InetAddress addr : addrs) {
                if (protocol == EnableBoth || isValidAddr(addr, protocol)) {
                    addresses.add(new InetSocketAddress(addr, port));
                }
            }

            if (protocol == EnableBoth) {
                if (preferIPv6) {
                    Collections.sort(addresses, _preferIPv6Comparator);
                } else {
                    Collections.sort(addresses, _preferIPv4Comparator);
                }
            }
        } catch (UnknownHostException ex) {
            throw new DNSException(host, ex);
        } catch (java.lang.SecurityException ex) {
            throw new SocketException(ex);
        }

        //
        // No Inet4Address/Inet6Address available.
        //
        if (addresses.isEmpty()) {
            throw new DNSException(host);
        }

        return addresses;
    }

    public static ArrayList<InetAddress> getLocalAddresses(int protocol) {
        ArrayList<InetAddress> result = new ArrayList<>();
        try {
            Enumeration<NetworkInterface> ifaces =
                NetworkInterface.getNetworkInterfaces();
            while (ifaces.hasMoreElements()) {
                NetworkInterface iface = ifaces.nextElement();
                Enumeration<InetAddress> addrs = iface.getInetAddresses();
                while (addrs.hasMoreElements()) {
                    InetAddress addr = addrs.nextElement();
                    if (!result.contains(addr)
                        && (protocol == EnableBoth || isValidAddr(addr, protocol))) {
                        result.add(addr);
                        break;
                    }
                }
            }
        } catch (java.net.SocketException ex) {
            throw new SocketException(ex);
        } catch (java.lang.SecurityException ex) {
            throw new SocketException(ex);
        }

        return result;
    }

    public static List<String> getInterfacesForMulticast(
            String intf, int protocolSupport) {
        ArrayList<String> interfaces = new ArrayList<>();
        if (isWildcard(intf)) {
            for (InetAddress addr : getLocalAddresses(protocolSupport)) {
                interfaces.add(addr.getHostAddress());
            }
        }
        if (interfaces.isEmpty()) {
            interfaces.add(intf);
        }
        return interfaces;
    }

    public static void setTcpBufSize(
            SocketChannel socket, ProtocolInstance instance) {
        //
        // By default, on Windows we use a 128KB buffer size. On Unix platforms, we use the system
        // defaults.
        //
        int dfltBufSize = 0;
        if (System.getProperty("os.name").startsWith("Windows")) {
            dfltBufSize = 128 * 1024;
        }

        int rcvSize =
            instance.properties().getPropertyAsIntWithDefault("Ice.TCP.RcvSize", dfltBufSize);
        int sndSize =
            instance.properties().getPropertyAsIntWithDefault("Ice.TCP.SndSize", dfltBufSize);

        setTcpBufSize(socket, rcvSize, sndSize, instance);
    }

    public static void setTcpBufSize(
            SocketChannel socket,
            int rcvSize,
            int sndSize,
            ProtocolInstance instance) {
        if (rcvSize > 0) {
            //
            // Try to set the buffer size. The kernel will silently adjust the size to an acceptable
            // value. Then read the size back to get the size that was actually set.
            //
            setRecvBufferSize(socket, rcvSize);
            int size = getRecvBufferSize(socket);
            if (size < rcvSize) {
                // Warn if the size that was set is less than the requested size and we have not
                // already warned.
                BufSizeWarnInfo winfo = instance.getBufSizeWarn(TCPEndpointType.value);
                if (!winfo.rcvWarn || rcvSize != winfo.rcvSize) {
                    instance.logger()
                        .warning(
                            "TCP receive buffer size: requested size of "
                                + rcvSize
                                + " adjusted to "
                                + size);
                    instance.setRcvBufSizeWarn(TCPEndpointType.value, rcvSize);
                }
            }
        }

        if (sndSize > 0) {
            //
            // Try to set the buffer size. The kernel will silently adjust the size to an acceptable
            // value. Then read the size back to get the size that was actually set.
            //
            setSendBufferSize(socket, sndSize);
            int size = getSendBufferSize(socket);
            if (size < sndSize) {
                // Warn if the size that was set is less than the requested size and we have not
                // already warned.
                BufSizeWarnInfo winfo = instance.getBufSizeWarn(TCPEndpointType.value);
                if (!winfo.sndWarn || sndSize != winfo.sndSize) {
                    instance.logger()
                        .warning(
                            "TCP send buffer size: requested size of "
                                + sndSize
                                + " adjusted to "
                                + size);
                    instance.setSndBufSizeWarn(TCPEndpointType.value, sndSize);
                }
            }
        }
    }

    public static void setTcpBufSize(
            ServerSocketChannel socket, ProtocolInstance instance) {
        //
        // By default, on Windows we use a 128KB buffer size. On Unix platforms, we use the system
        // defaults.
        //
        int dfltBufSize = 0;
        if (System.getProperty("os.name").startsWith("Windows")) {
            dfltBufSize = 128 * 1024;
        }

        //
        // Get property for buffer size.
        //
        int sizeRequested =
            instance.properties().getPropertyAsIntWithDefault("Ice.TCP.RcvSize", dfltBufSize);
        if (sizeRequested > 0) {
            //
            // Try to set the buffer size. The kernel will silently adjust the size to an acceptable
            // value. Then read the size back to get the size that was actually set.
            //
            setRecvBufferSize(socket, sizeRequested);
            int size = getRecvBufferSize(socket);
            if (size < sizeRequested) {
                // Warn if the size that was set is less than the requested size and we have not
                // already warned.
                BufSizeWarnInfo winfo = instance.getBufSizeWarn(TCPEndpointType.value);
                if (!winfo.rcvWarn || sizeRequested != winfo.rcvSize) {
                    instance.logger()
                        .warning(
                            "TCP receive buffer size: requested size of "
                                + sizeRequested
                                + " adjusted to "
                                + size);
                    instance.setRcvBufSizeWarn(TCPEndpointType.value, sizeRequested);
                }
            }
        }
    }

    public static String fdToString(
            SelectableChannel fd,
            NetworkProxy proxy,
            InetSocketAddress target) {
        if (fd == null) {
            return "<closed>";
        }

        InetAddress localAddr = null, remoteAddr = null;
        int localPort = -1, remotePort = -1;

        if (fd instanceof SocketChannel) {
            Socket socket = ((SocketChannel) fd).socket();
            localAddr = socket.getLocalAddress();
            localPort = socket.getLocalPort();
            remoteAddr = socket.getInetAddress();
            remotePort = socket.getPort();
        } else if (fd instanceof DatagramChannel) {
            DatagramSocket socket = ((DatagramChannel) fd).socket();
            localAddr = socket.getLocalAddress();
            localPort = socket.getLocalPort();
            remoteAddr = socket.getInetAddress();
            remotePort = socket.getPort();
        } else {
            assert false;
        }

        return addressesToString(localAddr, localPort, remoteAddr, remotePort, proxy, target);
    }

    public static String fdToString(SelectableChannel fd) {
        if (fd == null) {
            return "<closed>";
        }

        InetAddress localAddr = null, remoteAddr = null;
        int localPort = -1, remotePort = -1;

        if (fd instanceof SocketChannel) {
            Socket socket = ((SocketChannel) fd).socket();
            localAddr = socket.getLocalAddress();
            localPort = socket.getLocalPort();
            remoteAddr = socket.getInetAddress();
            remotePort = socket.getPort();
        } else if (fd instanceof DatagramChannel) {
            DatagramSocket socket = ((DatagramChannel) fd).socket();
            localAddr = socket.getLocalAddress();
            localPort = socket.getLocalPort();
            remoteAddr = socket.getInetAddress();
            remotePort = socket.getPort();
        } else {
            assert false;
        }

        return addressesToString(localAddr, localPort, remoteAddr, remotePort);
    }

    public static String addressesToString(
            InetAddress localAddr,
            int localPort,
            InetAddress remoteAddr,
            int remotePort,
            NetworkProxy proxy,
            InetSocketAddress target) {
        StringBuilder s = new StringBuilder(128);
        s.append("local address = ");
        s.append(addrToString(localAddr, localPort));

        if (proxy != null) {
            if (remoteAddr == null) {
                InetSocketAddress addr = proxy.getAddress();
                remoteAddr = addr.getAddress();
                remotePort = addr.getPort();
            }
            s.append("\n");
            s.append(proxy.getName());
            s.append(" proxy address = ");
            s.append(addrToString(remoteAddr, remotePort));
            s.append("\nremote address = ");
            s.append(addrToString(target.getAddress(), target.getPort()));
        } else {
            if (remoteAddr == null && target != null) {
                remoteAddr = target.getAddress();
                remotePort = target.getPort();
            }

            if (remoteAddr == null) {
                s.append("\nremote address = <not connected>");
            } else {
                s.append("\nremote address = ");
                s.append(addrToString(remoteAddr, remotePort));
            }
        }

        return s.toString();
    }

    public static String addressesToString(
            InetAddress localAddr,
            int localPort,
            InetAddress remoteAddr,
            int remotePort) {
        return addressesToString(localAddr, localPort, remoteAddr, remotePort, null, null);
    }

    public static String addrToString(InetSocketAddress addr) {
        StringBuilder s = new StringBuilder(128);
        s.append(addr.getAddress().getHostAddress());
        s.append(':');
        s.append(addr.getPort());
        return s.toString();
    }

    private static boolean isValidAddr(InetAddress addr, int protocol) {
        byte[] bytes = null;
        if (addr != null) {
            bytes = addr.getAddress();
        }
        return bytes != null
            && ((bytes.length == 16 && protocol == EnableIPv6)
            || (bytes.length == 4 && protocol == EnableIPv4));
    }

    public static String addrToString(InetAddress addr, int port) {
        StringBuffer s = new StringBuffer();

        //
        // In early Android releases, sockets don't correctly report their address and port
        // information.
        //

        if (addr == null || addr.isAnyLocalAddress()) {
            s.append("<not available>");
        } else {
            s.append(addr.getHostAddress());
        }

        if (port > 0) {
            s.append(':');
            s.append(port);
        }

        return s.toString();
    }

    private static InetAddress[] getLoopbackAddresses(int protocol) {
        try {
            InetAddress[] addrs = new InetAddress[protocol == EnableBoth ? 2 : 1];
            int i = 0;
            if (protocol != EnableIPv6) {
                addrs[i++] = InetAddress.getByName("127.0.0.1");
            }
            if (protocol != EnableIPv4) {
                addrs[i++] = InetAddress.getByName("::1");
            }
            return addrs;
        } catch (UnknownHostException ex) {
            assert false;
            return null;
        } catch (java.lang.SecurityException ex) {
            throw new SocketException(ex);
        }
    }

    public static InetSocketAddress getNumericAddress(String address) {
        InetSocketAddress addr = null;
        if (!address.isEmpty() && isNumericAddress(address)) {
            try {
                addr = new InetSocketAddress(InetAddress.getByName(address), 0);
            } catch (UnknownHostException ex) {}
        }
        return addr;
    }

    private static boolean isWildcard(String host) {
        if (host == null || host.isEmpty()) {
            return true;
        }
        try {
            return InetAddress.getByName(host).isAnyLocalAddress();
        } catch (UnknownHostException ex) {} catch (java.lang.SecurityException ex) {
            throw new SocketException(ex);
        }
        return false;
    }

    static class IPAddressComparator implements Comparator<InetSocketAddress> {
        IPAddressComparator(boolean ipv6) {
            _ipv6 = ipv6;
        }

        @Override
        public int compare(InetSocketAddress lhs, InetSocketAddress rhs) {
            if (lhs.getAddress().getAddress().length < rhs.getAddress().getAddress().length) {
                return _ipv6 ? 1 : -1;
            } else if (lhs.getAddress().getAddress().length
                > rhs.getAddress().getAddress().length) {
                return _ipv6 ? -1 : 1;
            } else {
                return 0;
            }
        }

        private final boolean _ipv6;
    }

    private static final IPAddressComparator _preferIPv4Comparator = new IPAddressComparator(false);
    private static final IPAddressComparator _preferIPv6Comparator = new IPAddressComparator(true);

    private Network() {}
}
