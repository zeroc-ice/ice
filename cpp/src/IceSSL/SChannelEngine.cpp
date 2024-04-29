//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "SChannelEngine.h"
#include "Ice/Communicator.h"
#include "Ice/LocalException.h"
#include "Ice/Logger.h"
#include "Ice/StringConverter.h"
#include "Ice/UUID.h"
#include "IceUtil/FileUtil.h"
#include "IceUtil/StringUtil.h"
#include "SChannelTransceiverI.h"
#include "SSLUtil.h"

#include <wincrypt.h>

#include <iostream>
#include <mutex>

//
// SP_PROT_TLS1_3 is new in v10.0.15021 SDK
//
#ifndef SP_PROT_TLS1_3
#    define SP_PROT_TLS1_3_SERVER 0x00001000
#    define SP_PROT_TLS1_3_CLIENT 0x00002000
#    define SP_PROT_TLS1_3 (SP_PROT_TLS1_3_SERVER | SP_PROT_TLS1_3_CLIENT)
#endif

#ifndef SECURITY_FLAG_IGNORE_CERT_CN_INVALID
#    define SECURITY_FLAG_IGNORE_CERT_CN_INVALID 0x00001000
#endif

//
// CALG_ECDH_EPHEM algorithm constant is not defined in older version of the SDK headers
//
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa375549(v=vs.85).aspx
//

const int ICESSL_CALG_ECDH_EPHEM = 0x0000AE06;

using namespace std;
using namespace Ice;
using namespace IceUtil;
using namespace IceUtilInternal;
using namespace IceSSL;

namespace
{
    mutex globalMutex;

    void addMatchingCertificates(HCERTSTORE source, HCERTSTORE target, DWORD findType, const void* findParam)
    {
        PCCERT_CONTEXT next = 0;
        do
        {
            if ((next = CertFindCertificateInStore(
                     source,
                     X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                     0,
                     findType,
                     findParam,
                     next)) != 0)
            {
                if (!CertAddCertificateContextToStore(target, next, CERT_STORE_ADD_ALWAYS, 0))
                {
                    throw InitializationException(
                        __FILE__,
                        __LINE__,
                        "IceSSL: error adding certificate to store:\n" + IceUtilInternal::lastErrorToString());
                }
            }
        } while (next);
    }

