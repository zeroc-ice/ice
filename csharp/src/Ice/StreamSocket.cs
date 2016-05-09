// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
    using System.Diagnostics;
    using System.Net;
    using System.Net.Sockets;

    public sealed class StreamSocket
    {
        public StreamSocket(ProtocolInstance instance, NetworkProxy proxy, EndPoint addr, EndPoint sourceAddr)
        {
            _instance = instance;
            _proxy = proxy;
            _addr = addr;
            _sourceAddr = sourceAddr;
            _fd = Network.createSocket(false, (_proxy != null ? _proxy.getAddress() : _addr).AddressFamily);
            _state = StateNeedConnect;

            init();
        }

        public StreamSocket(ProtocolInstance instance, Socket fd)
        {
            _instance = instance;
            _fd = fd;
            _state = StateConnected;
            try
            {
                _desc = IceInternal.Network.fdToString(_fd);
            }
            catch(Exception ex)
            {
                Network.closeSocketNoThrow(_fd);
                throw ex;
            }
            init();
        }

#if !SILVERLIGHT
        public void setBlock(bool block)
        {
            Network.setBlock(_fd, block);
        }
#endif

        public int connect(Buffer readBuffer, Buffer writeBuffer, ref bool moreData)
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
                _state = _proxy != null ? StateProxyWrite : StateConnected;
            }

            if(_state == StateProxyWrite)
            {
                _proxy.beginWrite(_addr, writeBuffer);
                return SocketOperation.Write;
            }
            else if(_state == StateProxyRead)
            {
                _proxy.beginRead(readBuffer);
                return SocketOperation.Read;
            }
            else if(_state == StateProxyConnected)
            {
                _proxy.finish(readBuffer, writeBuffer);

                readBuffer.clear();
                writeBuffer.clear();

                _state = StateConnected;
            }

            Debug.Assert(_state == StateConnected);
            return SocketOperation.None;
        }

        public bool isConnected()
        {
            return _state == StateConnected && _fd != null;
        }

        public Socket fd()
        {
            return _fd;
        }

        public int getSendPacketSize(int length)
        {
            return _maxSendPacketSize > 0 ? System.Math.Min(length, _maxSendPacketSize) : length;
        }

        public int getRecvPacketSize(int length)
        {
            return _maxRecvPacketSize > 0 ? System.Math.Min(length, _maxRecvPacketSize) : length;
        }

        public void setBufferSize(int rcvSize, int sndSize)
        {
            Network.setTcpBufSize(_fd, rcvSize, sndSize, _instance);
        }

        public int read(Buffer buf)
        {
            if(_state == StateProxyRead)
            {
                while(true)
                {
                    int ret = read(buf.b);
                    if(ret == 0)
                    {
                        return SocketOperation.Read;
                    }

                    _state = toState(_proxy.endRead(buf));
                    if(_state != StateProxyRead)
                    {
                        return SocketOperation.None;
                    }
                }
            }
            read(buf.b);
            return buf.b.hasRemaining() ? SocketOperation.Read : SocketOperation.None;
        }

        public int write(Buffer buf)
        {
            if(_state == StateProxyWrite)
            {
                while(true)
                {
                    int ret = write(buf.b);
                    if(ret == 0)
                    {
                        return SocketOperation.Write;
                    }
                    _state = toState(_proxy.endWrite(buf));
                    if(_state != StateProxyWrite)
                    {
                        return SocketOperation.None;
                    }
                }
            }
            write(buf.b);
            return buf.b.hasRemaining() ? SocketOperation.Write : SocketOperation.None;
        }

        public bool startRead(Buffer buf, AsyncCallback callback, object state)
        {
#if ICE_SOCKET_ASYNC_API
            Debug.Assert(_fd != null && _readEventArgs != null);
#else
            Debug.Assert(_fd != null && _readResult == null);
#endif

            int packetSize = getRecvPacketSize(buf.b.remaining());
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
                buf.b.position(buf.b.position() + ret);

                if(_state == StateProxyRead)
                {
                    _state = toState(_proxy.endRead(buf));
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

            if(_state == StateConnectPending)
            {
                completed = false;
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

            int packetSize = getSendPacketSize(buf.b.remaining());
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

            if(_state < StateConnected && _state != StateProxyWrite)
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
                buf.b.position(buf.b.position() + ret);

                if(_state == StateProxyWrite)
                {
                    _state = toState(_proxy.endWrite(buf));
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

        public void close()
        {
            Debug.Assert(_fd != null);
            try
            {
                Network.closeSocket(_fd);
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

        public override string ToString()
        {
            return _desc;
        }

        private int read(ByteBuffer buf)
        {
            Debug.Assert(_fd != null);

#if COMPACT || SILVERLIGHT
            //
            // Silverlight and the Compact .NET Framework don't
            // support the use of synchronous socket operations on a
            // non-blocking socket. Returning 0 here forces the caller
            // to schedule an asynchronous operation.
            //
            return 0;
#else
            int read = 0;
            while(buf.hasRemaining())
            {
                try
                {
                    int ret = _fd.Receive(buf.rawBytes(), buf.position(), buf.remaining(), SocketFlags.None);
                    if(ret == 0)
                    {
                        throw new Ice.ConnectionLostException();
                    }
                    read += ret;
                    buf.position(buf.position() + ret);
                }
                catch(SocketException ex)
                {
                    if(Network.wouldBlock(ex))
                    {
                        return read;
                    }
                    else if(Network.interrupted(ex))
                    {
                        continue;
                    }
                    else if(Network.connectionLost(ex))
                    {
                        throw new Ice.ConnectionLostException(ex);
                    }

                    throw new Ice.SocketException(ex);
                }
            }
            return read;
#endif
        }

        private int write(ByteBuffer buf)
        {
            Debug.Assert(_fd != null);

#if COMPACT || SILVERLIGHT
            //
            // Silverlight and the Compact .NET Frameworks don't
            // support the use of synchronous socket operations on a
            // non-blocking socket. Returning 0 here forces the caller
            // to schedule an asynchronous operation.
            //
            return 0;
#else

            int packetSize = buf.remaining();
            if(AssemblyUtil.platform_ == AssemblyUtil.Platform.Windows)
            {
                //
                // On Windows, limiting the buffer size is important to prevent
                // poor throughput performances when transfering large amount of
                // data. See Microsoft KB article KB823764.
                //
                if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize / 2)
                {
                    packetSize = _maxSendPacketSize / 2;
                }
            }

            int sent = 0;
            while(buf.hasRemaining())
            {
                try
                {
                    int ret = _fd.Send(buf.rawBytes(), buf.position(), packetSize, SocketFlags.None);
                    Debug.Assert(ret > 0);

                    sent += ret;
                    buf.position(buf.position() + ret);
                    if(packetSize > buf.remaining())
                    {
                        packetSize = buf.remaining();
                    }
                }
                catch(SocketException ex)
                {
                    if(Network.wouldBlock(ex))
                    {
                        return sent;
                    }
                    else if(Network.connectionLost(ex))
                    {
                        throw new Ice.ConnectionLostException(ex);
                    }
                    throw new Ice.SocketException(ex);
                }
            }
            return sent;
#endif
        }

#if ICE_SOCKET_ASYNC_API
        private void ioCompleted(object sender, SocketAsyncEventArgs e)
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
        private void readCompleted(IAsyncResult result)
        {
            if(!result.CompletedSynchronously)
            {
                _readCallback(result.AsyncState);
            }
        }

        private void writeCompleted(IAsyncResult result)
        {
            if(!result.CompletedSynchronously)
            {
                _writeCallback(result.AsyncState);
            }
        }
#endif

        private void init()
        {
#if !SILVERLIGHT
            Network.setBlock(_fd, false);
#endif
            Network.setTcpBufSize(_fd, _instance);

#if ICE_SOCKET_ASYNC_API
            _readEventArgs = new SocketAsyncEventArgs();
            _readEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(ioCompleted);

            _writeEventArgs = new SocketAsyncEventArgs();
            _writeEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(ioCompleted);
#  if SILVERLIGHT
            String policy = _instance.properties().getProperty("Ice.ClientAccessPolicyProtocol");
            if(policy.Equals("Http"))
            {
                _readEventArgs.SocketClientAccessPolicyProtocol = SocketClientAccessPolicyProtocol.Http;
                _writeEventArgs.SocketClientAccessPolicyProtocol = SocketClientAccessPolicyProtocol.Http;
            }
            else if(!String.IsNullOrEmpty(policy))
            {
                _instance.logger().warning("Ignoring invalid Ice.ClientAccessPolicyProtocol value `" + policy + "'");
            }
#  endif
#endif

            //
            // For timeouts to work properly, we need to receive/send
            // the data in several chunks. Otherwise, we would only be
            // notified when all the data is received/written. The
            // connection timeout could easily be triggered when
            // receiging/sending large messages.
            //
            _maxSendPacketSize = System.Math.Max(512, Network.getSendBufferSize(_fd));
            _maxRecvPacketSize = System.Math.Max(512, Network.getRecvBufferSize(_fd));
        }

        private int toState(int operation)
        {
            switch(operation)
            {
            case SocketOperation.Read:
                return StateProxyRead;
            case SocketOperation.Write:
                return StateProxyWrite;
            default:
                return StateProxyConnected;
            }
        }

        private readonly ProtocolInstance _instance;
        private readonly IceInternal.NetworkProxy _proxy;
        private readonly EndPoint _addr;
        private readonly EndPoint _sourceAddr;

        private Socket _fd;
        private int _maxSendPacketSize;
        private int _maxRecvPacketSize;
        private int _state;
        private string _desc;

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
        private const int StateProxyWrite = 2;
        private const int StateProxyRead = 3;
        private const int StateProxyConnected = 4;
        private const int StateConnected = 5;
    }

}
