// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

/**
 * Interface that allows applications to interact with the IceSSL plug-in.
 **/
public interface Plugin extends Ice.Plugin
{
    /**
     * Establishes the SSL context. The context must be established before
     * plug-in is initialized. Therefore, the application must set
     * the property <code>Ice.InitPlugins</code> to zero, call
     * <code>setContext</code> to set the context, and finally
     * invoke {@link Ice.PluginManager#initializePlugins}.
     * <p>
     * If an application supplies its own SSL context, the
     * plug-in skips its normal property-based configuration.
     *
     * @param context The SSL context for the plug-in.
     **/
    void setContext(javax.net.ssl.SSLContext context);

    /**
     * Returns the SSL context. Use caution when modifying the returned
     * value: changes made to this value do not affect existing connections.
     *
     * @return The SSL context for the plug-in.
     **/
    javax.net.ssl.SSLContext getContext();

    /**
     * Establishes the certificate verifier. This must be
     * done before any connections are established.
     *
     * @param verifier The certificate verifier.
     **/
    void setCertificateVerifier(CertificateVerifier verifier);

    /**
     * Returns the certificate verifier.
     *
     * @return The certificate verifier (<code>null</code> if not set).
     **/
    CertificateVerifier getCertificateVerifier();

    /**
     * Establishes the password callback. This must be
     * done before the plug-in is initialized.
     *
     * @param callback The password callback.
     **/
    void setPasswordCallback(PasswordCallback callback);

    /**
     * Returns the password callback.
     *
     * @return The password callback (<code>null</code> if not set).
     **/
    PasswordCallback getPasswordCallback();

    /**
     * Supplies an input stream for the keystore. Calling this method
     * causes IceSSL to ignore the <code>IceSSL.Keystore</code> property.
     *
     * @param stream The input stream for the keystore.
     **/
    void setKeystoreStream(java.io.InputStream stream);

    /**
     * Supplies an input stream for the truststore. Calling this method
     * causes IceSSL to ignore the <code>IceSSL.Truststore</code> property. It is
     * legal to supply the same input stream as the one for {@link #setKeystoreStream},
     * in which case IceSSL uses the certificates contained in the keystore.
     *
     * @param stream The input stream for the truststore.
     **/
    void setTruststoreStream(java.io.InputStream stream);

    /**
     * Adds an input stream for the random number seed. You may call
     * this method multiple times if necessary.
     *
     * @param stream The input stream for the random number seed.
     **/
    void addSeedStream(java.io.InputStream stream);
}