    vector<PCCERT_CONTEXT>
    findCertificates(const string& location, const string& storeName, const string& value, vector<HCERTSTORE>& stores)
    {
        HCERTSTORE store = CertOpenStore(
            CERT_STORE_PROV_SYSTEM,
            0,
            0,
            location == "CurrentUser" ? CERT_SYSTEM_STORE_CURRENT_USER : CERT_SYSTEM_STORE_LOCAL_MACHINE,
            Ice::stringToWstring(storeName).c_str());

        if (!store)
        {
            throw InitializationException(
                __FILE__,
                __LINE__,
                "IceSSL: failed to open certificate store `" + storeName + "':\n" +
                    IceUtilInternal::lastErrorToString());
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
            if (value != "*")
            {
                if (value.find(':', 0) == string::npos)
                {
                    throw InitializationException(__FILE__, __LINE__, "IceSSL: no key in `" + value + "'");
                }
                size_t start = 0;
                size_t pos;
                while ((pos = value.find(':', start)) != string::npos)
                {
                    string field = IceUtilInternal::toUpper(IceUtilInternal::trim(value.substr(start, pos - start)));
                    if (field != "SUBJECT" && field != "SUBJECTDN" && field != "ISSUER" && field != "ISSUERDN" &&
                        field != "THUMBPRINT" && field != "SUBJECTKEYID" && field != "SERIAL")
                    {
                        throw InitializationException(__FILE__, __LINE__, "IceSSL: unknown key in `" + value + "'");
                    }

                    start = pos + 1;
                    while (start < value.size() && (value[start] == ' ' || value[start] == '\t'))
                    {
                        ++start;
                    }

                    if (start == value.size())
                    {
                        throw InitializationException(
                            __FILE__,
                            __LINE__,
                            "IceSSL: missing argument in `" + value + "'");
                    }

                    string arg;
                    if (value[start] == '"' || value[start] == '\'')
                    {
                        size_t end = start;
                        ++end;
                        while (end < value.size())
                        {
                            if (value[end] == value[start] && value[end - 1] != '\\')
                            {
                                break;
                            }
                            ++end;
                        }
                        if (end == value.size() || value[end] != value[start])
                        {
                            throw InitializationException(
                                __FILE__,
                                __LINE__,
                                "IceSSL: unmatched quote in `" + value + "'");
                        }
                        ++start;
                        arg = value.substr(start, end - start);
                        start = end + 1;
                    }
                    else
                    {
                        size_t end = value.find_first_of(" \t", start);
                        if (end == string::npos)
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
                    if (!tmpStore)
                    {
                        throw InitializationException(
                            __FILE__,
                            __LINE__,
                            "IceSSL: error adding certificate to store:\n" + IceUtilInternal::lastErrorToString());
                    }

                    if (field == "SUBJECT" || field == "ISSUER")
                    {
                        const wstring argW = Ice::stringToWstring(arg);
                        DWORD findType = field == "SUBJECT" ? CERT_FIND_SUBJECT_STR : CERT_FIND_ISSUER_STR;
                        addMatchingCertificates(store, tmpStore, findType, argW.c_str());
                    }
                    else if (field == "SUBJECTDN" || field == "ISSUERDN")
                    {
                        const wstring argW = Ice::stringToWstring(arg);
                        DWORD flags[] = {
                            CERT_OID_NAME_STR,
                            CERT_OID_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
                            CERT_OID_NAME_STR | CERT_NAME_STR_FORCE_UTF8_DIR_STR_FLAG,
                            CERT_OID_NAME_STR | CERT_NAME_STR_FORCE_UTF8_DIR_STR_FLAG | CERT_NAME_STR_REVERSE_FLAG};
                        for (size_t i = 0; i < sizeof(flags) / sizeof(DWORD); ++i)
                        {
                            DWORD length = 0;
                            if (!CertStrToNameW(X509_ASN_ENCODING, argW.c_str(), flags[i], 0, 0, &length, 0))
                            {
                                throw InitializationException(
                                    __FILE__,
                                    __LINE__,
                                    "IceSSL: invalid value `" + value + "' for `IceSSL.FindCert' property:\n" +
                                        IceUtilInternal::lastErrorToString());
                            }

                            vector<BYTE> buffer(length);
                            if (!CertStrToNameW(X509_ASN_ENCODING, argW.c_str(), flags[i], 0, &buffer[0], &length, 0))
                            {
                                throw InitializationException(
                                    __FILE__,
                                    __LINE__,
                                    "IceSSL: invalid value `" + value + "' for `IceSSL.FindCert' property:\n" +
                                        IceUtilInternal::lastErrorToString());
                            }

                            CERT_NAME_BLOB name = {length, &buffer[0]};

                            DWORD findType = field == "SUBJECTDN" ? CERT_FIND_SUBJECT_NAME : CERT_FIND_ISSUER_NAME;
                            addMatchingCertificates(store, tmpStore, findType, &name);
                        }
                    }
                    else if (field == "THUMBPRINT" || field == "SUBJECTKEYID")
                    {
                        vector<BYTE> buffer;
                        if (!parseBytes(arg, buffer))
                        {
                            throw InitializationException(
                                __FILE__,
                                __LINE__,
                                "IceSSL: invalid `IceSSL.FindCert' property: can't decode the value");
                        }

                        CRYPT_HASH_BLOB hash = {static_cast<DWORD>(buffer.size()), &buffer[0]};
                        DWORD findType = field == "THUMBPRINT" ? CERT_FIND_HASH : CERT_FIND_KEY_IDENTIFIER;
                        addMatchingCertificates(store, tmpStore, findType, &hash);
                    }
                    else if (field == "SERIAL")
                    {
                        vector<BYTE> buffer;
                        if (!parseBytes(arg, buffer))
                        {
                            throw InitializationException(
                                __FILE__,
                                __LINE__,
                                "IceSSL: invalid value `" + value + "' for `IceSSL.FindCert' property");
                        }

                        CRYPT_INTEGER_BLOB serial = {static_cast<DWORD>(buffer.size()), &buffer[0]};
                        PCCERT_CONTEXT next = 0;
                        do
                        {
                            if ((next = CertFindCertificateInStore(
                                     store,
                                     X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                     0,
                                     CERT_FIND_ANY,
                                     0,
                                     next)) != 0)
                            {
                                if (CertCompareIntegerBlob(&serial, &next->pCertInfo->SerialNumber))
                                {
                                    if (!CertAddCertificateContextToStore(tmpStore, next, CERT_STORE_ADD_ALWAYS, 0))
                                    {
                                        throw InitializationException(
                                            __FILE__,
                                            __LINE__,
                                            "IceSSL: error adding certificate to store:\n" +
                                                IceUtilInternal::lastErrorToString());
                                    }
                                }
                            }
                        } while (next);
                    }
                    CertCloseStore(store, 0);
                    store = tmpStore;
                }
            }
        }
        catch (...)
        {
            if (store && store != tmpStore)
            {
                CertCloseStore(store, 0);
            }

            if (tmpStore)
            {
                CertCloseStore(tmpStore, 0);
                tmpStore = 0;
            }
            throw;
        }

        vector<PCCERT_CONTEXT> certs;
        if (store)
        {
            PCCERT_CONTEXT next = 0;
            do
            {
                if ((next = CertFindCertificateInStore(
                         store,
                         X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                         0,
                         CERT_FIND_ANY,
                         0,
                         next)) != 0)
                {
                    certs.push_back(next);
                }
            } while (next);
            stores.push_back(store);
        }
        return certs;
    }

    void addCertificatesToStore(const string& file, HCERTSTORE store, PCCERT_CONTEXT* cert = 0)
    {
        vector<char> buffer;
        readFile(file, buffer);
        if (buffer.empty())
        {
            throw InitializationException(__FILE__, __LINE__, "IceSSL: certificate file is empty:\n" + file);
        }

        string strbuf(buffer.begin(), buffer.end());
        string::size_type size, startpos, endpos = 0;
        bool first = true;
        while (true)
        {
            startpos = strbuf.find("-----BEGIN CERTIFICATE-----", endpos);
            if (startpos != string::npos)
            {
                endpos = strbuf.find("-----END CERTIFICATE-----", startpos);
                size = endpos - startpos + sizeof("-----END CERTIFICATE-----");
            }
            else if (first)
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
            if (!CryptStringToBinary(
                    &buffer[startpos],
                    static_cast<DWORD>(size),
                    CRYPT_STRING_ANY,
                    &outBuffer[0],
                    &outLength,
                    0,
                    0))
            {
                assert(GetLastError() != ERROR_MORE_DATA); // Base64 data should always be bigger than binary
                throw InitializationException(
                    __FILE__,
                    __LINE__,
                    "IceSSL: error decoding certificate:\n" + lastErrorToString());
            }

            if (!CertAddEncodedCertificateToStore(
                    store,
                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                    &outBuffer[0],
                    outLength,
                    CERT_STORE_ADD_NEW,
                    first ? cert : 0))
            {
                if (GetLastError() != static_cast<DWORD>(CRYPT_E_EXISTS))
                {
                    throw InitializationException(
                        __FILE__,
                        __LINE__,
                        "IceSSL: error decoding certificate:\n" + lastErrorToString());
                }
            }

            first = false;
        }
    }
}

SChannel::SSLEngine::SSLEngine(const IceInternal::InstancePtr& instance)
    : IceSSL::SSLEngine(instance),
      _rootStore(0),
      _chainEngine(0),
      _strongCrypto(false),
      _clientCredentials({0, 0}),
      _serverCredentials({0, 0})
{
}

void
SChannel::SSLEngine::initialize()
{
    //
    // BUGFIX: we use a global mutex for the initialization of SChannel to
    // avoid crashes occurring with last SChannel updates see:
    // https://github.com/zeroc-ice/ice/issues/242
    //
    lock_guard globalLock(globalMutex);

    IceSSL::SSLEngine::initialize();

    const string prefix = "IceSSL.";
    const PropertiesPtr properties = getProperties();

    const_cast<bool&>(_strongCrypto) = properties->getPropertyAsIntWithDefault(prefix + "SchannelStrongCrypto", 0) > 0;

    // Check for a default directory. We look in this directory for files mentioned in the configuration.
    const string defaultDir = properties->getProperty(prefix + "DefaultDir");

    string certStoreLocation = properties->getPropertyWithDefault(prefix + "CertStoreLocation", "CurrentUser");
    if (certStoreLocation != "CurrentUser" && certStoreLocation != "LocalMachine")
    {
        getLogger()->warning(
            "invalid IceSSL.CertStoreLocation value `" + certStoreLocation + "' adjusted to `CurrentUser'");
        certStoreLocation = "CurrentUser";
    }

    //
    // Create trusted CA store with contents of IceSSL.CAs
    //
    string caFile = properties->getProperty(prefix + "CAs");
    if (!caFile.empty() || properties->getPropertyAsInt("IceSSL.UsePlatformCAs") <= 0)
    {
        _rootStore = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, 0);
        if (!_rootStore)
        {
            throw InitializationException(
                __FILE__,
                __LINE__,
                "IceSSL: error creating in memory certificate store:\n" + lastErrorToString());
        }
    }
    if (!caFile.empty())
    {
        string resolved;
        if (!checkPath(caFile, defaultDir, false, resolved))
        {
            throw InitializationException(__FILE__, __LINE__, "IceSSL: CA certificate file not found:\n" + caFile);
        }

        addCertificatesToStore(resolved, _rootStore);
    }

