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
#include <IceSSL/ContextOpenSSLServer.h>
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
                                   connectionContext.size());

    if(_traceLevels->security >= SECURITY_PROTOCOL)
    {
        ostringstream s;

        s << endl;
        s << "general configuration (server)" << endl;
        s << "------------------------------" << endl;
        s << generalConfig   << endl << endl;

        s << "CA file: " << certificateAuthority.getCAFileName() << endl;
        s << "CA path: " << certificateAuthority.getCAPath() << endl;

        s << "base certificates (server)" << endl;
        s << "--------------------------" << endl;
        s << baseCertificates << endl << endl;

        _logger->trace(_traceLevels->securityCat, s.str());
    }
}

IceSSL::SslTransceiverPtr 
IceSSL::ServerContext::createTransceiver(int socket, const PluginBaseIPtr& plugin)
{
    if(_sslContext == 0)
    {
        OpenSSL::ContextNotConfiguredException contextEx(__FILE__, __LINE__);

        throw contextEx;
    }

    SSL* ssl = createSSLConnection(socket);
    SslTransceiverPtr transceiver = new SslServerTransceiver(plugin, socket, _certificateVerifier, ssl);

    transceiverSetup(transceiver);

    return transceiver;
}

//
// Protected
//

IceSSL::ServerContext::ServerContext(const TraceLevelsPtr& traceLevels, const LoggerPtr& logger,
                                     const PropertiesPtr& properties) :
    Context(traceLevels, logger, properties)
{
    _rsaPrivateKeyProperty = "IceSSL.Server.Overrides.RSA.PrivateKey";
    _rsaPublicKeyProperty  = "IceSSL.Server.Overrides.RSA.Certificate";
    _dsaPrivateKeyProperty = "IceSSL.Server.Overrides.DSA.PrivateKey";
    _dsaPublicKeyProperty  = "IceSSL.Server.Overrides.DSA.Certificate";
    _caCertificateProperty = "IceSSL.Server.Overrides.CACertificate";
    _handshakeTimeoutProperty = "IceSSL.Server.Handshake.ReadTimeout";
    _passphraseRetriesProperty = "IceSSL.Client.Passphrase.Retries";
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
            string errorString = "unable to load certificate authorities certificate names from " + caFile + "\n";
            errorString += OpenSSL::sslGetErrors();
            _logger->trace(_traceLevels->securityCat, "WRN " + errorString);
        }
    }
    else
    {
        SSL_CTX_set_client_CA_list(_sslContext, certNames);
    }
}
