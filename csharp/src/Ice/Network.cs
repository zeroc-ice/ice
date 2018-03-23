// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
#if !COMPACT && !UNITY
    using System.Net.NetworkInformation;
#endif
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

#if COMPACT
        public static SocketError socketErrorCode(SocketException ex)
        {
            return (SocketError)ex.ErrorCode;
        }
#else
        public static SocketError socketErrorCode(SocketException ex)
        {
            return ex.SocketErrorCode;
        }
#endif

#if COMPACT
        //
        // SocketError enumeration isn't available with Silverlight
        //
        public enum SocketError
        {
            Interrupted = 10004,                    // A blocking Socket call was canceled.
            //AccessDenied =10013,                  // An attempt was made to access a Socket in a way that is forbidden by its access permissions.
            Fault = 10014,                          // An invalid pointer address was detected by the underlying socket provider.
            InvalidArgument = 10022,                // An invalid argument was supplied to a Socket member.
            TooManyOpenSockets = 10024,             // There are too many open sockets in the underlying socket provider.
            WouldBlock = 10035,                     // An operation on a nonblocking socket cannot be completed immediately.
            InProgress = 10036,                     // A blocking operation is in progress.
            //AlreadyInProgress = 10037,            // The nonblocking Socket already has an operation in progress.
            //NotSocket = 10038,                    // A Socket operation was attempted on a non-socket.
            //DestinationAddressRequired = 10039,   // A required address was omitted from an operation on a Socket.
            MessageSize = 10040,                    // The datagram is too long.
            //ProtocolType = 10041,                 // The protocol type is incorrect for this Socket.
            //ProtocolOption = 10042,               // An unknown, invalid, or unsupported option or level was used with a Socket.
            //ProtocolNotSupported = 10043,         // The protocol is not implemented or has not been configured.
            //SocketNotSupported = 10044,           // The support for the specified socket type does not exist in this address family.
            //OperationNotSupported = 10045,        // The address family is not supported by the protocol family.
            //ProtocolFamilyNotSupported = 10046,   // The protocol family is not implemented or has not been configured.
            //AddressFamilyNotSupported = 10047,    // The address family specified is not supported.
            //AddressAlreadyInUse = 10048,          // Only one use of an address is normally permitted.
            //AddressNotAvailable = 10049,          // The selected IP address is not valid in this context.
            NetworkDown = 10050,                    // The network is not available.
            NetworkUnreachable = 10051,             // No route to the remote host exists.
            NetworkReset = 10052,                   // The application tried to set KeepAlive on a connection that has already timed out.
            ConnectionAborted = 10053,              // The connection was aborted by the .NET Framework or the underlying socket provider.
            ConnectionReset = 10054,                // The connection was reset by the remote peer.
            NoBufferSpaceAvailable = 10055,         // No free buffer space is available for a Socket operation.
            //IsConnected = 10056,                  // The Socket is already connected.
            NotConnected = 10057,                   // The application tried to send or receive data, and the Socket is not connected.
            Shutdown = 10058,                       // A request to send or receive data was disallowed because the Socket has already been closed.
            TimedOut = 10060,                       // The connection attempt timed out, or the connected host has failed to respond.
            ConnectionRefused = 10061,              // The remote host is actively refusing a connection.
            //HostDown = 10064,                     // The operation failed because the remote host is down.
            HostUnreachable = 10065,                // There is no network route to the specified host.
            //ProcessLimit = 10067,                 // Too many processes are using the underlying socket provider.
            //SystemNotReady = 10091,               // The network subsystem is unavailable.
            //VersionNotSupported = 10092,          // The version of the underlying socket provider is out of range.
            //NotInitialized = 10093,               // The underlying socket provider has not been initialized.
            //Disconnecting = 10101,                // A graceful shutdown is in progress.
            //TypeNotFound = 10109,                 // The specified class was not found.
            //HostNotFound = 11001,                 // No such host is known. The name is not an official host name or alias.
            TryAgain = 11002,                       // The name of the host could not be resolved. Try again later.
            //NoRecovery = 11003,                   // The error is unrecoverable or the requested database cannot be located.
            //NoData = 11004,                       // The requested name or IP address was not found on the name server.
            //IOPending = 997,                      // The application has initiated an overlapped operation that cannot be completed immediately.
            OperationAborted =995                   // The overlapped operation was aborted due to the closure of the Socket.
        }
#endif

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

#if !UNITY
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
#endif

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

        public static bool noMoreFds(System.Exception ex)
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
#if COMPACT
            string ip = addr.Address.ToString().ToUpper();
#else
            string ip = addr.Address.ToString().ToUpperInvariant();
