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
#include <Ice/Properties.h>

#include <IceSSL/DefaultCertificateVerifier.h>
#include <IceSSL/Exception.h>
#include <IceSSL/RSAKeyPair.h>
#include <IceSSL/CertificateDesc.h>
#include <IceSSL/SslTransceiver.h>
#include <IceSSL/Context.h>
#include <IceSSL/OpenSSLJanitors.h>
#include <IceSSL/OpenSSLUtils.h>
#include <IceSSL/TraceLevels.h>

#include <openssl/err.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(::IceSSL::Context* p) { p->__incRef(); }
void IceInternal::decRef(::IceSSL::Context* p) { p->__decRef(); }

IceSSL::Context::~Context()
{
    cleanUp();
}

bool
IceSSL::Context::isConfigured()
{
    return (_sslContext != 0 ? true : false);
}

void
IceSSL::Context::cleanUp()
{
    if(_sslContext != 0)
    {
        SSL_CTX_free(_sslContext);

        _sslContext = 0;
    }
}

void
IceSSL::Context::setCertificateVerifier(const CertificateVerifierPtr& verifier)
{
    _certificateVerifier = verifier;
    _certificateVerifier->setContext(_contextType);
}

void
IceSSL::Context::addTrustedCertificateBase64(const string& trustedCertString)
{
    RSAPublicKey pubKey(trustedCertString);

    addTrustedCertificate(pubKey);
}

void
IceSSL::Context::addTrustedCertificate(const Ice::ByteSeq& trustedCert)
{
    RSAPublicKey pubKey(trustedCert);

    addTrustedCertificate(pubKey);
}

void
IceSSL::Context::setRSAKeysBase64(const string& privateKey, const string& publicKey)
{
    if(privateKey.empty())
    {
        IceSSL::PrivateKeyException privateKeyEx(__FILE__, __LINE__);

        privateKeyEx.message = "Empty private key supplied.";

        throw privateKeyEx;
    }

    addKeyCert(privateKey, publicKey);
}

void
IceSSL::Context::setRSAKeys(const Ice::ByteSeq& privateKey, const Ice::ByteSeq& publicKey)
{
    if(privateKey.empty())
    {
        IceSSL::PrivateKeyException privateKeyEx(__FILE__, __LINE__);

        privateKeyEx.message = "Empty private key supplied.";

        throw privateKeyEx;
    }

    addKeyCert(privateKey, publicKey);
}

void
IceSSL::Context::configure(const GeneralConfig& generalConfig,
                           const CertificateAuthority& certificateAuthority,
                           const BaseCertificates& baseCertificates)
{
    // Create an SSL Context based on the context params.
    createContext(generalConfig.getProtocol());

    // Enable workarounds and disable SSLv2.
    SSL_CTX_set_options(_sslContext, SSL_OP_ALL|SSL_OP_NO_SSLv2);

    // Get the cipherlist and set it in the context.
    setCipherList(generalConfig.getCipherList());

    // Set the certificate verification mode.
    SSL_CTX_set_verify(_sslContext, generalConfig.getVerifyMode(), verifyCallback);

    // Set the certificate verify depth
    SSL_CTX_set_verify_depth(_sslContext, generalConfig.getVerifyDepth());

    // Determine the number of retries the user gets on passphrase entry.
    string passphraseRetries = _communicator->getProperties()->getPropertyWithDefault(_passphraseRetriesProperty,
										      _maxPassphraseRetriesDefault);
    int retries = atoi(passphraseRetries.c_str());
    retries = (retries < 0 ? 0 : retries);
    _maxPassphraseTries = retries + 1;

    // Process the RSA Certificate
    setKeyCert(baseCertificates.getRSACert(), _rsaPrivateKeyProperty, _rsaPublicKeyProperty);

    // Process the DSA Certificate
    setKeyCert(baseCertificates.getDSACert(), _dsaPrivateKeyProperty, _dsaPublicKeyProperty);

    // Set the DH key agreement parameters.
    if(baseCertificates.getDHParams().getKeySize() != 0)
    {
        setDHParams(baseCertificates);
    }
}

//
// Protected
//

IceSSL::Context::Context(const TraceLevelsPtr& traceLevels, const CommunicatorPtr& communicator,
                         const ContextType& type) :
    _traceLevels(traceLevels),
    _communicator(communicator),
    _contextType(type)
{
    _certificateVerifier = new DefaultCertificateVerifier(traceLevels, communicator);
    _certificateVerifier->setContext(_contextType);
    _sslContext = 0;

    _maxPassphraseRetriesDefault = "4";
}

