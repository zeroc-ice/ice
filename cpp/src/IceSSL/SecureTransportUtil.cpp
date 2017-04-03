// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/SecureTransportUtil.h>
#include <IceSSL/Plugin.h>

#include <Ice/Base64.h>
#include <Ice/UniqueRef.h>
#include <Ice/LocalException.h>

#include <IceUtil/FileUtil.h>
#include <IceUtil/StringUtil.h>

#include <fstream>

#include <Security/Security.h>
#include <CoreFoundation/CoreFoundation.h>

using namespace Ice;
using namespace IceInternal;
using namespace IceSSL;
using namespace IceSSL::SecureTransport;
using namespace std;


namespace
{

CFMutableDataRef
readCertFile(const string& file)
{
    ifstream is(IceUtilInternal::streamFilename(file).c_str(), ios::in | ios::binary);
    if(!is.good())
    {
        throw CertificateReadException(__FILE__, __LINE__, "error opening file " + file);
    }

    is.seekg(0, is.end);
    size_t size = is.tellg();
    is.seekg(0, is.beg);

    UniqueRef<CFMutableDataRef> data(CFDataCreateMutable(kCFAllocatorDefault, size));
    CFDataSetLength(data.get(), size);
    is.read(reinterpret_cast<char*>(CFDataGetMutableBytePtr(data.get())), size);
    if(!is.good())
    {
        throw CertificateReadException(__FILE__, __LINE__, "error reading file " + file);
    }
    return data.release();
}

}

string
IceSSL::SecureTransport::sslErrorToString(CFErrorRef err)
{
    ostringstream os;
    if(err)
    {
        UniqueRef<CFStringRef> s(CFErrorCopyDescription(err));
        os << "(error: " << CFErrorGetCode(err) << " description: " << fromCFString(s.get()) << ")";
    }
    return os.str();
}

string
IceSSL::SecureTransport::sslErrorToString(OSStatus status)
{
    ostringstream os;
    os << "(error: " << status;
#if defined(ICE_USE_SECURE_TRANSPORT_MACOS)
    UniqueRef<CFStringRef> s(SecCopyErrorMessageString(status, 0));
    if(s)
    {
        os << " description: " << fromCFString(s.get());
    }
#endif
    os << ")";
    return os.str();
}

