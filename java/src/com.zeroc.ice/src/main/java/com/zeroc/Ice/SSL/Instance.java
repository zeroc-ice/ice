// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.SSL;

import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ProtocolInstance;

/**
 * @hidden Public because it's used by Ice.Instance
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

    void traceConnection(String desc, javax.net.ssl.SSLEngine engine, boolean incoming) {
        _engine.traceConnection(desc, engine, incoming);
    }

    void verifyPeer(String address, ConnectionInfo info, String desc) {
        _engine.verifyPeer(address, info, desc);
    }

    InitializationData initializationData() {
        return _engine.communicator().getInstance().initializationData();
    }

    private final SSLEngine _engine;
}
