//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System;
    using System.Collections.Generic;
    using System.Net;
    using System.Net.NetworkInformation;
    using System.Net.Sockets;
    using System.Globalization;
    using System.Runtime.InteropServices;

    public sealed class Network
    {
        // ProtocolSupport
        public const int EnableIPv4 = 0;
        public const int EnableIPv6 = 1;
        public const int EnableBoth = 2;

        public static SocketError socketErrorCode(SocketException ex)
        {
            return ex.SocketErrorCode;
        }

        public static bool interrupted(SocketException ex)
        {
            return socketErrorCode(ex) == SocketError.Interrupted;
        }

        public static bool acceptInterrupted(SocketException ex)
        {
            if(interrupted(ex))
            {
                return true;
            }
            SocketError error = socketErrorCode(ex);
            return error == SocketError.ConnectionAborted ||
                   error == SocketError.ConnectionReset ||
                   error == SocketError.TimedOut;
        }

        public static bool noBuffers(SocketException ex)
        {
            SocketError error = socketErrorCode(ex);
            return error == SocketError.NoBufferSpaceAvailable ||
                   error == SocketError.Fault;
        }

        public static bool wouldBlock(SocketException ex)
        {
            return socketErrorCode(ex) == SocketError.WouldBlock;
        }

        public static bool connectFailed(SocketException ex)
        {
            SocketError error = socketErrorCode(ex);
            return error == SocketError.ConnectionRefused ||
                   error == SocketError.TimedOut ||
                   error == SocketError.NetworkUnreachable ||
                   error == SocketError.HostUnreachable ||
                   error == SocketError.ConnectionReset ||
                   error == SocketError.Shutdown ||
                   error == SocketError.ConnectionAborted ||
                   error == SocketError.NetworkDown;
        }

        public static bool connectInProgress(SocketException ex)
        {
            SocketError error = socketErrorCode(ex);
            return error == SocketError.WouldBlock ||
                   error == SocketError.InProgress;
        }

        public static bool connectionLost(SocketException ex)
        {
            SocketError error = socketErrorCode(ex);
            return error == SocketError.ConnectionReset ||
                   error == SocketError.Shutdown ||
                   error == SocketError.ConnectionAborted ||
                   error == SocketError.NetworkDown ||
                   error == SocketError.NetworkReset;
        }

        public static bool connectionLost(System.IO.IOException ex)
        {
            //
            // In some cases the IOException has an inner exception that we can pass directly
            // to the other overloading of connectionLost().
            //
            if(ex.InnerException != null && ex.InnerException is SocketException)
            {
                return connectionLost(ex.InnerException as SocketException);
            }

            //
            // In other cases the IOException has no inner exception. We could examine the
            // exception's message, but that is fragile due to localization issues. We
            // resort to extracting the value of the protected HResult member via reflection.
            //
            int hr = (int)ex.GetType().GetProperty("HResult",
                System.Reflection.BindingFlags.Instance |
                System.Reflection.BindingFlags.NonPublic |
                System.Reflection.BindingFlags.Public).GetValue(ex, null);

            //
            // This value corresponds to the following errors:
            //
            // "Authentication failed because the remote party has closed the transport stream"
            //
            if(hr == -2146232800)
            {
                return true;
            }
            return false;
        }

        public static bool connectionRefused(SocketException ex)
        {
            return socketErrorCode(ex) == SocketError.ConnectionRefused;
        }

        public static bool notConnected(SocketException ex)
        {
            // BUGFIX: SocketError.InvalidArgument because shutdown() under macOS returns EINVAL
            // if the server side is gone.
            // BUGFIX: shutdown() under Vista might return SocketError.ConnectionReset
            SocketError error = socketErrorCode(ex);
            return error == SocketError.NotConnected ||
                   error == SocketError.InvalidArgument ||
                   error == SocketError.ConnectionReset;
        }

        public static bool recvTruncated(SocketException ex)
        {
            return socketErrorCode(ex) == SocketError.MessageSize;
        }

        public static bool operationAborted(SocketException ex)
        {
            return socketErrorCode(ex) == SocketError.OperationAborted;
        }

        public static bool timeout(System.IO.IOException ex)
        {
            //
            // TODO: Instead of testing for an English substring, we need to examine the inner
            // exception (if there is one).
            //
            return ex.Message.IndexOf("period of time", StringComparison.Ordinal) >= 0;
        }

        public static bool noMoreFds(Exception ex)
        {
            try
            {
                return ex != null && socketErrorCode((SocketException)ex) == SocketError.TooManyOpenSockets;
            }
            catch(InvalidCastException)
            {
                return false;
            }
        }

        public static bool isMulticast(IPEndPoint addr)
        {
            string ip = addr.Address.ToString().ToUpperInvariant();
            if(addr.AddressFamily == AddressFamily.InterNetwork)
            {
                char[] splitChars = { '.' };
                string[] arr = ip.Split(splitChars);
                try
                {
                    int i = int.Parse(arr[0], CultureInfo.InvariantCulture);
                    if(i >= 223 && i <= 239)
                    {
                        return true;
                    }
                }
                catch(FormatException)
                {
                    return false;
                }
            }
            else // AddressFamily.InterNetworkV6
            {
                if(ip.StartsWith("FF", StringComparison.Ordinal))
                {
                    return true;
                }
            }
            return false;
        }

        public static bool isIPv6Supported()
        {
            try
            {
                Socket socket = new Socket(AddressFamily.InterNetworkV6, SocketType.Stream, ProtocolType.Tcp);
                closeSocketNoThrow(socket);
                return true;
            }
            catch(SocketException)
            {
                return false;
            }
        }

        public static Socket createSocket(bool udp, AddressFamily family)
        {
            Socket socket;

            try
            {
                if(udp)
                {
                    socket = new Socket(family, SocketType.Dgram, ProtocolType.Udp);
                }
                else
                {
                    socket = new Socket(family, SocketType.Stream, ProtocolType.Tcp);
                }
            }
            catch(SocketException ex)
            {
                throw new Ice.SocketException(ex);
            }
            catch(ArgumentException ex)
            {
                throw new Ice.SocketException(ex);
            }

            if(!udp)
            {
                try
                {
                    setTcpNoDelay(socket);
                    socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive, 1);
                    //
                    // FIX: the fast path loopback appears to cause issues with
                    // connection closure when it's enabled. Sometime, a peer
                    // doesn't receive the TCP/IP connection closure (RST) from
                    // the other peer and it ends up hanging. See bug #6093.
                    //
                    //setTcpLoopbackFastPath(socket);
                }
                catch(SocketException ex)
                {
                    closeSocketNoThrow(socket);
                    throw new Ice.SocketException(ex);
                }
            }
            return socket;
        }

        public static Socket createServerSocket(bool udp, AddressFamily family, int protocol)
        {
            Socket socket = createSocket(udp, family);
            if(family == AddressFamily.InterNetworkV6 && protocol != EnableIPv4)
            {
                try
                {
                    int flag = protocol == EnableIPv6 ? 1 : 0;
                    socket.SetSocketOption(SocketOptionLevel.IPv6, SocketOptionName.IPv6Only, flag);
                }
                catch(SocketException ex)
                {
                    closeSocketNoThrow(socket);
                    throw new Ice.SocketException(ex);
                }
            }
            return socket;
        }

        public static void closeSocketNoThrow(Socket socket)
        {
            if(socket == null)
            {
                return;
            }
            try
            {
                socket.Close();
            }
            catch(SocketException)
            {
                // Ignore
            }
        }

        public static void closeSocket(Socket socket)
        {
            if(socket == null)
            {
                return;
            }
            try
            {
                socket.Close();
            }
            catch(SocketException ex)
            {
                throw new Ice.SocketException(ex);
            }
        }

        public static void setTcpNoDelay(Socket socket)
        {
            try
            {
                socket.SetSocketOption(SocketOptionLevel.Tcp, SocketOptionName.NoDelay, 1);
            }
            catch(Exception ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }

        //
        // FIX: the fast path loopback appears to cause issues with
        // connection closure when it's enabled. Sometime, a peer
        // doesn't receive the TCP/IP connection closure (RST) from
        // the other peer and it ends up hanging. See bug #6093.
        //
        // public static void setTcpLoopbackFastPath(Socket socket)
        // {
        //     const int SIO_LOOPBACK_FAST_PATH = (-1744830448);
        //     byte[] OptionInValue = BitConverter.GetBytes(1);
        //     try
        //     {
        //         socket.IOControl(SIO_LOOPBACK_FAST_PATH, OptionInValue, null);
        //     }
        //     catch(Exception)
        //     {
        //         // Expected on platforms that do not support TCP Loopback Fast Path
        //     }
        // }

        public static void setBlock(Socket socket, bool block)
        {
            try
            {
                socket.Blocking = block;
            }
            catch(SocketException ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }

        public static void setKeepAlive(Socket socket)
        {
            try
            {
                socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive, 1);
            }
            catch(Exception ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }

        public static void setSendBufferSize(Socket socket, int sz)
        {
            try
            {
                socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer, sz);
            }
            catch(SocketException ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }

        public static int getSendBufferSize(Socket socket)
        {
            int sz;
            try
            {
                sz = (int)socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer);
            }
            catch(SocketException ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
            return sz;
        }

        public static void setRecvBufferSize(Socket socket, int sz)
        {
            try
            {
                socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer, sz);
            }
            catch(SocketException ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }

        public static int getRecvBufferSize(Socket socket)
        {
            int sz = 0;
            try
            {
                sz = (int)socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer);
            }
            catch(SocketException ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
            return sz;
        }

        public static void setReuseAddress(Socket socket, bool reuse)
        {
            try
            {
                socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, reuse ? 1 : 0);
            }
            catch(SocketException ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }

        public static void setMcastInterface(Socket socket, string iface, AddressFamily family)
        {
            try
            {
                if(family == AddressFamily.InterNetwork)
                {
                    socket.SetSocketOption(SocketOptionLevel.IP,
                                           SocketOptionName.MulticastInterface,
                                           getInterfaceAddress(iface, family).GetAddressBytes());
                }
                else
                {
                    socket.SetSocketOption(SocketOptionLevel.IPv6, SocketOptionName.MulticastInterface,
                                           getInterfaceIndex(iface, family));
                }
            }
            catch(Exception ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }

        public static void setMcastGroup(Socket s, IPAddress group, string iface)
        {
            try
            {
                var indexes = new HashSet<int>();
                foreach(string intf in getInterfacesForMulticast(iface, getProtocolSupport(group)))
                {
                    if(group.AddressFamily == AddressFamily.InterNetwork)
                    {
                        MulticastOption option;
                        IPAddress addr = getInterfaceAddress(intf, group.AddressFamily);
                        if(addr == null)
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
                        if(!indexes.Contains(index))
                        {
                            indexes.Add(index);
                            IPv6MulticastOption option;
                            if(index == -1)
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
            catch(Exception ex)
            {
                closeSocketNoThrow(s);
                throw new Ice.SocketException(ex);
            }
        }

        public static void setMcastTtl(Socket socket, int ttl, AddressFamily family)
        {
            try
            {
                if(family == AddressFamily.InterNetwork)
                {
                    socket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.MulticastTimeToLive, ttl);
                }
                else
                {
                    socket.SetSocketOption(SocketOptionLevel.IPv6, SocketOptionName.MulticastTimeToLive, ttl);
                }
            }
            catch(SocketException ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }

#if NETSTANDARD2_0
        [DllImport("libc", SetLastError = true)]
        private static extern int setsockopt(int socket, int level, int name, ref int value, uint len);

        private const int SOL_SOCKET_MACOS= 0xffff;
        private const int SO_REUSEADDR_MACOS = 0x0004;
        private const int SOL_SOCKET_LINUX = 0x0001;
        private const int SO_REUSEADDR_LINUX = 0x0002;
#endif

        public static IPEndPoint doBind(Socket socket, EndPoint addr)
        {
            try
            {
#if NETSTANDARD2_0
                //
                // TODO: Workaround .NET Core 2.0 bug where SO_REUSEADDR isn't set on sockets which are bound. This
                // fix is included in the Bind() implementation of .NET Core 2.1. This workaround should be removed
                // once we no longer support .NET Core 2.0.
                //
                int value = 1;
                int err = 0;
                var fd = socket.Handle.ToInt32();
                if(AssemblyUtil.isLinux)
                {
                    err = setsockopt(fd, SOL_SOCKET_LINUX, SO_REUSEADDR_LINUX, ref value, sizeof(int));
                }
                else if(AssemblyUtil.isMacOS)
                {
                    err = setsockopt(fd, SOL_SOCKET_MACOS, SO_REUSEADDR_MACOS, ref value, sizeof(int));
                }
                if(err != 0)
                {
                    throw new SocketException(err);
                }
#endif
                socket.Bind(addr);
                return (IPEndPoint)socket.LocalEndPoint;
            }
            catch(SocketException ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }

        public static void doListen(Socket socket, int backlog)
        {

        repeatListen:

            try
            {
                socket.Listen(backlog);
            }
            catch(SocketException ex)
            {
                if(interrupted(ex))
                {
                    goto repeatListen;
                }

                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }

        public static bool doConnect(Socket fd, EndPoint addr, EndPoint sourceAddr)
        {
            EndPoint bindAddr = sourceAddr;
            if(bindAddr == null)
            {
                //
                // Even though we are on the client side, the call to Bind()
                // is necessary to work around a .NET bug: if a socket is
                // connected non-blocking, the LocalEndPoint and RemoteEndPoint
                // properties are null. The call to Bind() fixes this.
                //
                IPAddress any = fd.AddressFamily == AddressFamily.InterNetworkV6 ? IPAddress.IPv6Any : IPAddress.Any;
                bindAddr = new IPEndPoint(any, 0);
            }
            doBind(fd, bindAddr);

        repeatConnect:
            try
            {
                IAsyncResult result = fd.BeginConnect(addr, null, null);
                if(!result.CompletedSynchronously)
                {
                    return false;
                }
                fd.EndConnect(result);
            }
            catch(SocketException ex)
            {
                if(interrupted(ex))
                {
                    goto repeatConnect;
                }

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

            //
            // On Windows, we need to set the socket's blocking status again
            // after the asynchronous connect. Seems like a bug in .NET.
            //
            setBlock(fd, fd.Blocking);
            if(!AssemblyUtil.isWindows)
            {
                //
                // Prevent self connect (self connect happens on Linux when a client tries to connect to
                // a server which was just deactivated if the client socket re-uses the same ephemeral
                // port as the server).
                //
                if(addr.Equals(getLocalAddress(fd)))
                {
                    throw new Ice.ConnectionRefusedException();
                }
            }
            return true;
        }

        public static IAsyncResult doConnectAsync(Socket fd, EndPoint addr, EndPoint sourceAddr, AsyncCallback callback,
                                                  object state)
        {
            //
            // NOTE: It's the caller's responsability to close the socket upon
            // failure to connect. The socket isn't closed by this method.
            //
            EndPoint bindAddr = sourceAddr;
            if(bindAddr == null)
            {
                //
                // Even though we are on the client side, the call to Bind()
                // is necessary to work around a .NET bug: if a socket is
                // connected non-blocking, the LocalEndPoint and RemoteEndPoint
                // properties are null. The call to Bind() fixes this.
                //
                IPAddress any = fd.AddressFamily == AddressFamily.InterNetworkV6 ? IPAddress.IPv6Any : IPAddress.Any;
                bindAddr = new IPEndPoint(any, 0);
            }
            fd.Bind(bindAddr);

        repeatConnect:
            try
            {
                return fd.BeginConnect(addr,
                                       delegate(IAsyncResult result)
                                       {
                                           if(!result.CompletedSynchronously)
                                           {
                                               callback(result.AsyncState);
                                           }
                                       }, state);
            }
            catch(SocketException ex)
            {
                if(interrupted(ex))
                {
                    goto repeatConnect;
                }

                if(connectionRefused(ex))
                {
                    throw new Ice.ConnectionRefusedException(ex);
                }
                else
                {
                    throw new Ice.ConnectFailedException(ex);
                }
            }
        }

        public static void doFinishConnectAsync(Socket fd, IAsyncResult result)
        {
            //
            // NOTE: It's the caller's responsability to close the socket upon
            // failure to connect. The socket isn't closed by this method.
            //
            try
            {
                fd.EndConnect(result);
            }
            catch(SocketException ex)
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

            //
            // On Windows, we need to set the socket's blocking status again
            // after the asynchronous connect. Seems like a bug in .NET.
            //
            setBlock(fd, fd.Blocking);
            if(!AssemblyUtil.isWindows)
            {
                //
                // Prevent self connect (self connect happens on Linux when a client tries to connect to
                // a server which was just deactivated if the client socket re-uses the same ephemeral
                // port as the server).
                //
                EndPoint remoteAddr = getRemoteAddress(fd);
                if(remoteAddr.Equals(getLocalAddress(fd)))
                {
                    throw new Ice.ConnectionRefusedException();
                }
            }
        }

        public static int getProtocolSupport(IPAddress addr)
        {
            return addr.AddressFamily == AddressFamily.InterNetwork ? EnableIPv4 : EnableIPv6;
        }

        public static EndPoint getAddressForServer(string host, int port, int protocol, bool preferIPv6)
        {
            if(host.Length == 0)
            {
                if(protocol != EnableIPv4)
                {
                    return new IPEndPoint(IPAddress.IPv6Any, port);
                }
                else
                {
                    return new IPEndPoint(IPAddress.Any, port);
                }
            }
            return getAddresses(host, port, protocol, Ice.EndpointSelectionType.Ordered, preferIPv6, true)[0];
        }

        public static List<EndPoint> getAddresses(string host, int port, int protocol,
                                                  Ice.EndpointSelectionType selType, bool preferIPv6, bool blocking)
        {
            List<EndPoint> addresses = new List<EndPoint>();
            if(host.Length == 0)
            {
                foreach(IPAddress a in getLoopbackAddresses(protocol))
                {
                    addresses.Add(new IPEndPoint(a, port));
                }
                if(protocol == EnableBoth)
                {
                    if(preferIPv6)
                    {
                        IceUtilInternal.Collections.Sort(ref addresses, _preferIPv6Comparator);
                    }
                    else
                    {
                        IceUtilInternal.Collections.Sort(ref addresses, _preferIPv4Comparator);
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
                    if((addr.AddressFamily == AddressFamily.InterNetwork && protocol != EnableIPv6) ||
                       (addr.AddressFamily == AddressFamily.InterNetworkV6 && protocol != EnableIPv4))
                    {
                        addresses.Add(new IPEndPoint(addr, port));
                        return addresses;
                    }
                    else
                    {
                        Ice.DNSException e = new Ice.DNSException();
                        e.host = host;
                        throw e;
                    }
                }
                catch(FormatException)
                {
                    if(!blocking)
                    {
                        return addresses;
                    }
                }

                foreach(IPAddress a in Dns.GetHostAddresses(host))
                {
                    if((a.AddressFamily == AddressFamily.InterNetwork && protocol != EnableIPv6) ||
                       (a.AddressFamily == AddressFamily.InterNetworkV6 && protocol != EnableIPv4))
                    {
                        addresses.Add(new IPEndPoint(a, port));
                    }
                }

                if(selType == Ice.EndpointSelectionType.Random)
                {
                    IceUtilInternal.Collections.Shuffle(ref addresses);
                }

                if(protocol == EnableBoth)
                {
                    if(preferIPv6)
                    {
                        IceUtilInternal.Collections.Sort(ref addresses, _preferIPv6Comparator);
                    }
                    else
                    {
                        IceUtilInternal.Collections.Sort(ref addresses, _preferIPv4Comparator);
                    }
                }
            }
            catch(SocketException ex)
            {
                if(socketErrorCode(ex) == SocketError.TryAgain && --retry >= 0)
                {
                    goto repeatGetHostByName;
                }
                Ice.DNSException e = new Ice.DNSException(ex);
                e.host = host;
                throw e;
            }
            catch(Exception ex)
            {
                Ice.DNSException e = new Ice.DNSException(ex);
                e.host = host;
                throw e;
            }

            //
            // No InterNetwork/InterNetworkV6 available.
            //
            if(addresses.Count == 0)
            {
                Ice.DNSException e = new Ice.DNSException();
                e.host = host;
                throw e;
            }
            return addresses;
        }

        public static IPAddress[] getLocalAddresses(int protocol, bool includeLoopback, bool singleAddressPerInterface)
        {
            List<IPAddress> addresses;
            int retry = 5;

            repeatGetHostByName:
            try
            {
                addresses = new List<IPAddress>();
                NetworkInterface[] nics = NetworkInterface.GetAllNetworkInterfaces();
                foreach(NetworkInterface ni in nics)
                {
                    IPInterfaceProperties ipProps = ni.GetIPProperties();
                    UnicastIPAddressInformationCollection uniColl = ipProps.UnicastAddresses;
                    foreach(UnicastIPAddressInformation uni in uniColl)
                    {
                        if((uni.Address.AddressFamily == AddressFamily.InterNetwork && protocol != EnableIPv6) ||
                           (uni.Address.AddressFamily == AddressFamily.InterNetworkV6 && protocol != EnableIPv4))
                        {
                            if(!addresses.Contains(uni.Address) &&
                               (includeLoopback || !IPAddress.IsLoopback(uni.Address)))
                            {
                                addresses.Add(uni.Address);
                                if(singleAddressPerInterface)
                                {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            catch(SocketException ex)
            {
                if(socketErrorCode(ex) == SocketError.TryAgain && --retry >= 0)
                {
                    goto repeatGetHostByName;
                }
                Ice.DNSException e = new Ice.DNSException(ex);
                e.host = "0.0.0.0";
                throw e;
            }
            catch(Exception ex)
            {
                Ice.DNSException e = new Ice.DNSException(ex);
                e.host = "0.0.0.0";
                throw e;
            }

            return addresses.ToArray();
        }

        public static bool
        isLinklocal(IPAddress addr)
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

        public static void
        setTcpBufSize(Socket socket, ProtocolInstance instance)
        {
            //
            // By default, on Windows we use a 128KB buffer size. On Unix
            // platforms, we use the system defaults.
            //
            int dfltBufSize = 0;
            if(AssemblyUtil.isWindows)
            {
                dfltBufSize = 128 * 1024;
            }
            int rcvSize = instance.properties().getPropertyAsIntWithDefault("Ice.TCP.RcvSize", dfltBufSize);
            int sndSize = instance.properties().getPropertyAsIntWithDefault("Ice.TCP.SndSize", dfltBufSize);
            setTcpBufSize(socket, rcvSize, sndSize, instance);
        }

        public static void
        setTcpBufSize(Socket socket, int rcvSize, int sndSize, ProtocolInstance instance)
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
                if(size < sndSize) // Warn if the size that was set is less than the requested size.
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

        public static List<string> getHostsForEndpointExpand(string host, int protocol, bool includeLoopback)
        {
            List<string> hosts = new List<string>();
            bool ipv4Wildcard = false;
            if(isWildcard(host, out ipv4Wildcard))
            {
                foreach(IPAddress a in getLocalAddresses(ipv4Wildcard ? EnableIPv4 : protocol, includeLoopback, false))
                {
                    if(!isLinklocal(a))
                    {
                        hosts.Add(a.ToString());
                    }
                }
                if(hosts.Count == 0)
                {
                    // Return loopback if only loopback is available no other local addresses are available.
                    foreach(IPAddress a in getLoopbackAddresses(protocol))
                    {
                        hosts.Add(a.ToString());
                    }
                }
            }
            return hosts;
        }

        public static List<string> getInterfacesForMulticast(string intf, int protocol)
        {
            List<string> interfaces = new List<string>();
            bool ipv4Wildcard = false;
            if(isWildcard(intf, out ipv4Wildcard))
            {
                foreach(IPAddress a in getLocalAddresses(ipv4Wildcard ? EnableIPv4 : protocol, true, true))
                {
                    interfaces.Add(a.ToString());
                }
            }
            if(interfaces.Count == 0)
            {
                interfaces.Add(intf);
            }
            return interfaces;
        }

        public static string fdToString(Socket socket, NetworkProxy proxy, EndPoint target)
        {
            try
            {
                if(socket == null)
                {
                    return "<closed>";
                }

                EndPoint remote = getRemoteAddress(socket);

                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("local address = " + localAddrToString(getLocalAddress(socket)));
                if(proxy != null)
                {
                    if(remote == null)
                    {
                        remote = proxy.getAddress();
                    }
                    s.Append("\n" + proxy.getName() + " proxy address = " + remoteAddrToString(remote));
                    s.Append("\nremote address = " + remoteAddrToString(target));
                }
                else
                {
                    if(remote == null)
                    {
                        remote = target;
                    }
                    s.Append("\nremote address = " + remoteAddrToString(remote));
                }
                return s.ToString();
            }
            catch(ObjectDisposedException)
            {
                return "<closed>";
            }
        }

        public static string fdToString(Socket socket)
        {
            try
            {
                if(socket == null)
                {
                    return "<closed>";
                }
                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("local address = " + localAddrToString(getLocalAddress(socket)));
                s.Append("\nremote address = " + remoteAddrToString(getRemoteAddress(socket)));
                return s.ToString();
            }
            catch(ObjectDisposedException)
            {
                return "<closed>";
            }
        }

        public static string fdLocalAddressToString(Socket socket)
        {
            return "local address = " + localAddrToString(getLocalAddress(socket));
        }

        public static string
        addrToString(EndPoint addr)
        {
            return endpointAddressToString(addr) + ":" + endpointPort(addr);
        }

        public static string
        localAddrToString(EndPoint endpoint)
        {
            if(endpoint == null)
            {
                return "<not bound>";
            }
            return endpointAddressToString(endpoint) + ":" + endpointPort(endpoint);
        }

        public static string
        remoteAddrToString(EndPoint endpoint)
        {
            if(endpoint == null)
            {
                return "<not connected>";
            }
            return endpointAddressToString(endpoint) + ":" + endpointPort(endpoint);
        }

        public static EndPoint
        getLocalAddress(Socket socket)
        {
            try
            {
                return socket.LocalEndPoint;
            }
            catch(SocketException ex)
            {
                throw new Ice.SocketException(ex);
            }
        }

        public static EndPoint
        getRemoteAddress(Socket socket)
        {
            try
            {
                return socket.RemoteEndPoint;
            }
            catch(SocketException)
            {
            }
            return null;
        }

        private static IPAddress
        getInterfaceAddress(string iface, AddressFamily family)
        {
            if(iface.Length == 0)
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
            catch(FormatException)
            {
            }

            NetworkInterface[] nics = NetworkInterface.GetAllNetworkInterfaces();
            try
            {
                int index = int.Parse(iface, CultureInfo.InvariantCulture);
                foreach(NetworkInterface ni in nics)
                {
                    IPInterfaceProperties ipProps = ni.GetIPProperties();
                    int interfaceIndex = -1;
                    if(family == AddressFamily.InterNetwork)
                    {
                        IPv4InterfaceProperties ipv4Props = ipProps.GetIPv4Properties();
                        if(ipv4Props != null && ipv4Props.Index == index)
                        {
                            interfaceIndex = ipv4Props.Index;
                        }
                    }
                    else
                    {
                        IPv6InterfaceProperties ipv6Props = ipProps.GetIPv6Properties();
                        if(ipv6Props != null && ipv6Props.Index == index)
                        {
                            interfaceIndex = ipv6Props.Index;
                        }
                    }
                    if(interfaceIndex >= 0)
                    {
                        foreach(UnicastIPAddressInformation a in ipProps.UnicastAddresses)
                        {
                            if(a.Address.AddressFamily == family)
                            {
                                return a.Address;
                            }
                        }
                    }
                }
            }
            catch(FormatException)
            {
            }

            foreach(NetworkInterface ni in nics)
            {
                if(ni.Name == iface)
                {
                    IPInterfaceProperties ipProps = ni.GetIPProperties();
                    foreach(UnicastIPAddressInformation a in ipProps.UnicastAddresses)
                    {
                        if(a.Address.AddressFamily == family)
                        {
                            return a.Address;
                        }
                    }
                }
            }

            throw new ArgumentException("couldn't find interface `" + iface + "'");
        }

        private static int
        getInterfaceIndex(string iface, AddressFamily family)
        {
            if(iface.Length == 0)
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
            catch(FormatException)
            {
            }

            NetworkInterface[] nics = NetworkInterface.GetAllNetworkInterfaces();
            try
            {
                IPAddress addr = IPAddress.Parse(iface);
                foreach(NetworkInterface ni in nics)
                {
                    IPInterfaceProperties ipProps = ni.GetIPProperties();
                    foreach(UnicastIPAddressInformation uni in ipProps.UnicastAddresses)
                    {
                        if(uni.Address.Equals(addr))
                        {
                            if(addr.AddressFamily == AddressFamily.InterNetwork)
                            {
                                IPv4InterfaceProperties ipv4Props = ipProps.GetIPv4Properties();
                                if(ipv4Props != null)
                                {
                                    return ipv4Props.Index;
                                }
                            }
                            else
                            {
                                IPv6InterfaceProperties ipv6Props = ipProps.GetIPv6Properties();
                                if(ipv6Props != null)
                                {
                                    return ipv6Props.Index;
                                }
                            }
                        }
                    }
                }
            }
            catch(FormatException)
            {
            }

            foreach(NetworkInterface ni in nics)
            {
                if(ni.Name == iface)
                {
                    IPInterfaceProperties ipProps = ni.GetIPProperties();
                    if(family == AddressFamily.InterNetwork)
                    {
                        IPv4InterfaceProperties ipv4Props = ipProps.GetIPv4Properties();
                        if(ipv4Props != null)
                        {
                            return ipv4Props.Index;
                        }
                    }
                    else
                    {
                        IPv6InterfaceProperties ipv6Props = ipProps.GetIPv6Properties();
                        if(ipv6Props != null)
                        {
                            return ipv6Props.Index;
                        }
                    }
                }
            }

            throw new ArgumentException("couldn't find interface `" + iface + "'");
        }

        public static EndPoint
        getNumericAddress(string sourceAddress)
        {
            EndPoint addr = null;
            if(!string.IsNullOrEmpty(sourceAddress))
            {
                List<EndPoint> addrs = getAddresses(sourceAddress, 0, EnableBoth, Ice.EndpointSelectionType.Ordered,
                                                    false, false);
                if(addrs.Count != 0)
                {
                    return addrs[0];
                }
            }
            return addr;
        }

        private static bool
        isWildcard(string address, out bool ipv4Wildcard)
        {
            ipv4Wildcard = false;
            if(address.Length == 0)
            {
                return true;
            }

            try
            {
                IPAddress addr = IPAddress.Parse(address);
                if(addr.Equals(IPAddress.Any))
                {
                    ipv4Wildcard = true;
                    return true;
                }
                return addr.Equals(IPAddress.IPv6Any);
            }
            catch(Exception)
            {
            }

            return false;
        }

        public static List<IPAddress> getLoopbackAddresses(int protocol)
        {
            List<IPAddress> addresses = new List<IPAddress>();
            if(protocol != EnableIPv4)
            {
                addresses.Add(IPAddress.IPv6Loopback);
            }
            if(protocol != EnableIPv6)
            {
                addresses.Add(IPAddress.Loopback);
            }
            return addresses;
        }

        public static bool
        addressEquals(EndPoint addr1, EndPoint addr2)
        {
            if(addr1 == null)
            {
                if(addr2 == null)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else if(addr2 == null)
            {
                return false;
            }

            return addr1.Equals(addr2);
        }

        public static string
        endpointAddressToString(EndPoint endpoint)
        {
            if(endpoint != null)
            {
                if(endpoint is IPEndPoint)
                {
                    IPEndPoint ipEndpoint = (IPEndPoint) endpoint;
                    return ipEndpoint.Address.ToString();
                }
            }
            return "";
        }

        public static int
        endpointPort(EndPoint endpoint)
        {
            if(endpoint != null)
            {
                if(endpoint is IPEndPoint)
                {
                    IPEndPoint ipEndpoint = (IPEndPoint) endpoint;
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
                if(lhs.AddressFamily == AddressFamily.InterNetwork &&
                   rhs.AddressFamily == AddressFamily.InterNetworkV6)
                {
                    return _ipv6 ? 1 : -1;
                }
                else if(lhs.AddressFamily == AddressFamily.InterNetworkV6 &&
                        rhs.AddressFamily == AddressFamily.InterNetwork)
                {
                    return _ipv6 ? -1 : 1;
                }
                else
                {
                    return 0;
                }
            }

            private bool _ipv6;
        }

        private readonly static EndPointComparator _preferIPv4Comparator = new EndPointComparator(false);
        private readonly static EndPointComparator _preferIPv6Comparator = new EndPointComparator(true);
    }
}
