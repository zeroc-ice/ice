// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Note: This pragma is used to disable spurious warning messages having
//       to do with the length of debug symbols exceeding 255 characters.
//       This is due to STL template identifiers expansion.
//       The MSDN Library recommends that you put this pragma directive
//       in place to avoid the warnings.
#ifdef WIN32
#   pragma warning(disable:4786)
#endif

//
// This needs to be first since <openssl/e_os.h> #include <windows.h>
// without our configuration settings.
//
#include <IceUtil/Config.h>

#include <sstream>
#include <IceUtil/Mutex.h>
#include <openssl/err.h>
#include <openssl/e_os.h>
#include <openssl/rand.h>
#include <Ice/OpenSSL.h>
#include <Ice/SslSystem.h>
#include <Ice/SecurityException.h>
#include <Ice/SslConnectionOpenSSLClient.h>
#include <Ice/SslConnectionOpenSSLServer.h>
#include <Ice/SslConfig.h>
#include <Ice/SslRSAKeyPair.h>
#include <Ice/SslRSAPublicKey.h>
#include <Ice/SslJanitors.h>
#include <Ice/SslCertificateVerifierOpenSSL.h>

#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>

using namespace std;
using IceInternal::TraceLevelsPtr;
using Ice::LoggerPtr;

namespace IceSecurity
{

namespace Ssl
{

namespace OpenSSL
{

//
// TODO: These Diffie-Hellman params have been blatantly stolen from
//       OpenSSL's demo programs.  We SHOULD define our own here, but
//       these will suffice for testing purposes.  Please note, these
//       are not keys themselves, simply a DH Group that allows OpenSSL
//       to create Diffie-Hellman keys.
//

// Instantiation of temporary Diffie-Hellman 512bit key.
unsigned char System::_tempDiffieHellman512p[] =
{
    0xDA,0x58,0x3C,0x16,0xD9,0x85,0x22,0x89,0xD0,0xE4,0xAF,0x75,
    0x6F,0x4C,0xCA,0x92,0xDD,0x4B,0xE5,0x33,0xB8,0x04,0xFB,0x0F,
    0xED,0x94,0xEF,0x9C,0x8A,0x44,0x03,0xED,0x57,0x46,0x50,0xD3,
    0x69,0x99,0xDB,0x29,0xD7,0x76,0x27,0x6B,0xA2,0xD3,0xD4,0x12,
    0xE2,0x18,0xF4,0xDD,0x1E,0x08,0x4C,0xF6,0xD8,0x00,0x3E,0x7C,
    0x47,0x74,0xE8,0x33,
};

unsigned char System::_tempDiffieHellman512g[] =
{
    0x02,
};

// TODO: Very possibly a problem later if we have mutliple loggers going on simultaneously.
// This is a horrible necessity in order to make the trace levels
// and logger available to the bio_dump_cb() callback function.
// Otherwise, we would have to jump through hoops, creating a mapping
// from BIO pointers to the relevent System object.  The system object
// will initialize these.  NOTE: If we SHOULD have multiple loggers
// going on simultaneously, this will definitely cause a problem.
TraceLevelsPtr System::_globalTraceLevels = 0;
LoggerPtr System::_globalLogger = 0;

}

}

}

using IceSecurity::Ssl::OpenSSL::ContextException;
using IceSecurity::Ssl::SystemPtr;

