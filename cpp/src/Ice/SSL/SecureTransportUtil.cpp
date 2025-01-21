// Copyright (c) ZeroC, Inc.

#include "SecureTransportUtil.h"
#include "../Base64.h"
#include "../FileUtil.h"
#include "../UniqueRef.h"
#include "DistinguishedName.h"
#include "Ice/LocalExceptions.h"
#include "Ice/SSL/SSLException.h"
#include "Ice/StringUtil.h"

#include <fstream>
#include <sstream>

using namespace Ice;
using namespace IceInternal;
using namespace Ice::SSL;
using namespace Ice::SSL::SecureTransport;
using namespace std;

#if defined(__clang__)
#    pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace
{
    CFMutableDataRef readCertFile(const string& file)
    {
        ifstream is(IceInternal::streamFilename(file).c_str(), ios::in | ios::binary);
        if (!is.good())
        {
            throw CertificateReadException(__FILE__, __LINE__, "error opening file " + file);
        }

        is.seekg(0, is.end);
        size_t size = static_cast<size_t>(is.tellg());
        is.seekg(0, is.beg);

        UniqueRef<CFMutableDataRef> data(CFDataCreateMutable(kCFAllocatorDefault, static_cast<CFIndex>(size)));
        CFDataSetLength(data.get(), static_cast<CFIndex>(size));
        is.read(reinterpret_cast<char*>(CFDataGetMutableBytePtr(data.get())), static_cast<streamsize>(size));
        if (!is.good())
        {
            throw CertificateReadException(__FILE__, __LINE__, "error reading file " + file);
        }
        return data.release();
    }

#if defined(ICE_USE_SECURE_TRANSPORT_MACOS)
    // Map alternative name alias to its types.
    const char* certificateAlternativeNameTypes[] =
        {"", "Email Address", "DNS Name", "", "Directory Name", "", "URI", "IP Address"};
    const int certificateAlternativeNameTypesSize = sizeof(certificateAlternativeNameTypes) / sizeof(char*);

    int certificateAlternativeNameType(const string& alias)
    {
        if (!alias.empty())
        {
            for (int i = 0; i < certificateAlternativeNameTypesSize; ++i)
            {
                if (alias == certificateAlternativeNameTypes[i])
                {
                    return i;
                }
            }
        }
        return -1; // Not supported
    }

    string escapeX509Name(const string& name)
    {
        ostringstream os;
        for (char i : name)
        {
            switch (i)
            {
                case ',':
                case '=':
                case '+':
                case '<':
                case '>':
                case '#':
                case ';':
                {
                    os << '\\';
                }
                default:
                {
                    break;
                }
            }
            os << i;
        }
        return os.str();
    }

    CFDictionaryRef getCertificateProperty(SecCertificateRef cert, CFTypeRef key)
    {
        UniqueRef<CFDictionaryRef> property;
        UniqueRef<CFArrayRef> keys(CFArrayCreate(nullptr, &key, 1, &kCFTypeArrayCallBacks));
        UniqueRef<CFErrorRef> err;
        UniqueRef<CFDictionaryRef> values(SecCertificateCopyValues(cert, keys.get(), &err.get()));
        if (err)
        {
            ostringstream os;
            os << "SSL transport: error getting property for certificate:\n" << sslErrorToString(err);
            throw CertificateReadException(__FILE__, __LINE__, os.str());
        }

        assert(values);
        property.retain(static_cast<CFDictionaryRef>(CFDictionaryGetValue(values.get(), key)));
        return property.release();
    }

    DistinguishedName getX509Name(SecCertificateRef cert, CFTypeRef key)
    {
        assert(key == kSecOIDX509V1IssuerName || key == kSecOIDX509V1SubjectName);
        list<pair<string, string>> rdnPairs;
        UniqueRef<CFDictionaryRef> property(getCertificateProperty(cert, key));
        if (property)
        {
            auto dn = static_cast<CFArrayRef>(CFDictionaryGetValue(property.get(), kSecPropertyKeyValue));
            CFIndex size = CFArrayGetCount(dn);
            for (CFIndex i = 0; i < size; ++i)
            {
                auto dict = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(dn, i));
                rdnPairs.emplace_front(
                    certificateOIDAlias(
                        fromCFString((static_cast<CFStringRef>(CFDictionaryGetValue(dict, kSecPropertyKeyLabel))))),
                    escapeX509Name(
                        fromCFString(static_cast<CFStringRef>(CFDictionaryGetValue(dict, kSecPropertyKeyValue)))));
            }
        }
        return DistinguishedName(rdnPairs);
    }

    vector<pair<int, string>> getX509AltName(SecCertificateRef cert, CFTypeRef key)
    {
        assert(key == kSecOIDIssuerAltName || key == kSecOIDSubjectAltName);
        UniqueRef<CFDictionaryRef> property(getCertificateProperty(cert, key));

        vector<pair<int, string>> pairs;
        if (property)
        {
            auto names = static_cast<CFArrayRef>(CFDictionaryGetValue(property.get(), kSecPropertyKeyValue));
            CFIndex size = CFArrayGetCount(names);

            for (CFIndex i = 0; i < size; ++i)
            {
                auto dict = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(names, i));

                int type = certificateAlternativeNameType(
                    fromCFString(static_cast<CFStringRef>(CFDictionaryGetValue(dict, kSecPropertyKeyLabel))));
                if (type != -1)
                {
                    auto v = static_cast<CFStringRef>(CFDictionaryGetValue(dict, kSecPropertyKeyValue));
                    auto t = static_cast<CFStringRef>(CFDictionaryGetValue(dict, kSecPropertyKeyType));
                    if (CFEqual(t, kSecPropertyTypeString) || CFEqual(t, kSecPropertyTypeTitle))
                    {
                        pairs.emplace_back(type, fromCFString(v));
                    }
                    else if (CFEqual(t, kSecPropertyTypeURL))
                    {
                        pairs.emplace_back(type, fromCFString(CFURLGetString((CFURLRef)v)));
                    }
                    else if (CFEqual(t, kSecPropertyTypeSection))
                    {
                        auto section = (CFArrayRef)v;
                        ostringstream os;
                        for (CFIndex j = 0, count = CFArrayGetCount(section); j < count;)
                        {
                            auto d = (CFDictionaryRef)CFArrayGetValueAtIndex(section, j);

                            auto sectionLabel = static_cast<CFStringRef>(CFDictionaryGetValue(d, kSecPropertyKeyLabel));
                            auto sectionValue = static_cast<CFStringRef>(CFDictionaryGetValue(d, kSecPropertyKeyValue));

                            os << certificateOIDAlias(fromCFString(sectionLabel)) << "=" << fromCFString(sectionValue);
                            if (++j < count)
                            {
                                os << ",";
                            }
                        }
                        pairs.emplace_back(type, os.str());
                    }
                }
            }
        }
        return pairs;
    }