    if (_rootStore)
    {
        //
        // Create a chain engine that uses our Trusted Root Store
        //
        CERT_CHAIN_ENGINE_CONFIG config;
        memset(&config, 0, sizeof(CERT_CHAIN_ENGINE_CONFIG));
        config.cbSize = sizeof(CERT_CHAIN_ENGINE_CONFIG);
        config.hExclusiveRoot = _rootStore;

        //
        // Build the chain using the LocalMachine registry location as opposed
        // to the CurrentUser location.
        //
        if (certStoreLocation == "LocalMachine")
        {
            config.dwFlags = CERT_CHAIN_USE_LOCAL_MACHINE_STORE;
        }

        if (!CertCreateCertificateChainEngine(&config, &_chainEngine))
        {
            throw InitializationException(
                __FILE__,
                __LINE__,
                "IceSSL: error creating certificate chain engine:\n" + lastErrorToString());
        }
    }
    else
    {
        _chainEngine = (certStoreLocation == "LocalMachine") ? HCCE_LOCAL_MACHINE : HCCE_CURRENT_USER;
    }

    string certFileValue = properties->getProperty(prefix + "CertFile");
    string keyFile = properties->getProperty(prefix + "KeyFile");
    string findCert = properties->getProperty("IceSSL.FindCert");

