// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.IOException;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.PortUnreachableException;
import java.net.SocketAddress;
import java.nio.channels.AsynchronousCloseException;
import java.nio.channels.DatagramChannel;
import java.nio.channels.SelectableChannel;

final class UdpTransceiver implements Transceiver {
    @Override
    public SelectableChannel fd() {
        assert (_fd != null);
        return _fd;
    }

    @Override
    public void setReadyCallback(ReadyCallback callback) {}

    @Override
    public int initialize(Buffer readBuffer, Buffer writeBuffer) {
        // Nothing to do.
        return SocketOperation.None;
    }

    @Override
    public int closing(boolean initiator, LocalException ex) {
        // Nothing to do.
        return SocketOperation.None;
    }

    @Override
    public void close() {
        assert (_fd != null);

        try {
            _fd.close();
        } catch (IOException ex) {}
        _fd = null;
    }

    @Override
    public EndpointI bind() {
        if (_addr.getAddress().isMulticastAddress()) {
            // Set SO_REUSEADDR socket option to allow multiple sockets to bind to the same multicast address.
            Network.setReuseAddress(_fd, true);
            _mcastAddr = _addr;
            if (System.getProperty("os.name").startsWith("Windows")) {
                // Windows does not allow binding to the mcast address itself so we bind to INADDR_ANY instead.
                int protocolSupport = Network.getProtocolSupport(_mcastAddr);
                _addr = Network.getAddressForServer("", _port, protocolSupport, _instance.preferIPv6());
            }
            _addr = Network.doBind(_fd, _addr);
            if (_port == 0) {
                _mcastAddr = new InetSocketAddress(_mcastAddr.getAddress(), _addr.getPort());
            }
            Network.setMcastGroup(_fd, _mcastAddr, _mcastInterface);
        } else {
            _addr = Network.doBind(_fd, _addr);
        }

        _bound = true;
        _endpoint = _endpoint.endpoint(this);
        return _endpoint;
    }

    @Override
    public int write(Buffer buf) {
        if (!buf.b.hasRemaining()) {
            return SocketOperation.None;
        }

        assert (buf.b.position() == 0);
        assert (_fd != null && _state >= StateConnected);

        // The caller is supposed to check the send size before by calling checkSendSize
        assert (java.lang.Math.min(_maxPacketSize, _sndSize - _udpOverhead) >= buf.size());

        int ret = 0;
        while (true) {
            try {
                if (_state == StateConnected) {
                    ret = _fd.write(buf.b);
                } else {
                    if (_peerAddr == null) {
                        throw new SocketException(); // No peer has sent a datagram yet.
                    }
                    ret = _fd.send(buf.b, _peerAddr);
                }
                break;
            } catch (AsynchronousCloseException ex) {
                throw new ConnectionLostException(_addr, ex);
            } catch (PortUnreachableException ex) {
                throw new ConnectionLostException(_addr, ex);
            } catch (IOException ex) {
                throw new SocketException(ex);
            }
        }

        if (ret == 0) {
            return SocketOperation.Write;
        }

        assert (ret == buf.b.limit());
        buf.position(buf.b.limit());
        return SocketOperation.None;
    }

    @Override
    public int read(Buffer buf) {
        if (!buf.b.hasRemaining()) {
            return SocketOperation.None;
        }

        assert (buf.b.position() == 0);

        final int packetSize = java.lang.Math.min(_maxPacketSize, _rcvSize - _udpOverhead);
        buf.resize(packetSize, true);
        buf.position(0);

        int ret = 0;
        while (true) {
            try {
                SocketAddress peerAddr = _fd.receive(buf.b);
                if (peerAddr == null || buf.b.position() == 0) {
                    return SocketOperation.Read;
                }

                _peerAddr = (InetSocketAddress) peerAddr;
                ret = buf.b.position();
                break;
            } catch (AsynchronousCloseException ex) {
                throw new ConnectionLostException(_addr, ex);
            } catch (PortUnreachableException ex) {
                throw new ConnectionLostException(_addr, ex);
            } catch (IOException ex) {
                throw new ConnectionLostException(_addr, ex);
            }
        }

        // A client connection is already connected at this point, and a server connection is never connected.
        assert _state != StateNeedConnect;

        buf.resize(ret, true);
        buf.position(ret);

        return SocketOperation.None;
    }

    @Override
    public String protocol() {
        return _instance.protocol();
    }