#endif
}

string
Ice::SSL::SecureTransport::sslErrorToString(CFErrorRef err)
{
    ostringstream os;
    if (err)
    {
        UniqueRef<CFStringRef> s(CFErrorCopyDescription(err));
        os << "(error: " << CFErrorGetCode(err) << " description: " << fromCFString(s.get()) << ")";
    }
    return os.str();
}

string
Ice::SSL::SecureTransport::sslErrorToString(OSStatus status)
{
    ostringstream os;
    os << "(error: " << status;
#if defined(ICE_USE_SECURE_TRANSPORT_MACOS)
    UniqueRef<CFStringRef> s(SecCopyErrorMessageString(status, nullptr));
    if (s)
    {
        os << " description: " << fromCFString(s.get());
    }
#endif
    os << ")";
    return os.str();
}

#if defined(ICE_USE_SECURE_TRANSPORT_MACOS)
CFDictionaryRef
Ice::SSL::SecureTransport::getCertificateProperty(SecCertificateRef cert, CFTypeRef key)
{
    UniqueRef<CFDictionaryRef> property;
    UniqueRef<CFArrayRef> keys(CFArrayCreate(nullptr, &key, 1, &kCFTypeArrayCallBacks));
    UniqueRef<CFErrorRef> err;
    UniqueRef<CFDictionaryRef> values(SecCertificateCopyValues(cert, keys.get(), &err.get()));
    if (err)
    {
        ostringstream os;
        os << "SSL transport: error getting property for certificate:\n" << sslErrorToString(err);
        throw CertificateReadException(__FILE__, __LINE__, os.str());
    }

    assert(values);
    property.retain(static_cast<CFDictionaryRef>(CFDictionaryGetValue(values.get(), key)));
    return property.release();
}

