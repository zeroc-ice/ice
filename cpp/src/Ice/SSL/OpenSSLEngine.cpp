// Copyright (c) ZeroC, Inc.

// Default to OpenSSL 3.0.0 compatibility (decimal mmnnpp format)
// Hide deprecated APIs unless the user explicitly wants them
#define OPENSSL_API_COMPAT 30000
#define OPENSSL_NO_DEPRECATED

#include "OpenSSLEngine.h"
#include "../FileUtil.h"
#include "Ice/Communicator.h"
#include "Ice/Config.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Logger.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"
#include "Ice/StringUtil.h"
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
using namespace Ice::SSL;

extern "C"
{
    int Ice_SSL_opensslPasswordCallback(char* buf, int size, int /*flag*/, void* userData)
    {
        auto* p = reinterpret_cast<OpenSSL::SSLEngine*>(userData);
        assert(p);
        string passwd = p->password();
        int sz = static_cast<int>(passwd.size());
        if (sz > size)
        {
            sz = size - 1;
        }
        strncpy(buf, passwd.c_str(), sz);
        buf[sz] = '\0';

        for (auto& character : passwd)
        {
            character = '\0';
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

OpenSSL::SSLEngine::SSLEngine(const IceInternal::InstancePtr& instance) : Ice::SSL::SSLEngine(instance) {}

OpenSSL::SSLEngine::~SSLEngine() = default;

void
OpenSSL::SSLEngine::initialize()
{
    lock_guard lock(_mutex);
    try
    {
        Ice::SSL::SSLEngine::initialize();

        PropertiesPtr properties = getProperties();

        // Create an SSL context.
        _ctx = SSL_CTX_new(TLS_method());
        if (!_ctx)
        {
            throw InitializationException(__FILE__, __LINE__, "IceSSL: unable to create SSL context:\n" + sslErrors());
        }

        // Check for a default directory. We look in this directory for files mentioned in the configuration.
        const string defaultDir = properties->getIceProperty("IceSSL.DefaultDir");

        _password = properties->getIceProperty("IceSSL.Password");

        // Establish the location of CA certificates.
        {
            string path = properties->getIceProperty("IceSSL.CAs");
            optional<string> resolved;
            const char* file = nullptr;
            const char* dir = nullptr;
            if (!path.empty())
            {
                resolved = resolveFilePath(path, defaultDir);
                if (resolved)
                {
                    path = *resolved;
                    file = path.c_str();
                }

                if (!file)
                {
                    resolved = resolveDirPath(path, defaultDir);
                    if (resolved)
                    {
                        path = *resolved;
                        dir = path.c_str();
                    }
                }

                if (!file && !dir)
                {
                    ostringstream os;
                    os << "IceSSL: CA certificate path not found: '" << path << "'";
                    throw InitializationException(__FILE__, __LINE__, os.str());
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
            else if (properties->getIcePropertyAsInt("IceSSL.UsePlatformCAs") > 0)
            {
                SSL_CTX_set_default_verify_paths(_ctx);
            }
        }

        // Establish the certificate chain and private key.
        string certFile = properties->getIceProperty("IceSSL.CertFile");
        string keyFile = properties->getIceProperty("IceSSL.KeyFile");
        bool keyLoaded = false;

        if (!certFile.empty())
        {
            optional<string> resolved = resolveFilePath(certFile, defaultDir);
            if (!resolved)
            {
                ostringstream os;
                os << "IceSSL: certificate file not found '" << certFile << "'";
                throw InitializationException(__FILE__, __LINE__, os.str());
            }

            // First we try to load the certificate using PKCS12 format if that fails we fallback to PEM format.
            vector<char> buffer;
            readFile(*resolved, buffer);
            int success = 0;

            const unsigned char* b = reinterpret_cast<unsigned char*>(&buffer[0]);
            PKCS12* p12 = d2i_PKCS12(nullptr, &b, static_cast<long>(buffer.size()));
            if (p12)
            {
                EVP_PKEY* key = nullptr;
                X509* cert = nullptr;
                STACK_OF(X509)* chain = nullptr;

                try
                {
                    success = PKCS12_parse(p12, _password.c_str(), &key, &cert, &chain);

                    if (!success)
                    {
                        ostringstream os;
                        os << "IceSSL: error loading SSL certificate from PKCS12 file '" << certFile << "':\n"
                           << sslErrors();
                        throw InitializationException(__FILE__, __LINE__, os.str());
                    }

                    if (!cert)
                    {
                        ostringstream os;
                        os << "IceSSL: error loading SSL certificate from PKCS12 file '" << certFile << "':\n"
                           << "certificate not found";
                        throw InitializationException(__FILE__, __LINE__, os.str());
                    }

                    if (!SSL_CTX_use_certificate(_ctx, cert))
                    {
                        ostringstream os;
                        os << "IceSSL: error loading SSL certificate from PKCS12 file '" << certFile << "':\n"
                           << sslErrors();
                        throw InitializationException(__FILE__, __LINE__, os.str());
                    }

                    if (!key)
                    {
                        ostringstream os;
                        os << "IceSSL: error loading SSL private key from PKCS12 file '" << certFile << "':\n"
                           << "key not found";
                        throw InitializationException(__FILE__, __LINE__, os.str());
                    }

                    if (!SSL_CTX_use_PrivateKey(_ctx, key))
                    {
                        ostringstream os;
                        os << "IceSSL: error loading SSL private key from PKCS12 file '" << certFile << "':\n"
                           << sslErrors();
                        throw InitializationException(__FILE__, __LINE__, os.str());
                    }
                    keyLoaded = true;

                    if (chain && sk_X509_num(chain))
                    {
                        // Pop each cert from the stack so we can free the stack later.
                        // The CTX destruction will take care of the certificates
                        X509* c = nullptr;
                        while ((c = sk_X509_pop(chain)))
                        {
                            if (!SSL_CTX_add_extra_chain_cert(_ctx, c))
                            {
                                ostringstream os;
                                os << "IceSSL: unable to add extra SSL certificate:\n" << sslErrors();
                                throw InitializationException(__FILE__, __LINE__, os.str());
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
                success = SSL_CTX_use_certificate_chain_file(_ctx, resolved.value().c_str());
            }

            if (!success)
            {
                ostringstream os;
                os << "IceSSL: unable to load certificate chain from file '" << certFile << "':\n" << sslErrors();
                throw InitializationException(__FILE__, __LINE__, os.str());
            }
        }

        if (keyFile.empty())
        {
            keyFile = certFile; // Assume the certificate file also contains the private key.
        }

        if (!keyLoaded && !keyFile.empty())
        {
            optional<string> resolved = resolveFilePath(keyFile, defaultDir);
            if (!resolved)
            {
                ostringstream os;
                os << "IceSSL: key file not found: '" << keyFile << "'";
                throw InitializationException(__FILE__, __LINE__, os.str());
            }

            // The private key may be stored in an encrypted file.
            if (!SSL_CTX_use_PrivateKey_file(_ctx, resolved.value().c_str(), SSL_FILETYPE_PEM))
            {
                ostringstream os;
                os << "IceSSL: error loading SSL private key from '" << keyFile << "':\n" << sslErrors();
                throw InitializationException(__FILE__, __LINE__, os.str());
            }
            keyLoaded = true;
        }

        if (keyLoaded && !SSL_CTX_check_private_key(_ctx))
        {
            ostringstream os;
            os << "IceSSL: unable to validate private key:\n" << sslErrors();
            throw InitializationException(__FILE__, __LINE__, os.str());
        }

        int revocationCheck = getRevocationCheck();
        if (revocationCheck > 0)
        {
            vector<string> crlFiles = properties->getIcePropertyAsList("IceSSL.CertificateRevocationListFiles");
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
                optional<string> resolved = resolveFilePath(crlFile, defaultDir);
                if (!resolved)
                {
                    ostringstream os;
                    os << "IceSSL: CRL file not found '" << crlFile << "'";
                    throw InitializationException(__FILE__, __LINE__, os.str());
                }

                if (X509_LOOKUP_load_file(lookup, resolved.value().c_str(), X509_FILETYPE_PEM) == 0)
                {
                    ostringstream os;
                    os << "IceSSL: CRL load failure '" << crlFile << "'";
                    throw InitializationException(__FILE__, __LINE__, os.str());
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

        // Store a pointer to ourself for use in OpenSSL callbacks.
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
        // pointer to this object.
        //
        SSL_CTX_set_session_id_context(
            _ctx,
            reinterpret_cast<unsigned char*>(this),
            static_cast<unsigned int>(sizeof(this)));
    }
    catch (...)
    {
        SSL_CTX_free(_ctx);
        _ctx = nullptr;
        throw;
    }
}

ClientAuthenticationOptions
OpenSSL::SSLEngine::createClientAuthenticationOptions(const std::string&) const
{
    return ClientAuthenticationOptions{
        .clientSSLContextSelectionCallback =
            [this](const string&)
        {
            // Ensure the SSL context remains valid for the lifetime of the connection.
            SSL_CTX_up_ref(_ctx);
            return _ctx;
        },
        .sslNewSessionCallback =
            [this](::SSL* ssl, const string& host)
        {
            if (getCheckCertName())
            {
                X509_VERIFY_PARAM* param = SSL_get0_param(ssl);
                if (IceInternal::isIpAddress(host))
                {
                    if (!X509_VERIFY_PARAM_set1_ip_asc(param, host.c_str()))
                    {
                        ostringstream os;
                        os << "IceSSL: error setting the expected IP address '" << host << "'";
                        throw SecurityException(__FILE__, __LINE__, os.str());
                    }
                }
                else
                {
                    if (!X509_VERIFY_PARAM_set1_host(param, host.c_str(), 0))
                    {
                        ostringstream os;
                        os << "IceSSL: error setting the expected host name '" << host << "'";
                        throw SecurityException(__FILE__, __LINE__, os.str());
                    }
                }
            }
            SSL_set_verify(ssl, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
        },
        .serverCertificateValidationCallback =
            [this](bool ok, X509_STORE_CTX* ctx, const Ice::SSL::ConnectionInfoPtr& info)
        { return validationCallback(ok, ctx, info); },
    };
}

ServerAuthenticationOptions
OpenSSL::SSLEngine::createServerAuthenticationOptions() const
{
    return ServerAuthenticationOptions{
        .serverSSLContextSelectionCallback =
            [this](const string&)
        {
            // Ensure the SSL context remains valid for the lifetime of the connection.
            SSL_CTX_up_ref(_ctx);
            return _ctx;
        },
        .sslNewSessionCallback =
            [this](::SSL* ssl, const string&)
        {
            int sslVerifyMode;
            switch (getVerifyPeer())
            {
                case 0:
                    sslVerifyMode = SSL_VERIFY_NONE;
                    break;
                case 1:
                    sslVerifyMode = SSL_VERIFY_PEER;
                    break;
                default:
                    sslVerifyMode = SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
                    break;
            }
            SSL_set_verify(ssl, sslVerifyMode, nullptr);
        },
        .clientCertificateValidationCallback =
            [this](bool ok, X509_STORE_CTX* ctx, const Ice::SSL::ConnectionInfoPtr& info)
        { return validationCallback(ok, ctx, info); }};
}

bool
OpenSSL::SSLEngine::validationCallback(bool ok, X509_STORE_CTX* ctx, const Ice::SSL::ConnectionInfoPtr& info) const
{
    // At this point before the SSL handshake is completed, the connection info doesn't contain the peer's
    // certificate chain required for verifyPeer. We set it here.
    int depth = X509_STORE_CTX_get_error_depth(ctx);
    if (ok && depth == 0)
    {
        // Only called for the peer certificate.
        verifyPeer(info);
    }
    return ok;
}

string
OpenSSL::SSLEngine::sslErrors() const
{
    return getErrors();
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
