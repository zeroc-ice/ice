// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/SChannelEngine.h>
#include <IceSSL/SChannelTransceiverI.h>
#include <IceSSL/Plugin.h>
#include <IceSSL/Util.h>

#include <Ice/LocalException.h>
#include <Ice/Logger.h>
#include <Ice/Communicator.h>
#include <Ice/StringConverter.h>

#include <IceUtil/StringUtil.h>
#include <IceUtil/FileUtil.h>
#include <Ice/UUID.h>

#include <wincrypt.h>

//
// CALG_ECDH_EPHEM algorithm constant is not defined in older version of the SDK headers
//
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa375549(v=vs.85).aspx
//

const int ICESSL_CALG_ECDH_EPHEM = 0x0000AE06;


//
// COMPILERFIX SCH_USE_STRONG_CRYPTO not defined with VC90
//
#if defined(_MSC_VER) && (_MSC_VER == 1500)
#  ifndef SCH_USE_STRONG_CRYPTO
#    define SCH_USE_STRONG_CRYPTO 0x00400000
#  endif
#endif

using namespace std;
using namespace Ice;
using namespace IceUtil;
using namespace IceUtilInternal;
using namespace IceSSL;

Shared* SChannel::upCast(SChannel::SSLEngine* p)
{
    return p;
}

namespace
{

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
                    "IceSSL: error adding certificate to store:\n" + IceUtilInternal::lastErrorToString());
            }
        }
    }
    while(next);
}

vector<PCCERT_CONTEXT>
findCertificates(const string& location, const string& name, const string& value, vector<HCERTSTORE>& stores)
{
    DWORD storeLoc;
    if(location == "CurrentUser")
    {
        storeLoc = CERT_SYSTEM_STORE_CURRENT_USER;
    }
    else
    {
        storeLoc = CERT_SYSTEM_STORE_LOCAL_MACHINE;
    }

    HCERTSTORE store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0, storeLoc, Ice::stringToWstring(name).c_str());
    if(!store)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: failed to open certificate store `" + name +
                                            "':\n" + IceUtilInternal::lastErrorToString());
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
            if(value.find(':', 0) == string::npos)
            {
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: no key in `" + value + "'");
            }
            size_t start = 0;
            size_t pos;
            while((pos = value.find(':', start)) != string::npos)
            {
                string field = IceUtilInternal::toUpper(IceUtilInternal::trim(value.substr(start, pos - start)));
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
                    throw PluginInitializationException(__FILE__, __LINE__,
                                                        "IceSSL: missing argument in `" + value + "'");
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

                tmpStore = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, 0);
                if(!tmpStore)
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                "IceSSL: error adding certificate to store:\n" + IceUtilInternal::lastErrorToString());
                }

                if(field == "SUBJECT" || field == "ISSUER")
                {
                    const wstring argW = Ice::stringToWstring(arg);
                    DWORD findType = field == "SUBJECT" ? CERT_FIND_SUBJECT_STR : CERT_FIND_ISSUER_STR;
                    addMatchingCertificates(store, tmpStore, findType, argW.c_str());
                }
                else if(field == "SUBJECTDN" || field == "ISSUERDN")
                {
                    const wstring argW = Ice::stringToWstring(arg);
                    DWORD flags[] = {
                        CERT_OID_NAME_STR,
                        CERT_OID_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
                        CERT_OID_NAME_STR | CERT_NAME_STR_FORCE_UTF8_DIR_STR_FLAG,
                        CERT_OID_NAME_STR | CERT_NAME_STR_FORCE_UTF8_DIR_STR_FLAG | CERT_NAME_STR_REVERSE_FLAG
                    };
                    for(size_t i = 0; i < sizeof(flags) / sizeof(DWORD); ++i)
                    {
                        DWORD length = 0;
                        if(!CertStrToNameW(X509_ASN_ENCODING, argW.c_str(), flags[i], 0, 0, &length, 0))
                        {
                            throw PluginInitializationException(
                                __FILE__, __LINE__,
                                "IceSSL: invalid value `" + value + "' for `IceSSL.FindCert' property:\n" +
                                IceUtilInternal::lastErrorToString());
                        }

                        vector<BYTE> buffer(length);
                        if(!CertStrToNameW(X509_ASN_ENCODING, argW.c_str(), flags[i], 0, &buffer[0], &length, 0))
                        {
                            throw PluginInitializationException(
                                __FILE__, __LINE__,
                                "IceSSL: invalid value `" + value + "' for `IceSSL.FindCert' property:\n" +
                                IceUtilInternal::lastErrorToString());
                        }

                        CERT_NAME_BLOB name = { length, &buffer[0] };

                        DWORD findType = field == "SUBJECTDN" ? CERT_FIND_SUBJECT_NAME : CERT_FIND_ISSUER_NAME;
                        addMatchingCertificates(store, tmpStore, findType, &name);
                    }
                }
                else if(field == "THUMBPRINT" || field == "SUBJECTKEYID")
                {
                    vector<BYTE> buffer;
                    if(!parseBytes(arg, buffer))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: invalid `IceSSL.FindCert' property: can't decode the value");
                    }

                    CRYPT_HASH_BLOB hash = { static_cast<DWORD>(buffer.size()), &buffer[0] };
                    DWORD findType = field == "THUMBPRINT" ? CERT_FIND_HASH : CERT_FIND_KEY_IDENTIFIER;
                    addMatchingCertificates(store, tmpStore, findType, &hash);
                }
                else if(field == "SERIAL")
                {
                    vector<BYTE> buffer;
                    if(!parseBytes(arg, buffer))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: invalid value `" + value + "' for `IceSSL.FindCert' property");
                    }

                    CRYPT_INTEGER_BLOB serial = { static_cast<DWORD>(buffer.size()), &buffer[0] };
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
                                                                    "IceSSL: error adding certificate to store:\n" +
                                                                    IceUtilInternal::lastErrorToString());
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


