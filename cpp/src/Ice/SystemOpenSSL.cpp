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
#include <IceUtil/Mutex.h>
#include <IceUtil/RecMutex.h>
#include <Ice/SslConnectionOpenSSL.h>
#include <Ice/SystemOpenSSL.h>
#include <Ice/SslException.h>
#include <Ice/ConfigParser.h>
#include <Ice/OpenSSLJanitors.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>

#include <openssl/e_os.h>
#include <openssl/rand.h>

#include <sstream>

using namespace std;
using IceInternal::TraceLevelsPtr;
using Ice::LoggerPtr;

using IceSSL::OpenSSL::ContextException;
using IceSSL::OpenSSL::UnsupportedContextException;
using IceSSL::SystemInternalPtr;

IceSSL::ConnectionPtr
IceSSL::OpenSSL::System::createConnection(ContextType connectionType, int socket)
{
    IceUtil::RecMutex::Lock sync(_configMutex);

    if (connectionType == ClientServer)
    {
        UnsupportedContextException unsupportedException(__FILE__, __LINE__);

        unsupportedException._message = "Unable to create ClientServer connections.";

        throw unsupportedException;
    }

    // Configure the context if need be.
    if (!isConfigured(connectionType))
    {
        configure(connectionType);
    }

    IceSSL::ConnectionPtr connection;

    if (connectionType == Client)
    {
        connection = _clientContext.createConnection(socket, this);
    }
    else if (connectionType == Server)
    {
        connection = _serverContext.createConnection(socket, this);
    }

    return connection;
}

