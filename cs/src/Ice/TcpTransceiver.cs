// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// .NET and Silverlight use the new socket asynchronous APIs whereas
// the compact framework and mono still use the old Begin/End APIs.
//
// #if !COMPACT && !__MonoCS__ && !UNITY
// #define ICE_SOCKET_ASYNC_API
// #endif

namespace IceInternal
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Net;
    using System.Net.Sockets;

    sealed class TcpTransceiver : Transceiver
    {
        public Socket fd()
        {
            return _fd;
        }

        public int initialize(Buffer readBuffer, Buffer writeBuffer, ref bool hasMoreData)
        {
            if(_state == StateNeedConnect)
            {
                _state = StateConnectPending;
                return SocketOperation.Connect;
            }
            else if(_state <= StateConnectPending)
            {
#if ICE_SOCKET_ASYNC_API
                if(_writeEventArgs.SocketError != SocketError.Success)
                {
                    SocketException ex = new SocketException((int)_writeEventArgs.SocketError);
                    if(Network.connectionRefused(ex))
                    {
                        throw new Ice.ConnectionRefusedException(ex);
                    }
                    else
                    {
                        throw new Ice.ConnectFailedException(ex);
                    }
                }
#else
                Network.doFinishConnectAsync(_fd, _writeResult);
                _writeResult = null;
#endif
                _desc = Network.fdToString(_fd, _proxy, _addr);

                if(_proxy != null)
                {
                    _state = StateProxyConnectRequest; // Send proxy connect request
                    return SocketOperation.Write;
                }

                _state = StateConnected;
            }
            else if(_state == StateProxyConnectRequest)
            {
                _state = StateProxyConnectRequestPending; // Wait for proxy response
                return SocketOperation.Read;
            }
            else if(_state == StateProxyConnectRequestPending)
            {
                _state = StateConnected;
            }

            Debug.Assert(_state == StateConnected);
            return SocketOperation.None;
        }

        public int closing(bool initiator, Ice.LocalException ex)
        {
            // If we are initiating the connection closure, wait for the peer
            // to close the TCP/IP connection. Otherwise, close immediately.
            return initiator ? SocketOperation.Read : SocketOperation.None;
        }

        public void close()
        {
            Debug.Assert(_fd != null);
            try
            {
                _fd.Close();
            }
            catch(SocketException ex)
            {
                throw new Ice.SocketException(ex);
            }
            finally
            {
                _fd = null;
            }
        }

        public EndpointI bind(EndpointI endp)
        {
            Debug.Assert(false);
            return null;
        }

        public void destroy()
        {
#if ICE_SOCKET_ASYNC_API
            Debug.Assert(_readEventArgs != null && _writeEventArgs != null);
            _readEventArgs.Dispose();
            _writeEventArgs.Dispose();
#endif
        }

        public int write(Buffer buf)
        {
            int packetSize = buf.b.remaining();
            if(packetSize == 0)
            {
                return SocketOperation.None;
            }

#if COMPACT || SILVERLIGHT
            if(_writeResult != null)
            {
                return SocketOperation.None;
            }
            //
            // Silverlight and the Compact .NET Frameworks don't support the use of synchronous socket
            // operations on a non-blocking socket. Returning SocketOperation.Write here forces the caller
            // to schedule an asynchronous operation.
            //
            return SocketOperation.Write;
#else
            if(AssemblyUtil.platform_ == AssemblyUtil.Platform.Windows)
            {
                //
                // On Windows, limiting the buffer size is important to prevent
                // poor throughput performance when transferring large amounts of
                // data. See Microsoft KB article KB823764.
                //
                if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize / 2)
                {
                    packetSize = _maxSendPacketSize / 2;
                }
            }

            while(buf.b.hasRemaining())
            {
                try
                {
                    Debug.Assert(_fd != null);

                    int ret;
                    try
                    {
                        ret = _fd.Send(buf.b.rawBytes(), buf.b.position(), packetSize, SocketFlags.None);
                    }
                    catch(SocketException e)
                    {
                        if(Network.wouldBlock(e))
                        {
                            return SocketOperation.Write;
                        }
                        throw;
                    }

                    Debug.Assert(ret > 0);
                    buf.b.position(buf.b.position() + ret);
                    if(packetSize > buf.b.remaining())
                    {
                        packetSize = buf.b.remaining();
                    }
                }
                catch(SocketException ex)
                {
                    if(Network.connectionLost(ex))
                    {
                        throw new Ice.ConnectionLostException(ex);
                    }

                    throw new Ice.SocketException(ex);
                }
            }

            return SocketOperation.None; // No more data to send.
#endif
        }

        public int read(Buffer buf, ref bool hasMoreData)
        {
            int remaining = buf.b.remaining();
            if(remaining == 0)
            {
                return SocketOperation.None;
            }

#if COMPACT || SILVERLIGHT
            //
            // Silverlight and the Compact .NET Framework don't support the use of synchronous socket
            // operations on a non-blocking socket.
            //
            return SocketOperation.Read;
#else
            int position = buf.b.position();
            while(buf.b.hasRemaining())
            {
                try
                {
                    Debug.Assert(_fd != null);

                    int ret;
                    try
                    {
                        //
                        // Try to receive first. Much of the time, this will work and we
                        // avoid the cost of calling Poll().
                        //
                        ret = _fd.Receive(buf.b.rawBytes(), position, remaining, SocketFlags.None);
                        if(ret == 0)
                        {
                            throw new Ice.ConnectionLostException();
                        }
                    }
                    catch(SocketException e)
                    {
                        if(Network.wouldBlock(e))
                        {
                            return SocketOperation.Read;
                        }
                        throw;
                    }

                    Debug.Assert(ret > 0);
                    remaining -= ret;
                    buf.b.position(position += ret);
                }
                catch(SocketException ex)
                {
                    //
                    // On Mono, calling shutdownReadWrite() followed by read() causes Socket.Receive() to
                    // raise a socket exception with the "interrupted" error code. We need to check the
                    // socket's Connected status before checking for the interrupted case.
                    //
                    if(!_fd.Connected)
                    {
                        throw new Ice.ConnectionLostException(ex);
                    }

                    if(Network.interrupted(ex))
                    {
                        continue;
                    }

                    if(Network.connectionLost(ex))
                    {
                        throw new Ice.ConnectionLostException(ex);
                    }

                    throw new Ice.SocketException(ex);
                }
            }

            return SocketOperation.None;
#endif
        }

        public bool startRead(Buffer buf, AsyncCallback callback, object state)
        {
#if ICE_SOCKET_ASYNC_API
            Debug.Assert(_fd != null && _readEventArgs != null);
#else
            Debug.Assert(_fd != null && _readResult == null);
#endif
            if(_state == StateProxyConnectRequestPending)
            {
                _proxy.beginReadConnectRequestResponse(buf);
            }

            int packetSize = buf.b.remaining();
            if(_maxReceivePacketSize > 0 && packetSize > _maxReceivePacketSize)
            {
                packetSize = _maxReceivePacketSize;
            }

            try
            {
                _readCallback = callback;
#if ICE_SOCKET_ASYNC_API
                _readEventArgs.UserToken = state;
                _readEventArgs.SetBuffer(buf.b.rawBytes(), buf.b.position(), packetSize);
                return !_fd.ReceiveAsync(_readEventArgs);
#else
                _readResult = _fd.BeginReceive(buf.b.rawBytes(), buf.b.position(), packetSize, SocketFlags.None,
                                               readCompleted, state);
                return _readResult.CompletedSynchronously;
#endif
            }
            catch(SocketException ex)
            {
                if(Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }

                throw new Ice.SocketException(ex);
            }
        }

        public void finishRead(Buffer buf)
        {
            if(_fd == null) // Transceiver was closed
            {
#if !ICE_SOCKET_ASYNC_API
                _readResult = null;
#endif
                return;
            }

#if ICE_SOCKET_ASYNC_API
            Debug.Assert(_fd != null && _readEventArgs != null);
#else
            Debug.Assert(_fd != null && _readResult != null);
#endif

            try
            {
#if ICE_SOCKET_ASYNC_API
                if(_readEventArgs.SocketError != SocketError.Success)
                {
                    throw new SocketException((int)_readEventArgs.SocketError);
                }
                int ret = _readEventArgs.BytesTransferred;
#else
                int ret = _fd.EndReceive(_readResult);
                _readResult = null;
#endif
                if(ret == 0)
                {
                    throw new Ice.ConnectionLostException();
                }

                Debug.Assert(ret > 0);
                buf.b.position(buf.b.position() + ret);

                if(_state == StateProxyConnectRequestPending)
                {
                    _proxy.endReadConnectRequestResponse(buf);
                }
            }
            catch(SocketException ex)
            {
                if(Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }

                throw new Ice.SocketException(ex);
            }
            catch(ObjectDisposedException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
        }

        public bool startWrite(Buffer buf, AsyncCallback callback, object state, out bool completed)
        {
#if ICE_SOCKET_ASYNC_API
            Debug.Assert(_fd != null && _writeEventArgs != null);
#else
            Debug.Assert(_fd != null && _writeResult == null);
#endif

            if(_state < StateConnected)
            {
                completed = false;
                if(_state == StateConnectPending)
                {
                    _writeCallback = callback;
                    try
                    {
                        EndPoint addr = _proxy != null ? _proxy.getAddress() : _addr;
#if ICE_SOCKET_ASYNC_API
                        _writeEventArgs.RemoteEndPoint = addr;
                        _writeEventArgs.UserToken = state;
                        return !_fd.ConnectAsync(_writeEventArgs);
#else
                        _writeResult = Network.doConnectAsync(_fd, addr, _sourceAddr, callback, state);
                        return _writeResult.CompletedSynchronously;
#endif
                    }
                    catch(Exception ex)
                    {
                        throw new Ice.SocketException(ex);
                    }
                }
                else if(_state == StateProxyConnectRequest)
                {
                    _proxy.beginWriteConnectRequest(_addr, buf);
                }
            }

            //
            // We limit the packet size for beginWrite to ensure connection timeouts are based
            // on a fixed packet size.
            //
            int packetSize = buf.b.remaining();
            if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize)
            {
                packetSize = _maxSendPacketSize;
            }

            try
            {
                _writeCallback = callback;
#if ICE_SOCKET_ASYNC_API
                _writeEventArgs.UserToken = state;
                _writeEventArgs.SetBuffer(buf.b.rawBytes(), buf.b.position(), packetSize);
                bool completedSynchronously = !_fd.SendAsync(_writeEventArgs);
#else
                _writeResult = _fd.BeginSend(buf.b.rawBytes(), buf.b.position(), packetSize, SocketFlags.None,
                                             writeCompleted, state);
                bool completedSynchronously = _writeResult.CompletedSynchronously;
#endif
                completed = packetSize == buf.b.remaining();
                return completedSynchronously;
            }
            catch(SocketException ex)
            {
                if(Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }

                throw new Ice.SocketException(ex);
            }
            catch(ObjectDisposedException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
        }

        public void finishWrite(Buffer buf)
        {
            if(_fd == null) // Transceiver was closed
            {
                if(buf.size() - buf.b.position() < _maxSendPacketSize)
                {
                    buf.b.position(buf.b.limit()); // Assume all the data was sent for at-most-once semantics.
                }
#if !ICE_SOCKET_ASYNC_API
                _writeResult = null;
#endif
                return;
            }

#if ICE_SOCKET_ASYNC_API
            Debug.Assert(_fd != null && _writeEventArgs != null);
#else
            Debug.Assert(_fd != null && _writeResult != null);
#endif

            if(_state < StateConnected && _state != StateProxyConnectRequest)
            {
                return;
            }

            try
            {
#if ICE_SOCKET_ASYNC_API
                if(_writeEventArgs.SocketError != SocketError.Success)
                {
                    throw new SocketException((int)_writeEventArgs.SocketError);
                }
                int ret = _writeEventArgs.BytesTransferred;
#else
                int ret = _fd.EndSend(_writeResult);
                _writeResult = null;
#endif
                if(ret == 0)
                {
                    throw new Ice.ConnectionLostException();
                }
                Debug.Assert(ret > 0);
                buf.b.position(buf.b.position() + ret);

                if(_state == StateProxyConnectRequest)
                {
                    _proxy.endWriteConnectRequest(buf);
                }
            }
            catch(SocketException ex)
            {
                if(Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }

                throw new Ice.SocketException(ex);
            }
            catch(ObjectDisposedException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
        }

        public string protocol()
        {
            return _instance.protocol();
        }

        public Ice.ConnectionInfo getInfo()
        {
            Ice.TCPConnectionInfo info = new Ice.TCPConnectionInfo();
            if(_fd != null)
            {
                EndPoint localEndpoint = Network.getLocalAddress(_fd);
                info.localAddress = Network.endpointAddressToString(localEndpoint);
                info.localPort = Network.endpointPort(localEndpoint);
                EndPoint remoteEndpoint = Network.getRemoteAddress(_fd);
                info.remoteAddress = Network.endpointAddressToString(remoteEndpoint);
                info.remotePort = Network.endpointPort(remoteEndpoint);
            }
            return info;
        }

        public void checkSendSize(Buffer buf, int messageSizeMax)
        {
            if(buf.size() > messageSizeMax)
            {
                Ex.throwMemoryLimitException(buf.size(), messageSizeMax);
            }
        }

        public override string ToString()
        {
            return _desc;
        }

        public string toDetailedString()
        {
            return ToString();
        }

        //
        // Only for use by TcpConnector, TcpAcceptor
        //
        internal TcpTransceiver(ProtocolInstance instance, Socket fd, EndPoint addr, NetworkProxy proxy,
                                EndPoint sourceAddr, bool connected)
        {
            _instance = instance;
            _fd = fd;
            _addr = addr;
            _proxy = proxy;
            _sourceAddr = sourceAddr;

            _state = connected ? StateConnected : StateNeedConnect;
            _desc = connected ? Network.fdToString(_fd, _proxy, _addr) : "<not connected>";

#if ICE_SOCKET_ASYNC_API
            _readEventArgs = new SocketAsyncEventArgs();
            _readEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(ioCompleted);

            _writeEventArgs = new SocketAsyncEventArgs();
            _writeEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(ioCompleted);
#if SILVERLIGHT
            String policy = instance.properties().getProperty("Ice.ClientAccessPolicyProtocol");
            if(policy.Equals("Http"))
            {
                _readEventArgs.SocketClientAccessPolicyProtocol = SocketClientAccessPolicyProtocol.Http;
                _writeEventArgs.SocketClientAccessPolicyProtocol = SocketClientAccessPolicyProtocol.Http;
            }
            else if(!String.IsNullOrEmpty(policy))
            {
                _instance.logger().warning("Ignoring invalid Ice.ClientAccessPolicyProtocol value `" + policy + "'");
            }
#endif
#endif

            _maxSendPacketSize = Network.getSendBufferSize(fd);
            if(_maxSendPacketSize < 512)
            {
                _maxSendPacketSize = 0;
            }

            _maxReceivePacketSize = Network.getRecvBufferSize(fd);
            if(_maxReceivePacketSize < 512)
            {
                _maxReceivePacketSize = 0;
            }
        }

#if ICE_SOCKET_ASYNC_API
        internal void ioCompleted(object sender, SocketAsyncEventArgs e)
        {
            switch (e.LastOperation)
            {
            case SocketAsyncOperation.Receive:
                _readCallback(e.UserToken);
                break;
            case SocketAsyncOperation.Send:
            case SocketAsyncOperation.Connect:
                _writeCallback(e.UserToken);
                break;
            default:
                throw new ArgumentException("The last operation completed on the socket was not a receive or send");
            }
        }
#else
        internal void readCompleted(IAsyncResult result)
        {
            if(!result.CompletedSynchronously)
            {
                _readCallback(result.AsyncState);
            }
        }

        internal void writeCompleted(IAsyncResult result)
        {
            if(!result.CompletedSynchronously)
            {
                _writeCallback(result.AsyncState);
            }
        }
#endif

        private ProtocolInstance _instance;
        private Socket _fd;
        private EndPoint _addr;
        private NetworkProxy _proxy;
        private EndPoint _sourceAddr;
        private string _desc;
        private int _state;
        private int _maxSendPacketSize;
        private int _maxReceivePacketSize;

#if ICE_SOCKET_ASYNC_API
        private SocketAsyncEventArgs _writeEventArgs;
        private SocketAsyncEventArgs _readEventArgs;
#else
        private IAsyncResult _writeResult;
        private IAsyncResult _readResult;
#endif

        AsyncCallback _writeCallback;
        AsyncCallback _readCallback;

        private const int StateNeedConnect = 0;
        private const int StateConnectPending = 1;
        private const int StateProxyConnectRequest = 2;
        private const int StateProxyConnectRequestPending = 3;
        private const int StateConnected = 4;
    }
}
