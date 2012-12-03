// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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

        private static EndPoint getAddressImpl(string host, int port, int protocol, bool server)
        {
            if(host.Length == 0)
            {
#if SILVERLIGHT
                if(server)
                {
                    if(protocol != EnableIPv4)
                    {
                        return new DnsEndPoint(IPAddress.IPv6Any.ToString(), port);
                    }
                    else
                    {
                        return new DnsEndPoint(IPAddress.Any.ToString(), port);
                    }
                }
                else
                {
                    if(protocol != EnableIPv4)
                    {
                        return new DnsEndPoint(IPAddress.IPv6Loopback.ToString(), port);
                    }
                    else
                    {
                        return new DnsEndPoint(IPAddress.Loopback.ToString(), port);
                    }
                }
#else
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
#endif
            }

#if SILVERLIGHT
            return new DnsEndPoint(host, port);
#else
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

#  if COMPACT
                foreach(IPAddress a in Dns.GetHostEntry(host).AddressList)
#  else
                foreach(IPAddress a in Dns.GetHostAddresses(host))
#  endif
                {
                    if((a.AddressFamily == AddressFamily.InterNetwork && protocol != EnableIPv6) ||
                       (a.AddressFamily == AddressFamily.InterNetworkV6 && protocol != EnableIPv4))
                    {
                        return new IPEndPoint(a, port);
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
            // No InterNetwork/InterNetworkV6 address available.
            //
            Ice.DNSException dns = new Ice.DNSException();
            dns.host = host;
            throw dns;
#endif
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
            // BUGFIX: SocketError.InvalidArgument because shutdown() under OS X returns EINVAL
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
        
        public static void setMcastGroup(Socket socket, IPAddress group, string iface)
        {
            try
            {
                if(group.AddressFamily == AddressFamily.InterNetwork)
                {
                    IPAddress ifaceAddr = IPAddress.Any;
                    if(iface.Length != 0)
                    {
                        ifaceAddr = getInterfaceAddress(iface);
                        if(ifaceAddr == IPAddress.Any)
                        {
                            ifaceAddr = ((IPEndPoint)getAddress(iface, 0, EnableIPv4)).Address;
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

        public static EndPoint getAddress(string host, int port, int protocol)
        {
            return getAddressImpl(host, port, protocol, false);
        }

        public static EndPoint getAddressForServer(string host, int port, int protocol)
        {
            return getAddressImpl(host, port, protocol, true);
        }

        public static List<EndPoint> getAddresses(string host, int port, int protocol)
        {
            return getAddresses(host, port, protocol, true);
        }

        public static List<EndPoint> getAddresses(string host, int port, int protocol, bool blocking)
        {
            List<EndPoint> addresses = new List<EndPoint>();
#if SILVERLIGHT
            if(host.Length == 0)
            {
                if(protocol != EnableIPv4)
                {
                    addresses.Add(new DnsEndPoint(IPAddress.IPv6Loopback.ToString(), port));
                }
                else
                {
                    addresses.Add(new DnsEndPoint(IPAddress.Loopback.ToString(), port));
                }
            }
            else
            {
                addresses.Add(new DnsEndPoint(host, port));
            }
#else
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

#  if COMPACT
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
            }
#endif
            return addresses;
        }

        public static IPAddress[] getLocalAddresses(int protocol)
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
                            if(!IPAddress.IsLoopback(uni.Address))
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
                        if(!IPAddress.IsLoopback(a))
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
#if COMPACT
                    if(!IPAddress.IsLoopback(a))
#else
                    if(!a.IsIPv6LinkLocal)
#endif
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

        public static string fdLocalAddressToString(Socket socket)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder();
#if SILVERLIGHT
            // Silverlight Socket doesn't expose the local endpoint
            s.Append("local address = <not available>");
#else
            IPEndPoint localEndpoint = (IPEndPoint)getLocalAddress(socket);
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
#endif
            return s.ToString();
        }

        public static string
        addressesToString(EndPoint localEndpoint, EndPoint remoteEndpoint)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder();
#if SILVERLIGHT
            DnsEndPoint remoteDnsEndPoint = (DnsEndPoint)remoteEndpoint;
            // Silverlight Socket doesn't expose the local endpoint
            s.Append("local address = <not available>");

            if(remoteDnsEndPoint == null)
            {
                s.Append("\nremote address = <not connected>");
            }
            else
            {
                s.Append("\nremote address = " + remoteDnsEndPoint.Host);
                s.Append(":" + remoteDnsEndPoint.Port);
            }
#else
            IPEndPoint localPEndpoint = (IPEndPoint)localEndpoint;
            IPEndPoint remoteIPEndPoint = (IPEndPoint)remoteEndpoint;
            if(localPEndpoint == null)
            {
                // This might occur if connect was not called yet, see also comments in doBeginConnectAsync
                s.Append("local address = <not bound>");
            }
            else
            {
                s.Append("local address = " + localPEndpoint.Address);
                s.Append(":" + localPEndpoint.Port);
            }

            if(remoteIPEndPoint == null)
            {
                s.Append("\nremote address = <not connected>");
            }
            else
            {
                s.Append("\nremote address = " + remoteIPEndPoint.Address);
                s.Append(":" + remoteIPEndPoint.Port);
            }
#endif
            return s.ToString();
        }
        
        public static string
        addrToString(EndPoint addr)
        {
            return addr.ToString();
        }

        public static EndPoint
        getLocalAddress(Socket socket)
        {
            EndPoint localEndpoint = null;
            // Silverlight socket doesn't exposes a local endpoint
#if !SILVERLIGHT
            try
            {
                localEndpoint = (EndPoint)socket.LocalEndPoint;
            }
            catch(SocketException ex)
            {
                throw new Ice.SocketException(ex);
            }
#endif
            return localEndpoint;
        }

        public static EndPoint
        getRemoteAddress(Socket socket)
        { 
            EndPoint remoteEndpoint = null;
            try
            {
                remoteEndpoint = (EndPoint)socket.RemoteEndPoint;
            }
            catch(SocketException)
            {
            }
            return remoteEndpoint;
        }

        private static int
        getInterfaceIndex(string name)
        {
#if !COMPACT && !SILVERLIGHT && !UNITY
            NetworkInterface[] nics = NetworkInterface.GetAllNetworkInterfaces();
            foreach(NetworkInterface ni in nics)
            {
                if(ni.Name == name)
                {
                    IPInterfaceProperties ipProps = ni.GetIPProperties();
                    try
                    {
                        IPv6InterfaceProperties ipv6Props = ipProps.GetIPv6Properties();
                        if(ipv6Props != null)
                        {
                            return ipv6Props.Index;
                        }
                    }
                    catch(System.NotImplementedException)
                    {
                    }
                }
            }
#endif
            return 0;
        }

        private static IPAddress
        getInterfaceAddress(string name)
        {
#if !COMPACT && !SILVERLIGHT && !UNITY
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
#endif
            return IPAddress.Any;
        }

        public static string
        endpointAddressToString(EndPoint endpoint)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder();
            if(endpoint != null)
            {
#if SILVERLIGHT
                DnsEndPoint dnsEndpoint = (DnsEndPoint) endpoint;
                s.Append(dnsEndpoint.Host);
#else
                IPEndPoint ipEndpoint = (IPEndPoint) endpoint;
                s.Append(ipEndpoint.Address.ToString());
#endif
            }
            return s.ToString();
        }

        public static int
        endpointPort(EndPoint endpoint)
        {
            int port = -1;
            if(endpoint != null)
            {
#if SILVERLIGHT
                DnsEndPoint dnsEndpoint = (DnsEndPoint) endpoint;
                port = dnsEndpoint.Port;
#else
                IPEndPoint ipEndpoint = (IPEndPoint) endpoint;
                port = ipEndpoint.Port;
#endif
            }
            return port;
        }
    }
}