#if defined(ICE_USE_SECURE_TRANSPORT_MACOS)
CFDictionaryRef
IceSSL::SecureTransport::getCertificateProperty(SecCertificateRef cert, CFTypeRef key)
{
    UniqueRef<CFDictionaryRef> property;
    UniqueRef<CFArrayRef> keys(CFArrayCreate(ICE_NULLPTR, &key , 1, &kCFTypeArrayCallBacks));
    UniqueRef<CFErrorRef> err;
    UniqueRef<CFDictionaryRef> values(SecCertificateCopyValues(cert, keys.get(), &err.get()));
    if(err)
    {
        ostringstream os;
        os << "IceSSL: error getting property for certificate:\n" << sslErrorToString(err);
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
bool
isCA(SecCertificateRef cert)
{
    UniqueRef<CFDictionaryRef> property(getCertificateProperty(cert, kSecOIDBasicConstraints));
    if(property)
    {
        CFArrayRef propertyValues = static_cast<CFArrayRef>(CFDictionaryGetValue(property.get(), kSecPropertyKeyValue));
        for(int i = 0, size = CFArrayGetCount(propertyValues); i < size; ++i)
        {
            CFDictionaryRef dict = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(propertyValues, i));
            CFStringRef label = static_cast<CFStringRef>(CFDictionaryGetValue(dict, kSecPropertyKeyLabel));
            if(CFEqual(label, CFSTR("Certificate Authority")))
            {
                return CFEqual(static_cast<CFStringRef>(CFDictionaryGetValue(dict, kSecPropertyKeyValue)), CFSTR("Yes"));
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
loadKeychainItems(const string& file, SecExternalItemType type, SecKeychainRef keychain, const string& passphrase,
                  const PasswordPromptPtr& prompt, int retryMax)
{
    UniqueRef<CFMutableDataRef> data(readCertFile(file));

    SecItemImportExportKeyParameters params;
    memset(&params, 0, sizeof(params));
    params.version =  SEC_KEY_IMPORT_EXPORT_PARAMS_VERSION;
    params.flags |= kSecKeyNoAccessControl;
    UniqueRef<CFStringRef> passphraseHolder;
    if(!passphrase.empty())
    {
        passphraseHolder.reset(toCFString(passphrase));
        params.passphrase = passphraseHolder.get();
    }

    UniqueRef<CFArrayRef> items;
    SecExternalItemType importType = type;
    SecExternalFormat format = type == kSecItemTypeUnknown ? kSecFormatPKCS12 : kSecFormatUnknown;
    UniqueRef<CFStringRef> path(toCFString(file));
    OSStatus err = SecItemImport(data.get(), path.get(), &format, &importType, 0, &params, keychain, &items.get());

    //
    // If passphrase failure and no password was configured, we obtain
    // the password from the given prompt or configure the import to
    // prompt the user with an alert dialog.
    //
    UniqueRef<CFStringRef> alertPromptHolder;
    if(passphrase.empty() &&
       (err == errSecPassphraseRequired || err == errSecInvalidData || err == errSecPkcs12VerifyFailure))
    {
        if(!prompt)
        {
            params.flags |= kSecKeySecurePassphrase;
            ostringstream os;
            os << "Enter the password for\n" << file;
            alertPromptHolder.reset(toCFString(os.str()));
            params.alertPrompt = alertPromptHolder.get();
        }

        int count = 0;
        while((err == errSecPassphraseRequired || err == errSecInvalidData || err == errSecPkcs12VerifyFailure) &&
              count < retryMax)
        {
            if(prompt)
            {
                passphraseHolder.reset(toCFString(prompt->getPassword()));
                params.passphrase = passphraseHolder.get();
            }
            err = SecItemImport(data.get(), path.get(), &format, &importType, 0, &params, keychain, &items.get());
            ++count;
        }
    }

    if(err != noErr)
    {
        ostringstream os;
        os << "IceSSL: error reading " << (type == kSecItemTypePrivateKey ? "private key" : "certificate");
        os << " `" << file << "':\n" << sslErrorToString(err);
        throw CertificateReadException(__FILE__, __LINE__, os.str());
    }

    if(type != kSecItemTypeUnknown && importType != kSecItemTypeAggregate && importType != type)
    {
        ostringstream os;
        os << "IceSSL: error reading " << (type == kSecItemTypePrivateKey ? "private key" : "certificate");
        os << " `" << file << "' doesn't contain the expected item";
        throw CertificateReadException(__FILE__, __LINE__, os.str());
    }

    return items.release();
}

SecKeychainRef
openKeychain(const std::string& path, const std::string& keychainPassword)
{
    string keychainPath = path;
    UniqueRef<SecKeychainRef> keychain;
    OSStatus err = 0;
    if(keychainPath.empty())
    {
        if((err = SecKeychainCopyDefault(&keychain.get())))
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: unable to retrieve default keychain:\n" + sslErrorToString(err));
        }
    }
    else
    {
        //
        // KeyChain path is relative to the current working directory.
        //
        if(!IceUtilInternal::isAbsolutePath(keychainPath))
        {
            string cwd;
            if(IceUtilInternal::getcwd(cwd) == 0)
            {
                keychainPath = string(cwd) + '/' + keychainPath;
            }
        }

        if((err = SecKeychainOpen(keychainPath.c_str(), &keychain.get())))
        {
            throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unable to open keychain: `" +
                                                keychainPath + "'\n" + sslErrorToString(err));
        }
    }

    SecKeychainStatus status;
    err = SecKeychainGetStatus(keychain.get(), &status);
    if(err == noErr)
    {
        const char* pass = keychainPassword.empty() ? 0 : keychainPassword.c_str();
        if((err = SecKeychainUnlock(keychain.get(), keychainPassword.size(), pass, pass != 0)))
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: unable to unlock keychain:\n" + sslErrorToString(err));
        }
    }
    else if(err == errSecNoSuchKeychain)
    {
        const char* pass = keychainPassword.empty() ? 0 : keychainPassword.c_str();
        keychain.reset(0);
        if((err = SecKeychainCreate(keychainPath.c_str(), keychainPassword.size(), pass, pass == 0, 0, &keychain.get())))
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: unable to create keychain:\n" + sslErrorToString(err));
        }
    }
    else
    {
        throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: unable to open keychain:\n" + sslErrorToString(err));
    }

    //
    // Set keychain settings to avoid keychain lock.
    //
    SecKeychainSettings settings;
    settings.version = SEC_KEYCHAIN_SETTINGS_VERS1;
    settings.lockOnSleep = FALSE;
    settings.useLockInterval = FALSE;
    settings.lockInterval = INT_MAX;
    if((err = SecKeychainSetSettings(keychain.get(), &settings)))
    {
        throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: error setting keychain settings:\n" + sslErrorToString(err));
    }

    return keychain.release();
}

//
// Imports a certificate private key and optionally add it to a keychain.
//
SecIdentityRef
loadPrivateKey(const string& file, SecCertificateRef cert, SecKeychainRef keychain, const string& password,
               const PasswordPromptPtr& prompt, int retryMax)
{
    //
    // Check if we already imported the certificate
    //
    UniqueRef<CFDataRef> hash;
    UniqueRef<CFDictionaryRef> subjectKeyProperty(getCertificateProperty(cert, kSecOIDSubjectKeyIdentifier));
    if(subjectKeyProperty)
    {
        CFArrayRef values = static_cast<CFArrayRef>(CFDictionaryGetValue(subjectKeyProperty.get(),
                                                                         kSecPropertyKeyValue));
        for(int i = 0; i < CFArrayGetCount(values); ++i)
        {
            CFDictionaryRef dict = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(values, i));
            if(CFEqual(CFDictionaryGetValue(dict, kSecPropertyKeyLabel), CFSTR("Key Identifier")))
            {
                hash.retain(static_cast<CFDataRef>(CFDictionaryGetValue(dict, kSecPropertyKeyValue)));
                break;
            }
        }
    }

    const void* values[] = { keychain };
    UniqueRef<CFArrayRef> searchList(CFArrayCreate(kCFAllocatorDefault, values, 1, &kCFTypeArrayCallBacks));

    UniqueRef<CFMutableDictionaryRef> query(CFDictionaryCreateMutable(0,
                                                                      0,
                                                                      &kCFTypeDictionaryKeyCallBacks,
                                                                      &kCFTypeDictionaryValueCallBacks));

    CFDictionarySetValue(query.get(), kSecClass, kSecClassCertificate);
    CFDictionarySetValue(query.get(), kSecMatchLimit, kSecMatchLimitOne);
    CFDictionarySetValue(query.get(), kSecMatchSearchList, searchList.get());
    CFDictionarySetValue(query.get(), kSecAttrSubjectKeyID, hash.get());
    CFDictionarySetValue(query.get(), kSecReturnRef, kCFBooleanTrue);

    UniqueRef<CFTypeRef> value(0);
    OSStatus err = SecItemCopyMatching(query.get(), &value.get());
    UniqueRef<SecCertificateRef> item(static_cast<SecCertificateRef>(const_cast<void*>(value.release())));
    if(err == noErr)
    {
        //
        // If the certificate has already been imported, create the
        // identity. The key should also have been imported.
        //
        UniqueRef<SecIdentityRef> identity;
        err = SecIdentityCreateWithCertificate(keychain, item.get(), &identity.get());
        if(err != noErr)
        {
            ostringstream os;
            os << "IceSSL: error creating certificate identity:\n" << sslErrorToString(err);
            throw CertificateReadException(__FILE__, __LINE__, os.str());
        }
        return identity.release();
    }
    else if(err != errSecItemNotFound)
    {
        ostringstream os;
        os << "IceSSL: error searching for keychain items:\n" << sslErrorToString(err);
        throw CertificateReadException(__FILE__, __LINE__, os.str());
    }

    //
    // If the certificate isn't already in the keychain, load the
    // private key into the keychain and add the certificate.
    //
    UniqueRef<CFArrayRef> items(loadKeychainItems(file, kSecItemTypePrivateKey, keychain, password, prompt, retryMax));
    int count = CFArrayGetCount(items.get());
    UniqueRef<SecKeyRef> key;
    for(int i = 0; i < count; ++i)
    {
        SecKeychainItemRef item =
            static_cast<SecKeychainItemRef>(const_cast<void*>(CFArrayGetValueAtIndex(items.get(), 0)));
        if(SecKeyGetTypeID() == CFGetTypeID(item))
        {
            key.retain(reinterpret_cast<SecKeyRef>(item));
            break;
        }
    }
    if(!key)
    {
        throw CertificateReadException(__FILE__, __LINE__, "IceSSL: no key in file `" + file + "'");
    }

    //
    // Add the certificate to the keychain
    //
    query.reset(CFDictionaryCreateMutable(kCFAllocatorDefault,
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
    if(err != noErr)
    {
        ostringstream os;
        os << "IceSSL: failure adding certificate to keychain\n" << sslErrorToString(err);
        throw CertificateReadException(__FILE__, __LINE__, os.str());
    }
    item.retain(static_cast<SecCertificateRef>(const_cast<void*>(CFArrayGetValueAtIndex(added.get(), 0))));

    //
    // Create the association between the private  key and the certificate,
    // kSecKeyLabel attribute should match the subject key identifier.
    //
    vector<SecKeychainAttribute> attributes;
    if(hash)
    {
        SecKeychainAttribute attr;
        attr.tag = kSecKeyLabel;
        attr.data = const_cast<UInt8*>(CFDataGetBytePtr(hash.get()));
        attr.length = CFDataGetLength(hash.get());
        attributes.push_back(attr);
    }

    //
    // kSecKeyPrintName attribute correspond to the keychain display
    // name.
    //
    string label;
    UniqueRef<CFStringRef> commonName(0);
    if(SecCertificateCopyCommonName(item.get(), &commonName.get()) == noErr)
    {
        label = fromCFString(commonName.get());
        SecKeychainAttribute attr;
        attr.tag = kSecKeyPrintName;
        attr.data = const_cast<char*>(label.c_str());
        attr.length = label.size();
        attributes.push_back(attr);
    }

    SecKeychainAttributeList attrs;
    attrs.attr = &attributes[0];
    attrs.count = attributes.size();
    SecKeychainItemModifyAttributesAndData(reinterpret_cast<SecKeychainItemRef>(key.get()), &attrs, 0, 0);

    UniqueRef<SecIdentityRef> identity;
    err = SecIdentityCreateWithCertificate(keychain, item.get(), &identity.get());
    if(err != noErr)
    {
        ostringstream os;
        os << "IceSSL: error creating certificate identity:\n" << sslErrorToString(err);
        throw CertificateReadException(__FILE__, __LINE__, os.str());
    }
    return identity.release();
}

} // anonymous namespace end

#else

namespace
{

CFArrayRef
loadCerts(const string& file)
{
    UniqueRef<CFArrayRef> certs(CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks));
    if(file.find(".pem") != string::npos)
    {
        vector<char> buffer;
        readFile(file, buffer);
        string strbuf(buffer.begin(), buffer.end());
        string::size_type size, startpos, endpos = 0;
        bool first = true;
        while(true)
        {
            startpos = strbuf.find("-----BEGIN CERTIFICATE-----", endpos);
            if(startpos != string::npos)
            {
                startpos += sizeof("-----BEGIN CERTIFICATE-----");
                endpos = strbuf.find("-----END CERTIFICATE-----", startpos);
                if(endpos == string::npos)
                {
                    InitializationException ex(__FILE__, __LINE__);
                    ex.reason = "IceSSL: certificate " + file + " is not a valid PEM-encoded certificate";
                    throw ex;
                }
                size = endpos - startpos;
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

            vector<unsigned char> data(IceInternal::Base64::decode(string(&buffer[startpos], size)));
            UniqueRef<CFDataRef> certdata(CFDataCreate(kCFAllocatorDefault, &data[0], data.size()));
            UniqueRef<SecCertificateRef> cert(SecCertificateCreateWithData(0, certdata.get()));
            if(!cert)
            {
                InitializationException ex(__FILE__, __LINE__);
                ex.reason = "IceSSL: certificate " + file + " is not a valid PEM-encoded certificate";
                throw ex;
            }
            CFArrayAppendValue(const_cast<CFMutableArrayRef>(certs.get()), cert.get());
            first = false;
        }
    }
    else
    {
        UniqueRef<CFDataRef> data(readCertFile(file));
        UniqueRef<SecCertificateRef> cert(SecCertificateCreateWithData(0, data.get()));
        if(!cert)
        {
            InitializationException ex(__FILE__, __LINE__);
            ex.reason = "IceSSL: certificate " + file + " is not a valid DER-encoded certificate";
            throw ex;
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
IceSSL::SecureTransport::loadCertificateChain(const string& file,
                                              const string& keyFile,
                                              const std::string& keychainPath,
                                              const string& keychainPassword,
                                              const string& password,
                                              const PasswordPromptPtr& prompt,
                                              int retryMax)
{
    UniqueRef<CFArrayRef> chain;
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    UniqueRef<CFDataRef> cert(readCertFile(file));

    UniqueRef<CFMutableDictionaryRef> settings(CFDictionaryCreateMutable(0,
                                                                         1,
                                                                         &kCFTypeDictionaryKeyCallBacks,
                                                                         &kCFTypeDictionaryValueCallBacks));
    UniqueRef<CFArrayRef> items;
    OSStatus err;
    int count = 0;
    do
    {
        UniqueRef<CFStringRef> pass(toCFString(password.empty() && prompt ? prompt->getPassword() : password));
        CFDictionarySetValue(settings.get(), kSecImportExportPassphrase, pass.get());
        err = SecPKCS12Import(cert.get(), settings.get(), &items.get());
        ++count;
    }
    while(password.empty() && prompt && err == errSecAuthFailed && count < retryMax);

    if(err != noErr)
    {
        ostringstream os;
        os << "IceSSL: unable to import certificate from file " << file << " (error = " << err << ")";
        throw InitializationException(__FILE__, __LINE__, os.str());
    }

    for(int i = 0; i < CFArrayGetCount(items.get()); ++i)
    {
        CFDictionaryRef dict = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(items.get(), i));
        SecIdentityRef identity = static_cast<SecIdentityRef>(
            const_cast<void*>(CFDictionaryGetValue(dict, kSecImportItemIdentity)));
        if(identity)
        {
            CFArrayRef certs = static_cast<CFArrayRef>(CFDictionaryGetValue(dict, kSecImportItemCertChain));
            chain.reset(CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, certs));
            CFArraySetValueAtIndex(const_cast<CFMutableArrayRef>(chain.get()), 0, identity);
        }
    }

    if(!chain)
    {
        ostringstream os;
        os << "IceSSL: couldn't find identity in file " << file;
        throw InitializationException(__FILE__, __LINE__, os.str());
    }
#else
    UniqueRef<SecKeychainRef> keychain(openKeychain(keychainPath, keychainPassword));
    if(keyFile.empty())
    {
        chain.reset(loadKeychainItems(file, kSecItemTypeUnknown, keychain.get(), password, prompt, retryMax));
    }
    else
    {
        //
        // Load the certificate, don't load into the keychain as it
        // might already have been imported.
        //
        UniqueRef<CFArrayRef> items(loadKeychainItems(file, kSecItemTypeCertificate, 0, password, prompt, retryMax));
        SecCertificateRef cert =
            static_cast<SecCertificateRef>(const_cast<void*>(CFArrayGetValueAtIndex(items.get(), 0)));
        if(SecCertificateGetTypeID() != CFGetTypeID(cert))
        {
            ostringstream os;
            os << "IceSSL: couldn't find certificate in `" << file << "'";
            throw CertificateReadException(__FILE__, __LINE__, os.str());
        }

        //
        // Load the private key for the given certificate. This will
        // add the certificate/key to the keychain if they aren't
        // already present in the keychain.
        //
        UniqueRef<SecIdentityRef> identity(loadPrivateKey(keyFile, cert, keychain.get(), password, prompt, retryMax));
        chain.reset(CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, items.get()));
        CFArraySetValueAtIndex(const_cast<CFMutableArrayRef>(chain.get()), 0, identity.get());
    }
#endif
    return chain.release();
}

SecCertificateRef
IceSSL::SecureTransport::loadCertificate(const string& file)
{
    UniqueRef<SecCertificateRef> cert;
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    UniqueRef<CFArrayRef> certs(loadCerts(file));
    assert(CFArrayGetCount(certs.get()) > 0);
    cert.retain((SecCertificateRef)CFArrayGetValueAtIndex(certs.get(), 0));
#else
    UniqueRef<CFArrayRef> items(loadKeychainItems(file, kSecItemTypeCertificate, 0, "", 0, 0));
    cert.retain((SecCertificateRef)CFArrayGetValueAtIndex(items.get(), 0));
#endif
    return cert.release();
}

CFArrayRef
IceSSL::SecureTransport::loadCACertificates(const string& file)
{
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    return loadCerts(file);
#else
    UniqueRef<CFArrayRef> items(loadKeychainItems(file, kSecItemTypeCertificate, 0, "", 0, 0));
    UniqueRef<CFArrayRef> certificateAuthorities(CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks));
    int count = CFArrayGetCount(items.get());
    for(CFIndex i = 0; i < count; ++i)
    {
        SecCertificateRef cert =
            static_cast<SecCertificateRef>(const_cast<void*>(CFArrayGetValueAtIndex(items.get(), i)));
        assert(SecCertificateGetTypeID() == CFGetTypeID(cert));
        if(isCA(cert))
        {
            CFArrayAppendValue(const_cast<CFMutableArrayRef>(certificateAuthorities.get()), cert);
        }
    }
    return certificateAuthorities.release();
#endif
}

CFArrayRef
IceSSL::SecureTransport::findCertificateChain(const std::string& keychainPath,
                                              const std::string& keychainPassword,
                                              const string& value)
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
    UniqueRef<CFMutableDictionaryRef> query(CFDictionaryCreateMutable(0,
                                                                      0,
                                                                      &kCFTypeDictionaryKeyCallBacks,
                                                                      &kCFTypeDictionaryValueCallBacks));

#if defined(ICE_USE_SECURE_TRANSPORT_MACOS)
    UniqueRef<SecKeychainRef> keychain(openKeychain(keychainPath, keychainPassword));
    const void* values[] = { keychain.get() };
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
    while((pos = value.find(':', start)) != string::npos)
    {
        string field = IceUtilInternal::toUpper(IceUtilInternal::trim(value.substr(start, pos - start)));
        string arg;
        if(field != "LABEL" && field != "SERIAL" && field != "SUBJECT" && field != "SUBJECTKEYID")
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
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unmatched quote in `" + value + "'");
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

        if(field == "SUBJECT" || field == "LABEL")
        {
            UniqueRef<CFStringRef> v(toCFString(arg));
            CFDictionarySetValue(query.get(), field == "LABEL" ? kSecAttrLabel : kSecMatchSubjectContains, v.get());
            valid = true;
        }
        else if(field == "SUBJECTKEYID" || field == "SERIAL")
        {
            vector<unsigned char> buffer;
            if(!parseBytes(arg, buffer))
            {
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: invalid value `" + value + "'");
            }
            UniqueRef<CFDataRef> v(CFDataCreate(kCFAllocatorDefault, &buffer[0], buffer.size()));
            CFDictionarySetValue(query.get(), field == "SUBJECTKEYID" ? kSecAttrSubjectKeyID : kSecAttrSerialNumber,
                                 v.get());
            valid = true;
        }
    }

    if(!valid)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: invalid value `" + value + "'");
    }

    UniqueRef<SecCertificateRef> cert;
    OSStatus err = SecItemCopyMatching(query.get(), (CFTypeRef*)&cert.get());
    if(err != noErr)
    {
        throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: find certificate `" + value + "' failed:\n" + sslErrorToString(err));
    }

    //
    // Retrieve the certificate chain
    //
    UniqueRef<SecPolicyRef> policy(SecPolicyCreateSSL(true, 0));
    UniqueRef<SecTrustRef> trust;
    err = SecTrustCreateWithCertificates(reinterpret_cast<CFArrayRef>(cert.get()), policy.get(), &trust.get());
    if(err || !trust)
    {
        throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: error creating trust object" +
                                            (err ? ":\n" + sslErrorToString(err) : ""));
    }

    SecTrustResultType trustResult;
    if((err = SecTrustEvaluate(trust.get(), &trustResult)))
    {
        throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: error evaluating trust:\n" + sslErrorToString(err));
    }

    int chainLength = SecTrustGetCertificateCount(trust.get());
    UniqueRef<CFArrayRef> items(CFArrayCreateMutable(kCFAllocatorDefault, chainLength, &kCFTypeArrayCallBacks));
    for(int i = 0; i < chainLength; ++i)
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
    // using the certicate label. If the user added the identity with SecItemAdd the
    // identity has the same label as the certificate.
    //
    query.reset(CFDictionaryCreateMutable(0, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    CFDictionarySetValue(query.get(), kSecClass, kSecClassCertificate);
    CFDictionarySetValue(query.get(), kSecValueRef, cert.get());
    CFDictionarySetValue(query.get(), kSecReturnAttributes, kCFBooleanTrue);
    UniqueRef<CFDictionaryRef> attributes;
    err = SecItemCopyMatching(query.get(), reinterpret_cast<CFTypeRef*>(&attributes.get()));
    if(err != noErr)
    {
        ostringstream os;
        os << "IceSSL: couldn't create identity for certificate found in the keychain:\n" << sslErrorToString(err);
        throw PluginInitializationException(__FILE__, __LINE__, os.str());
    }

    // Now lookup the identity with the label
    query.reset(CFDictionaryCreateMutable(0, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    CFDictionarySetValue(query.get(), kSecMatchLimit, kSecMatchLimitOne);
    CFDictionarySetValue(query.get(), kSecClass, kSecClassIdentity);
    CFDictionarySetValue(query.get(), kSecAttrLabel, (CFDataRef)CFDictionaryGetValue(attributes.get(), kSecAttrLabel));
    CFDictionarySetValue(query.get(), kSecReturnRef, kCFBooleanTrue);
    err = SecItemCopyMatching(query.get(), (CFTypeRef*)&identity.get());
    if(err == noErr)
    {
        UniqueRef<SecCertificateRef> cert2;
        if((err = SecIdentityCopyCertificate(identity.get(), &cert2.get())) == noErr)
        {
            err = CFEqual(cert2.get(), cert.get()) ? noErr : errSecItemNotFound;
        }
    }
#else
    err = SecIdentityCreateWithCertificate(keychain.get(), cert.get(), &identity.get());
#endif
    if(err != noErr)
    {
        ostringstream os;
        os << "IceSSL: couldn't create identity for certificate found in the keychain:\n" << sslErrorToString(err);
        throw PluginInitializationException(__FILE__, __LINE__, os.str());
    }
    CFArraySetValueAtIndex(const_cast<CFMutableArrayRef>(items.get()), 0, identity.get());
    return items.release();
}
