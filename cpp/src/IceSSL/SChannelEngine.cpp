// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/SSLEngine.h>
#include <IceSSL/Plugin.h>

#include <Ice/LocalException.h>
#include <Ice/Logger.h>
#include <Ice/Communicator.h>

#include <IceUtil/StringUtil.h>
#include <IceUtil/FileUtil.h>
#include <IceUtil/UUID.h>

using namespace std;
using namespace Ice;
using namespace IceUtil;
using namespace IceUtilInternal;
using namespace IceSSL;

#ifdef ICE_USE_SCHANNEL

Shared* IceSSL::upCast(IceSSL::SChannelEngine* p) { return p; }

namespace
{

#   ifdef __MINGW32__
//
// CERT_CHAIN_ENGINE_CONFIG struct in mingw headers doesn't include
// new members added in Windows 7, we add our ouwn definition and
// then cast it to CERT_CHAIN_ENGINE_CONFIG this works because the
// linked libraries include the new version.
//
struct CertChainEngineConfig
{
    DWORD cbSize;
    HCERTSTORE hRestrictedRoot;
    HCERTSTORE hRestrictedTrust;
    HCERTSTORE hRestrictedOther;
    DWORD cAdditionalStore;
    HCERTSTORE *rghAdditionalStore;
    DWORD dwFlags;
    DWORD dwUrlRetrievalTimeout;
    DWORD MaximumCachedCertificates;
    DWORD CycleDetectionModulus;
    HCERTSTORE hExclusiveRoot;
    HCERTSTORE hExclusiveTrustedPeople;
};
#   endif

void
addCertificateToStore(const string& file, HCERTSTORE store, PCCERT_CONTEXT* cert = 0)
{
    vector<char> buffer;
    readFile(file, buffer);
    vector<BYTE> outBuffer;
    outBuffer.resize(buffer.size());
    DWORD outLength = static_cast<DWORD>(outBuffer.size());

    if(!CryptStringToBinary(&buffer[0], static_cast<DWORD>(buffer.size()), CRYPT_STRING_BASE64HEADER,
                            &outBuffer[0], &outLength, 0, 0))
    {
        //
        // Base64 data should always be bigger than binary
        //
        assert(GetLastError() != ERROR_MORE_DATA);
        throw PluginInitializationException(__FILE__, __LINE__,
                                "IceSSL: error decoding certificate:\n" + lastErrorToString());
    }

    if(!CertAddEncodedCertificateToStore(store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, &outBuffer[0],
                                         outLength, CERT_STORE_ADD_NEW, cert))
    {
        if(GetLastError() != static_cast<DWORD>(CRYPT_E_EXISTS))
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                "IceSSL: error decoding certificate:\n" + lastErrorToString());
        }
    }
}