#if defined(__MINGW32__) || (defined(_MSC_VER) && (_MSC_VER <= 1500))
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

#endif

void
addCertificatesToStore(const string& file, HCERTSTORE store, PCCERT_CONTEXT* cert = 0)
{
    vector<char> buffer;
    readFile(file, buffer);
    if(buffer.empty())
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: certificate file is empty:\n" + file);
    }

    string strbuf(buffer.begin(), buffer.end());
    string::size_type size, startpos, endpos = 0;
    bool first = true;
    while(true)
    {
        startpos = strbuf.find("-----BEGIN CERTIFICATE-----", endpos);
        if(startpos != string::npos)
        {
            endpos = strbuf.find("-----END CERTIFICATE-----", startpos);
            size = endpos - startpos + sizeof("-----END CERTIFICATE-----");
        }
        else if(first)
        {
            startpos = 0;
            endpos = string::npos;
            size = strbuf.size();
        }
        else
        {
            break;
        }

        vector<BYTE> outBuffer;
        outBuffer.resize(size);
        DWORD outLength = static_cast<DWORD>(outBuffer.size());
        if(!CryptStringToBinary(&buffer[startpos], static_cast<DWORD>(size), CRYPT_STRING_ANY, &outBuffer[0],
                                &outLength, 0, 0))
        {
            assert(GetLastError() != ERROR_MORE_DATA); // Base64 data should always be bigger than binary
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: error decoding certificate:\n" + lastErrorToString());
        }

        if(!CertAddEncodedCertificateToStore(store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, &outBuffer[0],
                                             outLength, CERT_STORE_ADD_NEW, first ? cert : 0))
        {
            if(GetLastError() != static_cast<DWORD>(CRYPT_E_EXISTS))
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                                    "IceSSL: error decoding certificate:\n" + lastErrorToString());
            }
        }

        first = false;
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
    else if(name == "3DES_112")
    {
        return CALG_3DES_112;
    } 
    else if(name == "AES")
    {
        return CALG_AES;
    } 
    else if(name == "AES_128")
    {
        return CALG_AES_128;
    } 
    else if(name == "AES_192")
    {
        return CALG_AES_192;
    } 
    else if(name == "AES_256")
    {
        return CALG_AES_256;
    } 
    else if(name == "AGREEDKEY_ANY")
    {
        return CALG_AGREEDKEY_ANY;
    } 
    else if(name == "CYLINK_MEK")
    {
        return CALG_CYLINK_MEK;
    } 
    else if(name == "DES")
    {
        return CALG_DES;
    } 
    else if(name == "DESX")
    {
        return CALG_DESX;
    } 
    else if(name == "DH_EPHEM")
    {
        return CALG_DH_EPHEM;
    } 
    else if(name == "DH_SF")
    {
        return CALG_DH_SF;
    } 
    else if(name == "DSS_SIGN")
    {
        return CALG_DSS_SIGN;
    } 
    else if(name == "ECDH")
    {
        return CALG_ECDH;
    } 
    else if(name == "ECDH_EPHEM")
    {
        return ICESSL_CALG_ECDH_EPHEM;
    } 
    else if(name == "ECDSA")
    {
        return CALG_ECDSA;
    } 
    else if(name == "HASH_REPLACE_OWF")
    {
        return CALG_HASH_REPLACE_OWF;
    } 
    else if(name == "HUGHES_MD5")
    {
        return CALG_HUGHES_MD5;
    } 
    else if(name == "HMAC")
    {
        return CALG_HMAC;
    } 
    else if(name == "MAC")
    {
        return CALG_MAC;
    } 
    else if(name == "MD2")
    {
        return CALG_MD2;
    } 
    else if(name == "MD4")
    {
        return CALG_MD4;
    } 
    else if(name == "MD5")
    {
        return CALG_MD5;
    } 
    else if(name == "NO_SIGN")
    {
        return CALG_NO_SIGN;
    } 
    else if(name == "RC2")
    {
        return CALG_RC2;
    } 
    else if(name == "RC4")
    {
        return CALG_RC4;
    } 
    else if(name == "RC5")
    {
        return CALG_RC5;
    } 
    else if(name == "RSA_KEYX")
    {
        return CALG_RSA_KEYX;
    } 
    else if(name == "RSA_SIGN")
    {
        return CALG_RSA_SIGN;
    }
    else if(name == "SHA1")
    {
        return CALG_SHA1;
    } 
    else if(name == "SHA_256")
    {
        return CALG_SHA_256;
    } 
    else if(name == "SHA_384")
    {
        return CALG_SHA_384;
    } 
    else if(name == "SHA_512")
    {
        return CALG_SHA_512;
    } 
    return 0;
}

}

