//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Security;
using System.Security.Cryptography.X509Certificates;

using ZeroC.Ice;

namespace ZeroC.IceSSL
{
    /// <summary>Plug-in factories must implement this interface.</summary>
    public sealed class PluginFactory : IPluginFactory
    {
        /// <summary>Returns a new plug-in.</summary>
        /// <param name="communicator">The communicator for the plug-in.</param>
        /// <param name="name">The name of the plug-in.</param>
        /// <param name="args">The arguments that are specified in the plug-in's configuration.</param>
        /// <returns>The new plug-in. null can be returned to indicate that a general error occurred. Alternatively,
        /// create can throw an exception to provide more detailed information.</returns>
        public IPlugin Create(Communicator communicator, string name, string[] args) => new Plugin(communicator);

        public static void Register(bool loadOnInitialize) =>
            Communicator.RegisterPluginFactory("IceSSL", new PluginFactory(), loadOnInitialize);
    }

    /// <summary>The ICertificateVerifier allows an application to customize the certificate verification process.
    /// </summary>
    public interface ICertificateVerifier
    {
        /// <summary>Return true to allow a connection using the provided certificate information, or false to reject
        /// the connection.</summary>
        /// <param name="info">The connection info associated with the connection being verify.</param>
        /// <returns>Return true to allow the connection, or false to reject it.</returns>
        bool Verify(ConnectionInfo info);
    }

    /// <summary>The IPasswordCallback interface provides application a way of supplying the plug-in with passwords;
    /// this avoids using plain text configuration properties.</summary>
    public interface IPasswordCallback
    {
        /// <summary>Obtain the password necessary to access the private key associated with the certificate in the
        /// given file.</summary>
        /// <param name="file">The certificate file name.</param>
        /// <returns>The password for the key or null, if no password is necessary.</returns>
        SecureString GetPassword(string file);

        /// <summary>Obtain a password for a certificate being imported via an IceSSL.ImportCert property. Return null
        /// if no password is necessary.</summary>
        /// <param name="file">The certificate file name.</param>
        /// <returns>The password for the key or null, if no password is necessary.</returns>
        SecureString GetImportPassword(string file);
    }

    public sealed class Plugin : IPlugin
    {
        internal Plugin(Communicator communicator)
        {
            ITransportPluginFacade facade = Util.GetTransportPluginFacade(communicator);

            _engine = new SSLEngine(facade);

            //
            // SSL based on TCP
            //
            var instance = new Instance(_engine, EndpointType.SSL, "ssl");
            facade.AddEndpointFactory(new EndpointFactoryI(instance, EndpointType.TCP));
        }

        public void Initialize() => _engine.Initialize();

        public void Destroy()
        {
        }

        /// <summary>Specify the certificate authorities certificates to use when validating SSL peer certificates.
        /// This must be done before the plug-in is initialized; therefore, the application must define the property
        /// Ice.InitPlugins=0, set the certificates, and finally invoke initializePlugins on the communicator. When
        /// the application supplies its own certificate authorities certificates, the plug-in skips its normal
        /// property-based configuration.</summary>
        /// <param name="certs">The certificate authorities certificates to use.</param>
        public void SetCACertificates(X509Certificate2Collection certs) => _engine.SetCACertificates(certs);

        /// <summary>Specify the certificates to use for SSL connections. This must be done before the plug-in is
        /// initialized; therefore, the application must define the property Ice.InitPlugins=0, set the certificates,
        /// and finally invoke initializePlugins on the communicator. When the application supplies its own
        /// certificates, the plug-in skips its normal property-based configuration.</summary>
        /// <param name="certs">The certificates to use for SSL connections.</param>
        public void SetCertificates(X509Certificate2Collection certs) => _engine.SetCertificates(certs);

        /// <summary>Establish the certificate verifier object. This must be done before any connections are
        /// established.</summary>
        /// <param name="verifier">The certificate verifier.</param>
        public void SetCertificateVerifier(ICertificateVerifier verifier) => _engine.SetCertificateVerifier(verifier);

        /// <summary>Obtain the certificate verifier object.</summary>
        /// <returns>The certificate verifier (null if not set).</returns>
        public ICertificateVerifier? GetCertificateVerifier() => _engine.GetCertificateVerifier();

        /// <summary>Establish the password callback object. This must be done before the plug-in is initialized.
        /// </summary>
        /// <param name="callback">The password callback.</param>
        public void SetPasswordCallback(IPasswordCallback callback) => _engine.SetPasswordCallback(callback);

        /// <summary>Returns the password callback.</summary>
        /// <returns>The password callback (null if not set).</returns>
        public IPasswordCallback? GetPasswordCallback() => _engine.GetPasswordCallback();

        private readonly SSLEngine _engine;
    }
}