namespace
{
    //
    // Check the certificate basic constraints to check if the certificate is marked as a CA.
    //
    bool isCA(SecCertificateRef cert)
    {
        UniqueRef<CFDictionaryRef> property(getCertificateProperty(cert, kSecOIDBasicConstraints));
        if (property)
        {
            auto propertyValues = static_cast<CFArrayRef>(CFDictionaryGetValue(property.get(), kSecPropertyKeyValue));
            for (CFIndex i = 0, size = CFArrayGetCount(propertyValues); i < size; ++i)
            {
                auto dict = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(propertyValues, i));
                auto label = static_cast<CFStringRef>(CFDictionaryGetValue(dict, kSecPropertyKeyLabel));
                if (CFEqual(label, CFSTR("Certificate Authority")))
                {
                    return CFEqual(
                        static_cast<CFStringRef>(CFDictionaryGetValue(dict, kSecPropertyKeyValue)),
                        CFSTR("Yes"));
                }
            }
        }
        return false;
    }

    //
    // Load keychain items (Certificates or Private Keys) from a file. On return items param contain
    // the list of items, the caller must release it.
    //
    CFArrayRef
    loadKeychainItems(const string& file, SecExternalItemType type, SecKeychainRef keychain, const string& passphrase)
    {
        UniqueRef<CFMutableDataRef> data(readCertFile(file));

        SecItemImportExportKeyParameters params;
        memset(&params, 0, sizeof(params));
        params.version = SEC_KEY_IMPORT_EXPORT_PARAMS_VERSION;
        params.flags |= kSecKeyNoAccessControl;
        UniqueRef<CFStringRef> passphraseHolder;
        if (!passphrase.empty())
        {
            passphraseHolder.reset(toCFString(passphrase));
            params.passphrase = passphraseHolder.get();
        }

        UniqueRef<CFArrayRef> items;
        SecExternalItemType importType = type;
        SecExternalFormat format = type == kSecItemTypeUnknown ? kSecFormatPKCS12 : kSecFormatUnknown;
        UniqueRef<CFStringRef> path(toCFString(file));
        OSStatus err = SecItemImport(data.get(), path.get(), &format, &importType, 0, &params, keychain, &items.get());

        if (err != noErr)
        {
            ostringstream os;
            os << "SSL transport: error reading " << (type == kSecItemTypePrivateKey ? "private key" : "certificate");
            os << " '" << file << "':\n" << sslErrorToString(err);
            throw CertificateReadException(__FILE__, __LINE__, os.str());
        }

        if (type != kSecItemTypeUnknown && importType != kSecItemTypeAggregate && importType != type)
        {
            ostringstream os;
            os << "SSL transport: error reading " << (type == kSecItemTypePrivateKey ? "private key" : "certificate");
            os << " '" << file << "' doesn't contain the expected item";
            throw CertificateReadException(__FILE__, __LINE__, os.str());
        }

        return items.release();
    }

    SecKeychainRef openKeychain(const string& path, const string& keychainPassword)
    {
        string keychainPath = path;
        UniqueRef<SecKeychainRef> keychain;
        OSStatus err = 0;
        if (keychainPath.empty())
        {
            if ((err = SecKeychainCopyDefault(&keychain.get())))
            {
                throw InitializationException(
                    __FILE__,
                    __LINE__,
                    "SSL transport: unable to retrieve default keychain:\n" + sslErrorToString(err));
            }
        }
        else
        {
            //
            // KeyChain path is relative to the current working directory.
            //
            if (!IceInternal::isAbsolutePath(keychainPath))
            {
                string cwd;
                if (IceInternal::getcwd(cwd) == 0)
                {
                    keychainPath = string(cwd) + '/' + keychainPath;
                }
            }

            if ((err = SecKeychainOpen(keychainPath.c_str(), &keychain.get())))
            {
                throw InitializationException(
                    __FILE__,
                    __LINE__,
                    "SSL transport: unable to open keychain: '" + keychainPath + "'\n" + sslErrorToString(err));
            }
        }

        SecKeychainStatus status;
        err = SecKeychainGetStatus(keychain.get(), &status);
        if (err == noErr)
        {
            const char* pass = keychainPassword.empty() ? nullptr : keychainPassword.c_str();
            if ((err = SecKeychainUnlock(
                     keychain.get(),
                     static_cast<UInt32>(keychainPassword.size()),
                     pass,
                     pass != nullptr)))
            {
                throw InitializationException(
                    __FILE__,
                    __LINE__,
                    "SSL transport: unable to unlock keychain:\n" + sslErrorToString(err));
            }
        }
        else if (err == errSecNoSuchKeychain)
        {
            const char* pass = keychainPassword.empty() ? nullptr : keychainPassword.c_str();
            keychain.reset(0);
            if ((err = SecKeychainCreate(
                     keychainPath.c_str(),
                     static_cast<UInt32>(keychainPassword.size()),
                     pass,
                     pass == nullptr,
                     nullptr,
                     &keychain.get())))
            {
                throw InitializationException(
                    __FILE__,
                    __LINE__,
                    "SSL transport: unable to create keychain:\n" + sslErrorToString(err));
            }
        }
        else
        {
            throw InitializationException(
                __FILE__,
                __LINE__,
                "SSL transport: unable to open keychain:\n" + sslErrorToString(err));
        }

        //
        // Set keychain settings to avoid keychain lock.
        //
        SecKeychainSettings settings;
        settings.version = SEC_KEYCHAIN_SETTINGS_VERS1;
        settings.lockOnSleep = FALSE;
        settings.useLockInterval = FALSE;
        settings.lockInterval = INT_MAX;
        if ((err = SecKeychainSetSettings(keychain.get(), &settings)))
        {
            throw InitializationException(
                __FILE__,
                __LINE__,
                "SSL transport: error setting keychain settings:\n" + sslErrorToString(err));
        }

        return keychain.release();
    }

    //
    // Imports a certificate private key and optionally add it to a keychain.
    //
    SecIdentityRef
    loadPrivateKey(const string& file, SecCertificateRef cert, SecKeychainRef keychain, const string& password)
    {
        //
        // Check if we already imported the certificate
        //
        UniqueRef<CFDataRef> hash;
        UniqueRef<CFDictionaryRef> subjectKeyProperty(getCertificateProperty(cert, kSecOIDSubjectKeyIdentifier));
        if (subjectKeyProperty)
        {
            auto values = static_cast<CFArrayRef>(CFDictionaryGetValue(subjectKeyProperty.get(), kSecPropertyKeyValue));
            for (int i = 0; i < CFArrayGetCount(values); ++i)
            {
                auto dict = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(values, i));
                if (CFEqual(CFDictionaryGetValue(dict, kSecPropertyKeyLabel), CFSTR("Key Identifier")))
                {
                    hash.retain(static_cast<CFDataRef>(CFDictionaryGetValue(dict, kSecPropertyKeyValue)));
                    break;
                }
            }
        }

        const void* values[] = {keychain};
        UniqueRef<CFArrayRef> searchList(CFArrayCreate(kCFAllocatorDefault, values, 1, &kCFTypeArrayCallBacks));

        UniqueRef<CFMutableDictionaryRef> query(
            CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));

        CFDictionarySetValue(query.get(), kSecClass, kSecClassCertificate);
        CFDictionarySetValue(query.get(), kSecMatchLimit, kSecMatchLimitOne);
        CFDictionarySetValue(query.get(), kSecMatchSearchList, searchList.get());
        CFDictionarySetValue(query.get(), kSecAttrSubjectKeyID, hash.get());
        CFDictionarySetValue(query.get(), kSecReturnRef, kCFBooleanTrue);

        UniqueRef<CFTypeRef> value(0);
        OSStatus err = SecItemCopyMatching(query.get(), &value.get());
        UniqueRef<SecCertificateRef> item(static_cast<SecCertificateRef>(const_cast<void*>(value.release())));
        if (err == noErr)
        {
            //
            // If the certificate has already been imported, create the
            // identity. The key should also have been imported.
            //
            UniqueRef<SecIdentityRef> identity;
            err = SecIdentityCreateWithCertificate(keychain, item.get(), &identity.get());
            if (err != noErr)
            {
                ostringstream os;
                os << "SSL transport: error creating certificate identity:\n" << sslErrorToString(err);
                throw CertificateReadException(__FILE__, __LINE__, os.str());
            }
            return identity.release();
        }
        else if (err != errSecItemNotFound)
        {
            ostringstream os;
            os << "SSL transport: error searching for keychain items:\n" << sslErrorToString(err);
            throw CertificateReadException(__FILE__, __LINE__, os.str());
        }

        //
        // If the certificate isn't already in the keychain, load the
        // private key into the keychain and add the certificate.
        //
        UniqueRef<CFArrayRef> items(loadKeychainItems(file, kSecItemTypePrivateKey, keychain, password));
        CFIndex count = CFArrayGetCount(items.get());
        UniqueRef<SecKeyRef> key;
        for (CFIndex i = 0; i < count; ++i)
        {
            auto itemRef = static_cast<SecKeychainItemRef>(const_cast<void*>(CFArrayGetValueAtIndex(items.get(), 0)));
            if (SecKeyGetTypeID() == CFGetTypeID(itemRef))
            {
                key.retain(reinterpret_cast<SecKeyRef>(itemRef));
                break;
            }
        }
        if (!key)
        {
            throw CertificateReadException(__FILE__, __LINE__, "SSL transport: no key in file '" + file + "'");
        }

        //
        // Add the certificate to the keychain
        //
        query.reset(CFDictionaryCreateMutable(
            kCFAllocatorDefault,
            0,
            &kCFTypeDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks));

        CFDictionarySetValue(query.get(), kSecUseKeychain, keychain);
        CFDictionarySetValue(query.get(), kSecClass, kSecClassCertificate);
        CFDictionarySetValue(query.get(), kSecValueRef, cert);
        CFDictionarySetValue(query.get(), kSecReturnRef, kCFBooleanTrue);

        value.reset(0);
        err = SecItemAdd(query.get(), static_cast<CFTypeRef*>(&value.get()));
        UniqueRef<CFArrayRef> added(static_cast<CFArrayRef>(value.release()));
        if (err != noErr)
        {
            ostringstream os;
            os << "SSL transport: failure adding certificate to keychain\n" << sslErrorToString(err);
            throw CertificateReadException(__FILE__, __LINE__, os.str());
        }
        item.retain(static_cast<SecCertificateRef>(const_cast<void*>(CFArrayGetValueAtIndex(added.get(), 0))));

        //
        // Create the association between the private key and the certificate,
        // kSecKeyLabel attribute should match the subject key identifier.
        //
        vector<SecKeychainAttribute> attributes;
        if (hash)
        {
            SecKeychainAttribute attr;
            attr.tag = kSecKeyLabel;
            attr.data = const_cast<UInt8*>(CFDataGetBytePtr(hash.get()));
            attr.length = static_cast<UInt32>(CFDataGetLength(hash.get()));
            attributes.push_back(attr);
        }

        //
        // kSecKeyPrintName attribute correspond to the keychain display
        // name.
        //
        string label;
        UniqueRef<CFStringRef> commonName(0);
        if (SecCertificateCopyCommonName(item.get(), &commonName.get()) == noErr)
        {
            label = fromCFString(commonName.get());
            SecKeychainAttribute attr;
            attr.tag = kSecKeyPrintName;
            attr.data = const_cast<char*>(label.c_str());
            attr.length = static_cast<UInt32>(label.size());
            attributes.push_back(attr);
        }

        SecKeychainAttributeList attrs;
        attrs.attr = &attributes[0];
        attrs.count = static_cast<UInt32>(attributes.size());
        SecKeychainItemModifyAttributesAndData(reinterpret_cast<SecKeychainItemRef>(key.get()), &attrs, 0, nullptr);

        UniqueRef<SecIdentityRef> identity;
        err = SecIdentityCreateWithCertificate(keychain, item.get(), &identity.get());
        if (err != noErr)
        {
            ostringstream os;
            os << "SSL transport: error creating certificate identity:\n" << sslErrorToString(err);
            throw CertificateReadException(__FILE__, __LINE__, os.str());
        }
        return identity.release();
    }

} // anonymous namespace end

