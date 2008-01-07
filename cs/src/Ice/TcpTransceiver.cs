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

    using System.Collections;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Net.Sockets;

    sealed class TcpTransceiver : Transceiver
    {
        public Socket fd()
        {
            Debug.Assert(_fd != null);
            return _fd;
        }

        public SocketStatus initialize(int timeout)
        {
            if(_state == StateNeedConnect && timeout == 0)
            {
                _state = StateConnectPending;
                return SocketStatus.NeedConnect;
            }
            else if(_state <= StateConnectPending)
            {
                try
                {
                    Network.doFinishConnect(_fd, timeout);
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
                    throw ex;
                }
                
                if(_traceLevels.network >= 1)
                {
                    string s = "tcp connection established\n" + _desc;
                    _logger.trace(_traceLevels.networkCat, s);
                }
            }
            Debug.Assert(_state == StateConnected);
            return SocketStatus.Finished;
        }

        public void close()
        {
            if(_traceLevels.network >= 1)
            {
                string s = "closing tcp connection\n" + ToString();
                _logger.trace(_traceLevels.networkCat, s);
            }

            lock(this)
            {
                Debug.Assert(_fd != null);
                try
                {
                    _fd.Close();
                }
                catch(System.IO.IOException ex)
                {
                    throw new Ice.SocketException(ex);
                }
                finally
                {
                    _fd = null;
                }
            }
        }

        public void shutdownWrite()
        {
            if(_state < StateConnected)
            {           
                return;
            }

            if(_traceLevels.network >= 2)
            {
                string s = "shutting down tcp connection for writing\n" + ToString();
                _logger.trace(_traceLevels.networkCat, s);
            }

            Debug.Assert(_fd != null);
            try
            {
                _fd.Shutdown(SocketShutdown.Send);
            }
            catch(SocketException ex)
            {
                if(Network.notConnected(ex))
                {
                    return;
                }
                throw new Ice.SocketException(ex);
            }
        }

        public void shutdownReadWrite()
        {
            if(_state < StateConnected)
            {           
                return;
            }

            if(_traceLevels.network >= 2)
            {
                string s = "shutting down tcp connection for reading and writing\n" + ToString();
                _logger.trace(_traceLevels.networkCat, s);
            }

            Debug.Assert(_fd != null);
            try
            {
                _fd.Shutdown(SocketShutdown.Both);
            }
            catch(SocketException ex)
            {
                if(Network.notConnected(ex))
                {
                    return;
                }
                throw new Ice.SocketException(ex);
            }
        }

        public bool write(Buffer buf, int timeout)
        {
            while(writeBuffer(buf.b))
            {
                //
                // There is more data to write but the socket would block; now we
                // must deal with timeouts.
                //
                Debug.Assert(buf.b.hasRemaining());

                if(timeout == 0)
                {
                    return false;
                }

                if(!Network.doPoll(_fd, timeout, Network.PollMode.Write))
                {
                    throw new Ice.TimeoutException();
                }
            }
            return true;
        }

        public bool read(Buffer buf, int timeout)
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
                            if(timeout == 0)
                            {
                                return false;
                            }

                            if(!Network.doPoll(_fd, timeout, Network.PollMode.Read))
                            {
                                throw new Ice.TimeoutException();
                            }

                            continue;
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

        public string type()
        {
            return "tcp";
        }

        public override string ToString()
        {
            return _desc;
        }

        public void checkSendSize(Buffer buf, int messageSizeMax)
        {
            if(buf.size() > messageSizeMax)
            {
                throw new Ice.MemoryLimitException();
            }
        }

        //
        // Only for use by TcpConnector, TcpAcceptor
        //
        internal TcpTransceiver(Instance instance, Socket fd, bool connected)
        {
            _fd = fd;
            _traceLevels = instance.traceLevels();
            _logger = instance.initializationData().logger;
            _stats = instance.initializationData().stats;
            _state = connected ? StateConnected : StateNeedConnect;
            _desc = Network.fdToString(_fd);

            _maxPacketSize = 0;
            if(AssemblyUtil.platform_ == AssemblyUtil.Platform.Windows)
            {
		//
		// On Windows, limiting the buffer size is important to prevent
		// poor throughput performances when transfering large amount of
		// data. See Microsoft KB article KB823764.
		//
                _maxPacketSize = Network.getSendBufferSize(fd) / 2;
                if(_maxPacketSize < 512)
                {
                    _maxPacketSize = 0;
                }
            }
        }

        private bool writeBuffer(ByteBuffer buf)
        {
            int size = buf.limit();
            int packetSize = size - buf.position();
            if(_maxPacketSize > 0 && packetSize > _maxPacketSize)
            {
                packetSize = _maxPacketSize;
                buf.limit(buf.position() + packetSize);
            }

            while(buf.hasRemaining())
            {
                try
                {
                    Debug.Assert(_fd != null);

                    int ret;
                    try
                    {
                        //
                        // Try to send first. Most of the time, this will work and
                        // avoids the cost of calling Poll().
                        //
                        ret = _fd.Send(buf.rawBytes(), buf.position(), buf.remaining(), SocketFlags.None);
                    }
                    catch(Win32Exception e)
                    {
                        if(Network.wouldBlock(e))
                        {
                            // 
                            // Writing would block, so we reset the limit (if necessary) and return true to indicate
                            // that more data must be sent.
                            //
                            if(packetSize == _maxPacketSize)
                            {   
                                buf.limit(size);
                            }
                            return true;
                        }
                        throw;
                    }

                    Debug.Assert(ret > 0);

                    if(_traceLevels.network >= 3)
                    {
                        string s = "sent " + ret + " of " + buf.remaining() + " bytes via tcp\n" + ToString();
                        _logger.trace(_traceLevels.networkCat, s);
                    }

                    if(_stats != null)
                    {
                        _stats.bytesSent(type(), ret);
                    }

                    buf.position(buf.position() + ret);

                    if(packetSize == _maxPacketSize)
                    {
                        Debug.Assert(buf.position() == buf.limit());
                        packetSize = size - buf.position();
                        if(packetSize > _maxPacketSize)
                        {
                            packetSize = _maxPacketSize;
                        }
                        buf.limit(buf.position() + packetSize);
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

            return false; // No more data to send.
        }

        private Socket _fd;
        private TraceLevels _traceLevels;
        private Ice.Logger _logger;
        private Ice.Stats _stats;
        private string _desc;
        private int _state;
        private int _maxPacketSize;

        private const int StateNeedConnect = 0;
        private const int StateConnectPending = 1;
        private const int StateConnected = 2;
    }

}
