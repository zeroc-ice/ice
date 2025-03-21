// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace Ice.Internal;

#pragma warning disable CA1001 // _readEventArgs and _writeEventArgs are disposed by destroy.
internal sealed class UdpTransceiver : Transceiver
#pragma warning restore CA1001
{
    public Socket fd()
    {
        return _fd;
    }

    public int initialize(Buffer readBuffer, Buffer writeBuffer, ref bool hasMoreData)
    {
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
                if (Network.wouldBlock(ex))
                {
                    return SocketOperation.Connect;
                }
                throw new Ice.ConnectFailedException(ex);
            }
            catch (System.Exception ex)
            {
                throw new Ice.ConnectFailedException(ex);
            }
            _state = StateConnected;
        }

        Debug.Assert(_state >= StateConnected);
        return SocketOperation.None;
    }

    public int closing(bool initiator, Ice.LocalException ex)
    {
        //
        // Nothing to do.
        //
        return SocketOperation.None;
    }

    public void close()
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

    public EndpointI bind()
    {
        if (Network.isMulticast((IPEndPoint)_addr))
        {
            Network.setReuseAddress(_fd, true);
            _mcastAddr = (IPEndPoint)_addr;
            if (AssemblyUtil.isWindows)
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

            _addr = Network.doBind(_fd, _addr);
            if (_port == 0)
            {
                _mcastAddr.Port = ((IPEndPoint)_addr).Port;
            }
            Network.setMcastGroup(_fd, _mcastAddr.Address, _mcastInterface);
        }
        else
        {
            _addr = Network.doBind(_fd, _addr);
        }
        _bound = true;
        _endpoint = _endpoint.endpoint(this);
        return _endpoint;
    }

    public void destroy()
    {
        _readEventArgs.Dispose();
        _writeEventArgs.Dispose();
    }

    public int write(Buffer buf)
    {
        if (!buf.b.hasRemaining())
        {
            return SocketOperation.None;
        }

        Debug.Assert(buf.b.position() == 0);
        Debug.Assert(_fd != null && _state >= StateConnected);

        // The caller is supposed to check the send size before by calling checkSendSize
        Debug.Assert(Math.Min(_maxPacketSize, _sndSize - _udpOverhead) >= buf.size());

        int ret = 0;
        while (true)
        {
            try
            {
                if (_state == StateConnected)
                {
                    ret = _fd.Send(buf.b.rawBytes(), 0, buf.size(), SocketFlags.None);
                }
                else
                {
                    if (_peerAddr == null)
                    {
                        throw new Ice.SocketException();
                    }
                    ret = _fd.SendTo(buf.b.rawBytes(), 0, buf.size(), SocketFlags.None, _peerAddr);
                }
                break;
            }
            catch (System.Net.Sockets.SocketException ex)
            {
                if (Network.interrupted(ex))
                {
                    continue;
                }

                if (Network.wouldBlock(ex))
                {
                    return SocketOperation.Write;
                }

                if (Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                else
                {
                    throw new Ice.SocketException(ex);
                }
            }
            catch (System.Exception e)
            {
                throw new Ice.SyscallException(e);
            }
        }

        Debug.Assert(ret > 0);
        Debug.Assert(ret == buf.b.limit());
        buf.b.position(buf.b.limit());
        return SocketOperation.None;
    }

    public int read(Buffer buf, ref bool hasMoreData)
    {
        if (!buf.b.hasRemaining())
        {
            return SocketOperation.None;
        }

        Debug.Assert(buf.b.position() == 0);
        Debug.Assert(_fd != null);

        int packetSize = Math.Min(_maxPacketSize, _rcvSize - _udpOverhead);
        buf.resize(packetSize, true);
        buf.b.position(0);

        int ret = 0;
        while (true)
        {
            try
            {
                EndPoint peerAddr = _peerAddr;
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
                   (AssemblyUtil.isMacOS && _fd.AddressFamily == AddressFamily.InterNetworkV6 && _fd.DualMode))
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
            catch (System.Net.Sockets.SocketException e)
            {
                if (Network.recvTruncated(e))
                {
                    // The message was truncated and the whole buffer is filled. We ignore
                    // this error here, it will be detected at the connection level when
                    // the Ice message size is checked against the buffer size.
                    ret = buf.size();
                    break;
                }

                if (Network.interrupted(e))
                {
                    continue;
                }

                if (Network.wouldBlock(e))
                {
                    return SocketOperation.Read;
                }

                if (Network.connectionLost(e))
                {
                    throw new ConnectionLostException(e);
                }
                else
                {
                    throw new Ice.SocketException(e);
                }
            }
            catch (System.Exception e)
            {
                throw new SyscallException(e);
            }
        }

        if (ret == 0)
        {
            throw new Ice.ConnectionLostException();
        }

        Debug.Assert(_state != StateNeedConnect);

        buf.resize(ret, true);
        buf.b.position(ret);

        return SocketOperation.None;
    }

    public bool startRead(Buffer buf, AsyncCallback callback, object state)
    {
        Debug.Assert(buf.b.position() == 0);

        int packetSize = Math.Min(_maxPacketSize, _rcvSize - _udpOverhead);
        buf.resize(packetSize, true);
        buf.b.position(0);

        try
        {
            // TODO: Workaround for https://github.com/dotnet/corefx/issues/31182
            if (_state == StateConnected ||
               (AssemblyUtil.isMacOS && _fd.AddressFamily == AddressFamily.InterNetworkV6 && _fd.DualMode))
            {
                _readCallback = callback;
                _readEventArgs.UserToken = state;
                _readEventArgs.SetBuffer(buf.b.rawBytes(), buf.b.position(), packetSize);
                return !_fd.ReceiveAsync(_readEventArgs);
            }
            else
            {
                Debug.Assert(_incoming);
                _readCallback = callback;
                _readEventArgs.UserToken = state;
                _readEventArgs.SetBuffer(buf.b.rawBytes(), 0, buf.b.limit());
                return !_fd.ReceiveFromAsync(_readEventArgs);
            }
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            if (Network.recvTruncated(ex))
            {
                // Nothing todo
                return true;
            }
            else
            {
                if (Network.connectionLost(ex))
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
               !(AssemblyUtil.isMacOS && _fd.AddressFamily == AddressFamily.InterNetworkV6 && _fd.DualMode))
            {
                _peerAddr = _readEventArgs.RemoteEndPoint;
            }
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            if (Network.recvTruncated(ex))
            {
                // The message was truncated and the whole buffer is filled. We ignore
                // this error here, it will be detected at the connection level when
                // the Ice message size is checked against the buffer size.
                ret = buf.size();
            }
            else
            {
                if (Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }

                if (Network.connectionRefused(ex))
                {
                    throw new Ice.ConnectionRefusedException(ex);
                }
                else
                {
                    throw new Ice.SocketException(ex);
                }
            }
        }

        if (ret == 0)
        {
            throw new Ice.ConnectionLostException();
        }

        Debug.Assert(_state != StateNeedConnect);
        buf.resize(ret, true);
        buf.b.position(ret);
    }

    public bool startWrite(Buffer buf, AsyncCallback callback, object state, out bool messageFullyWritten)
    {
        if (!_incoming && _state < StateConnected)
        {
            Debug.Assert(_addr != null);
            messageFullyWritten = false;
            if (_sourceAddr != null)
            {
                _fd.Bind(_sourceAddr);
            }
            _writeEventArgs.UserToken = state;
            return !_fd.ConnectAsync(_writeEventArgs);
        }

        Debug.Assert(_fd != null);

        // The caller is supposed to check the send size before by calling checkSendSize
        Debug.Assert(Math.Min(_maxPacketSize, _sndSize - _udpOverhead) >= buf.size());

        Debug.Assert(buf.b.position() == 0);

        bool completedSynchronously;
        try
        {
            _writeCallback = callback;

            if (_state == StateConnected)
            {
                _writeEventArgs.UserToken = state;
                _writeEventArgs.SetBuffer(buf.b.rawBytes(), 0, buf.b.limit());
                completedSynchronously = !_fd.SendAsync(_writeEventArgs);
            }
            else
            {
                if (_peerAddr == null)
                {
                    throw new Ice.SocketException();
                }
                _writeEventArgs.RemoteEndPoint = _peerAddr;
                _writeEventArgs.UserToken = state;
                _writeEventArgs.SetBuffer(buf.b.rawBytes(), 0, buf.b.limit());
                completedSynchronously = !_fd.SendToAsync(_writeEventArgs);
            }
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            if (Network.connectionLost(ex))
            {
                throw new Ice.ConnectionLostException(ex);
            }
            else
            {
                throw new Ice.SocketException(ex);
            }
        }

        messageFullyWritten = true;
        return completedSynchronously;
    }

    public void finishWrite(Buffer buf)
    {
        if (_fd == null)
        {
            buf.b.position(buf.size()); // Assume all the data was sent for at-most-once semantics.
            _writeEventArgs = null;
            return;
        }

        if (!_incoming && _state < StateConnected)
        {
            if (_writeEventArgs.SocketError != SocketError.Success)
            {
                System.Net.Sockets.SocketException ex =
                    new System.Net.Sockets.SocketException((int)_writeEventArgs.SocketError);
                if (Network.connectionRefused(ex))
                {
                    throw new Ice.ConnectionRefusedException(ex);
                }
                else
                {
                    throw new Ice.ConnectFailedException(ex);
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
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            if (Network.connectionLost(ex))
            {
                throw new Ice.ConnectionLostException(ex);
            }
            else
            {
                throw new Ice.SocketException(ex);
            }
        }

        if (ret == 0)
        {
            throw new Ice.ConnectionLostException();
        }

        Debug.Assert(ret > 0);
        Debug.Assert(ret == buf.b.limit());
        buf.b.position(buf.b.position() + ret);
    }

    public string protocol()
    {
        return _instance.protocol();
    }

    public ConnectionInfo getInfo(bool incoming, string adapterName, string connectionId)
    {
        if (_fd is null)
        {
            return new UDPConnectionInfo(incoming, adapterName, connectionId);
        }
        else
        {
            EndPoint localEndpoint = Network.getLocalAddress(_fd);

            if (_state == StateNotConnected) // a server connection
            {
                Debug.Assert(incoming);

                // Since this info is cached in the Connection object shared by all the clients, we don't store the
                // remote address/port of the latest client in this info.
                return new UDPConnectionInfo(
                    incoming,
                    adapterName,
                    connectionId,
                    Network.endpointAddressToString(localEndpoint),
                    Network.endpointPort(localEndpoint),
                    remoteAddress: "",
                    remotePort: -1,
                    _mcastAddr is not null ? Network.endpointAddressToString(_mcastAddr) : "",
                    _mcastAddr is not null ? Network.endpointPort(_mcastAddr) : -1,
                    _rcvSize,
                    _sndSize);
            }
            else // client connection
            {
                Debug.Assert(!incoming);
                EndPoint remoteEndpoint = Network.getRemoteAddress(_fd);

                return new UDPConnectionInfo(
                    incoming,
                    adapterName,
                    connectionId,
                    Network.endpointAddressToString(localEndpoint),
                    Network.endpointPort(localEndpoint),
                    remoteEndpoint is not null ? Network.endpointAddressToString(remoteEndpoint) : "",
                    remoteEndpoint is not null ? Network.endpointPort(remoteEndpoint) : -1,
                    _mcastAddr is not null ? Network.endpointAddressToString(_mcastAddr) : "",
                    _mcastAddr is not null ? Network.endpointPort(_mcastAddr) : -1,
                    _rcvSize,
                    _sndSize);
            }
        }
    }

    public void checkSendSize(Buffer buf)
    {
        //
        // The maximum packetSize is either the maximum allowable UDP packet size, or
        // the UDP send buffer size (which ever is smaller).
        //
        int packetSize = Math.Min(_maxPacketSize, _sndSize - _udpOverhead);
        if (packetSize < buf.size())
        {
            throw new Ice.DatagramLimitException();
        }
    }

    public void setBufferSize(int rcvSize, int sndSize)
    {
        setBufSize(rcvSize, sndSize);
    }

    public override string ToString()
    {
        if (_fd == null)
        {
            return "<closed>";
        }

        string s;
        if (_incoming && !_bound)
        {
            s = "local address = " + Network.addrToString(_addr);
        }
        else if (_state == StateNotConnected)
        {
            s = "local address = " + Network.localAddrToString(Network.getLocalAddress(_fd));
            if (_peerAddr != null)
            {
                s += "\nremote address = " + Network.addrToString(_peerAddr);
            }
        }
        else
        {
            s = Network.fdToString(_fd);
        }

        if (_mcastAddr != null)
        {
            s += "\nmulticast address = " + Network.addrToString(_mcastAddr);
        }
        return s;
    }

    public string toDetailedString()
    {
        StringBuilder s = new StringBuilder(ToString());
        if (_mcastAddr is not null)
        {
            var intfs = Network.getInterfacesForMulticast(
                _mcastInterface,
                Network.getProtocolSupport(_mcastAddr.Address));
            if (intfs.Count != 0)
            {
                s.Append("\nlocal interfaces = ");
                s.Append(string.Join(", ", intfs.ToArray()));
            }
        }
        return s.ToString();
    }

    public int effectivePort()
    {
        return Network.endpointPort(_addr);
    }

    //
    // Only for use by UdpConnector.
    //
    internal UdpTransceiver(
        ProtocolInstance instance,
        EndPoint addr,
        EndPoint sourceAddr,
        string mcastInterface,
        int mcastTtl)
    {
        _instance = instance;
        _addr = addr;
        _sourceAddr = sourceAddr;

        _readEventArgs = new SocketAsyncEventArgs();
        _readEventArgs.RemoteEndPoint = _addr;
        _readEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(ioCompleted);

        _writeEventArgs = new SocketAsyncEventArgs();
        _writeEventArgs.RemoteEndPoint = _addr;
        _writeEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(ioCompleted);

        _mcastInterface = mcastInterface;
        _state = StateNeedConnect;
        _incoming = false;

        try
        {
            _fd = Network.createSocket(true, _addr.AddressFamily);
            setBufSize(-1, -1);
            Network.setBlock(_fd, false);
            if (Network.isMulticast((IPEndPoint)_addr))
            {
                if (_mcastInterface.Length > 0)
                {
                    Network.setMcastInterface(_fd, _mcastInterface, _addr.AddressFamily);
                }
                if (mcastTtl != -1)
                {
                    Network.setMcastTtl(_fd, mcastTtl, _addr.AddressFamily);
                }
            }
        }
        catch (Ice.LocalException)
        {
            _fd = null;
            throw;
        }
    }

    //
    // Only for use by UdpEndpoint.
    //
    internal UdpTransceiver(
        UdpEndpointI endpoint,
        ProtocolInstance instance,
        string host,
        int port,
        string mcastInterface)
    {
        _endpoint = endpoint;
        _instance = instance;
        _state = StateNotConnected;
        _mcastInterface = mcastInterface;
        _incoming = true;
        _port = port;

        try
        {
            _addr = Network.getAddressForServer(host, port, instance.protocolSupport(), instance.preferIPv6());

            _readEventArgs = new SocketAsyncEventArgs();
            _readEventArgs.RemoteEndPoint = _addr;
            _readEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(ioCompleted);

            _writeEventArgs = new SocketAsyncEventArgs();
            _writeEventArgs.RemoteEndPoint = _addr;
            _writeEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(ioCompleted);

            _fd = Network.createServerSocket(true, _addr.AddressFamily, instance.protocolSupport());
            setBufSize(-1, -1);
            Network.setBlock(_fd, false);
        }
        catch (Ice.LocalException)
        {
            _readEventArgs?.Dispose();
            _writeEventArgs?.Dispose();
            _fd = null;
            throw;
        }
    }

    private void setBufSize(int rcvSize, int sndSize)
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
                dfltSize = Network.getRecvBufferSize(_fd);
                sizeRequested = rcvSize;
                _rcvSize = dfltSize;
            }
            else
            {
                isSnd = true;
                direction = "send";
                prop = "Ice.UDP.SndSize";
                dfltSize = Network.getSendBufferSize(_fd);
                sizeRequested = sndSize;
                _sndSize = dfltSize;
            }

            //
            // Get property for buffer size if size not passed in.
            //
            if (sizeRequested == -1)
            {
                sizeRequested = _instance.properties().getPropertyAsIntWithDefault(prop, dfltSize);
            }
            //
            // Check for sanity.
            //
            if (sizeRequested < (_udpOverhead + Protocol.headerSize))
            {
                _instance.logger().warning("Invalid " + prop + " value of " + sizeRequested + " adjusted to " +
                                           dfltSize);
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
                // Warn if the size that was set is less than the requested size
                // and we have not already warned
                //
                if (sizeSet < sizeRequested)
                {
                    BufSizeWarnInfo winfo = _instance.getBufSizeWarn(Ice.UDPEndpointType.value);
                    if ((isSnd && (!winfo.sndWarn || winfo.sndSize != sizeRequested)) ||
                       (!isSnd && (!winfo.rcvWarn || winfo.rcvSize != sizeRequested)))
                    {
                        _instance.logger().warning("UDP " + direction + " buffer size: requested size of " +
                                                   sizeRequested + " adjusted to " + sizeSet);

                        if (isSnd)
                        {
                            _instance.setSndBufSizeWarn(Ice.UDPEndpointType.value, sizeRequested);
                        }
                        else
                        {
                            _instance.setRcvBufSizeWarn(Ice.UDPEndpointType.value, sizeRequested);
                        }
                    }
                }
            }
        }
    }

    internal void ioCompleted(object sender, SocketAsyncEventArgs e)
    {
        switch (e.LastOperation)
        {
            case SocketAsyncOperation.Receive:
            case SocketAsyncOperation.ReceiveFrom:
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

    private UdpEndpointI _endpoint;
    private readonly ProtocolInstance _instance;
    private int _state;
    private readonly bool _incoming;
    private int _rcvSize;
    private int _sndSize;
    private Socket _fd;
    private EndPoint _addr;
    private readonly EndPoint _sourceAddr;
    private IPEndPoint _mcastAddr;
    private EndPoint _peerAddr;
    private readonly string _mcastInterface;

    private readonly int _port;
    private bool _bound;

    private SocketAsyncEventArgs _writeEventArgs;
    private readonly SocketAsyncEventArgs _readEventArgs;
    private AsyncCallback _writeCallback;
    private AsyncCallback _readCallback;

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
