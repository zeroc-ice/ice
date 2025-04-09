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
                //
                // Enable SO_REUSEADDR on Unix platforms to allow re-using the
                // socket even if it's in the TIME_WAIT state. On Windows,
                // this doesn't appear to be necessary and enabling
                // SO_REUSEADDR would actually not be a good thing since it
                // allows a second process to bind to an address even it's
                // already bound by another process.
                //
                // TODO: using SO_EXCLUSIVEADDRUSE on Windows would probably
                // be better but it's only supported by recent Windows
                // versions (XP SP2, Windows Server 2003).
                //
                Network.setReuseAddress(_fd, true);
            }

            _addr =
                Network.getAddressForServer(
                    host, port, instance.protocolSupport(), instance.preferIPv6());
        } catch (RuntimeException ex) {
            _fd = null;
            throw ex;
        }
    }

    @SuppressWarnings("nofinalizer")
    @Override
    protected synchronized void finalize() throws Throwable {
        try {
            Assert.FinalizerAssert(_fd == null);
        } catch (Exception ex) {
        } finally {
            super.finalize();
        }
    }

    private TcpEndpointI _endpoint;
    private final ProtocolInstance _instance;
    private ServerSocketChannel _fd;
    private final int _backlog;
    private InetSocketAddress _addr;
}
