// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

class Instance extends IceInternal.ProtocolInstance
{
    Instance(SharedInstance sharedInstance, short type, String protocol)
    {
        super(sharedInstance.communicator(), type, protocol);
        _sharedInstance = sharedInstance;
    }

    SharedInstance sharedInstance()
    {
        return _sharedInstance;
    }

    int securityTraceLevel()
    {
        return _sharedInstance.securityTraceLevel();
    }

    String securityTraceCategory()
    {
        return _sharedInstance.securityTraceCategory();
    }

    boolean initialized()
    {
        return _sharedInstance.initialized();
    }

    javax.net.ssl.SSLEngine createSSLEngine(boolean incoming, java.net.InetSocketAddress peerAddr)
    {
        return _sharedInstance.createSSLEngine(incoming, peerAddr);
    }

    void traceConnection(java.nio.channels.SocketChannel fd, javax.net.ssl.SSLEngine engine, boolean incoming)
    {
        _sharedInstance.traceConnection(fd, engine, incoming);
    }

    void verifyPeer(NativeConnectionInfo info, java.nio.channels.SelectableChannel fd, String address)
    {
        _sharedInstance.verifyPeer(info, fd, address);
    }

    void trustManagerFailure(boolean incoming, java.security.cert.CertificateException ex)
        throws java.security.cert.CertificateException
    {
        _sharedInstance.trustManagerFailure(incoming, ex);
    }

    private SharedInstance _sharedInstance;
}