#else

namespace
{
    CFArrayRef loadCerts(const string& file)
    {
        UniqueRef<CFArrayRef> certs(CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks));
        if (file.find(".pem") != string::npos)
        {
            vector<char> buffer;
            readFile(file, buffer);
            string strbuf(buffer.begin(), buffer.end());
            string::size_type size, startpos, endpos = 0;
            bool first = true;
            while (true)
            {
                startpos = strbuf.find("-----BEGIN CERTIFICATE-----", endpos);
                if (startpos != string::npos)
                {
                    startpos += sizeof("-----BEGIN CERTIFICATE-----");
                    endpos = strbuf.find("-----END CERTIFICATE-----", startpos);
                    if (endpos == string::npos)
                    {
                        throw InitializationException(
                            __FILE__,
                            __LINE__,
                            "SSL transport: certificate " + file + " is not a valid PEM-encoded certificate");
                    }
                    size = endpos - startpos;
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

                vector<byte> data(IceInternal::Base64::decode(string(&buffer[startpos], size)));
                UniqueRef<CFDataRef> certdata(CFDataCreate(
                    kCFAllocatorDefault,
                    reinterpret_cast<uint8_t*>(&data[0]),
                    static_cast<CFIndex>(data.size())));
                UniqueRef<SecCertificateRef> cert(SecCertificateCreateWithData(0, certdata.get()));
                if (!cert)
                {
                    throw InitializationException(
                        __FILE__,
                        __LINE__,
                        "SSL transport: certificate " + file + " is not a valid PEM-encoded certificate");
                }
                CFArrayAppendValue(const_cast<CFMutableArrayRef>(certs.get()), cert.get());
                first = false;
            }
        }
        else
        {
            UniqueRef<CFDataRef> data(readCertFile(file));
            UniqueRef<SecCertificateRef> cert(SecCertificateCreateWithData(0, data.get()));
            if (!cert)
            {
                throw InitializationException(
                    __FILE__,
                    __LINE__,
                    "SSL transport: certificate " + file + " is not a valid DER-encoded certificate");
            }
            CFArrayAppendValue(const_cast<CFMutableArrayRef>(certs.get()), cert.get());
        }
        return certs.release();
    }
}