extern "C"
{

RSA*
tmpRSACallback(SSL *s, int isExport, int keyLength)
{
    IceSecurity::Ssl::SystemPtr sslSystem = IceSecurity::Ssl::Factory::getSystemFromHandle(s);

    IceSecurity::Ssl::OpenSSL::System* openSslSystem = 0;
    openSslSystem = dynamic_cast<IceSecurity::Ssl::OpenSSL::System*>(sslSystem.get());

    RSA* rsaKey = openSslSystem->getRSAKey(s, isExport, keyLength);

    return rsaKey;
}

DH*
tmpDHCallback(SSL *s, int isExport, int keyLength)
{
    IceSecurity::Ssl::SystemPtr sslSystem = IceSecurity::Ssl::Factory::getSystemFromHandle(s);

    IceSecurity::Ssl::OpenSSL::System* openSslSystem = 0;
    openSslSystem = dynamic_cast<IceSecurity::Ssl::OpenSSL::System*>(sslSystem.get());

    DH* dh = openSslSystem->getDHParams(s, isExport, keyLength);

    return dh;
}

// verifyCallback - Certificate Verification callback function.
int
verifyCallback(int ok, X509_STORE_CTX *ctx)
{
    // Tricky method to get access to our connection.  I would use SSL_get_ex_data() to get
    // the Connection object, if only I had some way to retrieve the index of the object
    // in this function.  Hence, we have to invent our own reference system here.
    SSL* ssl = static_cast<SSL*>(X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
    IceSecurity::Ssl::OpenSSL::ConnectionPtr connection = 0;
    connection = IceSecurity::Ssl::OpenSSL::Connection::getConnection(ssl);
    assert(connection);

    // Call the connection, get it to perform the verification.
    return connection->verifyCertificate(ok, ctx);
}

// TODO: This is a complete hack to get this working again with the CA certificate.
//       Of course, this will have to be rewritten to handle this in the same manner
//       as the verifyCallback does.
//       -ASN
int
passwordCallback(char* buffer, int bufferSize, int rwFlag, void* userData)
{
    strncpy(buffer, "demo", bufferSize);
    buffer[bufferSize - 1] = '\0';
    return strlen(buffer);
}

// This code duplicates functionality that existed in the BIO library of
// OpenSSL, but outputs to a Logger compatible source (ostringstream).
void
dump(ostringstream& outStringStream, const char* s, int len)
{
    unsigned char ch;
    char hexStr[8];

    int trunc = 0;
	
    // Calculate how much white space we're truncating.
    for(; (len > 0) && ((s[len - 1] == ' ') || (s[len - 1] == '\0')); len--) 
    {
        trunc++;
    }

    int dump_width = 12;

    int rows = len / dump_width;

    if ((rows * dump_width) < len)
    {
	rows++;
    }

    if (rows > 0)
    {
        outStringStream << endl;
    }

    for(int i = 0; i < rows; i++)
    {
        // Would like to have not used sprintf(), but
        // I could not find an appropriate STL methodology
        // for preserving the field width.
        sprintf(hexStr,"%04x",(i * dump_width));
        outStringStream << hexStr << " - ";

        int j;

        // Hex Dump
        for(j = 0; j < dump_width; j++)
	{
	    if (((i * dump_width) + j) >= len)
	    {
                outStringStream << "   ";
	    }
            else
	    {
                char sep = (j == 7 ? '-' : ' ');

                // Get a character from the dump we've been handed.
                ch = ((unsigned char)*(s + i * dump_width + j)) & 0xff;

                // Would like to have not used sprintf(), but
                // I could not find an appropriate STL methodology
                // for preserving the field width.
                sprintf(hexStr,"%02x",ch);
                outStringStream << hexStr << sep;
	    }
	}

        outStringStream << "  ";

        // Printable characters dump.
        for(j = 0; j < dump_width; j++)
	{
	    if (((i * dump_width) + j) >= len)
            {
		break;
            }

            ch = ((unsigned char) * (s + i * dump_width + j)) & 0xff;

            // Print printables only.
            ch = ((ch >= ' ') && (ch <= '~')) ? ch : '.';

            outStringStream << ch;
	}

        outStringStream << endl;
    }

    if (trunc > 0)
    {
        outStringStream << hex << (len + trunc) << " - " << "<SPACES/NULS>" << endl;
    }
}

long
bio_dump_cb(BIO *bio, int cmd, const char *argp, int argi, long argl, long ret)
{
    if (IceSecurity::Ssl::OpenSSL::System::_globalTraceLevels->security >= IceSecurity::SECURITY_PROTOCOL)
    {
        ostringstream outStringStream;

        outStringStream << "PTC ";

        if (cmd == (BIO_CB_READ|BIO_CB_RETURN))
        {
            outStringStream << "read from " << hex << (void *)bio << " [" << hex << (void *)argp;
            outStringStream << "] (" << dec << argi << " bytes => " << ret << " (0x";
            outStringStream << hex << ret << "))";
//            dump(outStringStream, argp,(int)ret);
        }
        else if (cmd == (BIO_CB_WRITE|BIO_CB_RETURN))
        {
            outStringStream << "write to " << hex << (void *)bio << " [" << hex << (void *)argp;
            outStringStream << "] (" << dec << argi << " bytes => " << ret << " (0x";
            outStringStream << hex << ret << "))";
 //           dump(outStringStream, argp,(int)ret);
        }

        if (cmd == (BIO_CB_READ|BIO_CB_RETURN) || cmd == (BIO_CB_WRITE|BIO_CB_RETURN))
        {
            IceSecurity::Ssl::OpenSSL::System::_globalLogger->trace(
		IceSecurity::Ssl::OpenSSL::System::_globalTraceLevels->securityCat, outStringStream.str());
        }
    }

    return ret;
}

}

IceSecurity::Ssl::Connection*
IceSecurity::Ssl::OpenSSL::System::createServerConnection(int socket)
{
    if (_sslServerContext == 0)
    {
        ContextException contextEx(__FILE__, __LINE__);

        contextEx._message = "Server context has not been set up - ";
        contextEx._message += "please specify an SSL server configuration file.";

	throw contextEx;
    }

    SSL* sslConnection = createConnection(_sslServerContext, socket);

    // Set the Accept Connection state for this connection.
    SSL_set_accept_state(sslConnection);

    Connection* connection = new ServerConnection(_serverVerifier, sslConnection, SystemPtr(this));

    commonConnectionSetup(connection);

    return connection;
}

IceSecurity::Ssl::Connection*
IceSecurity::Ssl::OpenSSL::System::createClientConnection(int socket)
{
    if (_sslClientContext == 0)
    {
        ContextException contextEx(__FILE__, __LINE__);

        contextEx._message = "Client context has not been set up - ";
        contextEx._message += "please specify an SSL client configuration file.";

	throw contextEx;
    }

    SSL* sslConnection = createConnection(_sslClientContext, socket);

    // Set the Connect Connection state for this connection.
    SSL_set_connect_state(sslConnection);

    Connection* connection = new ClientConnection(_clientVerifier, sslConnection, SystemPtr(this));

    commonConnectionSetup(connection);

    return connection;
}

void
IceSecurity::Ssl::OpenSSL::System::shutdown()
{
    if (_sslServerContext != 0)
    {
        SSL_CTX_free(_sslServerContext);

        _sslServerContext = 0;
    }

    if (_sslClientContext != 0)
    {
        SSL_CTX_free(_sslClientContext);

        _sslClientContext = 0;
    }

    // Free our temporary RSA keys.
    RSAMap::iterator iRSA = _tempRSAKeys.begin();
    RSAMap::iterator eRSA = _tempRSAKeys.end();

    while (iRSA != eRSA)
    {
        RSA_free((*iRSA).second);
        iRSA++;
    }

    // Free our temporary DH params.
    DHMap::iterator iDH = _tempDHKeys.begin();
    DHMap::iterator eDH = _tempDHKeys.end();

    while (iDH != eDH)
    {
        DH_free((*iDH).second);
        iDH++;
    }
}

void
IceSecurity::Ssl::OpenSSL::System::setTrace(const TraceLevelsPtr& traceLevels)
{
    // Note: Due to a known bug with VC++, I cannot simply call the base-class 
    // implementation here, I get a C2352 error about calling a static function.
    // Bug# Q153801
    _traceLevels = traceLevels;

    DefaultCertificateVerifier* clientVerifier = dynamic_cast<DefaultCertificateVerifier*>(_clientVerifier.get());
    DefaultCertificateVerifier* serverVerifier = dynamic_cast<DefaultCertificateVerifier*>(_serverVerifier.get());

    if (clientVerifier)
    {
        clientVerifier->setTraceLevels(traceLevels);
    }

    if (serverVerifier)
    {
        serverVerifier->setTraceLevels(traceLevels);
    }
}

void
IceSecurity::Ssl::OpenSSL::System::setLogger(const LoggerPtr& logger)
{
    // Note: Due to a known bug with VC++, I cannot simply call the base-class 
    // implementation here, I get a C2352 error about calling a static function.
    // Bug# Q153801
    _logger = logger;

    DefaultCertificateVerifier* clientVerifier = dynamic_cast<DefaultCertificateVerifier*>(_clientVerifier.get());
    DefaultCertificateVerifier* serverVerifier = dynamic_cast<DefaultCertificateVerifier*>(_serverVerifier.get());

    if (clientVerifier)
    {
        clientVerifier->setLogger(logger);
    }

    if (serverVerifier)
    {
        serverVerifier->setLogger(logger);
    }
}

bool
IceSecurity::Ssl::OpenSSL::System::isConfigLoaded()
{
    return _configLoaded;
}

void
IceSecurity::Ssl::OpenSSL::System::loadConfig()
{
    // This step is required in order to supply callback functions
    // with access to the TraceLevels and Logger.
    if (_globalTraceLevels == 0)
    {
        _globalTraceLevels = _traceLevels;
        _globalLogger = _logger;
    }

    string configFile = _properties->getProperty("Ice.Security.Ssl.Config");
    string certificatePath = _properties->getProperty("Ice.Security.Ssl.CertPath");
    Parser sslConfig(configFile, certificatePath);

    sslConfig.setTrace(_traceLevels);
    sslConfig.setLogger(_logger);

    // Actually parse the file now.
    sslConfig.process();

    GeneralConfig clientGeneral;
    CertificateAuthority clientCertAuth;
    BaseCertificates clientBaseCerts;

    // Walk the parse tree, get the Client configuration.
    if (sslConfig.loadClientConfig(clientGeneral, clientCertAuth, clientBaseCerts))
    {
        if (_traceLevels->security >= IceSecurity::SECURITY_PROTOCOL)
        {
            ostringstream s;

            s << endl;
            s << "General Configuration - Client" << endl;
            s << "------------------------------" << endl;
            s << clientGeneral << endl << endl;

            s << "CA File: " << clientCertAuth.getCAFileName() << endl;
            s << "CA Path: " << clientCertAuth.getCAPath() << endl;

            s << "Base Certificates - Client" << endl;
            s << "--------------------------" << endl;
            s << clientBaseCerts << endl;

            _logger->trace(_traceLevels->securityCat, s.str());
        }

        initClient(clientGeneral, clientCertAuth, clientBaseCerts);
    }

    GeneralConfig serverGeneral;
    CertificateAuthority serverCertAuth;
    BaseCertificates serverBaseCerts;
    TempCertificates serverTempCerts;

    // Walk the parse tree, get the Server configuration.
    if (sslConfig.loadServerConfig(serverGeneral, serverCertAuth, serverBaseCerts, serverTempCerts))
    {
        if (_traceLevels->security >= IceSecurity::SECURITY_PROTOCOL)
        {
            ostringstream s;

            s << endl;
            s << "General Configuration - Server" << endl;
            s << "------------------------------" << endl;
            s << serverGeneral   << endl << endl;

            s << "CA File: " << serverCertAuth.getCAFileName() << endl;
            s << "CA Path: " << serverCertAuth.getCAPath() << endl;

            s << "Base Certificates - Server" << endl;
            s << "--------------------------" << endl;
            s << serverBaseCerts << endl << endl;

            s << "Temp Certificates - Server" << endl;
            s << "--------------------------" << endl;
            s << serverTempCerts << endl;

            _logger->trace(_traceLevels->securityCat, s.str());
        }

        initServer(serverGeneral, serverCertAuth, serverBaseCerts, serverTempCerts);
    }

    _configLoaded = true;
}

RSA*
IceSecurity::Ssl::OpenSSL::System::getRSAKey(SSL *s, int isExport, int keyLength)
{
    IceUtil::Mutex::Lock sync(_tempRSAKeysMutex);

    RSA* rsa_tmp = 0;

    RSAMap::iterator retVal = _tempRSAKeys.find(keyLength);

    // Does the key already exist?
    if (retVal != _tempRSAKeys.end())
    {
        // Yes!  Use it.
        rsa_tmp = (*retVal).second;
    }
    else
    {
        const RSACertMap::iterator& it = _tempRSAFileMap.find(keyLength);

        if (it != _tempRSAFileMap.end())
        {
            CertificateDesc& rsaKeyCert = (*it).second;

            const string& privKeyFile = rsaKeyCert.getPrivate().getFileName();
            const string& pubCertFile = rsaKeyCert.getPublic().getFileName();

            RSA* rsaCert = 0;
            RSA* rsaKey = 0;
            BIO* bio = 0;

            if ((bio = BIO_new_file(pubCertFile.c_str(), "r")) != 0)
            {
                rsaCert = PEM_read_bio_RSAPublicKey(bio, 0, 0, 0);

                BIO_free(bio);
                bio = 0;
            }

            if (rsaCert != 0)
            {
                if ((bio = BIO_new_file(privKeyFile.c_str(), "r")) != 0)
                {
                    rsaKey = PEM_read_bio_RSAPrivateKey(bio, &rsaCert, 0, 0);

                    BIO_free(bio);
                    bio = 0;
                }
            }

            // Now, if all was well, the Certificate and Key should both be loaded into
            // rsaCert. We check to ensure that both are not 0, because if either are,
            // one of the reads failed.

            if ((rsaCert != 0) && (rsaKey != 0))
            {
                rsa_tmp = rsaCert;
            }
            else
            {
                RSA_free(rsaCert);
                rsaCert = 0;
            }
        }

        // Last ditch effort - generate a key on the fly.
        if (rsa_tmp == 0)
        {
            rsa_tmp = RSA_generate_key(keyLength, RSA_F4, 0, 0);
        }

        // Save in our temporary key cache.
        if (rsa_tmp == 0)
        {
            _tempRSAKeys[keyLength] = rsa_tmp;
        }
    }

    return rsa_tmp;
}

DH*
IceSecurity::Ssl::OpenSSL::System::getDHParams(SSL *s, int isExport, int keyLength)
{
    IceUtil::Mutex::Lock sync(_tempDHKeysMutex);

    DH *dh_tmp = 0;

    const DHMap::iterator& retVal = _tempDHKeys.find(keyLength);

    // Does the key already exist?
    if (retVal != _tempDHKeys.end())
    {
        // Yes!  Use it.
        dh_tmp = (*retVal).second;
    }
    else
    {
        const DHParamsMap::iterator& it = _tempDHParamsFileMap.find(keyLength);

        if (it != _tempDHParamsFileMap.end())
        {
            DiffieHellmanParamsFile& dhParamsFile = (*it).second;

            string dhFile = dhParamsFile.getFileName();

            dh_tmp = loadDHParam(dhFile.c_str());

            if (dh_tmp != 0)
            {
                _tempDHKeys[keyLength] = dh_tmp;
            }
        }
    }

    return dh_tmp;
}

IceSecurity::Ssl::OpenSSL::CertificateVerifierPtr
IceSecurity::Ssl::OpenSSL::System::certificateVerifierTypeCheck(const IceSecurity::Ssl::CertificateVerifierPtr& verifier)
{
    // IceSecurity::Ssl::CertificateVerifier* passedVerifier = verifier.get();
    // IceSecurity::Ssl::OpenSSL::CertificateVerifier* castVerifier;
    // castVerifier = dynamic_cast<IceSecurity::Ssl::OpenSSL::CertificateVerifier*>(passedVerifier);

    IceSecurity::Ssl::OpenSSL::CertificateVerifierPtr castVerifier;
    castVerifier = IceSecurity::Ssl::OpenSSL::CertificateVerifierPtr::dynamicCast(verifier);

    if (!castVerifier.get())
    {
        IceSecurity::Ssl::CertificateVerifierTypeException cvtEx(__FILE__, __LINE__);
        throw cvtEx;
    }

    return castVerifier;
}

void
IceSecurity::Ssl::OpenSSL::System::setServerCertificateVerifier(const IceSecurity::Ssl::CertificateVerifierPtr& serverVerifier)
{
    _serverVerifier = certificateVerifierTypeCheck(serverVerifier);
}

void
IceSecurity::Ssl::OpenSSL::System::setClientCertificateVerifier(const IceSecurity::Ssl::CertificateVerifierPtr& clientVerifier)
{
    _clientVerifier = certificateVerifierTypeCheck(clientVerifier);
}

void
IceSecurity::Ssl::OpenSSL::System::setServerCertAuthorityCertificate(const string& caCertString)
{
    if (_sslServerContext == 0)
    {
        ContextException contextEx(__FILE__, __LINE__);

        contextEx._message = "Server context has not been set up - ";
        contextEx._message += "please specify an SSL server configuration file.";

	throw contextEx;
    }

    assert(_sslClientContext);

    RSAPublicKey pubKey(caCertString);

    X509_STORE* certStore = SSL_CTX_get_cert_store(_sslServerContext);

    int addedCertAuthorityCert = X509_STORE_add_cert(certStore, pubKey.getX509PublicKey());

    assert(addedCertAuthorityCert != 0);
}

void
IceSecurity::Ssl::OpenSSL::System::setClientCertAuthorityCertificate(const string& caCertString)
{
    if (_sslClientContext == 0)
    {
        ContextException contextEx(__FILE__, __LINE__);

        contextEx._message = "Client context has not been set up - ";
        contextEx._message += "please specify an SSL client configuration file.";

	throw contextEx;
    }

    assert(_sslClientContext);

    RSAPublicKey pubKey(caCertString);

    X509_STORE* certStore = SSL_CTX_get_cert_store(_sslClientContext);

    int addedCertAuthorityCert = X509_STORE_add_cert(certStore, pubKey.getX509PublicKey());

    assert(addedCertAuthorityCert != 0);
}

void
IceSecurity::Ssl::OpenSSL::System::setServerRSAKeysBase64(const std::string& privateKey, const std::string& publicKey)
{
    assert(_sslServerContext);
    addKeyCert(_sslServerContext, privateKey, publicKey);
}

void
IceSecurity::Ssl::OpenSSL::System::setClientRSAKeysBase64(const std::string& privateKey, const std::string& publicKey)
{
    assert(_sslClientContext);
    addKeyCert(_sslClientContext, privateKey, publicKey);
}

//
// Protected
//

IceSecurity::Ssl::OpenSSL::System::System()
{
    _configLoaded = false;

    _sessionContext = "iceServer";

    _randSeeded = 0;
    _sslServerContext = 0;
    _sslClientContext = 0;

    // Here we create a default verifier, which does very little other
    // than check the verification depth.  This can be overridden.
    _clientVerifier = new DefaultCertificateVerifier();
    _serverVerifier = _clientVerifier;

    SSL_load_error_strings();

    OpenSSL_add_ssl_algorithms();
}

IceSecurity::Ssl::OpenSSL::System::~System()
{
    shutdown();
}

//
// Private
//

void
IceSecurity::Ssl::OpenSSL::System::setKeyCert(SSL_CTX* context,
                                              const CertificateDesc& certDesc,
                                              const string& privateKey,
                                              const string& publicKey)
{
    if (!privateKey.empty() && !publicKey.empty())
    {
        addKeyCert(context, privateKey, publicKey);
    }
    else if (certDesc.getKeySize() != 0)
    {
        processCertificate(context, certDesc);
    }
}

void
IceSecurity::Ssl::OpenSSL::System::initClient(GeneralConfig& general,
                                              CertificateAuthority& certAuth,
                                              BaseCertificates& baseCerts)
{
    // Init the Random Number System.
    initRandSystem(general.getRandomBytesFiles());

    // Create an SSL Context based on the context params.
    _sslClientContext = createContext(general.getProtocol());

    // Begin setting up the SSL Context.
    if (_sslClientContext != 0)
    {
        // Get the cipherlist and set it in the context.
        setCipherList(_sslClientContext, general.getCipherList());

        // Set the certificate verification mode.
        SSL_CTX_set_verify(_sslClientContext, general.getVerifyMode(), verifyCallback);

        // Set the certificate verify depth
        SSL_CTX_set_verify_depth(_sslClientContext, general.getVerifyDepth());

        // Process the RSA Certificate
        string privateRSAKey = _properties->getProperty("Ice.Security.Ssl.Overrides.Client.RSA.PrivateKey");
        string publicRSAKey = _properties->getProperty("Ice.Security.Ssl.Overrides.Client.RSA.Certificate");
        setKeyCert(_sslClientContext, baseCerts.getRSACert(), privateRSAKey, publicRSAKey);

        // Process the DSA Certificate
        string privateDSAKey; // = _properties->getProperty("Ice.Security.Ssl.Overrides.Client.DSA.PrivateKey");
        string publicDSAKey; // = _properties->getProperty("Ice.Security.Ssl.Overrides.Client.DSA.Certificate");
        setKeyCert(_sslClientContext, baseCerts.getDSACert(), privateDSAKey, publicDSAKey);

        // Set the DH key agreement parameters.
        if (baseCerts.getDHParams().getKeySize() != 0)
        {
            setDHParams(_sslClientContext, baseCerts);
        }

        // Load the Certificate Authority files, and check them.
        loadCAFiles(_sslClientContext, certAuth);
    }
}

void
IceSecurity::Ssl::OpenSSL::System::initServer(GeneralConfig& general,
                                              CertificateAuthority& certAuth,
                                              BaseCertificates& baseCerts,
                                              TempCertificates& tempCerts)
{
    // Init the Random Number System.
    initRandSystem(general.getRandomBytesFiles());

    // Create an SSL Context based on the context params.
    _sslServerContext = createContext(general.getProtocol());

    // Begin setting up the SSL Context.
    if (_sslServerContext != 0)
    {
        // On servers, Attempt to use non-export (strong) encryption
        // first.  This option does not always work, and in the OpenSSL
        // documentation is declared as 'broken'.
        // SSL_CTX_set_options(_sslServerContext,SSL_OP_NON_EXPORT_FIRST);

        // Always use a new DH key when using Diffie-Hellman key agreement.
        SSL_CTX_set_options(_sslServerContext, SSL_OP_SINGLE_DH_USE);

        loadTempCerts(tempCerts);

        // Load the Certificate Authority files, and check them.
        loadAndCheckCAFiles(_sslServerContext, certAuth);

        // Process the RSA Certificate
        string privateRSAKey = _properties->getProperty("Ice.Security.Ssl.Overrides.Server.RSA.PrivateKey");
        string publicRSAKey = _properties->getProperty("Ice.Security.Ssl.Overrides.Server.RSA.Certificate");
        setKeyCert(_sslServerContext, baseCerts.getRSACert(), privateRSAKey, publicRSAKey);

        // Process the DSA Certificate
        string privateDSAKey; // = _properties->getProperty("Ice.Security.Ssl.Overrides.Server.DSA.PrivateKey");
        string publicDSAKey; // = _properties->getProperty("Ice.Security.Ssl.Overrides.Server.DSA.Certificate");
        setKeyCert(_sslServerContext, baseCerts.getDSACert(), privateDSAKey, publicDSAKey);

        // Set the DH key agreement parameters.
        if (baseCerts.getDHParams().getKeySize() != 0)
        {
            setDHParams(_sslServerContext, baseCerts);
        }

        // Set the RSA Callback routine in case we need to build a temporary RSA key.
        SSL_CTX_set_tmp_rsa_callback(_sslServerContext, tmpRSACallback);

        // Set the DH Callback routine in case we need a temporary DH key.
        SSL_CTX_set_tmp_dh_callback(_sslServerContext, tmpDHCallback);

        // Get the cipherlist and set it in the context.
        setCipherList(_sslServerContext, general.getCipherList());

        // Set the certificate verification mode.
        SSL_CTX_set_verify(_sslServerContext, general.getVerifyMode(), verifyCallback);

        // Set the certificate verify depth
        SSL_CTX_set_verify_depth(_sslServerContext, general.getVerifyDepth());

        // Set the default context for the SSL system (can be overridden if needed) [SERVER ONLY].
        SSL_CTX_set_session_id_context(_sslServerContext,
                                       reinterpret_cast<const unsigned char *>(_sessionContext.c_str()),
                                       _sessionContext.size());
    }
}

SSL_METHOD*
IceSecurity::Ssl::OpenSSL::System::getSslMethod(SslProtocol sslVersion)
{
    SSL_METHOD* sslMethod = 0;

    switch (sslVersion)
    {
        case SSL_V2 :
        {
            sslMethod   = SSLv2_method();
            break;
        }

        case SSL_V23 :
        {
            sslMethod   = SSLv23_method();
            break;
        }

        case SSL_V3 :
        {
            sslMethod   = SSLv3_method();
            break;
        }

        case TLS_V1 :
        {
            sslMethod   = TLSv1_method();
            break;
        }

        default :
        {
            if (_traceLevels->security >= IceSecurity::SECURITY_WARNINGS)
            { 
                string errorString;

                errorString = "SSL Version ";
                errorString += sslVersion;
                errorString += " not supported - defaulting to SSL_V23.";
                _logger->trace(_traceLevels->securityCat, "WRN " + errorString);
            }

            sslMethod   = SSLv23_method();
        }
    }

    return sslMethod;
}

void
IceSecurity::Ssl::OpenSSL::System::processCertificate(SSL_CTX* sslContext, const CertificateDesc& certificateDesc)
{
    const CertificateFile& publicCert = certificateDesc.getPublic();
    const CertificateFile& privateKey = certificateDesc.getPrivate();

    addKeyCert(sslContext, publicCert, privateKey);
}

void
IceSecurity::Ssl::OpenSSL::System::addKeyCert(SSL_CTX* sslContext,
                                              const CertificateFile& publicCert,
                                              const CertificateFile& privateKey)
{
    if (!publicCert.getFileName().empty())
    {
	string publicCertFile = publicCert.getFileName();
        const char* publicFile = publicCertFile.c_str();
        int publicEncoding = publicCert.getEncoding();

        string privCertFile = privateKey.getFileName();
        const char* privKeyFile = privCertFile.c_str();
        int privKeyFileType = privateKey.getEncoding();

        // Set which Public Key file to use.
        if (SSL_CTX_use_certificate_file(sslContext, publicFile, publicEncoding) <= 0)
        {
            ContextException contextEx(__FILE__, __LINE__);

            contextEx._message = "Unable to get certificate from '";
            contextEx._message += publicFile;
            contextEx._message += "'\n";
            contextEx._message += sslGetErrors();

            throw contextEx;
        }

        if (privateKey.getFileName().empty())
        {
            if (_traceLevels->security >= IceSecurity::SECURITY_WARNINGS)
            { 
                _logger->trace(_traceLevels->securityCat, "WRN No private key specified - using the certificate.");
            }

            privKeyFile = publicFile;
            privKeyFileType = publicEncoding;
        }

        // Set which Private Key file to use.
        if (SSL_CTX_use_PrivateKey_file(sslContext, privKeyFile, privKeyFileType) <= 0)
        {
            ContextException contextEx(__FILE__, __LINE__);

            contextEx._message = "Unable to get private key from '";
            contextEx._message += privKeyFile;
            contextEx._message += "'\n";
            contextEx._message += sslGetErrors();

	    throw contextEx;
        }

        // Check to see if the Private and Public keys that have been
        // set against the SSL context match up.
        if (!SSL_CTX_check_private_key(sslContext))
        {
            ContextException contextEx(__FILE__, __LINE__);

            contextEx._message = "Private key does not match the certificate public key.";
            string sslError = sslGetErrors();

            if (!sslError.empty())
            {
                contextEx._message += "\n";
                contextEx._message += sslError;
            }

            throw contextEx;
        }
    }
}

void
IceSecurity::Ssl::OpenSSL::System::addKeyCert(SSL_CTX* sslContext,
                                              const string& privateKey,
                                              const string& publicKey)
{
    string privKey = privateKey;

    if (privKey.empty())
    {
        if (_traceLevels->security >= IceSecurity::SECURITY_WARNINGS)
        { 
            _logger->trace(_traceLevels->securityCat, "WRN No private key specified - using the certificate.");
        }

        privKey = publicKey;
    }

    // Make a key pair based on the Base64 encoded strings
    RSAKeyPair keyPair(privateKey, publicKey);

    // Janitors to ensure that everything gets cleaned up properly
    RSAJanitor rsaJanitor(keyPair.getRSAPrivateKey());
    X509Janitor x509Janitor(keyPair.getX509PublicKey());

    // Set which Public Key file to use.
    if (SSL_CTX_use_certificate(sslContext, x509Janitor.get()) <= 0)
    {
        ContextException contextEx(__FILE__, __LINE__);

        contextEx._message = "Unable to set certificate from memory.";
        string sslError = sslGetErrors();

        if (!sslError.empty())
        {
            contextEx._message += "\n";
            contextEx._message += sslError;
        }

        throw contextEx;
    }

    x509Janitor.clear();

    // Set which Private Key file to use.
    if (SSL_CTX_use_RSAPrivateKey(sslContext, rsaJanitor.get()) <= 0)
    {
        ContextException contextEx(__FILE__, __LINE__);

        contextEx._message = "Unable to set private key from memory.";
        string sslError = sslGetErrors();

        if (!sslError.empty())
        {
            contextEx._message += "\n";
            contextEx._message += sslError;
        }

        throw contextEx;
    }

    rsaJanitor.clear();

    // Check to see if the Private and Public keys that have been
    // set against the SSL context match up.
    if (!SSL_CTX_check_private_key(sslContext))
    {
        ContextException contextEx(__FILE__, __LINE__);

        contextEx._message = "Private key does not match the certificate public key.";
        string sslError = sslGetErrors();

        if (!sslError.empty())
        {
            contextEx._message += "\n";
            contextEx._message += sslError;
        }

        throw contextEx;
    }
}


SSL_CTX*
IceSecurity::Ssl::OpenSSL::System::createContext(SslProtocol sslProtocol)
{
    SSL_CTX* context = SSL_CTX_new(getSslMethod(sslProtocol));

    if (context == 0)
    {
        ContextException contextEx(__FILE__, __LINE__);

        contextEx._message = "Unable to create SSL Context.\n" + sslGetErrors();

        throw contextEx;
    }

    // Turn off session caching, supposedly fixes a problem with multithreading.
    SSL_CTX_set_session_cache_mode(context, SSL_SESS_CACHE_OFF);

    return context;
}


string
IceSecurity::Ssl::OpenSSL::System::sslGetErrors()
{
    string errorMessage;
    char buf[200];
    char bigBuffer[1024];
    const char* file = 0;
    const char* data = 0;
    int line = 0;
    int flags = 0;
    unsigned errorCode = 0;
    int errorNum = 1;

    unsigned long es = CRYPTO_thread_id();

    while ((errorCode = ERR_get_error_line_data(&file, &line, &data, &flags)) != 0)
    {
        sprintf(bigBuffer,"%6d - Thread ID: %lu\n", errorNum, es);
        errorMessage += bigBuffer;

        sprintf(bigBuffer,"%6d - Error:     %u\n", errorNum, errorCode);
        errorMessage += bigBuffer;

        // Request an error from the OpenSSL library
        ERR_error_string_n(errorCode, buf, sizeof(buf));
        sprintf(bigBuffer,"%6d - Message:   %s\n", errorNum, buf);
        errorMessage += bigBuffer;

        sprintf(bigBuffer,"%6d - Location:  %s, %d\n", errorNum, file, line);
        errorMessage += bigBuffer;

        if (flags & ERR_TXT_STRING)
        {
            sprintf(bigBuffer,"%6d - Data:      %s\n", errorNum, data);
            errorMessage += bigBuffer;
        }

        errorNum++;
    }

    ERR_clear_error();

    return errorMessage;
}

void
IceSecurity::Ssl::OpenSSL::System::commonConnectionSetup(IceSecurity::Ssl::OpenSSL::Connection* connection)
{
    connection->setTrace(_traceLevels);
    connection->setLogger(_logger);

    // Set the Post-Hanshake Read timeout
    // This timeout is implemented once on the first read after hanshake.
    int handshakeReadTimeout;
    string value = _properties->getProperty("Ice.Security.Ssl.Handshake.ReadTimeout");

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

SSL*
IceSecurity::Ssl::OpenSSL::System::createConnection(SSL_CTX* sslContext, int socket)
{
    SSL* sslConnection = SSL_new(sslContext);

    SSL_clear(sslConnection);

    SSL_set_fd(sslConnection, socket);

    if (_traceLevels->security >= IceSecurity::SECURITY_PROTOCOL_DEBUG)
    {
        sslConnection->debug = 1;
        BIO_set_callback(SSL_get_rbio(sslConnection), bio_dump_cb);
        BIO_set_callback_arg(SSL_get_rbio(sslConnection), 0);
        BIO_set_callback(SSL_get_wbio(sslConnection), bio_dump_cb);
        BIO_set_callback_arg(SSL_get_rbio(sslConnection), 0);
    }

    return sslConnection;
}

void
IceSecurity::Ssl::OpenSSL::System::loadCAFiles(SSL_CTX* sslContext, CertificateAuthority& certAuth)
{
    assert(sslContext);

    string caFile = certAuth.getCAFileName();
    string caPath = certAuth.getCAPath();

    loadCAFiles(sslContext, caFile.c_str(), caPath.c_str());
}

void
IceSecurity::Ssl::OpenSSL::System::loadCAFiles(SSL_CTX* sslContext, const char* caFile, const char* caPath)
{
    assert(sslContext);

    // The following checks are required to send the expected values to the OpenSSL library.
    // It does not like receiving "", but prefers NULLs.
    if ((caFile != 0) && (strlen(caFile) == 0))
    {
        caFile = 0;
    }

    if ((caPath != 0) && (strlen(caPath) == 0))
    {
        caPath = 0;
    }

    // SSL_CTX_set_default_passwd_cb(sslContext, passwordCallback);

    // Check the Certificate Authority file(s).
    int loadVerifyRet = SSL_CTX_load_verify_locations(sslContext, caFile, caPath);

    if (!loadVerifyRet)
    { 
        if (_traceLevels->security >= IceSecurity::SECURITY_WARNINGS)
        {
            _logger->trace(_traceLevels->securityCat, "WRN Unable to load Certificate Authorities.");
        }
    }
    else
    {
        int setDefaultVerifyPathsRet = SSL_CTX_set_default_verify_paths(sslContext);


        if (!setDefaultVerifyPathsRet && (_traceLevels->security >= IceSecurity::SECURITY_WARNINGS))
        { 
            _logger->trace(_traceLevels->securityCat, "WRN Unable to verify Certificate Authorities.");
        }
    }
}

void
IceSecurity::Ssl::OpenSSL::System::loadAndCheckCAFiles(SSL_CTX* sslContext, CertificateAuthority& certAuth)
{
    assert(sslContext);

    string caFile = certAuth.getCAFileName();
    string caPath = certAuth.getCAPath();

    // Check the Certificate Authority file(s).
    loadCAFiles(sslContext, caFile.c_str(), caPath.c_str());

    // NOTE: This might require some cleaning up.
    string caCertBase64 = _properties->getProperty("Ice.Security.Ssl.Overrides.Server.CACertificate");
    if (!caCertBase64.empty())
    {
        setServerCertAuthorityCertificate(caCertBase64);
    }

    // TODO: Check this if things stop working
    if (!caFile.empty())
    {
        STACK_OF(X509_NAME)* certNames = SSL_load_client_CA_file(caFile.c_str());

        if (certNames == 0)
        {
            if (_traceLevels->security >= IceSecurity::SECURITY_WARNINGS)
            {
                string errorString = "Unable to load Certificate Authorities certificate names from " + caFile + ".\n";
                errorString += sslGetErrors();
                _logger->trace(_traceLevels->securityCat, "WRN " + errorString);
            }
        }
        else
        {
            SSL_CTX_set_client_CA_list(sslContext, certNames);
        }
    }
}

DH*
IceSecurity::Ssl::OpenSSL::System::loadDHParam(const char* dhfile)
{
    assert(dhfile);

    DH* ret = 0;
    BIO* bio;

    if ((bio = BIO_new_file(dhfile,"r")) != 0)
    {
        ret = PEM_read_bio_DHparams(bio, 0, 0, 0);
    }

    if (bio != 0)
    {
        BIO_free(bio);
    }

    return ret;
}

DH*
IceSecurity::Ssl::OpenSSL::System::getTempDH(unsigned char* p, int plen, unsigned char* g, int glen)
{
    DH* dh = 0;

    if ((dh = DH_new()) != 0)
    {
        dh->p = BN_bin2bn(p, plen, 0);

        dh->g = BN_bin2bn(g, glen, 0);

        if ((dh->p == 0) || (dh->g == 0))
        {
            DH_free(dh);
            dh = 0;
        }
    }

    return dh;
}

DH*
IceSecurity::Ssl::OpenSSL::System::getTempDH512()
{
    DH* dh = getTempDH(_tempDiffieHellman512p, sizeof(_tempDiffieHellman512p),
                       _tempDiffieHellman512g, sizeof(_tempDiffieHellman512g));

    return dh;
}

void
IceSecurity::Ssl::OpenSSL::System::setDHParams(SSL_CTX* sslContext, BaseCertificates& baseCerts)
{
    string dhFile;
    int encoding = 0;

    // TODO: This just looks plain wrong.  RSA instead of DH params??? -ASN

    if (baseCerts.getDHParams().getKeySize() != 0)
    {
        dhFile = baseCerts.getDHParams().getFileName();
        encoding = baseCerts.getDHParams().getEncoding();
    }
    else if (baseCerts.getRSACert().getKeySize() != 0)
    {
        dhFile = baseCerts.getRSACert().getPublic().getFileName();
        encoding = baseCerts.getRSACert().getPublic().getEncoding();
    }

    DH* dh = 0;

    // File type must be PEM - that's the only way we can load
    // DH Params, apparently.
    if ((!dhFile.empty()) && (encoding == SSL_FILETYPE_PEM))
    {
        dh = loadDHParam(dhFile.c_str());
    }

    if (dh == 0)
    {
        if (_traceLevels->security >= IceSecurity::SECURITY_WARNINGS)
        { 
            _logger->trace(_traceLevels->securityCat,
                           "WRN Could not load Diffie-Hellman params, generating a temporary 512bit key.");
        }

        dh = getTempDH512();
    }

    if (dh != 0)
    {
        SSL_CTX_set_tmp_dh(sslContext, dh);

        DH_free(dh);
    }
}

void
IceSecurity::Ssl::OpenSSL::System::setCipherList(SSL_CTX* sslContext, const string& cipherList)
{
    if (!cipherList.empty() && (!SSL_CTX_set_cipher_list(sslContext, cipherList.c_str())) &&
        (_traceLevels->security >= IceSecurity::SECURITY_WARNINGS))
    {
        string errorString = "WRN Error setting cipher list " + cipherList + " - using default list.\n";
        errorString += sslGetErrors();
        _logger->trace(_traceLevels->securityCat, errorString);
    }
}

int
IceSecurity::Ssl::OpenSSL::System::seedRand()
{
    int retCode = 1;
    char buffer[1024];
	
#ifdef WINDOWS
    RAND_screen();
#endif

    const char* file = RAND_file_name(buffer, sizeof(buffer));

    if (file == 0 || !RAND_load_file(file, -1))
    {
        retCode = 0;
    }
    else
    {
        _randSeeded = 1;
    }

    return retCode;
}

long
IceSecurity::Ssl::OpenSSL::System::loadRandFiles(const string& names)
{
    long tot = 0;

    if (!names.empty())
    {
        int egd;

        // Make a modifiable copy of the string.
        char* namesString = new char[names.length() + 1];
        strcpy(namesString, names.c_str());

        char seps[5];

        sprintf(seps, "%c", LIST_SEPARATOR_CHAR);

        char* token = strtok(namesString, seps);

        while (token != 0)
        {
            egd = RAND_egd(token);

            if (egd > 0)
            {
                tot += egd;
            }
            else
            {
                tot += RAND_load_file(token, -1);
            }

            token = strtok(0, seps);
        }

        if (tot > 512)
        {
            _randSeeded = 1;
        }

        delete []namesString;
    }

    return tot;
}

void
IceSecurity::Ssl::OpenSSL::System::initRandSystem(const string& randBytesFiles)
{
    if (!_randSeeded)
    {
        long randBytesLoaded = 0;

        if (!seedRand() && randBytesFiles.empty() && !RAND_status() &&
            (_traceLevels->security >= IceSecurity::SECURITY_WARNINGS))
        {
            _logger->trace(_traceLevels->securityCat,
                           "WRN There is a lack of random data, consider specifying a random data file.");
        }

        if (!randBytesFiles.empty())
        {
            randBytesLoaded = loadRandFiles(randBytesFiles);
        }
    }
}

void
IceSecurity::Ssl::OpenSSL::System::loadTempCerts(TempCertificates& tempCerts)
{
    RSAVector::iterator iRSA = tempCerts.getRSACerts().begin();
    RSAVector::iterator eRSA = tempCerts.getRSACerts().end();

    while (iRSA != eRSA)
    {
        _tempRSAFileMap[(*iRSA).getKeySize()] = *iRSA;
        iRSA++;
    }

    DSAVector::iterator iDSA = tempCerts.getDSACerts().begin();
    DSAVector::iterator eDSA = tempCerts.getDSACerts().end();

    while (iDSA != eDSA)
    {
        _tempDSAFileMap[(*iDSA).getKeySize()] = *iDSA;
        iDSA++;
    }

    DHVector::iterator iDHP = tempCerts.getDHParams().begin();
    DHVector::iterator eDHP = tempCerts.getDHParams().end();

    while (iDHP != eDHP)
    {
        _tempDHParamsFileMap[(*iDHP).getKeySize()] = *iDHP;
        iDHP++;
    }
}
