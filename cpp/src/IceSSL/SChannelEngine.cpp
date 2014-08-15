// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
        string prot = *p;

        if(prot == "ssl3" || prot == "sslv3")
        {
            v |= SP_PROT_SSL3_SERVER;
            v |= SP_PROT_SSL3_CLIENT;
        }
        else if(prot == "tls" || prot == "tls1" || prot == "tlsv1" || prot == "tls1_0" || prot == "tlsv1_0")
        {
            v |= SP_PROT_TLS1_SERVER;
            v |= SP_PROT_TLS1_CLIENT;
        }
        else if(prot == "tls1_1" || prot == "tlsv1_1")
        {
            v |= SP_PROT_TLS1_1_SERVER;
            v |= SP_PROT_TLS1_1_CLIENT;
        }
        else if(prot == "tls1_2" || prot == "tlsv1_2")
        {
            v |= SP_PROT_TLS1_2_SERVER;
            v |= SP_PROT_TLS1_2_CLIENT;
        }
        else
        {
            throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unrecognized protocol `" + prot + "'");
        }
    }

    return v;
}

const ALG_ID supportedChipers[] = { CALG_3DES, CALG_AES_128, CALG_AES_256, CALG_DES, CALG_RC2, CALG_RC4 };
const int supportedChipersSize = sizeof(supportedChipers)/sizeof(ALG_ID);

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

//
// Parse a string of the form "location.name" into two parts.
//
void
parseStore(const string& prop, const string& store, DWORD& loc, string& sname)
{
    size_t pos = store.find('.');
    if(pos == string::npos)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: property `" + prop + "' has invalid format");
    }

    const string sloc = toUpper(store.substr(0, pos));
    if(sloc == "CURRENTUSER")
    {
        loc = CERT_SYSTEM_STORE_CURRENT_USER;
    }
    else if(sloc == "LOCALMACHINE")
    {
        loc = CERT_SYSTEM_STORE_LOCAL_MACHINE;
    }
    else
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unknown store location `" + sloc + "' in " + prop);
    }

    sname = store.substr(pos + 1);
    if(sname.empty())
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: invalid store name in " + prop);
    }
}


bool
parseBytes(const string& arg, vector<BYTE>& buffer)
{
    string v = toUpper(arg);

    //
    // Check for any invalid characters.
    //
    size_t pos = v.find_first_not_of(" :0123456789ABCDEF");
    if(pos != string::npos)
    {
        return false;
    }

    //
    // Remove any separator characters.
    //
    ostringstream s;
    for(string::const_iterator i = v.begin(); i != v.end(); ++i)
    {
        if(*i == ' ' || *i == ':')
        {
            continue;
        }
        s << *i;
    }
    v = s.str();

    //
    // Convert the bytes.
    //
    for(size_t i = 0, length = v.size(); i + 2 <= length;)
    {
        buffer.push_back(static_cast<BYTE>(strtol(v.substr(i, 2).c_str(), 0, 16)));
        i += 2;
    }
    return true;
}

void
addMatchingCertificates(HCERTSTORE source, HCERTSTORE target, DWORD findType, const void* findParam)
{
    PCCERT_CONTEXT next = 0;
    do
    { 
        if((next = CertFindCertificateInStore(source, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, 
                                              findType, findParam, next)))
        {
            if(!CertAddCertificateContextToStore(target, next, CERT_STORE_ADD_ALWAYS, 0))
            {
                throw PluginInitializationException(__FILE__, __LINE__, 
                    "IceSSL: error adding certificate to store:\n" + lastErrorToString());
            }
        }
    }
    while(next);
}

vector<PCCERT_CONTEXT> 
findCertificates(const string& prop, const string& storeSpec, const string& value, vector<HCERTSTORE>& stores)
{
    DWORD storeLoc = 0;
    string storeName;
    parseStore(prop, storeSpec, storeLoc, storeName);

    HCERTSTORE store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0, storeLoc, stringToWstring(storeName).c_str());
    if(!store)
    {
        throw PluginInitializationException(__FILE__, __LINE__, 
            "IceSSL: failure while opening store specified by " + prop + ":\n" + lastErrorToString());
    }

    //
    // Start with all of the certificates in the collection and filter as necessary.
    //
    // - If the value is "*", return all certificates.
    // - Otherwise, search using key:value pairs. The following keys are supported:
    //
    //   Issuer
    //   IssuerDN
    //   Serial
    //   Subject
    //   SubjectDN
    //   SubjectKeyId
    //   Thumbprint
    //
    //   A value must be enclosed in single or double quotes if it contains whitespace.
    //

    HCERTSTORE tmpStore = 0;
    try
    {
        if(value != "*")
        {
            size_t start = 0;
            size_t pos;
            while((pos = value.find(':', start)) != string::npos)
            {
                string field = toUpper(trim(value.substr(start, pos - start)));
                if(field != "SUBJECT" && field != "SUBJECTDN" && field != "ISSUER" && field != "ISSUERDN" && 
                   field != "THUMBPRINT" && field != "SUBJECTKEYID" && field != "SERIAL")
                {
                    throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unknown key in `" + value + "'");
                }

                start = pos + 1;
                while(start < value.size() && (value[start] == ' ' || value[start] == '\t'))
                {
                    ++start;
                }
                
                if(start == value.size())
                {
                    throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: missing argument in `" + value + "'");
                }

                string arg;
                if(value[start] == '"' || value[start] == '\'')
                {
                    size_t end = start;
                    ++end;
                    while(end < value.size())
                    {
                        if(value[end] == value[start] && value[end - 1] != '\\')
                        {
                            break;
                        }
                        ++end;
                    }
                    if(end == value.size() || value[end] != value[start])
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                                                            "IceSSL: unmatched quote in `" + value + "'");
                    }
                    ++start;
                    arg = value.substr(start, end - start);
                    start = end + 1;
                }
                else
                {
                    size_t end = value.find_first_of(" \t", start);
                    if(end == string::npos)
                    {
                        arg = value.substr(start);
                        start = value.size();
                    }
                    else
                    {
                        arg = value.substr(start, end - start);
                        start = end + 1;
                    }
                }

                DWORD findType = 0;

                tmpStore = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, 0);
                if(!tmpStore)
                {
                    throw PluginInitializationException(__FILE__, __LINE__, 
                                                    "IceSSL: error adding certificate to store:\n" + lastErrorToString());
                }

                if(field == "SUBJECT" || field == "ISSUER")
                {
                    const wstring argW = stringToWstring(arg);
                    DWORD findType = field == "SUBJECT" ? CERT_FIND_SUBJECT_STR : CERT_FIND_ISSUER_STR;
                    addMatchingCertificates(store, tmpStore, findType, argW.c_str());
                }
                else if(field == "SUBJECTDN" || field == "ISSUERDN")
                {
                    const wstring argW = stringToWstring(arg);
                    DWORD length = 0;
                    if(!CertStrToNameW(X509_ASN_ENCODING, argW.c_str(), CERT_OID_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
                                       0, 0, &length, 0))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                            "IceSSL: invalid value `" + value + "' for property `" + prop + "'\n" + 
                            lastErrorToString());
                    }

                    vector<BYTE> buffer(length);
                    if(!CertStrToNameW(X509_ASN_ENCODING, argW.c_str(), CERT_OID_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
                                       0, &buffer[0], &length, 0))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                            "IceSSL: invalid value `" + value + "' for property `" + prop + "'\n" + 
                            lastErrorToString());
                    }

                    CERT_NAME_BLOB name = { length, &buffer[0] };
                    DWORD findType = field == "SUBJECTDN" ? CERT_FIND_SUBJECT_NAME : CERT_FIND_ISSUER_NAME;
                    addMatchingCertificates(store, tmpStore, findType, &name);
                }
                else if(field == "THUMBPRINT" || field == "SUBJECTKEYID")
                {
                    vector<BYTE> buffer;
                    if(!parseBytes(arg, buffer))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                                                "IceSSL: invalid value `" + value + "' for property `" + prop + "'");
                    }

                    CRYPT_HASH_BLOB hash = { buffer.size(), &buffer[0] };
                    DWORD findType = field == "THUMBPRINT" ? CERT_FIND_HASH : CERT_FIND_KEY_IDENTIFIER;
                    addMatchingCertificates(store, tmpStore, findType, &hash);
                }
                else if(field == "SERIAL")
                {
                    vector<BYTE> buffer;
                    if(!parseBytes(arg, buffer))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                                                "IceSSL: invalid value `" + value + "' for property `" + prop + "'");
                    }
                    
                    CRYPT_INTEGER_BLOB serial = { buffer.size(), &buffer[0] };
                    PCCERT_CONTEXT next = 0;
                    do
                    {
                        if((next = CertFindCertificateInStore(store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, 
                                                              CERT_FIND_ANY, 0, next)))
                        {
                            if(CertCompareIntegerBlob(&serial, &next->pCertInfo->SerialNumber))
                            {
                                if(!CertAddCertificateContextToStore(tmpStore, next, CERT_STORE_ADD_ALWAYS, 0))
                                {
                                    throw PluginInitializationException(__FILE__, __LINE__, 
                                        "IceSSL: error adding certificate to store:\n" + lastErrorToString());
                                }
                            }
                        }
                    }
                    while(next);
                }
                CertCloseStore(store, 0);
                store = tmpStore;
            }
        }
    }
    catch(...)
    {
        if(store && store != tmpStore)
        {
            CertCloseStore(store, 0);
        }

        if(tmpStore)
        {
            CertCloseStore(tmpStore, 0);
            tmpStore = 0;
        }
        throw;
    }

    vector<PCCERT_CONTEXT> certs;
    if(store)
    {
        PCCERT_CONTEXT next = 0;
        do
        { 
            if((next = CertFindCertificateInStore(store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_ANY, 0, 
                                                  next)))
            {
                certs.push_back(next);
            }
        }
        while(next);
        stores.push_back(store);
    }
    return certs;
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
    // Protocols selects which protocols to enable.
    //
    const_cast<DWORD&>(_protocols) = parseProtocols(properties->getPropertyAsList(prefix + "Protocols"));

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
            for(int i = 0; i < supportedChipersSize; ++i)
            {
                os << "\n " << getCipherName(supportedChipers[i]);
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
                    if(CryptAcquireContextW(&cryptProv, keyProvInfo->pwszContainerName, keyProvInfo->pwszProvName, 
                                            keyProvInfo->dwProvType, 0))
                    {
                        CryptAcquireContextW(&cryptProv, keyProvInfo->pwszContainerName, keyProvInfo->pwszProvName, 
                                             keyProvInfo->dwProvType, CRYPT_DELETEKEYSET);
                    }
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
