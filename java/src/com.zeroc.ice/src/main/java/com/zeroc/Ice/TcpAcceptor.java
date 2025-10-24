// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.net.InetSocketAddress;
import java.nio.channels.ServerSocketChannel;

class TcpAcceptor implements Acceptor {
    @Override
    public ServerSocketChannel fd() {
        return _fd;
    }

    @Override
    public void setReadyCallback(ReadyCallback callback) {
        // No need to for the ready callback.
    }

    @Override
    public void close() {
        if (_fd != null) {
            Network.closeSocketNoThrow(_fd);
            _fd = null;
        }
    }

    @Override
    public EndpointI listen() {
        try {
            _addr = Network.doBind(_fd, _addr, _backlog);
        } catch (LocalException ex) {
            _fd = null;
            throw ex;
        }
        _endpoint = _endpoint.endpoint(this);
        return _endpoint;
    }

    @Override
    public Transceiver accept() {
        return new TcpTransceiver(_instance, new StreamSocket(_instance, Network.doAccept(_fd)));
    }

    @Override
    public String protocol() {
        return _instance.protocol();
    }

    @Override
    public String toString() {
        return Network.addrToString(_addr);
    }

    @Override
    public String toDetailedString() {
        StringBuffer s = new StringBuffer("local address = ");
        s.append(toString());
        return s.toString();
    }

    int effectivePort() {
        return _addr.getPort();
    }

    TcpAcceptor(TcpEndpointI endpoint, ProtocolInstance instance, String host, int port) {
        _endpoint = endpoint;
        _instance = instance;
        _backlog = instance.properties().getIcePropertyAsInt("Ice.TCP.Backlog");

        try {
            _fd = Network.createTcpServerSocket();
            Network.setBlock(_fd, false);
            Network.setTcpBufSize(_fd, instance);
            if (!System.getProperty("os.name").startsWith("Windows")) {
                // Set SO_REUSEADDR socket option on Unix platforms to allow re-using the address even if the socket
                // remains in the TIME_WAIT state. On Windows this isn't necessary.
                Network.setReuseAddress(_fd, true);
            }

            _addr = Network.getAddressForServer(host, port, instance.protocolSupport(), instance.preferIPv6());
        } catch (RuntimeException ex) {
            _fd = null;
            throw ex;
        }
    }

    private TcpEndpointI _endpoint;
    private final ProtocolInstance _instance;
    private ServerSocketChannel _fd;
    private final int _backlog;
    private InetSocketAddress _addr;
}
