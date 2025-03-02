// Copyright (c) ZeroC, Inc.

using System.Globalization;
using System.Net;
using System.Net.NetworkInformation;
using System.Net.Sockets;

namespace Ice.Internal;

internal sealed class Network
{
    // ProtocolSupport
    public const int EnableIPv4 = 0;
    public const int EnableIPv6 = 1;
    public const int EnableBoth = 2;

    internal static SocketError socketErrorCode(System.Net.Sockets.SocketException ex)
    {
        return ex.SocketErrorCode;
    }

    internal static bool interrupted(System.Net.Sockets.SocketException ex)
    {
        return socketErrorCode(ex) == SocketError.Interrupted;
    }

    internal static bool wouldBlock(System.Net.Sockets.SocketException ex)
    {
        return socketErrorCode(ex) == SocketError.WouldBlock;
    }

    internal static bool connectionLost(System.Net.Sockets.SocketException ex)
    {
        SocketError error = socketErrorCode(ex);
        return error == SocketError.ConnectionReset ||
               error == SocketError.Shutdown ||
               error == SocketError.ConnectionAborted ||
               error == SocketError.NetworkDown ||
               error == SocketError.NetworkReset;
    }

    internal static bool connectionLost(System.IO.IOException ex)
    {
        //
        // In some cases the IOException has an inner exception that we can pass directly
        // to the other overloading of connectionLost().
        //
        if (ex.InnerException != null && ex.InnerException is System.Net.Sockets.SocketException)
        {
            return connectionLost(ex.InnerException as System.Net.Sockets.SocketException);
        }

        //
        // In other cases the IOException has no inner exception. We could examine the
        // exception's message, but that is fragile due to localization issues. We
        // resort to extracting the value of the protected HResult member via reflection.
        //
        int hr = (int)ex.GetType().GetProperty(
            "HResult",
            System.Reflection.BindingFlags.Instance |
            System.Reflection.BindingFlags.NonPublic |
            System.Reflection.BindingFlags.Public).GetValue(ex, null);

        //
        // This value corresponds to the following errors:
        //
        // "Authentication failed because the remote party has closed the transport stream"
        //
        if (hr == -2146232800)
        {
            return true;
        }
        return false;
    }

    internal static bool connectionRefused(System.Net.Sockets.SocketException ex)
    {
        return socketErrorCode(ex) == SocketError.ConnectionRefused;
    }

    internal static bool recvTruncated(System.Net.Sockets.SocketException ex)
    {
        return socketErrorCode(ex) == SocketError.MessageSize;
    }

    internal static bool timeout(System.IO.IOException ex)
    {
        //
        // TODO: Instead of testing for an English substring, we need to examine the inner
        // exception (if there is one).
        //
        return ex.Message.Contains("period of time", StringComparison.Ordinal);
    }

    internal static bool noMoreFds(System.Exception ex)
    {
        try
        {
            return ex != null && socketErrorCode((System.Net.Sockets.SocketException)ex) == SocketError.TooManyOpenSockets;
        }
        catch (InvalidCastException)
        {
            return false;
        }
    }

    internal static bool isMulticast(IPEndPoint addr)
    {
        string ip = addr.Address.ToString().ToUpperInvariant();
        if (addr.AddressFamily == AddressFamily.InterNetwork)
        {
            char[] splitChars = { '.' };
            string[] arr = ip.Split(splitChars);
            try
            {
                int i = int.Parse(arr[0], CultureInfo.InvariantCulture);
                if (i >= 223 && i <= 239)
                {
                    return true;
                }
            }
            catch (FormatException)
            {
                return false;
            }
        }
        else // AddressFamily.InterNetworkV6
        {
            if (ip.StartsWith("FF", StringComparison.Ordinal))
            {
                return true;
            }
        }
        return false;
    }

    internal static bool isIPv6Supported()
    {
        try
        {
            using Socket socket = new Socket(AddressFamily.InterNetworkV6, SocketType.Stream, ProtocolType.Tcp);
            closeSocketNoThrow(socket);
            return true;
        }
        catch (System.Net.Sockets.SocketException)
        {
            return false;
        }
    }

