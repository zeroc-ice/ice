// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

    javax.net.ssl.SSLEngine createSSLEngine(boolean incoming, java.net.InetSocketAddress peerAddr)
    {
        return _engine.createSSLEngine(incoming, peerAddr);
    }

    void traceConnection(java.nio.channels.SocketChannel fd, javax.net.ssl.SSLEngine engine, boolean incoming)
    {
        _engine.traceConnection(fd, engine, incoming);
    }

    void verifyPeer(NativeConnectionInfo info, java.nio.channels.SelectableChannel fd, String address)
    {
        _engine.verifyPeer(info, fd, address);
    }

    void trustManagerFailure(boolean incoming, java.security.cert.CertificateException ex)
        throws java.security.cert.CertificateException
    {
        _engine.trustManagerFailure(incoming, ex);
    }

    private SSLEngine _engine;
}
