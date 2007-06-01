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
    using System.Threading;

    sealed class UdpTransceiver : Transceiver
    {
        public Socket fd()
        {
            Debug.Assert(_fd != null);
            return _fd;
        }
        
        public void close()
        {
            lock(this)
            {
                if(_traceLevels.network >= 1)
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
        }

        public void shutdownWrite()
        {
        }

        public void shutdownReadWrite()
        {
            if(_traceLevels.network >= 2)
            {
                string s = "shutting down udp connection for reading and writing\n" + ToString();
                _logger.trace(_traceLevels.networkCat, s);
            }

            //
            // Set a flag and then shutdown the socket in order to wake a thread that is
            // blocked in read().
            //
            lock(_shutdownReadWriteMutex)
            {
                _shutdownReadWrite = true;

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

                if(AssemblyUtil.platform_ == AssemblyUtil.Platform.Windows)
                {
                    //
                    // On Windows, shutting down the socket doesn't unblock a call to
                    // receive or select. Setting an event wakes up the thread in
                    // read().
                    //
                    _shutdownReadWriteEvent.Set();
                }
            }
        }
        
        public void write(BasicStream stream, int timeout)
        {
            Debug.Assert(_fd != null);

            ByteBuffer buf = stream.prepareWrite();
            
            Debug.Assert(buf.position() == 0);
            int packetSize = System.Math.Min(_maxPacketSize, _sndSize - _udpOverhead);
            if(packetSize < buf.limit())
            {
                //
                // We don't log a warning here because the client gets an exception anyway.
                //
                throw new Ice.DatagramLimitException();
            }

            try
            {   
                int remaining = buf.remaining();
                int ret;
                try
                {
                    ret = _fd.Send(buf.rawBytes(), 0, remaining, SocketFlags.None);
                }
                catch(Win32Exception e)
                {
                    if(Network.wouldBlock(e))
                    {
                        if(timeout == 0)
                        {
                            throw new Ice.TimeoutException();
                        }
                        ret = 0;
                    }
                    else
                    {
                        throw;
                    }
                }    
                if(ret == 0)
                {
                    if(!Network.doPoll(_fd, timeout, Network.PollMode.Write))
                    {
                        throw new Ice.TimeoutException();
                    }
                    ret = _fd.Send(buf.rawBytes(), 0, remaining, SocketFlags.None);
                }
                if(ret != remaining)
                {
                    throw new Ice.DatagramLimitException();
                }

                if(_traceLevels.network >= 3)
                {
                    string s = "sent " + ret + " bytes via udp\n" + ToString();
                    _logger.trace(_traceLevels.networkCat, s);
                }
                
                if(_stats != null)
                {
                    _stats.bytesSent(type(), ret);
                }

                buf.position(remaining);
            }
            catch(SocketException ex)
            {
                throw new Ice.SocketException(ex);
            }
            catch(Ice.LocalException)
            {
                throw;
            }
            catch(System.Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }
        }

        public void read(BasicStream stream, int timeout)
        {
            Debug.Assert(stream.pos() == 0);

            int packetSize = System.Math.Min(_maxPacketSize, _rcvSize - _udpOverhead);
            if(packetSize < stream.size())
            {
                //
                // We log a warning here because this is the server side -- without the
                // the warning, there would only be silence.
                //
                if(_warn)
                {
                    _logger.warning("DatagramLimitException: maximum size of " + packetSize + " exceeded");
                }
                throw new Ice.DatagramLimitException();
            }
            stream.resize(packetSize, true);
            ByteBuffer buf = stream.prepareRead();
            buf.position(0);

            if(AssemblyUtil.platform_ == AssemblyUtil.Platform.NonWindows)
            {

        repeat:

                //
                // Check the shutdown flag.
                //
                lock(_shutdownReadWriteMutex)
                {
                    if(_shutdownReadWrite)
                    {
                        throw new Ice.ConnectionLostException();
                    }
                }

                try
                {
                    int ret;
                    if(_connect)
                    {
                        //
                        // If we must connect, then we connect to the first peer that
                        // sends us a packet.
                        //
                        EndPoint peerAddr = new IPEndPoint(IPAddress.Any, 0);
                        ret = _fd.ReceiveFrom(buf.rawBytes(), 0, buf.limit(), SocketFlags.None, ref peerAddr);

                        Network.doConnect(_fd, peerAddr, -1);
                        _connect = false; // We're connected now

                        if(_traceLevels.network >= 1)
                        {
                            string s = "connected udp socket\n" + ToString();
                            _logger.trace(_traceLevels.networkCat, s);
                        }
                    }
                    else
                    {
                        Debug.Assert(_fd != null);
                        ret = _fd.Receive(buf.rawBytes(), 0, buf.limit(), SocketFlags.None);
                    }

                    if(_traceLevels.network >= 3)
                    {
                        string s = "received " + ret + " bytes via udp\n" + ToString();
                        _logger.trace(_traceLevels.networkCat, s);
                    }

                    if(_stats != null)
                    {
                        _stats.bytesReceived(type(), ret);
                    }

                    stream.resize(ret, true);
                }
                catch(Win32Exception e)
                {
                    if(Network.interrupted(e))
                    {
                        goto repeat;
                    }

                    if(Network.wouldBlock(e))
                    {
                    repeatPoll:
                        try
                        {
                            Network.doPoll(_fd, -1, Network.PollMode.Read);
                        }
                        catch(Win32Exception we)
                        {
                            if(Network.interrupted(we))
                            {
                                goto repeatPoll;
                            }
                            throw new Ice.SocketException(we);
                        }
                        catch(System.Exception se)
                        {
                            throw new Ice.SyscallException(se);
                        }

                        goto repeat;
                    }

                    if(Network.recvTruncated(e))
                    {
                        if(_warn)
                        {
                            _logger.warning("DatagramLimitException: maximum size of " + packetSize + " exceeded");
                        }
                        throw new Ice.DatagramLimitException();
                    }

                    if(Network.connectionLost(e))
                    {
                        throw new Ice.ConnectionLostException();
                    }

                    if(Network.connectionRefused(e))
                    {
                        throw new Ice.ConnectionRefusedException();
                    }

                    throw new Ice.SocketException(e);
                }
                catch(Ice.LocalException)
                {
                    throw;
                }
                catch(System.Exception e)
                {
                    throw new Ice.SyscallException(e);
                }
            }
            else
            {
                //
                // On Windows, we use asynchronous I/O to properly handle a call to
                // shutdownReadWrite. After calling BeginReceiveFrom, we wait for
                // the receive to complete or for the _shutdownReadWriteEvent to be
                // signaled.
                //

                WaitHandle[] handles = new WaitHandle[2];
                handles[0] = _shutdownReadWriteEvent;

                EndPoint peerAddr = new IPEndPoint(IPAddress.Any, 0);

                //
                // Check the shutdown flag.
                //
                lock(_shutdownReadWriteMutex)
                {
                    if(_shutdownReadWrite)
                    {
                        throw new Ice.ConnectionLostException();
                    }
                }

                try
                {
                    IAsyncResult ar = _fd.BeginReceiveFrom(buf.rawBytes(), 0, buf.limit(), SocketFlags.None,
                                                           ref peerAddr, null, null);
                    handles[1] = ar.AsyncWaitHandle;
                    int num = WaitHandle.WaitAny(handles);
                    if(num == 0)
                    {
                        //
                        // shutdownReadWrite was called.
                        //
                        throw new Ice.ConnectionLostException();
                    }
                    else
                    {
                        int ret = _fd.EndReceiveFrom(ar, ref peerAddr);

                        if(_connect)
                        {
                            //
                            // If we must connect, then we connect to the first peer that
                            // sends us a packet.
                            //
                            Network.doConnect(_fd, peerAddr, -1);
                            _connect = false; // We're connected now

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
                            _stats.bytesReceived(type(), ret);
                        }

                        stream.resize(ret, true);
                    }
                }
                catch(Win32Exception e)
                {
                    if(Network.recvTruncated(e))
                    {
                        if(_warn)
                        {
                            _logger.warning("DatagramLimitException: maximum size of " + packetSize + " exceeded");
                        }
                        throw new Ice.DatagramLimitException();
                    }

                    if(Network.connectionLost(e))
                    {
                        throw new Ice.ConnectionLostException();
                    }

                    if(Network.connectionRefused(e))
                    {
                        throw new Ice.ConnectionRefusedException();
                    }

                    throw new Ice.SocketException(e);
                }
                catch(Ice.LocalException)
                {
                    throw;
                }
                catch(System.Exception e)
                {
                    throw new Ice.SyscallException(e);
                }
            }
        }

        public string type()
        {
            return "udp";
        }

        public void initialize(int timeout)
        {
        }

        public void checkSendSize(BasicStream stream, int messageSizeMax)
        {
            if(stream.size() > messageSizeMax)
            {
                throw new Ice.MemoryLimitException();
            }
            int packetSize = System.Math.Min(_maxPacketSize, _sndSize - _udpOverhead);
            if(packetSize < stream.size())
            {
                throw new Ice.DatagramLimitException();
            }
        }

        public override string ToString()
        {
            if(mcastServer && _fd != null)
            {
                return Network.addressesToString(_addr, Network.getRemoteAddress(_fd));
            }
            else
            {
                return Network.fdToString(_fd);
            }
        }
        
        public bool equivalent(string host, int port)
        {
            IPEndPoint addr = ((IPEndPoint)Network.getAddress(host, port));
            return addr.Equals(_addr);
        }
        
        public int effectivePort()
        {
            return _addr.Port;
        }
        
        //
        // Only for use by UdpEndpoint
        //
        internal UdpTransceiver(Instance instance, IPEndPoint addr, string mcastInterface, int mcastTtl)
        {
            _traceLevels = instance.traceLevels();
            _logger = instance.initializationData().logger;
            _stats = instance.initializationData().stats;
            _connect = true;
            _warn = instance.initializationData().properties.getPropertyAsInt("Ice.Warn.Datagrams") > 0;
            _addr = addr;
            
            try
            {
                _fd = Network.createSocket(true);
                setBufSize(instance);
                Network.setBlock(_fd, false);
                Network.doConnect(_fd, _addr, -1);
                _connect = false; // We're connected now
                if(Network.isMulticast(_addr))
                {
                    IPAddress ip = IPAddress.Any;
                    if(mcastInterface.Length != 0)
                    {
                        ip = Network.getAddress(mcastInterface, _addr.Port).Address;
                    }
                    Network.setMcastGroup(_fd, _addr.Address, ip);

                    if(mcastTtl != -1)
                    {
                        Network.setMcastTtl(_fd, mcastTtl);
                    }
                }
                
                if(_traceLevels.network >= 1)
                {
                    string s = "starting to send udp packets\n" + ToString();
                    _logger.trace(_traceLevels.networkCat, s);
                }
            }
            catch(Ice.LocalException)
            {
                _fd = null;
                throw;
            }
        }
        
        //
        // Only for use by UdpEndpoint
        //
        internal UdpTransceiver(Instance instance, string host, int port, string mcastInterface, bool connect)
        {
            _traceLevels = instance.traceLevels();
            _logger = instance.initializationData().logger;
            _stats = instance.initializationData().stats;
            _connect = connect;
            _warn = instance.initializationData().properties.getPropertyAsInt("Ice.Warn.Datagrams") > 0;
            _shutdownReadWrite = false;
            
            try
            {
                _fd = Network.createSocket(true);
                setBufSize(instance);
                Network.setBlock(_fd, false);
                _addr = Network.getAddress(host, port);
                if(_traceLevels.network >= 2)
                {
                    string s = "attempting to bind to udp socket " + Network.addrToString(_addr);
                    _logger.trace(_traceLevels.networkCat, s);
                }
                if(Network.isMulticast(_addr))
                {
                    Network.setReuseAddress(_fd, true);
                    Network.doBind(_fd, Network.getAddress("0.0.0.0", port));
                    IPAddress addr;
                    if(mcastInterface.Length != 0)
                    {
                        addr = Network.getAddress(mcastInterface, port).Address;
                    }
                    else
                    {
                        addr = IPAddress.Any;
                    }
                    Network.setMcastGroup(_fd, _addr.Address, addr);
                    mcastServer = true;
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
                
                if(_traceLevels.network >= 1)
                {
                    string s = "starting to receive udp packets\n" + ToString();
                    _logger.trace(_traceLevels.networkCat, s);
                }
            }
            catch(Ice.LocalException)
            {
                _fd = null;
                throw;
            }
        }
        
        private void setBufSize(Instance instance)
        {
            lock(this)
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
                    if(sizeRequested < _udpOverhead)
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
        }

        private TraceLevels _traceLevels;
        private Ice.Logger _logger;
        private Ice.Stats _stats;
        private bool _connect;
        private readonly bool _warn;
        private int _rcvSize;
        private int _sndSize;
        private Socket _fd;
        private IPEndPoint _addr;
        private bool mcastServer = false;
        
        //
        // The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for the UDP header
        // to get the maximum payload.
        //
        private const int _udpOverhead = 20 + 8;
        private static readonly int _maxPacketSize = 65535 - _udpOverhead;

        private bool _shutdownReadWrite;
        private object _shutdownReadWriteMutex = new object();
        private AutoResetEvent _shutdownReadWriteEvent = new AutoResetEvent(false);
    }
}
