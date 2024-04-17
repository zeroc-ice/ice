//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "OpenSSLEngine.h"
#include "Ice/Communicator.h"
#include "Ice/Config.h"
#include "Ice/LocalException.h"
#include "Ice/Logger.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"
#include "IceUtil/FileUtil.h"
#include "IceUtil/StringUtil.h"
#include "OpenSSLEngineF.h"
#include "OpenSSLTransceiverI.h"
#include "SSLUtil.h"
#include "TrustManager.h"

#include <mutex>

#include <openssl/err.h>
#include <openssl/pkcs12.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>

#ifdef _MSC_VER
#    pragma warning(disable : 4127) // conditional expression is constant
#elif defined(__GNUC__)
#    // Ignore OpenSSL 3.0 deprecation warning
#    pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

using namespace std;
using namespace Ice;
using namespace IceSSL;

extern "C"
{
    int IceSSL_opensslPasswordCallback(char* buf, int size, int /*flag*/, void* userData)
    {
        OpenSSL::SSLEngine* p = reinterpret_cast<OpenSSL::SSLEngine*>(userData);
        assert(p);
        string passwd = p->password();
        int sz = static_cast<int>(passwd.size());
        if (sz > size)
        {
            sz = size - 1;
        }
        strncpy(buf, passwd.c_str(), sz);
        buf[sz] = '\0';

        for (string::iterator i = passwd.begin(); i != passwd.end(); ++i)
        {
            *i = '\0';
        }

        return sz;
    }
}

namespace
{
    bool passwordError()
    {
        int reason = ERR_GET_REASON(ERR_peek_error());
        return (
            reason == PEM_R_BAD_BASE64_DECODE || reason == PEM_R_BAD_DECRYPT || reason == PEM_R_BAD_PASSWORD_READ ||
            reason == PEM_R_PROBLEMS_GETTING_PASSWORD || reason == PKCS12_R_MAC_VERIFY_FAILURE);
    }
}

OpenSSL::SSLEngine::SSLEngine(const IceInternal::InstancePtr& instance) : IceSSL::SSLEngine(instance), _ctx(0) {}

OpenSSL::SSLEngine::~SSLEngine() {}

