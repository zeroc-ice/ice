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
using System.Runtime.InteropServices;

namespace IceInternal
{
    public static class Network
    {
        // Which versions of the Internet Protocol are enabled?
        public const int EnableIPv4 = 0;
        public const int EnableIPv6 = 1;
        public const int EnableBoth = 2;

        public static SocketError SocketErrorCode(SocketException ex) => ex.SocketErrorCode;

        public static bool Interrupted(SocketException ex) => SocketErrorCode(ex) == SocketError.Interrupted;

        public static bool AcceptInterrupted(SocketException ex)
        {
            if (Interrupted(ex))
            {
                return true;
            }
            SocketError error = SocketErrorCode(ex);
            return error == SocketError.ConnectionAborted ||
                   error == SocketError.ConnectionReset ||
                   error == SocketError.TimedOut;
        }

        public static bool NoBuffers(SocketException ex)
        {
            SocketError error = SocketErrorCode(ex);
            return error == SocketError.NoBufferSpaceAvailable ||
                   error == SocketError.Fault;
        }

        public static bool WouldBlock(SocketException ex) => SocketErrorCode(ex) == SocketError.WouldBlock;

        public static bool ConnectFailed(SocketException ex)
        {
            SocketError error = SocketErrorCode(ex);
            return error == SocketError.ConnectionRefused ||
                   error == SocketError.TimedOut ||
                   error == SocketError.NetworkUnreachable ||
                   error == SocketError.HostUnreachable ||
                   error == SocketError.ConnectionReset ||
                   error == SocketError.Shutdown ||
                   error == SocketError.ConnectionAborted ||
                   error == SocketError.NetworkDown;
        }

        public static bool ConnectInProgress(SocketException ex)
        {
            SocketError error = SocketErrorCode(ex);
            return error == SocketError.WouldBlock ||
                   error == SocketError.InProgress;
        }

        public static bool ConnectionLost(SocketException ex)
        {
            SocketError error = SocketErrorCode(ex);
            return error == SocketError.ConnectionReset ||
                   error == SocketError.Shutdown ||
                   error == SocketError.ConnectionAborted ||
                   error == SocketError.NetworkDown ||
                   error == SocketError.NetworkReset;
        }

