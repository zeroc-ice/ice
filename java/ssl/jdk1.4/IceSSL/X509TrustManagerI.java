// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class X509TrustManagerI implements javax.net.ssl.X509TrustManager
{
    X509TrustManagerI(javax.net.ssl.X509TrustManager delegate)
    {
        _delegate = delegate;
    }

    public void
    checkClientTrusted(java.security.cert.X509Certificate[] chain, String authType)
        throws java.security.cert.CertificateException
    {
        if(!authType.equals("DH_anon") && _delegate != null)
        {
            _delegate.checkClientTrusted(chain, authType);
        }
    }

    public void
    checkServerTrusted(java.security.cert.X509Certificate[] chain, String authType)
        throws java.security.cert.CertificateException
    {
        if(!authType.equals("DH_anon") && _delegate != null)
        {
            _delegate.checkServerTrusted(chain, authType);
        }
    }

    public java.security.cert.X509Certificate[]
    getAcceptedIssuers()
    {
        if(_delegate != null)
        {
            return _delegate.getAcceptedIssuers();
        }
        return null;
    }

    private javax.net.ssl.X509TrustManager _delegate;
}
