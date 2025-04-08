// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.SSL;

import com.zeroc.Ice.Acceptor;
import com.zeroc.Ice.ReadyCallback;
import com.zeroc.Ice.Transceiver;

import java.nio.channels.ServerSocketChannel;

final class AcceptorI implements Acceptor {
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
    public com.zeroc.Ice.EndpointI listen() {
        _endpoint = _endpoint.endpoint(_delegate.listen());
        return _endpoint;
    }

    @Override
    public Transceiver accept() {
        return new TransceiverI(
            _instance, _delegate.accept(), _adapterName, true, _sslEngineFactory);
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

    AcceptorI(
            EndpointI endpoint,
            Instance instance,
            Acceptor delegate,
            String adapterName,
            SSLEngineFactory sslEngineFactory) {
        _endpoint = endpoint;
        _instance = instance;
        _delegate = delegate;
        _adapterName = adapterName;
        if (sslEngineFactory == null) {
            _sslEngineFactory =
                (peerHost, peerPort) ->
                    instance.engine().createSSLEngine(true, peerHost, peerPort);
        } else {
            _sslEngineFactory = sslEngineFactory;
        }
    }

    private EndpointI _endpoint;
    private final Instance _instance;
    private final Acceptor _delegate;
    private final String _adapterName;
    private SSLEngineFactory _sslEngineFactory;
}
