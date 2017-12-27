// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Config.h>
#ifdef _WIN32
#   include <winsock2.h>
#endif

#include <IceSSL/Instance.h>
#include <IceSSL/EndpointI.h>
#include <IceSSL/Util.h>
#include <IceSSL/TrustManager.h>

#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/Logger.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Properties.h>
#include <Ice/ProtocolPluginFacade.h>
#include <Ice/StringConverter.h>

#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/StringUtil.h>

#include <openssl/rand.h>
#include <openssl/err.h>

#include <IceUtil/DisableWarnings.h>

// Ignore OS X OpenSSL deprecation warnings
#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceUtil::Shared* IceSSL::upCast(IceSSL::Instance* p) { return p; }

namespace
{

IceUtil::Mutex* staticMutex = 0;
int instanceCount = 0;
IceUtil::Mutex* locks = 0;

class Init
{
public:

    Init()
    {
        staticMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete staticMutex;
        staticMutex = 0;

        if(locks)
        {
            delete[] locks;
            locks = 0;
        }
    }
};

Init init;

}

extern "C"
{

//
// OpenSSL mutex callback.
//
void
IceSSL_opensslLockCallback(int mode, int n, const char* /*file*/, int /*line*/)
{
    assert(locks);
    if(mode & CRYPTO_LOCK)
    {
        locks[n].lock();
    }
    else
    {
        locks[n].unlock();
    }
}

//
// OpenSSL thread id callback.
//
unsigned long
IceSSL_opensslThreadIdCallback()
{
#if defined(_WIN32)
    return static_cast<unsigned long>(GetCurrentThreadId());
#elif defined(__FreeBSD__) || defined(__APPLE__) || defined(__osf1__)
    //
    // On some platforms, pthread_t is a pointer to a per-thread structure.
    //
    return reinterpret_cast<unsigned long>(pthread_self());
#elif (defined(__linux) || defined(__sun) || defined(__hpux)) || defined(_AIX)
    //
    // On Linux, Solaris, HP-UX and AIX, pthread_t is an integer.
    //
    return static_cast<unsigned long>(pthread_self());
#else
#   error "Unknown platform"
#endif
}

int
IceSSL_opensslPasswordCallback(char* buf, int size, int flag, void* userData)
{
    IceSSL::Instance* p = reinterpret_cast<IceSSL::Instance*>(userData);
    string passwd = p->password(flag == 1);
    int sz = static_cast<int>(passwd.size());
    if(sz > size)
    {
        sz = size - 1;
    }
    strncpy(buf, passwd.c_str(), sz);
    buf[sz] = '\0';

    for(string::iterator i = passwd.begin(); i != passwd.end(); ++i)
    {
        *i = '\0';
    } 

    return sz;
}

#ifndef OPENSSL_NO_DH
DH*
IceSSL_opensslDHCallback(SSL* ssl, int /*isExport*/, int keyLength)
{
#if defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER >= 0x1010000fL
    SSL_CTX* sslCtx = SSL_get_SSL_CTX(ssl);
#else
    SSL_CTX* sslCtx = ssl->ctx;
#endif
    IceSSL::Instance* p = reinterpret_cast<IceSSL::Instance*>(SSL_CTX_get_ex_data(sslCtx, 0));
    return p->dhParams(keyLength);
}
#endif

int
IceSSL_opensslVerifyCallback(int ok, X509_STORE_CTX* ctx)
{
    SSL* ssl = reinterpret_cast<SSL*>(X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
#if defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER >= 0x1010000fL
    SSL_CTX* sslCtx = SSL_get_SSL_CTX(ssl);
#else
    SSL_CTX* sslCtx = ssl->ctx;
#endif
    IceSSL::Instance* p = reinterpret_cast<IceSSL::Instance*>(SSL_CTX_get_ex_data(sslCtx, 0));
    return p->verifyCallback(ok, ssl, ctx);
}

}

static bool
passwordError()
{
    int reason = ERR_GET_REASON(ERR_peek_error());
    return (reason == PEM_R_BAD_BASE64_DECODE ||
            reason == PEM_R_BAD_DECRYPT ||
            reason == PEM_R_BAD_PASSWORD_READ ||
            reason == PEM_R_PROBLEMS_GETTING_PASSWORD);
}

IceSSL::Instance::Instance(const CommunicatorPtr& communicator) :
    _logger(communicator->getLogger()),
    _initialized(false),
    _ctx(0)
{
    __setNoDelete(true);

    //
    // Initialize OpenSSL if necessary.
    //
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(staticMutex);
    instanceCount++;

    if(instanceCount == 1)
    {
        PropertiesPtr properties = communicator->getProperties();

        //
        // The IceSSL.InitOpenSSL property specifies whether we should perform the global
        // startup (and shutdown) tasks for the OpenSSL library.
        //
        // If an application uses multiple components that each depend on OpenSSL, the
        // application should disable OpenSSL initialization in those components and
        // perform the initialization itself.
        //
        _initOpenSSL = properties->getPropertyAsIntWithDefault("IceSSL.InitOpenSSL", 1) > 0;
        if(_initOpenSSL)
        {
            //
            // Create the mutexes and set the callbacks.
            //
            if(!locks)
            {
                locks = new IceUtil::Mutex[CRYPTO_num_locks()];
                CRYPTO_set_locking_callback(IceSSL_opensslLockCallback);
                CRYPTO_set_id_callback(IceSSL_opensslThreadIdCallback);
            }

            //
            // Load human-readable error messages.
            //
            SSL_load_error_strings();

            //
            // Initialize the SSL library.
            //
            SSL_library_init();

            //
            // This is necessary to allow programs that use OpenSSL 0.9.x to
            // load private key files generated by OpenSSL 1.x.
            //
            OpenSSL_add_all_algorithms();

            //
            // Initialize the PRNG.
            //
#ifdef WINDOWS
            RAND_screen(); // Uses data from the screen if possible.
#endif
            char randFile[1024];
            if(RAND_file_name(randFile, sizeof(randFile))) // Gets the name of a default seed file.
            {
                RAND_load_file(randFile, 1024);
            }
            string randFiles = Ice::nativeToUTF8(communicator, properties->getProperty("IceSSL.Random"));

            if(!randFiles.empty())
            {
                vector<string> files;
#ifdef _WIN32
                const string sep = ";";
#else
                const string sep = ":";
#endif
                string defaultDir = Ice::nativeToUTF8(communicator, properties->getProperty("IceSSL.DefaultDir"));

                if(!IceUtilInternal::splitString(randFiles, sep, files))
                {
                    PluginInitializationException ex(__FILE__, __LINE__);
                    ex.reason = "IceSSL: invalid value for IceSSL.Random:\n" + randFiles;
                    throw ex;
                }
                for(vector<string>::iterator p = files.begin(); p != files.end(); ++p)
                {
                    string file = *p;
                    if(!checkPath(file, defaultDir, false))
                    {
                        PluginInitializationException ex(__FILE__, __LINE__);
                        ex.reason = "IceSSL: entropy data file not found:\n" + file;
                        throw ex;
                    }
                    if(!RAND_load_file(file.c_str(), 1024))
                    {
                        PluginInitializationException ex(__FILE__, __LINE__);
                        ex.reason = "IceSSL: unable to load entropy data from " + file;
                        throw ex;
                    }
                }
            }
#if !defined (_WIN32) && !defined (OPENSSL_NO_EGD)
            //
            // The Entropy Gathering Daemon (EGD) is not available on Windows.
            // The file should be a Unix domain socket for the daemon.
            //
            string entropyDaemon = properties->getProperty("IceSSL.EntropyDaemon");
            if(!entropyDaemon.empty())
            {
                if(RAND_egd(entropyDaemon.c_str()) <= 0)
                {
                    PluginInitializationException ex(__FILE__, __LINE__);
                    ex.reason = "IceSSL: EGD failure using file " + entropyDaemon;
                    throw ex;
                }
            }
#endif
            if(!RAND_status())
            {
                communicator->getLogger()->warning("IceSSL: insufficient data to initialize PRNG");
            }
        }
        else
        {
            if(!properties->getProperty("IceSSL.Random").empty())
            {
                _logger->warning("IceSSL: ignoring IceSSL.Random because OpenSSL initialization is disabled");
            }
#ifndef _WIN32
            else if(!properties->getProperty("IceSSL.EntropyDaemon").empty())
            {
                _logger->warning("IceSSL: ignoring IceSSL.EntropyDaemon because OpenSSL initialization is disabled");
            }
#endif
        }
    }

    _facade = IceInternal::getProtocolPluginFacade(communicator);
    _securityTraceLevel = communicator->getProperties()->getPropertyAsInt("IceSSL.Trace.Security");
    _securityTraceCategory = "Security";
    _trustManager = new TrustManager(communicator);
    
    //
    // Register the endpoint factory. We have to do this now, rather than
    // in initialize, because the communicator may need to interpret
    // proxies before the plug-in is fully initialized.
    //
    _facade->addEndpointFactory(new EndpointFactoryI(this));

    __setNoDelete(false);
}

IceSSL::Instance::~Instance()
{
    //
    // Clean up OpenSSL resources.
    //
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(staticMutex);

    if(--instanceCount == 0 && _initOpenSSL)
    {
        //
        // NOTE: We can't destroy the locks here: threads which might have called openssl methods
        // might access openssl locks upon termination (from DllMain/THREAD_DETACHED). Instead, 
        // we release the locks in the ~Init() static destructor. See bug #4156.
        //
        //CRYPTO_set_locking_callback(0);
        //CRYPTO_set_id_callback(0);
        //delete[] locks;
        //locks = 0;

        CRYPTO_cleanup_all_ex_data();
        RAND_cleanup();
        ERR_free_strings();
        EVP_cleanup();
    }
}

void
IceSSL::Instance::initialize()
{
    if(_initialized)
    {
        return;
    }

    try
    {
        const string propPrefix = "IceSSL.";
        PropertiesPtr properties = communicator()->getProperties();

        //
        // CheckCertName determines whether we compare the name in a peer's
        // certificate against its hostname.
        //
        _checkCertName = properties->getPropertyAsIntWithDefault(propPrefix + "CheckCertName", 0) > 0;

        //
        // VerifyDepthMax establishes the maximum length of a peer's certificate
        // chain, including the peer's certificate. A value of 0 means there is
        // no maximum.
        //
        _verifyDepthMax = properties->getPropertyAsIntWithDefault(propPrefix + "VerifyDepthMax", 2);

        //
        // VerifyPeer determines whether certificate validation failures abort a connection.
        //
        _verifyPeer = properties->getPropertyAsIntWithDefault(propPrefix + "VerifyPeer", 2);

        //
        // Protocols selects which protocols to enable.
        //
        const int protocols = parseProtocols(properties->getPropertyAsList(propPrefix + "Protocols"));

        //
        // Create an SSL context if the application hasn't supplied one.
        //
        if(!_ctx)
        {
            _ctx = SSL_CTX_new(getMethod(protocols));
            if(!_ctx)
            {
                PluginInitializationException ex(__FILE__, __LINE__);
                ex.reason = "IceSSL: unable to create SSL context:\n" + sslErrors();
                throw ex;
            }

            //
            // Check for a default directory. We look in this directory for
            // files mentioned in the configuration.
            //
            string defaultDir = properties->getProperty(propPrefix + "DefaultDir");

            //
            // If the configuration defines a password, or the application has supplied
            // a password prompt object, then register a password callback. Otherwise,
            // let OpenSSL use its default behavior.
            //
            {
                // TODO: Support quoted value?
                string password = properties->getProperty(propPrefix + "Password");
                if(!password.empty() || _prompt)
                {
                    SSL_CTX_set_default_passwd_cb(_ctx, IceSSL_opensslPasswordCallback);
                    SSL_CTX_set_default_passwd_cb_userdata(_ctx, this);
                    _password = password;
                }
            }

            int passwordRetryMax = properties->getPropertyAsIntWithDefault(propPrefix + "PasswordRetryMax", 3);

            //
            // Establish the location of CA certificates.
            //
            {
                string caFile = properties->getProperty(propPrefix + "CertAuthFile");
                string caDir = properties->getPropertyWithDefault(propPrefix + "CertAuthDir", defaultDir);
                const char* file = 0;
                const char* dir = 0;
                if(!caFile.empty())
                {
                    if(!checkPath(caFile, defaultDir, false))
                    {
                        PluginInitializationException ex(__FILE__, __LINE__);
                        ex.reason = "IceSSL: CA certificate file not found:\n" + caFile;
                        throw ex;
                    }
                    file = caFile.c_str();
                }
                if(!caDir.empty())
                {
                    if(!checkPath(caDir, defaultDir, true))
                    {
                        PluginInitializationException ex(__FILE__, __LINE__);
                        ex.reason = "IceSSL: CA certificate directory not found:\n" + caDir;
                        throw ex;
                    }
                    dir = caDir.c_str();
                }
                if(file || dir)
                {
                    //
                    // The certificate may be stored in an encrypted file, so handle
                    // password retries.
                    //
                    int count = 0;
                    int err = 0;
                    while(count < passwordRetryMax)
                    {
                        ERR_clear_error();
                        err = SSL_CTX_load_verify_locations(_ctx, file, dir);
                        if(err)
                        {
                            break;
                        }
                        ++count;
                    }
                    if(err == 0)
                    {
                        string msg = "IceSSL: unable to establish CA certificates";
                        if(passwordError())
                        {
                            msg += ":\ninvalid password";
                        }
                        else
                        {
                            string err = sslErrors();
                            if(!err.empty())
                            {
                                msg += ":\n" + err;
                            }
                        }
                        PluginInitializationException ex(__FILE__, __LINE__);
                        ex.reason = msg;
                        throw ex;
                    }
                }
            }

            //
            // Establish the certificate chains and private keys. One RSA certificate and
            // one DSA certificate are allowed.
            //
            {
#ifdef _WIN32
                const string sep = ";";
#else
                const string sep = ":";
#endif
                string certFile = properties->getProperty(propPrefix + "CertFile");
                string keyFile = properties->getProperty(propPrefix + "KeyFile");
                vector<string>::size_type numCerts = 0;
                if(!certFile.empty())
                {
                    vector<string> files;
                    if(!IceUtilInternal::splitString(certFile, sep, files) || files.size() > 2)
                    {
                        PluginInitializationException ex(__FILE__, __LINE__);
                        ex.reason = "IceSSL: invalid value for " + propPrefix + "CertFile:\n" + certFile;
                        throw ex;
                    }
                    numCerts = files.size();
                    for(vector<string>::iterator p = files.begin(); p != files.end(); ++p)
                    {
                        string file = *p;
                        if(!checkPath(file, defaultDir, false))
                        {
                            PluginInitializationException ex(__FILE__, __LINE__);
                            ex.reason = "IceSSL: certificate file not found:\n" + file;
                            throw ex;
                        }
                        //
                        // The certificate may be stored in an encrypted file, so handle
                        // password retries.
                        //
                        int count = 0;
                        int err = 0;
                        while(count < passwordRetryMax)
                        {
                            ERR_clear_error();
                            err = SSL_CTX_use_certificate_chain_file(_ctx, file.c_str());
                            if(err)
                            {
                                break;
                            }
                            ++count;
                        }
                        if(err == 0)
                        {
                            string msg = "IceSSL: unable to load certificate chain from file " + file;
                            if(passwordError())
                            {
                                msg += ":\ninvalid password";
                            }
                            else
                            {
                                string err = sslErrors();
                                if(!err.empty())
                                {
                                    msg += ":\n" + err;
                                }
                            }
                            PluginInitializationException ex(__FILE__, __LINE__);
                            ex.reason = msg;
                            throw ex;
                        }
                    }
                }
                if(keyFile.empty())
                {
                    keyFile = certFile; // Assume the certificate file also contains the private key.
                }
                if(!keyFile.empty())
                {
                    vector<string> files;
                    if(!IceUtilInternal::splitString(keyFile, sep, files) || files.size() > 2)
                    {
                        PluginInitializationException ex(__FILE__, __LINE__);
                        ex.reason = "IceSSL: invalid value for " + propPrefix + "KeyFile:\n" + keyFile;
                        throw ex;
                    }
                    if(files.size() != numCerts)
                    {
                        PluginInitializationException ex(__FILE__, __LINE__);
                        ex.reason = "IceSSL: " + propPrefix + "KeyFile does not agree with " + propPrefix + "CertFile";
                        throw ex;
                    }
                    for(vector<string>::iterator p = files.begin(); p != files.end(); ++p)
                    {
                        string file = *p;
                        if(!checkPath(file, defaultDir, false))
                        {
                            PluginInitializationException ex(__FILE__, __LINE__);
                            ex.reason = "IceSSL: key file not found:\n" + file;
                            throw ex;
                        }
                        //
                        // The private key may be stored in an encrypted file, so handle
                        // password retries.
                        //
                        int count = 0;
                        int err = 0;
                        while(count < passwordRetryMax)
                        {
                            ERR_clear_error();
                            err = SSL_CTX_use_PrivateKey_file(_ctx, file.c_str(), SSL_FILETYPE_PEM);
                            if(err)
                            {
                                break;
                            }
                            ++count;
                        }
                        if(err == 0)
                        {
                            string msg = "IceSSL: unable to load private key from file " + file;
                            if(passwordError())
                            {
                                msg += ":\ninvalid password";
                            }
                            else
                            {
                                string err = sslErrors();
                                if(!err.empty())
                                {
                                    msg += ":\n" + err;
                                }
                            }
                            PluginInitializationException ex(__FILE__, __LINE__);
                            ex.reason = msg;
                            throw ex;
                        }
                    }
                    if(!SSL_CTX_check_private_key(_ctx))
                    {
                        PluginInitializationException ex(__FILE__, __LINE__);
                        ex.reason = "IceSSL: unable to validate private key(s):\n" + sslErrors();
                        throw ex;
                    }
                }
            }

            //
            // Diffie Hellman configuration.
            //
            {
#ifndef OPENSSL_NO_DH
                _dhParams = new DHParams;
                SSL_CTX_set_options(_ctx, SSL_OP_SINGLE_DH_USE);
                SSL_CTX_set_tmp_dh_callback(_ctx, IceSSL_opensslDHCallback);
#endif
                //
                // Properties have the following form:
                //
                // ...DH.<keyLength>=file
                //
                const string dhPrefix = propPrefix + "DH.";
                PropertyDict d = properties->getPropertiesForPrefix(dhPrefix);
                if(!d.empty())
                {
#ifdef OPENSSL_NO_DH
                    _logger->warning("IceSSL: OpenSSL is not configured for Diffie Hellman");
#else
                    for(PropertyDict::iterator p = d.begin(); p != d.end(); ++p)
                    {
                        string s = p->first.substr(dhPrefix.size());
                        int keyLength = atoi(s.c_str());
                        if(keyLength > 0)
                        {
                            string file = p->second;
                            if(!checkPath(file, defaultDir, false))
                            {
                                PluginInitializationException ex(__FILE__, __LINE__);
                                ex.reason = "IceSSL: DH parameter file not found:\n" + file;
                                throw ex;
                            }
                            if(!_dhParams->add(keyLength, file))
                            {
                                PluginInitializationException ex(__FILE__, __LINE__);
                                ex.reason = "IceSSL: unable to read DH parameter file " + file;
                                throw ex;
                            }
                        }
                    }
#endif
                }
            }
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
        // pointer to this Instance object.
        //
        SSL_CTX_set_session_id_context(_ctx, reinterpret_cast<unsigned char*>(this),
                                       static_cast<unsigned int>(sizeof(this)));

        //
        // Select protocols.
        //
        if(protocols != 0)
        {
            setOptions(protocols);
        }

        //
        // Establish the cipher list.
        //
        string ciphers = properties->getProperty(propPrefix + "Ciphers");
        if(!ciphers.empty())
        {
            if(!SSL_CTX_set_cipher_list(_ctx, ciphers.c_str()))
            {
                PluginInitializationException ex(__FILE__, __LINE__);
                ex.reason = "IceSSL: unable to set ciphers using `" + ciphers + "':\n" + sslErrors();
                throw ex;
            }
        }

        //
        // Determine whether a certificate is required from the peer.
        //
        {
            int sslVerifyMode;
            switch(_verifyPeer)
            {
            case 0:
                sslVerifyMode = SSL_VERIFY_NONE;
                break;
            case 1:
                sslVerifyMode = SSL_VERIFY_PEER;
                break;
            case 2:
                sslVerifyMode = SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
                break;
            default:
            {
                PluginInitializationException ex(__FILE__, __LINE__);
                ex.reason = "IceSSL: invalid value for " + propPrefix + "VerifyPeer";
                throw ex;
            }
            }
            SSL_CTX_set_verify(_ctx, sslVerifyMode, IceSSL_opensslVerifyCallback);
        }
    }
    catch(...)
    {
        //
        // We free the SSL context regardless of whether the plugin created it
        // or the application supplied it.
        //
        SSL_CTX_free(_ctx);
        _ctx = 0;
        throw;
    }

    _initialized = true;
}

void
IceSSL::Instance::context(SSL_CTX* context)
{
    if(_initialized)
    {
        PluginInitializationException ex(__FILE__, __LINE__);
        ex.reason = "IceSSL: plug-in is already initialized";
        throw ex;
    }

    assert(!_ctx);
    _ctx = context;
}

SSL_CTX*
IceSSL::Instance::context() const
{
    return _ctx;
}

void
IceSSL::Instance::setCertificateVerifier(const CertificateVerifierPtr& verifier)
{
    _verifier = verifier;
}

void
IceSSL::Instance::setPasswordPrompt(const PasswordPromptPtr& prompt)
{
    _prompt = prompt;
}

CommunicatorPtr
IceSSL::Instance::communicator() const
{
    return _facade->getCommunicator();
}

IceInternal::EndpointHostResolverPtr
IceSSL::Instance::endpointHostResolver() const
{
    return _facade->getEndpointHostResolver();
}

IceInternal::ProtocolSupport
IceSSL::Instance::protocolSupport() const
{
    return _facade->getProtocolSupport();
}

bool
IceSSL::Instance::preferIPv6() const
{
    return _facade->preferIPv6();
}

IceInternal::NetworkProxyPtr
IceSSL::Instance::networkProxy() const
{
    return _facade->getNetworkProxy();
}

string
IceSSL::Instance::defaultHost() const
{
    return _facade->getDefaultHost();
}

Ice::EncodingVersion
IceSSL::Instance::defaultEncoding() const
{
    return _facade->getDefaultEncoding();
}

int
IceSSL::Instance::networkTraceLevel() const
{
    return _facade->getNetworkTraceLevel();
}

string
IceSSL::Instance::networkTraceCategory() const
{
    return _facade->getNetworkTraceCategory();
}

int
IceSSL::Instance::securityTraceLevel() const
{
    return _securityTraceLevel;
}

string
IceSSL::Instance::securityTraceCategory() const
{
    return _securityTraceCategory;
}

void
IceSSL::Instance::verifyPeer(SSL* ssl, SOCKET fd, const string& address, const NativeConnectionInfoPtr& info)
{
    long result = SSL_get_verify_result(ssl);
    if(result != X509_V_OK)
    {
        if(_verifyPeer == 0)
        {
            if(_securityTraceLevel >= 1)
            {
                ostringstream ostr;
                ostr << "IceSSL: ignoring certificate verification failure:\n" << X509_verify_cert_error_string(result);
                _logger->trace(_securityTraceCategory, ostr.str());
            }
        }
        else
        {
            ostringstream ostr;
            ostr << "IceSSL: certificate verification failed:\n" << X509_verify_cert_error_string(result);
            string msg = ostr.str();
            if(_securityTraceLevel >= 1)
            {
                _logger->trace(_securityTraceCategory, msg);
            }
            SecurityException ex(__FILE__, __LINE__);
            ex.reason = msg;
            throw ex;
        }
    }

    X509* rawCert = SSL_get_peer_certificate(ssl);
    CertificatePtr cert;
    if(rawCert != 0)
    {
        cert = new Certificate(rawCert);
    }

    //
    // For an outgoing connection, we compare the proxy address (if any) against
    // fields in the server's certificate (if any).
    //
    if(cert && !address.empty())
    {
        //
        // Extract the IP addresses and the DNS names from the subject
        // alternative names.
        //
        vector<pair<int, string> > subjectAltNames = cert->getSubjectAlternativeNames();
        vector<string> ipAddresses;
        vector<string> dnsNames;
        for(vector<pair<int, string> >::const_iterator p = subjectAltNames.begin(); p != subjectAltNames.end(); ++p)
        {
            if(p->first == 7)
            {
                ipAddresses.push_back(IceUtilInternal::toLower(p->second));
            }
            else if(p->first == 2)
            {
                dnsNames.push_back(IceUtilInternal::toLower(p->second));
            }
        }

        //
        // Compare the peer's address against the common name.
        //
        bool certNameOK = false;
        string dn;
        string addrLower = IceUtilInternal::toLower(address);
        {
            DistinguishedName d = cert->getSubjectDN();
            dn = IceUtilInternal::toLower(string(d));
            string cn = "cn=" + addrLower;
            string::size_type pos = dn.find(cn);
            if(pos != string::npos)
            {
                //
                // Ensure we match the entire common name.
                //
                certNameOK = (pos + cn.size() == dn.size()) || (dn[pos + cn.size()] == ',');
            }
        }

        //
        // Compare the peer's address against the dnsName and ipAddress
        // values in the subject alternative name.
        //
        if(!certNameOK)
        {
            certNameOK = find(ipAddresses.begin(), ipAddresses.end(), addrLower) != ipAddresses.end();
        }
        if(!certNameOK)
        {
            certNameOK = find(dnsNames.begin(), dnsNames.end(), addrLower) != dnsNames.end();
        }

        //
        // Log a message if the name comparison fails. If CheckCertName is defined,
        // we also raise an exception to abort the connection. Don't log a message if
        // CheckCertName is not defined and a verifier is present.
        //
        if(!certNameOK && (_checkCertName || (_securityTraceLevel >= 1 && !_verifier)))
        {
            ostringstream ostr;
            ostr << "IceSSL: ";
            if(!_checkCertName)
            {
                ostr << "ignoring ";
            }
            ostr << "certificate validation failure:\npeer certificate does not have `" << address
                 << "' as its commonName or in its subjectAltName extension";
            if(!dn.empty())
            {
                ostr << "\nSubject DN: " << dn;
            }
            if(!dnsNames.empty())
            {
                ostr << "\nDNS names found in certificate: ";
                for(vector<string>::const_iterator p = dnsNames.begin(); p != dnsNames.end(); ++p)
                {
                    if(p != dnsNames.begin())
                    {
                        ostr << ", ";
                    }
                    ostr << *p;
                }
            }
            if(!ipAddresses.empty())
            {
                ostr << "\nIP addresses found in certificate: ";
                for(vector<string>::const_iterator p = ipAddresses.begin(); p != ipAddresses.end(); ++p)
                {
                    if(p != ipAddresses.begin())
                    {
                        ostr << ", ";
                    }
                    ostr << *p;
                }
            }
            string msg = ostr.str();
            if(_securityTraceLevel >= 1)
            {
                Trace out(_logger, _securityTraceCategory);
                out << msg;
            }
            if(_checkCertName)
            {
                SecurityException ex(__FILE__, __LINE__);
                ex.reason = msg;
                throw ex;
            }
        }
    }

    if(_verifyDepthMax > 0 && static_cast<int>(info->certs.size()) > _verifyDepthMax)
    {
        ostringstream ostr;
        ostr << (info->incoming ? "incoming" : "outgoing") << " connection rejected:\n"
             << "length of peer's certificate chain (" << info->certs.size() << ") exceeds maximum of "
             << _verifyDepthMax;
        string msg = ostr.str();
        if(_securityTraceLevel >= 1)
        {
            _logger->trace(_securityTraceCategory, msg + "\n" + IceInternal::fdToString(fd));
        }
        SecurityException ex(__FILE__, __LINE__);
        ex.reason = msg;
        throw ex;
    }

    if(!_trustManager->verify(info))
    {
        string msg = string(info->incoming ? "incoming" : "outgoing") + " connection rejected by trust manager";
        if(_securityTraceLevel >= 1)
        {
            _logger->trace(_securityTraceCategory, msg + "\n" + IceInternal::fdToString(fd));
        }
        SecurityException ex(__FILE__, __LINE__);
        ex.reason = msg;
        throw ex;
    }

    if(_verifier && !_verifier->verify(info))
    {
        string msg = string(info->incoming ? "incoming" : "outgoing") + " connection rejected by certificate verifier";
        if(_securityTraceLevel >= 1)
        {
            _logger->trace(_securityTraceCategory, msg + "\n" + IceInternal::fdToString(fd));
        }
        SecurityException ex(__FILE__, __LINE__);
        ex.reason = msg;
        throw ex;
    }
}

string
IceSSL::Instance::sslErrors() const
{
    return getSslErrors(_securityTraceLevel >= 1);
}

void
IceSSL::Instance::destroy()
{
    _facade = 0;

    if(_ctx)
    {
        SSL_CTX_free(_ctx);
    }
}

string
IceSSL::Instance::password(bool /*encrypting*/)
{
    if(_prompt)
    {
        try
        {
            return _prompt->getPassword();
        }
        catch(...)
        {
            //
            // Don't allow exceptions to cross an OpenSSL boundary.
            //
            return string();
        }
    }
    else
    {
        return _password;
    }
}

int
IceSSL::Instance::verifyCallback(int ok, SSL* ssl, X509_STORE_CTX* c)
{
    if(!ok && _securityTraceLevel >= 1)
    {
        X509* cert = X509_STORE_CTX_get_current_cert(c);
        int err = X509_STORE_CTX_get_error(c);
        char buf[256];

        Trace out(_logger, _securityTraceCategory);
        out << "certificate verification failure\n";

        X509_NAME_oneline(X509_get_issuer_name(cert), buf, static_cast<int>(sizeof(buf)));
        out << "issuer = " << buf << '\n';
        X509_NAME_oneline(X509_get_subject_name(cert), buf, static_cast<int>(sizeof(buf)));
        out << "subject = " << buf << '\n';
        out << "depth = " << X509_STORE_CTX_get_error_depth(c) << '\n';
        out << "error = " << X509_verify_cert_error_string(err) << '\n';
        out << IceInternal::fdToString(SSL_get_fd(ssl));
    }
    return ok;
}

#ifndef OPENSSL_NO_DH
DH*
IceSSL::Instance::dhParams(int keyLength)
{
    return _dhParams->get(keyLength);
}
#endif

void
IceSSL::Instance::traceConnection(SSL* ssl, bool incoming)
{
    Trace out(_logger, _securityTraceCategory);
    out << "SSL summary for " << (incoming ? "incoming" : "outgoing") << " connection\n";

    //
    // The const_cast is necesary because Solaris still uses OpenSSL 0.9.7.
    //
    //const SSL_CIPHER *cipher = SSL_get_current_cipher(ssl);
    SSL_CIPHER *cipher = const_cast<SSL_CIPHER*>(SSL_get_current_cipher(ssl));
    if(!cipher)
    {
        out << "unknown cipher\n";
    }
    else
    {
        out << "cipher = " << SSL_CIPHER_get_name(cipher) << "\n";
        out << "bits = " << SSL_CIPHER_get_bits(cipher, 0) << "\n";
        out << "protocol = " << SSL_get_version(ssl) << "\n";
    }
    out << IceInternal::fdToString(SSL_get_fd(ssl));
}

int
IceSSL::Instance::parseProtocols(const StringSeq& protocols)
{
    int v = 0;

    for(Ice::StringSeq::const_iterator p = protocols.begin(); p != protocols.end(); ++p)
    {
        string prot = *p;

        if(prot == "ssl3" || prot == "sslv3")
        {
            v |= SSLv3;
        }
        else if(prot == "tls" || prot == "tls1" || prot == "tlsv1" || prot == "tls1_0" || prot == "tlsv1_0")
        {
            v |= TLSv1_0;
        }
        else if(prot == "tls1_1" || prot == "tlsv1_1")
        {
            v |= TLSv1_1;
        }
        else if(prot == "tls1_2" || prot == "tlsv1_2")
        {
            v |= TLSv1_2;
        }
        else
        {
            PluginInitializationException ex(__FILE__, __LINE__);
            ex.reason = "IceSSL: unrecognized protocol `" + prot + "'";
            throw ex;
        }
    }

    return v;
}

SSL_METHOD*
IceSSL::Instance::getMethod(int /*protocols*/)
{
    //
    // Despite its name, the SSLv23 method can negotiate SSL3, TLS1.0, TLS1.1, and TLS1.2.
    // We use the const_cast for backward compatibility with older OpenSSL releases.
    //
    SSL_METHOD* meth = const_cast<SSL_METHOD*>(SSLv23_method());

    /*
     * Early versions of OpenSSL 1.0.1 would not negotiate a TLS1.2 connection using
     * the SSLv23 method. You can enable the code below to override the method.
    if(protocols & TLSv1_2)
    {
        meth = const_cast<SSL_METHOD*>(TLSv1_2_method());
    }
    */

    return meth;
}

void
IceSSL::Instance::setOptions(int protocols)
{
    long opts = SSL_OP_NO_SSLv2; // SSLv2 is not supported.
    if(!(protocols & SSLv3))
    {
        opts |= SSL_OP_NO_SSLv3;
    }
    if(!(protocols & TLSv1_0))
    {
        opts |= SSL_OP_NO_TLSv1;
    }
#ifdef SSL_OP_NO_TLSv1_1
    if(!(protocols & TLSv1_1))
    {
        opts |= SSL_OP_NO_TLSv1_1;
        //
        // The value of SSL_OP_NO_TLSv1_1 changed between 1.0.1a and 1.0.1b.
        //
        if(SSL_OP_NO_TLSv1_1 == 0x00000400L)
        {
            opts |= 0x10000000L; // New value of SSL_OP_NO_TLSv1_1.
        }
    }
#endif
#ifdef SSL_OP_NO_TLSv1_2
    if(!(protocols & TLSv1_2))
    {
        opts |= SSL_OP_NO_TLSv1_2;
    }
#endif
    SSL_CTX_set_options(_ctx, opts);
}