void
OpenSSL::SSLEngine::initialize()
{
    lock_guard lock(_mutex);
    try
    {
        IceSSL::SSLEngine::initialize();

        const string propPrefix = "IceSSL.";
        PropertiesPtr properties = getProperties();

        // Create an SSL context if the application hasn't supplied one.
        if (!_ctx)
        {
            _ctx = SSL_CTX_new(TLS_method());
            if (!_ctx)
            {
                throw InitializationException(
                    __FILE__,
                    __LINE__,
                    "IceSSL: unable to create SSL context:\n" + sslErrors());
            }

            int securityLevel = properties->getPropertyAsIntWithDefault(propPrefix + "SecurityLevel", -1);
            if (securityLevel != -1)
            {
                SSL_CTX_set_security_level(_ctx, securityLevel);
                if (SSL_CTX_get_security_level(_ctx) != securityLevel)
                {
                    throw InitializationException(
                        __FILE__,
                        __LINE__,
                        "IceSSL: unable to set SSL security level:\n" + sslErrors());
                }
            }

            // Check for a default directory. We look in this directory for files mentioned in the configuration.
            const string defaultDir = properties->getProperty(propPrefix + "DefaultDir");

            _password = properties->getProperty(propPrefix + "Password");

            // Establish the location of CA certificates.
            {
                string path = properties->getProperty(propPrefix + "CAs");
                string resolved;
                const char* file = nullptr;
                const char* dir = nullptr;
                if (!path.empty())
                {
                    if (checkPath(path, defaultDir, false, resolved))
                    {
                        path = resolved;
                        file = path.c_str();
                    }

                    if (!file)
                    {
                        if (checkPath(path, defaultDir, true, resolved))
                        {
                            path = resolved;
                            dir = path.c_str();
                        }
                    }

                    if (!file && !dir)
                    {
                        throw InitializationException(
                            __FILE__,
                            __LINE__,
                            "IceSSL: CA certificate path not found:\n" + path);
                    }
                }

                if (file || dir)
                {
                    // The certificate may be stored in an encrypted file.
                    if (!SSL_CTX_load_verify_locations(_ctx, file, dir))
                    {
                        string msg = "IceSSL: unable to establish CA certificates";
                        if (passwordError())
                        {
                            msg += ":\ninvalid password";
                        }
                        else
                        {
                            string err = sslErrors();
                            if (!err.empty())
                            {
                                msg += ":\n" + err;
                            }
                        }
                        throw InitializationException(__FILE__, __LINE__, msg);
                    }
                }
                else if (properties->getPropertyAsInt("IceSSL.UsePlatformCAs") > 0)
                {
                    SSL_CTX_set_default_verify_paths(_ctx);
                }
            }

            // Establish the certificate chains and private keys. One RSA certificate and one DSA certificate are
            // allowed.
            string certFile = properties->getProperty(propPrefix + "CertFile");
            string keyFile = properties->getProperty(propPrefix + "KeyFile");
            bool keyLoaded = false;

            vector<string>::size_type numCerts = 0;
            if (!certFile.empty())
            {
                vector<string> files;
                if (!IceUtilInternal::splitString(certFile, IceUtilInternal::pathsep, files) || files.size() > 2)
                {
                    throw InitializationException(
                        __FILE__,
                        __LINE__,
                        "IceSSL: invalid value for " + propPrefix + "CertFile:\n" + certFile);
                }
                numCerts = files.size();
                for (const string& file : files)
                {
                    string resolved;
                    if (!checkPath(file, defaultDir, false, resolved))
                    {
                        throw InitializationException(
                            __FILE__,
                            __LINE__,
                            "IceSSL: certificate file not found:\n" + file);
                    }

                    // First we try to load the certificate using PKCS12 format if that fails we fallback to PEM format.
                    vector<char> buffer;
                    readFile(resolved, buffer);
                    int success = 0;

                    const unsigned char* b =
                        const_cast<const unsigned char*>(reinterpret_cast<unsigned char*>(&buffer[0]));
                    PKCS12* p12 = d2i_PKCS12(0, &b, static_cast<long>(buffer.size()));
                    if (p12)
                    {
                        EVP_PKEY* key = nullptr;
                        X509* cert = nullptr;
                        STACK_OF(X509)* chain = nullptr;

                        try
                        {
                            success = PKCS12_parse(p12, _password.c_str(), &key, &cert, &chain);

                            if (!cert || !SSL_CTX_use_certificate(_ctx, cert))
                            {
                                throw InitializationException(
                                    __FILE__,
                                    __LINE__,
                                    "IceSSL: unable to load SSL certificate:\n" +
                                        (cert ? sslErrors() : "certificate not found"));
                            }

                            if (!key || !SSL_CTX_use_PrivateKey(_ctx, key))
                            {
                                throw InitializationException(
                                    __FILE__,
                                    __LINE__,
                                    "IceSSL: unable to load SSL private key:\n" +
                                        (key ? sslErrors() : "key not found"));
                            }
                            keyLoaded = true;

                            if (chain && sk_X509_num(chain))
                            {
                                // Pop each cert from the stack so we can free the stack later.
                                // The CTX destruction will take care of the certificates
                                X509* c = nullptr;
                                while ((c = sk_X509_pop(chain)) != 0)
                                {
                                    if (!SSL_CTX_add_extra_chain_cert(_ctx, c))
                                    {
                                        throw InitializationException(
                                            __FILE__,
                                            __LINE__,
                                            "IceSSL: unable to add extra SSL certificate:\n" + sslErrors());
                                    }
                                }
                            }

                            if (chain)
                            {
                                // This chain should now be empty. No need to call sk_X509_pop_free()
                                sk_X509_free(chain);
                            }
                            assert(key && cert);
                            EVP_PKEY_free(key);
                            X509_free(cert);
                            PKCS12_free(p12);
                        }
                        catch (...)
                        {
                            PKCS12_free(p12);
                            if (chain)
                            {
                                sk_X509_pop_free(chain, X509_free);
                            }

                            if (key)
                            {
                                EVP_PKEY_free(key);
                            }

                            if (cert)
                            {
                                X509_free(cert);
                            }
                            throw;
                        }
                    }
                    else
                    {
                        // The certificate may be stored in an encrypted file.
                        success = SSL_CTX_use_certificate_chain_file(_ctx, resolved.c_str());
                    }

                    if (!success)
                    {
                        string msg = "IceSSL: unable to load certificate chain from file " + file;
                        if (passwordError())
                        {
                            msg += ":\ninvalid password";
                        }
                        else
                        {
                            string err = sslErrors();
                            if (!err.empty())
                            {
                                msg += ":\n" + err;
                            }
                        }
                        throw InitializationException(__FILE__, __LINE__, msg);
                    }
                }
            }

            if (keyFile.empty())
            {
                keyFile = certFile; // Assume the certificate file also contains the private key.
            }

            if (!keyLoaded && !keyFile.empty())
            {
                vector<string> files;
                if (!IceUtilInternal::splitString(keyFile, IceUtilInternal::pathsep, files) || files.size() > 2)
                {
                    throw InitializationException(
                        __FILE__,
                        __LINE__,
                        "IceSSL: invalid value for " + propPrefix + "KeyFile:\n" + keyFile);
                }
                if (files.size() != numCerts)
                {
                    throw InitializationException(
                        __FILE__,
                        __LINE__,
                        "IceSSL: " + propPrefix + "KeyFile does not agree with " + propPrefix + "CertFile");
                }

                for (const auto& file : files)
                {
                    string resolved;
                    if (!checkPath(file, defaultDir, false, resolved))
                    {
                        throw InitializationException(__FILE__, __LINE__, "IceSSL: key file not found:\n" + file);
                    }

                    // The private key may be stored in an encrypted file.
                    if (!SSL_CTX_use_PrivateKey_file(_ctx, resolved.c_str(), SSL_FILETYPE_PEM))
                    {
                        ostringstream os;
                        os << "IceSSL: unable to load private key from file " << file;
                        if (passwordError())
                        {
                            os << ":\ninvalid password";
                        }
                        else
                        {
                            string errStr = sslErrors();
                            if (!errStr.empty())
                            {
                                os << ":\n" << errStr;
                            }
                        }
                        throw InitializationException(__FILE__, __LINE__, os.str());
                    }
                }
                keyLoaded = true;
            }

            if (keyLoaded && !SSL_CTX_check_private_key(_ctx))
            {
                throw InitializationException(
                    __FILE__,
                    __LINE__,
                    "IceSSL: unable to validate private key(s):\n" + sslErrors());
            }

            int revocationCheck = getRevocationCheck();
            if (revocationCheck > 0)
            {
                vector<string> crlFiles = properties->getPropertyAsList(propPrefix + "CertificateRevocationListFiles");
                if (crlFiles.empty())
                {
                    throw InitializationException(
                        __FILE__,
                        __LINE__,
                        "IceSSL: cannot enable revocation checks without setting certificate revocation list files");
                }

                X509_STORE* store = SSL_CTX_get_cert_store(_ctx);
                if (!store)
                {
                    throw InitializationException(__FILE__, __LINE__, "IceSSL: unable to obtain the certificate store");
                }

                X509_LOOKUP* lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());
                if (!lookup)
                {
                    throw InitializationException(__FILE__, __LINE__, "IceSSL: add lookup failed");
                }

                for (const string& crlFile : crlFiles)
                {
                    string resolved;
                    if (!checkPath(crlFile, defaultDir, false, resolved))
                    {
                        throw InitializationException(
                            __FILE__,
                            __LINE__,
                            "IceSSL: CRL file not found `" + crlFile + "'");
                    }

                    if (X509_LOOKUP_load_file(lookup, resolved.c_str(), X509_FILETYPE_PEM) == 0)
                    {
                        throw InitializationException(__FILE__, __LINE__, "IceSSL: CRL load failure `" + crlFile + "'");
                    }
                }

                unsigned long flags = X509_V_FLAG_CRL_CHECK;
                if (revocationCheck > 1)
                {
                    flags |= X509_V_FLAG_CRL_CHECK_ALL;
                }
                X509_STORE_set_flags(store, flags);
            }

            SSL_CTX_set_mode(_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);
        }

        //
        // Store a pointer to ourself for use in OpenSSL callbacks.
        //
        SSL_CTX_set_ex_data(_ctx, 0, this);

        //
        // This is necessary for successful interop with Java. Without it, a Java
        // client would fail to reestablish a connection: the server gets the
        // error "session id context uninitialized" and the client receives
        // "SSLHandshakeException: Remote host closed connection during handshake".
        //
        SSL_CTX_set_session_cache_mode(_ctx, SSL_SESS_CACHE_OFF);

        //
        // Although we disable session caching, we still need to set a session ID
        // context (ICE-5103). The value can be anything; here we just use the
        // pointer to this SharedInstance object.
        //
        SSL_CTX_set_session_id_context(
            _ctx,
            reinterpret_cast<unsigned char*>(this),
            static_cast<unsigned int>(sizeof(this)));
    }
    catch (...)
    {
        //
        // We free the SSL context regardless of whether the plugin created it
        // or the application supplied it.
        //
        SSL_CTX_free(_ctx);
        _ctx = nullptr;
        throw;
    }
}

SSL_CTX*
OpenSSL::SSLEngine::context() const
{
    return _ctx;
}

string
OpenSSL::SSLEngine::sslErrors() const
{
    return getSslErrors(securityTraceLevel() >= 1);
}

void
OpenSSL::SSLEngine::destroy()
{
    if (_ctx)
    {
        SSL_CTX_free(_ctx);
        _ctx = nullptr;
    }
}

IceInternal::TransceiverPtr
OpenSSL::SSLEngine::createTransceiver(
    const InstancePtr& instance,
    const IceInternal::TransceiverPtr& delegate,
    const string& hostOrAdapterName,
    bool incoming)
{
    return make_shared<OpenSSL::TransceiverI>(instance, delegate, hostOrAdapterName, incoming);
}
