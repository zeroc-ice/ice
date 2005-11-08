// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <Ice/LoggerUtil.h>

#include <IceSSL/Exception.h>
#include <IceSSL/ClientContext.h>
#include <IceSSL/SslTransceiver.h>
#include <IceSSL/TraceLevels.h>

using namespace std;
using namespace Ice;

void
IceSSL::ClientContext::configure(const GeneralConfig& generalConfig,
                                 const CertificateAuthority& certificateAuthority,
                                 const BaseCertificates& baseCertificates)
{
    Context::configure(generalConfig, certificateAuthority, baseCertificates);

    loadCertificateAuthority(certificateAuthority);

    if(_traceLevels->security >= SECURITY_PROTOCOL)
    {
        Trace out(_communicator->getLogger(), _traceLevels->securityCat);

        out << "\n";
        out << "general configuration (client)\n";
        out << "------------------------------\n";
        IceSSL::operator<<(out,  generalConfig);
        out << "\n\n";

        out << "certificate authority (client)\n";
        out << "------------------------------\n";
        out << "file: " << certificateAuthority.getCAFileName() << "\n";
        out << "path: " << certificateAuthority.getCAPath() << "\n";

        out << "base certificates (client)\n";
        out << "--------------------------\n";
        IceSSL::operator<<(out, baseCertificates);
        out << "\n";
    }
}

IceSSL::SslTransceiverPtr
IceSSL::ClientContext::createTransceiver(int socket, const OpenSSLPluginIPtr& plugin, int timeout)
{
    if(_sslContext == 0)
    {
        ContextNotConfiguredException contextEx(__FILE__, __LINE__);

        throw contextEx;
    }

    SSL* ssl = createSSLConnection(socket);
    return new SslTransceiver(IceSSL::Client, plugin, socket, _certificateVerifier, ssl, timeout);
}

IceSSL::ClientContext::ClientContext(const TraceLevelsPtr& traceLevels, const CommunicatorPtr& communicator) :
    Context(traceLevels, communicator, Client)
{
    _rsaPrivateKeyProperty = "IceSSL.Client.Overrides.RSA.PrivateKey";
    _rsaPublicKeyProperty  = "IceSSL.Client.Overrides.RSA.Certificate";
    _dsaPrivateKeyProperty = "IceSSL.Client.Overrides.DSA.PrivateKey";
    _dsaPublicKeyProperty  = "IceSSL.Client.Overrides.DSA.Certificate";
    _caCertificateProperty = "IceSSL.Client.Overrides.CACertificate";
    _passphraseRetriesProperty = "IceSSL.Client.Passphrase.Retries";
}