DWORD
parseProtocols(const StringSeq& protocols)
{
    DWORD v = 0;

    for(Ice::StringSeq::const_iterator p = protocols.begin(); p != protocols.end(); ++p)
    {
        string prot = IceUtilInternal::toUpper(*p);

        if(prot == "SSL3" || prot == "SSLV3")
        {
            v |= SP_PROT_SSL3_SERVER;
            v |= SP_PROT_SSL3_CLIENT;
        }
        else if(prot == "TLS" || prot == "TLS1" || prot == "TLSV1" || prot == "TLS1_0" || prot == "TLSV1_0")
        {
            v |= SP_PROT_TLS1_SERVER;
            v |= SP_PROT_TLS1_CLIENT;
        }
        else if(prot == "TLS1_1" || prot == "TLSV1_1")
        {
            v |= SP_PROT_TLS1_1_SERVER;
            v |= SP_PROT_TLS1_1_CLIENT;
        }
        else if(prot == "TLS1_2" || prot == "TLSV1_2")
        {
            v |= SP_PROT_TLS1_2_SERVER;
            v |= SP_PROT_TLS1_2_CLIENT;
        }
        else
        {
            throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unrecognized protocol `" + *p + "'");
        }
    }

    return v;
}

const ALG_ID supportedCiphers[] = { CALG_3DES, CALG_AES_128, CALG_AES_256, CALG_DES, CALG_RC2, CALG_RC4 };
const int supportedCiphersSize = sizeof(supportedCiphers)/sizeof(ALG_ID);

ALG_ID
algorithmId(const string& name)
{
    if(name == "3DES")
    {
        return CALG_3DES;
    }
    if(name == "AES_128")
    {
        return CALG_AES_128;
    }
    if(name == "AES_256")
    {
        return CALG_AES_256;
    }
    if(name == "DES")
    {
        return CALG_DES;
    }
    if(name == "RC2")
    {
        return CALG_RC2;
    }
    if(name == "RC4")
    {
        return CALG_RC4;
    }
    return 0;
}

}

SChannelEngine::SChannelEngine(const CommunicatorPtr& communicator) :
    SSLEngine(communicator),
    _initialized(false),
    _rootStore(0),
    _chainEngine(0)
{
}

void
SChannelEngine::initialize()
{
    Mutex::Lock lock(_mutex);
    if(_initialized)
    {
        return;
    }

    SSLEngine::initialize();

    const string prefix = "IceSSL.";
    const PropertiesPtr properties = communicator()->getProperties();

    //
    // Protocols selects which protocols to enable, by default we only enable TLS1.0
    // TLS1.1 and TLS1.2 to avoid security issues with SSLv3
    //
    vector<string> defaultProtocols;
    defaultProtocols.push_back("tls1_0");
    defaultProtocols.push_back("tls1_1");
    defaultProtocols.push_back("tls1_2");
    const_cast<DWORD&>(_protocols) = 
                    parseProtocols(properties->getPropertyAsListWithDefault(prefix + "Protocols", defaultProtocols));

    //
    // Check for a default directory. We look in this directory for
    // files mentioned in the configuration.
    //
    string defaultDir = properties->getProperty(prefix + "DefaultDir");

    int passwordRetryMax = properties->getPropertyAsIntWithDefault(prefix + "PasswordRetryMax", 3);
    PasswordPromptPtr passwordPrompt = getPasswordPrompt();
    setPassword(properties->getProperty(prefix + "Password"));

    string ciphers = properties->getProperty(prefix + "Ciphers");
    if(!ciphers.empty())
    {
        parseCiphers(ciphers);
    }

    if(securityTraceLevel() >= 1)
    {
        ostringstream os;
        os << "enabling SSL ciphersuites:";
        if(_ciphers.empty())
        {
            for(int i = 0; i < supportedCiphersSize; ++i)
            {
                os << "\n " << getCipherName(supportedCiphers[i]);
            }
        }
        else
        {
            for(vector<ALG_ID>::const_iterator i = _ciphers.begin(); i != _ciphers.end(); ++i)
            {
                os << "\n " << getCipherName(*i);
            }
        }
        getLogger()->trace(securityTraceCategory(), os.str());
    }

    string certStore = properties->getPropertyWithDefault(prefix + "CertStore", "CurrentUser");
    if(certStore != "CurrentUser" && certStore != "LocalMachine")
    {
        getLogger()->warning("Invalid IceSSL.CertStore value `" + certStore + "' adjusted to `CurrentUser'");
        certStore = "CurrentUser";
    }

    //
    // Create trusted CA store with contents of CertAuthFile
    //
    string caFile = properties->getProperty(prefix + "CertAuthFile");
    if(!caFile.empty())
    {
        _rootStore = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, 0);
        if(!_rootStore)
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                    "IceSSL: error creating in memory certificate store:\n" + lastErrorToString());
        }

        if(!checkPath(caFile, defaultDir, false))
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: CA certificate file not found:\n" + caFile);
        }

        addCertificateToStore(caFile, _rootStore);

        //
        // Create a chain engine that uses our Trusted Root Store
        //
#ifdef __MINGW32__
        CertChainEngineConfig config;
        memset(&config, 0, sizeof(CertChainEngineConfig));
        config.cbSize = sizeof(CertChainEngineConfig);
#else
        CERT_CHAIN_ENGINE_CONFIG config;
        memset(&config, 0, sizeof(CERT_CHAIN_ENGINE_CONFIG));
        config.cbSize = sizeof(CERT_CHAIN_ENGINE_CONFIG);
#endif
        config.hExclusiveRoot = _rootStore;

        //
        // Build the chain using the LocalMachine registry location as opposed
        // to the CurrentUser location.
        //
        if(certStore == "LocalMachine")
        {
            config.dwFlags = CERT_CHAIN_USE_LOCAL_MACHINE_STORE;
        }

#ifdef __MINGW32__
        if(!CertCreateCertificateChainEngine(reinterpret_cast<CERT_CHAIN_ENGINE_CONFIG*>(&config), &_chainEngine))
#else
        if(!CertCreateCertificateChainEngine(&config, &_chainEngine))
