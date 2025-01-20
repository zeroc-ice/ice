// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.SSL;

import com.zeroc.Ice.InitializationData;

public class Instance extends com.zeroc.Ice.ProtocolInstance {
    public Instance(SSLEngine engine, short type, String protocol) {
        super(engine.communicator(), type, protocol, true);
        _engine = engine;
    }

    SSLEngine engine() {
        return _engine;
    }

    int securityTraceLevel() {
        return _engine.securityTraceLevel();
    }

    String securityTraceCategory() {
        return _engine.securityTraceCategory();
    }

    javax.net.ssl.SSLEngine createSSLEngine(boolean incoming, String host, int port) {
        return _engine.createSSLEngine(incoming, host, port);
    }

    void traceConnection(String desc, javax.net.ssl.SSLEngine engine, boolean incoming) {
        _engine.traceConnection(desc, engine, incoming);
    }

    void verifyPeer(String address, ConnectionInfo info, String desc) {
        _engine.verifyPeer(address, info, desc);
    }

    void trustManagerFailure(boolean incoming, java.security.cert.CertificateException ex)
            throws java.security.cert.CertificateException {
        _engine.trustManagerFailure(incoming, ex);
    }

    InitializationData initializationData() {
        return _engine.communicator().getInstance().initializationData();
    }

    private SSLEngine _engine;
}
