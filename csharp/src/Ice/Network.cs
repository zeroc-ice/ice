//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Net;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    internal class Network
    {
        // Which versions of the Internet Protocol are enabled?
        public const int EnableIPv4 = 0;
        public const int EnableIPv6 = 1;
        public const int EnableBoth = 2;

        public static bool ConnectionLost(Exception ex)
        {
            Debug.Assert(!(ex is TransportException));

            // Check the inner exceptions if the given exception isn't a socket exception. Streams wrapping a socket
            // typically throw an IOException with the SocketException as the InnerException.
            while (!(ex is SocketException) && ex.InnerException != null)
            {
                ex = ex.InnerException;
            }

            if (ex is SocketException socketException)
            {
                SocketError error = socketException.SocketErrorCode;
                return error == SocketError.ConnectionReset ||
                       error == SocketError.Shutdown ||
                       error == SocketError.ConnectionAborted ||
                       error == SocketError.NetworkDown ||
                       error == SocketError.NetworkReset;
            }

            // In other cases the IOException has no inner exception. We could examine the
            // exception's message, but that is fragile due to localization issues. We
            // resort to extracting the value of the protected HResult member via reflection.
            System.Reflection.PropertyInfo? hresult = ex.GetType().GetProperty("HResult",
                System.Reflection.BindingFlags.Instance |
                System.Reflection.BindingFlags.NonPublic |
                System.Reflection.BindingFlags.Public);
            if (hresult != null && hresult.GetValue(ex, null) is int hresultValue &&
                hresultValue == -2146232800)
            {
                // This value corresponds to the following errors:
                //
                // "Authentication failed because the remote party has closed the transport stream"
                return true;
            }

            return false;
        }

        public static bool IsMulticast(IPEndPoint addr)
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

        public static bool IsIPv6Supported()
        {
            try
            {
                var socket = new Socket(AddressFamily.InterNetworkV6, SocketType.Stream, ProtocolType.Tcp);
                CloseSocketNoThrow(socket);
                return true;
            }
            catch (SocketException)
            {
                return false;
            }
        }

        public static Socket CreateSocket(bool udp, AddressFamily family)
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
            catch (SocketException ex)
            {
                throw new TransportException(ex);
            }

            if (!udp)
            {
                try
                {
                    socket.SetSocketOption(SocketOptionLevel.Tcp, SocketOptionName.NoDelay, 1);
                    socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive, 1);
                    //
                    // FIX: the fast path loopback appears to cause issues with
                    // connection closure when it's enabled. Sometime, a peer
                    // doesn't receive the TCP/IP connection closure (RST) from
                    // the other peer and it ends up hanging. See bug #6093.
                    //
                    //setTcpLoopbackFastPath(socket);
                }
                catch (SocketException ex)
                {
                    CloseSocketNoThrow(socket);
                    throw new TransportException(ex);
                }
            }
            return socket;
        }

        public static Socket CreateServerSocket(bool udp, AddressFamily family, int ipVersion)
        {
            Socket socket = CreateSocket(udp, family);
            if (family == AddressFamily.InterNetworkV6 && ipVersion != EnableIPv4)
            {
                try
                {
                    int flag = ipVersion == EnableIPv6 ? 1 : 0;
                    socket.SetSocketOption(SocketOptionLevel.IPv6, SocketOptionName.IPv6Only, flag);
                }
                catch (SocketException ex)
                {
                    CloseSocketNoThrow(socket);
                    throw new TransportException(ex);
                }
            }
            return socket;
        }

        public static void CloseSocketNoThrow(Socket socket)
        {
            if (socket == null)
            {
                return;
            }
            try
            {
                socket.Close();
            }
            catch (SocketException)
            {
                // Ignore
            }
        }

        public static void SetMulticastInterface(Socket socket, string iface, AddressFamily family)
        {
            if (family == AddressFamily.InterNetwork)
            {
                socket.SetSocketOption(SocketOptionLevel.IP,
                                        SocketOptionName.MulticastInterface,
                                        GetInterfaceAddress(iface, family)!.GetAddressBytes());
            }
            else
            {
                socket.SetSocketOption(SocketOptionLevel.IPv6,
                                        SocketOptionName.MulticastInterface,
                                        GetInterfaceIndex(iface, family));
            }
        }

        public static void SetMulticastGroup(Socket s, IPAddress group, string iface)
        {
            var indexes = new HashSet<int>();
            foreach (string intf in GetInterfacesForMulticast(iface, GetIPVersion(group)))
            {
                if (group.AddressFamily == AddressFamily.InterNetwork)
                {
                    MulticastOption option;
                    IPAddress? addr = GetInterfaceAddress(intf, group.AddressFamily);
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
                    int index = GetInterfaceIndex(intf, group.AddressFamily);
                    if (!indexes.Contains(index))
                    {
                        indexes.Add(index);
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

        public static void SetMulticastTtl(Socket socket, int ttl, AddressFamily family)
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

        public static int GetIPVersion(IPAddress addr) =>
            addr.AddressFamily == AddressFamily.InterNetwork ? EnableIPv4 : EnableIPv6;

        public static IPEndPoint GetAddressForServerEndpoint(string host, int port, int ipVersion, bool preferIPv6)
        {
            // TODO: Fix this method to be asynchronous.

            // For server endpoints, an empty host is the same as the "any" address
            if (host.Length == 0)
            {
                if (ipVersion != EnableIPv4)
                {
                    return new IPEndPoint(IPAddress.IPv6Any, port);
                }
                else
                {
                    return new IPEndPoint(IPAddress.Any, port);
                }
            }

            try
            {
                // Get the addresses for the given host and return the first one
                ValueTask<IEnumerable<IPEndPoint>> task = GetAddressesAsync(host,
                                                                            port,
                                                                            ipVersion,
                                                                            EndpointSelectionType.Ordered,
                                                                            preferIPv6);
                return (task.IsCompleted ? task.Result : task.AsTask().Result).First();
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
        }

        public static async ValueTask<IEnumerable<IPEndPoint>> GetAddressesForClientEndpointAsync(
            string host,
            int port,
            int ipVersion,
            EndpointSelectionType selType,
            bool preferIPv6,
            CancellationToken cancel)
        {
            // For client endpoints, an empty host is the same as the loopback address
            if (host.Length == 0)
            {
                var addresses = new List<IPEndPoint>();
                foreach (IPAddress a in GetLoopbackAddresses(ipVersion))
                {
                    addresses.Add(new IPEndPoint(a, port));
                }

                if (ipVersion == EnableBoth)
                {
                    if (preferIPv6)
                    {
                        return addresses.OrderByDescending(addr => addr.AddressFamily);
                    }
                    else
                    {
                        return addresses.OrderBy(addr => addr.AddressFamily);
                    }
                }
                return addresses;
            }

            return await GetAddressesAsync(host, port, ipVersion, selType, preferIPv6, cancel).ConfigureAwait(false);
        }

        public static IEnumerable<IPEndPoint> GetAddresses(
            string host,
            int port,
            int ipVersion,
            EndpointSelectionType selType,
            bool preferIPv6)
        {
            try
            {
                ValueTask<IEnumerable<IPEndPoint>> task = GetAddressesAsync(host,
                                                                            port,
                                                                            ipVersion,
                                                                            selType,
                                                                            preferIPv6);
                return task.IsCompleted ? task.Result : task.AsTask().Result;
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
        }

        public static async ValueTask<IEnumerable<IPEndPoint>> GetAddressesAsync(
            string host,
            int port,
            int ipVersion,
            EndpointSelectionType selType,
            bool preferIPv6,
            CancellationToken cancel = default)
        {
            Debug.Assert(host.Length > 0);

            int retry = 5;
            while (true)
            {
                var addresses = new List<IPEndPoint>();
                try
                {
                    // Trying to parse the IP address is necessary to handle wildcard addresses such as 0.0.0.0 or ::0
                    // since GetHostAddressesAsync fails to resolve them.
                    var a = IPAddress.Parse(host);
                    if ((a.AddressFamily == AddressFamily.InterNetwork && ipVersion != EnableIPv6) ||
                        (a.AddressFamily == AddressFamily.InterNetworkV6 && ipVersion != EnableIPv4))
                    {
                        addresses.Add(new IPEndPoint(a, port));
                        return addresses;
                    }
                    else
                    {
                        throw new DNSException(host);
                    }
                }
                catch (FormatException)
                {
                }

                try
                {
                    foreach (IPAddress a in await Dns.GetHostAddressesAsync(host).WaitAsync(cancel).ConfigureAwait(false))
                    {
                        if ((a.AddressFamily == AddressFamily.InterNetwork && ipVersion != EnableIPv6) ||
                            (a.AddressFamily == AddressFamily.InterNetworkV6 && ipVersion != EnableIPv4))
                        {
                            addresses.Add(new IPEndPoint(a, port));
                        }
                    }

                    //
                    // No InterNetwork/InterNetworkV6 available.
                    //
                    if (addresses.Count == 0)
                    {
                        throw new DNSException(host);
                    }

                    IEnumerable<IPEndPoint> addrs = addresses;
                    if (selType == EndpointSelectionType.Random)
                    {
                        addrs = addrs.Shuffle();
                    }

                    if (ipVersion == EnableBoth)
                    {
                        if (preferIPv6)
                        {
                            return addrs.OrderByDescending(addr => addr.AddressFamily);
                        }
                        else
                        {
                            return addrs.OrderBy(addr => addr.AddressFamily);
                        }
                    }
                    return addrs;
                }
                catch (DNSException)
                {
                    throw;
                }
                catch (SocketException ex)
                {
                    if (ex.SocketErrorCode == SocketError.TryAgain && --retry >= 0)
                    {
                        continue;
                    }
                    throw new DNSException(host, ex);
                }
                catch (Exception ex)
                {
                    throw new DNSException(host, ex);
                }
            }
        }

        public static IPAddress[] GetLocalAddresses(int ipVersion, bool includeLoopback, bool singleAddressPerInterface)
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
                        if ((uni.Address.AddressFamily == AddressFamily.InterNetwork && ipVersion != EnableIPv6) ||
                           (uni.Address.AddressFamily == AddressFamily.InterNetworkV6 && ipVersion != EnableIPv4))
                        {
                            if (!addresses.Contains(uni.Address) &&
                               (includeLoopback || !IPAddress.IsLoopback(uni.Address)))
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
            catch (SocketException ex)
            {
                if (ex.SocketErrorCode == SocketError.TryAgain && --retry >= 0)
                {
                    goto repeatGetHostByName;
                }
                throw new DNSException("0.0.0.0", ex);
            }
            catch (Exception ex)
            {
                throw new DNSException("0.0.0.0", ex);
            }

            return addresses.ToArray();
        }

        public static bool IsLinklocal(IPAddress addr)
        {
            if (addr.IsIPv6LinkLocal)
            {
                return true;
            }
            else if (addr.AddressFamily == AddressFamily.InterNetwork)
            {
                byte[] bytes = addr.GetAddressBytes();
                return bytes[0] == 169 && bytes[1] == 254;
            }
            return false;
        }

        public static void SetBufSize(Socket socket, Communicator communicator, Transport transport)
        {
            int rcvSize = communicator.GetPropertyAsByteSize($"Ice.{transport}.RcvSize") ?? 0;
            if (rcvSize > 0)
            {
                // Try to set the buffer size. The kernel will silently adjust the size to an acceptable value. Then
                // read the size back to get the size that was actually set.
                socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer, rcvSize);
                int size = (int)socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer);
                if (size < rcvSize)
                {
                    // Warn if the size that was set is less than the requested size and we have not already warned.
                    BufSizeWarnInfo warningInfo = communicator.GetBufSizeWarn(Transport.TCP);
                    if (!warningInfo.RcvWarn || rcvSize != warningInfo.RcvSize)
                    {
                        communicator.Logger.Warning(
                            $"{transport} receive buffer size: requested size of {rcvSize} adjusted to {size}");
                        communicator.SetRcvBufSizeWarn(Transport.TCP, rcvSize);
                    }
                }
            }

            int sndSize = communicator.GetPropertyAsByteSize($"Ice.{transport}.SndSize") ?? 0;
            if (sndSize > 0)
            {
                // Try to set the buffer size. The kernel will silently adjust the size to an acceptable value. Then
                // read the size back to get the size that was actually set.
                socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer, sndSize);
                int size = (int)socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer);
                if (size < sndSize) // Warn if the size that was set is less than the requested size.
                {
                    // Warn if the size that was set is less than the requested size and we have not already warned.
                    BufSizeWarnInfo warningInfo = communicator.GetBufSizeWarn(Transport.TCP);
                    if (!warningInfo.SndWarn || sndSize != warningInfo.SndSize)
                    {
                        communicator.Logger.Warning(
                            $"{transport} send buffer size: requested size of {sndSize} adjusted to {size}");
                        communicator.SetSndBufSizeWarn(Transport.TCP, sndSize);
                    }
                }
            }
        }

        public static List<string> GetHostsForEndpointExpand(string host, int ipVersion, bool includeLoopback)
        {
            var hosts = new List<string>();
            if (IsWildcard(host, out bool ipv4Wildcard))
            {
                foreach (IPAddress a in GetLocalAddresses(ipv4Wildcard ? EnableIPv4 : ipVersion, includeLoopback, false))
                {
                    if (!IsLinklocal(a))
                    {
                        hosts.Add(a.ToString());
                    }
                }
                if (hosts.Count == 0)
                {
                    // Return loopback if only loopback is available no other local addresses are available.
                    foreach (IPAddress a in GetLoopbackAddresses(ipVersion))
                    {
                        hosts.Add(a.ToString());
                    }
                }
            }
            return hosts;
        }

        public static List<string> GetInterfacesForMulticast(string intf, int ipVersion)
        {
            var interfaces = new List<string>();
            if (IsWildcard(intf, out bool ipv4Wildcard))
            {
                foreach (IPAddress a in GetLocalAddresses(ipv4Wildcard ? EnableIPv4 : ipVersion, true, true))
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

        public static string FdToString(Socket socket, INetworkProxy? proxy, EndPoint? target)
        {
            try
            {
                if (socket == null)
                {
                    return "<closed>";
                }

                EndPoint? remote = GetRemoteAddress(socket);

                var s = new System.Text.StringBuilder();
                s.Append("local address = " + LocalAddrToString(GetLocalAddress(socket)));
                if (proxy != null)
                {
                    if (remote == null)
                    {
                        remote = proxy.Address;
                    }
                    s.Append("\n" + proxy.Name + " proxy address = " + RemoteAddrToString(remote));
                    s.Append("\nremote address = " + RemoteAddrToString(target));
                }
                else
                {
                    if (remote == null)
                    {
                        remote = target;
                    }
                    s.Append("\nremote address = " + RemoteAddrToString(remote));
                }
                return s.ToString();
            }
            catch (ObjectDisposedException)
            {
                return "<closed>";
            }
        }

        public static string SocketToString(Socket? socket)
        {
            try
            {
                if (socket == null)
                {
                    return "<closed>";
                }
                var s = new System.Text.StringBuilder();
                s.Append("local address = " + LocalAddrToString(GetLocalAddress(socket)));
                s.Append("\nremote address = " + RemoteAddrToString(GetRemoteAddress(socket)));
                return s.ToString();
            }
            catch (ObjectDisposedException)
            {
                return "<closed>";
            }
        }

        public static string AddrToString(EndPoint addr) => EndpointAddressToString(addr) + ":" + EndpointPort(addr);

        public static string LocalAddrToString(EndPoint endpoint)
        {
            if (endpoint == null)
            {
                return "<not bound>";
            }
            return EndpointAddressToString(endpoint) + ":" + EndpointPort(endpoint);
        }

        public static string RemoteAddrToString(EndPoint? endpoint)
        {
            if (endpoint == null)
            {
                return "<not connected>";
            }
            return EndpointAddressToString(endpoint) + ":" + EndpointPort(endpoint);
        }

        public static EndPoint GetLocalAddress(Socket socket)
        {
            try
            {
                return socket.LocalEndPoint;
            }
            catch (SocketException ex)
            {
                throw new TransportException(ex);
            }
        }

        public static EndPoint? GetRemoteAddress(Socket socket)
        {
            try
            {
                return socket.RemoteEndPoint;
            }
            catch (SocketException)
            {
            }
            return null;
        }

        private static IPAddress? GetInterfaceAddress(string iface, AddressFamily family)
        {
            if (iface.Length == 0)
            {
                return null;
            }

            //
            // The iface parameter must either be an IP address, an
            // index or the name of an interface. If it's an index we
            // just return it. If it's an IP addess we search for an
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

        private static int GetInterfaceIndex(string iface, AddressFamily family)
        {
            if (iface.Length == 0)
            {
                return -1;
            }

            //
            // The iface parameter must either be an IP address, an
            // index or the name of an interface. If it's an index we
            // just return it. If it's an IP addess we search for an
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
                var addr = IPAddress.Parse(iface);
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

        private static bool IsWildcard(string address, out bool ipv4Wildcard)
        {
            ipv4Wildcard = false;
            if (address.Length == 0)
            {
                return true;
            }

            try
            {
                var addr = IPAddress.Parse(address);
                if (addr.Equals(IPAddress.Any))
                {
                    ipv4Wildcard = true;
                    return true;
                }
                return addr.Equals(IPAddress.IPv6Any);
            }
            catch (Exception)
            {
            }

            return false;
        }

        public static List<IPAddress> GetLoopbackAddresses(int ipVersion)
        {
            var addresses = new List<IPAddress>();
            if (ipVersion != EnableIPv4)
            {
                addresses.Add(IPAddress.IPv6Loopback);
            }
            if (ipVersion != EnableIPv6)
            {
                addresses.Add(IPAddress.Loopback);
            }
            return addresses;
        }

        public static string EndpointAddressToString(EndPoint? endpoint)
        {
            if (endpoint != null && endpoint is IPEndPoint ipEndpoint)
            {
                return ipEndpoint.Address.ToString();
            }
            return "";
        }

        internal static ushort EndpointPort(EndPoint? endpoint)
        {
            if (endpoint != null && endpoint is IPEndPoint ipEndpoint)
            {
                return (ushort)ipEndpoint.Port;
            }
            else
            {
                return 0;
            }
        }
    }
}