    if (!certFileValue.empty())
    {
        vector<string> certFiles;
        if (!splitString(certFileValue, IceUtilInternal::pathsep, certFiles) || certFiles.size() > 2)
        {
            throw InitializationException(
                __FILE__,
                __LINE__,
                "IceSSL: invalid value for " + prefix + "CertFile:\n" + certFileValue);
        }

        vector<string> keyFiles;
        if (!keyFile.empty())
        {
            if (!splitString(keyFile, IceUtilInternal::pathsep, keyFiles) || keyFiles.size() > 2)
            {
                throw InitializationException(
                    __FILE__,
                    __LINE__,
                    "IceSSL: invalid value for " + prefix + "KeyFile:\n" + keyFile);
            }

            if (certFiles.size() != keyFiles.size())
            {
                throw InitializationException(
                    __FILE__,
                    __LINE__,
                    "IceSSL: " + prefix + "KeyFile does not agree with " + prefix + "CertFile");
            }
        }

        for (size_t i = 0; i < certFiles.size(); ++i)
        {
            string certFile = certFiles[i];
            string resolved;
            if (!checkPath(certFile, defaultDir, false, resolved))
            {
                throw InitializationException(__FILE__, __LINE__, "IceSSL: certificate file not found:\n" + certFile);
            }
            certFile = resolved;

            vector<char> buffer;
            readFile(certFile, buffer);
            if (buffer.empty())
            {
                throw InitializationException(__FILE__, __LINE__, "IceSSL: certificate file is empty:\n" + certFile);
            }

            CRYPT_DATA_BLOB pfxBlob;
            pfxBlob.cbData = static_cast<DWORD>(buffer.size());
            pfxBlob.pbData = reinterpret_cast<BYTE*>(&buffer[0]);

            PCCERT_CONTEXT cert = 0;
            DWORD importFlags = (certStoreLocation == "LocalMachine") ? CRYPT_MACHINE_KEYSET : CRYPT_USER_KEYSET;
            HCERTSTORE store = PFXImportCertStore(
                &pfxBlob,
                Ice::stringToWstring(properties->getProperty(prefix + "Password")).c_str(),
                importFlags);
            int err = store ? 0 : GetLastError();

            if (store)
            {
                // Try to find a certificate chain.
                CERT_CHAIN_FIND_BY_ISSUER_PARA para;
                memset(&para, 0, sizeof(CERT_CHAIN_FIND_BY_ISSUER_PARA));
                para.cbSize = sizeof(CERT_CHAIN_FIND_BY_ISSUER_PARA);

                PCCERT_CHAIN_CONTEXT chain = 0;
                while (!cert)
                {
                    chain = CertFindChainInStore(
                        store,
                        X509_ASN_ENCODING,
                        CERT_CHAIN_FIND_BY_ISSUER_CACHE_ONLY_URL_FLAG, // Don't fetch anything from the Internet
                        CERT_CHAIN_FIND_BY_ISSUER,
                        &para,
                        chain);
                    if (!chain)
                    {
                        break; // No more chains found in the store.
                    }

                    if (chain->cChain > 0 && chain->rgpChain[0]->cElement > 0)
                    {
                        cert = CertDuplicateCertificateContext(chain->rgpChain[0]->rgpElement[0]->pCertContext);
                    }
                    CertFreeCertificateChain(chain);
                }

                // Check if we can find a certificate if we couldn't find a chain.
                if (!cert)
                {
                    cert = CertFindCertificateInStore(store, X509_ASN_ENCODING, 0, CERT_FIND_ANY, 0, cert);
                }
                if (!cert)
                {
                    throw InitializationException(
                        __FILE__,
                        __LINE__,
                        "IceSSL: certificate error:\n" + lastErrorToString());
                }
                _allCerts.push_back(cert);
                _stores.push_back(store);
                continue;
            }

            assert(err);
            if (err != CRYPT_E_BAD_ENCODE)
            {
                throw InitializationException(
                    __FILE__,
                    __LINE__,
                    "IceSSL: error decoding certificate:\n" + lastErrorToString());
            }

            // Try to load certificate & key as PEM files.
            if (keyFiles.empty())
            {
                throw InitializationException(__FILE__, __LINE__, "IceSSL: no key file specified");
            }

            err = 0;
            keyFile = keyFiles[i];
            if (!checkPath(keyFile, defaultDir, false, resolved))
            {
                throw InitializationException(__FILE__, __LINE__, "IceSSL: key file not found:\n" + keyFile);
            }
            keyFile = resolved;

            readFile(keyFile, buffer);
            if (buffer.empty())
            {
                throw InitializationException(__FILE__, __LINE__, "IceSSL: key file is empty:\n" + keyFile);
            }

            vector<BYTE> outBuffer;
            outBuffer.resize(buffer.size());
            DWORD outLength = static_cast<DWORD>(buffer.size());

            // Convert the PEM encoded buffer to DER binary format.
            if (!CryptStringToBinary(
                    &buffer[0],
                    static_cast<DWORD>(buffer.size()),
                    CRYPT_STRING_BASE64HEADER,
                    &outBuffer[0],
                    &outLength,
                    0,
                    0))
            {
                throw InitializationException(
                    __FILE__,
                    __LINE__,
                    "IceSSL: error decoding key `" + keyFile + "':\n" + lastErrorToString());
            }

            PCRYPT_PRIVATE_KEY_INFO keyInfo = 0;
            BYTE* key = 0;
            HCRYPTKEY hKey = 0;
            try
            {
                // First try to decode as a PKCS#8 key, if that fails try PKCS#1.
                DWORD decodedLength = 0;
                if (CryptDecodeObjectEx(
                        X509_ASN_ENCODING,
                        PKCS_PRIVATE_KEY_INFO,
                        &outBuffer[0],
                        outLength,
                        CRYPT_DECODE_ALLOC_FLAG,
                        0,
                        &keyInfo,
                        &decodedLength))
                {
                    // Check that we are using an RSA Key.
                    if (strcmp(keyInfo->Algorithm.pszObjId, szOID_RSA_RSA))
                    {
                        throw InitializationException(
                            __FILE__,
                            __LINE__,
                            string("IceSSL: error unknow key algorithm: `") + keyInfo->Algorithm.pszObjId + "'");
                    }

                    // Decode the private key BLOB.
                    if (!CryptDecodeObjectEx(
                            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                            PKCS_RSA_PRIVATE_KEY,
                            keyInfo->PrivateKey.pbData,
                            keyInfo->PrivateKey.cbData,
                            CRYPT_DECODE_ALLOC_FLAG,
                            0,
                            &key,
                            &outLength))
                    {
                        throw InitializationException(
                            __FILE__,
                            __LINE__,
                            "IceSSL: error decoding key `" + keyFile + "':\n" + lastErrorToString());
                    }
                    LocalFree(keyInfo);
                    keyInfo = 0;
                }
                else
                {
                    // Decode the private key BLOB.
                    if (!CryptDecodeObjectEx(
                            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                            PKCS_RSA_PRIVATE_KEY,
                            &outBuffer[0],
                            outLength,
                            CRYPT_DECODE_ALLOC_FLAG,
                            0,
                            &key,
                            &outLength))
                    {
                        throw InitializationException(
                            __FILE__,
                            __LINE__,
                            "IceSSL: error decoding key `" + keyFile + "':\n" + lastErrorToString());
                    }
                }

                // Create a new RSA key set to store our key.
                const wstring keySetName = Ice::stringToWstring(generateUUID());
                HCRYPTPROV cryptProv = 0;

                DWORD contextFlags = CRYPT_NEWKEYSET;
                if (certStoreLocation == "LocalMachine")
                {
                    contextFlags |= CRYPT_MACHINE_KEYSET;
                };

                if (!CryptAcquireContextW(
                        &cryptProv,
                        keySetName.c_str(),
                        MS_ENHANCED_PROV_W,
                        PROV_RSA_FULL,
                        contextFlags))
                {
                    throw InitializationException(
                        __FILE__,
                        __LINE__,
                        "IceSSL: error acquiring cryptographic "
                        "context:\n" +
                            lastErrorToString());
                }

                // Import the private key.
                if (!CryptImportKey(cryptProv, key, outLength, 0, 0, &hKey))
                {
                    throw InitializationException(
                        __FILE__,
                        __LINE__,
                        "IceSSL: error importing key `" + keyFile + "':\n" + lastErrorToString());
                }
                LocalFree(key);
                key = 0;

                CryptDestroyKey(hKey);
                hKey = 0;

                // Create a new memory store to place the certificate.
                store = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, 0);
                if (!store)
                {
                    throw InitializationException(
                        __FILE__,
                        __LINE__,
                        "IceSSL: error creating certificate "
                        "store:\n" +
                            lastErrorToString());
                }

                addCertificatesToStore(certFile, store, &cert);

                // Associate key & certificate.
                CRYPT_KEY_PROV_INFO keyProvInfo;
                memset(&keyProvInfo, 0, sizeof(keyProvInfo));
                keyProvInfo.pwszContainerName = const_cast<wchar_t*>(keySetName.c_str());
                keyProvInfo.pwszProvName = const_cast<wchar_t*>(MS_DEF_PROV_W);
                keyProvInfo.dwProvType = PROV_RSA_FULL;
                keyProvInfo.dwKeySpec = AT_KEYEXCHANGE;
                if (!CertSetCertificateContextProperty(cert, CERT_KEY_PROV_INFO_PROP_ID, 0, &keyProvInfo))
                {
                    throw InitializationException(
                        __FILE__,
                        __LINE__,
                        "IceSSL: error setting certificate "
                        "property:\n" +
                            lastErrorToString());
                }

                _importedCerts.push_back(cert);
                _allCerts.push_back(cert);
                _stores.push_back(store);
            }
            catch (...)
            {
                if (keyInfo)
                {
                    LocalFree(keyInfo);
                }

                if (key)
                {
                    LocalFree(key);
                }

                if (hKey)
                {
                    CryptDestroyKey(hKey);
                }

                if (cert)
                {
                    CertFreeCertificateContext(cert);
                }

                if (store)
                {
                    CertCloseStore(store, 0);
                }
                throw;
            }
        }
    }
    else if (!findCert.empty())
    {
        string certStore = properties->getPropertyWithDefault(prefix + "CertStore", "My");
        vector<PCCERT_CONTEXT> certs = findCertificates(certStoreLocation, certStore, findCert, _stores);
        if (certs.empty())
        {
            throw InitializationException(__FILE__, __LINE__, "IceSSL: no certificates found");
        }
        _allCerts.insert(_allCerts.end(), certs.begin(), certs.end());
    }
}

