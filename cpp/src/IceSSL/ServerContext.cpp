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
#include <IceSSL/ServerContext.h>
#include <IceSSL/SslServerTransceiver.h>
#include <IceSSL/OpenSSLUtils.h>
#include <IceSSL/TraceLevels.h>

using namespace std;
using namespace Ice;

void
IceSSL::ServerContext::configure(const GeneralConfig& generalConfig,
                                 const CertificateAuthority& certificateAuthority,
                                 const BaseCertificates& baseCertificates)
{
    Context::configure(generalConfig, certificateAuthority, baseCertificates);

    assert(_sslContext != 0);

    // On servers, Attempt to use non-export (strong) encryption
    // first.  This option does not always work, and in the OpenSSL
    // documentation is declared as 'broken'.
    // SSL_CTX_set_options(_sslContext, SSL_OP_NON_EXPORT_FIRST);

    // Always use a new DH key when using Diffie-Hellman key agreement.
    SSL_CTX_set_options(_sslContext, SSL_OP_SINGLE_DH_USE);

    // Set the RSA Callback routine in case we need to build a temporary (ephemeral) RSA key.
    SSL_CTX_set_tmp_rsa_callback(_sslContext, tmpRSACallback);

    // Set the DH Callback routine in case we need a temporary (ephemeral) DH key.
    SSL_CTX_set_tmp_dh_callback(_sslContext, tmpDHCallback);

    loadCertificateAuthority(certificateAuthority);

    // Set the session context for the SSL system [SERVER ONLY].
    string connectionContext = generalConfig.getContext();
    SSL_CTX_set_session_id_context(_sslContext,
                                   reinterpret_cast<const unsigned char *>(connectionContext.c_str()),
                                   (unsigned int) connectionContext.size());

    if(_traceLevels->security >= SECURITY_PROTOCOL)
    {
        Trace out(_communicator->getLogger(), _traceLevels->securityCat);

        out << "\n";
        out << "general configuration (server)\n";
        out << "------------------------------\n";
	IceSSL::operator<<(out, generalConfig);
        out << "\n\n";

        out << "CA file: " << certificateAuthority.getCAFileName() << "\n";
        out << "CA path: " << certificateAuthority.getCAPath() << "\n";

        out << "base certificates (server)\n";
        out << "--------------------------\n";
	IceSSL::operator<<(out, baseCertificates);
        out << "\n\n";
    }
}

IceSSL::SslTransceiverPtr 
IceSSL::ServerContext::createTransceiver(int socket, const OpenSSLPluginIPtr& plugin, int timeout)
{
    if(_sslContext == 0)
    {
        ContextNotConfiguredException contextEx(__FILE__, __LINE__);

        throw contextEx;
    }

    SSL* ssl = createSSLConnection(socket);
    SslTransceiverPtr transceiver = new SslServerTransceiver(plugin, socket, _certificateVerifier, ssl);

    transceiverSetup(transceiver, timeout);

    return transceiver;
}

//
// Protected
//

IceSSL::ServerContext::ServerContext(const TraceLevelsPtr& traceLevels, const CommunicatorPtr& communicator) :
    Context(traceLevels, communicator, Server)
{
    _rsaPrivateKeyProperty = "IceSSL.Server.Overrides.RSA.PrivateKey";
    _rsaPublicKeyProperty  = "IceSSL.Server.Overrides.RSA.Certificate";
    _dsaPrivateKeyProperty = "IceSSL.Server.Overrides.DSA.PrivateKey";
    _dsaPublicKeyProperty  = "IceSSL.Server.Overrides.DSA.Certificate";
    _caCertificateProperty = "IceSSL.Server.Overrides.CACertificate";
    _passphraseRetriesProperty = "IceSSL.Server.Passphrase.Retries";
    _connectionHandshakeRetries = "IceSSL.Server.Handshake.Retries";
}

void
IceSSL::ServerContext::loadCertificateAuthority(const CertificateAuthority& certAuth)
{
    assert(_sslContext != 0);

    Context::loadCertificateAuthority(certAuth);

    string caFile = certAuth.getCAFileName();

    if(caFile.empty())
    {
        return;
    }

    STACK_OF(X509_NAME)* certNames = SSL_load_client_CA_file(caFile.c_str());

    if(certNames == 0)
    {
        if(_traceLevels->security >= SECURITY_WARNINGS)
        {
            Trace out(_communicator->getLogger(), _traceLevels->securityCat);
            out << "WRN unable to load certificate authorities certificate names from " << caFile << "\n";
            out << sslGetErrors();
        }
    }
    else
    {
        SSL_CTX_set_client_CA_list(_sslContext, certNames);
    }
}
