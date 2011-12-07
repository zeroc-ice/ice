// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
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
                        System.Text.StringBuilder s = new System.Text.StringBuilder();
                        s.Append("failed to establish tcp connection\n");
                        s.Append(Network.fdLocalAddressToString(_fd));
                        Debug.Assert(_addr != null);
                        s.Append("\nremote address = " + _addr.ToString() + "\n");
                        s.Append(ex.ToString());
                        _logger.trace(_traceLevels.networkCat, s.ToString());
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
#if COMPACT
            //
            // The Compact Framework does not support the use of synchronous socket
            // operations on a non-blocking socket, so we start an asynchronous
            // send here.
            //

            Debug.Assert(_fd != null && _writeResult == null && _state == StateConnected);

            //
            // We limit the packet size for writing to ensure connection timeouts are based
            // on a fixed packet size.
            //
            int packetSize = buf.b.remaining();
            if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize)
            {
                packetSize = _maxSendPacketSize;
            }

            IAsyncResult r = null;

            lock(this) // A monitor is not necessary here.
            {
                while(buf.b.hasRemaining())
                {
                    try
                    {
                        r = _fd.BeginSend(buf.b.rawBytes(), buf.b.position(), packetSize, SocketFlags.None,
                                          writeCallback, null);
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

                    if(r.CompletedSynchronously)
                    {
                        try
                        {
                            int ret = _fd.EndSend(r);
                            if(ret == 0)
                            {
                                throw new Ice.ConnectionLostException();
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
                    else
                    {
                        _writeResult = r;
                        return false;
                    }
                }
            }

            return true; // No more data to send.
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
            else
            {
                if(_blocking > 0)
                {
                    return false;
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
#endif
        }

        public bool read(Buffer buf)
        {
#if COMPACT
            //
            // The .NET Compact Framework does not support the use of synchronous socket
            // operations on a non-blocking socket, so we use asynchronous reads instead.
            //

            IAsyncResult r = null;

            lock(this) // A monitor is not necessary here.
            {
                while(buf.b.hasRemaining())
                {
                    int packetSize = buf.b.remaining();
                    if(_maxReceivePacketSize > 0 && packetSize > _maxReceivePacketSize)
                    {
                        packetSize = _maxReceivePacketSize;
                    }

                    Debug.Assert(_fd != null);

                    try
                    {
                        r = _fd.BeginReceive(buf.b.rawBytes(), buf.b.position(), packetSize, SocketFlags.None, 
                                             readCallback, null);
                    }
                    catch(Win32Exception ex)
                    {
                        if(Network.connectionLost(ex))
                        {
                            throw new Ice.ConnectionLostException(ex);
                        }

                        throw new Ice.SocketException(ex);
                    }

                    if(r.CompletedSynchronously)
                    {
                        try
                        {
                            int ret = _fd.EndReceive(r);
                            if(ret == 0)
                            {
                                throw new Ice.ConnectionLostException();
                            }

                            Debug.Assert(ret > 0);

                            if(_traceLevels.network >= 3)
                            {
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
                    else
                    {
                        _readResult = r;
                        return false;
                    }
                }
            }

            return true;
#else
            // COMPILERFIX: Workaround for Mac OS X broken poll(), see Mono bug #470120
            if(AssemblyUtil.osx_)
            {
                if(_blocking > 0)
                {
                    return false;
                }
            }

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
#endif
        }

#if COMPACT
        private void writeCallback(IAsyncResult r)
        {
            //
            // Nothing to do if BeginSend completed synchronously.
            //
            if(r.CompletedSynchronously)
            {
                return;
            }

            AsyncCallback cb = null;
            object state = null;

            lock(this) // A monitor is not necessary here.
            {
                //
                // If startWrite saved a callback, we call it outside the synchronization.
                //
                cb = _writeCallback;
                state = _writeCallbackState;
                _writeCallback = null;
                _writeCallbackState = null;
            }

            //
            // Invoke the callback with an IAsyncResult wrapper object.
            //
            if(cb != null)
            {
                _resultWrapper.update(r, state, false);
                cb(_resultWrapper);
            }
        }

        private void readCallback(IAsyncResult r)
        {
            //
            // Nothing to do if BeginSend completed synchronously.
            //
            if(r.CompletedSynchronously)
            {
                return;
            }

            AsyncCallback cb = null;
            object state = null;

            lock(this) // A monitor is not necessary here.
            {
                //
                // If startRead saved a callback, we call it outside the synchronization.
                //
                cb = _readCallback;
                state = _readCallbackState;
                _readCallback = null;
                _readCallbackState = null;
            }

            //
            // Invoke the callback with an IAsyncResult wrapper object.
            //
            if(cb != null)
            {
                _resultWrapper.update(r, state, false);
                cb(_resultWrapper);
            }
        }
#endif

        public bool startRead(Buffer buf, AsyncCallback callback, object state)
        {
            Debug.Assert(_fd != null);

            int packetSize = buf.b.remaining();
            if(_maxReceivePacketSize > 0 && packetSize > _maxReceivePacketSize)
            {
                packetSize = _maxReceivePacketSize;
            }

#if COMPACT
            bool readPending = false;

            lock(this) // A monitor is not necessary here.
            {
                //
                // Check if an asynchronous read is already pending (started in read()).
                //
                if(_readResult != null)
                {
                    Debug.Assert(_state >= StateConnected && !_readResult.CompletedSynchronously);

                    //
                    // If the read already completed, we behave as if it completed
                    // synchronously and invoke the callback immediately, otherwise
                    // we let readCallback invoke it later.
                    //
                    if(_readResult.IsCompleted)
                    {
                        readPending = true;
                    }
                    else
                    {
                        _readCallback = callback;
                        _readCallbackState = state;
                        return false;
                    }
                }
            }

            if(readPending)
            {
                _resultWrapper.update(_readResult, state, true);
                callback(_resultWrapper);
                return true;
            }
#endif

            Debug.Assert(_readResult == null);

            // COMPILERFIX: Workaround for Mac OS X broken poll(), see Mono bug #470120
            if(AssemblyUtil.osx_)
            {
                if(++_blocking == 1)
                {
                    Network.setBlock(_fd, true);
                }
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

                // COMPILERFIX: Workaround for Mac OS X broken poll(), see Mono bug #470120
                if(AssemblyUtil.osx_)
                {
                    if(--_blocking == 0)
                    {
                        Network.setBlock(_fd, false);
                    }
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

        public bool startWrite(Buffer buf, AsyncCallback callback, object state, out bool completed)
        {
            Debug.Assert(_fd != null);

            //
            // We limit the packet size for writing to ensure connection timeouts are based
            // on a fixed packet size.
            //
            int packetSize = buf.b.remaining();
            if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize)
            {
                packetSize = _maxSendPacketSize;
            }

#if COMPACT
            bool writePending = false;

            lock(this) // A monitor is not necessary here.
            {
                //
                // Check if an asynchronous write is already pending (started in write()).
                //
                if(_writeResult != null)
                {
                    Debug.Assert(_state >= StateConnected && !_writeResult.CompletedSynchronously);

                    //
                    // If the write already completed, we behave as if it completed
                    // synchronously and invoke the callback immediately, otherwise
                    // we let writeCallback invoke it later.
                    //
                    if(_writeResult.IsCompleted)
                    {
                        writePending = true;
                    }
                    else
                    {
                        _writeCallback = callback;
                        _writeCallbackState = state;
                        completed = packetSize == buf.b.remaining();
                        return false;
                    }
                }
            }

            if(writePending)
            {
                completed = packetSize == buf.b.remaining();
                _resultWrapper.update(_writeResult, state, true);
                callback(_resultWrapper);
                return true;
            }
#endif

            Debug.Assert(_writeResult == null);

            // COMPILERFIX: Workaround for Mac OS X broken poll(), see Mono bug #470120
            if(AssemblyUtil.osx_)
            {
                if(++_blocking == 1)
                {
                    Network.setBlock(_fd, true);
                }
            }

            if(_state < StateConnected)
            {
                _writeResult = Network.doConnectAsync(_fd, _addr, callback, state);
                completed = false;
                return _writeResult.CompletedSynchronously;
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

            completed = packetSize == buf.b.remaining();
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

                // COMPILERFIX: Workaround for Mac OS X broken poll(), see Mono bug #470120
                if(AssemblyUtil.osx_)
                {
                    if(--_blocking == 0)
                    {
                        Network.setBlock(_fd, false);
                    }
                }

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
#if COMPACT
            _instance = instance;
#endif
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

#if COMPACT
        //
        // We need a wrapper that implements IAsyncResult in order to manage the
        // state value correctly.
        //
        internal class ResultWrapper : IAsyncResult
        {
            internal void update(IAsyncResult r, object state, bool completedSynchronously)
            {
                _result = r;
                _state = state;
                _completedSynchronously = completedSynchronously;
            }

            public Object AsyncState
            {
                get
                {
                    return _state;
                }
            }

            public System.Threading.WaitHandle AsyncWaitHandle
            {
                get
                {
                    return _result.AsyncWaitHandle;
                }
            }

            public bool CompletedSynchronously
            {
                get
                {
                    return _completedSynchronously;
                }
            }

            public bool IsCompleted
            {
                get
                {
                    return _result.IsCompleted;
                }
            }

            private IAsyncResult _result;
            private object _state;
            private bool _completedSynchronously;
        }
#endif

#if COMPACT
        private Instance _instance;
#endif
        private Socket _fd;
        private IPEndPoint _addr;
        private TraceLevels _traceLevels;
        private Ice.Logger _logger;
        private Ice.Stats _stats;
        private string _desc;
        private int _state;
        private int _maxSendPacketSize;
        private int _maxReceivePacketSize;

        private int _blocking = 0;
        private IAsyncResult _writeResult;
        private IAsyncResult _readResult;

#if COMPACT
        private AsyncCallback _writeCallback;
        private object _writeCallbackState;
        private AsyncCallback _readCallback;
        private object _readCallbackState;
        private ResultWrapper _resultWrapper = new ResultWrapper();
#endif

        private const int StateNeedConnect = 0;
        private const int StateConnectPending = 1;
        private const int StateConnected = 2;
    }

}
