// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Instance.h>
#include <Ice/Properties.h>

#include <Ice/DefaultCertificateVerifier.h>
#include <Ice/SecurityException.h>
#include <Ice/SslRSAKeyPair.h>
#include <Ice/SslRSAPublicKey.h>
#include <Ice/SslCertificateDesc.h>
#include <Ice/SslConnectionOpenSSL.h>
#include <Ice/SslContextOpenSSL.h>

#include <Ice/SslJanitors.h>
#include <Ice/SslOpenSSLUtils.h>

using IceSSL::ConnectionPtr;

void ::IceInternal::incRef(::IceSSL::OpenSSL::Context* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSSL::OpenSSL::Context* p) { p->__decRef(); }

IceSSL::OpenSSL::Context::~Context()
{
    if (_sslContext != 0)
    {
        SSL_CTX_free(_sslContext);

        _sslContext = 0;
    }
}

bool
IceSSL::OpenSSL::Context::isConfigured()
{
    return (_sslContext != 0 ? true : false);
}

void
IceSSL::OpenSSL::Context::setCertificateVerifier(const CertificateVerifierPtr& verifier)
{
    _certificateVerifier = verifier;
}

void
IceSSL::OpenSSL::Context::addTrustedCertificate(const std::string& trustedCertString)
{
    if (_sslContext == 0)
    {
        // ContextNotConfiguredException contextEx(__FILE__, __LINE__);
        IceSSL::OpenSSL::ContextException contextEx(__FILE__, __LINE__);

        contextEx._message = "SSL Context not configured.";

        throw contextEx;
    }

    RSAPublicKey pubKey(trustedCertString);

    X509_STORE* certStore = SSL_CTX_get_cert_store(_sslContext);

    int addedCertAuthorityCert = X509_STORE_add_cert(certStore, pubKey.getX509PublicKey());

    // TODO: Make this an exception?
    assert(addedCertAuthorityCert != 0);
}

void
IceSSL::OpenSSL::Context::setRSAKeysBase64(const std::string& privateKey,
                                           const std::string& publicKey)
{
    if (_sslContext == 0)
    {
        // ContextNotConfiguredException contextEx(__FILE__, __LINE__);
        IceSSL::OpenSSL::ContextException contextEx(__FILE__, __LINE__);

        contextEx._message = "SSL Context not configured.";

        throw contextEx;
    }

    addKeyCert(privateKey, publicKey);
}

void
IceSSL::OpenSSL::Context::setRSAKeys(const Ice::ByteSeq& privateKey, const Ice::ByteSeq& publicKey)
{
    if (_sslContext == 0)
    {
        // ContextNotConfiguredException contextEx(__FILE__, __LINE__);
        IceSSL::OpenSSL::ContextException contextEx(__FILE__, __LINE__);

        contextEx._message = "SSL Context not configured.";

        throw contextEx;
    }

    addKeyCert(privateKey, publicKey);
}

void
IceSSL::OpenSSL::Context::configure(const GeneralConfig& generalConfig,
                                    const CertificateAuthority& certificateAuthority,
                                    const BaseCertificates& baseCertificates)
{
    // Create an SSL Context based on the context params.
    createContext(generalConfig.getProtocol());

    // Get the cipherlist and set it in the context.
    setCipherList(generalConfig.getCipherList());

    // Set the certificate verification mode.
    SSL_CTX_set_verify(_sslContext, generalConfig.getVerifyMode(), verifyCallback);

    // Set the certificate verify depth
    SSL_CTX_set_verify_depth(_sslContext, generalConfig.getVerifyDepth());

    // Process the RSA Certificate
    setKeyCert(baseCertificates.getRSACert(), _rsaPrivateKeyProperty, _rsaPublicKeyProperty);

    // Process the DSA Certificate
    setKeyCert(baseCertificates.getDSACert(), _dsaPrivateKeyProperty, _dsaPublicKeyProperty);

    // Set the DH key agreement parameters.
    if (baseCertificates.getDHParams().getKeySize() != 0)
    {
        setDHParams(baseCertificates);
    }
}

