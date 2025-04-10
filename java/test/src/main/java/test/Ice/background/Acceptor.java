// Copyright (c) ZeroC, Inc.

package test.Ice.background;

import com.zeroc.Ice.ReadyCallback;

import java.nio.channels.ServerSocketChannel;

class Acceptor implements com.zeroc.Ice.Acceptor {
    @Override
    public ServerSocketChannel fd() {
        return _acceptor.fd();
    }

    @Override
    public void setReadyCallback(ReadyCallback callback) {
        // No need to for the ready callback.
    }

    @Override
    public void close() {
        _acceptor.close();
    }

    @Override
    public com.zeroc.Ice.EndpointI listen() {
        _endpoint = _endpoint.endpoint(_acceptor.listen());
        return _endpoint;
    }

    @Override
    public com.zeroc.Ice.Transceiver accept() {
        return new Transceiver(_configuration, _acceptor.accept());
    }

    @Override
    public String protocol() {
        return _acceptor.protocol();
    }

    @Override
    public String toString() {
        return _acceptor.toString();
    }

    @Override
    public String toDetailedString() {
        return _acceptor.toDetailedString();
    }

    public com.zeroc.Ice.Acceptor delegate() {
        return _acceptor;
    }

    Acceptor(EndpointI endpoint, Configuration configuration, com.zeroc.Ice.Acceptor acceptor) {
        _endpoint = endpoint;
        _configuration = configuration;
        _acceptor = acceptor;
    }

    private EndpointI _endpoint;
    private final com.zeroc.Ice.Acceptor _acceptor;
    private final Configuration _configuration;
}
