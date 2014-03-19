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
        public Ice.Plugin
        create(Ice.Communicator communicator, string name, string[] args)
        {
            return new PluginI(communicator);
        }
    }

    public sealed class PluginI : Plugin
    {
        public
        PluginI(Ice.Communicator communicator)
        {
            instance_ = new Instance(communicator);
        }

        public override void initialize()
        {
            instance_.initialize();
        }

        public override void
        destroy()
        {
        }

        public override void setCertificates(X509Certificate2Collection certs)
        {
            instance_.setCertificates(certs);
        }

        public override void setCertificateVerifier(CertificateVerifier verifier)
        {
            instance_.setCertificateVerifier(verifier);
        }

        public override CertificateVerifier getCertificateVerifier()
        {
            return instance_.getCertificateVerifier();
        }

        public override void setPasswordCallback(PasswordCallback callback)
        {
            instance_.setPasswordCallback(callback);
        }

        public override PasswordCallback getPasswordCallback()
        {
            return instance_.getPasswordCallback();
        }

        private Instance instance_;
    }
}