#endif
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                    "IceSSL: error creating certificate chain engine:\n" + lastErrorToString());
        }
    }
    else
    {
        _chainEngine = (certStore == "LocalMachine") ? HCCE_LOCAL_MACHINE : HCCE_CURRENT_USER;
    }

    //
    // Import the application certificate and private keys.
    //
    string keySet = properties->getPropertyWithDefault(prefix + "KeySet", "DefaultKeySet");
    if(keySet != "DefaultKeySet" && keySet != "UserKeySet" && keySet != "MachineKeySet")
    {
        getLogger()->warning("Invalid IceSSL.KeySet value `" + keySet + "' adjusted to `DefaultKeySet'");
        keySet = "DefaultKeySet";
    }

    DWORD importFlags = (keySet == "MachineKeySet") ? CRYPT_MACHINE_KEYSET : CRYPT_USER_KEYSET;

    string certFile = properties->getProperty(prefix + "CertFile");
    string keyFile = properties->getPropertyWithDefault(prefix + "KeyFile", certFile);

    if(!certFile.empty())
    {
        vector<string> certFiles;
        if(!splitString(certFile, IceUtilInternal::pathsep, certFiles) || certFiles.size() > 2)
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: invalid value for " + prefix + "CertFile:\n" + certFile);
        }

        vector<string> keyFiles;
        if(!splitString(keyFile, IceUtilInternal::pathsep, keyFiles) || keyFiles.size() > 2)
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: invalid value for " + prefix + "KeyFile:\n" + keyFile);
        }

        if(certFiles.size() != keyFiles.size())
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                        "IceSSL: " + prefix + "KeyFile does not agree with " + prefix + "CertFile");
        }

        for(size_t i = 0; i < certFiles.size(); ++i)
        {
            string certFile = certFiles[i];
            if(!checkPath(certFile, defaultDir, false))
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                                    "IceSSL: certificate file not found:\n" + certFile);
            }

            vector<char> buffer;
            readFile(certFile, buffer);

            CRYPT_DATA_BLOB pfxBlob;
            pfxBlob.cbData = static_cast<DWORD>(buffer.size());
            pfxBlob.pbData = reinterpret_cast<BYTE*>(&buffer[0]);

            HCERTSTORE store = 0;
            PCCERT_CONTEXT cert = 0;
            int err = 0;
            int count = 0;
            do
            {
                string s = password(false);
                store = PFXImportCertStore(&pfxBlob, stringToWstring(s).c_str(), importFlags);
                err = store ? 0 : GetLastError();
            }
            while(err == ERROR_INVALID_PASSWORD && passwordPrompt && ++count < passwordRetryMax);

            if(store)
            {
                _stores.push_back(store);
                cert = CertFindCertificateInStore(store, X509_ASN_ENCODING, 0, CERT_FIND_ANY, 0, cert);
                if(!cert)
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: certificate error:\n" + lastErrorToString());
                }
                _certs.push_back(cert);
                continue;
            }

            assert(err);

            if(err != CRYPT_E_BAD_ENCODE)
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                    "IceSSL: error decoding certificate:\n" + lastErrorToString());
            }

            //
            // Try to load certificate & key as PEM files.
            //
            err = 0;
            keyFile = keyFiles[i];
            if(!checkPath(keyFile, defaultDir, false))
            {
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: key file not found:\n" + keyFile);
            }

            readFile(keyFile, buffer);

            vector<BYTE> outBuffer;
            outBuffer.resize(buffer.size());
            DWORD outLength = static_cast<DWORD>(buffer.size());

            //
            // Convert the PEM encoded buffer to DER binary format.
            //
            if(!CryptStringToBinary(&buffer[0], static_cast<DWORD>(buffer.size()), CRYPT_STRING_BASE64HEADER,
                                    &outBuffer[0], &outLength, 0, 0))
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: error decoding key:\n" + lastErrorToString());
            }

            PCRYPT_PRIVATE_KEY_INFO keyInfo = 0;
            BYTE* key = 0;
            HCRYPTKEY hKey = 0;

            try
            {
                DWORD decodedLength = 0;
                if(!CryptDecodeObjectEx(X509_ASN_ENCODING, PKCS_PRIVATE_KEY_INFO, &outBuffer[0], outLength,
                                        CRYPT_DECODE_ALLOC_FLAG, 0, &keyInfo, &decodedLength))
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: error decoding key:\n" + lastErrorToString());
                }

                //
                // Check that we are using a RSA Key
                //
                if(strcmp(keyInfo->Algorithm.pszObjId, szOID_RSA_RSA))
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                string("IceSSL: error unknow key algorithm: `") + keyInfo->Algorithm.pszObjId + "'");
                }

                //
                // Create a new RSA key set to store our key
                //
                const wstring keySetName = stringToWstring(generateUUID());
                HCRYPTPROV cryptProv = 0;

                DWORD contextFlags = (keySet == "MachineKeySet") ? CRYPT_MACHINE_KEYSET | CRYPT_NEWKEYSET :
                                                                   CRYPT_NEWKEYSET;

                if(!CryptAcquireContextW(&cryptProv, keySetName.c_str(), MS_DEF_PROV_W, PROV_RSA_FULL, contextFlags))
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                            "IceSSL: error acquiring cryptographic context:\n" + lastErrorToString());
                }

                //
                // Decode the private key BLOB
                //
                if(!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RSA_PRIVATE_KEY,
                                        keyInfo->PrivateKey.pbData, keyInfo->PrivateKey.cbData,
                                        CRYPT_DECODE_ALLOC_FLAG, 0, &key, &outLength))
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: error decoding key:\n" + lastErrorToString());
                }
                LocalFree(keyInfo);
                keyInfo = 0;

                //
                // Import the private key
                //
                if(!CryptImportKey(cryptProv, key, outLength, 0, 0, &hKey))
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: error importing key:\n" + lastErrorToString());
                }
                LocalFree(key);
                key = 0;

                CryptDestroyKey(hKey);
                hKey = 0;

                //
                // Create a new memory store to place the certificate
                //
                store = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, 0);
                if(!store)
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                "IceSSL: error creating certificate store:\n" + lastErrorToString());
                }

                addCertificateToStore(certFile, store, &cert);

                //
                // Associate key & certificate
                //
                CRYPT_KEY_PROV_INFO keyProvInfo;
                memset(&keyProvInfo, 0, sizeof(keyProvInfo));
                keyProvInfo.pwszContainerName = const_cast<wchar_t*>(keySetName.c_str());
                keyProvInfo.pwszProvName = const_cast<wchar_t*>(MS_DEF_PROV_W);
                keyProvInfo.dwProvType = PROV_RSA_FULL;
                keyProvInfo.dwKeySpec = AT_KEYEXCHANGE;

                if(!CertSetCertificateContextProperty(cert, CERT_KEY_PROV_INFO_PROP_ID, 0, &keyProvInfo))
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                "IceSSL: error seting certificate property:\n" + lastErrorToString());
                }

                _certs.push_back(cert);
                _stores.push_back(store);
            }
            catch(...)
            {
                if(keyInfo)
                {
                    LocalFree(keyInfo);
                }

                if(key)
                {
                    LocalFree(key);
                }

                if(hKey)
                {
                    CryptDestroyKey(hKey);
                }

                if(cert)
                {
                    CertFreeCertificateContext(cert);
                }

                if(store)
                {
                    CertCloseStore(store, 0);
                }
                throw;
            }
        }

        _allCerts.insert(_allCerts.end(), _certs.begin(), _certs.end());
    }

    const string findPrefix = prefix + "FindCert.";
    map<string, string> certProps = properties->getPropertiesForPrefix(findPrefix);
    if(!certProps.empty())
    {
        for(map<string, string>::const_iterator i = certProps.begin(); i != certProps.end(); ++i)
        {
            const string name = i->first;
            const string val = i->second;

            if(!val.empty())
            {
                string storeSpec = name.substr(findPrefix.size());
                vector<PCCERT_CONTEXT> certs = findCertificates(name, storeSpec, val, _stores);
                _allCerts.insert(_allCerts.end(), certs.begin(), certs.end());
            }
        }

        if(_allCerts.empty())
        {
            throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: no certificates found");
        }
    }
    _initialized = true;
}