string
SChannel::SSLEngine::getCipherName(ALG_ID cipher) const
{
    switch (cipher)
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
        {
            ostringstream os;
            os << "Unknown cipher: " << cipher;
            return os.str();
        }
    }
}

SCHANNEL_CRED
SChannel::SSLEngine::newCredentialsHandle(bool incoming) const
{
    SCHANNEL_CRED cred;
    memset(&cred, 0, sizeof(cred));
    cred.dwVersion = SCHANNEL_CRED_VERSION;

    if (!_allCerts.empty())
    {
        cred.cCreds = static_cast<DWORD>(_allCerts.size());
        cred.paCred = const_cast<PCCERT_CONTEXT*>(&_allCerts[0]);
    }

    if (incoming)
    {
        // Don't set SCH_SEND_ROOT_CERT as it seems to cause problems with Java certificate validation and SChannel
        // doesn't seems to send the root certificate either way.
        cred.dwFlags = SCH_CRED_NO_SYSTEM_MAPPER | SCH_CRED_DISABLE_RECONNECTS;

        // There's no way to prevent SChannel from sending "CA names" to the client. Recent Windows versions don't CA
        // names but older ones do send all the trusted root CA names. We provide the root store to ensure that for
        // these older Windows versions, we also include the CA names of our trusted roots.
        cred.hRootStore = _rootStore;
    }
    else
    {
        cred.dwFlags = SCH_CRED_MANUAL_CRED_VALIDATION | SCH_CRED_NO_SERVERNAME_CHECK | SCH_CRED_NO_DEFAULT_CREDS;
    }

    if (_strongCrypto)
    {
        cred.dwFlags |= SCH_USE_STRONG_CRYPTO;
    }
    return cred;
}

