// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SslException.h>
#include <Ice/SslConnectionOpenSSL.h>
#include <Ice/ContextOpenSSLClient.h>
#include <Ice/SslConnectionOpenSSLClient.h>

#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>

#include <iostream>

using IceSSL::ConnectionPtr;
using IceSSL::SystemInternalPtr;

void
IceSSL::OpenSSL::ClientContext::configure(const GeneralConfig& generalConfig,
                                          const CertificateAuthority& certificateAuthority,
                                          const BaseCertificates& baseCertificates)
{
    Context::configure(generalConfig, certificateAuthority, baseCertificates);

    loadCertificateAuthority(certificateAuthority);

    if (_traceLevels->security >= IceSSL::SECURITY_PROTOCOL)
    {
        std::ostringstream s;

        s << std::endl;
        s << "general configuration (client)" << std::endl;
        s << "------------------------------" << std::endl;
        s << generalConfig << std::endl << std::endl;

        s << "certificate authority (client)" << std::endl;
        s << "------------------------------" << std::endl;
        s << "file: " << certificateAuthority.getCAFileName() << std::endl;
        s << "path: " << certificateAuthority.getCAPath() << std::endl;

        s << "base certificates (client)" << std::endl;
        s << "--------------------------" << std::endl;
        s << baseCertificates << std::endl;

        _logger->trace(_traceLevels->securityCat, s.str());
    }
}

IceSSL::ConnectionPtr
IceSSL::OpenSSL::ClientContext::createConnection(int socket, const SystemInternalPtr& system)
{
    if (_sslContext == 0)
    {
        IceSSL::OpenSSL::ContextNotConfiguredException contextEx(__FILE__, __LINE__);

        throw contextEx;
    }

    ConnectionPtr connection = new ClientConnection(_traceLevels,
                                                    _logger,
                                                    _certificateVerifier,
                                                    createSSLConnection(socket),
                                                    system);

    connectionSetup(connection);

    return connection;
}

IceSSL::OpenSSL::ClientContext::ClientContext(const IceInternal::InstancePtr& instance) :
                               Context(instance)
{
    _rsaPrivateKeyProperty = "Ice.SSL.Client.Overrides.RSA.PrivateKey";
    _rsaPublicKeyProperty  = "Ice.SSL.Client.Overrides.RSA.Certificate";
    _dsaPrivateKeyProperty = "Ice.SSL.Client.Overrides.DSA.PrivateKey";
    _dsaPublicKeyProperty  = "Ice.SSL.Client.Overrides.DSA.Certificate";
    _caCertificateProperty = "Ice.SSL.Client.Overrides.CACertificate";
    _handshakeTimeoutProperty = "Ice.SSL.Client.Handshake.ReadTimeout";
}