    internal static Socket createSocket(bool udp, AddressFamily family)
    {
        Socket socket;

        try
        {
            if (udp)
            {
                socket = new Socket(family, SocketType.Dgram, ProtocolType.Udp);
            }
            else
            {
                socket = new Socket(family, SocketType.Stream, ProtocolType.Tcp);
            }
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            throw new Ice.SocketException(ex);
        }
        catch (ArgumentException ex)
        {
            throw new Ice.SocketException(ex);
        }

        if (!udp)
        {
            try
            {
                setTcpNoDelay(socket);
                socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive, 1);
            }
            catch (System.Net.Sockets.SocketException ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }
        return socket;
    }

    internal static Socket createServerSocket(bool udp, AddressFamily family, int protocol)
    {
        Socket socket = createSocket(udp, family);
        if (family == AddressFamily.InterNetworkV6 && protocol != EnableIPv4)
        {
            try
            {
                int flag = protocol == EnableIPv6 ? 1 : 0;
                socket.SetSocketOption(SocketOptionLevel.IPv6, SocketOptionName.IPv6Only, flag);
            }
            catch (System.Net.Sockets.SocketException ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }
        return socket;
    }

    internal static void closeSocketNoThrow(Socket socket)
    {
        if (socket == null)
        {
            return;
        }
        try
        {
            socket.Close();
        }
        catch (System.Net.Sockets.SocketException)
        {
            // Ignore
        }
    }

    internal static void closeSocket(Socket socket)
    {
        if (socket == null)
        {
            return;
        }
        try
        {
            socket.Close();
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            throw new Ice.SocketException(ex);
        }
    }

    internal static void setTcpNoDelay(Socket socket)
    {
        try
        {
            socket.SetSocketOption(SocketOptionLevel.Tcp, SocketOptionName.NoDelay, 1);
        }
        catch (System.Exception ex)
        {
            closeSocketNoThrow(socket);
            throw new Ice.SocketException(ex);
        }
    }

    internal static void setBlock(Socket socket, bool block)
    {
        try
        {
            socket.Blocking = block;
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            closeSocketNoThrow(socket);
            throw new Ice.SocketException(ex);
        }
    }

    internal static void setSendBufferSize(Socket socket, int sz)
    {
        try
        {
            socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer, sz);
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            closeSocketNoThrow(socket);
            throw new Ice.SocketException(ex);
        }
    }

