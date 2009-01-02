// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Security.Cryptography.X509Certificates;

namespace IceSSL
{
    public sealed class PluginFactory : Ice.PluginFactory
    {
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
