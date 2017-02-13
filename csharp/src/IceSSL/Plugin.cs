// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    using System.Security;
    using System.Security.Cryptography.X509Certificates;

    //
    // An application can customize the certificate verification process
    // by implementing the CertificateVerifier interface.
    //
    public interface CertificateVerifier
    {
        //
        // Return true to allow a connection using the provided certificate
        // information, or false to reject the connection.
        //
        bool verify(NativeConnectionInfo info);
    }

    /// <summary>
    /// A password callback is an alternate way of supplying the plug-in with
    /// passwords; this avoids using plain text configuration properties.
    /// </summary>
    public interface PasswordCallback
    {
        /// <summary>
        /// Obtain the password necessary to access the private key associated with
        /// the certificate in the given file.
        /// <param name="file">The certificate file name.</param>
        /// <returns>The password for the key or null, if no password is necessary.</returns>
        /// </summary>
        SecureString getPassword(string file);

        /// <summary>
        /// Obtain a password for a certificate being imported via an IceSSL.ImportCert
        /// property. Return null if no password is necessary.
        /// </summary>
        /// <param name="file">The certificate file name.</param>
        /// <returns>The password for the key or null, if no password is necessary.</returns>
        SecureString getImportPassword(string file);
    }

    /// <summary>
    /// Interface that allows applications to interact with the IceSSL plug-in.
    /// </summary>
    abstract public class Plugin : Ice.Plugin
    {
        abstract public void initialize();

        /// <summary>
        /// Specify the certificate authorities certificates to use 
        /// when validating SSL peer certificates. This must be done
        /// before the plug-in is initialized; therefore, the application 
        /// must define the property Ice.InitPlugins=0, set the certificates, 
        /// and finally invoke initializePlugins on the PluginManager.
        /// When the application supplies its own certificate authorities 
        /// certificates, the plug-in skips its normal property-based 
        /// configuration.
        /// </summary>
        /// <param name="certs">The certificate authorities certificates to use.</param>
        abstract public void setCACertificates(X509Certificate2Collection certs);

        /// <summary>
        /// Specify the certificates to use for SSL connections. This
        /// must be done before the plug-in is initialized; therefore,
        /// the application must define the property Ice.InitPlugins=0,
        /// set the certificates, and finally invoke initializePlugins
        /// on the PluginManager.
        /// When the application supplies its own certificates, the
        /// plug-in skips its normal property-based configuration.
        /// </summary>
        /// <param name="certs">The certificates to use for SSL connections.</param>
        abstract public void setCertificates(X509Certificate2Collection certs);

        /// <summary>
        /// Establish the certificate verifier object. This must be
        /// done before any connections are established.
        /// </summary>
        /// <param name="verifier">The certificate verifier.</param>
        abstract public void setCertificateVerifier(CertificateVerifier verifier);

        /// <summary>
        /// Obtain the certificate verifier object.
        /// </summary>
        /// <returns>The certificate verifier (null if not set).</returns>
        abstract public CertificateVerifier getCertificateVerifier();

        /// <summary>
        /// Establish the password callback object. This must be
        /// done before the plug-in is initialized.
        /// </summary>
        /// <param name="callback">The password callback.</param>
        abstract public void setPasswordCallback(PasswordCallback callback);

        /// <summary>
        /// Returns the password callback.
        /// </summary>
        /// <returns>The password callback (null if not set).</returns>
        abstract public PasswordCallback getPasswordCallback();

        /// <summary>
        /// This method is for internal use.
        /// </summary>
        abstract public void destroy();
    }
}
