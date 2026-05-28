// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.nio.channels.ServerSocketChannel;
import java.util.Set;

final class WSAcceptor implements Acceptor {
    @Override
    public ServerSocketChannel fd() {
        return _delegate.fd();
    }

    @Override
    public void setReadyCallback(ReadyCallback callback) {
        _delegate.setReadyCallback(callback);
    }

    @Override
    public void close() {
        _delegate.close();
    }

    @Override
    public EndpointI listen() {
        _endpoint = _endpoint.endpoint(_delegate.listen());
        return _endpoint;
    }

    @Override
    public Transceiver accept() {
        // WebSocket handshaking is performed in TransceiverI::initialize, since accept must not block.
        return new WSTransceiver(_instance, _delegate.accept(), _allowedOrigins);
    }

    @Override
    public String protocol() {
        return _delegate.protocol();
    }

    @Override
    public String toString() {
        return _delegate.toString();
    }

    @Override
    public String toDetailedString() {
        return _delegate.toDetailedString();
    }

    public Acceptor delegate() {
        return _delegate;
    }

    WSAcceptor(WSEndpoint endpoint, ProtocolInstance instance, Acceptor del, Set<String> allowedOrigins) {
        _endpoint = endpoint;
        _instance = instance;
        _delegate = del;
        _allowedOrigins = allowedOrigins;
    }

    private WSEndpoint _endpoint;
    private final ProtocolInstance _instance;
    private final Acceptor _delegate;
    private final Set<String> _allowedOrigins;
}