    internal static int getSendBufferSize(Socket socket)
    {
        int sz;
        try
        {
            sz = (int)socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer);
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            closeSocketNoThrow(socket);
            throw new Ice.SocketException(ex);
        }
        return sz;
    }

    internal static void setRecvBufferSize(Socket socket, int sz)
    {
        try
        {
            socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer, sz);
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            closeSocketNoThrow(socket);
            throw new Ice.SocketException(ex);
        }
    }

    internal static int getRecvBufferSize(Socket socket)
    {
        int sz = 0;
        try
        {
            sz = (int)socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer);
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            closeSocketNoThrow(socket);
            throw new Ice.SocketException(ex);
        }
        return sz;
    }

    internal static void setReuseAddress(Socket socket, bool reuse)
    {
        try
        {
            socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, reuse ? 1 : 0);
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            closeSocketNoThrow(socket);
            throw new Ice.SocketException(ex);
        }
    }

    internal static void setMcastInterface(Socket socket, string iface, AddressFamily family)
    {
        try
        {
            if (family == AddressFamily.InterNetwork)
            {
                socket.SetSocketOption(
                    SocketOptionLevel.IP,
                    SocketOptionName.MulticastInterface,
                    getInterfaceAddress(iface, family).GetAddressBytes());
            }
            else
            {
                socket.SetSocketOption(
                    SocketOptionLevel.IPv6,
                    SocketOptionName.MulticastInterface,
                    getInterfaceIndex(iface, family));
            }
        }
        catch (System.Exception ex)
        {
            closeSocketNoThrow(socket);
            throw new Ice.SocketException(ex);
        }
    }

    internal static void setMcastGroup(Socket s, IPAddress group, string iface)
    {
        try
        {
            var indexes = new HashSet<int>();
            foreach (string intf in getInterfacesForMulticast(iface, getProtocolSupport(group)))
            {
                if (group.AddressFamily == AddressFamily.InterNetwork)
                {
                    MulticastOption option;
                    IPAddress addr = getInterfaceAddress(intf, group.AddressFamily);
                    if (addr == null)
                    {
                        option = new MulticastOption(group);
                    }
                    else
                    {
                        option = new MulticastOption(group, addr);
                    }
                    s.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.AddMembership, option);
                }
                else
                {
                    int index = getInterfaceIndex(intf, group.AddressFamily);
                    if (indexes.Add(index))
                    {
                        IPv6MulticastOption option;
                        if (index == -1)
                        {
                            option = new IPv6MulticastOption(group);
                        }
                        else
                        {
                            option = new IPv6MulticastOption(group, index);
                        }
                        s.SetSocketOption(SocketOptionLevel.IPv6, SocketOptionName.AddMembership, option);
                    }
                }
            }
        }
        catch (System.Exception ex)
        {
            closeSocketNoThrow(s);
            throw new Ice.SocketException(ex);
        }
    }

    internal static void setMcastTtl(Socket socket, int ttl, AddressFamily family)
    {
        try
        {
            if (family == AddressFamily.InterNetwork)
            {
                socket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.MulticastTimeToLive, ttl);
            }
            else
            {
                socket.SetSocketOption(SocketOptionLevel.IPv6, SocketOptionName.MulticastTimeToLive, ttl);
            }
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            closeSocketNoThrow(socket);
            throw new Ice.SocketException(ex);
        }
    }

    internal static IPEndPoint doBind(Socket socket, EndPoint addr)
    {
        try
        {
            socket.Bind(addr);
            return (IPEndPoint)socket.LocalEndPoint;
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            closeSocketNoThrow(socket);
            throw new Ice.SocketException(ex);
        }
    }

    internal static void doListen(Socket socket, int backlog)
    {
    repeatListen:

        try
        {
            socket.Listen(backlog);
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            if (interrupted(ex))
            {
                goto repeatListen;
            }

            closeSocketNoThrow(socket);
            throw new Ice.SocketException(ex);
        }
    }

    internal static int getProtocolSupport(IPAddress addr)
    {
        return addr.AddressFamily == AddressFamily.InterNetwork ? EnableIPv4 : EnableIPv6;
    }

    internal static EndPoint getAddressForServer(string host, int port, int protocol, bool preferIPv6)
    {
        if (host.Length == 0)
        {
            if (protocol != EnableIPv4)
            {
                return new IPEndPoint(IPAddress.IPv6Any, port);
            }
            else
            {
                return new IPEndPoint(IPAddress.Any, port);
            }
        }
        return getAddresses(host, port, protocol, preferIPv6, true)[0];
    }

    internal static List<EndPoint> getAddresses(
        string host,
        int port,
        int protocol,
        bool preferIPv6,
        bool blocking)
    {
        List<EndPoint> addresses = new List<EndPoint>();
        if (host.Length == 0)
        {
            foreach (IPAddress a in getLoopbackAddresses(protocol))
            {
                addresses.Add(new IPEndPoint(a, port));
            }
            if (protocol == EnableBoth)
            {
                if (preferIPv6)
                {
                    Ice.UtilInternal.Collections.Sort(ref addresses, _preferIPv6Comparator);
                }
                else
                {
                    Ice.UtilInternal.Collections.Sort(ref addresses, _preferIPv4Comparator);
                }
            }
            return addresses;
        }

        int retry = 5;

    repeatGetHostByName:
        try
        {
            //
            // No need for lookup if host is ip address.
            //
            try
            {
                IPAddress addr = IPAddress.Parse(host);
                if ((addr.AddressFamily == AddressFamily.InterNetwork && protocol != EnableIPv6) ||
                   (addr.AddressFamily == AddressFamily.InterNetworkV6 && protocol != EnableIPv4))
                {
                    addresses.Add(new IPEndPoint(addr, port));
                    return addresses;
                }
                else
                {
                    throw new DNSException(host);
                }
            }
            catch (FormatException)
            {
                if (!blocking)
                {
                    return addresses;
                }
            }

            foreach (IPAddress a in Dns.GetHostAddresses(host))
            {
                if ((a.AddressFamily == AddressFamily.InterNetwork && protocol != EnableIPv6) ||
                   (a.AddressFamily == AddressFamily.InterNetworkV6 && protocol != EnableIPv4))
                {
                    addresses.Add(new IPEndPoint(a, port));
                }
            }

            if (protocol == EnableBoth)
            {
                if (preferIPv6)
                {
                    Ice.UtilInternal.Collections.Sort(ref addresses, _preferIPv6Comparator);
                }
                else
                {
                    Ice.UtilInternal.Collections.Sort(ref addresses, _preferIPv4Comparator);
                }
            }
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            if (socketErrorCode(ex) == SocketError.TryAgain && --retry >= 0)
            {
                goto repeatGetHostByName;
            }
            throw new DNSException(host, ex);
        }
        catch (System.Exception ex)
        {
            throw new DNSException(host, ex);
        }

        //
        // No InterNetwork/InterNetworkV6 available.
        //
        if (addresses.Count == 0)
        {
            throw new DNSException(host);
        }
        return addresses;
    }

    internal static IPAddress[] getLocalAddresses(int protocol, bool singleAddressPerInterface)
    {
        List<IPAddress> addresses;
        int retry = 5;

    repeatGetHostByName:
        try
        {
            addresses = new List<IPAddress>();
            NetworkInterface[] nics = NetworkInterface.GetAllNetworkInterfaces();
            foreach (NetworkInterface ni in nics)
            {
                IPInterfaceProperties ipProps = ni.GetIPProperties();
                UnicastIPAddressInformationCollection uniColl = ipProps.UnicastAddresses;
                foreach (UnicastIPAddressInformation uni in uniColl)
                {
                    if ((uni.Address.AddressFamily == AddressFamily.InterNetwork && protocol != EnableIPv6) ||
                       (uni.Address.AddressFamily == AddressFamily.InterNetworkV6 && protocol != EnableIPv4))
                    {
                        if (!addresses.Contains(uni.Address))
                        {
                            addresses.Add(uni.Address);
                            if (singleAddressPerInterface)
                            {
                                break;
                            }
                        }
                    }
                }
            }
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            if (socketErrorCode(ex) == SocketError.TryAgain && --retry >= 0)
            {
                goto repeatGetHostByName;
            }
            throw new DNSException("0.0.0.0", ex);
        }
        catch (System.Exception ex)
        {
            throw new DNSException("0.0.0.0", ex);
        }

        return addresses.ToArray();
    }

    internal static void setTcpBufSize(Socket socket, ProtocolInstance instance)
    {
        //
        // By default, on Windows we use a 128KB buffer size. On Unix
        // platforms, we use the system defaults.
        //
        int dfltBufSize = 0;
        if (AssemblyUtil.isWindows)
        {
            dfltBufSize = 128 * 1024;
        }
        int rcvSize = instance.properties().getPropertyAsIntWithDefault("Ice.TCP.RcvSize", dfltBufSize);
        int sndSize = instance.properties().getPropertyAsIntWithDefault("Ice.TCP.SndSize", dfltBufSize);
        setTcpBufSize(socket, rcvSize, sndSize, instance);
    }

    internal static void setTcpBufSize(Socket socket, int rcvSize, int sndSize, ProtocolInstance instance)
    {
        if (rcvSize > 0)
        {
            //
            // Try to set the buffer size. The kernel will silently adjust
            // the size to an acceptable value. Then read the size back to
            // get the size that was actually set.
            //
            setRecvBufferSize(socket, rcvSize);
            int size = getRecvBufferSize(socket);
            if (size < rcvSize)
            {
                // Warn if the size that was set is less than the requested size and
                // we have not already warned.
                BufSizeWarnInfo winfo = instance.getBufSizeWarn(Ice.TCPEndpointType.value);
                if (!winfo.rcvWarn || rcvSize != winfo.rcvSize)
                {
                    instance.logger().warning("TCP receive buffer size: requested size of " + rcvSize +
                                              " adjusted to " + size);
                    instance.setRcvBufSizeWarn(Ice.TCPEndpointType.value, rcvSize);
                }
            }
        }

        if (sndSize > 0)
        {
            //
            // Try to set the buffer size. The kernel will silently adjust
            // the size to an acceptable value. Then read the size back to
            // get the size that was actually set.
            //
            setSendBufferSize(socket, sndSize);
            int size = getSendBufferSize(socket);
            if (size < sndSize) // Warn if the size that was set is less than the requested size.
            {
                // Warn if the size that was set is less than the requested size and
                // we have not already warned.
                BufSizeWarnInfo winfo = instance.getBufSizeWarn(Ice.TCPEndpointType.value);
                if (!winfo.sndWarn || sndSize != winfo.sndSize)
                {
                    instance.logger().warning("TCP send buffer size: requested size of " + sndSize +
                                              " adjusted to " + size);
                    instance.setSndBufSizeWarn(Ice.TCPEndpointType.value, sndSize);
                }
            }
        }
    }

    internal static List<string> getInterfacesForMulticast(string intf, int protocol)
    {
        List<string> interfaces = new List<string>();
        bool ipv4Wildcard = false;
        if (isWildcard(intf, out ipv4Wildcard))
        {
            foreach (IPAddress a in getLocalAddresses(ipv4Wildcard ? EnableIPv4 : protocol, true))
            {
                interfaces.Add(a.ToString());
            }
        }
        if (interfaces.Count == 0)
        {
            interfaces.Add(intf);
        }
        return interfaces;
    }

    internal static string fdToString(Socket socket, NetworkProxy proxy, EndPoint target)
    {
        try
        {
            if (socket == null)
            {
                return "<closed>";
            }

            EndPoint remote = getRemoteAddress(socket);

            System.Text.StringBuilder s = new System.Text.StringBuilder();
            s.Append("local address = " + localAddrToString(getLocalAddress(socket)));
            if (proxy != null)
            {
                if (remote == null)
                {
                    remote = proxy.getAddress();
                }
                s.Append("\n" + proxy.getName() + " proxy address = " + remoteAddrToString(remote));
                s.Append("\nremote address = " + remoteAddrToString(target));
            }
            else
            {
                if (remote == null)
                {
                    remote = target;
                }
                s.Append("\nremote address = " + remoteAddrToString(remote));
            }
            return s.ToString();
        }
        catch (ObjectDisposedException)
        {
            return "<closed>";
        }
    }

    internal static string fdToString(Socket socket)
    {
        try
        {
            if (socket == null)
            {
                return "<closed>";
            }
            System.Text.StringBuilder s = new System.Text.StringBuilder();
            s.Append("local address = " + localAddrToString(getLocalAddress(socket)));
            s.Append("\nremote address = " + remoteAddrToString(getRemoteAddress(socket)));
            return s.ToString();
        }
        catch (ObjectDisposedException)
        {
            return "<closed>";
        }
    }

    internal static string
    addrToString(EndPoint addr)
    {
        return endpointAddressToString(addr) + ":" + endpointPort(addr);
    }

    internal static string localAddrToString(EndPoint endpoint)
    {
        if (endpoint == null)
        {
            return "<not bound>";
        }
        return endpointAddressToString(endpoint) + ":" + endpointPort(endpoint);
    }

    internal static string remoteAddrToString(EndPoint endpoint)
    {
        if (endpoint == null)
        {
            return "<not connected>";
        }
        return endpointAddressToString(endpoint) + ":" + endpointPort(endpoint);
    }

    internal static EndPoint getLocalAddress(Socket socket)
    {
        try
        {
            return socket.LocalEndPoint;
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            throw new Ice.SocketException(ex);
        }
    }

    internal static EndPoint getRemoteAddress(Socket socket)
    {
        try
        {
            return socket.RemoteEndPoint;
        }
        catch (System.Net.Sockets.SocketException)
        {
        }
        return null;
    }

    private static IPAddress getInterfaceAddress(string iface, AddressFamily family)
    {
        if (iface.Length == 0)
        {
            return null;
        }

        //
        // The iface parameter must either be an IP address, an
        // index or the name of an interface. If it's an index we
        // just return it. If it's an IP address we search for an
        // interface which has this IP address. If it's a name we
        // search an interface with this name.
        //

        try
        {
            return IPAddress.Parse(iface);
        }
        catch (FormatException)
        {
        }

        NetworkInterface[] nics = NetworkInterface.GetAllNetworkInterfaces();
        try
        {
            int index = int.Parse(iface, CultureInfo.InvariantCulture);
            foreach (NetworkInterface ni in nics)
            {
                IPInterfaceProperties ipProps = ni.GetIPProperties();
                int interfaceIndex = -1;
                if (family == AddressFamily.InterNetwork)
                {
                    IPv4InterfaceProperties ipv4Props = ipProps.GetIPv4Properties();
                    if (ipv4Props != null && ipv4Props.Index == index)
                    {
                        interfaceIndex = ipv4Props.Index;
                    }
                }
                else
                {
                    IPv6InterfaceProperties ipv6Props = ipProps.GetIPv6Properties();
                    if (ipv6Props != null && ipv6Props.Index == index)
                    {
                        interfaceIndex = ipv6Props.Index;
                    }
                }
                if (interfaceIndex >= 0)
                {
                    foreach (UnicastIPAddressInformation a in ipProps.UnicastAddresses)
                    {
                        if (a.Address.AddressFamily == family)
                        {
                            return a.Address;
                        }
                    }
                }
            }
        }
        catch (FormatException)
        {
        }

        foreach (NetworkInterface ni in nics)
        {
            if (ni.Name == iface)
            {
                IPInterfaceProperties ipProps = ni.GetIPProperties();
                foreach (UnicastIPAddressInformation a in ipProps.UnicastAddresses)
                {
                    if (a.Address.AddressFamily == family)
                    {
                        return a.Address;
                    }
                }
            }
        }

        throw new ArgumentException("couldn't find interface `" + iface + "'");
    }

    private static int getInterfaceIndex(string iface, AddressFamily family)
    {
        if (iface.Length == 0)
        {
            return -1;
        }

        //
        // The iface parameter must either be an IP address, an
        // index or the name of an interface. If it's an index we
        // just return it. If it's an IP address we search for an
        // interface which has this IP address. If it's a name we
        // search an interface with this name.
        //
        try
        {
            return int.Parse(iface, CultureInfo.InvariantCulture);
        }
        catch (FormatException)
        {
        }

        NetworkInterface[] nics = NetworkInterface.GetAllNetworkInterfaces();
        try
        {
            IPAddress addr = IPAddress.Parse(iface);
            foreach (NetworkInterface ni in nics)
            {
                IPInterfaceProperties ipProps = ni.GetIPProperties();
                foreach (UnicastIPAddressInformation uni in ipProps.UnicastAddresses)
                {
                    if (uni.Address.Equals(addr))
                    {
                        if (addr.AddressFamily == AddressFamily.InterNetwork)
                        {
                            IPv4InterfaceProperties ipv4Props = ipProps.GetIPv4Properties();
                            if (ipv4Props != null)
                            {
                                return ipv4Props.Index;
                            }
                        }
                        else
                        {
                            IPv6InterfaceProperties ipv6Props = ipProps.GetIPv6Properties();
                            if (ipv6Props != null)
                            {
                                return ipv6Props.Index;
                            }
                        }
                    }
                }
            }
        }
        catch (FormatException)
        {
        }

        foreach (NetworkInterface ni in nics)
        {
            if (ni.Name == iface)
            {
                IPInterfaceProperties ipProps = ni.GetIPProperties();
                if (family == AddressFamily.InterNetwork)
                {
                    IPv4InterfaceProperties ipv4Props = ipProps.GetIPv4Properties();
                    if (ipv4Props != null)
                    {
                        return ipv4Props.Index;
                    }
                }
                else
                {
                    IPv6InterfaceProperties ipv6Props = ipProps.GetIPv6Properties();
                    if (ipv6Props != null)
                    {
                        return ipv6Props.Index;
                    }
                }
            }
        }

        throw new ArgumentException("couldn't find interface `" + iface + "'");
    }

    internal static EndPoint getNumericAddress(string sourceAddress)
    {
        EndPoint addr = null;
        if (!string.IsNullOrEmpty(sourceAddress))
        {
            List<EndPoint> addrs = getAddresses(
                sourceAddress,
                0,
                EnableBoth,
                preferIPv6: false,
                blocking: false);
            if (addrs.Count != 0)
            {
                return addrs[0];
            }
        }
        return addr;
    }

    private static bool isWildcard(string address, out bool ipv4Wildcard)
    {
        ipv4Wildcard = false;
        if (address.Length == 0)
        {
            return true;
        }

        try
        {
            IPAddress addr = IPAddress.Parse(address);
            if (addr.Equals(IPAddress.Any))
            {
                ipv4Wildcard = true;
                return true;
            }
            return addr.Equals(IPAddress.IPv6Any);
        }
        catch (System.Exception)
        {
        }

        return false;
    }

    internal static List<IPAddress> getLoopbackAddresses(int protocol)
    {
        List<IPAddress> addresses = new List<IPAddress>();
        if (protocol != EnableIPv4)
        {
            addresses.Add(IPAddress.IPv6Loopback);
        }
        if (protocol != EnableIPv6)
        {
            addresses.Add(IPAddress.Loopback);
        }
        return addresses;
    }

    internal static bool addressEquals(EndPoint addr1, EndPoint addr2)
    {
        if (addr1 == null)
        {
            if (addr2 == null)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (addr2 == null)
        {
            return false;
        }

        return addr1.Equals(addr2);
    }

    internal static string endpointAddressToString(EndPoint endpoint)
    {
        if (endpoint != null)
        {
            if (endpoint is IPEndPoint)
            {
                IPEndPoint ipEndpoint = (IPEndPoint)endpoint;
                return ipEndpoint.Address.ToString();
            }
        }
        return "";
    }

    internal static int endpointPort(EndPoint endpoint)
    {
        if (endpoint != null)
        {
            if (endpoint is IPEndPoint)
            {
                IPEndPoint ipEndpoint = (IPEndPoint)endpoint;
                return ipEndpoint.Port;
            }
        }
        return -1;
    }

    private class EndPointComparator : IComparer<EndPoint>
    {
        public EndPointComparator(bool ipv6)
        {
            _ipv6 = ipv6;
        }

        public int Compare(EndPoint lhs, EndPoint rhs)
        {
            if (lhs.AddressFamily == AddressFamily.InterNetwork &&
               rhs.AddressFamily == AddressFamily.InterNetworkV6)
            {
                return _ipv6 ? 1 : -1;
            }
            else if (lhs.AddressFamily == AddressFamily.InterNetworkV6 &&
                    rhs.AddressFamily == AddressFamily.InterNetwork)
            {
                return _ipv6 ? -1 : 1;
            }
            else
            {
                return 0;
            }
        }

        private readonly bool _ipv6;
    }

    private static readonly EndPointComparator _preferIPv4Comparator = new EndPointComparator(false);
    private static readonly EndPointComparator _preferIPv6Comparator = new EndPointComparator(true);
}
