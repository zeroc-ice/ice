// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

public interface Plugin extends Ice.Plugin
{
    // 
    // Manually initialize the plugin. The application must set the property
    // IceSSL.DelayInit=1 in order to use this method.
    //
    // It is legal to pass null as either argument, in which case the plugin
    // obtains its certificates as directed by configuration properties.
    //
    void initialize(javax.net.ssl.SSLContext clientContext, javax.net.ssl.SSLContext serverContext);

    // 
    // Establish the certificate verifier object. This should be
    // done before any connections are established.
    //
    void setCertificateVerifier(CertificateVerifier verifier);
}
