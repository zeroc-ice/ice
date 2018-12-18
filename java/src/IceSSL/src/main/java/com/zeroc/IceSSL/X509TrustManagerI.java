// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceSSL;

final class X509TrustManagerI implements javax.net.ssl.X509TrustManager
{
    X509TrustManagerI(SSLEngine engine, javax.net.ssl.X509TrustManager delegate)
    {
        _engine = engine;
        _delegate = delegate;
    }

    @Override
    public void
    checkClientTrusted(java.security.cert.X509Certificate[] chain, String authType)
        throws java.security.cert.CertificateException
    {
        //
        // Do not invoke the wrapped trust manager for anonymous ciphers.
        //
        // Possible values for authType are "DH_anon" and "ECDH_anon" (IBM JDK).
        //
        if(authType.indexOf("DH_anon") == -1)
        {
            try
            {
                _delegate.checkClientTrusted(chain, authType);
            }
            catch(java.security.cert.CertificateException ex)
            {
                _engine.trustManagerFailure(true, ex);
            }
        }
    }

    @Override
    public void
    checkServerTrusted(java.security.cert.X509Certificate[] chain, String authType)
        throws java.security.cert.CertificateException
    {
        //
        // Do not invoke the wrapped trust manager for anonymous ciphers.
        //
        // Possible values for authType are "DH_anon" and "ECDH_anon" (IBM JDK).
        //
        if(authType.indexOf("DH_anon") == -1)
        {
            try
            {
                _delegate.checkServerTrusted(chain, authType);
            }
            catch(java.security.cert.CertificateException ex)
            {
                _engine.trustManagerFailure(false, ex);
            }
        }
    }

    @Override
    public java.security.cert.X509Certificate[]
    getAcceptedIssuers()
    {
        //
        // This method is used to send CA names to the client as part of the CertificateRequest
        // message sent by the server to request a client certificate. We want the client to always
        // send its certificate so we don't provide any CAs here.
        //
        //return _delegate.getAcceptedIssuers();
        return EMPTY;
    }

    private SSLEngine _engine;
    private javax.net.ssl.X509TrustManager _delegate;
    private java.security.cert.X509Certificate[] EMPTY = new java.security.cert.X509Certificate[0];
}
