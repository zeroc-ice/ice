// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
            if(_state == StateNeedConnect)
            {
                _state = StateConnectPending;
                return SocketOperation.Connect;
            }
            else if(_state <= StateConnectPending)
            {
                try
                {
                    Network.doFinishConnectAsync(_fd, _writeResult);
                    _writeResult = null;
                    _state = StateConnected;
                    _desc = Network.fdToString(_fd);
                }
                catch(Ice.LocalException ex)
                {
                    if(_traceLevels.network >= 2)
                    {
                        string s = "failed to establish tcp connection\n" + _desc + "\n" + ex;
                        _logger.trace(_traceLevels.networkCat, s);
                    }
                    throw;
                }

                if(_traceLevels.network >= 1)
                {
                    string s = "tcp connection established\n" + _desc;
                    _logger.trace(_traceLevels.networkCat, s);
                }
            }
            Debug.Assert(_state == StateConnected);
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

        public bool write(Buffer buf)
        {
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
                    catch(Win32Exception e)
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
                        _stats.bytesSent(type(), ret);
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
        }

        public bool read(Buffer buf)
        {
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
                    catch(Win32Exception e)
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
                        _stats.bytesReceived(type(), ret);
                    }

                    remaining -= ret;
                    buf.b.position(position += ret);
                }
                catch(Win32Exception ex)
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
        }

        public bool startRead(Buffer buf, AsyncCallback callback, object state)
        {
            Debug.Assert(_fd != null && _readResult == null);

            int packetSize = buf.b.remaining();
            if(_maxReceivePacketSize > 0 && packetSize > _maxReceivePacketSize)
            {
                packetSize = _maxReceivePacketSize;
            }

            try
            {
                _readResult = _fd.BeginReceive(buf.b.rawBytes(), buf.b.position(), packetSize, SocketFlags.None, 
                                               callback, state);
            }
            catch(Win32Exception ex)
            {
                if(Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }

                throw new Ice.SocketException(ex);
            }

            return _readResult.CompletedSynchronously;
        }

        public void finishRead(Buffer buf)
        {
            if(_fd == null) // Transceiver was closed
            {
                _readResult = null;
                return;
            }

            Debug.Assert(_fd != null && _readResult != null);

            try
            {
                int ret = _fd.EndReceive(_readResult);
                _readResult = null;
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
                    _stats.bytesReceived(type(), ret);
                }

                buf.b.position(buf.b.position() + ret);
            }
            catch(Win32Exception ex)
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

        public bool startWrite(Buffer buf, AsyncCallback callback, object state)
        {
            Debug.Assert(_fd != null && _writeResult == null);

            if(_state < StateConnected)
            {
                _writeResult = Network.doConnectAsync(_fd, _addr, callback, state);
                return _writeResult.CompletedSynchronously;
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
                _writeResult = _fd.BeginSend(buf.b.rawBytes(), buf.b.position(), packetSize, SocketFlags.None, 
                                             callback, state);
            }
            catch(Win32Exception ex)
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

            return _writeResult.CompletedSynchronously;
        }

        public void finishWrite(Buffer buf)
        {
            if(_fd == null) // Transceiver was closed
            {
                if(buf.size() - buf.b.position() < _maxSendPacketSize)
                {
                    buf.b.position(buf.size()); // Assume all the data was sent for at-most-once semantics.
                }
                _writeResult = null;
                return;
            }

            Debug.Assert(_fd != null && _writeResult != null);

            if(_state < StateConnected)
            {
                return;
            }

            try
            {
                int ret = _fd.EndSend(_writeResult);
                _writeResult = null;
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
                    _stats.bytesSent(type(), ret);
                }

                buf.b.position(buf.b.position() + ret);
            }
            catch(Win32Exception ex)
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
            Debug.Assert(_fd != null);
            Ice.TCPConnectionInfo info = new Ice.TCPConnectionInfo();
            IPEndPoint localEndpoint = Network.getLocalAddress(_fd);
            info.localAddress = localEndpoint.Address.ToString();
            info.localPort = localEndpoint.Port;
            IPEndPoint remoteEndpoint = Network.getRemoteAddress(_fd);
            if(remoteEndpoint != null)
            {
                info.remoteAddress = remoteEndpoint.Address.ToString();
                info.remotePort = remoteEndpoint.Port;
            }
            else
            {
                info.remoteAddress = "";
                info.remotePort = -1;
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
        internal TcpTransceiver(Instance instance, Socket fd, IPEndPoint addr, bool connected)
        {
            _fd = fd;
            _addr = addr;
            _traceLevels = instance.traceLevels();
            _logger = instance.initializationData().logger;
            _stats = instance.initializationData().stats;
            _state = connected ? StateConnected : StateNeedConnect;
            _desc = connected ? Network.fdToString(_fd) : "<not connected>";

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

        private Socket _fd;
        private IPEndPoint _addr;
        private TraceLevels _traceLevels;
        private Ice.Logger _logger;
        private Ice.Stats _stats;
        private string _desc;
        private int _state;
        private int _maxSendPacketSize;
        private int _maxReceivePacketSize;

        private IAsyncResult _writeResult;
        private IAsyncResult _readResult;

        private const int StateNeedConnect = 0;
        private const int StateConnectPending = 1;
        private const int StateConnected = 2;
    }

}