#endif
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
#if !SILVERLIGHT
                    socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive, 1);
                    //
                    // FIX: the fast path loopback appears to cause issues with
                    // connection closure when it's enabled. Sometime, a peer
                    // doesn't receive the TCP/IP connection closure (RST) from
                    // the other peer and it ends up hanging. See bug #6093.
                    //
                    //setTcpLoopbackFastPath(socket);
#endif
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
#  if !COMPACT && !UNITY && !__MonoCS__ && !SILVERLIGHT && !DOTNET3_5
            //
            // The IPv6Only enumerator was added in .NET 4.
            //
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
#endif
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
#if SILVERLIGHT
                socket.NoDelay = true;
#else
                socket.SetSocketOption(SocketOptionLevel.Tcp, SocketOptionName.NoDelay, 1);
#endif
            }
            catch(System.Exception ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }

#if !SILVERLIGHT
        //
        // FIX: the fast path loopback appears to cause issues with
        // connection closure when it's enabled. Sometime, a peer
        // doesn't receive the TCP/IP connection closure (RST) from
        // the other peer and it ends up hanging. See bug #6093.
        //
        // public static void setTcpLoopbackFastPath(Socket socket)
        // {
        //     const int SIO_LOOPBACK_FAST_PATH = (-1744830448);
        //     Byte[] OptionInValue = BitConverter.GetBytes(1);
        //     try
        //     {
        //         socket.IOControl(SIO_LOOPBACK_FAST_PATH, OptionInValue, null);
        //     }
        //     catch(System.Exception)
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
            catch(System.Exception ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }
#endif

        public static void setSendBufferSize(Socket socket, int sz)
        {
            try
            {
#if SILVERLIGHT
                socket.SendBufferSize = sz;
#else
                socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer, sz);
#endif
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
#if SILVERLIGHT
                sz = socket.SendBufferSize;
#else
                sz = (int)socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer);
#endif
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
#if SILVERLIGHT
                socket.ReceiveBufferSize = sz;
#else
                socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer, sz);
#endif
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
#if SILVERLIGHT
                sz = socket.ReceiveBufferSize;
#else
                sz = (int)socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer);
#endif
            }
            catch(SocketException ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
            return sz;
        }

#if !SILVERLIGHT
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
                int ifaceIndex = getInterfaceIndex(iface, family);
                if(ifaceIndex == -1)
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

                if(family == AddressFamily.InterNetwork)
                {
                    ifaceIndex = (int)IPAddress.HostToNetworkOrder(ifaceIndex);
                    socket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.MulticastInterface, ifaceIndex);
                }
                else
                {
                    socket.SetSocketOption(SocketOptionLevel.IPv6, SocketOptionName.MulticastInterface, ifaceIndex);
                }
            }
            catch(SocketException ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }

        public static void setMcastGroup(Socket s, IPAddress group, string iface)
        {
            try
            {
                int index = getInterfaceIndex(iface, group.AddressFamily);
                if(group.AddressFamily == AddressFamily.InterNetwork)
                {
                    MulticastOption option;
#if COMPACT
                    option = new MulticastOption(group);
#else
                    if(index == -1)
                    {
                        option = new MulticastOption(group);
                    }

                    else
                    {
                        option = new MulticastOption(group, index);
                    }
#endif
                    s.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.AddMembership, option);
                }
                else
                {
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
            catch(Exception ex)
            {
                closeSocketNoThrow(s);
                throw new Ice.SocketException(ex);
            }
        }
#endif

        public static void setMcastTtl(Socket socket, int ttl, AddressFamily family)
        {
            try
            {
#if SILVERLIGHT
                socket.Ttl = (short)ttl;
#else
                if(family == AddressFamily.InterNetwork)
                {
                    socket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.MulticastTimeToLive, ttl);
                }
                else
                {
                    socket.SetSocketOption(SocketOptionLevel.IPv6, SocketOptionName.MulticastTimeToLive, ttl);
                }
#endif
            }
            catch(SocketException ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }

#if !SILVERLIGHT
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
#endif

#if !SILVERLIGHT
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
#endif

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
#if !SILVERLIGHT
                    if(!blocking)
                    {
                        return addresses;
                    }
#endif
                }

#if SILVERLIGHT
                if(protocol != EnableIPv6)
                {
                    addresses.Add(new DnsEndPoint(host, port, AddressFamily.InterNetwork));
                }
                if(protocol != EnableIPv4)
                {
                    addresses.Add(new DnsEndPoint(host, port, AddressFamily.InterNetworkV6));
                }
#else
# if COMPACT
                foreach(IPAddress a in Dns.GetHostEntry(host).AddressList)