#endif

//
// Imports a certificate (it might contain an identity or certificate depending on the format).
//
CFArrayRef
Ice::SSL::SecureTransport::loadCertificateChain(
    const string& file,
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    const string& /*keyFile*/,
    const string& /*keychainPath*/,
    const string& /*keychainPassword*/,
#else
    const string& keyFile,
    const string& keychainPath,
    const string& keychainPassword,
#endif
    const string& password)
{
    UniqueRef<CFArrayRef> chain;
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    UniqueRef<CFDataRef> cert(readCertFile(file));

    UniqueRef<CFMutableDictionaryRef> settings(
        CFDictionaryCreateMutable(0, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    UniqueRef<CFArrayRef> items;
    UniqueRef<CFStringRef> pass(toCFString(password));
    CFDictionarySetValue(settings.get(), kSecImportExportPassphrase, pass.get());
    OSStatus err = SecPKCS12Import(cert.get(), settings.get(), &items.get());

    if (err != noErr)
    {
        ostringstream os;
        os << "SSL transport: unable to import certificate from file " << file << " (error = " << err << ")";
        throw InitializationException(__FILE__, __LINE__, os.str());
    }

    for (int i = 0; i < CFArrayGetCount(items.get()); ++i)
    {
        CFDictionaryRef dict = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(items.get(), i));
        SecIdentityRef identity =
            static_cast<SecIdentityRef>(const_cast<void*>(CFDictionaryGetValue(dict, kSecImportItemIdentity)));
        if (identity)
        {
            CFArrayRef certs = static_cast<CFArrayRef>(CFDictionaryGetValue(dict, kSecImportItemCertChain));
            chain.reset(CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, certs));
            CFArraySetValueAtIndex(const_cast<CFMutableArrayRef>(chain.get()), 0, identity);
        }
    }

    if (!chain)
    {
        ostringstream os;
        os << "SSL transport: couldn't find identity in file " << file;
        throw InitializationException(__FILE__, __LINE__, os.str());
    }
#else
    UniqueRef<SecKeychainRef> keychain(openKeychain(keychainPath, keychainPassword));
    if (keyFile.empty())
    {
        chain.reset(loadKeychainItems(file, kSecItemTypeUnknown, keychain.get(), password));
    }
    else
    {
        // Load the certificate, don't load into the keychain as it might already have been imported.
        UniqueRef<CFArrayRef> items(loadKeychainItems(file, kSecItemTypeCertificate, nullptr, password));
        auto cert = static_cast<SecCertificateRef>(const_cast<void*>(CFArrayGetValueAtIndex(items.get(), 0)));
        if (SecCertificateGetTypeID() != CFGetTypeID(cert))
        {
            ostringstream os;
            os << "SSL transport: couldn't find certificate in '" << file << "'";
            throw CertificateReadException(__FILE__, __LINE__, os.str());
        }

        //
        // Load the private key for the given certificate. This will
        // add the certificate/key to the keychain if they aren't
        // already present in the keychain.
        //
        UniqueRef<SecIdentityRef> identity(loadPrivateKey(keyFile, cert, keychain.get(), password));
        chain.reset(CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, items.get()));
        CFArraySetValueAtIndex(const_cast<CFMutableArrayRef>(chain.get()), 0, identity.get());
    }