string
SChannelEngine::getCipherName(ALG_ID cipher) const
{
    switch(cipher)
    {
        case CALG_RSA_KEYX:
            return "RSA_KEYX";
        case CALG_RSA_SIGN:
            return "RSA_SIGN";
        case CALG_DSS_SIGN:
            return "DSS_SIGN";
        case CALG_KEA_KEYX:
            return "KEA_KEYX";
        case CALG_DH_EPHEM:
            return "DH_EPHEM";
        case CALG_ECDH:
            return "ECDH";
        case CALG_ECDSA:
            return "ECDSA";
        case CALG_3DES:
            return "3DES";
        case CALG_AES_128:
            return "AES_128";
        case CALG_AES_256:
            return "AES_256";
        case CALG_DES:
            return "DES";
        case CALG_RC2:
            return "RC2";
        case CALG_RC4:
            return "RC4";
        default:
            return "Unknown";
    }
}

bool
SChannelEngine::initialized() const
{
    Mutex::Lock lock(_mutex);
    return _initialized;
}

CredHandle
SChannelEngine::newCredentialsHandle(bool incoming)
{
    SCHANNEL_CRED cred;
    memset(&cred, 0, sizeof(cred));
    cred.dwVersion = SCHANNEL_CRED_VERSION;

    if(!_allCerts.empty())
    {
        cred.cCreds = static_cast<DWORD>(_allCerts.size());
        cred.paCred = &_allCerts[0];
    }

    cred.grbitEnabledProtocols = _protocols;

    if(incoming)
    {
        //
        // Don't set SCH_SEND_ROOT_CERT as it seems to cause problems with
        // Java certificate validation and SChannel doesn't seems to send
        // the root certificate either way.
        //
        cred.dwFlags = SCH_CRED_NO_SYSTEM_MAPPER;
        if(_rootStore)
        {
            cred.hRootStore = _rootStore;
        }
    }
    else
    {
        cred.dwFlags = SCH_CRED_MANUAL_CRED_VALIDATION | SCH_CRED_NO_SERVERNAME_CHECK | SCH_CRED_NO_DEFAULT_CREDS;
    }

    if(!_ciphers.empty())
    {
        cred.cSupportedAlgs = static_cast<DWORD>(_ciphers.size());
        cred.palgSupportedAlgs = &_ciphers[0];
    }

    CredHandle credHandle;
    memset(&credHandle, 0, sizeof(credHandle));

    SECURITY_STATUS err =
        AcquireCredentialsHandle(0, const_cast<char*>(UNISP_NAME), (incoming ? SECPKG_CRED_INBOUND : SECPKG_CRED_OUTBOUND), 0, &cred, 0,
                                 0, &credHandle, 0);

    if(err != SEC_E_OK)
    {
        throw SecurityException(__FILE__, __LINE__,
                        "IceSSL: failed to acquire credentials handle:\n" + lastErrorToString());
    }
    return credHandle;
}