#  else
                foreach(IPAddress a in Dns.GetHostAddresses(host))
#  endif
                {
                    if((a.AddressFamily == AddressFamily.InterNetwork && protocol != EnableIPv6) ||
                       (a.AddressFamily == AddressFamily.InterNetworkV6 && protocol != EnableIPv4))
                    {
                        addresses.Add(new IPEndPoint(a, port));
                    }
                }
#endif

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
            return addresses;
        }

        public static IPAddress[] getLocalAddresses(int protocol, bool includeLoopback)
        {
#if SILVERLIGHT
            return new List<IPAddress>().ToArray();
#else
            List<IPAddress> addresses;
            int retry = 5;

            repeatGetHostByName:
            try
            {
                addresses = new List<IPAddress>();
#  if !COMPACT && !UNITY
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
                            if(includeLoopback || !IPAddress.IsLoopback(uni.Address))
                            {
                                addresses.Add(uni.Address);
                            }
                        }
                    }
                }
#  else
#     if COMPACT
                foreach(IPAddress a in Dns.GetHostEntry(Dns.GetHostName()).AddressList)
#     else
                foreach(IPAddress a in Dns.GetHostAddresses(Dns.GetHostName()))
#     endif
                {
                    if((a.AddressFamily == AddressFamily.InterNetwork && protocol != EnableIPv6) ||
                       (a.AddressFamily == AddressFamily.InterNetworkV6 && protocol != EnableIPv4))
                    {
                        if(includeLoopback || !IPAddress.IsLoopback(a))
                        {
                            addresses.Add(a);
                        }
                    }
                }
#  endif
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
            catch(System.Exception ex)
            {
                Ice.DNSException e = new Ice.DNSException(ex);
                e.host = "0.0.0.0";
                throw e;
            }

            return addresses.ToArray();
#endif
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
                Byte[] bytes = addr.GetAddressBytes();
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
            if(AssemblyUtil.platform_ == AssemblyUtil.Platform.Windows)
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
            bool wildcard = host.Length == 0;
            bool ipv4Wildcard = false;
            if(!wildcard)
            {
                try
                {
                    IPAddress addr = IPAddress.Parse(host);
                    ipv4Wildcard = addr.Equals(IPAddress.Any);
                    wildcard = ipv4Wildcard || addr.Equals(IPAddress.IPv6Any);
                }
                catch(Exception)
                {
                }
            }

            List<string> hosts = new List<string>();
            if(wildcard)
            {
                IPAddress[] addrs =
                    getLocalAddresses(ipv4Wildcard ? Network.EnableIPv4 : protocol, includeLoopback);
                foreach(IPAddress a in addrs)
                {
#if COMPACT
                    if(!IPAddress.IsLoopback(a))
#else
                    if(!isLinklocal(a))
#endif
                    {
                        hosts.Add(a.ToString());
                    }
                }
            }
            return hosts;
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
#if SILVERLIGHT
                return "<not available>";
#else
                return "<not bound>";
#endif
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
            // Silverlight socket doesn't exposes a local endpoint
#if !SILVERLIGHT
            try
            {
                return socket.LocalEndPoint;
            }
            catch(SocketException ex)
            {
                throw new Ice.SocketException(ex);
            }
#else
            return null;
#endif
        }

        public static EndPoint
        getRemoteAddress(Socket socket)
        {
            try
            {
                return (EndPoint)socket.RemoteEndPoint;
            }
            catch(SocketException)
            {
            }
            return null;
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
                return System.Int32.Parse(iface, CultureInfo.InvariantCulture);
            }
            catch(System.FormatException)
            {
            }

#if !COMPACT && !SILVERLIGHT && !UNITY
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
#endif
            return -1;
        }

        public static EndPoint
        getNumericAddress(string sourceAddress)
        {
            EndPoint addr = null;
            if(!String.IsNullOrEmpty(sourceAddress))
            {
                List<EndPoint> addrs = getAddresses(sourceAddress, 0, Network.EnableBoth,
                                                    Ice.EndpointSelectionType.Ordered, false, false);
                if(addrs.Count != 0)
                {
                    return addrs[0];
                }
            }
            return addr;
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
#if SILVERLIGHT
                if(endpoint is DnsEndPoint)
                {
                    DnsEndPoint dnsEndpoint = (DnsEndPoint)endpoint;
                    return dnsEndpoint.Host;
                }
#endif
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
#if SILVERLIGHT
                if(endpoint is DnsEndPoint)
                {
                    DnsEndPoint dnsEndpoint = (DnsEndPoint)endpoint;
                    return dnsEndpoint.Port;
                }
#endif
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