void
SChannel::SSLEngine::destroy()
{
    if (_chainEngine && _chainEngine != HCCE_CURRENT_USER && _chainEngine != HCCE_LOCAL_MACHINE)
    {
        CertFreeCertificateChainEngine(_chainEngine);
    }

    if (_rootStore)
    {
        CertCloseStore(_rootStore, 0);
    }

    for (vector<PCCERT_CONTEXT>::const_iterator i = _importedCerts.begin(); i != _importedCerts.end(); ++i)
    {
        // Retrieve the certificate CERT_KEY_PROV_INFO_PROP_ID property, we use the CRYPT_KEY_PROV_INFO data to remove
        // the key set associated with the certificate.
        DWORD length = 0;
        if (!CertGetCertificateContextProperty(*i, CERT_KEY_PROV_INFO_PROP_ID, 0, &length))
        {
            continue;
        }
        vector<char> buf(length);
        if (!CertGetCertificateContextProperty(*i, CERT_KEY_PROV_INFO_PROP_ID, &buf[0], &length))
        {
            continue;
        }
        CRYPT_KEY_PROV_INFO* key = reinterpret_cast<CRYPT_KEY_PROV_INFO*>(&buf[0]);
        HCRYPTPROV prov = 0;
        CryptAcquireContextW(&prov, key->pwszContainerName, key->pwszProvName, key->dwProvType, CRYPT_DELETEKEYSET);
    }

    for (vector<PCCERT_CONTEXT>::const_iterator i = _allCerts.begin(); i != _allCerts.end(); ++i)
    {
        CertFreeCertificateContext(*i);
    }

    for (vector<HCERTSTORE>::const_iterator i = _stores.begin(); i != _stores.end(); ++i)
    {
        CertCloseStore(*i, 0);
    }
}

Ice::SSL::ClientAuthenticationOptions
SChannel::SSLEngine::createClientAuthenticationOptions(string_view host) const
{
    return Ice::SSL::ClientAuthenticationOptions{
        .clientCredentialsSelectionCallback = [this](string_view) { return newCredentialsHandle(false); },
        .serverCertificateValidationCallback = [self = shared_from_this(),
                                                host](CtxtHandle ssl, const ConnectionInfoPtr& info) -> bool
        { return self->validationCallback(ssl, info, false, host); }};
}

Ice::SSL::ServerAuthenticationOptions
SChannel::SSLEngine::createServerAuthenticationOptions() const
{
    return Ice::SSL::ServerAuthenticationOptions{
        .serverCredentialsSelectionCallback = [this](string_view) { return newCredentialsHandle(true); },
        .clientCertificateRequired = getVerifyPeer() > 0,
        .clientCertificateValidationCallback =
            [self = shared_from_this()](CtxtHandle ssl, const ConnectionInfoPtr& info) -> bool
        { return self->validationCallback(ssl, info, true, ""); }};
}

namespace
{
    struct ScopedCertChainContext
    {
        ScopedCertChainContext(PCCERT_CHAIN_CONTEXT chain) : _chain(chain) {}
        ~ScopedCertChainContext() { CertFreeCertificateChain(_chain); }
        PCCERT_CHAIN_CONTEXT _chain;
    };

    struct ScopedCertContext
    {
        ScopedCertContext(PCCERT_CONTEXT cert) : _cert(cert) {}
        ~ScopedCertContext() { CertFreeCertificateContext(_cert); }
        PCCERT_CONTEXT _cert;
    };
}

