//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Security;
using System.Security.Cryptography.X509Certificates;
using Ice;

namespace IceSSL
{
    /// <summary>
    /// Plug-in factories must implement this interface.
    /// </summary>
    public sealed class PluginFactory : IPluginFactory
    {
        /// <summary>
        /// Returns a new plug-in.
        /// </summary>
        /// <param name="communicator">The communicator for the plug-in.</param>
        /// <param name="name">The name of the plug-in.</param>
        /// <param name="args">The arguments that are specified in the plug-in's configuration.</param>
        ///
        /// <returns>The new plug-in. null can be returned to indicate
        /// that a general error occurred. Alternatively, create can throw
        /// PluginInitializationException to provide more detailed information.</returns>
        public IPlugin create(Communicator communicator, string name, string[] args) => new Plugin(communicator);
    }

    //
    // An application can customize the certificate verification process
    // by implementing the CertificateVerifier interface.
    //
    public interface ICertificateVerifier
    {
        //
        // Return true to allow a connection using the provided certificate
        // information, or false to reject the connection.
        //
        bool verify(ConnectionInfo info);
    }

    /// <summary>
    /// A password callback is an alternate way of supplying the plug-in with
    /// passwords; this avoids using plain text configuration properties.
    /// </summary>
    public interface IPasswordCallback
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

    public sealed class Plugin : IPlugin
    {
        internal Plugin(Communicator communicator)
        {
            IceInternal.IProtocolPluginFacade facade = IceInternal.Util.getProtocolPluginFacade(communicator);

            _engine = new SSLEngine(facade);

            //
            // SSL based on TCP
            //
            Instance instance = new Instance(_engine, SSLEndpointType.value, "ssl");
            facade.addEndpointFactory(new EndpointFactoryI(instance, Ice.TCPEndpointType.value));
        }

        public void initialize()
        {
            _engine.initialize();
        }

        public void destroy()
        {
        }

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
        public void setCACertificates(X509Certificate2Collection certs)
        {
            _engine.setCACertificates(certs);
        }

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
        public void setCertificates(X509Certificate2Collection certs)
        {
            _engine.setCertificates(certs);
        }

        /// <summary>
        /// Establish the certificate verifier object. This must be
        /// done before any connections are established.
        /// </summary>
        /// <param name="verifier">The certificate verifier.</param>
        public void setCertificateVerifier(ICertificateVerifier verifier)
        {
            _engine.setCertificateVerifier(verifier);
        }

        /// <summary>
        /// Obtain the certificate verifier object.
        /// </summary>
        /// <returns>The certificate verifier (null if not set).</returns>
        public ICertificateVerifier? getCertificateVerifier()
        {
            return _engine.getCertificateVerifier();
        }

        /// <summary>
        /// Establish the password callback object. This must be
        /// done before the plug-in is initialized.
        /// </summary>
        /// <param name="callback">The password callback.</param>
        public void setPasswordCallback(IPasswordCallback callback)
        {
            _engine.setPasswordCallback(callback);
        }

        /// <summary>
        /// Returns the password callback.
        /// </summary>
        /// <returns>The password callback (null if not set).</returns>
        public IPasswordCallback? getPasswordCallback()
        {
            return _engine.getPasswordCallback();
        }

        private SSLEngine _engine;
    }

    public static class Util
    {
        public static void
        registerIceSSL(bool loadOnInitialize)
        {
            Communicator.RegisterPluginFactory("IceSSL", new PluginFactory(), loadOnInitialize);
        }
    }
}