HCERTCHAINENGINE
SChannelEngine::chainEngine() const
{
    return _chainEngine;
}

void
SChannelEngine::parseCiphers(const std::string& ciphers)
{
    vector<string> tokens;
    splitString(ciphers, " \t", tokens);
    for(vector<string>::const_iterator i = tokens.begin(); i != tokens.end(); ++i)
    {
        ALG_ID id = algorithmId(*i);
        if(id)
        {
            _ciphers.push_back(id);
        }
    }
}

void
SChannelEngine::destroy()
{
    if(_chainEngine && _chainEngine != HCCE_CURRENT_USER && _chainEngine != HCCE_LOCAL_MACHINE)
    {
        CertFreeCertificateChainEngine(_chainEngine);
    }

    if(_rootStore)
    {
        CertCloseStore(_rootStore, 0);
    }

    for(vector<PCCERT_CONTEXT>::const_iterator i = _allCerts.begin(); i != _allCerts.end(); ++i)
    {
        PCCERT_CONTEXT cert = *i;

        //
        // Only remove the keysets we create.
        //
        if(find(_certs.begin(), _certs.end(), cert) != _certs.end())
        {
            //
            // Retrieve the certificate CERT_KEY_PROV_INFO_PROP_ID property, we use the CRYPT_KEY_PROV_INFO
            // data to then remove the key set associated with the certificate.
            //
            DWORD length = 0;
            if(CertGetCertificateContextProperty(cert, CERT_KEY_PROV_INFO_PROP_ID, 0, &length))
            {
                vector<char> buf(length);
                if(CertGetCertificateContextProperty(cert, CERT_KEY_PROV_INFO_PROP_ID, &buf[0], &length))
                {
                    CRYPT_KEY_PROV_INFO* keyProvInfo = reinterpret_cast<CRYPT_KEY_PROV_INFO*>(&buf[0]);
                    HCRYPTPROV cryptProv = 0;
                    CryptAcquireContextW(&cryptProv, keyProvInfo->pwszContainerName, keyProvInfo->pwszProvName,
                                         keyProvInfo->dwProvType, CRYPT_DELETEKEYSET);
                }
                CertFreeCertificateContext(cert);
            }
        }
    }

    for(vector<HCERTSTORE>::const_iterator i = _stores.begin(); i != _stores.end(); ++i)
    {
        CertCloseStore(*i, 0);
    }
}
#endif
