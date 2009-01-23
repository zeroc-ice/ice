// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
    // plug-in is initialized, therefore the application must define
    // the property Ice.InitPlugins=0, set the context, and finally
    // invoke initializePlugins on the PluginManager.
    //
    // When the application supplies its own SSL context, the
    // plug-in skips its normal property-based configuration.
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

    //
    // Obtain the certificate verifier object. Returns null if no
    // verifier is set.
    //
    CertificateVerifier getCertificateVerifier();

    //
    // Establish the password callback object. This should be
    // done before the plug-in is initialized.
    //
    void setPasswordCallback(PasswordCallback callback);

    //
    // Obtain the password callback object. Returns null if no
    // callback is set.
    //
    PasswordCallback getPasswordCallback();

    //
    // Supplies an input stream for the keystore. Calling this method
    // causes IceSSL to ignore the IceSSL.Keystore property.
    //
    void setKeystoreStream(java.io.InputStream stream);

    //
    // Supplies an input stream for the truststore. Calling this method
    // causes IceSSL to ignore the IceSSL.Truststore property. It is
    // legal to supply the same input stream as setKeystoreStream, in
    // which case IceSSL uses the certificates contained in the keystore.
    //
    void setTruststoreStream(java.io.InputStream stream);

    //
    // Add an input stream for the random number seed. You may call
    // this method multiple times if necessary.
    //
    void addSeedStream(java.io.InputStream stream);
}