SSL_METHOD*
IceSSL::Context::getSslMethod(SslProtocol sslVersion)
{
    SSL_METHOD* sslMethod = 0;

    switch(sslVersion)
    {
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
            if(_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
            { 
                Trace out(_communicator->getLogger(), _traceLevels->securityCat);
                out << "WRN ssl version " << sslVersion;
                out << " not supported (defaulting to SSL_V23)";
            }

            sslMethod = SSLv23_method();
        }
    }

    return sslMethod;
}

void
IceSSL::Context::createContext(SslProtocol sslProtocol)
{
    if(_sslContext != 0)
    {
        SSL_CTX_free(_sslContext);
        _sslContext = 0;
    }

    _sslContext = SSL_CTX_new(getSslMethod(sslProtocol));

    if(_sslContext == 0)
    {
        ContextInitializationException contextInitEx(__FILE__, __LINE__);

        contextInitEx.message = "unable to create ssl context\n" + sslGetErrors();

        throw contextInitEx;
    }

    // Turn off session caching, supposedly fixes a problem with multithreading.
    SSL_CTX_set_session_cache_mode(_sslContext, SSL_SESS_CACHE_OFF);
}

void
IceSSL::Context::loadCertificateAuthority(const CertificateAuthority& certAuth)
{
    assert(_sslContext != 0);

    string fileName = certAuth.getCAFileName();
    string certPath = certAuth.getCAPath();

    const char* caFile = 0;
    const char* caPath = 0;

    // The following checks are required to send the expected values to the OpenSSL library.
    // It does not like receiving "", but prefers NULLs.

    if(!fileName.empty())
    {
        caFile = fileName.c_str();
    }

    if(!certPath.empty())
    {
        caPath = certPath.c_str();
    }

    // SSL_CTX_set_default_passwd_cb(sslContext, passwordCallback);

    // Check the Certificate Authority file(s).
    int loadVerifyRet = SSL_CTX_load_verify_locations(_sslContext, caFile, caPath);

    if(!loadVerifyRet)
    { 
        if(_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
        {
            Trace out(_communicator->getLogger(), _traceLevels->securityCat);
            out << "WRN unable to load certificate authorities.";
        }
    }
    else
    {
        int setDefaultVerifyPathsRet = SSL_CTX_set_default_verify_paths(_sslContext);

        if(!setDefaultVerifyPathsRet && (_traceLevels->security >= IceSSL::SECURITY_WARNINGS))
        { 
            Trace out(_communicator->getLogger(), _traceLevels->securityCat);
            out << "WRN unable to verify certificate authorities.";
        }
    }

    // Now we add whatever override/addition that we wish to put into the trusted certificates list
    string caCertBase64 = _communicator->getProperties()->getProperty(_caCertificateProperty);
    if(!caCertBase64.empty())
    {
         addTrustedCertificateBase64(caCertBase64);
    }
}

void
IceSSL::Context::setKeyCert(const CertificateDesc& certDesc,
                            const string& privateProperty,
                            const string& publicProperty)
{
    string privateKey;
    string publicKey;

    if(!privateProperty.empty())
    {
        privateKey = _communicator->getProperties()->getProperty(privateProperty);
    }

    if(!publicProperty.empty())
    {
        publicKey = _communicator->getProperties()->getProperty(publicProperty);
    }

    if(!privateKey.empty() && !publicKey.empty())
    {
        addKeyCert(privateKey, publicKey);
    }
    else if(certDesc.getKeySize() != 0)
    {
        addKeyCert(certDesc.getPrivate(), certDesc.getPublic());
    }
}

void
IceSSL::Context::checkKeyCert()
{
    assert(_sslContext != 0);

    // Check to see if the Private and Public keys that have been
    // set against the SSL context match up.
    if(!SSL_CTX_check_private_key(_sslContext))
    {
        CertificateKeyMatchException certKeyMatchEx(__FILE__, __LINE__);

        certKeyMatchEx.message = "private key does not match the certificate public key";
        string sslError = sslGetErrors();

        if(!sslError.empty())
        {
            certKeyMatchEx.message += "\n";
            certKeyMatchEx.message += sslError;
        }

        throw certKeyMatchEx;
    }
}

void
IceSSL::Context::addTrustedCertificate(const RSAPublicKey& trustedCertificate)
{
    if(_sslContext == 0)
    {
        ContextNotConfiguredException contextConfigEx(__FILE__, __LINE__);

        contextConfigEx.message = "ssl context not configured";

        throw contextConfigEx;
    }

    X509_STORE* certStore = SSL_CTX_get_cert_store(_sslContext);

    assert(certStore != 0);

    if(X509_STORE_add_cert(certStore, trustedCertificate.getX509PublicKey()) == 0)
    {
        TrustedCertificateAddException trustEx(__FILE__, __LINE__);

        trustEx.message = sslGetErrors();

        throw trustEx;
    }
}

void
IceSSL::Context::addKeyCert(const CertificateFile& privateKey, const CertificateFile& publicCert)
{
    assert(_sslContext != 0);

    if(!publicCert.getFileName().empty())
    {
	string publicCertFile = publicCert.getFileName();
        const char* publicFile = publicCertFile.c_str();
        int publicEncoding = publicCert.getEncoding();

        string privCertFile = privateKey.getFileName();
        const char* privKeyFile = privCertFile.c_str();
        int privKeyFileType = privateKey.getEncoding();

        // Set which Public Key file to use.
        if(SSL_CTX_use_certificate_file(_sslContext, publicFile, publicEncoding) <= 0)
        {
            CertificateLoadException certLoadEx(__FILE__, __LINE__);

            certLoadEx.message = "unable to load certificate from '";
            certLoadEx.message += publicFile;
            certLoadEx.message += "'\n";
            certLoadEx.message += sslGetErrors();

            throw certLoadEx;
        }

        if(privateKey.getFileName().empty())
        {
            if(_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
            { 
                Trace out(_communicator->getLogger(), _traceLevels->securityCat);
                out << "WRN no private key specified -- using the certificate";
            }

            privKeyFile = publicFile;
            privKeyFileType = publicEncoding;
        }

        int retryCount = 0;
        int pkLoadResult = 0;
        int errCode = 0;

        while(retryCount != _maxPassphraseTries)
        {
            // We ignore the errors and remove them from the stack.
            string errorString = sslGetErrors();

            // Set which Private Key file to use.
            pkLoadResult = SSL_CTX_use_PrivateKey_file(_sslContext, privKeyFile, privKeyFileType);

            if(pkLoadResult <= 0)
            {
                errCode = ERR_GET_REASON(ERR_peek_error());
            }
            else
            {
                // The load went fine - continue on.
                break;
            }

            // PEM errors, most likely related to a bad passphrase.
            if(errCode != PEM_R_BAD_PASSWORD_READ &&
                errCode != PEM_R_BAD_DECRYPT &&
                errCode != PEM_R_BAD_BASE64_DECODE)
            {
                // Other errors get dealt with below.
                break;
            }

            cout << "Passphrase error!" << endl;

            retryCount++;
        }

        if(pkLoadResult <= 0)
        {
            errCode = ERR_GET_REASON(ERR_peek_error());

            // Note: Because OpenSSL currently (V0.9.6b) performs a check to see if the
            //       key matches the private key when calling SSL_CTX_use_PrivateKey_file().
            if(errCode == X509_R_KEY_VALUES_MISMATCH || errCode == X509_R_KEY_TYPE_MISMATCH)
            {
                CertificateKeyMatchException certKeyMatchEx(__FILE__, __LINE__);

                certKeyMatchEx.message = "private key does not match the certificate public key";
                string sslError = sslGetErrors();

                if(!sslError.empty())
                {
                    certKeyMatchEx.message += "\n";
                    certKeyMatchEx.message += sslError;
                }

                throw certKeyMatchEx;
            }
            else
            {
                PrivateKeyLoadException pklEx(__FILE__, __LINE__);

                pklEx.message = "unable to load private key from '";
                pklEx.message += privKeyFile;
                pklEx.message += "'\n";
                pklEx.message += sslGetErrors();

	        throw pklEx;
            }
        }

        checkKeyCert();
    }
}

void
IceSSL::Context::addKeyCert(const RSAKeyPair& keyPair)
{
    if(_sslContext == 0)
    {
        ContextNotConfiguredException contextConfigEx(__FILE__, __LINE__);

        contextConfigEx.message = "ssl context not configured";

        throw contextConfigEx;
    }

    // Note: Normally I would use an X509Janitor and RSAJanitor to ensure that
    //       memory was being freed properly when exceptions are thrown, but
    //       both SSL_CTX_use_certificate and SSL_CTX_use_RSAPrivateKey free
    //       certificate/key memory regardless if the call succeeded.

    // Set which Public Key file to use.
    if(SSL_CTX_use_certificate(_sslContext, keyPair.getX509PublicKey()) <= 0)
    {
        CertificateLoadException certLoadEx(__FILE__, __LINE__);

        certLoadEx.message = "unable to set certificate from memory";
        string sslError = sslGetErrors();

        if(!sslError.empty())
        {
            certLoadEx.message += "\n";
            certLoadEx.message += sslError;
        }

        throw certLoadEx;
    }

    // Set which Private Key file to use.
    if(SSL_CTX_use_RSAPrivateKey(_sslContext, keyPair.getRSAPrivateKey()) <= 0)
    {
        int errCode = ERR_GET_REASON(ERR_peek_error());

        // Note: Because OpenSSL currently (V0.9.6b) performs a check to see if the
        //       key matches the private key when calling SSL_CTX_use_PrivateKey_file().
        if(errCode == X509_R_KEY_VALUES_MISMATCH || errCode == X509_R_KEY_TYPE_MISMATCH)
        {
            CertificateKeyMatchException certKeyMatchEx(__FILE__, __LINE__);

            certKeyMatchEx.message = "private key does not match the certificate public key";
            string sslError = sslGetErrors();

            if(!sslError.empty())
            {
                certKeyMatchEx.message += "\n";
                certKeyMatchEx.message += sslError;
            }

            throw certKeyMatchEx;
        }
        else
        {
            PrivateKeyLoadException pklEx(__FILE__, __LINE__);

            pklEx.message = "unable to set private key from memory";
            string sslError = sslGetErrors();

            if(!sslError.empty())
            {
                pklEx.message += "\n";
                pklEx.message += sslError;
            }

            throw pklEx;
        }
    }

    checkKeyCert();
}

void
IceSSL::Context::addKeyCert(const Ice::ByteSeq& privateKey, const Ice::ByteSeq& publicKey)
{
    Ice::ByteSeq privKey = privateKey;

    if(privKey.empty())
    {
        if(_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
        { 
            Trace out(_communicator->getLogger(), _traceLevels->securityCat);
            out << "WRN no private key specified -- using the certificate";
        }

        privKey = publicKey;
    }

    // Make a key pair based on the DER encoded byte sequences.
    RSAKeyPair rsaKeyPair(privKey, publicKey);
    addKeyCert(rsaKeyPair);
}

void
IceSSL::Context::addKeyCert(const string& privateKey, const string& publicKey)
{
    string privKey = privateKey;

    if(privKey.empty())
    {
        if(_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
        { 
            Trace out(_communicator->getLogger(), _traceLevels->securityCat);
            out << "WRN no private key specified -- using the certificate";
        }

        privKey = publicKey;
    }

    // Make a key pair based on the Base64 encoded strings.
    RSAKeyPair rsaKeyPair(privKey, publicKey);
    addKeyCert(rsaKeyPair);
}

SSL*
IceSSL::Context::createSSLConnection(int socket)
{
    assert(_sslContext != 0);

    SSL* sslConnection = SSL_new(_sslContext);
    assert(sslConnection != 0);

    SSL_clear(sslConnection);

    SSL_set_fd(sslConnection, socket);

    return sslConnection;
}

void
IceSSL::Context::setCipherList(const string& cipherList)
{
    assert(_sslContext != 0);

    if(!cipherList.empty() && (!SSL_CTX_set_cipher_list(_sslContext, cipherList.c_str())) &&
        (_traceLevels->security >= IceSSL::SECURITY_WARNINGS))
    {
        Trace out(_communicator->getLogger(), _traceLevels->securityCat);
        out << "WRN error setting cipher list " << cipherList << " -- using default list" << "\n";
        out << sslGetErrors();
    }
}

void
IceSSL::Context::setDHParams(const BaseCertificates& baseCerts)
{
    DH* dh = 0;

    string dhFile = baseCerts.getDHParams().getFileName();
    int encoding = baseCerts.getDHParams().getEncoding();

    // File type must be PEM - that's the only way we can load DH Params, apparently.
    if((!dhFile.empty()) && (encoding == SSL_FILETYPE_PEM))
    {
        dh = loadDHParam(dhFile.c_str());
    }

    if(dh == 0)
    {
        if(_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
        { 
            Trace out(_communicator->getLogger(), _traceLevels->securityCat);
            out << "WRN Could not load Diffie-Hellman params, generating a temporary 512bit key.";
        }

        dh = getTempDH512();
    }

    if(dh != 0)
    {
        SSL_CTX_set_tmp_dh(_sslContext, dh);

        DH_free(dh);
    }
}
