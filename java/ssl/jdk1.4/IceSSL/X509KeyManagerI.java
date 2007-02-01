// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class X509KeyManagerI implements javax.net.ssl.X509KeyManager
{
    X509KeyManagerI(javax.net.ssl.X509KeyManager del, String alias)
    {
        _delegate = del;
        _alias = alias;
    }

    public String
    chooseClientAlias(String[] keyType, java.security.Principal[] issuers, java.net.Socket socket)
    {
        return _alias;
    }

    public String
    chooseServerAlias(String keyType, java.security.Principal[] issuers, java.net.Socket socket)
    {
        return _alias;
    }

    public java.security.cert.X509Certificate[]
    getCertificateChain(String alias)
    {
        return _delegate.getCertificateChain(alias); 
    }

    public String[]
    getClientAliases(String keyType, java.security.Principal[] issuers)
    {
        return _delegate.getClientAliases(keyType, issuers);
    }

    public String[]
    getServerAliases(String keyType, java.security.Principal[] issuers)
    {
        return _delegate.getServerAliases(keyType, issuers);
    }

    public java.security.PrivateKey
    getPrivateKey(String alias)
    {
        return _delegate.getPrivateKey(alias);
    }

    private javax.net.ssl.X509KeyManager _delegate;
    private String _alias;
}
