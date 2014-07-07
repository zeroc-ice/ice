// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// .NET and Silverlight use the new socket asynchronous APIs whereas
// the compact framework and mono still use the old Begin/End APIs.
//
#if !COMPACT && !__MonoCS__ && !UNITY
#define ICE_SOCKET_ASYNC_API
#endif

namespace IceInternal
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Net;
    using System.Net.Sockets;

    sealed class TcpTransceiver : Transceiver
    {
        public int initialize()
        {
            try
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
            }
            catch(Ice.LocalException ex)
            {
                if(_traceLevels.network >= 2)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("failed to establish tcp connection\n");
                    s.Append(Network.fdToString(_fd, _proxy, _addr));
                    s.Append("\n");
                    s.Append(ex.ToString());
                    _logger.trace(_traceLevels.networkCat, s.ToString());
                }
                throw;
            }

            Debug.Assert(_state == StateConnected);
            if(_traceLevels.network >= 1)
            {
                string s = "tcp connection established\n" + _desc;
                _logger.trace(_traceLevels.networkCat, s);
            }
            return SocketOperation.None;
        }

        public void close()
        {
            //
            // If the transceiver is not connected, its description is simply "not connected",
            // which isn't very helpful.
            //
            if(_state == StateConnected && _traceLevels.network >= 1)
            {
                string s = "closing tcp connection\n" + ToString();
                _logger.trace(_traceLevels.networkCat, s);
            }

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

        public void destroy()
        {
#if ICE_SOCKET_ASYNC_API
            Debug.Assert(_readEventArgs != null && _writeEventArgs != null);
            _readEventArgs.Dispose();
            _writeEventArgs.Dispose();
#endif            
        }

        public bool write(Buffer buf)
        {
#if COMPACT || SILVERLIGHT
            //
            // Silverlight and the Compact .NET Frameworks don't support the use of synchronous socket
            // operations on a non-blocking socket. Returning false here forces the caller to schedule
            // an asynchronous operation.
            //
            return false;
#else
            int packetSize = buf.b.remaining();
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
                            return false;
                        }
                        throw;
                    }

                    Debug.Assert(ret > 0);

                    if(_traceLevels.network >= 3)
                    {
                        string s = "sent " + ret + " of " + packetSize + " bytes via tcp\n" + ToString();
                        _logger.trace(_traceLevels.networkCat, s);
                    }

                    if(_stats != null)
                    {
#pragma warning disable 618                        
                        _stats.bytesSent(type(), ret);
#pragma warning restore 618
                    }

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

            return true; // No more data to send.
#endif
        }

        public bool read(Buffer buf)
        {
#if COMPACT || SILVERLIGHT
            //
            // Silverlight and the Compact .NET Framework don't support the use of synchronous socket
            // operations on a non-blocking socket.
            //
            return false;
#else
            int remaining = buf.b.remaining();
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
                            return false;
                        }
                        throw;
                    }

                    Debug.Assert(ret > 0);

                    if(_traceLevels.network >= 3)
                    {
                        string s = "received " + ret + " of " + remaining + " bytes via tcp\n" + ToString();
                        _logger.trace(_traceLevels.networkCat, s);
                    }

                    if(_stats != null)
                    {
#pragma warning disable 618
                        _stats.bytesReceived(type(), ret);
#pragma warning restore 618
                    }

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

            return true;
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
                _readEventArgs.SetBuffer(null, 0, 0);
#else
                int ret = _fd.EndReceive(_readResult);
                _readResult = null;
#endif
                if(ret == 0)
                {
                    throw new Ice.ConnectionLostException();
                }

                Debug.Assert(ret > 0);

                if(_traceLevels.network >= 3)
                {
                    int packetSize = buf.b.remaining();
                    if(_maxReceivePacketSize > 0 && packetSize > _maxReceivePacketSize)
                    {
                        packetSize = _maxReceivePacketSize;
                    }
                    string s = "received " + ret + " of " + packetSize + " bytes via tcp\n" + ToString();
                    _logger.trace(_traceLevels.networkCat, s);
                }

                if(_stats != null)
                {
#pragma warning disable 618
                    _stats.bytesReceived(type(), ret);
#pragma warning restore 618
                }

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
                        _writeResult = Network.doConnectAsync(_fd, addr, callback, state);
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
                    buf.b.position(buf.size()); // Assume all the data was sent for at-most-once semantics.
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
                _writeEventArgs.SetBuffer(null, 0, 0);
#else
                int ret = _fd.EndSend(_writeResult);
                _writeResult = null;
#endif
                if(ret == 0)
                {
                    throw new Ice.ConnectionLostException();
                }
                Debug.Assert(ret > 0);

                if(_traceLevels.network >= 3)
                {
                    int packetSize = buf.b.remaining();
                    if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize)
                    {
                        packetSize = _maxSendPacketSize;
                    }
                    string s = "sent " + ret + " of " + packetSize + " bytes via tcp\n" + ToString();
                    _logger.trace(_traceLevels.networkCat, s);
                }

                if(_stats != null)
                {
#pragma warning disable 618
                    _stats.bytesSent(type(), ret);
#pragma warning restore 618
                }

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

        public string type()
        {
            return "tcp";
        }

        public Ice.ConnectionInfo
        getInfo()
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

        //
        // Only for use by TcpConnector, TcpAcceptor
        //
        internal TcpTransceiver(Instance instance, Socket fd, EndPoint addr, NetworkProxy proxy, bool connected)
        {
            _fd = fd;
            _addr = addr;
            _proxy = proxy;
            
            _traceLevels = instance.traceLevels();
            _logger = instance.initializationData().logger;
            _stats = instance.initializationData().stats;
            _state = connected ? StateConnected : StateNeedConnect;
            _desc = connected ? Network.fdToString(_fd, _proxy, _addr) : "<not connected>";
            
#if ICE_SOCKET_ASYNC_API
            _readEventArgs = new SocketAsyncEventArgs();
            _readEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(ioCompleted);

            _writeEventArgs = new SocketAsyncEventArgs();
            _writeEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(ioCompleted);
#if SILVERLIGHT
            String policy = instance.initializationData().properties.getProperty("Ice.ClientAccessPolicyProtocol");
            if(policy.Equals("Http"))
            {
                _readEventArgs.SocketClientAccessPolicyProtocol = SocketClientAccessPolicyProtocol.Http;
                _writeEventArgs.SocketClientAccessPolicyProtocol = SocketClientAccessPolicyProtocol.Http;
            }
            else if(!String.IsNullOrEmpty(policy))
            {
                _logger.warning("Ignoring invalid Ice.ClientAccessPolicyProtocol value `" + policy + "'");
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

        private Socket _fd;
        private EndPoint _addr;
        private NetworkProxy _proxy;
        private TraceLevels _traceLevels;
        private Ice.Logger _logger;
        private Ice.Stats _stats;
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
