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
#include <Ice/SslException.h>
#include <Ice/RSAKeyPair.h>
#include <Ice/CertificateDesc.h>
#include <Ice/SslConnectionOpenSSL.h>
#include <Ice/ContextOpenSSL.h>

#include <Ice/OpenSSLJanitors.h>
#include <Ice/OpenSSLUtils.h>
#include <openssl/err.h>

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
IceSSL::OpenSSL::Context::addTrustedCertificateBase64(const std::string& trustedCertString)
{
    RSAPublicKey pubKey(trustedCertString);

    addTrustedCertificate(pubKey);
}

void
IceSSL::OpenSSL::Context::addTrustedCertificate(const Ice::ByteSeq& trustedCert)
{
    RSAPublicKey pubKey(trustedCert);

    addTrustedCertificate(pubKey);
}

void
IceSSL::OpenSSL::Context::setRSAKeysBase64(const std::string& privateKey,
                                           const std::string& publicKey)
{
    if (privateKey.empty())
    {
        IceSSL::PrivateKeyException privateKeyEx(__FILE__, __LINE__);

        privateKeyEx._message = "Empty private key supplied.";

        throw privateKeyEx;
    }

    addKeyCert(privateKey, publicKey);
}

void
IceSSL::OpenSSL::Context::setRSAKeys(const Ice::ByteSeq& privateKey, const Ice::ByteSeq& publicKey)
{
    if (privateKey.empty())
    {
        IceSSL::PrivateKeyException privateKeyEx(__FILE__, __LINE__);

        privateKeyEx._message = "Empty private key supplied.";

        throw privateKeyEx;
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

    // Determine the number of retries the user gets on passphrase entry.
    std::string passphraseRetries = _properties->getPropertyWithDefault(_passphraseRetriesProperty,
                                                                        _maxPassphraseRetriesDefault);
    int retries = atoi(passphraseRetries.c_str());
    retries = (retries < 0 ? 0 : retries);
    _maxPassphraseTries = retries + 1;

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

    _maxPassphraseRetriesDefault = "4";
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

                errorString = "ssl version ";
                errorString += sslVersion;
                errorString += " not supported (defaulting to SSL_V23)";
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
        IceSSL::OpenSSL::ContextInitializationException contextInitEx(__FILE__, __LINE__);

        contextInitEx._message = "unable to create ssl context\n" + sslGetErrors();

        throw contextInitEx;
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
            _logger->trace(_traceLevels->securityCat, "WRN unable to load certificate authorities.");
        }
    }
    else
    {
        int setDefaultVerifyPathsRet = SSL_CTX_set_default_verify_paths(_sslContext);


        if (!setDefaultVerifyPathsRet && (_traceLevels->security >= IceSSL::SECURITY_WARNINGS))
        { 
            _logger->trace(_traceLevels->securityCat, "WRN unable to verify certificate authorities.");
        }
    }

    // Now we add whatever override/addition that we wish to put into the trusted certificates list
    std::string caCertBase64 = _properties->getProperty(_caCertificateProperty);
    if (!caCertBase64.empty())
    {
         addTrustedCertificateBase64(caCertBase64);
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
        publicKey = _properties->getProperty(publicProperty);
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
        IceSSL::OpenSSL::CertificateKeyMatchException certKeyMatchEx(__FILE__, __LINE__);

        certKeyMatchEx._message = "private key does not match the certificate public key";
        std::string sslError = sslGetErrors();

        if (!sslError.empty())
        {
            certKeyMatchEx._message += "\n";
            certKeyMatchEx._message += sslError;
        }

        throw certKeyMatchEx;
    }
}

void
IceSSL::OpenSSL::Context::addTrustedCertificate(const RSAPublicKey& trustedCertificate)
{
    if (_sslContext == 0)
    {
        IceSSL::OpenSSL::ContextNotConfiguredException contextConfigEx(__FILE__, __LINE__);

        contextConfigEx._message = "ssl context not configured";

        throw contextConfigEx;
    }

    X509_STORE* certStore = SSL_CTX_get_cert_store(_sslContext);

    assert(certStore != 0);

    if (X509_STORE_add_cert(certStore, trustedCertificate.getX509PublicKey()) == 0)
    {
        IceSSL::OpenSSL::TrustedCertificateAddException trustEx(__FILE__, __LINE__);

        trustEx._message = sslGetErrors();

        throw trustEx;
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
            IceSSL::OpenSSL::CertificateLoadException certLoadEx(__FILE__, __LINE__);

            certLoadEx._message = "unable to load certificate from '";
            certLoadEx._message += publicFile;
            certLoadEx._message += "'\n";
            certLoadEx._message += sslGetErrors();

            throw certLoadEx;
        }

        if (privateKey.getFileName().empty())
        {
            if (_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
            { 
                _logger->trace(_traceLevels->securityCat, "WRN no private key specified -- using the certificate");
            }

            privKeyFile = publicFile;
            privKeyFileType = publicEncoding;
        }

        int retryCount = 0;
        int pkLoadResult;
        int errCode = 0;

        while (retryCount != _maxPassphraseTries)
        {
            // We ignore the errors and remove them from the stack.
            std::string errorString = sslGetErrors();

            // Set which Private Key file to use.
            pkLoadResult = SSL_CTX_use_PrivateKey_file(_sslContext, privKeyFile, privKeyFileType);

            if (pkLoadResult <= 0)
            {
                errCode = ERR_GET_REASON(ERR_peek_error());
            }
            else
            {
                // The load went fine - continue on.
                break;
            }

            // PEM errors, most likely related to a bad passphrase.
            if (errCode != PEM_R_BAD_PASSWORD_READ &&
                errCode != PEM_R_BAD_DECRYPT &&
                errCode != PEM_R_BAD_BASE64_DECODE)
            {
                // Other errors get dealt with below.
                break;
            }

            std::cout << "Passphrase error!" << std::endl;

            retryCount++;
        }

        if (pkLoadResult <= 0)
        {
            int errCode = ERR_GET_REASON(ERR_peek_error());

            // Note: Because OpenSSL currently (V0.9.6b) performs a check to see if the
            //       key matches the private key when calling SSL_CTX_use_PrivateKey_file().
            if (errCode == X509_R_KEY_VALUES_MISMATCH || errCode == X509_R_KEY_TYPE_MISMATCH)
            {
                IceSSL::OpenSSL::CertificateKeyMatchException certKeyMatchEx(__FILE__, __LINE__);

                certKeyMatchEx._message = "private key does not match the certificate public key";
                std::string sslError = sslGetErrors();

                if (!sslError.empty())
                {
                    certKeyMatchEx._message += "\n";
                    certKeyMatchEx._message += sslError;
                }

                throw certKeyMatchEx;
            }
            else
            {
                IceSSL::OpenSSL::PrivateKeyLoadException pklEx(__FILE__, __LINE__);

                pklEx._message = "unable to load private key from '";
                pklEx._message += privKeyFile;
                pklEx._message += "'\n";
                pklEx._message += sslGetErrors();

	        throw pklEx;
            }
        }

        checkKeyCert();
    }
}