bool
SChannel::SSLEngine::validationCallback(CtxtHandle ssl, const ConnectionInfoPtr& info, bool incoming, string_view host)
    const
{
    // Build the peer certificate chain and verify it.
    PCCERT_CONTEXT cert = 0;
    SECURITY_STATUS err = QueryContextAttributes(&ssl, SECPKG_ATTR_REMOTE_CERT_CONTEXT, &cert);
    bool certificateRequired = !incoming || getVerifyPeer() > 1;
    if (err && (err != SEC_E_NO_CREDENTIALS || certificateRequired))
    {
        ostringstream os;
        os << "IceSSL: certificate verification failure:\n" << IceUtilInternal::errorToString(err);
        throw SecurityException(__FILE__, __LINE__, os.str());
    }

    if (cert) // Verify the remote certificate
    {
        ScopedCertContext scopedCertContext(cert);
        CERT_CHAIN_PARA chainP;
        memset(&chainP, 0, sizeof(chainP));
        chainP.cbSize = sizeof(chainP);

        string trustError;
        PCCERT_CHAIN_CONTEXT certChain;
        DWORD dwFlags = 0;
        int revocationCheck = getRevocationCheck();
        if (revocationCheck > 0)
        {
            if (getRevocationCheckCacheOnly())
            {
                // Disable network I/O for revocation checks.
                dwFlags = CERT_CHAIN_REVOCATION_CHECK_CACHE_ONLY | CERT_CHAIN_DISABLE_AIA;
            }

            dwFlags |=
                (revocationCheck == 1 ? CERT_CHAIN_REVOCATION_CHECK_END_CERT
                                      : CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT);
        }

        if (!CertGetCertificateChain(_chainEngine, cert, 0, cert->hCertStore, &chainP, dwFlags, 0, &certChain))
        {
            ostringstream os;
            os << "IceSSL: certificate verification failure:\n" << lastErrorToString();
            throw SecurityException(__FILE__, __LINE__, os.str());
        }
        ScopedCertChainContext scopedChainContext(certChain);

        DWORD errorStatus = certChain->TrustStatus.dwErrorStatus;
        if (errorStatus != CERT_TRUST_NO_ERROR)
        {
            throw SecurityException(__FILE__, __LINE__, errorStatusToString(errorStatus));
        }

        SSL_EXTRA_CERT_CHAIN_POLICY_PARA extraPolicyPara;
        memset(&extraPolicyPara, 0, sizeof(extraPolicyPara));
        extraPolicyPara.cbSize = sizeof(extraPolicyPara);
        extraPolicyPara.dwAuthType = incoming ? AUTHTYPE_CLIENT : AUTHTYPE_SERVER;
        // Disable because the policy only matches the CN of the certificate, not the SAN.
        extraPolicyPara.fdwChecks = SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
        extraPolicyPara.pwszServerName = const_cast<wchar_t*>(IceUtil::stringToWstring(host).c_str());

        CERT_CHAIN_POLICY_PARA policyPara;
        memset(&policyPara, 0, sizeof(policyPara));
        policyPara.cbSize = sizeof(policyPara);
        policyPara.pvExtraPolicyPara = &extraPolicyPara;

        CERT_CHAIN_POLICY_STATUS policyStatus;
        memset(&policyStatus, 0, sizeof(policyStatus));
        policyStatus.cbSize = sizeof(policyStatus);

        if (!CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_SSL, certChain, &policyPara, &policyStatus))
        {
            ostringstream os;
            os << "IceSSL: certificate verification failure:\n" << lastErrorToString();
            throw SecurityException(__FILE__, __LINE__, os.str());
        }

        if (policyStatus.dwError)
        {
            ostringstream os;
            os << "IceSSL: certificate verification failure:\n" << policyStatusToString(policyStatus.dwError);
            throw SecurityException(__FILE__, __LINE__, os.str());
        }

        if (!incoming)
        {
            verifyPeerCertName(info, host);
        }
        verifyPeer(info);
    }
    return true;
}

string
SChannel::SSLEngine::policyStatusToString(DWORD policyStatus) const
{
    assert(policyStatus);
    ostringstream os;
    switch (policyStatus)
    {
        case TRUST_E_CERT_SIGNATURE:
        {
            os << "The signature of the certificate cannot be verified.";
            break;
        }
        case CRYPT_E_REVOKED:
        {
            os << "The certificate or signature has been revoked.";
            break;
        }
        case CERT_E_UNTRUSTEDROOT:
        {
            os << "A certification chain processed correctly but terminated in a root certificate that is not "
                  "trusted by "
                  "the trust provider.";
            break;
        }
        case CERT_E_UNTRUSTEDTESTROOT:
        {
            os << "The root certificate is a testing certificate, and policy settings disallow test certificates.";
            break;
        }
        case CERT_E_CHAINING:
        {
            os << "A chain of certificates was not correctly created.";
            break;
        }
        case CERT_E_WRONG_USAGE:
        {
            os << "The certificate is not valid for the requested usage.";
            break;
        }
        case CERT_E_EXPIRED:
        {
            os << "A required certificate is not within its validity period.";
            break;
        }
        case CERT_E_INVALID_NAME:
        {
            os << "The certificate has an invalid name. Either the name is not included in the permitted list, or "
                  "it is "
                  "explicitly excluded.";
            break;
        }
        case CERT_E_INVALID_POLICY:
        {
            os << "The certificate has invalid policy.";
            break;
        }
        case TRUST_E_BASIC_CONSTRAINTS:
        {
            os << "The basic constraints of the certificate are not valid, or they are missing.";
            break;
        }
        case CERT_E_CRITICAL:
        {
            os << "The certificate is being used for a purpose other than the purpose specified by its CA.";
            break;
        }
        case CERT_E_VALIDITYPERIODNESTING:
        {
            os << "The validity periods of the certification chain do not nest correctly.";
            break;
        }
        case CRYPT_E_NO_REVOCATION_CHECK:
        {
            os << "The revocation function was unable to check revocation for the certificate.";
            break;
        }
        case CRYPT_E_REVOCATION_OFFLINE:
        {
            os << "The revocation function was unable to check revocation because the revocation server was "
                  "offline.";
            break;
        }
        case CERT_E_CN_NO_MATCH:
        {
            os << "The certificate's CN name does not match the passed value.";
            break;
        }
        case CERT_E_PURPOSE:
        {
            os << "The certificate is being used for a purpose other than the purpose specified by its CA.";
            break;
        }
        case CERT_E_REVOKED:
        {
            os << "The certificate has been explicitly revoked by the issuer.";
            break;
        }
        case CERT_E_REVOCATION_FAILURE:
        {
            os << "The revocation process could not continue, and the certificate could not be checked.";
            break;
        }
        case CERT_E_ROLE:
        {
            os << "The certificate does not have a valid role.";
            break;
        }
        default:
        {
            os << "Unknown policy status: " << policyStatus;
            break;
        }
    }
    return os.str();
}

