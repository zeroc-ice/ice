// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/LoggerUtil.h>

#include <IceSSL/Exception.h>
#include <IceSSL/ClientContext.h>
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
        Trace out(_logger, _traceLevels->securityCat);

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
    SslTransceiverPtr transceiver = new SslClientTransceiver(plugin, socket, _certificateVerifier, ssl);

    transceiverSetup(transceiver, timeout);

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
    _passphraseRetriesProperty = "IceSSL.Client.Passphrase.Retries";
    _connectionHandshakeRetries = "IceSSL.Client.Handshake.Retries";
}