//
// Protected
//

IceSSL::OpenSSL::Context::Context(const IceInternal::InstancePtr& instance) :
                         _traceLevels(instance->traceLevels()),
                         _logger(instance->logger()),
                         _properties(instance->properties())
{
    assert(_traceLevels != 0);
    assert(_logger != 0);
    assert(_properties != 0);

    _certificateVerifier = new DefaultCertificateVerifier(instance);
    _sslContext = 0;
}

SSL_METHOD*
IceSSL::OpenSSL::Context::getSslMethod(SslProtocol sslVersion)
{
    SSL_METHOD* sslMethod = 0;

    switch (sslVersion)
    {
        case SSL_V2 :
        {
            sslMethod = SSLv2_method();
            break;
        }

        case SSL_V23 :
        {
            sslMethod = SSLv23_method();
            break;
        }

        case SSL_V3 :
        {
            sslMethod = SSLv3_method();
            break;
        }

        case TLS_V1 :
        {
            sslMethod = TLSv1_method();
            break;
        }

        default :
        {
            if (_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
            { 
                std::string errorString;

                errorString = "SSL Version ";
                errorString += sslVersion;
                errorString += " not supported - defaulting to SSL_V23.";
                _logger->trace(_traceLevels->securityCat, "WRN " + errorString);
            }

            sslMethod = SSLv23_method();
        }
    }

    return sslMethod;
}

void
IceSSL::OpenSSL::Context::createContext(SslProtocol sslProtocol)
{
    if (_sslContext != 0)
    {
        SSL_CTX_free(_sslContext);
        _sslContext = 0;
    }

    _sslContext = SSL_CTX_new(getSslMethod(sslProtocol));

    if (_sslContext == 0)
    {
        IceSSL::OpenSSL::ContextException contextEx(__FILE__, __LINE__);

        contextEx._message = "Unable to create SSL Context.\n" + sslGetErrors();

        throw contextEx;
    }

    // Turn off session caching, supposedly fixes a problem with multithreading.
    SSL_CTX_set_session_cache_mode(_sslContext, SSL_SESS_CACHE_OFF);
}

void
IceSSL::OpenSSL::Context::loadCertificateAuthority(const CertificateAuthority& certAuth)
{
    assert(_sslContext != 0);

    std::string fileName = certAuth.getCAFileName();
    std::string certPath = certAuth.getCAPath();

    const char* caFile = 0;
    const char* caPath = 0;

    // The following checks are required to send the expected values to the OpenSSL library.
    // It does not like receiving "", but prefers NULLs.

    if (!fileName.empty())
    {
        caFile = fileName.c_str();
    }

    if (!certPath.length())
    {
        caPath = certPath.c_str();
    }

    // SSL_CTX_set_default_passwd_cb(sslContext, passwordCallback);

    // Check the Certificate Authority file(s).
    int loadVerifyRet = SSL_CTX_load_verify_locations(_sslContext, caFile, caPath);

    if (!loadVerifyRet)
    { 
        if (_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
        {
            _logger->trace(_traceLevels->securityCat, "WRN Unable to load Certificate Authorities.");
        }
    }
    else
    {
        int setDefaultVerifyPathsRet = SSL_CTX_set_default_verify_paths(_sslContext);


        if (!setDefaultVerifyPathsRet && (_traceLevels->security >= IceSSL::SECURITY_WARNINGS))
        { 
            _logger->trace(_traceLevels->securityCat, "WRN Unable to verify Certificate Authorities.");
        }
    }

    // Now we add whatever override/addition that we wish to put into the trusted certificates list
    std::string caCertBase64 = _properties->getProperty(_caCertificateProperty);
    if (!caCertBase64.empty())
    {
         addTrustedCertificate(caCertBase64);
    }
}

void
IceSSL::OpenSSL::Context::setKeyCert(const CertificateDesc& certDesc,
                                     const std::string& privateProperty,
                                     const std::string& publicProperty)
{
    std::string privateKey;
    std::string publicKey;

    if (!privateProperty.empty())
    {
        privateKey = _properties->getProperty(privateProperty);
    }

    if (!publicProperty.empty())
    {
        publicKey  = _properties->getProperty(publicProperty);
    }

    if (!privateKey.empty() && !publicKey.empty())
    {
        addKeyCert(privateKey, publicKey);
    }
    else if (certDesc.getKeySize() != 0)
    {
        const CertificateFile& privateKey = certDesc.getPrivate();
        const CertificateFile& publicKey  = certDesc.getPublic();

        addKeyCert(privateKey, publicKey);
    }
}

void
IceSSL::OpenSSL::Context::checkKeyCert()
{
    assert(_sslContext != 0);

    // Check to see if the Private and Public keys that have been
    // set against the SSL context match up.
    if (!SSL_CTX_check_private_key(_sslContext))
    {
        IceSSL::OpenSSL::ContextException contextEx(__FILE__, __LINE__);

        contextEx._message = "Private key does not match the certificate public key.";
        std::string sslError = sslGetErrors();

        if (!sslError.empty())
        {
            contextEx._message += "\n";
            contextEx._message += sslError;
        }

        throw contextEx;
    }
}

void
IceSSL::OpenSSL::Context::addKeyCert(const CertificateFile& privateKey, const CertificateFile& publicCert)
{
    assert(_sslContext != 0);

    if (!publicCert.getFileName().empty())
    {
	std::string publicCertFile = publicCert.getFileName();
        const char* publicFile = publicCertFile.c_str();
        int publicEncoding = publicCert.getEncoding();

        std::string privCertFile = privateKey.getFileName();
        const char* privKeyFile = privCertFile.c_str();
        int privKeyFileType = privateKey.getEncoding();

        // Set which Public Key file to use.
        if (SSL_CTX_use_certificate_file(_sslContext, publicFile, publicEncoding) <= 0)
        {
            IceSSL::OpenSSL::ContextException contextEx(__FILE__, __LINE__);

            contextEx._message = "Unable to get certificate from '";
            contextEx._message += publicFile;
            contextEx._message += "'\n";
            contextEx._message += sslGetErrors();

            throw contextEx;
        }

        if (privateKey.getFileName().empty())
        {
            if (_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
            { 
                _logger->trace(_traceLevels->securityCat, "WRN No private key specified - using the certificate.");
            }

            privKeyFile = publicFile;
            privKeyFileType = publicEncoding;
        }

        // Set which Private Key file to use.
        if (SSL_CTX_use_PrivateKey_file(_sslContext, privKeyFile, privKeyFileType) <= 0)
        {
            IceSSL::OpenSSL::ContextException contextEx(__FILE__, __LINE__);

            contextEx._message = "Unable to get private key from '";
            contextEx._message += privKeyFile;
            contextEx._message += "'\n";
            contextEx._message += sslGetErrors();

	    throw contextEx;
        }

        checkKeyCert();
    }
}

void
IceSSL::OpenSSL::Context::addKeyCert(const RSAKeyPair& keyPair)
{
    // Janitors to ensure that everything gets cleaned up properly
    RSAJanitor rsaJanitor(keyPair.getRSAPrivateKey());
    X509Janitor x509Janitor(keyPair.getX509PublicKey());

    // Set which Public Key file to use.
    if (SSL_CTX_use_certificate(_sslContext, x509Janitor.get()) <= 0)
    {
        IceSSL::OpenSSL::ContextException contextEx(__FILE__, __LINE__);

        contextEx._message = "Unable to set certificate from memory.";
        std::string sslError = sslGetErrors();

        if (!sslError.empty())
        {
            contextEx._message += "\n";
            contextEx._message += sslError;
        }

        throw contextEx;
    }

    x509Janitor.clear();

    // Set which Private Key file to use.
    if (SSL_CTX_use_RSAPrivateKey(_sslContext, rsaJanitor.get()) <= 0)
    {
        IceSSL::OpenSSL::ContextException contextEx(__FILE__, __LINE__);

        contextEx._message = "Unable to set private key from memory.";
        std::string sslError = sslGetErrors();

        if (!sslError.empty())
        {
            contextEx._message += "\n";
            contextEx._message += sslError;
        }

        throw contextEx;
    }

    rsaJanitor.clear();

    checkKeyCert();
}

void
IceSSL::OpenSSL::Context::addKeyCert(const Ice::ByteSeq& privateKey, const Ice::ByteSeq& publicKey)
{
    Ice::ByteSeq privKey = privateKey;

    if (privKey.empty())
    {
        if (_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
        { 
            _logger->trace(_traceLevels->securityCat, "WRN No private key specified - using the certificate.");
        }

        privKey = publicKey;
    }

    // Make a key pair based on the DER encoded byte sequences.
    RSAKeyPair keyPair(privKey, publicKey);

    addKeyCert(keyPair);
}

void
IceSSL::OpenSSL::Context::addKeyCert(const std::string& privateKey, const std::string& publicKey)
{
    std::string privKey = privateKey;

    if (privKey.empty())
    {
        if (_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
        { 
            _logger->trace(_traceLevels->securityCat, "WRN No private key specified - using the certificate.");
        }

        privKey = publicKey;
    }

    // Make a key pair based on the Base64 encoded strings.
    RSAKeyPair keyPair(privKey, publicKey);

    addKeyCert(keyPair);
}

SSL*
IceSSL::OpenSSL::Context::createSSLConnection(int socket)
{
    assert(_sslContext != 0);

    SSL* sslConnection = SSL_new(_sslContext);
    assert(sslConnection != 0);

    SSL_clear(sslConnection);

    SSL_set_fd(sslConnection, socket);

    return sslConnection;
}

void
IceSSL::OpenSSL::Context::connectionSetup(const ConnectionPtr& connection)
{
    // Set the Post-Handshake Read timeout
    // This timeout is implemented once on the first read after hanshake.
    int handshakeReadTimeout;
    std::string value = _properties->getProperty(_handshakeTimeoutProperty);

    if (!value.empty())
    {
	handshakeReadTimeout = atoi(value.c_str());
    }
    else
    {
        handshakeReadTimeout = 5000;
    }

    connection->setHandshakeReadTimeout(handshakeReadTimeout);
}

void
IceSSL::OpenSSL::Context::setCipherList(const std::string& cipherList)
{
    assert(_sslContext != 0);

    if (!cipherList.empty() && (!SSL_CTX_set_cipher_list(_sslContext, cipherList.c_str())) &&
        (_traceLevels->security >= IceSSL::SECURITY_WARNINGS))
    {
        std::string errorString = "WRN Error setting cipher list " + cipherList + " - using default list.\n";
        errorString += sslGetErrors();
        _logger->trace(_traceLevels->securityCat, errorString);
    }
}

void
IceSSL::OpenSSL::Context::setDHParams(const BaseCertificates& baseCerts)
{
    DH* dh = 0;

    std::string dhFile = baseCerts.getDHParams().getFileName();
    int encoding = baseCerts.getDHParams().getEncoding();

    // File type must be PEM - that's the only way we can load
    // DH Params, apparently.
    if ((!dhFile.empty()) && (encoding == SSL_FILETYPE_PEM))
    {
        dh = loadDHParam(dhFile.c_str());
    }

    if (dh == 0)
    {
        if (_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
        { 
            _logger->trace(_traceLevels->securityCat,
                           "WRN Could not load Diffie-Hellman params, generating a temporary 512bit key.");
        }

        dh = getTempDH512();
    }

    if (dh != 0)
    {
        SSL_CTX_set_tmp_dh(_sslContext, dh);

        DH_free(dh);
    }
}

