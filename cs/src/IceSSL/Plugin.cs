// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
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
        // This method is for internal use.
        //
        abstract public void destroy();
    }
}
