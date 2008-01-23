// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
        bool verify(ConnectionInfo info);
    }

    //
    // A password callback is an alternate way of supplying the plugin with
    // passwords that avoids using plaintext configuration properties.
    //
    public interface PasswordCallback
    {
        //
        // Obtain the password necessary to access the private key associated with
        // the certificate in the given file. Return null if no password is necessary.
        //
        SecureString getPassword(string file);

        //
        // Obtain a password for a certificate being imported via an IceSSL.ImportCert
        // property. Return null if no password is necessary.
        //
        SecureString getImportPassword(string file);
    }

    abstract public class Plugin : Ice.Plugin
    {
        //
        // From Ice.Plugin.
        //
        abstract public void initialize();

        //
        // Specify the certificates to use for SSL connections. This
        // must be done before the plugin is initialized, therefore
        // the application must define the property Ice.InitPlugins=0,
        // set the certificates, and finally invoke initializePlugins
        // on the PluginManager.
        //
        // When the application supplies its own certificates, the
        // plugin skips its normal property-based configuration.
        //
        abstract public void setCertificates(X509Certificate2Collection certs);

        //
        // Establish the certificate verifier object. This should be
        // done before any connections are established.
        //
        abstract public void setCertificateVerifier(CertificateVerifier verifier);

        //
        // Obtain the certificate verifier object. Returns null if no verifier
        // is set.
        //
        abstract public CertificateVerifier getCertificateVerifier();

        //
        // Establish the password callback object. This should be
        // done before the plugin is initialized.
        //
        abstract public void setPasswordCallback(PasswordCallback callback);

        //
        // Obtain the password callback object. Returns null if no callback
        // is set.
        //
        abstract public PasswordCallback getPasswordCallback();

        //
        // This method is for internal use.
        //
        abstract public void destroy();
    }
}
