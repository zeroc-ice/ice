// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System;
    using System.Collections;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Net;
    using System.Net.Sockets;
    using System.Runtime.InteropServices;
    using System.Threading;

    public sealed class Network
    {
        //
        // Magic numbers taken from winsock2.h
        //
        const int WSAEINTR        = 10004;
        const int WSAEFAULT       = 10014;
        const int WSAEINVAL       = 10022;
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
            string msg = ex.Message.ToLower();
            return msg.IndexOf("connection was forcibly closed") >= 0 ||
                   msg.IndexOf("remote party has closed the transport stream") >= 0 ||
                   msg.IndexOf("established connection was aborted") >= 0 ||
                   msg.IndexOf("received an unexpected eof or 0 bytes from the transport stream") >= 0;
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

        public static bool timeout(System.IO.IOException ex)
        {
            return ex.Message.IndexOf("period of time") >= 0;
        }

        public static bool isMulticast(IPEndPoint addr)
        {
            string ip = addr.Address.ToString();
            char[] splitChars = { '.' };
            string[] arr = ip.Split(splitChars);
            try
            {
                int i = System.Int32.Parse(arr[0]);
                if(i < 223 || i > 239)
                {
                    return false;
                }
            }
            catch(System.FormatException)
            {
                return false;
            }
            return true;
        }

        public static Socket createSocket(bool udp)
        {
            Socket socket;

            try
            {
                if(udp)
                {
                    socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
                }
                else
                {
                    socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
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

        public static void setMcastGroup(Socket socket, IPAddress group, IPAddress iface)
        {
            try
            {
                socket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.AddMembership, 
                                       new MulticastOption(group, iface));
            }
            catch(SocketException ex)
            {
                closeSocketNoThrow(socket);
                throw new Ice.SocketException(ex);
            }
        }

        public static void setMcastTtl(Socket socket, int ttl)
        {
            try
            {
                socket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.MulticastTimeToLive, ttl);
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
        
        public static void doConnect(Socket socket, EndPoint addr, int timeout)
        {
            //
            // MONO workaround for
            // http://bugzilla.zeroc.com/bugzilla/show_bug.cgi?id=1647. 
            //
            // It would have been nice to be able to get rid of the 2
            // implementations of doConnect() and only use
            // doConnectAsync() however, with .NET doConnectAsync()
            // doesn't work with the TCP transport.  In particular,
            // the test/Ice/timeout test fails in the connect timeout
            // test because the client hangs in the Receive() call
            // waiting for the connection validation (for some reasons
            // Receive blocks even though the socket is
            // non-blocking...)
            //
            if(AssemblyUtil.runtime_ == AssemblyUtil.Runtime.Mono)
            {
                doConnectAsync(socket, addr, timeout);
                return;
            }

        repeatConnect:
            try
            {
                socket.Connect(addr);
            }
            catch(SocketException ex)
            {
                if(interrupted(ex))
                {
                    goto repeatConnect;
                }

                if(!connectInProgress(ex))
                {
                    closeSocketNoThrow(socket);
                    //
                    // Check for connectionRefused, and connectFailed
                    // here.
                    //
                    if(connectionRefused(ex))
                    {
                        throw new Ice.ConnectionRefusedException(ex);
                    }
                    else if(connectFailed(ex))
                    {
                        throw new Ice.ConnectFailedException(ex);
                    }
                    else
                    {
                        throw new Ice.SocketException(ex);
                    }
                }

            repeatSelect:
                bool ready;
                bool error;
                try
                {
                    ArrayList writeList = new ArrayList();
                    writeList.Add(socket);
                    ArrayList errorList = new ArrayList();
                    errorList.Add(socket);
                    doSelect(null, writeList, errorList, timeout);
                    ready = writeList.Count != 0;
                    error = errorList.Count != 0;

                    //
                    // As with C++ we need to get the SO_ERROR error
                    // to determine whether the connect has actually
                    // failed.
                    //
                    int val = (int)socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.Error);
                    if(val > 0)
                    {
                        closeSocketNoThrow(socket);

                        //
                        // Create a Win32Exception out of this error value. Check the for refused, and
                        // failed. Otherwise its a plain old socket exception.
                        //
                        Win32Exception sockEx = new Win32Exception(val);
                        if(connectionRefused(sockEx))
                        {
                            throw new Ice.ConnectionRefusedException(sockEx);
                        }
                        else if(connectFailed(sockEx))
                        {
                            throw new Ice.ConnectFailedException(sockEx);
                        }
                        else
                        {
                            throw new Ice.SocketException(sockEx);
                        }                       
                    }
                    
                    Debug.Assert(!(ready && error));
                }
                catch(SocketException e)
                {
                    if(interrupted(e))
                    {
                        goto repeatSelect;
                    }

                    closeSocketNoThrow(socket);
                    throw new Ice.SocketException(e);
                }

                if(error || !ready)
                {
                    closeSocketNoThrow(socket);

                    //
                    // If GetSocketOption didn't return an error and we've failed then we cannot
                    // distinguish between connect failed and connection refused.
                    //
                    if(error)
                    {
                        throw new Ice.ConnectFailedException();
                    }
                    else
                    {
                        throw new Ice.ConnectTimeoutException();
                    }
                }
            }
        }

        internal class AsyncConnectInfo
        {
            internal AsyncConnectInfo(Socket fd)
            {
                this.fd = fd;
                this.ex = null;
                this.done = false;
            }

            internal Socket fd;
            volatile internal Exception ex;
            volatile internal bool done;
        }

        private static void asyncConnectCallback(IAsyncResult ar)
        {
            AsyncConnectInfo info = (AsyncConnectInfo)ar.AsyncState;
            lock(info)
            {
                try
                {
                    info.fd.EndConnect(ar);
                }
                catch(Exception ex)
                {
                    info.ex = ex;
                }
                finally
                {
                    info.done = true;
                    Monitor.Pulse(info);
                }
            }
        }

        public static void doConnectAsync(Socket socket, EndPoint addr, int timeout)
        {
        repeatConnect:
            try
            {
                AsyncConnectInfo info = new AsyncConnectInfo(socket);
                /* IAsyncResult ar = */ socket.BeginConnect(addr, new AsyncCallback(asyncConnectCallback), info);
                lock(info)
                {
                    if(!info.done)
                    {
                        if(!Monitor.Wait(info, timeout == -1 ? Timeout.Infinite : timeout))
                        {
                            throw new Ice.ConnectTimeoutException();
                        }
                    }
                    if(info.ex != null)
                    {
                        throw info.ex;
                    }
                }
            }
            catch(SocketException ex)
            {
                if(interrupted(ex))
                {
                    goto repeatConnect;
                }

                closeSocketNoThrow(socket);

                //
                // Check for connectionRefused, and connectFailed
                // here.
                //
                if(connectionRefused(ex))
                {
                    throw new Ice.ConnectionRefusedException(ex);
                }
                else if(connectFailed(ex))
                {
                    throw new Ice.ConnectFailedException(ex);
                }
                else
                {
                    throw new Ice.SocketException(ex);
                }
            }
        }

        public static Socket doAccept(Socket socket, int timeout)
        {
            Socket ret = null;

        repeatAccept:
            try
            {
                ret = socket.Accept();
            }
            catch(SocketException ex)
            {
                if(acceptInterrupted(ex))
                {
                    goto repeatAccept;
                }
                if(wouldBlock(ex))
                {
                repeatSelect:
                    ArrayList readList = new ArrayList();
                    readList.Add(socket);
                    try
                    {
                        doSelect(readList, null, null, timeout);
                    }
                    catch(Win32Exception we)
                    {
                        if(interrupted(we))
                        {
                            goto repeatSelect;
                        }
                        throw new Ice.SocketException(we);
                    }
                    catch(System.Exception se)
                    {
                        throw new Ice.SocketException(se);
                    }

                    if(readList.Count == 0)
                    {
                        throw new Ice.TimeoutException();
                    }

                    goto repeatAccept;
                }
            }

            setTcpNoDelay(ret);
            setKeepAlive(ret);
            return ret;
        }
        
        private static ArrayList copyList(IList list)
        {
            if(list == null)
            {
                return null;
            }
            return new ArrayList(list);
        }

        private static void overwriteList(ArrayList from, IList to)
        {
            if(from != null && to != null)
            {
                to.Clear();
                foreach(object o in from)
                {
                    to.Add(o);
                }
            }
        }

        public enum PollMode { Read, Write, Error };

        public static bool doPoll(Socket s, int timeout, PollMode mode)
        {
            //
            // Poll() wants microseconds, so we need to deal with overflow.
            //
            while((timeout > System.Int32.MaxValue / 1000))
            {
                if(s.Poll((System.Int32.MaxValue / 1000) * 1000, (SelectMode)mode))
                {
                    return true;
                }
                timeout -= System.Int32.MaxValue / 1000;
            }
            return s.Poll(timeout * 1000, (SelectMode)mode);
        }

        public static void doSelect(IList checkRead, IList checkWrite, IList checkError, int milliSeconds)
        {
            ArrayList cr = null;
            ArrayList cw = null;
            ArrayList ce = null;

            if(milliSeconds < 0)
            {
                //
                // Socket.Select() returns immediately if the timeout is < 0 (instead
                // of blocking indefinitely), so we have to emulate a blocking select here.
                // (Using Int32.MaxValue isn't good enough because that's only about 35 minutes.)
                //
                do {
                    cr = copyList(checkRead);
                    cw = copyList(checkWrite);
                    ce = copyList(checkError);
                    try
                    {
                        Socket.Select(cr, cw, ce, System.Int32.MaxValue);
                    }
                    catch(SocketException e)
                    {
                        if(interrupted(e))
                        {
                                continue;
                        }
                        throw new Ice.SocketException(e);
                    }
                }
                while((cr == null || cr.Count == 0) &&
                      (cw == null || cw.Count == 0) &&
                      (ce == null || ce.Count == 0));
                overwriteList(cr, checkRead);
                overwriteList(cw, checkWrite);
                overwriteList(ce, checkError);
            }
            else
            {
                //
                // Select() wants microseconds, so we need to deal with overflow.
                //
                while((milliSeconds > System.Int32.MaxValue / 1000) &&
                      ((cr == null) || cr.Count == 0) &&
                      ((cw == null) || cw.Count == 0) &&
                      ((ce == null) || ce.Count == 0))
                {
                    cr = copyList(checkRead);
                    cw = copyList(checkWrite);
                    ce = copyList(checkError);
                    try
                    {
                        Socket.Select(cr, cw, ce, (System.Int32.MaxValue / 1000) * 1000);
                    }
                    catch(SocketException e)
                    {
                        if(interrupted(e))
                        {
                            continue;
                        }
                        throw new Ice.SocketException(e);
                    }
                    milliSeconds -= System.Int32.MaxValue / 1000;
                }
                if(cr == null && cw == null && ce == null)
                {
                    Socket.Select(checkRead, checkWrite, checkError, milliSeconds * 1000);
                }
                overwriteList(cr, checkRead);
                overwriteList(cw, checkWrite);
                overwriteList(ce, checkError);
            }
        }

        public static IPEndPoint getAddress(string host, int port)
        {
            int retry = 5;

        repeatGetHostByName:
            try
            {
                try
                {
                    return new IPEndPoint(IPAddress.Parse(host), port);
                }
                catch (FormatException)
                {
                }
                IPHostEntry e = Dns.GetHostEntry(host);
                for(int i = 0; i < e.AddressList.Length; ++i)
                {
                    if(e.AddressList[i].AddressFamily != AddressFamily.InterNetworkV6)
                    {
                        return new IPEndPoint(e.AddressList[i], port);
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
            // No InterNetworkV4 address available.
            //
            Ice.DNSException dns = new Ice.DNSException();
            dns.host = host;
            throw dns;
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
            Debug.Assert(larr.Length == rarr.Length);
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
        
        public static string getNumericHost(string hostname)
        {
            int retry = 5;

        repeatGetHostByName:
            try
            {
                IPHostEntry e = Dns.GetHostEntry(hostname);
                for(int i = 0; i < e.AddressList.Length; ++i)
                {
                    if(e.AddressList[i].AddressFamily != AddressFamily.InterNetworkV6)
                    {
                        return e.AddressList[i].ToString();
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
                e.host = hostname;
                throw e;
            }
            catch(System.Exception ex)
            {
                Ice.DNSException e = new Ice.DNSException(ex);
                e.host = hostname;
                throw e;
            }

            //
            // No InterNetworkV4 address available.
            //
            Ice.DNSException dns = new Ice.DNSException();
            dns.host = hostname;
            throw dns;
        }

        public static IPEndPoint[] getAddresses(string host, int port)
        {
            ArrayList addresses = new ArrayList();

            int retry = 5;

        repeatGetHostByName:
            try
            {
                //
                // No need for lookup if host is ip address.
                //
                try
                {
                    addresses.Add(new IPEndPoint(IPAddress.Parse(host), port));
                }
                catch (FormatException)
                {
                    IPHostEntry e = Dns.GetHostEntry(host);
                    for(int i = 0; i < e.AddressList.Length; ++i)
                    {
                        if(e.AddressList[i].AddressFamily != AddressFamily.InterNetworkV6)
                        {
                            addresses.Add(new IPEndPoint(e.AddressList[i], port));
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
                e.host = host;
                throw e;
            }
            catch(System.Exception ex)
            {
                Ice.DNSException e = new Ice.DNSException(ex);
                e.host = host;
                throw e;
            }

            return (IPEndPoint[])addresses.ToArray(typeof(IPEndPoint));
        }

        public static string[] getLocalHosts()
        {
            ArrayList hosts;

            int retry = 5;

        repeatGetHostByName:
            try
            {
                IPHostEntry e = Dns.GetHostEntry(Dns.GetHostName());
                hosts = new ArrayList();
                for(int i = 0; i < e.AddressList.Length; ++i)
                {
                    if(e.AddressList[i].AddressFamily != AddressFamily.InterNetworkV6)
                    {
                        hosts.Add(e.AddressList[i].ToString());
                    }
                }
                hosts.Add(IPAddress.Loopback.ToString());
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

            return (string[])hosts.ToArray(typeof(string));
        }

        public sealed class SocketPair
        {
            public Socket source;
            public Socket sink;

            public SocketPair()
            {
                sink = createSocket(false);
                Socket listener = createSocket(false);

                doBind(listener, new IPEndPoint(IPAddress.Parse("127.0.0.1"), 0));
                doListen(listener, 1);
                doConnect(sink, listener.LocalEndPoint, 1000);
                try
                {
                    source = doAccept(listener, -1);
                }
                catch(Ice.SocketException)
                {
                    try
                    {
                        sink.Close();
                    }
                    catch(System.Exception)
                    {
                        // ignore
                    }
                    throw;
                }
                finally
                {
                    try
                    {
                        listener.Close();
                    }
                    catch(System.Exception)
                    {
                    }
                }
            }
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

        public static SocketPair createPipe()
        {
            return new SocketPair();
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct in_addr
        {
            [MarshalAs(UnmanagedType.ByValArray, SizeConst=4)]
            public byte[] sin_addr;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct sockaddr
        {
            public short sin_family;
            public ushort sin_port;
            public in_addr sin_addr;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst=8)]
            public byte[] sin_zero;
        }

        [DllImport("wsock32.dll")]
        private static extern int getsockname(IntPtr s, ref sockaddr name, ref int namelen);

        [DllImport("wsock32.dll")]
        private static extern int getpeername(IntPtr s, ref sockaddr name, ref int namelen);

        [DllImport("ws2_32.dll")]
        private static extern IntPtr inet_ntoa(in_addr a);

        [DllImport("ws2_32.dll")]
        private static extern ushort ntohs(ushort netshort);

        public static string fdToString(Socket socket)
        {
            if(socket == null)
            {
                return "<closed>";
            }
            return addressesToString(getLocalAddress(socket), getRemoteAddress(socket));
        }

        public static string
        addressesToString(IPEndPoint localEndpoint, IPEndPoint remoteEndpoint)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder();
            s.Append("local address = " + localEndpoint.Address);
            s.Append(":" + localEndpoint.Port);

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
            //
            // .Net BUG: The LocalEndPoint and RemoteEndPoint properties
            // are null for a socket that was connected in non-blocking
            // mode. The only way to make this work is to step down to
            // the native API and use platform invoke :-(
            //
            IPEndPoint localEndpoint;
            if(AssemblyUtil.platform_ == AssemblyUtil.Platform.Windows)
            {
                sockaddr addr = new sockaddr();
                int addrLen = 16;

                if(getsockname(socket.Handle, ref addr, ref addrLen) != 0)
                {
                    throw new Ice.SyscallException();
                }
                string ip = Marshal.PtrToStringAnsi(inet_ntoa(addr.sin_addr));
                int port = ntohs(addr.sin_port);
                localEndpoint = new IPEndPoint(IPAddress.Parse(ip), port);
            }
            else
            {
                try
                {
                    localEndpoint = (IPEndPoint)socket.LocalEndPoint;
                }
                catch(SocketException ex)
                {
                    throw new Ice.SocketException(ex);
                }
            }
            return localEndpoint;
        }

        public static IPEndPoint
        getRemoteAddress(Socket socket)
        { 
            //
            // .Net BUG: The LocalEndPoint and RemoteEndPoint properties
            // are null for a socket that was connected in non-blocking
            // mode. The only way to make this work is to step down to
            // the native API and use platform invoke :-(
            //
            IPEndPoint remoteEndpoint = null;
            if(AssemblyUtil.platform_ == AssemblyUtil.Platform.Windows)
            {
                sockaddr addr = new sockaddr();
                int addrLen = 16;

                if(getpeername(socket.Handle, ref addr, ref addrLen) == 0)
                { 
                    string ip = Marshal.PtrToStringAnsi(inet_ntoa(addr.sin_addr));
                    int port = ntohs(addr.sin_port);
                    remoteEndpoint = new IPEndPoint(IPAddress.Parse(ip), port);
                }
            }
            else
            {
                try
                {
                    remoteEndpoint = (IPEndPoint)socket.RemoteEndPoint;
                }
                catch(SocketException)
                {
                    remoteEndpoint = null;
                }
            }
            return remoteEndpoint;
        }
    }
}