        public static bool ConnectionLost(System.IO.IOException ex)
        {
            //
            // In some cases the IOException has an inner exception that we can pass directly
            // to the other overloading of connectionLost().
            //
            if (ex.InnerException != null && ex.InnerException is SocketException)
            {
                return ConnectionLost((SocketException)ex.InnerException);
            }

            //
            // In other cases the IOException has no inner exception. We could examine the
            // exception's message, but that is fragile due to localization issues. We
            // resort to extracting the value of the protected HResult member via reflection.
            //
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

        public static bool ConnectionRefused(SocketException ex) => SocketErrorCode(ex) == SocketError.ConnectionRefused;

        public static bool NotConnected(SocketException ex)
        {
            // BUGFIX: SocketError.InvalidArgument because shutdown() under macOS returns EINVAL
            // if the server side is gone.
            // BUGFIX: shutdown() under Vista might return SocketError.ConnectionReset
            SocketError error = SocketErrorCode(ex);
            return error == SocketError.NotConnected ||
                   error == SocketError.InvalidArgument ||
                   error == SocketError.ConnectionReset;
        }

        public static bool RecvTruncated(SocketException ex) => SocketErrorCode(ex) == SocketError.MessageSize;

        public static bool OperationAborted(SocketException ex) => SocketErrorCode(ex) == SocketError.OperationAborted;

        public static bool Timeout(System.IO.IOException ex)
        {
            //
            // TODO: Instead of testing for an English substring, we need to examine the inner
            // exception (if there is one).
            //
            return ex.Message.IndexOf("period of time", StringComparison.Ordinal) >= 0;
        }

        public static bool NoMoreFds(Exception ex)
        {
            try
            {
                return ex != null && SocketErrorCode((SocketException)ex) == SocketError.TooManyOpenSockets;
            }
            catch (InvalidCastException)
            {
                return false;
            }
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
                throw new Ice.TransportException(ex);
            }

            if (!udp)
            {
                try
                {
                    SetTcpNoDelay(socket);
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
                    throw new Ice.TransportException(ex);
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
                    throw new Ice.TransportException(ex);
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

        public static void CloseSocket(Socket socket)
        {
            if (socket == null)
            {
                return;
            }
            try
            {
                socket.Close();
            }
            catch (SocketException ex)
            {
                throw new Ice.TransportException(ex);
            }
        }

        public static void SetTcpNoDelay(Socket socket)
        {
            try
            {
                socket.SetSocketOption(SocketOptionLevel.Tcp, SocketOptionName.NoDelay, 1);
            }
            catch (Exception ex)
            {
                CloseSocketNoThrow(socket);
                throw new Ice.TransportException(ex);
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

        public static void SetBlock(Socket socket, bool block)
        {
            try
            {
                socket.Blocking = block;
            }
            catch (SocketException ex)
            {
                CloseSocketNoThrow(socket);
                throw new Ice.TransportException(ex);
            }
        }

        public static void SetKeepAlive(Socket socket)
        {
            try
            {
                socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive, 1);
            }
            catch (Exception ex)
            {
                CloseSocketNoThrow(socket);
                throw new Ice.TransportException(ex);
            }
        }

        public static void SetSendBufferSize(Socket socket, int sz)
        {
            try
            {
                socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer, sz);
            }
            catch (SocketException ex)
            {
                CloseSocketNoThrow(socket);
                throw new Ice.TransportException(ex);
            }
        }

        public static int GetSendBufferSize(Socket socket)
        {
            int sz;
            try
            {
                sz = (int)socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer);
            }
            catch (SocketException ex)
            {
                CloseSocketNoThrow(socket);
                throw new Ice.TransportException(ex);
            }
            return sz;
        }

        public static void SetRecvBufferSize(Socket socket, int sz)
        {
            try
            {
                socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer, sz);
            }
            catch (SocketException ex)
            {
                CloseSocketNoThrow(socket);
                throw new Ice.TransportException(ex);
            }
        }

        public static int GetRecvBufferSize(Socket socket)
        {
            int sz;
            try
            {
                sz = (int)socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer);
            }
            catch (SocketException ex)
            {
                CloseSocketNoThrow(socket);
                throw new Ice.TransportException(ex);
            }
            return sz;
        }

        public static void SetReuseAddress(Socket socket, bool reuse)
        {
            try
            {
                socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, reuse ? 1 : 0);
            }
            catch (SocketException ex)
            {
                CloseSocketNoThrow(socket);
                throw new Ice.TransportException(ex);
            }
        }

        public static void SetMcastInterface(Socket socket, string iface, AddressFamily family)
        {
            try
            {
                if (family == AddressFamily.InterNetwork)
                {
                    socket.SetSocketOption(SocketOptionLevel.IP,
                                           SocketOptionName.MulticastInterface,
                                           GetInterfaceAddress(iface, family)!.GetAddressBytes());
                }
                else
                {
                    socket.SetSocketOption(SocketOptionLevel.IPv6, SocketOptionName.MulticastInterface,
                                           GetInterfaceIndex(iface, family));
                }
            }
            catch (Exception ex)
            {
                CloseSocketNoThrow(socket);
                throw new Ice.TransportException(ex);
            }
        }

        public static void SetMcastGroup(Socket s, IPAddress group, string iface)
        {
            try
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
            catch (Exception ex)
            {
                CloseSocketNoThrow(s);
                throw new Ice.TransportException(ex);
            }
        }

        public static void SetMcastTtl(Socket socket, int ttl, AddressFamily family)
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
            catch (SocketException ex)
            {
                CloseSocketNoThrow(socket);
                throw new Ice.TransportException(ex);
            }
        }

        [DllImport("libc", SetLastError = true)]
