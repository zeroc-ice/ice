// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.SSL;

final class ConnectorI implements com.zeroc.Ice.Connector {
    @Override
    public com.zeroc.Ice.Transceiver connect() {
        SSLEngineFactory clientSSLEngineFactory =
                _instance.initializationData().clientSSLEngineFactory;
        if (clientSSLEngineFactory == null) {
            clientSSLEngineFactory =
                    (peerHost, peerPort) ->
                            _instance.engine().createSSLEngine(false, peerHost, peerPort);
        }
        return new TransceiverI(
                _instance, _delegate.connect(), _host, false, clientSSLEngineFactory);
    }

    @Override
    public short type() {
        return _delegate.type();
    }

    @Override
    public String toString() {
        return _delegate.toString();
    }

    @Override
    public int hashCode() {
        return _delegate.hashCode();
    }

    //
    // Only for use by EndpointI.
    //
    ConnectorI(Instance instance, com.zeroc.Ice.Connector delegate, String host) {
        _instance = instance;
        _delegate = delegate;
        _host = host;
    }

    @Override
    public boolean equals(java.lang.Object obj) {
        if (!(obj instanceof ConnectorI)) {
            return false;
        }

        if (this == obj) {
            return true;
        }

        ConnectorI p = (ConnectorI) obj;
        return p._delegate.equals(_delegate);
    }

    private final Instance _instance;
    private com.zeroc.Ice.Connector _delegate;
    private String _host;
}
