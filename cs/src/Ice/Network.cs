// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Net;
    using System.Net.NetworkInformation;
    using System.Net.Sockets;
    using System.Runtime.InteropServices;
    using System.Threading;
    using System.Globalization;

    public sealed class Network
    {
        // ProtocolSupport
        public const int EnableIPv4 = 0;
        public const int EnableIPv6 = 1;
        public const int EnableBoth = 2;

        //
        // Magic numbers taken from winsock2.h
        //
        const int WSAEINTR        = 10004;
        const int WSAEFAULT       = 10014;
        const int WSAEINVAL       = 10022;
        const int WSAEMFILE       = 10024;
        const int WSAEWOULDBLOCK  = 10035;
        const int WSAEINPROGRESS  = 10036; // Deprecated in winsock2, but still used by Mono Beta 1
        const int WSAEMSGSIZE     = 10040;
        const int WSAENETDOWN     = 10050;
        const int WSAENETUNREACH  = 10051;
        const int WSAENETRESET    = 10052;
        const int WSAECONNABORTED = 10053;
        const int WSAECONNRESET   = 10054;
        const int WSAENOBUFS      = 10055;
        const int WSAENOTCONN     = 10057;
        const int WSAESHUTDOWN    = 10058;
        const int WSAETIMEDOUT    = 10060;
        const int WSAECONNREFUSED = 10061;
        const int WSAEHOSTUNREACH = 10065;
        const int WSATRY_AGAIN    = 11002;

        private static IPEndPoint getAddressImpl(string host, int port, int protocol, bool server)
        {
            if(host.Length == 0)
            {
                if(server)
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
                else
                {
                    if(protocol != EnableIPv4)
                    {
                        return new IPEndPoint(IPAddress.IPv6Loopback, port);
                    }
                    else
                    {
                        return new IPEndPoint(IPAddress.Loopback, port);
                    }
                }
            }

            int retry = 5;

        repeatGetHostByName:
            try
            {
                try
                {
                    IPAddress addr = IPAddress.Parse(host);
                    if((addr.AddressFamily == AddressFamily.InterNetwork && protocol != EnableIPv6) ||
                       (addr.AddressFamily == AddressFamily.InterNetworkV6 && protocol != EnableIPv4))
                    {
                        return new IPEndPoint(addr, port);
                    }
                }
                catch (FormatException)
                {
                }

                foreach(IPAddress a in Dns.GetHostAddresses(host))
                {
                    if((a.AddressFamily == AddressFamily.InterNetwork && protocol != EnableIPv6) ||
                       (a.AddressFamily == AddressFamily.InterNetworkV6 && protocol != EnableIPv4))
                    {
                        return new IPEndPoint(a, port);
                    }
                }
            }
            catch(Win32Exception ex)
            {
                if(ex.NativeErrorCode == WSATRY_AGAIN && --retry >= 0)
                {
                    goto repeatGetHostByName;
                }
                Ice.DNSException e = new Ice.DNSException(ex);
                e.host = host;
                throw e;
            }
            catch(System.Exception ex)
            {
                Ice.DNSException e = new Ice.DNSException(ex);
                e.host = host;
                throw e;
            }

            //
            // No InterNetwork/InterNetworkV6 address available.
            //
            Ice.DNSException dns = new Ice.DNSException();
            dns.host = host;
            throw dns;
        }

        public static bool interrupted(Win32Exception ex)
        {
            return ex.NativeErrorCode == WSAEINTR;
        }

        public static bool acceptInterrupted(Win32Exception ex)
        {
            if(interrupted(ex))
            {
                return true;
            }
            int error = ex.NativeErrorCode;
            return error == WSAECONNABORTED ||
                   error == WSAECONNRESET ||
                   error == WSAETIMEDOUT;
        }

        public static bool noBuffers(Win32Exception ex)
        {
            int error = ex.NativeErrorCode;
            return error == WSAENOBUFS ||
                   error == WSAEFAULT;
        }

        public static bool wouldBlock(Win32Exception ex)
        {
            return ex.NativeErrorCode == WSAEWOULDBLOCK;
        }

        public static bool connectFailed(Win32Exception ex)
        {
            int error = ex.NativeErrorCode;
            return error == WSAECONNREFUSED ||
                   error == WSAETIMEDOUT ||
                   error == WSAENETUNREACH ||
                   error == WSAEHOSTUNREACH ||
                   error == WSAECONNRESET ||
                   error == WSAESHUTDOWN ||
                   error == WSAECONNABORTED ||
                   error == WSAENETDOWN;
        }

        public static bool connectInProgress(Win32Exception ex)
        {
            int error = ex.NativeErrorCode;
            return error == WSAEWOULDBLOCK ||
                   error == WSAEINPROGRESS;
        }

        public static bool connectionLost(Win32Exception ex)
        {
            int error = ex.NativeErrorCode;
            return error == WSAECONNRESET ||
                   error == WSAESHUTDOWN ||
                   error == WSAECONNABORTED ||
                   error == WSAENETDOWN ||
                   error == WSAENETRESET;
        }

        public static bool connectionLost(System.IO.IOException ex)
        {
            //
            // In some cases the IOException has an inner exception that we can pass directly
            // to the other overloading of connectionLost().
            //
            if(ex.InnerException != null && ex.InnerException is Win32Exception)
            {
                return connectionLost(ex.InnerException as Win32Exception);
            }

            //
            // In other cases the IOException has no inner exception. We could examine the
            // exception's message, but that is fragile due to localization issues. We
            // resort to extracting the value of the protected HResult member via reflection.
            //
            int hr = (int)ex.GetType().GetProperty("HResult",
                System.Reflection.BindingFlags.Instance |
                System.Reflection.BindingFlags.NonPublic).GetValue(ex, null);

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

        public static bool connectionRefused(Win32Exception ex)
        {
            return ex.NativeErrorCode == WSAECONNREFUSED;
        }
        
        public static bool notConnected(Win32Exception ex)
        {
            // BUGFIX: WSAEINVAL because shutdown() under MacOS returns EINVAL if the server side is gone.
            // BUGFIX: shutdown() under Vista might return WSAECONNRESET
            return ex.NativeErrorCode == WSAENOTCONN ||
                   ex.NativeErrorCode == WSAEINVAL ||
                   ex.NativeErrorCode == WSAECONNRESET;
        }

        public static bool recvTruncated(Win32Exception ex)
        {
            return ex.NativeErrorCode == WSAEMSGSIZE;
        }

        public static bool operationAborted(Win32Exception ex)
        {
            return ex.NativeErrorCode == 995;
        }

        public static bool timeout(System.IO.IOException ex)
        {
            //
            // TODO: Instead of testing for an English substring, we need to examine the inner
            // exception (if there is one).
            //
            return ex.Message.IndexOf("period of time", StringComparison.Ordinal) >= 0;
        }

        public static bool noMoreFds(System.Exception ex)
        {
            try
            {
                return ex != null && ((Win32Exception)ex).NativeErrorCode == WSAEMFILE;
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
                    int i = System.Int32.Parse(arr[0], CultureInfo.InvariantCulture);
                    if(i >= 223 && i <= 239)
                    {
                        return true;
                    }
                }
                catch(System.FormatException)
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

            if(!udp)
            {
                try
                {
                    setTcpNoDelay(socket);
                    socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive, 1);
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

        public static void setTcpNoDelay(Socket socket)
        {
            try
            {
                socket.SetSocketOption(SocketOptionLevel.Tcp, SocketOptionName.NoDelay, 1);
            }
            catch(System.Exception ex)
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
            catch(System.Exception ex)
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

        public static void setMcastGroup(Socket socket, IPAddress group, string iface)
        {
            try
            {
                if(group.AddressFamily == AddressFamily.InterNetwork)
                {
                    IPAddress ifaceAddr = IPAddress.Any;
                    if(iface.Length != 0)
                    {
                        ifaceAddr = Network.getInterfaceAddress(iface);
                        if(ifaceAddr == IPAddress.Any)
                        {
                            ifaceAddr = Network.getAddress(iface, 0, Network.EnableIPv4).Address;
                        }
                    }
                    socket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.AddMembership, 
                                           new MulticastOption(group, ifaceAddr));
                }
                else
                {
                    int ifaceIndex = 0;
                    if(iface.Length != 0)
                    {
                        ifaceIndex = getInterfaceIndex(iface);
                        if(ifaceIndex == 0)
                        {
                            try
                            {
                                ifaceIndex = System.Int32.Parse(iface, CultureInfo.InvariantCulture);
                            }
                            catch(System.FormatException ex)
                            {
                                closeSocketNoThrow(socket);
                                throw new Ice.SocketException(ex);
                            }
                        }
                    }
                    socket.SetSocketOption(SocketOptionLevel.IPv6, SocketOptionName.AddMembership, 
                                           new IPv6MulticastOption(group, ifaceIndex));
                }
            }
            catch(SocketException ex)
            {
                closeSocketNoThrow(socket);
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
        
        public static IPEndPoint doBind(Socket socket, EndPoint addr)
        {
            try
            {
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

        public static bool doConnect(Socket fd, EndPoint addr)
        {
        repeatConnect:
            try
            {
                //
                // Even though we are on the client side, the call to Bind()
                // is necessary to work around a .NET bug: if a socket is
                // connected non-blocking, the LocalEndPoint and RemoteEndPoint
                // properties are null. The call to Bind() fixes this.
                //
                IPAddress any = fd.AddressFamily == AddressFamily.InterNetworkV6 ? IPAddress.IPv6Any : IPAddress.Any;
                fd.Bind(new IPEndPoint(any, 0));
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

            if(AssemblyUtil.platform_ == AssemblyUtil.Platform.NonWindows)
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

        public static IAsyncResult doConnectAsync(Socket fd, EndPoint addr, AsyncCallback callback, object state)
        {
            //
            // NOTE: It's the caller's responsability to close the socket upon
            // failure to connect. The socket isn't closed by this method.
            //

        repeatConnect:
            try
            {
                //
                // Even though we are on the client side, the call to Bind()
                // is necessary to work around a .NET bug: if a socket is
                // connected non-blocking, the LocalEndPoint and RemoteEndPoint
                // properties are null. The call to Bind() fixes this.
                //
                IPAddress any = fd.AddressFamily == AddressFamily.InterNetworkV6 ? IPAddress.IPv6Any : IPAddress.Any;
                fd.Bind(new IPEndPoint(any, 0));
                return fd.BeginConnect(addr, callback, state);
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

            if(AssemblyUtil.platform_ == AssemblyUtil.Platform.NonWindows)
            {
                //
                // Prevent self connect (self connect happens on Linux when a client tries to connect to
                // a server which was just deactivated if the client socket re-uses the same ephemeral
                // port as the server).
                //
                EndPoint remoteAddr = getRemoteAddress(fd);
                if(remoteAddr != null && remoteAddr.Equals(getLocalAddress(fd)))
                {
                    throw new Ice.ConnectionRefusedException();
                }
            }
        }

        public static IPEndPoint getAddress(string host, int port, int protocol)
        {
            return getAddressImpl(host, port, protocol, false);
        }
        
        public static IPEndPoint getAddressForServer(string host, int port, int protocol)
        {
            return getAddressImpl(host, port, protocol, true);
        }

        public static int compareAddress(IPEndPoint addr1, IPEndPoint addr2)
        {
            if(addr1.Port < addr2.Port)
            {
                return -1;
            }
            else if(addr2.Port < addr1.Port)
            {
                return 1;
            }

            byte[] larr = addr1.Address.GetAddressBytes();
            byte[] rarr = addr2.Address.GetAddressBytes();

            if(larr.Length < rarr.Length)
            {
                return -1;
            }
            else if(rarr.Length < larr.Length)
            {
                return 1;
            }

            for(int i = 0; i < larr.Length; i++)
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

        public static List<IPEndPoint> getAddresses(string host, int port, int protocol)
        {
            return getAddresses(host, port, protocol, true);
        }

        public static List<IPEndPoint> getAddresses(string host, int port, int protocol, bool blocking)
        {
            List<IPEndPoint> addresses = new List<IPEndPoint>();
            if(host.Length == 0)
            {
                if(protocol != EnableIPv4)
                {
                    addresses.Add(new IPEndPoint(IPAddress.IPv6Loopback, port));
                }

                if(protocol != EnableIPv6)
                {
                    addresses.Add(new IPEndPoint(IPAddress.Loopback, port));
                }
                return addresses;
            }
            else
            {
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
                }
                catch(Win32Exception ex)
                {
                    if(ex.NativeErrorCode == WSATRY_AGAIN && --retry >= 0)
                    {
                        goto repeatGetHostByName;
                    }
                    Ice.DNSException e = new Ice.DNSException(ex);
                    e.host = host;
                    throw e;
                }
                catch(System.Exception ex)
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
            }

            return addresses;
        }

        public static IPAddress[] getLocalAddresses(int protocol)
        {
            ArrayList addresses;

            int retry = 5;

        repeatGetHostByName:
            try
            {
                addresses = new ArrayList();
                if(AssemblyUtil.runtime_ != AssemblyUtil.Runtime.Mono)
                {
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
                                if(!IPAddress.IsLoopback(uni.Address))
                                {
                                    addresses.Add(uni.Address);
                                }
                            }
                        }
                    }
                }
                else
                {
                    foreach(IPAddress a in Dns.GetHostAddresses(Dns.GetHostName()))
                    {
                        if((a.AddressFamily == AddressFamily.InterNetwork && protocol != EnableIPv6) ||
                           (a.AddressFamily == AddressFamily.InterNetworkV6 && protocol != EnableIPv4))
                        {
                            if(!IPAddress.IsLoopback(a))
                            {
                                addresses.Add(a);
                            }
                        }
                    }
                }
            }
            catch(Win32Exception ex)
            {
                if(ex.NativeErrorCode == WSATRY_AGAIN && --retry >= 0)
                {
                    goto repeatGetHostByName;
                }
                Ice.DNSException e = new Ice.DNSException(ex);
                e.host = "0.0.0.0";
                throw e;
            }
            catch(System.Exception ex)
            {
                Ice.DNSException e = new Ice.DNSException(ex);
                e.host = "0.0.0.0";
                throw e;
            }

            return (IPAddress[])addresses.ToArray(typeof(IPAddress));
        }

        public static void
        setTcpBufSize(Socket socket, Ice.Properties properties, Ice.Logger logger)
        {
            //
            // By default, on Windows we use a 128KB buffer size. On Unix
            // platforms, we use the system defaults.
            //
            int dfltBufSize = 0;
            if(AssemblyUtil.platform_ == AssemblyUtil.Platform.Windows)
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
                    logger.warning("TCP receive buffer size: requested size of " + sizeRequested + " adjusted to " +
                                   size);
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

        public static List<string> getHostsForEndpointExpand(string host, int protocol, bool includeLoopback)
        {
            bool wildcard = host.Length == 0;
            if(!wildcard)
            {
                try
                {
                    IPAddress addr = IPAddress.Parse(host);
                    wildcard = addr.Equals(IPAddress.Any) || addr.Equals(IPAddress.IPv6Any);
                }
                catch(Exception)
                {
                }
            }

            List<string> hosts = new List<string>();
            if(wildcard)
            {
                IPAddress[] addrs = getLocalAddresses(protocol);
                foreach(IPAddress a in addrs)
                {
                    if(!a.IsIPv6LinkLocal)
                    {
                        hosts.Add(a.ToString());
                    }
                }
                
                if(includeLoopback || hosts.Count == 0)
                {
                    if(protocol != EnableIPv6)
                    {
                        hosts.Add("127.0.0.1");
                    }
                    if(protocol != EnableIPv4)
                    {
                        hosts.Add("0:0:0:0:0:0:0:1");
                    }
                }
            }
            return hosts;
        }
        
        public static string fdToString(Socket socket)
        {
            try
            {
                if(socket == null)
                {
                    return "<closed>";
                }
                return addressesToString(getLocalAddress(socket), getRemoteAddress(socket));
            }
            catch(ObjectDisposedException)
            {
                return "<closed>";
            }
        }

        public static string
        addressesToString(IPEndPoint localEndpoint, IPEndPoint remoteEndpoint)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder();
            if(localEndpoint == null)
            {
                // This might occur if connect was not called yet, see also comments in doBeginConnectAsync
                s.Append("local address = <not bound>");
            }
            else
            {
                s.Append("local address = " + localEndpoint.Address);
                s.Append(":" + localEndpoint.Port);
            }

            if(remoteEndpoint == null)
            {
                s.Append("\nremote address = <not connected>");
            }
            else
            {
                s.Append("\nremote address = " + remoteEndpoint.Address);
                s.Append(":" + remoteEndpoint.Port);
            }

            return s.ToString();
        }
        
        public static string
        addrToString(EndPoint addr)
        {
            return addr.ToString();
        }

        public static IPEndPoint
        getLocalAddress(Socket socket)
        { 
            IPEndPoint localEndpoint;
            try
            {
                localEndpoint = (IPEndPoint)socket.LocalEndPoint;
            }
            catch(SocketException ex)
            {
                throw new Ice.SocketException(ex);
            }
            return localEndpoint;
        }

        public static IPEndPoint
        getRemoteAddress(Socket socket)
        { 
            IPEndPoint remoteEndpoint = null;
            try
            {
                remoteEndpoint = (IPEndPoint)socket.RemoteEndPoint;
            }
            catch(SocketException)
            {
            }
            return remoteEndpoint;
        }

        private static int
        getInterfaceIndex(string name)
        {
            NetworkInterface[] nics = NetworkInterface.GetAllNetworkInterfaces();
            foreach(NetworkInterface ni in nics)
            {
                if(ni.Name == name)
                {
                    IPInterfaceProperties ipProps = ni.GetIPProperties();
                    IPv6InterfaceProperties ipv6Props = ipProps.GetIPv6Properties();
                    if(ipv6Props != null)
                    {
                        return ipv6Props.Index;
                    }
                }
            }
            return 0;
        }

        private static IPAddress
        getInterfaceAddress(string name)
        {
            NetworkInterface[] nics = NetworkInterface.GetAllNetworkInterfaces();
            foreach(NetworkInterface ni in nics)
            {
                if(ni.Name == name)
                {
                    IPInterfaceProperties ipProps = ni.GetIPProperties();
                    UnicastIPAddressInformationCollection uniColl = ipProps.UnicastAddresses;
                    foreach(UnicastIPAddressInformation uni in uniColl)
                    {
                        if(uni.Address.AddressFamily == AddressFamily.InterNetwork)
                        {
                            return uni.Address;
                        }
                    }
                }
            }
            return IPAddress.Any;
        }
    }
}