#pragma warning disable IDE1006 // Naming Styles
        private static extern int setsockopt(int socket, int level, int name, ref int value, uint len);
#pragma warning restore IDE1006 // Naming Styles

        public static IPEndPoint DoBind(Socket socket, EndPoint addr)
        {
            try
            {
                socket.Bind(addr);
                return (IPEndPoint)socket.LocalEndPoint;
            }
            catch (SocketException ex)
            {
                CloseSocketNoThrow(socket);
                throw new Ice.TransportException(ex);
            }
        }

        public static void DoListen(Socket socket, int backlog)
        {
        repeatListen:

            try
            {
                socket.Listen(backlog);
            }
            catch (SocketException ex)
            {
                if (Interrupted(ex))
                {
                    goto repeatListen;
                }

                CloseSocketNoThrow(socket);
                throw new Ice.TransportException(ex);
            }
        }

        public static bool DoConnect(Socket fd, EndPoint addr, EndPoint? sourceAddr)
        {
            EndPoint? bindAddr = sourceAddr;
            if (bindAddr == null)
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
            DoBind(fd, bindAddr);

        repeatConnect:
            try
            {
                IAsyncResult result = fd.BeginConnect(addr, null, null);
                if (!result.CompletedSynchronously)
                {
                    return false;
                }
                fd.EndConnect(result);
            }
            catch (SocketException ex)
            {
                if (Interrupted(ex))
                {
                    goto repeatConnect;
                }

                CloseSocketNoThrow(fd);

                if (ConnectionRefused(ex))
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
            SetBlock(fd, fd.Blocking);
            if (!Ice.AssemblyUtil.IsWindows)
            {
                //
                // Prevent self connect (self connect happens on Linux when a client tries to connect to
                // a server which was just deactivated if the client socket re-uses the same ephemeral
                // port as the server).
                //
                if (addr.Equals(GetLocalAddress(fd)))
                {
                    throw new Ice.ConnectionRefusedException();
                }
            }
            return true;
        }

        public static IAsyncResult DoConnectAsync(Socket fd, EndPoint addr, EndPoint sourceAddr, AsyncCallback callback,
                                                  object state)
        {
            //
            // NOTE: It's the caller's responsability to close the socket upon
            // failure to connect. The socket isn't closed by this method.
            //
            EndPoint bindAddr = sourceAddr;
            if (bindAddr == null)
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
                                       result =>
                                       {
                                           if (!result.CompletedSynchronously)
                                           {
                                               Debug.Assert(result.AsyncState != null);
                                               callback(result.AsyncState);
                                           }
                                       }, state);
            }
            catch (SocketException ex)
            {
                if (Interrupted(ex))
                {
                    goto repeatConnect;
                }

                if (ConnectionRefused(ex))
                {
                    throw new Ice.ConnectionRefusedException(ex);
                }
                else
                {
                    throw new Ice.ConnectFailedException(ex);
                }
            }
        }

        public static void DoFinishConnectAsync(Socket fd, IAsyncResult result)
        {
            //
            // NOTE: It's the caller's responsability to close the socket upon
            // failure to connect. The socket isn't closed by this method.
            //
            try
            {
                fd.EndConnect(result);
            }
            catch (SocketException ex)
            {
                if (ConnectionRefused(ex))
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
            SetBlock(fd, fd.Blocking);
            if (!Ice.AssemblyUtil.IsWindows)
            {
                //
                // Prevent self connect (self connect happens on Linux when a client tries to connect to
                // a server which was just deactivated if the client socket re-uses the same ephemeral
                // port as the server).
                //
                EndPoint? remoteAddr = GetRemoteAddress(fd);
                if (remoteAddr!.Equals(GetLocalAddress(fd)))
                {
                    throw new Ice.ConnectionRefusedException();
                }
            }
        }

        public static int GetIPVersion(IPAddress addr) => addr.AddressFamily == AddressFamily.InterNetwork ? EnableIPv4 : EnableIPv6;

        public static IPEndPoint GetAddressForServer(string host, int port, int ipVersion, bool preferIPv6)
        {
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
            return GetAddresses(host, port, ipVersion, Ice.EndpointSelectionType.Ordered, preferIPv6, true)[0];
        }

        public static List<IPEndPoint> GetAddresses(string host, int port, int ipVersion,
                                                  Ice.EndpointSelectionType selType, bool preferIPv6, bool blocking)
        {
            var addresses = new List<IPEndPoint>();
            if (host.Length == 0)
            {
                foreach (IPAddress a in GetLoopbackAddresses(ipVersion))
                {
                    addresses.Add(new IPEndPoint(a, port));
                }

                if (ipVersion == EnableBoth)
                {
                    if (preferIPv6)
                    {
                        addresses = addresses.OrderByDescending(addr => addr.AddressFamily).ToList();
                    }
                    else
                    {
                        addresses = addresses.OrderBy(addr => addr.AddressFamily).ToList();
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
                    var addr = IPAddress.Parse(host);
                    if ((addr.AddressFamily == AddressFamily.InterNetwork && ipVersion != EnableIPv6) ||
                        (addr.AddressFamily == AddressFamily.InterNetworkV6 && ipVersion != EnableIPv4))
                    {
                        addresses.Add(new IPEndPoint(addr, port));
                        return addresses;
                    }
                    else
                    {
                        throw new Ice.DNSException(host);
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
                    if ((a.AddressFamily == AddressFamily.InterNetwork && ipVersion != EnableIPv6) ||
                       (a.AddressFamily == AddressFamily.InterNetworkV6 && ipVersion != EnableIPv4))
                    {
                        addresses.Add(new IPEndPoint(a, port));
                    }
                }

                if (selType == Ice.EndpointSelectionType.Random)
                {
                    var rnd = new Random();
                    addresses = addresses.OrderBy(x => (endpoint: rnd.Next(), i: x)).ToList();
                }

                if (ipVersion == EnableBoth)
                {
                    if (preferIPv6)
                    {
                        addresses.OrderByDescending(addr => addr.AddressFamily).ToList();
                    }
                    else
                    {
                        addresses.OrderBy(addr => addr.AddressFamily).ToList();
                    }
                }
            }
            catch (SocketException ex)
            {
                if (SocketErrorCode(ex) == SocketError.TryAgain && --retry >= 0)
                {
                    goto repeatGetHostByName;
                }
                throw new Ice.DNSException(host, ex);
            }
            catch (Exception ex)
            {
                throw new Ice.DNSException(host, ex);
            }

            //
            // No InterNetwork/InterNetworkV6 available.
            //
            if (addresses.Count == 0)
            {
                throw new Ice.DNSException(host);
            }
            return addresses;
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
                if (SocketErrorCode(ex) == SocketError.TryAgain && --retry >= 0)
                {
                    goto repeatGetHostByName;
                }
                throw new Ice.DNSException("0.0.0.0", ex);
            }
            catch (Exception ex)
            {
                throw new Ice.DNSException("0.0.0.0", ex);
            }

            return addresses.ToArray();
        }

        public static bool
        IsLinklocal(IPAddress addr)
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
        SetTcpBufSize(Socket socket, TransportInstance instance)
        {
            //
            // By default, on Windows we use a 128KB buffer size. On Unix
            // platforms, we use the system defaults.
            //
            int dfltBufSize = 0;
            if (Ice.AssemblyUtil.IsWindows)
            {
                dfltBufSize = 128 * 1024;
            }
            int rcvSize = instance.Communicator.GetPropertyAsInt("Ice.TCP.RcvSize") ?? dfltBufSize;
            int sndSize = instance.Communicator.GetPropertyAsInt("Ice.TCP.SndSize") ?? dfltBufSize;
            SetTcpBufSize(socket, rcvSize, sndSize, instance);
        }

        public static void
        SetTcpBufSize(Socket socket, int rcvSize, int sndSize, TransportInstance instance)
        {
            if (rcvSize > 0)
            {
                //
                // Try to set the buffer size. The kernel will silently adjust
                // the size to an acceptable value. Then read the size back to
                // get the size that was actually set.
                //
                SetRecvBufferSize(socket, rcvSize);
                int size = GetRecvBufferSize(socket);
                if (size < rcvSize)
                {
                    // Warn if the size that was set is less than the requested size and
                    // we have not already warned.
                    Ice.BufSizeWarnInfo winfo = instance.GetBufSizeWarn(Ice.EndpointType.TCP);
                    if (!winfo.RcvWarn || rcvSize != winfo.RcvSize)
                    {
                        instance.Logger.Warning(
                            $"TCP receive buffer size: requested size of {rcvSize} adjusted to {size}");
                        instance.SetRcvBufSizeWarn(Ice.EndpointType.TCP, rcvSize);
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
                SetSendBufferSize(socket, sndSize);
                int size = GetSendBufferSize(socket);
                if (size < sndSize) // Warn if the size that was set is less than the requested size.
                {
                    // Warn if the size that was set is less than the requested size and
                    // we have not already warned.
                    Ice.BufSizeWarnInfo winfo = instance.GetBufSizeWarn(Ice.EndpointType.TCP);
                    if (!winfo.SndWarn || sndSize != winfo.SndSize)
                    {
                        instance.Logger.Warning(
                            $"TCP send buffer size: requested size of {sndSize} adjusted to {size}");
                        instance.SetSndBufSizeWarn(Ice.EndpointType.TCP, sndSize);
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
                        remote = proxy.GetAddress();
                    }
                    s.Append("\n" + proxy.GetName() + " proxy address = " + RemoteAddrToString(remote));
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

        public static string FdToString(Socket? socket)
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

        public static string FdLocalAddressToString(Socket socket) => "local address = " + LocalAddrToString(GetLocalAddress(socket));

        public static string
        AddrToString(EndPoint addr) => EndpointAddressToString(addr) + ":" + EndpointPort(addr);

        public static string
        LocalAddrToString(EndPoint endpoint)
        {
            if (endpoint == null)
            {
                return "<not bound>";
            }
            return EndpointAddressToString(endpoint) + ":" + EndpointPort(endpoint);
        }

        public static string
        RemoteAddrToString(EndPoint? endpoint)
        {
            if (endpoint == null)
            {
                return "<not connected>";
            }
            return EndpointAddressToString(endpoint) + ":" + EndpointPort(endpoint);
        }

        public static EndPoint
        GetLocalAddress(Socket socket)
        {
            try
            {
                return socket.LocalEndPoint;
            }
            catch (SocketException ex)
            {
                throw new Ice.TransportException(ex);
            }
        }

        public static EndPoint?
        GetRemoteAddress(Socket socket)
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

        private static IPAddress?
        GetInterfaceAddress(string iface, AddressFamily family)
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

        private static int
        GetInterfaceIndex(string iface, AddressFamily family)
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

        public static IPAddress? GetNumericAddress(string sourceAddress)
        {
            if (!string.IsNullOrEmpty(sourceAddress))
            {
                List<IPEndPoint> addrs = GetAddresses(sourceAddress, 0, EnableBoth, Ice.EndpointSelectionType.Ordered,
                                                    false, false);
                if (addrs.Count != 0)
                {
                    return addrs[0].Address;
                }
            }
            return null;
        }

        private static bool
        IsWildcard(string address, out bool ipv4Wildcard)
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

        public static string
        EndpointAddressToString(EndPoint? endpoint)
        {
            if (endpoint != null && endpoint is IPEndPoint ipEndpoint)
            {
                return ipEndpoint.Address.ToString();
            }
            return "";
        }

        public static int
        EndpointPort(EndPoint? endpoint)
        {
            if (endpoint != null && endpoint is IPEndPoint ipEndpoint)
            {
                return ipEndpoint.Port;
            }
            return -1;
        }
    }
}