#endif
    return chain.release();
}

SecCertificateRef
Ice::SSL::SecureTransport::loadCertificate(const string& file)
{
    UniqueRef<SecCertificateRef> cert;
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    UniqueRef<CFArrayRef> certs(loadCerts(file));
    assert(CFArrayGetCount(certs.get()) > 0);
    cert.retain((SecCertificateRef)CFArrayGetValueAtIndex(certs.get(), 0));
#else
    UniqueRef<CFArrayRef> items(loadKeychainItems(file, kSecItemTypeCertificate, nullptr, ""));
    cert.retain((SecCertificateRef)CFArrayGetValueAtIndex(items.get(), 0));
#endif
    return cert.release();
}

CFArrayRef
Ice::SSL::SecureTransport::loadCACertificates(const string& file)
{
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    return loadCerts(file);
#else
    UniqueRef<CFArrayRef> items(loadKeychainItems(file, kSecItemTypeCertificate, nullptr, ""));
    UniqueRef<CFArrayRef> certificateAuthorities(CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks));
    CFIndex count = CFArrayGetCount(items.get());
    for (CFIndex i = 0; i < count; ++i)
    {
        auto cert = static_cast<SecCertificateRef>(const_cast<void*>(CFArrayGetValueAtIndex(items.get(), i)));
        assert(SecCertificateGetTypeID() == CFGetTypeID(cert));
        if (isCA(cert))
        {
            CFArrayAppendValue(const_cast<CFMutableArrayRef>(certificateAuthorities.get()), cert);
        }
    }
    return certificateAuthorities.release();
#endif
}

CFArrayRef
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
Ice::SSL::SecureTransport::findCertificateChain(const string&, const string&, const string& value)
#else
Ice::SSL::SecureTransport::findCertificateChain(
    const string& keychainPath,
    const string& keychainPassword,
    const string& value)
#endif
{
    //
    //  Search the keychain using key:value pairs. The following keys are supported:
    //
    //   Label
    //   Serial
    //   Subject
    //   SubjectKeyId
    //
    //   A value must be enclosed in single or double quotes if it contains whitespace.
    //
    UniqueRef<CFMutableDictionaryRef> query(
        CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));

#if defined(ICE_USE_SECURE_TRANSPORT_MACOS)
    UniqueRef<SecKeychainRef> keychain(openKeychain(keychainPath, keychainPassword));
    const void* values[] = {keychain.get()};
    UniqueRef<CFArrayRef> searchList(CFArrayCreate(kCFAllocatorDefault, values, 1, &kCFTypeArrayCallBacks));
    CFDictionarySetValue(query.get(), kSecMatchSearchList, searchList.get());