    @Override
    public String toString() {
        if (_fd == null) {
            return "<closed>";
        }

        String s;
        if (_incoming && !_bound) {
            s = "local address = " + Network.addrToString(_addr);
        } else if (_state == StateNotConnected) {
            DatagramSocket socket = _fd.socket();
            s = "local address = " + Network.addrToString((InetSocketAddress) socket.getLocalSocketAddress());
        } else {
            s = Network.fdToString(_fd);
        }

        if (_mcastAddr != null) {
            s += "\nmulticast address = " + Network.addrToString(_mcastAddr);
        }
        return s;
    }

    @Override
    public String toDetailedString() {
        StringBuilder s = new StringBuilder(toString());
        if (_mcastAddr != null) {
            var intfs = Network.getInterfacesForMulticast(_mcastInterface, Network.getProtocolSupport(_mcastAddr));
            if (!intfs.isEmpty()) {
                s.append("\nlocal interfaces = ");
                s.append(String.join(", ", intfs));
            }
        }
        return s.toString();
    }

    @Override
    public ConnectionInfo getInfo(boolean incoming, String adapterName, String connectionId) {
        if (_fd == null) {
            return new UDPConnectionInfo(incoming, adapterName, connectionId);
        } else {
            DatagramSocket socket = _fd.socket();

            int rcvSize = 0;
            int sndSize = 0;
            if (!socket.isClosed()) {
                rcvSize = Network.getRecvBufferSize(_fd);
                sndSize = Network.getSendBufferSize(_fd);
            }

            if (_state == StateNotConnected) {
                assert _incoming;

                // Since this info is cached in the Connection object shared by all the clients,
                // we don't store the remote address/port of the latest client in this info.
                return new UDPConnectionInfo(
                    incoming,
                    adapterName,
                    connectionId,
                    socket.getLocalAddress().getHostAddress(),
                    socket.getLocalPort(),
                    "", // remoteAddress
                    -1, // remotePort
                    _mcastAddr != null ? _mcastAddr.getAddress().getHostAddress() : "",
                    _mcastAddr != null ? _mcastAddr.getPort() : -1,
                    rcvSize,
                    sndSize);
            } else {
                assert !_incoming;

                String remoteAddress = "";
                int remotePort = -1;
                if (socket.getInetAddress() != null) {
                    remoteAddress = socket.getInetAddress().getHostAddress();
                    remotePort = socket.getPort();
                }

                return new UDPConnectionInfo(
                    incoming,
                    adapterName,
                    connectionId,
                    socket.getLocalAddress().getHostAddress(),
                    socket.getLocalPort(),
                    remoteAddress,
                    remotePort,
                    _mcastAddr != null ? _mcastAddr.getAddress().getHostAddress() : "",
                    _mcastAddr != null ? _mcastAddr.getPort() : -1,
                    rcvSize,
                    sndSize);
            }
        }
    }

    @Override
    public synchronized void checkSendSize(Buffer buf) {
        // The maximum packetSize is either the maximum allowable UDP packet size, or the UDP send
        // buffer size (which ever is smaller).
        final int packetSize = java.lang.Math.min(_maxPacketSize, _sndSize - _udpOverhead);
        if (packetSize < buf.size()) {
            throw new DatagramLimitException(
                "message size of " + buf.size() + " exceeds the maximum packet size of " + packetSize);
        }
    }

    @Override
    public synchronized void setBufferSize(int rcvSize, int sndSize) {
        assert (_fd != null);

        // The default size is the size currently configured on the socket. We don't set the buffer size when the
        // requested size matches the default: re-setting it would needlessly grow the buffer on systems (e.g. Linux)
        // where the kernel doubles the value on each set.

        int rcvDefault = Network.getRecvBufferSize(_fd);
        rcvSize = adjustBufferSize(rcvSize, rcvDefault, "Ice.UDP.RcvSize");
        if (rcvSize == rcvDefault) {
            _rcvSize = rcvDefault;
        } else {
            // The kernel silently adjusts the size to an acceptable value, so read it back to get the size actually
            // set.
            Network.setRecvBufferSize(_fd, rcvSize);
            _rcvSize = Network.getRecvBufferSize(_fd);
            if (_rcvSize < rcvSize) {
                // The kernel reduced the requested size; warn unless we already warned for this size.
                BufSizeWarnInfo winfo = _instance.getBufSizeWarn(UDPEndpointType.value);
                if (!winfo.rcvWarn || winfo.rcvSize != rcvSize) {
                    _instance.logger().warning(
                        "UDP receive buffer size: requested size of " + rcvSize + " adjusted to " + _rcvSize);
                    _instance.setRcvBufSizeWarn(UDPEndpointType.value, rcvSize);
                }
            }
        }

        int sndDefault = Network.getSendBufferSize(_fd);
        sndSize = adjustBufferSize(sndSize, sndDefault, "Ice.UDP.SndSize");
        if (sndSize == sndDefault) {
            _sndSize = sndDefault;
        } else {
            Network.setSendBufferSize(_fd, sndSize);
            _sndSize = Network.getSendBufferSize(_fd);
            if (_sndSize < sndSize) {
                BufSizeWarnInfo winfo = _instance.getBufSizeWarn(UDPEndpointType.value);
                if (!winfo.sndWarn || winfo.sndSize != sndSize) {
                    _instance.logger().warning(
                        "UDP send buffer size: requested size of " + sndSize + " adjusted to " + _sndSize);
                    _instance.setSndBufSizeWarn(UDPEndpointType.value, sndSize);
                }
            }
        }
    }