void
IceSSL::OpenSSL::Context::addKeyCert(const RSAKeyPair& keyPair)
{
    if (_sslContext == 0)
    {
        IceSSL::OpenSSL::ContextNotConfiguredException contextConfigEx(__FILE__, __LINE__);

        contextConfigEx._message = "ssl context not configured";

        throw contextConfigEx;
    }

    // Note: Normally I would use an X509Janitor and RSAJanitor to ensure that
    //       memory was being freed properly when exceptions are thrown, but
    //       both SSL_CTX_use_certificate and SSL_CTX_use_RSAPrivateKey free
    //       certificate/key memory regardless if the call succeeded.

    // Set which Public Key file to use.
    if (SSL_CTX_use_certificate(_sslContext, keyPair.getX509PublicKey()) <= 0)
    {
        IceSSL::OpenSSL::CertificateLoadException certLoadEx(__FILE__, __LINE__);

        certLoadEx._message = "unable to set certificate from memory";
        std::string sslError = sslGetErrors();

        if (!sslError.empty())
        {
            certLoadEx._message += "\n";
            certLoadEx._message += sslError;
        }

        throw certLoadEx;
    }

    // Set which Private Key file to use.
    if (SSL_CTX_use_RSAPrivateKey(_sslContext, keyPair.getRSAPrivateKey()) <= 0)
    {
        int errCode = ERR_GET_REASON(ERR_peek_error());

        // Note: Because OpenSSL currently (V0.9.6b) performs a check to see if the
        //       key matches the private key when calling SSL_CTX_use_PrivateKey_file().
        if (errCode == X509_R_KEY_VALUES_MISMATCH || errCode == X509_R_KEY_TYPE_MISMATCH)
        {
            IceSSL::OpenSSL::CertificateKeyMatchException certKeyMatchEx(__FILE__, __LINE__);

            certKeyMatchEx._message = "private key does not match the certificate public key";
            std::string sslError = sslGetErrors();

            if (!sslError.empty())
            {
                certKeyMatchEx._message += "\n";
                certKeyMatchEx._message += sslError;
            }

            throw certKeyMatchEx;
        }
        else
        {
            IceSSL::OpenSSL::PrivateKeyLoadException pklEx(__FILE__, __LINE__);

            pklEx._message = "unable to set private key from memory";
            std::string sslError = sslGetErrors();

            if (!sslError.empty())
            {
                pklEx._message += "\n";
                pklEx._message += sslError;
            }

            throw pklEx;
        }
    }

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
            _logger->trace(_traceLevels->securityCat, "WRN no private key specified -- using the certificate");
        }

        privKey = publicKey;
    }

    // Make a key pair based on the DER encoded byte sequences.
    addKeyCert(RSAKeyPair(privKey, publicKey));
}

void
IceSSL::OpenSSL::Context::addKeyCert(const std::string& privateKey, const std::string& publicKey)
{
    std::string privKey = privateKey;

    if (privKey.empty())
    {
        if (_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
        { 
            _logger->trace(_traceLevels->securityCat, "WRN no private key specified -- using the certificate");
        }

        privKey = publicKey;
    }

    // Make a key pair based on the Base64 encoded strings.
    addKeyCert(RSAKeyPair(privKey, publicKey));
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
    int handshakeReadTimeout = _properties->getPropertyAsIntWithDefault(_handshakeTimeoutProperty, 5000);
    connection->setHandshakeReadTimeout(handshakeReadTimeout);
}

void
IceSSL::OpenSSL::Context::setCipherList(const std::string& cipherList)
{
    assert(_sslContext != 0);

    if (!cipherList.empty() && (!SSL_CTX_set_cipher_list(_sslContext, cipherList.c_str())) &&
        (_traceLevels->security >= IceSSL::SECURITY_WARNINGS))
    {
        std::string errorString = "WRN error setting cipher list " + cipherList + " -- using default list\n";
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

    // File type must be PEM - that's the only way we can load DH Params, apparently.
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