#endif
    CFDictionarySetValue(query.get(), kSecMatchLimit, kSecMatchLimitOne);
    CFDictionarySetValue(query.get(), kSecClass, kSecClassCertificate);
    CFDictionarySetValue(query.get(), kSecReturnRef, kCFBooleanTrue);
    CFDictionarySetValue(query.get(), kSecMatchCaseInsensitive, kCFBooleanTrue);

    size_t start = 0;
    size_t pos;
    bool valid = false;
    while ((pos = value.find(':', start)) != string::npos)
    {
        string field = IceInternal::toUpper(IceInternal::trim(value.substr(start, pos - start)));
        string arg;
        if (field != "LABEL" && field != "SERIAL" && field != "SUBJECT" && field != "SUBJECTKEYID")
        {
            throw InitializationException(__FILE__, __LINE__, "SSL transport: unknown key in '" + value + "'");
        }

        start = pos + 1;
        while (start < value.size() && (value[start] == ' ' || value[start] == '\t'))
        {
            ++start;
        }

        if (start == value.size())
        {
            throw InitializationException(__FILE__, __LINE__, "SSL transport: missing argument in '" + value + "'");
        }

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
                throw InitializationException(__FILE__, __LINE__, "SSL transport: unmatched quote in '" + value + "'");
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

        if (field == "SUBJECT" || field == "LABEL")
        {
            UniqueRef<CFStringRef> v(toCFString(arg));
            CFDictionarySetValue(query.get(), field == "LABEL" ? kSecAttrLabel : kSecMatchSubjectContains, v.get());
            valid = true;
        }
        else if (field == "SUBJECTKEYID" || field == "SERIAL")
        {
            vector<unsigned char> buffer;
            if (!parseBytes(arg, buffer))
            {
                throw InitializationException(__FILE__, __LINE__, "SSL transport: invalid value '" + value + "'");
            }
            UniqueRef<CFDataRef> v(CFDataCreate(kCFAllocatorDefault, &buffer[0], static_cast<CFIndex>(buffer.size())));
            CFDictionarySetValue(
                query.get(),
                field == "SUBJECTKEYID" ? kSecAttrSubjectKeyID : kSecAttrSerialNumber,
                v.get());
            valid = true;
        }
    }

    if (!valid)
    {
        throw InitializationException(__FILE__, __LINE__, "SSL transport: invalid value '" + value + "'");
    }

    UniqueRef<SecCertificateRef> cert;
    OSStatus err = SecItemCopyMatching(query.get(), (CFTypeRef*)&cert.get());
    if (err != noErr)
    {
        throw InitializationException(
            __FILE__,
            __LINE__,
            "SSL transport: find certificate '" + value + "' failed:\n" + sslErrorToString(err));
    }

    //
    // Retrieve the certificate chain
    //
    UniqueRef<SecPolicyRef> policy(SecPolicyCreateSSL(true, nullptr));
    UniqueRef<SecTrustRef> trust;
    err = SecTrustCreateWithCertificates(reinterpret_cast<CFArrayRef>(cert.get()), policy.get(), &trust.get());
    if (err || !trust)
    {
        throw InitializationException(
            __FILE__,
            __LINE__,
            "SSL transport: error creating trust object" + (err ? ":\n" + sslErrorToString(err) : ""));
    }

    SecTrustResultType trustResult;
    if ((err = SecTrustEvaluate(trust.get(), &trustResult)))
    {
        throw InitializationException(
            __FILE__,
            __LINE__,
            "SSL transport: error evaluating trust:\n" + sslErrorToString(err));
    }

    CFIndex chainLength = SecTrustGetCertificateCount(trust.get());
    UniqueRef<CFArrayRef> items(CFArrayCreateMutable(kCFAllocatorDefault, chainLength, &kCFTypeArrayCallBacks));
    for (int i = 0; i < chainLength; ++i)
    {
        CFArrayAppendValue(const_cast<CFMutableArrayRef>(items.get()), SecTrustGetCertificateAtIndex(trust.get(), i));
    }

    //
    // Replace the first certificate in the chain with the
    // identity.
    //
    UniqueRef<SecIdentityRef> identity;
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)

    //
    // SecIdentityCreateWithCertificate isn't supported on iOS so we lookup the identity
    // using the certificate label. If the user added the identity with SecItemAdd the
    // identity has the same label as the certificate.
    //
    query.reset(CFDictionaryCreateMutable(0, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    CFDictionarySetValue(query.get(), kSecClass, kSecClassCertificate);
    CFDictionarySetValue(query.get(), kSecValueRef, cert.get());
    CFDictionarySetValue(query.get(), kSecReturnAttributes, kCFBooleanTrue);
    UniqueRef<CFDictionaryRef> attributes;
    err = SecItemCopyMatching(query.get(), reinterpret_cast<CFTypeRef*>(&attributes.get()));
    if (err != noErr)
    {
        ostringstream os;
        os << "SSL transport: couldn't create identity for certificate found in the keychain:\n"
           << sslErrorToString(err);
        throw InitializationException(__FILE__, __LINE__, os.str());
    }

    // Now lookup the identity with the label
    query.reset(CFDictionaryCreateMutable(0, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    CFDictionarySetValue(query.get(), kSecMatchLimit, kSecMatchLimitOne);
    CFDictionarySetValue(query.get(), kSecClass, kSecClassIdentity);
    CFDictionarySetValue(query.get(), kSecAttrLabel, (CFDataRef)CFDictionaryGetValue(attributes.get(), kSecAttrLabel));
    CFDictionarySetValue(query.get(), kSecReturnRef, kCFBooleanTrue);
    err = SecItemCopyMatching(query.get(), (CFTypeRef*)&identity.get());
    if (err == noErr)
    {
        UniqueRef<SecCertificateRef> cert2;
        if ((err = SecIdentityCopyCertificate(identity.get(), &cert2.get())) == noErr)
        {
            err = CFEqual(cert2.get(), cert.get()) ? static_cast<OSStatus>(noErr) : errSecItemNotFound;
        }
    }
#else
    err = SecIdentityCreateWithCertificate(keychain.get(), cert.get(), &identity.get());
#endif
    if (err != noErr)
    {
        ostringstream os;
        os << "SSL transport: couldn't create identity for certificate found in the keychain:\n"
           << sslErrorToString(err);
        throw InitializationException(__FILE__, __LINE__, os.str());
    }
    CFArraySetValueAtIndex(const_cast<CFMutableArrayRef>(items.get()), 0, identity.get());
    return items.release();
}

#ifdef ICE_USE_SECURE_TRANSPORT_IOS
string
Ice::SSL::getSubjectName(SecCertificateRef)
{
    throw FeatureNotSupportedException{__FILE__, __LINE__, "IceSSL on iOS does not support getSubjectName"};
}

vector<pair<int, string>>
Ice::SSL::getSubjectAltNames(SecCertificateRef)
{
    throw FeatureNotSupportedException{__FILE__, __LINE__, "IceSSL on iOS does not support getSubjectAltNames"};
}

string
Ice::SSL::encodeCertificate(SecCertificateRef)
{
    throw FeatureNotSupportedException{__FILE__, __LINE__, "IceSSL on iOS does not support encodeCertificate"};
}

SecCertificateRef
Ice::SSL::decodeCertificate(const string&)
{
    throw FeatureNotSupportedException{__FILE__, __LINE__, "IceSSL on iOS does not support decodeCertificate"};
}
#else // macOS
string
Ice::SSL::getSubjectName(SecCertificateRef certificate)
{
    return getX509Name(certificate, kSecOIDX509V1SubjectName);
}

vector<pair<int, string>>
Ice::SSL::getSubjectAltNames(SecCertificateRef certificate)
{
    return getX509AltName(certificate, kSecOIDSubjectAltName);
}

string
Ice::SSL::encodeCertificate(SecCertificateRef certificate)
{
    UniqueRef<CFDataRef> exported;
    OSStatus err = SecItemExport(certificate, kSecFormatPEMSequence, kSecItemPemArmour, nullptr, &exported.get());
    if (err != noErr)
    {
        throw CertificateEncodingException(__FILE__, __LINE__, sslErrorToString(err));
    }
    return {
        reinterpret_cast<const char*>(CFDataGetBytePtr(exported.get())),
        static_cast<size_t>(CFDataGetLength(exported.get()))};
}

SecCertificateRef
Ice::SSL::decodeCertificate(const string& data)
{
    UniqueRef<CFDataRef> buffer(CFDataCreateWithBytesNoCopy(
        kCFAllocatorDefault,
        reinterpret_cast<const UInt8*>(data.c_str()),
        static_cast<CFIndex>(data.size()),
        kCFAllocatorNull));

    SecExternalFormat format = kSecFormatUnknown;
    SecExternalItemType type = kSecItemTypeCertificate;

    SecItemImportExportKeyParameters params;
    memset(&params, 0, sizeof(params));
    params.version = SEC_KEY_IMPORT_EXPORT_PARAMS_VERSION;

    UniqueRef<CFArrayRef> items;
    OSStatus err = SecItemImport(buffer.get(), nullptr, &format, &type, 0, &params, nullptr, &items.get());
    if (err)
    {
        throw CertificateEncodingException(__FILE__, __LINE__, sslErrorToString(err));
    }

    UniqueRef<SecKeychainItemRef> item;
    item.retain(static_cast<SecKeychainItemRef>(const_cast<void*>(CFArrayGetValueAtIndex(items.get(), 0))));
    assert(SecCertificateGetTypeID() == CFGetTypeID(item.get()));
    return reinterpret_cast<SecCertificateRef>(item.release());
}
#endif
