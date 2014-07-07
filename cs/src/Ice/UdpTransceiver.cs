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
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Net;
    using System.Net.Sockets;
    using System.Text;

    sealed class UdpTransceiver : Transceiver
    {
        public int initialize()
        {
            if(_state == StateNeedConnect)
            {
                _state = StateConnectPending;
#if ICE_SOCKET_ASYNC_API && !SILVERLIGHT
                try
                {
                    _fd.Connect(_addr);
                }
                catch(SocketException ex)
                {
                    if(Network.wouldBlock(ex))
                    {
                        return SocketOperation.Connect;
                    }
                    throw new Ice.ConnectFailedException(ex);
                }
                catch(Exception ex)
                {
                    throw new Ice.ConnectFailedException(ex);
                }
#else
                return SocketOperation.Connect;
#endif
            }

            if(_state <= StateConnectPending)
            {
#if !SILVERLIGHT
                if(!AssemblyUtil.osx_)
                {
                    //
                    // On Windows, we delay the join for the mcast group after the connection
                    // establishment succeeds. This is necessary for older Windows versions 
                    // where joining the group fails if the socket isn't bound. See ICE-5113.
                    //
                    if(Network.isMulticast((IPEndPoint)_addr))
                    {
                        Network.setMcastGroup(_fd, ((IPEndPoint)_addr).Address, _mcastInterface);
                        if(_mcastTtl != -1)
                        {
                            Network.setMcastTtl(_fd, _mcastTtl, _addr.AddressFamily);
                        }
                    }
                }
#endif                    
                _state = StateConnected;
            }

            if(_state == StateConnected)
            {
                if(_traceLevels.network >= 1)
                {
                    string s = "starting to send udp packets\n" + ToString();
                    _logger.trace(_traceLevels.networkCat, s);
                }
                Debug.Assert(_state == StateConnected);
            }

            return SocketOperation.None;
        }

        public void close()
        {
            if(_state >= StateConnected && _traceLevels.network >= 1)
            {
                string s = "closing udp connection\n" + ToString();
                _logger.trace(_traceLevels.networkCat, s);
            }
            
            if(_fd != null)
            {
                try
                {
                    _fd.Close();
                }
                catch(System.IO.IOException)
                {
                }
                _fd = null;
            }
        }

        public void destroy()
        {
#if ICE_SOCKET_ASYNC_API
            _readEventArgs.Dispose();
            _writeEventArgs.Dispose();
#endif            
        }

        public bool write(Buffer buf)
        {
#if COMPACT || SILVERLIGHT
            //
            // Silverlight and the Compact .NET Framework don't support the use of synchronous socket
            // operations on a non-blocking socket. Returning false here forces the
            // caller to schedule an asynchronous operation.
            //
            return false;
#else
            Debug.Assert(buf.b.position() == 0);
            Debug.Assert(_fd != null && _state >= StateConnected);

            // The caller is supposed to check the send size before by calling checkSendSize
            Debug.Assert(System.Math.Min(_maxPacketSize, _sndSize - _udpOverhead) >= buf.size());

            int ret = 0;
            while(true)
            {
                try
                {
                    if(_state == StateConnected)
                    {
                        ret = _fd.Send(buf.b.rawBytes(), 0, buf.size(), SocketFlags.None);
                    }
                    else
                    {
                        if(_peerAddr == null)
                        {
                            throw new Ice.SocketException();
                        }
                        ret = _fd.SendTo(buf.b.rawBytes(), 0, buf.size(), SocketFlags.None, _peerAddr);
                    }
                    break;
                }
                catch(SocketException ex)
                {
                    if(Network.interrupted(ex))
                    {
                        continue;
                    }
                    
                    if(Network.wouldBlock(ex))
                    {
                        return false;
                    }

                    if(Network.connectionLost(ex))
                    {
                        throw new Ice.ConnectionLostException(ex);
                    }
                    else
                    {
                        throw new Ice.SocketException(ex);
                    }
                }
                catch(System.Exception e)
                {
                    throw new Ice.SyscallException(e);
                }
            }

            Debug.Assert(ret > 0);
            
            if(_traceLevels.network >= 3)
            {
                string s = "sent " + ret + " bytes via udp\n" + ToString();
                _logger.trace(_traceLevels.networkCat, s);
            }
            
            if(_stats != null)
            {
#pragma warning disable 618
                _stats.bytesSent(type(), ret);
#pragma warning restore 618
            }
            
            Debug.Assert(ret == buf.b.limit());

            return true;
#endif
        }

        public bool read(Buffer buf)
        {
#if COMPACT || SILVERLIGHT
            //
            // Silverlight and the Compact .NET Framework don't support the use of synchronous socket
            // operations on a non-blocking socket. Returning false here forces the
            // caller to schedule an asynchronous operation.
            //
            return false;
#else
            Debug.Assert(buf.b.position() == 0);
            Debug.Assert(_fd != null);

            int packetSize = System.Math.Min(_maxPacketSize, _rcvSize - _udpOverhead);
            buf.resize(packetSize, true);
            buf.b.position(0);

            int ret = 0;
            while(true)
            {
                try
                {
                    EndPoint peerAddr = _peerAddr;
                    if(peerAddr == null)
                    {
                        if(_addr.AddressFamily == AddressFamily.InterNetwork)
                        {
                            peerAddr = new IPEndPoint(IPAddress.Any, 0);
                        }
                        else
                        {
                            Debug.Assert(_addr.AddressFamily == AddressFamily.InterNetworkV6);
                            peerAddr = new IPEndPoint(IPAddress.IPv6Any, 0);
                        }
                    }

                    if(_state == StateConnected)
                    {
                        ret = _fd.Receive(buf.b.rawBytes(), 0, buf.b.limit(), SocketFlags.None);
                    }
                    else
                    {
                        ret = _fd.ReceiveFrom(buf.b.rawBytes(), 0, buf.b.limit(), SocketFlags.None, ref peerAddr);
                        _peerAddr = (IPEndPoint)peerAddr;
                    }
                    break;
                }
                catch(SocketException e)
                {
                    if(Network.recvTruncated(e))
                    {
                        // The message was truncated and the whole buffer is filled. We ignore 
                        // this error here, it will be detected at the connection level when
                        // the Ice message size is checked against the buffer size.
                        ret = buf.size();
                        break;
                    }

                    if(Network.interrupted(e))
                    {
                        continue;
                    }
                    
                    if(Network.wouldBlock(e))
                    {
                        return false;
                    }
                    
                    if(Network.connectionLost(e))
                    {
                        throw new Ice.ConnectionLostException();
                    }
                    else
                    {
                        throw new Ice.SocketException(e);
                    }
                }
                catch(System.Exception e)
                {
                    throw new Ice.SyscallException(e);
                }
            }
                
            if(ret == 0)
            {
                throw new Ice.ConnectionLostException();
            }

            if(_state == StateNeedConnect)
            {
                Debug.Assert(_incoming);

                //
                // If we must connect, then we connect to the first peer that sends us a packet.
                //
                bool connected = Network.doConnect(_fd, _peerAddr);
                Debug.Assert(connected);
                _state = StateConnected; // We're connected now

                if(_traceLevels.network >= 1)
                {
                    string s = "connected udp socket\n" + ToString();
                    _logger.trace(_traceLevels.networkCat, s);
                }
            }

            if(_traceLevels.network >= 3)
            {
                string s = "received " + ret + " bytes via udp\n" + ToString();
                _logger.trace(_traceLevels.networkCat, s);
            }

            if(_stats != null)
            {
#pragma warning disable 618
                _stats.bytesReceived(type(), ret);
#pragma warning restore 618
            }

            buf.resize(ret, true);
            buf.b.position(ret);

            return true;
#endif
        }

        public bool startRead(Buffer buf, AsyncCallback callback, object state)
        {
            Debug.Assert(buf.b.position() == 0);

            int packetSize = System.Math.Min(_maxPacketSize, _rcvSize - _udpOverhead);
            buf.resize(packetSize, true);
            buf.b.position(0);

            try
            {                
                if(_state == StateConnected)
                {
                    _readCallback = callback;
#if ICE_SOCKET_ASYNC_API
                    _readEventArgs.UserToken = state;
                    _readEventArgs.SetBuffer(buf.b.rawBytes(), buf.b.position(), packetSize);
                    return !_fd.ReceiveAsync(_readEventArgs);
#else
                    _readResult = _fd.BeginReceive(buf.b.rawBytes(), 0, buf.b.limit(), SocketFlags.None, 
                                                   readCompleted, state);
                    return _readResult.CompletedSynchronously;
#endif
                }
                else
                {
                    Debug.Assert(_incoming);
                    _readCallback = callback;
#if ICE_SOCKET_ASYNC_API
                    _readEventArgs.UserToken = state;
                    _readEventArgs.SetBuffer(buf.b.rawBytes(), 0, buf.b.limit());
                    return !_fd.ReceiveFromAsync(_readEventArgs);
#else
                    EndPoint peerAddr = _peerAddr;
                    if(peerAddr == null)
                    {
                        if(_addr.AddressFamily == AddressFamily.InterNetwork)
                        {
                            peerAddr = new IPEndPoint(IPAddress.Any, 0);
                        }
                        else
                        {
                            Debug.Assert(_addr.AddressFamily == AddressFamily.InterNetworkV6);
                            peerAddr = new IPEndPoint(IPAddress.IPv6Any, 0);
                        }
                    }
                    _readResult = _fd.BeginReceiveFrom(buf.b.rawBytes(), 0, buf.b.limit(), SocketFlags.None, 
                                                       ref peerAddr, readCompleted, state);
                    return _readResult.CompletedSynchronously;
#endif
                }
            }
            catch(SocketException ex)
            {
                if(Network.recvTruncated(ex))
                {
                    // Nothing todo
                    return true;
                }
                else
                {
                    if(Network.connectionLost(ex))
                    {
                        throw new Ice.ConnectionLostException(ex);
                    }
                    else
                    {
                        throw new Ice.SocketException(ex);
                    }
                }
            }
        }

        public void finishRead(Buffer buf)
        {
            if(_fd == null)
            {
                return;
            }

            int ret;
            try
            {
#if ICE_SOCKET_ASYNC_API
                if(_readEventArgs.SocketError != SocketError.Success)
                {
                    throw new SocketException((int)_readEventArgs.SocketError);
                }
                ret = _readEventArgs.BytesTransferred;
                if(_state != StateConnected)
                {
                    _peerAddr = _readEventArgs.RemoteEndPoint;
                }
#else
                Debug.Assert(_readResult != null);
                if(_state == StateConnected)
                {
                    ret = _fd.EndReceive(_readResult);
                }
                else
                {
                    EndPoint peerAddr = _peerAddr;
                    if(_addr.AddressFamily == AddressFamily.InterNetwork)
                    {
                        peerAddr = new IPEndPoint(IPAddress.Any, 0);
                    }
                    else
                    {
                        Debug.Assert(_addr.AddressFamily == AddressFamily.InterNetworkV6);
                        peerAddr = new IPEndPoint(IPAddress.IPv6Any, 0);
                    }
                    ret = _fd.EndReceiveFrom(_readResult, ref peerAddr);
                    _peerAddr = (IPEndPoint)peerAddr;
                }
                _readResult = null;
#endif
            }
            catch(SocketException ex)
            {
                if(Network.recvTruncated(ex))
                {
                    // The message was truncated and the whole buffer is filled. We ignore 
                    // this error here, it will be detected at the connection level when
                    // the Ice message size is checked against the buffer size.
                    ret = buf.size();
                }
                else
                {
                    if(Network.connectionLost(ex))
                    {
                        throw new Ice.ConnectionLostException(ex);
                    }
                    
                    if(Network.connectionRefused(ex))
                    {
                        throw new Ice.ConnectionRefusedException(ex);
                    }
                    else
                    {
                        throw new Ice.SocketException(ex);
                    }
                }
            }

            if(ret == 0)
            {
                throw new Ice.ConnectionLostException();
            }

            Debug.Assert(ret > 0);

            if(_state == StateNeedConnect)
            {
                Debug.Assert(_incoming);

                //
                // If we must connect, then we connect to the first peer that
                // sends us a packet.
                //
#if ICE_SOCKET_ASYNC_API
                bool connected = !_fd.ConnectAsync(_readEventArgs);
#else
                bool connected = Network.doConnect(_fd, _peerAddr);
#endif
                Debug.Assert(connected);
                _state = StateConnected; // We're connected now

                if(_traceLevels.network >= 1)
                {
                    string s = "connected udp socket\n" + ToString();
                    _logger.trace(_traceLevels.networkCat, s);
                }
            }

            if(_traceLevels.network >= 3)
            {
                string s = "received " + ret + " bytes via udp\n" + ToString();
                _logger.trace(_traceLevels.networkCat, s);
            }

            if(_stats != null)
            {
#pragma warning disable 618
                _stats.bytesReceived(type(), ret);
#pragma warning restore 618
            }

            buf.resize(ret, true);
            buf.b.position(ret);
        }

        public bool startWrite(Buffer buf, AsyncCallback callback, object state, out bool completed)
        {
            if(!_incoming && _state < StateConnected)
            {
                Debug.Assert(_addr != null);
                completed = false;
#if ICE_SOCKET_ASYNC_API
                _writeEventArgs.UserToken = state;
                return !_fd.ConnectAsync(_writeEventArgs);
#else
                _writeResult = Network.doConnectAsync(_fd, _addr, callback, state);
                return _writeResult.CompletedSynchronously;
#endif
            }

            Debug.Assert(_fd != null);

            // The caller is supposed to check the send size before by calling checkSendSize
            Debug.Assert(System.Math.Min(_maxPacketSize, _sndSize - _udpOverhead) >= buf.size());

            Debug.Assert(buf.b.position() == 0);

            bool completedSynchronously;
            try
            {
                _writeCallback = callback;

                if(_state == StateConnected)
                {
#if ICE_SOCKET_ASYNC_API
                    _writeEventArgs.UserToken = state;
                    _writeEventArgs.SetBuffer(buf.b.rawBytes(), 0, buf.b.limit());
                    completedSynchronously = !_fd.SendAsync(_writeEventArgs);
#else
                    _writeResult = _fd.BeginSend(buf.b.rawBytes(), 0, buf.b.limit(), SocketFlags.None, 
                                                 writeCompleted, state);
                    completedSynchronously = _writeResult.CompletedSynchronously;
#endif
                }
                else
                {
                    if(_peerAddr == null)
                    {
                        throw new Ice.SocketException();
                    }
#if ICE_SOCKET_ASYNC_API
                    _writeEventArgs.RemoteEndPoint = _peerAddr;
                    _writeEventArgs.UserToken = state;
                    _writeEventArgs.SetBuffer(buf.b.rawBytes(), 0, buf.b.limit());
                    completedSynchronously = !_fd.SendToAsync(_writeEventArgs);
#else
                    _writeResult = _fd.BeginSendTo(buf.b.rawBytes(), 0, buf.b.limit(), SocketFlags.None, _peerAddr,
                                                   writeCompleted, state);
                    completedSynchronously = _writeResult.CompletedSynchronously;
#endif
                }
            }
            catch(SocketException ex)
            {
                if(Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                else
                {
                    throw new Ice.SocketException(ex);
                }
            }

            completed = true;
            return completedSynchronously;
        }

        public void finishWrite(Buffer buf)
        {
            if(_fd == null)
            {
                buf.b.position(buf.size()); // Assume all the data was sent for at-most-once semantics.
#if ICE_SOCKET_ASYNC_API
                _writeEventArgs = null;
#else
                _writeResult = null;
#endif
                return;
            }

            if(!_incoming && _state < StateConnected)
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
                Debug.Assert(_writeResult != null);
                Network.doFinishConnectAsync(_fd, _writeResult);
                _writeResult = null;
#endif
                return;
            }

            int ret;
            try
            {
#if ICE_SOCKET_ASYNC_API
                if(_writeEventArgs.SocketError != SocketError.Success)
                {
                    throw new SocketException((int)_writeEventArgs.SocketError);
                }
                ret = _writeEventArgs.BytesTransferred;
#else
                if (_state == StateConnected)
                {
                    ret = _fd.EndSend(_writeResult);
                }
                else
                {
                    ret = _fd.EndSendTo(_writeResult);
                }
                _writeResult = null;
#endif
            }
            catch(SocketException ex)
            {
                if(Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                else
                {
                    throw new Ice.SocketException(ex);
                }
            }

            if(ret == 0)
            {
                throw new Ice.ConnectionLostException();
            }
            
            Debug.Assert(ret > 0);
                
            if(_traceLevels.network >= 3)
            {
                string s = "sent " + ret + " bytes via udp\n" + ToString();
                _logger.trace(_traceLevels.networkCat, s);
            }

            if(_stats != null)
            {
#pragma warning disable 618
                _stats.bytesSent(type(), ret);
#pragma warning restore 618
            }

            Debug.Assert(ret == buf.b.limit());
            buf.b.position(buf.b.position() + ret);
        }

        public string type()
        {
            return "udp";
        }

        public Ice.ConnectionInfo
        getInfo()
        {
            Ice.UDPConnectionInfo info = new Ice.UDPConnectionInfo();
            if(_fd != null)
            {
                EndPoint localEndpoint = Network.getLocalAddress(_fd);
                info.localAddress = Network.endpointAddressToString(localEndpoint);
                info.localPort = Network.endpointPort(localEndpoint);
                if(_state == StateNotConnected)
                {
                    if(_peerAddr != null)
                    {
                        info.remoteAddress = Network.endpointAddressToString(_peerAddr);
                        info.remotePort = Network.endpointPort(_peerAddr);
                    }
                }
                else
                {
                    EndPoint remoteEndpoint = Network.getRemoteAddress(_fd);
                    if(remoteEndpoint != null)
                    {
                        info.remoteAddress = Network.endpointAddressToString(remoteEndpoint);
                        info.remotePort = Network.endpointPort(remoteEndpoint);
                    }
                }
            }
#if !SILVERLIGHT
            if(_mcastAddr != null)
            {
                info.mcastAddress = Network.endpointAddressToString(_mcastAddr);
                info.mcastPort = Network.endpointPort(_mcastAddr);
            }
#endif
            return info;
        }

        public void checkSendSize(Buffer buf, int messageSizeMax)
        {
            if(buf.size() > messageSizeMax)
            {
                Ex.throwMemoryLimitException(buf.size(), messageSizeMax);
            }

            //
            // The maximum packetSize is either the maximum allowable UDP packet size, or 
            // the UDP send buffer size (which ever is smaller).
            //
            int packetSize = System.Math.Min(_maxPacketSize, _sndSize - _udpOverhead);
            if(packetSize < buf.size())
            {
                throw new Ice.DatagramLimitException();
            }
        }

        public override string ToString()
        {
            if(_fd == null)
            {
                return "<closed>";
            }

            string s;
            if(_state == StateNotConnected)
            {
                s = "local address = " + Network.localAddrToString(Network.getLocalAddress(_fd));
                if(_peerAddr != null)
                {
                    s += "\nremote address = " + Network.addrToString(_peerAddr);
                }
            }
            else
            {
                s = Network.fdToString(_fd);
            }

#if !SILVERLIGHT
            if(_mcastAddr != null)
            {
                s += "\nmulticast address = " + Network.addrToString(_mcastAddr);
            }   
#endif         
            return s;
        }

        public int effectivePort()
        {
            return Network.endpointPort(_addr);
        }

        //
        // Only for use by UdpConnector.
        //
        internal UdpTransceiver(Instance instance, EndPoint addr, string mcastInterface, int mcastTtl)
        {
            _traceLevels = instance.traceLevels();
            _logger = instance.initializationData().logger;
            _stats = instance.initializationData().stats;
            _addr = addr;

#if ICE_SOCKET_ASYNC_API
            _readEventArgs = new SocketAsyncEventArgs();
            _readEventArgs.RemoteEndPoint = _addr;
            _readEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(ioCompleted);

            _writeEventArgs = new SocketAsyncEventArgs();
            _writeEventArgs.RemoteEndPoint = _addr;
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

            _mcastInterface = mcastInterface;
            _mcastTtl = mcastTtl;
            _state = StateNeedConnect;
            _incoming = false;

            try
            {
                _fd = Network.createSocket(true, _addr.AddressFamily);
                setBufSize(instance);
#if !SILVERLIGHT
                Network.setBlock(_fd, false);
                if(AssemblyUtil.osx_)
                {
                    //
                    // On Windows, we delay the join for the mcast group after the connection
                    // establishment succeeds. This is necessary for older Windows versions 
                    // where joining the group fails if the socket isn't bound. See ICE-5113.
                    //
                    if(Network.isMulticast((IPEndPoint)_addr))
                    {
                        Network.setMcastGroup(_fd, ((IPEndPoint)_addr).Address, _mcastInterface);
                        if(_mcastTtl != -1)
                        {
                            Network.setMcastTtl(_fd, _mcastTtl, _addr.AddressFamily);
                        }
                    }
                }
#endif
            }
            catch(Ice.LocalException)
            {
                _fd = null;
                throw;
            }
        }

        //
        // Only for use by UdpEndpoint.
        //
        internal UdpTransceiver(Instance instance, string host, int port, string mcastInterface, bool connect)
        {
            _traceLevels = instance.traceLevels();
            _logger = instance.initializationData().logger;
            _stats = instance.initializationData().stats;
            _state = connect ? StateNeedConnect : StateNotConnected;
            _incoming = true;
            
            try
            {
                _addr = Network.getAddressForServer(host, port, instance.protocolSupport(), instance.preferIPv6());

#if ICE_SOCKET_ASYNC_API
                _readEventArgs = new SocketAsyncEventArgs();
                _readEventArgs.RemoteEndPoint = _addr;
                _readEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(ioCompleted);
                
                _writeEventArgs = new SocketAsyncEventArgs();
                _writeEventArgs.RemoteEndPoint = _addr;
                _writeEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(ioCompleted);
#endif

                _fd = Network.createServerSocket(true, _addr.AddressFamily, instance.protocolSupport());
                setBufSize(instance);
#if !SILVERLIGHT
                Network.setBlock(_fd, false);
#endif
                if(_traceLevels.network >= 2)
                {
                    string s = "attempting to bind to udp socket " + Network.addrToString(_addr);
                    _logger.trace(_traceLevels.networkCat, s);
                }
                
#if !SILVERLIGHT
                if(Network.isMulticast((IPEndPoint)_addr))
                {
                    Network.setReuseAddress(_fd, true);
                    _mcastAddr = (IPEndPoint)_addr;
                    if(AssemblyUtil.platform_ == AssemblyUtil.Platform.Windows)
                    {
                        //
                        // Windows does not allow binding to the mcast address itself
                        // so we bind to INADDR_ANY (0.0.0.0) instead. As a result,
                        // bi-directional connection won't work because the source 
                        // address won't the multicast address and the client will
                        // therefore reject the datagram.
                        //
                        if(_addr.AddressFamily == AddressFamily.InterNetwork)
                        {
                            _addr = new IPEndPoint(IPAddress.Any, port);
                        }
                        else
                        {
                            _addr = new IPEndPoint(IPAddress.IPv6Any, port);
                        }
                    }
                    _addr = Network.doBind(_fd, _addr);
                    if(port == 0)
                    {
                        _mcastAddr.Port = ((IPEndPoint)_addr).Port;
                    }
                    Network.setMcastGroup(_fd, _mcastAddr.Address, mcastInterface);
                }
                else
                {
                    if(AssemblyUtil.platform_ != AssemblyUtil.Platform.Windows)
                    {
                        //
                        // Enable SO_REUSEADDR on Unix platforms to allow
                        // re-using the socket even if it's in the TIME_WAIT
                        // state. On Windows, this doesn't appear to be
                        // necessary and enabling SO_REUSEADDR would actually
                        // not be a good thing since it allows a second
                        // process to bind to an address even it's already
                        // bound by another process.
                        //
                        // TODO: using SO_EXCLUSIVEADDRUSE on Windows would
                        // probably be better but it's only supported by recent
                        // Windows versions (XP SP2, Windows Server 2003).
                        //
                        Network.setReuseAddress(_fd, true);
                    }
                    _addr = Network.doBind(_fd, _addr);
                }
#endif
                if(_traceLevels.network >= 1)
                {
                    StringBuilder s = new StringBuilder("starting to receive udp packets\n");
                    s.Append(ToString());
                    List<string> interfaces = Network.getHostsForEndpointExpand(
                        Network.endpointAddressToString(_addr),  instance.protocolSupport(), true);
                    if(interfaces.Count != 0)
                    {
                        s.Append("\nlocal interfaces: ");
                        s.Append(String.Join(", ", interfaces.ToArray()));
                    }
                    _logger.trace(_traceLevels.networkCat, s.ToString());
                }
            }
            catch(Ice.LocalException)
            {
#if ICE_SOCKET_ASYNC_API
                if(_readEventArgs != null)
                {
                    _readEventArgs.Dispose();
                }
                if(_writeEventArgs != null)
                {
                    _writeEventArgs.Dispose();
                }
#endif
                _fd = null;
                throw;
            }
        }

        private void setBufSize(Instance instance)
        {
            Debug.Assert(_fd != null);

            for (int i = 0; i < 2; ++i)
            {
                string direction;
                string prop;
                int dfltSize;
                if(i == 0)
                {
                    direction = "receive";
                    prop = "Ice.UDP.RcvSize";
                    dfltSize = Network.getRecvBufferSize(_fd);
                    _rcvSize = dfltSize;
                }
                else
                {
                    direction = "send";
                    prop = "Ice.UDP.SndSize";
                    dfltSize = Network.getSendBufferSize(_fd);
                    _sndSize = dfltSize;
                }

                //
                // Get property for buffer size and check for sanity.
                //
                int sizeRequested =
                    instance.initializationData().properties.getPropertyAsIntWithDefault(prop, dfltSize);
                if(sizeRequested < (_udpOverhead + IceInternal.Protocol.headerSize))
                {
                    _logger.warning("Invalid " + prop + " value of " + sizeRequested + " adjusted to " + dfltSize);
                    sizeRequested = dfltSize;
                }

                if(sizeRequested != dfltSize)
                {
                    //
                    // Try to set the buffer size. The kernel will silently adjust
                    // the size to an acceptable value. Then read the size back to
                    // get the size that was actually set.
                    //
                    int sizeSet;
                    if(i == 0)
                    {
                        Network.setRecvBufferSize(_fd, sizeRequested);
                        _rcvSize = Network.getRecvBufferSize(_fd);
                        sizeSet = _rcvSize;
                    }
                    else
                    {
                        Network.setSendBufferSize(_fd, sizeRequested);
                        _sndSize = Network.getSendBufferSize(_fd);
                        sizeSet = _sndSize;
                    }

                    //
                    // Warn if the size that was set is less than the requested size.
                    //
                    if(sizeSet < sizeRequested)
                    {
                        _logger.warning("UDP " + direction + " buffer size: requested size of " + sizeRequested +
                                        " adjusted to " + sizeSet);
                    }
                }
            }
        }

#if ICE_SOCKET_ASYNC_API
        internal void ioCompleted(object sender, SocketAsyncEventArgs e)
        {
            switch (e.LastOperation)
            {
            case SocketAsyncOperation.Receive:
#if !SILVERLIGHT
            case SocketAsyncOperation.ReceiveFrom:
#endif
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

        private TraceLevels _traceLevels;
        private Ice.Logger _logger;
        private Ice.Stats _stats;
        private int _state;
        private bool _incoming;
        private int _rcvSize;
        private int _sndSize;
        private Socket _fd;
        private EndPoint _addr;
#if !SILVERLIGHT
        private IPEndPoint _mcastAddr = null;
#endif
        private EndPoint _peerAddr = null;
        private string _mcastInterface = null;
        private int _mcastTtl = -1;

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
        private const int StateConnected = 2;
        private const int StateNotConnected = 3;

        //
        // The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for the UDP header
        // to get the maximum payload.
        //
        private const int _udpOverhead = 20 + 8;
        private const int _maxPacketSize = 65535 - _udpOverhead;
    }
}
