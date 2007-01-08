// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

public interface Plugin extends Ice.Plugin
{
    //
    // Establish the SSL context. This must be done before the
    // plugin is initialized, therefore the application must define
    // the property Ice.InitPlugins=0, set the context, and finally
    // invoke initializePlugins on the PluginManager.
    //
    // When the application supplies its own SSL context, the
    // plugin skips its normal property-based configuration.
    // 
    void setContext(javax.net.ssl.SSLContext context);

    //
    // Obtain the SSL context. Use caution when modifying this value.
    // Changes made to this value have no effect on existing connections.
    //
    javax.net.ssl.SSLContext getContext();

    // 
    // Establish the certificate verifier object. This should be
    // done before any connections are established.
    //
    void setCertificateVerifier(CertificateVerifier verifier);
}
