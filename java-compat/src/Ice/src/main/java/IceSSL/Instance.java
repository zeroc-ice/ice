// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

class Instance extends IceInternal.ProtocolInstance
{
    Instance(SSLEngine engine, short type, String protocol)
    {
        super(engine.communicator(), type, protocol, true);
        _engine = engine;
    }

    SSLEngine engine()
    {
        return _engine;
    }

    int securityTraceLevel()
    {
        return _engine.securityTraceLevel();
    }

    String securityTraceCategory()
    {
        return _engine.securityTraceCategory();
    }

    boolean initialized()
    {
        return _engine.initialized();
    }

    javax.net.ssl.SSLEngine createSSLEngine(boolean incoming, String host, int port)
    {
        return _engine.createSSLEngine(incoming, host, port);
    }

    void traceConnection(String desc, javax.net.ssl.SSLEngine engine, boolean incoming)
    {
        _engine.traceConnection(desc, engine, incoming);
    }

    void verifyPeer(String address, ConnectionInfo info, String desc)
    {
        _engine.verifyPeer(address, info, desc);
    }

    void trustManagerFailure(boolean incoming, java.security.cert.CertificateException ex)
        throws java.security.cert.CertificateException
    {
        _engine.trustManagerFailure(incoming, ex);
    }

    private SSLEngine _engine;
}