    public final int effectivePort() {
        return _addr.getPort();
    }

    //
    // Only for use by UdpEndpoint
    //
    UdpTransceiver(
            ProtocolInstance instance,
            InetSocketAddress addr,
            InetSocketAddress sourceAddr,
            String mcastInterface,
            int mcastTtl) {
        _instance = instance;
        _state = StateNeedConnect;
        _addr = addr;

        int rcvSize = _instance.properties().getIcePropertyAsInt("Ice.UDP.RcvSize");
        int sndSize = _instance.properties().getIcePropertyAsInt("Ice.UDP.SndSize");

        try {
            _fd = Network.createUdpSocket(_addr);

            // Sets _rcvSize and _sndSize:
            setBufferSize(rcvSize, sndSize);
            assert _rcvSize >= _udpOverhead + Protocol.headerSize;
            assert _sndSize >= _udpOverhead + Protocol.headerSize;

            Network.setBlock(_fd, false);
            //
            // NOTE: setting the multicast interface before performing the
            // connect is important for some OS such as macOS.
            //
            if (_addr.getAddress().isMulticastAddress()) {
                if (!mcastInterface.isEmpty()) {
                    Network.setMcastInterface(_fd, mcastInterface);
                }
                if (mcastTtl != -1) {
                    Network.setMcastTtl(_fd, mcastTtl);
                }
            }
            Network.doConnect(_fd, _addr, sourceAddr);
            _state = StateConnected; // We're connected now
        } catch (LocalException ex) {
            if (_fd != null) {
                Network.closeSocketNoThrow(_fd);
                _fd = null;
            }
            throw ex;
        }
    }

    //
    // Only for use by UdpEndpoint
    //
    UdpTransceiver(
            UdpEndpointI endpoint,
            ProtocolInstance instance,
            InetSocketAddress addr,
            String mcastInterface) {
        _endpoint = endpoint;
        _instance = instance;
        _state = StateNotConnected;
        _mcastInterface = mcastInterface;
        _incoming = true;
        _addr = addr;
        _port = addr.getPort();

        int rcvSize = _instance.properties().getIcePropertyAsInt("Ice.UDP.RcvSize");
        int sndSize = _instance.properties().getIcePropertyAsInt("Ice.UDP.SndSize");

        try {
            _fd = Network.createUdpSocket(_addr);

            // Sets _rcvSize and _sndSize:
            setBufferSize(rcvSize, sndSize);
            assert _rcvSize >= _udpOverhead + Protocol.headerSize;
            assert _sndSize >= _udpOverhead + Protocol.headerSize;

            Network.setBlock(_fd, false);
        } catch (LocalException ex) {
            if (_fd != null) {
                Network.closeSocketNoThrow(_fd);
                _fd = null;
            }
            throw ex;
        }
    }

    private int adjustBufferSize(int sizeRequested, int defaultSize, String prop) {
        if (sizeRequested == 0) {
            // 0 (the property default) means we want the default size.
            return defaultSize;
        }

        if (sizeRequested < (_udpOverhead + Protocol.headerSize)) {
            _instance.logger().warning(
                "Invalid " + prop + " value of " + sizeRequested + " adjusted to " + defaultSize);
            return defaultSize;
        }

        return sizeRequested;
    }

    private UdpEndpointI _endpoint;
    private final ProtocolInstance _instance;

    private int _state;
    private int _rcvSize;
    private int _sndSize;
    private DatagramChannel _fd;
    private InetSocketAddress _addr;
    private InetSocketAddress _mcastAddr;
    private String _mcastInterface;
    private InetSocketAddress _peerAddr;

    private boolean _incoming;
    private int _port;
    private boolean _bound;

    //
    // The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for
    // the UDP header to get the maximum payload.
    //
    private static final int _udpOverhead = 20 + 8;
    private static final int _maxPacketSize = 65535 - _udpOverhead;

    private static final int StateNeedConnect = 0;
    private static final int StateConnected = 1;
    private static final int StateNotConnected = 2;
}