string
SChannel::SSLEngine::errorStatusToString(DWORD errorStatus) const
{
    assert(errorStatus != CERT_TRUST_NO_ERROR);
    ostringstream os;
    os << "IceSSL: certificate verification failure:";

    if (errorStatus & CERT_TRUST_IS_NOT_TIME_VALID)
    {
        os << "\n  - This certificate or one of the certificates in the certificate chain is not time valid.";
    }

    if (errorStatus & CERT_TRUST_IS_REVOKED)
    {
        os << "\n  - Trust for this certificate or one of the certificates in the certificate chain has been "
              "revoked.";
    }

    if (errorStatus & CERT_TRUST_IS_NOT_SIGNATURE_VALID)
    {
        os << "\n  - The certificate or one of the certificates in the certificate chain does not have a valid "
              "signature.";
    }

    if (errorStatus & CERT_TRUST_IS_NOT_VALID_FOR_USAGE)
    {
        os << "\n  - The certificate or certificate chain is not valid for its proposed usage.";
    }

    if (errorStatus & CERT_TRUST_IS_UNTRUSTED_ROOT)
    {
        os << "\n  - The certificate or certificate chain is based on an untrusted root.";
    }

    if (errorStatus & CERT_TRUST_REVOCATION_STATUS_UNKNOWN)
    {
        os << "\n  - The revocation status of the certificate or one of the certificates in the certificate "
              "chain is unknown.";
    }

    if (errorStatus & CERT_TRUST_IS_CYCLIC)
    {
        os << "\n  - One of the certificates in the chain was issued by a certificate in the chain.";
    }

    if (errorStatus & CERT_TRUST_INVALID_EXTENSION)
    {
        os << "\n  - One of the certificates has an extension that is not valid.";
    }

    if (errorStatus & CERT_TRUST_INVALID_POLICY_CONSTRAINTS)
    {
        os << "\n  - The certificate or one of the certificates in the certificate chain has a policy "
              "constraints extension, and one of the issued certificates has a disallowed policy mapping "
              "extension or does not have a required issuance policies extension.";
    }

    if (errorStatus & CERT_TRUST_INVALID_BASIC_CONSTRAINTS)
    {
        os << "\n  - The certificate or one of the certificates in the certificate chain has a basic "
              "constraints "
              "extension, and either the certificate cannot be used to issue other certificates, or the chain "
              "path length has been exceeded.";
    }

    if (errorStatus & CERT_TRUST_HAS_NOT_SUPPORTED_NAME_CONSTRAINT)
    {
        os << "\n  - The certificate or one of the certificates in the certificate chain has a name "
              "constraints extension that contains unsupported fields.";
    }

    if (errorStatus & CERT_TRUST_HAS_NOT_DEFINED_NAME_CONSTRAINT)
    {
        os << "\n  - The certificate or one of the certificates in the certificate chain has a name "
              "constraints extension and a name constraint is missing for one of the name choices in the end "
              "certificate.";
    }

    if (errorStatus & CERT_TRUST_HAS_NOT_PERMITTED_NAME_CONSTRAINT)
    {
        os << "\n  - The certificate or one of the certificates in the certificate chain has a name "
              "constraints extension, and there is not a permitted name constraint for one of the name choices "
              "in the end certificate.";
    }

    if (errorStatus & CERT_TRUST_HAS_EXCLUDED_NAME_CONSTRAINT)
    {
        os << "\n  - The certificate or one of the certificates in the certificate chain has a name "
              "constraints extension, and one of the name choices in the end certificate is explicitly "
              "excluded.";
    }

    if (errorStatus & CERT_TRUST_IS_OFFLINE_REVOCATION)
    {
        os << "\n  - The revocation status of the certificate or one of the certificates in the certificate "
              "chain is either offline or stale.";
    }

    if (errorStatus & CERT_TRUST_NO_ISSUANCE_CHAIN_POLICY)
    {
        os << "\n  - The end certificate does not have any resultant issuance policies, and one of the issuing "
              "certification authority certificates has a policy constraints extension requiring it.";
    }

    if (errorStatus & CERT_TRUST_IS_EXPLICIT_DISTRUST)
    {
        os << "\n  - The certificate is explicitly distrusted.";
    }

    if (errorStatus & CERT_TRUST_HAS_NOT_SUPPORTED_CRITICAL_EXT)
    {
        os << "\n  - The certificate does not support a critical extension.";
    }

    if (errorStatus & CERT_TRUST_HAS_WEAK_SIGNATURE)
    {
        os << "\n  - The certificate has not been strong signed.";
    }

    if (errorStatus & CERT_TRUST_IS_PARTIAL_CHAIN)
    {
        os << "\n  - The certificate chain is not complete.";
    }

    if (errorStatus & CERT_TRUST_CTL_IS_NOT_TIME_VALID)
    {
        os << "\n  - A certificate trust list (CTL) used to create this chain was not time valid.";
    }

    if (errorStatus & CERT_TRUST_CTL_IS_NOT_SIGNATURE_VALID)
    {
        os << "\n  - A CTL used to create this chain did not have a valid signature.";
    }

    if (errorStatus & CERT_TRUST_CTL_IS_NOT_VALID_FOR_USAGE)
    {
        os << "\n  - A CTL used to create this chain is not valid for this usage.";
    }

    return os.str();
}
