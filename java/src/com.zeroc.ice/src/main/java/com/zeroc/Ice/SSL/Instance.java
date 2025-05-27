// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.SSL;

import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ProtocolInstance;

import java.security.cert.CertificateException;

/**
 * @hidden Used by Ice
 */
public class Instance extends ProtocolInstance {
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

    void trustManagerFailure(boolean incoming, CertificateException ex)
        throws CertificateException {
        _engine.trustManagerFailure(incoming, ex);
    }

    InitializationData initializationData() {
        return _engine.communicator().getInstance().initializationData();
    }

    private final SSLEngine _engine;
}