SChannel::SSLEngine::SSLEngine(const CommunicatorPtr& communicator) :
    IceSSL::SSLEngine(communicator),
    _rootStore(0),
    _chainEngine(0),
    _strongCrypto(false)
{
}

void
SChannel::SSLEngine::initialize()
{
    Mutex::Lock lock(_mutex);
    if(_initialized)
    {
        return;
    }

    IceSSL::SSLEngine::initialize();

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

    const_cast<bool&>(_strongCrypto) = properties->getPropertyAsIntWithDefault(prefix + "SchannelStrongCrypto", 0) > 0;

    //
    // Check for a default directory. We look in this directory for
    // files mentioned in the configuration.
    //
    const string defaultDir = properties->getProperty(prefix + "DefaultDir");

    const int passwordRetryMax = properties->getPropertyAsIntWithDefault(prefix + "PasswordRetryMax", 3);
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

    string certStoreLocation = properties->getPropertyWithDefault(prefix + "CertStoreLocation", "CurrentUser");
    if(certStoreLocation != "CurrentUser" && certStoreLocation != "LocalMachine")
    {
        getLogger()->warning("invalid IceSSL.CertStoreLocation value `" + certStoreLocation +
                             "' adjusted to `CurrentUser'");
        certStoreLocation = "CurrentUser";
    }

    //
    // Create trusted CA store with contents of CertAuthFile
    //
    string caFile = properties->getProperty(prefix + "CAs");
    if(caFile.empty())
    {
        caFile = properties->getProperty(prefix + "CertAuthFile");
    }
    if(!caFile.empty() || properties->getPropertyAsInt("IceSSL.UsePlatformCAs") <= 0)
    {
        _rootStore = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, 0);
        if(!_rootStore)
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: error creating in memory certificate store:\n" +
                                                lastErrorToString());
        }
    }
    if(!caFile.empty())
    {
        string resolved;
        if(!checkPath(caFile, defaultDir, false, resolved))
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: CA certificate file not found:\n" + caFile);
        }

        addCertificatesToStore(resolved, _rootStore);
    }

    if(_rootStore)
    {
        //
        // Create a chain engine that uses our Trusted Root Store
        //
#if defined(__MINGW32__) || (defined(_MSC_VER) && (_MSC_VER <= 1500))
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
        if(certStoreLocation == "LocalMachine")
        {
            config.dwFlags = CERT_CHAIN_USE_LOCAL_MACHINE_STORE;
        }

#if defined(__MINGW32__) || (defined(_MSC_VER) && (_MSC_VER <= 1500))
        if(!CertCreateCertificateChainEngine(reinterpret_cast<CERT_CHAIN_ENGINE_CONFIG*>(&config), &_chainEngine))
#else
        if(!CertCreateCertificateChainEngine(&config, &_chainEngine))
#endif
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: error creating certificate chain engine:\n" +
                                                lastErrorToString());
        }
    }
    else
    {
        _chainEngine = (certStoreLocation == "LocalMachine") ? HCCE_LOCAL_MACHINE : HCCE_CURRENT_USER;
    }

    string certFile = properties->getProperty(prefix + "CertFile");
    string keyFile = properties->getProperty(prefix + "KeyFile");
    string findCert = properties->getProperty("IceSSL.FindCert");

    if(!certFile.empty())
    {
        vector<string> certFiles;
        if(!splitString(certFile, IceUtilInternal::pathsep, certFiles) || certFiles.size() > 2)
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: invalid value for " + prefix + "CertFile:\n" + certFile);
        }

        vector<string> keyFiles;
        if(!keyFile.empty())
        {
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
        }

        for(size_t i = 0; i < certFiles.size(); ++i)
        {
            string certFile = certFiles[i];
            string resolved;
            if(!checkPath(certFile, defaultDir, false, resolved))
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                                    "IceSSL: certificate file not found:\n" + certFile);
            }
            certFile = resolved;

            vector<char> buffer;
            readFile(certFile, buffer);
            if(buffer.empty())
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                                    "IceSSL: certificate file is empty:\n" + certFile);
            }

            CRYPT_DATA_BLOB pfxBlob;
            pfxBlob.cbData = static_cast<DWORD>(buffer.size());
            pfxBlob.pbData = reinterpret_cast<BYTE*>(&buffer[0]);

            HCERTSTORE store = 0;
            PCCERT_CONTEXT cert = 0;
            int err = 0;
            int count = 0;
            DWORD importFlags = (certStoreLocation == "LocalMachine") ? CRYPT_MACHINE_KEYSET : CRYPT_USER_KEYSET;
            do
            {
                string s = password(false);
                store = PFXImportCertStore(&pfxBlob, Ice::stringToWstring(s).c_str(), importFlags);
                err = store ? 0 : GetLastError();
            }
            while(err == ERROR_INVALID_PASSWORD && passwordPrompt && ++count < passwordRetryMax);

            if(store)
            {
                //
                // Try to find a certificate chain.
                //
                CERT_CHAIN_FIND_BY_ISSUER_PARA para;
                memset(&para, 0, sizeof(CERT_CHAIN_FIND_BY_ISSUER_PARA));
                para.cbSize = sizeof(CERT_CHAIN_FIND_BY_ISSUER_PARA);

                DWORD ff = CERT_CHAIN_FIND_BY_ISSUER_CACHE_ONLY_URL_FLAG; // Don't fetch anything from the Internet
                PCCERT_CHAIN_CONTEXT chain = 0;
                while(!cert)
                {
                    chain = CertFindChainInStore(store, X509_ASN_ENCODING, ff, CERT_CHAIN_FIND_BY_ISSUER, &para, chain);
                    if(!chain)
                    {
                        break; // No more chains found in the store.
                    }

                    if(chain->cChain > 0 && chain->rgpChain[0]->cElement > 0)
                    {
                        cert = CertDuplicateCertificateContext(chain->rgpChain[0]->rgpElement[0]->pCertContext);
                    }
                    CertFreeCertificateChain(chain);
                }

                //
                // Check if we can find a certificate if we couldn't find a chain.
                //
                if(!cert)
                {
                    cert = CertFindCertificateInStore(store, X509_ASN_ENCODING, 0, CERT_FIND_ANY, 0, cert);
                }
                if(!cert)
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                                        "IceSSL: certificate error:\n" + lastErrorToString());
                }
                _allCerts.push_back(cert);
                _stores.push_back(store);
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
            if(keyFiles.empty())
            {
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: no key file specified");
            }

            err = 0;
            keyFile = keyFiles[i];
            if(!checkPath(keyFile, defaultDir, false, resolved))
            {
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: key file not found:\n" + keyFile);
            }
            keyFile = resolved;

            readFile(keyFile, buffer);
            if(buffer.empty())
            {
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: key file is empty:\n" + keyFile);
            }

            vector<BYTE> outBuffer;
            outBuffer.resize(buffer.size());
            DWORD outLength = static_cast<DWORD>(buffer.size());

            //
            // Convert the PEM encoded buffer to DER binary format.
            //
            if(!CryptStringToBinary(&buffer[0], static_cast<DWORD>(buffer.size()), CRYPT_STRING_BASE64HEADER,
                                    &outBuffer[0], &outLength, 0, 0))
            {
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: error decoding key `" + keyFile +
                                                    "':\n" + lastErrorToString());
            }

            PCRYPT_PRIVATE_KEY_INFO keyInfo = 0;
            BYTE* key = 0;
            HCRYPTKEY hKey = 0;
            try
            {
                //
                // First try to decode as a PKCS#8 key, if that fails try PKCS#1.
                //
                DWORD decodedLength = 0;
                if(CryptDecodeObjectEx(X509_ASN_ENCODING, PKCS_PRIVATE_KEY_INFO, &outBuffer[0], outLength,
                                       CRYPT_DECODE_ALLOC_FLAG, 0, &keyInfo, &decodedLength))
                {
                    //
                    // Check that we are using a RSA Key
                    //
                    if(strcmp(keyInfo->Algorithm.pszObjId, szOID_RSA_RSA))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__,
                                                        string("IceSSL: error unknow key algorithm: `") +
                                                            keyInfo->Algorithm.pszObjId + "'");
                    }

                    //
                    // Decode the private key BLOB
                    //
                    if(!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RSA_PRIVATE_KEY,
                                            keyInfo->PrivateKey.pbData, keyInfo->PrivateKey.cbData,
                                            CRYPT_DECODE_ALLOC_FLAG, 0, &key, &outLength))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: error decoding key `" +
                                                            keyFile + "':\n" + lastErrorToString());
                    }
                    LocalFree(keyInfo);
                    keyInfo = 0;
                }
                else
                {
                    //
                    // Decode the private key BLOB
                    //
                    if(!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RSA_PRIVATE_KEY,
                                            &outBuffer[0], outLength, CRYPT_DECODE_ALLOC_FLAG, 0, &key, &outLength))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: error decoding key `" +
                                                            keyFile + "':\n" + lastErrorToString());
                    }
                }

                //
                // Create a new RSA key set to store our key
                //
                const wstring keySetName = Ice::stringToWstring(generateUUID());
                HCRYPTPROV cryptProv = 0;

                DWORD contextFlags = CRYPT_NEWKEYSET;
                if(certStoreLocation == "LocalMachine")
                {
                    contextFlags |= CRYPT_MACHINE_KEYSET;
                }                                                                   ;

                if(!CryptAcquireContextW(&cryptProv, keySetName.c_str(), MS_ENHANCED_PROV_W, PROV_RSA_FULL,
                                         contextFlags))
                {
                    throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: error acquiring cryptographic "
                                                        "context:\n" + lastErrorToString());
                }

                //
                // Import the private key
                //
                if(!CryptImportKey(cryptProv, key, outLength, 0, 0, &hKey))
                {
                    throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: error importing key `" + keyFile +
                                                        "':\n" + lastErrorToString());
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
                    throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: error creating certificate "
                                                        "store:\n" + lastErrorToString());
                }

                addCertificatesToStore(certFile, store, &cert);

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
                    throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: error seting certificate "
                                                        "property:\n" + lastErrorToString());
                }

                _importedCerts.push_back(cert);
                _allCerts.push_back(cert);
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
    }
    else if(!findCert.empty())
    {
        string certStore = properties->getPropertyWithDefault(prefix + "CertStore", "My");
        vector<PCCERT_CONTEXT> certs = findCertificates(certStoreLocation, certStore, findCert, _stores);
        if(certs.empty())
        {
            throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: no certificates found");
        }
        _allCerts.insert(_allCerts.end(), certs.begin(), certs.end());
    }
    _initialized = true;
}

string
SChannel::SSLEngine::getCipherName(ALG_ID cipher) const
{
    switch(cipher)
    {
        case CALG_3DES: 
            return "3DES";
        case CALG_3DES_112: 
            return "3DES_112";
        case CALG_AES: 
            return "AES";
        case CALG_AES_128: 
            return "AES_128";
        case CALG_AES_192: 
            return "AES_192";
        case CALG_AES_256: 
            return "AES_256";
        case CALG_AGREEDKEY_ANY: 
            return "AGREEDKEY_ANY";
        case CALG_CYLINK_MEK: 
            return "CYLINK_MEK";
        case CALG_DES: 
            return "DES";
        case CALG_DESX: 
            return "DESX";
        case CALG_DH_EPHEM: 
            return "DH_EPHEM";
        case CALG_DH_SF: 
            return "DH_SF";
        case CALG_DSS_SIGN: 
            return "DSS_SIGN";
        case CALG_ECDH: 
            return "ECDH";
        case ICESSL_CALG_ECDH_EPHEM: 
            return "ECDH_EPHEM";
        case CALG_ECDSA: 
            return "ECDSA";
        case CALG_HASH_REPLACE_OWF: 
            return "HASH_REPLACE_OWF";
        case CALG_HUGHES_MD5: 
            return "HUGHES_MD5";
        case CALG_HMAC: 
            return "HMAC";
        case CALG_MAC: 
            return "MAC";
        case CALG_MD2: 
            return "MD2";
        case CALG_MD4: 
            return "MD4";
        case CALG_MD5: 
            return "MD5";
        case CALG_NO_SIGN: 
            return "NO_SIGN";
        case CALG_RC2: 
            return "RC2";
        case CALG_RC4: 
            return "RC4";
        case CALG_RC5: 
            return "RC5";
        case CALG_RSA_KEYX: 
            return "RSA_KEYX";
        case CALG_RSA_SIGN: 
            return "RSA_SIGN";
        case CALG_SHA1: 
            return "SHA1";
        case CALG_SHA_256: 
            return "SHA_256";
        case CALG_SHA_384: 
            return "SHA_384";
        case CALG_SHA_512: 
            return "SHA_512";
        default:
            return "Unknown";
    }
}

CredHandle
SChannel::SSLEngine::newCredentialsHandle(bool incoming)
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

        //
        // There's no way to prevent SChannel from sending "CA names" to the
        // client. Recent Windows versions don't CA names but older ones do
        // send all the trusted root CA names. We provide the root store to
        // ensure that for these older Windows versions, we also include the
        // CA names of our trusted roots.
        //
        cred.hRootStore = _rootStore;
    }
    else
    {
        cred.dwFlags = SCH_CRED_MANUAL_CRED_VALIDATION | SCH_CRED_NO_SERVERNAME_CHECK | SCH_CRED_NO_DEFAULT_CREDS;
    }

    if(_strongCrypto)
    {
        cred.dwFlags |= SCH_USE_STRONG_CRYPTO;
    }

    if(!_ciphers.empty())
    {
        cred.cSupportedAlgs = static_cast<DWORD>(_ciphers.size());
        cred.palgSupportedAlgs = &_ciphers[0];
    }

    CredHandle credHandle;
    memset(&credHandle, 0, sizeof(credHandle));

    SECURITY_STATUS err = AcquireCredentialsHandle(0, const_cast<char*>(UNISP_NAME),
                                                   (incoming ? SECPKG_CRED_INBOUND : SECPKG_CRED_OUTBOUND),
                                                   0, &cred, 0, 0, &credHandle, 0);

    if(err != SEC_E_OK)
    {
        throw SecurityException(__FILE__, __LINE__,
                        "IceSSL: failed to acquire credentials handle:\n" + lastErrorToString());
    }
    return credHandle;
}

HCERTCHAINENGINE
SChannel::SSLEngine::chainEngine() const
{
    return _chainEngine;
}

void
SChannel::SSLEngine::parseCiphers(const std::string& ciphers)
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
SChannel::SSLEngine::destroy()
{
    if(_chainEngine && _chainEngine != HCCE_CURRENT_USER && _chainEngine != HCCE_LOCAL_MACHINE)
    {
        CertFreeCertificateChainEngine(_chainEngine);
    }

    if(_rootStore)
    {
        CertCloseStore(_rootStore, 0);
    }

    for(vector<PCCERT_CONTEXT>::const_iterator i = _importedCerts.begin(); i != _importedCerts.end(); ++i)
    {
        //
        // Retrieve the certificate CERT_KEY_PROV_INFO_PROP_ID property, we use the CRYPT_KEY_PROV_INFO
        // data to remove the key set associated with the certificate.
        //
        DWORD length = 0;
        if(!CertGetCertificateContextProperty(*i, CERT_KEY_PROV_INFO_PROP_ID, 0, &length))
        {
            continue;
        }
        vector<char> buf(length);
        if(!CertGetCertificateContextProperty(*i, CERT_KEY_PROV_INFO_PROP_ID, &buf[0], &length))
        {
            continue;
        }
        CRYPT_KEY_PROV_INFO* key = reinterpret_cast<CRYPT_KEY_PROV_INFO*>(&buf[0]);
        HCRYPTPROV prov = 0;
        CryptAcquireContextW(&prov, key->pwszContainerName, key->pwszProvName, key->dwProvType, CRYPT_DELETEKEYSET);
    }

    for(vector<PCCERT_CONTEXT>::const_iterator i = _allCerts.begin(); i != _allCerts.end(); ++i)
    {
        CertFreeCertificateContext(*i);
    }

    for(vector<HCERTSTORE>::const_iterator i = _stores.begin(); i != _stores.end(); ++i)
    {
        CertCloseStore(*i, 0);
    }
}

void
SChannel::SSLEngine::verifyPeer(const string& address, const IceSSL::ConnectionInfoPtr& info, const string& desc)
{
    verifyPeerCertName(address, info);
    IceSSL::SSLEngine::verifyPeer(address, info, desc);
}

IceInternal::TransceiverPtr
SChannel::SSLEngine::createTransceiver(const InstancePtr& instance,
                                       const IceInternal::TransceiverPtr& delegate,
                                       const string& hostOrAdapterName,
                                       bool incoming)
{
    return new SChannel::TransceiverI(instance, delegate, hostOrAdapterName, incoming);
}
