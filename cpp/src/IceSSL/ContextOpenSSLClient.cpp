// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Logger.h>

#include <IceSSL/Exception.h>
#include <IceSSL/SslConnectionOpenSSL.h>
#include <IceSSL/ContextOpenSSLClient.h>
#include <IceSSL/SslConnectionOpenSSLClient.h>
#include <IceSSL/TraceLevels.h>

using namespace std;
using namespace Ice;

using IceSSL::ConnectionPtr;

void
IceSSL::OpenSSL::ClientContext::configure(const GeneralConfig& generalConfig,
                                          const CertificateAuthority& certificateAuthority,
                                          const BaseCertificates& baseCertificates)
{
    Context::configure(generalConfig, certificateAuthority, baseCertificates);

    loadCertificateAuthority(certificateAuthority);

    if (_traceLevels->security >= IceSSL::SECURITY_PROTOCOL)
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

IceSSL::ConnectionPtr
IceSSL::OpenSSL::ClientContext::createConnection(int socket, const PluginBaseIPtr& plugin)
{
    if (_sslContext == 0)
    {
        IceSSL::OpenSSL::ContextNotConfiguredException contextEx(__FILE__, __LINE__);

        throw contextEx;
    }

    ConnectionPtr connection = new ClientConnection(_certificateVerifier, createSSLConnection(socket), plugin);

    connectionSetup(connection);

    return connection;
}

IceSSL::OpenSSL::ClientContext::ClientContext(const IceSSL::TraceLevelsPtr& traceLevels, const LoggerPtr& logger,
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
