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
#include <Ice/ContextOpenSSLServer.h>
#include <Ice/SslConnectionOpenSSLServer.h>
#include <Ice/OpenSSLUtils.h>

#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>

#include <iostream.h>

using IceSSL::ConnectionPtr;
using IceSSL::SystemInternalPtr;

void
IceSSL::OpenSSL::ServerContext::configure(const GeneralConfig& generalConfig,
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
    std::string connectionContext = generalConfig.getContext();
    SSL_CTX_set_session_id_context(_sslContext,
                                   reinterpret_cast<const unsigned char *>(connectionContext.c_str()),
                                   connectionContext.size());

    if (_traceLevels->security >= IceSSL::SECURITY_PROTOCOL)
    {
        std::ostringstream s;

        s << std::endl;
        s << "general configuration (server)" << std::endl;
        s << "------------------------------" << std::endl;
        s << generalConfig   << std::endl << std::endl;

        s << "CA file: " << certificateAuthority.getCAFileName() << std::endl;
        s << "CA path: " << certificateAuthority.getCAPath() << std::endl;

        s << "base certificates (server)" << std::endl;
        s << "--------------------------" << std::endl;
        s << baseCertificates << std::endl << std::endl;

        _logger->trace(_traceLevels->securityCat, s.str());
    }
}

IceSSL::ConnectionPtr 
IceSSL::OpenSSL::ServerContext::createConnection(int socket, const SystemInternalPtr& system)
{
    if (_sslContext == 0)
    {
        IceSSL::OpenSSL::ContextNotConfiguredException contextEx(__FILE__, __LINE__);

        throw contextEx;
    }

    ConnectionPtr connection = new ServerConnection(_traceLevels,
                                                    _logger,
                                                    _certificateVerifier,
                                                    createSSLConnection(socket),
                                                    system);

    connectionSetup(connection);

    return connection;
}

//
// Protected
//

IceSSL::OpenSSL::ServerContext::ServerContext(const IceInternal::InstancePtr& instance) :
                                         Context(instance)
{
    _rsaPrivateKeyProperty = "Ice.SSL.Server.Overrides.RSA.PrivateKey";
    _rsaPublicKeyProperty  = "Ice.SSL.Server.Overrides.RSA.Certificate";
    _dsaPrivateKeyProperty = "Ice.SSL.Server.Overrides.DSA.PrivateKey";
    _dsaPublicKeyProperty  = "Ice.SSL.Server.Overrides.DSA.Certificate";
    _caCertificateProperty = "Ice.SSL.Server.Overrides.CACertificate";
    _handshakeTimeoutProperty = "Ice.SSL.Server.Handshake.ReadTimeout";
    _passphraseRetriesProperty = "Ice.SSL.Server.Passphrase.Retries";
}

void
IceSSL::OpenSSL::ServerContext::loadCertificateAuthority(const CertificateAuthority& certAuth)
{
    assert(_sslContext != 0);

    Context::loadCertificateAuthority(certAuth);

    std::string caFile = certAuth.getCAFileName();

    if (caFile.empty())
    {
        return;
    }

    STACK_OF(X509_NAME)* certNames = SSL_load_client_CA_file(caFile.c_str());

    if (certNames == 0)
    {
        if (_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
        {
            std::string errorString = "unable to load certificate authorities certificate names from " + caFile + "\n";
            errorString += sslGetErrors();
            _logger->trace(_traceLevels->securityCat, "WRN " + errorString);
        }
    }
    else
    {
        SSL_CTX_set_client_CA_list(_sslContext, certNames);
    }
}