void
IceSSL::OpenSSL::System::shutdown()
{
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

bool
IceSSL::OpenSSL::System::isConfigured(ContextType contextType)
{
    IceUtil::RecMutex::Lock sync(_configMutex);

    bool retCode = false;

    switch (contextType)
    {
        case Client :
        {
            retCode = _clientContext.isConfigured();
            break;
        }

        case Server :
        {
            retCode = _serverContext.isConfigured();
            break;
        }

        case ClientServer :
        {
            retCode = _clientContext.isConfigured() && _serverContext.isConfigured();
            break;
        }
    }

    return retCode;
}

void
IceSSL::OpenSSL::System::configure(ContextType contextType)
{
    IceUtil::RecMutex::Lock sync(_configMutex);

    switch (contextType)
    {
        case Client :
        {
            string configFile = _properties->getProperty("Ice.SSL.Client.Config");
            string certPath   = _properties->getProperty("Ice.SSL.Client.CertPath");
            loadConfig(Client, configFile, certPath);
            break;
        }

        case Server :
        {
            string configFile = _properties->getProperty("Ice.SSL.Server.Config");
            string certPath   = _properties->getProperty("Ice.SSL.Server.CertPath");
            loadConfig(Server, configFile, certPath);
            break;
        }

        case ClientServer :
        {
            string clientConfigFile = _properties->getProperty("Ice.SSL.Client.Config");
            string clientCertPath   = _properties->getProperty("Ice.SSL.Client.CertPath");
            string serverConfigFile = _properties->getProperty("Ice.SSL.Server.Config");
            string serverCertPath   = _properties->getProperty("Ice.SSL.Server.CertPath");

            // Short cut, so that we only have to load the file once.
            if ((clientConfigFile == serverConfigFile) && (clientCertPath == serverCertPath))
            {
                loadConfig(ClientServer, clientConfigFile, clientCertPath);
            }
            else
            {
                loadConfig(Client, clientConfigFile, clientCertPath);
                loadConfig(Server, serverConfigFile, serverCertPath);
            }
            break;
        }
    }
}

void
IceSSL::OpenSSL::System::loadConfig(ContextType contextType,
                                    const std::string& configFile,
                                    const std::string& certPath)
{
    if (configFile.empty())
    {
        IceSSL::ConfigurationLoadingException configEx(__FILE__, __LINE__);

        string contextString;

        switch (contextType)
        {
            case Client :
            {
                contextString = "Client";
                break;
            }

            case Server :
            {
                contextString = "Server";
                break;
            }

            case ClientServer :
            {
                contextString = "Client/Server";
                break;
            }
        }

        configEx._message = "No SSL configuration file specified for ";
        configEx._message += contextString;
        configEx._message += ".";

        throw configEx;
    }

    ConfigParser sslConfig(configFile, certPath);

    sslConfig.setTrace(_traceLevels);
    sslConfig.setLogger(_logger);

    // Actually parse the file now.
    sslConfig.process();

    if ((contextType == Client || contextType == ClientServer))
    {
        GeneralConfig clientGeneral;
        CertificateAuthority clientCertAuth;
        BaseCertificates clientBaseCerts;

        // Walk the parse tree, get the Client configuration.
        if (sslConfig.loadClientConfig(clientGeneral, clientCertAuth, clientBaseCerts))
        {
            initRandSystem(clientGeneral.getRandomBytesFiles());

            _clientContext.configure(clientGeneral, clientCertAuth, clientBaseCerts);
        }
    }

    if ((contextType == Server || contextType == ClientServer))
    {
        GeneralConfig serverGeneral;
        CertificateAuthority serverCertAuth;
        BaseCertificates serverBaseCerts;
        TempCertificates serverTempCerts;

        // Walk the parse tree, get the Server configuration.
        if (sslConfig.loadServerConfig(serverGeneral, serverCertAuth, serverBaseCerts, serverTempCerts))
        {
            initRandSystem(serverGeneral.getRandomBytesFiles());

            loadTempCerts(serverTempCerts);

            _serverContext.configure(serverGeneral, serverCertAuth, serverBaseCerts);

            if (_traceLevels->security >= IceSSL::SECURITY_PROTOCOL)
            {
                ostringstream s;

                s << "Temp Certificates - Server" << endl;
                s << "--------------------------" << endl;
                s << serverTempCerts << endl;

                _logger->trace(_traceLevels->securityCat, s.str());
            }
        }
    }
}

RSA*
IceSSL::OpenSSL::System::getRSAKey(int isExport, int keyLength)
{
    IceUtil::Mutex::Lock sync(_tempRSAKeysMutex);

    RSA* rsa_tmp = 0;

    RSAMap::iterator retVal = _tempRSAKeys.find(keyLength);

    // Does the key already exist?
    if (retVal != _tempRSAKeys.end())
    {
        // Yes!  Use it.
        rsa_tmp = (*retVal).second;

        assert(rsa_tmp != 0);
    }
    else
    {
        const RSACertMap::iterator& it = _tempRSAFileMap.find(keyLength);

        // First we try to load a private and public key from specified files
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
                BIOJanitor bioJanitor(bio);

                rsaCert = PEM_read_bio_RSAPublicKey(bio, 0, 0, 0);
            }

            if (rsaCert != 0)
            {
                if ((bio = BIO_new_file(privKeyFile.c_str(), "r")) != 0)
                {
                    BIOJanitor bioJanitor(bio);

                    rsaKey = PEM_read_bio_RSAPrivateKey(bio, &rsaCert, 0, 0);
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

        // Couldn't load file, last ditch effort - generate a key on the fly.
        if (rsa_tmp == 0)
        {
            rsa_tmp = RSA_generate_key(keyLength, RSA_F4, 0, 0);
        }

        // Save in our temporary key cache.
        if (rsa_tmp != 0)
        {
            _tempRSAKeys[keyLength] = rsa_tmp;
        }
    }

    return rsa_tmp;
}

DH*
IceSSL::OpenSSL::System::getDHParams(int isExport, int keyLength)
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

        // First we try to load params from specified files
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

void
IceSSL::OpenSSL::System::setCertificateVerifier(ContextType contextType,
                                                const IceSSL::CertificateVerifierPtr& verifier)
{
    IceUtil::RecMutex::Lock sync(_configMutex);

    IceSSL::OpenSSL::CertificateVerifierPtr castVerifier;
    castVerifier = IceSSL::OpenSSL::CertificateVerifierPtr::dynamicCast(verifier);

    if (!castVerifier.get())
    {
        IceSSL::CertificateVerifierTypeException cvtEx(__FILE__, __LINE__);
        throw cvtEx;
    }

    if (contextType == Client || contextType == ClientServer)
    {
        _clientContext.setCertificateVerifier(castVerifier);
    }

    if (contextType == Server || contextType == ClientServer)
    {
        _serverContext.setCertificateVerifier(castVerifier);
    }
}

void
IceSSL::OpenSSL::System::addTrustedCertificateBase64(ContextType contextType, const string& certString)
{
    IceUtil::RecMutex::Lock sync(_configMutex);

    if (contextType == Client || contextType == ClientServer)
    {
        _clientContext.addTrustedCertificateBase64(certString);
    }

    if (contextType == Server || contextType == ClientServer)
    {
        _serverContext.addTrustedCertificateBase64(certString);
    }
}

void
IceSSL::OpenSSL::System::addTrustedCertificate(ContextType contextType, const Ice::ByteSeq& certSeq)
{
    IceUtil::RecMutex::Lock sync(_configMutex);

    if (contextType == Client || contextType == ClientServer)
    {
        _clientContext.addTrustedCertificate(certSeq);
    }

    if (contextType == Server || contextType == ClientServer)
    {
        _serverContext.addTrustedCertificate(certSeq);
    }
}

void
IceSSL::OpenSSL::System::setRSAKeysBase64(ContextType contextType,
                                          const std::string& privateKey,
                                          const std::string& publicKey)
{
    IceUtil::RecMutex::Lock sync(_configMutex);

    if (contextType == Client || contextType == ClientServer)
    {
        _clientContext.setRSAKeysBase64(privateKey, publicKey);
    }

    if (contextType == Server || contextType == ClientServer)
    {
        _serverContext.setRSAKeysBase64(privateKey, publicKey);
    }
}

void
IceSSL::OpenSSL::System::setRSAKeys(ContextType contextType,
                                    const ::Ice::ByteSeq& privateKey,
                                    const ::Ice::ByteSeq& publicKey)
{
    IceUtil::RecMutex::Lock sync(_configMutex);

    if (contextType == Client || contextType == ClientServer)
    {
        _clientContext.setRSAKeys(privateKey, publicKey);
    }

    if (contextType == Server || contextType == ClientServer)
    {
        _serverContext.setRSAKeys(privateKey, publicKey);
    }
}

//
// Protected
//

IceSSL::OpenSSL::System::System(const IceInternal::InstancePtr& instance) :
                        IceSSL::SystemInternal(instance),
                        _serverContext(instance),
                        _clientContext(instance)
{
    _randSeeded = 0;

    SSL_load_error_strings();

    OpenSSL_add_ssl_algorithms();
}

IceSSL::OpenSSL::System::~System()
{
    shutdown();
}

//
// Private
//

int
IceSSL::OpenSSL::System::seedRand()
{
#ifdef WINDOWS
    RAND_screen();
#endif

    char buffer[1024];
    const char* file = RAND_file_name(buffer, sizeof(buffer));

    if (file == 0)
    {
        return 0;
    }
    
    return RAND_load_file(file, -1);
}

long
IceSSL::OpenSSL::System::loadRandFiles(const string& names)
{
    if (!names.empty())
    {
        return 0;
    }

    long tot = 0;
    int egd;

    // Make a modifiable copy of the string.
    char* namesString = new char[names.length() + 1];
    assert(namesString != 0);

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

    return tot;
}

void
IceSSL::OpenSSL::System::initRandSystem(const string& randBytesFiles)
{
    if (_randSeeded)
    {
        return;
    }

    long randBytesLoaded = seedRand();

    if (!randBytesFiles.empty())
    {
        randBytesLoaded += loadRandFiles(randBytesFiles);
    }

    if (!randBytesLoaded && !RAND_status() && (_traceLevels->security >= IceSSL::SECURITY_WARNINGS))
    {
        // In this case, there are two options open to us - specify a random data file using the
        // RANDFILE environment variable, or specify additional random data files in the
        // SSL configuration file.
        _logger->trace(_traceLevels->securityCat,
                       "WRN There is a lack of random data, consider specifying additional random data files.");
    }

    _randSeeded = (randBytesLoaded > 0 ? 1 : 0);
}

void
IceSSL::OpenSSL::System::loadTempCerts(TempCertificates& tempCerts)
{
    RSAVector::iterator iRSA = tempCerts.getRSACerts().begin();
    RSAVector::iterator eRSA = tempCerts.getRSACerts().end();

    while (iRSA != eRSA)
    {
        _tempRSAFileMap[(*iRSA).getKeySize()] = *iRSA;
        iRSA++;
    }

    DHVector::iterator iDHP = tempCerts.getDHParams().begin();
    DHVector::iterator eDHP = tempCerts.getDHParams().end();

    while (iDHP != eDHP)
    {
        _tempDHParamsFileMap[(*iDHP).getKeySize()] = *iDHP;
        iDHP++;
    }
}
