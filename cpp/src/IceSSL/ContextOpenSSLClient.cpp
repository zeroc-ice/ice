// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Logger.h>

#include <IceSSL/Exception.h>
#include <IceSSL/ContextOpenSSLClient.h>
#include <IceSSL/SslClientTransceiver.h>
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
        ostringstream s;

        s << endl;
        s << "general configuration (client)" << endl;
        s << "------------------------------" << endl;
        s << generalConfig << endl << endl;

        s << "certificate authority (client)" << endl;
        s << "------------------------------" << endl;
        s << "file: " << certificateAuthority.getCAFileName() << endl;
        s << "path: " << certificateAuthority.getCAPath() << endl;

        s << "base certificates (client)" << endl;
        s << "--------------------------" << endl;
        s << baseCertificates << endl;

        _logger->trace(_traceLevels->securityCat, s.str());
    }
}

IceSSL::SslTransceiverPtr
IceSSL::ClientContext::createTransceiver(int socket, const PluginBaseIPtr& plugin)
{
    if(_sslContext == 0)
    {
        OpenSSL::ContextNotConfiguredException contextEx(__FILE__, __LINE__);

        throw contextEx;
    }

    SSL* ssl = createSSLConnection(socket);
    SslTransceiverPtr transceiver = new SslClientTransceiver(plugin, socket, _certificateVerifier, ssl);

    transceiverSetup(transceiver);

    return transceiver;
}

IceSSL::ClientContext::ClientContext(const TraceLevelsPtr& traceLevels, const LoggerPtr& logger,
                                     const PropertiesPtr& properties) :
    Context(traceLevels, logger, properties)
{
    _rsaPrivateKeyProperty = "IceSSL.Client.Overrides.RSA.PrivateKey";
    _rsaPublicKeyProperty  = "IceSSL.Client.Overrides.RSA.Certificate";
    _dsaPrivateKeyProperty = "IceSSL.Client.Overrides.DSA.PrivateKey";
    _dsaPublicKeyProperty  = "IceSSL.Client.Overrides.DSA.Certificate";
    _caCertificateProperty = "IceSSL.Client.Overrides.CACertificate";
    _handshakeTimeoutProperty = "IceSSL.Client.Handshake.ReadTimeout";
    _passphraseRetriesProperty = "IceSSL.Client.Passphrase.Retries";
}
