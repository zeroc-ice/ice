//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace IceInternal
{
    internal sealed class UdpTransceiver : ITransceiver
    {
        public Socket? Fd() => _fd;

        public int Initialize(ref ArraySegment<byte> readBuffer, IList<ArraySegment<byte>> writeBuffer)
        {
            Debug.Assert(_fd != null);
            if (_state == StateNeedConnect)
            {
                _state = StateConnectPending;
                try
                {
                    if (_sourceAddr != null)
                    {
                        _fd.Bind(_sourceAddr);
                    }
                    _fd.Connect(_addr);
                }
                catch (System.Net.Sockets.SocketException ex)
                {
                    if (Network.WouldBlock(ex))
                    {
                        return SocketOperation.Connect;
                    }
                    throw new ConnectFailedException(ex);
                }
                catch (System.Exception ex)
                {
                    throw new ConnectFailedException(ex);
                }
                _state = StateConnected;
            }

            Debug.Assert(_state >= StateConnected);
            return SocketOperation.None;
        }

        public int Closing(bool initiator, System.Exception? ex)
        {
            //
            // Nothing to do.
            //
            return SocketOperation.None;
        }

        public void Close()
        {
            if (_fd != null)
            {
                try
                {
                    _fd.Close();
                }
                catch (System.IO.IOException)
                {
                }
                _fd = null;
            }
        }

        public Endpoint Bind()
        {
            Debug.Assert(_fd != null);
            if (Network.IsMulticast((IPEndPoint)_addr))
            {
                Network.SetReuseAddress(_fd, true);
                _mcastAddr = (IPEndPoint)_addr;
                if (AssemblyUtil.IsWindows)
                {
                    //
                    // Windows does not allow binding to the mcast address itself
                    // so we bind to INADDR_ANY (0.0.0.0) instead. As a result,
                    // bi-directional connection won't work because the source
                    // address won't the multicast address and the client will
                    // therefore reject the datagram.
                    //
                    if (_addr.AddressFamily == AddressFamily.InterNetwork)
                    {
                        _addr = new IPEndPoint(IPAddress.Any, _port);
                    }
                    else
                    {
                        _addr = new IPEndPoint(IPAddress.IPv6Any, _port);
                    }
                }

                _addr = Network.DoBind(_fd, _addr);
                if (_port == 0)
                {
                    _mcastAddr.Port = ((IPEndPoint)_addr).Port;
                }
                Debug.Assert(_mcastInterface != null);
                Network.SetMcastGroup(_fd, _mcastAddr.Address, _mcastInterface);
            }
            else
            {
                _addr = Network.DoBind(_fd, _addr);
            }
            _bound = true;
            Debug.Assert(_endpoint != null);
            _endpoint = _endpoint.GetEndpoint(this);
            return _endpoint;
        }

        public void Destroy()
        {
            _readEventArgs.Dispose();
            Debug.Assert(_writeEventArgs != null);
            _writeEventArgs.Dispose();
        }

        public int Write(IList<ArraySegment<byte>> buffer, ref int offset)
        {
            int count = buffer.GetByteCount();
            int remaining = count - offset;
            if (remaining == 0)
            {
                return SocketOperation.None;
            }

            Debug.Assert(_fd != null && _state >= StateConnected);

            // The caller is supposed to check the send size before by calling checkSendSize
            Debug.Assert(Math.Min(MaxPacketSize, _sndSize - UdpOverhead) >= count);

            int ret;
            while (true)
            {
                try
                {
                    if (_state == StateConnected)
                    {
                        ret = _fd.Send(buffer, SocketFlags.None);
                    }
                    else
                    {
                        if (_peerAddr == null)
                        {
                            throw new Ice.TransportException("cannot send datagram to undefined peer");
                        }

                        ArraySegment<byte> data = buffer.GetSegment(0, count);
                        ret = _fd.SendTo(data.Array, 0, data.Count, SocketFlags.None, _peerAddr);
                    }
                    Debug.Assert(ret == count);
                    offset += ret;
                    break;
                }
                catch (System.Net.Sockets.SocketException ex)
                {
                    if (Network.Interrupted(ex))
                    {
                        continue;
                    }

                    if (Network.WouldBlock(ex))
                    {
                        return SocketOperation.Write;
                    }

                    if (Network.ConnectionLost(ex))
                    {
                        throw new Ice.ConnectionLostException(ex);
                    }
                    else
                    {
                        throw new Ice.TransportException(ex);
                    }
                }
            }
            return SocketOperation.None;
        }

        public int Read(ref ArraySegment<byte> buffer, ref int offset)
        {
            if (buffer.Count - offset == 0)
            {
                return SocketOperation.None;
            }

            Debug.Assert(offset == 0);
            Debug.Assert(_fd != null);

            int packetSize = Math.Min(MaxPacketSize, _rcvSize - UdpOverhead);
            Debug.Assert(buffer.Count == 0);
            buffer = new byte[packetSize];

            int ret;
            while (true)
            {
                try
                {
                    EndPoint? peerAddr = _peerAddr;
                    if (peerAddr == null)
                    {
                        if (_addr.AddressFamily == AddressFamily.InterNetwork)
                        {
                            peerAddr = new IPEndPoint(IPAddress.Any, 0);
                        }
                        else
                        {
                            Debug.Assert(_addr.AddressFamily == AddressFamily.InterNetworkV6);
                            peerAddr = new IPEndPoint(IPAddress.IPv6Any, 0);
                        }
                    }

                    // TODO: Workaround for https://github.com/dotnet/corefx/issues/31182
                    if (_state == StateConnected ||
                       (AssemblyUtil.IsMacOS && _fd.AddressFamily == AddressFamily.InterNetworkV6 && _fd.DualMode))
                    {
                        ret = _fd.Receive(buffer.Array, 0, packetSize, SocketFlags.None);
                    }
                    else
                    {
                        ret = _fd.ReceiveFrom(buffer.Array, 0, packetSize, SocketFlags.None, ref peerAddr);
                        _peerAddr = (IPEndPoint)peerAddr;
                    }
                    break;
                }
                catch (System.Net.Sockets.SocketException e)
                {
                    if (Network.RecvTruncated(e))
                    {
                        // The message was truncated and the whole buffer is filled. We ignore
                        // this error here, it will be detected at the connection level when
                        // the Ice message size is checked against the buffer size.
                        ret = buffer.Count;
                        break;
                    }

                    if (Network.Interrupted(e))
                    {
                        continue;
                    }

                    if (Network.WouldBlock(e))
                    {
                        return SocketOperation.Read;
                    }

                    if (Network.ConnectionLost(e))
                    {
                        throw new Ice.ConnectionLostException();
                    }
                    else
                    {
                        throw new Ice.TransportException(e);
                    }
                }
            }

            if (ret == 0)
            {
                throw new ConnectionLostException();
            }

            if (_state == StateNeedConnect)
            {
                Debug.Assert(_incoming);

                //
                // If we must connect, then we connect to the first peer that sends us a packet.
                //
                Debug.Assert(_peerAddr != null);
                bool connected = Network.DoConnect(_fd, _peerAddr, null);
                Debug.Assert(connected);
                _state = StateConnected; // We're connected now

                if (_instance.TraceLevel >= 1)
                {
                    _instance.Logger.Trace(_instance.TraceCategory, $"connected {Transport()} socket\n{this}");
                }
            }

            buffer = buffer.Slice(0, ret);
            offset = ret;
            return SocketOperation.None;
        }

        public bool StartRead(ref ArraySegment<byte> buffer, ref int offset, AsyncCallback callback, object state)
        {
            Debug.Assert(_fd != null);
            Debug.Assert(offset == 0, $"offset: {offset}\n{Environment.StackTrace}");

            int packetSize = Math.Min(MaxPacketSize, _rcvSize - UdpOverhead);
            Debug.Assert(buffer.Count == 0);
            buffer = new byte[packetSize];

            try
            {
                // TODO: Workaround for https://github.com/dotnet/corefx/issues/31182
                _readCallback = callback;
                _readEventArgs.UserToken = state;
                _readEventArgs.SetBuffer(buffer.Array, 0, packetSize);
                if (_state == StateConnected ||
                   (AssemblyUtil.IsMacOS && _fd.AddressFamily == AddressFamily.InterNetworkV6 && _fd.DualMode))
                {
                    return !_fd.ReceiveAsync(_readEventArgs);
                }
                else
                {
                    Debug.Assert(_incoming);

                    return !_fd.ReceiveFromAsync(_readEventArgs);
                }
            }
            catch (System.Net.Sockets.SocketException ex)
            {
                if (Network.RecvTruncated(ex))
                {
                    // Nothing to do
                    return true;
                }
                else
                {
                    if (Network.ConnectionLost(ex))
                    {
                        throw new Ice.ConnectionLostException(ex);
                    }
                    else
                    {
                        throw new Ice.TransportException(ex);
                    }
                }
            }
        }

        public void FinishRead(ref ArraySegment<byte> buffer, ref int offset)
        {
            if (_fd == null)
            {
                return;
            }

            int ret;
            try
            {
                if (_readEventArgs.SocketError != SocketError.Success)
                {
                    throw new System.Net.Sockets.SocketException((int)_readEventArgs.SocketError);
                }
                ret = _readEventArgs.BytesTransferred;
                // TODO: Workaround for https://github.com/dotnet/corefx/issues/31182
                if (_state != StateConnected &&
                   !(AssemblyUtil.IsMacOS && _fd.AddressFamily == AddressFamily.InterNetworkV6 && _fd.DualMode))
                {
                    _peerAddr = _readEventArgs.RemoteEndPoint;
                }
            }
            catch (System.Net.Sockets.SocketException ex)
            {
                if (Network.RecvTruncated(ex))
                {
                    // The message was truncated and the whole buffer is filled. We ignore
                    // this error here, it will be detected at the connection level when
                    // the Ice message size is checked against the buffer size.
                    ret = buffer.Count;
                }
                else
                {
                    if (Network.ConnectionLost(ex))
                    {
                        throw new Ice.ConnectionLostException(ex);
                    }
                    else if (Network.ConnectionRefused(ex))
                    {
                        throw new Ice.ConnectionRefusedException(ex);
                    }
                    else
                    {
                        throw new Ice.TransportException(ex);
                    }
                }
            }

            if (ret == 0)
            {
                throw new Ice.ConnectionLostException();
            }

            Debug.Assert(ret > 0);

            if (_state == StateNeedConnect)
            {
                Debug.Assert(_incoming);

                //
                // If we must connect, then we connect to the first peer that
                // sends us a packet.
                //
                bool connected = !_fd.ConnectAsync(_readEventArgs);
                Debug.Assert(connected);
                _state = StateConnected; // We're connected now

                if (_instance.TraceLevel >= 1)
                {
                    _instance.Logger.Trace(_instance.TraceCategory, $"connected {Transport()} socket\n{this}");
                }
            }

            buffer = buffer.Slice(0, ret);
            offset = ret;
        }

        public bool
        StartWrite(IList<ArraySegment<byte>> buffer, int offset, AsyncCallback callback, object state, out bool completed)
        {
            Debug.Assert(_fd != null);
            Debug.Assert(_writeEventArgs != null);
            Debug.Assert(offset == 0);
            bool completedSynchronously;
            if (!_incoming && _state < StateConnected)
            {
                Debug.Assert(_addr != null);
                completed = false;
                if (_sourceAddr != null)
                {
                    _fd.Bind(_sourceAddr);
                }
                _writeEventArgs.UserToken = state;
                return !_fd.ConnectAsync(_writeEventArgs);
            }

            // The caller is supposed to check the send size before by calling checkSendSize
            Debug.Assert(Math.Min(MaxPacketSize, _sndSize - UdpOverhead) >= buffer.GetByteCount());

            try
            {
                _writeCallback = callback;

                if (_state == StateConnected)
                {
                    _writeEventArgs.UserToken = state;
                    _writeEventArgs.BufferList = buffer;
                    completedSynchronously = !_fd.SendAsync(_writeEventArgs);
                }
                else
                {
                    if (_peerAddr == null)
                    {
                        throw new Ice.TransportException("cannot send datagram to undefined peer");
                    }
                    _writeEventArgs.RemoteEndPoint = _peerAddr;
                    _writeEventArgs.UserToken = state;
                    ArraySegment<byte> data = buffer.GetSegment(0, buffer.GetByteCount());
                    _writeEventArgs.SetBuffer(data.Array, 0, data.Count);

                    completedSynchronously = !_fd.SendToAsync(_writeEventArgs);
                }
            }
            catch (System.Net.Sockets.SocketException ex)
            {
                if (Network.ConnectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                else
                {
                    throw new Ice.TransportException(ex);
                }
            }
            completed = true;
            return completedSynchronously;
        }

        public void FinishWrite(IList<ArraySegment<byte>> buffer, ref int offset)
        {
            Debug.Assert(_writeEventArgs != null);
            Debug.Assert(offset == 0);
            if (_fd == null)
            {
                int count = buffer.GetByteCount(); // Assume all the data was sent for at-most-once semantics.
                _writeEventArgs = null;
                offset = count;
                return;
            }

            if (!_incoming && _state < StateConnected)
            {
                if (_writeEventArgs.SocketError != SocketError.Success)
                {
                    var ex = new System.Net.Sockets.SocketException((int)_writeEventArgs.SocketError);
                    if (Network.ConnectionRefused(ex))
                    {
                        throw new ConnectionRefusedException(ex);
                    }
                    else
                    {
                        throw new ConnectFailedException(ex);
                    }
                }
                return;
            }

            int ret;
            try
            {
                if (_writeEventArgs.SocketError != SocketError.Success)
                {
                    throw new System.Net.Sockets.SocketException((int)_writeEventArgs.SocketError);
                }
                ret = _writeEventArgs.BytesTransferred;
                _writeEventArgs.SetBuffer(null, 0, 0);
                if (_writeEventArgs.BufferList != null && _writeEventArgs.BufferList != buffer)
                {
                    _writeEventArgs.BufferList.Clear();
                }
                _writeEventArgs.BufferList = null;
            }
            catch (System.Net.Sockets.SocketException ex)
            {
                if (Network.ConnectionLost(ex))
                {
                    throw new ConnectionLostException(ex);
                }
                else
                {
                    throw new Ice.TransportException(ex);
                }
            }

            if (ret == 0)
            {
                throw new ConnectionLostException();
            }

            Debug.Assert(ret > 0);
            Debug.Assert(ret == buffer.GetByteCount());
            offset = ret;
            return;
        }

        public string Transport() => _instance.Transport;

        public ConnectionInfo GetInfo()
        {
            var info = new UDPConnectionInfo();
            if (_fd != null)
            {
                EndPoint localEndpoint = Network.GetLocalAddress(_fd);
                info.LocalAddress = Network.EndpointAddressToString(localEndpoint);
                info.LocalPort = Network.EndpointPort(localEndpoint);
                if (_state == StateNotConnected)
                {
                    if (_peerAddr != null)
                    {
                        info.RemoteAddress = Network.EndpointAddressToString(_peerAddr);
                        info.RemotePort = Network.EndpointPort(_peerAddr);
                    }
                }
                else
                {
                    EndPoint? remoteEndpoint = Network.GetRemoteAddress(_fd);
                    if (remoteEndpoint != null)
                    {
                        info.RemoteAddress = Network.EndpointAddressToString(remoteEndpoint);
                        info.RemotePort = Network.EndpointPort(remoteEndpoint);
                    }
                }
                info.RcvSize = Network.GetRecvBufferSize(_fd);
                info.SndSize = Network.GetSendBufferSize(_fd);
            }

            if (_mcastAddr != null)
            {
                info.McastAddress = Network.EndpointAddressToString(_mcastAddr);
                info.McastPort = Network.EndpointPort(_mcastAddr);
            }
            return info;
        }

        public void CheckSendSize(int size)
        {
            // The maximum packetSize is either the maximum allowable UDP packet size, or the UDP send buffer size
            // (which ever is smaller).
            int packetSize = Math.Min(MaxPacketSize, _sndSize - UdpOverhead);
            if (packetSize < size)
            {
                throw new DatagramLimitException($"cannot send more than {packetSize} bytes with UDP");
            }
        }

        public void SetBufferSize(int rcvSize, int sndSize) => SetBufSize(rcvSize, sndSize);

        public override string ToString()
        {
            if (_fd == null)
            {
                return "<closed>";
            }

            string s;
            if (_incoming && !_bound)
            {
                s = "local address = " + Network.AddrToString(_addr);
            }
            else if (_state == StateNotConnected)
            {
                s = "local address = " + Network.LocalAddrToString(Network.GetLocalAddress(_fd));
                if (_peerAddr != null)
                {
                    s += "\nremote address = " + Network.AddrToString(_peerAddr);
                }
            }
            else
            {
                s = Network.FdToString(_fd);
            }

            if (_mcastAddr != null)
            {
                s += "\nmulticast address = " + Network.AddrToString(_mcastAddr);
            }
            return s;
        }

        public string ToDetailedString()
        {
            var s = new StringBuilder(ToString());
            List<string> intfs;
            if (_mcastAddr == null)
            {
                intfs = Network.GetHostsForEndpointExpand(Network.EndpointAddressToString(_addr),
                                                          _instance.IPVersion, true);
            }
            else
            {
                Debug.Assert(_mcastInterface != null);
                intfs = Network.GetInterfacesForMulticast(_mcastInterface,
                                                          Network.GetIPVersion(_mcastAddr.Address));
            }

            if (intfs.Count != 0)
            {
                s.Append("\nlocal interfaces = ");
                s.Append(string.Join(", ", intfs));
            }
            return s.ToString();
        }

        public int EffectivePort() => Network.EndpointPort(_addr);

        //
        // Only for use by UdpConnector.
        //
        internal UdpTransceiver(TransportInstance instance, EndPoint addr, IPAddress? sourceAddr, string mcastInterface,
                                int mcastTtl)
        {
            _instance = instance;
            _addr = addr;
            if (sourceAddr != null)
            {
                _sourceAddr = new IPEndPoint(sourceAddr, 0);
            }

            _readEventArgs = new SocketAsyncEventArgs();
            _readEventArgs.RemoteEndPoint = _addr;
            _readEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(IoCompleted);

            _writeEventArgs = new SocketAsyncEventArgs();
            _writeEventArgs.RemoteEndPoint = _addr;
            _writeEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(IoCompleted);

            _mcastInterface = mcastInterface;
            _state = StateNeedConnect;
            _incoming = false;

            try
            {
                _fd = Network.CreateSocket(true, _addr.AddressFamily);
                SetBufSize(-1, -1);
                Network.SetBlock(_fd, false);
                if (Network.IsMulticast((IPEndPoint)_addr))
                {
                    if (_mcastInterface.Length > 0)
                    {
                        Network.SetMcastInterface(_fd, _mcastInterface, _addr.AddressFamily);
                    }
                    if (mcastTtl != -1)
                    {
                        Network.SetMcastTtl(_fd, mcastTtl, _addr.AddressFamily);
                    }
                }
            }
            catch (System.Exception)
            {
                _fd = null;
                throw;
            }
        }

        //
        // Only for use by UdpEndpoint.
        //
        internal UdpTransceiver(UdpEndpoint endpoint, TransportInstance instance, string host, int port,
                                string mcastInterface, bool connect)
        {
            _endpoint = endpoint;
            _instance = instance;
            _state = connect ? StateNeedConnect : StateNotConnected;
            _mcastInterface = mcastInterface;
            _incoming = true;
            _port = port;

            try
            {
                _addr = Network.GetAddressForServer(host, port, instance.IPVersion, instance.PreferIPv6);

                _readEventArgs = new SocketAsyncEventArgs();
                _readEventArgs.RemoteEndPoint = _addr;
                _readEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(IoCompleted);

                _writeEventArgs = new SocketAsyncEventArgs();
                _writeEventArgs.RemoteEndPoint = _addr;
                _writeEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(IoCompleted);

                _fd = Network.CreateServerSocket(true, _addr.AddressFamily, instance.IPVersion);
                SetBufSize(-1, -1);
                Network.SetBlock(_fd, false);
            }
            catch (System.Exception)
            {
                if (_readEventArgs != null)
                {
                    _readEventArgs.Dispose();
                }
                if (_writeEventArgs != null)
                {
                    _writeEventArgs.Dispose();
                }
                _fd = null;
                throw;
            }
        }

        private void SetBufSize(int rcvSize, int sndSize)
        {
            Debug.Assert(_fd != null);

            for (int i = 0; i < 2; ++i)
            {
                bool isSnd;
                string direction;
                string prop;
                int dfltSize;
                int sizeRequested;
                if (i == 0)
                {
                    isSnd = false;
                    direction = "receive";
                    prop = "Ice.UDP.RcvSize";
                    dfltSize = Network.GetRecvBufferSize(_fd);
                    sizeRequested = rcvSize;
                    _rcvSize = dfltSize;
                }
                else
                {
                    isSnd = true;
                    direction = "send";
                    prop = "Ice.UDP.SndSize";
                    dfltSize = Network.GetSendBufferSize(_fd);
                    sizeRequested = sndSize;
                    _sndSize = dfltSize;
                }

                //
                // Get property for buffer size if size not passed in.
                //
                if (sizeRequested == -1)
                {
                    sizeRequested = _instance.Communicator.GetPropertyAsInt(prop) ?? dfltSize;
                }
                //
                // Check for sanity.
                //
                if (sizeRequested < (UdpOverhead + Ice1Definitions.HeaderSize))
                {
                    _instance.Logger.Warning($"Invalid {prop} value of {sizeRequested} adjusted to {dfltSize}");
                    sizeRequested = dfltSize;
                }

                if (sizeRequested != dfltSize)
                {
                    //
                    // Try to set the buffer size. The kernel will silently adjust
                    // the size to an acceptable value. Then read the size back to
                    // get the size that was actually set.
                    //
                    int sizeSet;
                    if (i == 0)
                    {
                        Network.SetRecvBufferSize(_fd, sizeRequested);
                        _rcvSize = Network.GetRecvBufferSize(_fd);
                        sizeSet = _rcvSize;
                    }
                    else
                    {
                        Network.SetSendBufferSize(_fd, sizeRequested);
                        _sndSize = Network.GetSendBufferSize(_fd);
                        sizeSet = _sndSize;
                    }

                    //
                    // Warn if the size that was set is less than the requested size
                    // and we have not already warned
                    //
                    if (sizeSet < sizeRequested)
                    {
                        Ice.BufSizeWarnInfo winfo = _instance.GetBufSizeWarn(Ice.EndpointType.UDP);
                        if ((isSnd && (!winfo.SndWarn || winfo.SndSize != sizeRequested)) ||
                           (!isSnd && (!winfo.RcvWarn || winfo.RcvSize != sizeRequested)))
                        {
                            _instance.Logger.Warning(
                                $"UDP {direction} buffer size: requested size of {sizeRequested} adjusted to {sizeSet}");

                            if (isSnd)
                            {
                                _instance.SetSndBufSizeWarn(Ice.EndpointType.UDP, sizeRequested);
                            }
                            else
                            {
                                _instance.SetRcvBufSizeWarn(Ice.EndpointType.UDP, sizeRequested);
                            }
                        }
                    }
                }
            }
        }

        internal void IoCompleted(object? sender, SocketAsyncEventArgs e)
        {
            switch (e.LastOperation)
            {
                case SocketAsyncOperation.Receive:
                case SocketAsyncOperation.ReceiveFrom:
                    Debug.Assert(_readCallback != null);
                    _readCallback(e.UserToken);
                    break;
                case SocketAsyncOperation.Send:
                case SocketAsyncOperation.Connect:
                    Debug.Assert(_writeCallback != null);
                    _writeCallback(e.UserToken);
                    break;
                default:
                    throw new ArgumentException("the last operation completed on the socket was not a receive or send",
                        nameof(e.LastOperation));
            }
        }

        private UdpEndpoint? _endpoint;
        private readonly TransportInstance _instance;
        private int _state;
        private readonly bool _incoming;
        private int _rcvSize;
        private int _sndSize;
        private Socket? _fd;
        private EndPoint _addr;
        private readonly IPEndPoint? _sourceAddr;
        private IPEndPoint? _mcastAddr = null;
        private EndPoint? _peerAddr = null;
        private readonly string? _mcastInterface = null;

        private readonly int _port = 0;
        private bool _bound = false;

        private SocketAsyncEventArgs? _writeEventArgs;
        private readonly SocketAsyncEventArgs _readEventArgs;

        private AsyncCallback? _writeCallback;
        private AsyncCallback? _readCallback;

        private const int StateNeedConnect = 0;
        private const int StateConnectPending = 1;
        private const int StateConnected = 2;
        private const int StateNotConnected = 3;

        //
        // The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for the UDP header
        // to get the maximum payload.
        //
        private const int UdpOverhead = 20 + 8;
        private const int MaxPacketSize = 65535 - UdpOverhead;
    }
}
