// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Security.Cryptography.X509Certificates;

namespace IceSSL
{
    /// <summary>
    /// Plug-in factories must implement this interface.
    /// </summary>
    public sealed class PluginFactory : Ice.PluginFactory
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
        public Ice.Plugin create(Ice.Communicator communicator, string name, string[] args)
        {
            return new PluginI(communicator);
        }
    }

    public sealed class PluginI : Plugin
    {
        public PluginI(Ice.Communicator communicator)
        {
            IceInternal.ProtocolPluginFacade facade = IceInternal.Util.getProtocolPluginFacade(communicator);

            _sharedInstance = new SharedInstance(facade);

            //
            // Register the endpoint factory. We have to do this now, rather than
            // in initialize, because the communicator may need to interpret
            // proxies before the plug-in is fully initialized.
            //
            facade.addEndpointFactory(
                new EndpointFactoryI(new Instance(_sharedInstance, IceSSL.EndpointType.value, "ssl")));
        }

        public override void initialize()
        {
            _sharedInstance.initialize();
        }

        public override void destroy()
        {
        }

        public override void setCertificates(X509Certificate2Collection certs)
        {
            _sharedInstance.setCertificates(certs);
        }

        public override void setCertificateVerifier(CertificateVerifier verifier)
        {
            _sharedInstance.setCertificateVerifier(verifier);
        }

        public override CertificateVerifier getCertificateVerifier()
        {
            return _sharedInstance.getCertificateVerifier();
        }

        public override void setPasswordCallback(PasswordCallback callback)
        {
            _sharedInstance.setPasswordCallback(callback);
        }

        public override PasswordCallback getPasswordCallback()
        {
            return _sharedInstance.getPasswordCallback();
        }

        private SharedInstance _sharedInstance;
    }
}
