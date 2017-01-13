// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/PermissionsVerifier.h>
#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Ice/UniqueRef.h>

#include <IceUtil/FileUtil.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/InputUtil.h>
#include <IceUtil/Mutex.h>

#include <fstream>

#if defined(__GLIBC__) || defined(_AIX)
#   include <crypt.h>
#elif defined(__FreeBSD__)
#   include <unistd.h>
#elif defined(__APPLE__)
#   include <CoreFoundation/CoreFoundation.h>
#   include <Security/Security.h>
#   include <CommonCrypto/CommonCrypto.h>
#elif defined(_WIN32)
#   include <Bcrypt.h>
#   include <Wincrypt.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace Glacier2;

namespace
{

#if defined(__FreeBSD__) && !defined(__GLIBC__)

//
// FreeBSD crypt is no reentrat we use this global mutex
// to serialize access.
//
IceUtil::Mutex* _staticMutex = 0;

class Init
{
public:

    Init()
    {
        _staticMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete _staticMutex;
        _staticMutex = 0;
    }
};

Init init;
#endif


class CryptPermissionsVerifierI : public PermissionsVerifier
{
public:

    CryptPermissionsVerifierI(const map<string, string>&);

    virtual bool checkPermissions(const string&, const string&, string&, const Ice::Current&) const;

private:

    const map<string, string> _passwords;
    IceUtil::Mutex _cryptMutex; // for old thread-unsafe crypt()
};

class CryptPermissionsVerifierPlugin : public Ice::Plugin
{
public:

    CryptPermissionsVerifierPlugin(const CommunicatorPtr&);

    virtual void initialize();
    virtual void destroy();

private:

    CommunicatorPtr _communicator;
};


map<string, string>
retrievePasswordMap(const string& file)
{
    ifstream passwordFile(IceUtilInternal::streamFilename(file).c_str());
    if(!passwordFile)
    {
        string err = IceUtilInternal::lastErrorToString();
        throw Ice::InitializationException(__FILE__, __LINE__, "cannot open `" + file + "' for reading: " + err);
    }
    map<string, string> passwords;

    while(true)
    {
        string userId;
        passwordFile >> userId;
        if(!passwordFile)
        {
            break;
        }

        string password;
        passwordFile >> password;
        if(!passwordFile)
        {
            break;
        }

        assert(!userId.empty());
        assert(!password.empty());
        passwords.insert(make_pair(userId, password));
    }
    return passwords;
}

CryptPermissionsVerifierI::CryptPermissionsVerifierI(const map<string, string>& passwords) :
    _passwords(passwords)
{
}

namespace
{

#if defined(__APPLE__) || defined(_WIN32)

const string padBytes0 = "";
const string padBytes1 = "=";
const string padBytes2 = "==";

inline string
paddingBytes(size_t length)
{
    switch(length % 4)
    {
        case 2:
        {
            return padBytes2;
        }
        case 3:
        {
            return padBytes1;
        }
        default:
        {
            return padBytes0;
        }
    }
}
#endif

}
bool
CryptPermissionsVerifierI::checkPermissions(const string& userId, const string& password, string&, const Current&) const
{
    map<string, string>::const_iterator p = _passwords.find(userId);

    if(p == _passwords.end())
    {
        return false;
    }
#if defined(__GLIBC__) || defined(__FreeBSD__)
    size_t i = p->second.rfind('$');
    string salt;
    if(i == string::npos)
    {
        //
        // Crypt DES
        //
        if(p->second.size() != 13) // DES passwords are 13 characters long.
        {
            return false;
        }
        salt = p->second.substr(0, 2);
    }
    else
    {
        salt = p->second.substr(0, i + 1);
        if(salt.empty())
        {
            return false;
        }
    }
#   if defined(__GLIBC__)
    struct crypt_data data;
    data.initialized = 0;
    return p->second == crypt_r(password.c_str(), salt.c_str(), &data);
#   else
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(_staticMutex);
    return p->second == crypt(password.c_str(), salt.c_str());
#   endif
#elif defined(__APPLE__) || defined(_WIN32)
    //
    // Pbkdf2 string format:
    //
    // $pbkdf2-digest$rounds$salt$checksum
    // $pbkdf2$rounds$salt$checksum (SHA1 digest)
    //
    size_t beg = 0;
    size_t end = 0;

    //
    // Determine the digest algorithm
    //
#   if defined(__APPLE__)
    CCPseudoRandomAlgorithm algorithmId = 0;
#   else
    LPCWSTR algorithmId = 0;
#   endif
    int checksumLength = 0;

    const string pbkdf2SHA1Token = "$pbkdf2$";

    if(p->second.find(pbkdf2SHA1Token) == 0)
    {
#   if defined(__APPLE__)
        algorithmId = kCCPRFHmacAlgSHA1;
#   else
        algorithmId = BCRYPT_SHA1_ALGORITHM;
#   endif
        checksumLength = 20;
        beg = pbkdf2SHA1Token.size();
    }
    else
    {
        //
        // Pbkdf2 string format:
        //
        // $pbkdf2-digest$rounds$salt$checksum
        //
        const string pbkdf2Token = "$pbkdf2-";
        if(p->second.find(pbkdf2Token) != 0)
        {
            return false; // PBKDF2 start token not found
        }

        beg = pbkdf2Token.size();
        end = p->second.find('$', beg);
        if(end == string::npos)
        {
            return false; // Digest algorithm end token not found
        }

        if(p->second.substr(beg, (end - beg)) == "sha256")
        {
#   if defined(__APPLE__)
            algorithmId = kCCPRFHmacAlgSHA256;
#   else
            algorithmId = BCRYPT_SHA256_ALGORITHM;
#   endif
            checksumLength = 32;
        }
        else if(p->second.substr(beg, (end - beg)) == "sha512")
        {
#   if defined(__APPLE__)
            algorithmId = kCCPRFHmacAlgSHA512;
#   else
            algorithmId = BCRYPT_SHA512_ALGORITHM;
#   endif
            checksumLength = 64;
        }
        else
        {
            return false; // Invalid digest algorithm
        }
        beg = end + 1;
    }
    //
    // Determine the number of rounds
    //
    end = p->second.find('$', beg);
    if(end == string::npos)
    {
        return false; // Rounds end token not found
    }

    IceUtil::Int64 rounds = 0;
    if(!IceUtilInternal::stringToInt64(p->second.substr(beg, (end - beg)), rounds))
    {
        return false; // Invalid rounds value
    }

    //
    // Determine salt and checksum
    //
    beg = end + 1;
    end = p->second.find('$', beg);
    if(end == string::npos)
    {
        return false; // Salt value end token not found
    }

    string salt = p->second.substr(beg, (end - beg));
    string checksum = p->second.substr(end + 1);
    if(checksum.empty())
    {
        return false;
    }

    //
    // passlib encoding is identical to base64 except that it uses . instead of +,
    // and omits trailing padding = and whitepsace.
    //
    std::replace(salt.begin(), salt.end(), '.', '+');
    salt += paddingBytes(salt.size());

    std::replace(checksum.begin(), checksum.end(), '.', '+');
    checksum += paddingBytes(checksum.size());
#   if defined(__APPLE__)
    UniqueRef<CFErrorRef> error;
    UniqueRef<SecTransformRef> decoder(SecDecodeTransformCreate(kSecBase64Encoding, &error.get()));
    if(error)
    {
        return false;
    }

    UniqueRef<CFDataRef> data(CFDataCreateWithBytesNoCopy(kCFAllocatorDefault,
                                                          reinterpret_cast<const uint8_t*>(salt.c_str()),
                                                          salt.size(), kCFAllocatorNull));

    SecTransformSetAttribute(decoder.get(), kSecTransformInputAttributeName, data.get(), &error.get());
    if(error)
    {
        return false;
    }

    UniqueRef<CFDataRef> saltBuffer(static_cast<CFDataRef>(SecTransformExecute(decoder.get(), &error.get())));
    if(error)
    {
        return false;
    }

    vector<uint8_t> checksumBuffer1(checksumLength);
    OSStatus status = CCKeyDerivationPBKDF(kCCPBKDF2, password.c_str(), password.size(),
                                           CFDataGetBytePtr(saltBuffer.get()), CFDataGetLength(saltBuffer.get()),
                                           algorithmId, rounds, &checksumBuffer1[0], checksumLength);
    if(status != errSecSuccess)
    {
        return false;
    }

    decoder.reset(SecDecodeTransformCreate(kSecBase64Encoding, &error.get()));
    if(error)
    {
        return false;
    }
    data.reset(CFDataCreateWithBytesNoCopy(kCFAllocatorDefault,
                                           reinterpret_cast<const uint8_t*>(checksum.c_str()),
                                           checksum.size(), kCFAllocatorNull));
    SecTransformSetAttribute(decoder.get(), kSecTransformInputAttributeName, data.get(), &error.get());
    if(error)
    {
        return false;
    }

    data.reset(static_cast<CFDataRef>(SecTransformExecute(decoder.get(), &error.get())));
    if(error)
    {
        return false;
    }

    vector<uint8_t> checksumBuffer2(CFDataGetBytePtr(data.get()), CFDataGetBytePtr(data.get()) + CFDataGetLength(data.get()));
    return checksumBuffer1 == checksumBuffer2;
#   else
    DWORD saltLength = static_cast<DWORD>(salt.size());
    vector<BYTE> saltBuffer(saltLength);

    if(!CryptStringToBinary(salt.c_str(), static_cast<DWORD>(salt.size()), CRYPT_STRING_BASE64, &saltBuffer[0], &saltLength, 0, 0))
    {
        return false;
    }
    saltBuffer.resize(saltLength);

    BCRYPT_ALG_HANDLE algorithmHandle = 0;
    if(BCryptOpenAlgorithmProvider(&algorithmHandle, algorithmId, 0, BCRYPT_ALG_HANDLE_HMAC_FLAG) != 0)
    {
        return false;
    }

    vector<BYTE> checksumBuffer1(checksumLength);

    vector<BYTE> passwordBuffer(password.begin(), password.end());

    DWORD status = BCryptDeriveKeyPBKDF2(algorithmHandle, &passwordBuffer[0],
                                         static_cast<DWORD>(passwordBuffer.size()),
                                         &saltBuffer[0], saltLength, rounds,
                                         &checksumBuffer1[0], static_cast<DWORD>(checksumLength), 0);

    BCryptCloseAlgorithmProvider(algorithmHandle, 0);

    if(status != 0)
    {
        return false;
    }

    DWORD checksumBuffer2Length = checksumLength;
    vector<BYTE> checksumBuffer2(checksumLength);

    if(!CryptStringToBinary(checksum.c_str(), static_cast<DWORD>(checksum.size()),
                            CRYPT_STRING_BASE64, &checksumBuffer2[0],
                            &checksumBuffer2Length, 0, 0))
    {
        return false;
    }
    return checksumBuffer1 == checksumBuffer2;
#   endif
#else
    // Fallback to plain crypt() - DES-style

    if(p->second.size() != 13)
    {
        return false;
    }
    string salt = p->second.substr(0, 2);

    IceUtil::Mutex::Lock lock(_cryptMutex);
    return p->second == crypt(password.c_str(), salt.c_str());

#endif
}


CryptPermissionsVerifierPlugin::CryptPermissionsVerifierPlugin(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
CryptPermissionsVerifierPlugin::initialize()
{
    const string prefix = "Glacier2CryptPermissionsVerifier.";
    const PropertyDict props = _communicator->getProperties()->getPropertiesForPrefix(prefix);

    if(!props.empty())
    {
        ObjectAdapterPtr adapter = _communicator->createObjectAdapter(""); // colloc-only adapter

        // Each prop represents a property to set + the associated password file

        for(PropertyDict::const_iterator p = props.begin(); p != props.end(); ++p)
        {
            string name = p->first.substr(prefix.size());
            Identity id;
            id.name = Ice::generateUUID();
            id.category = "Glacier2CryptPermissionsVerifier";
            ObjectPrx prx = adapter->add(new CryptPermissionsVerifierI(retrievePasswordMap(p->second)), id);
            _communicator->getProperties()->setProperty(name, _communicator->proxyToString(prx));
        }

        adapter->activate();
    }
}

void
CryptPermissionsVerifierPlugin::destroy()
{
}

}

#ifndef CRYPT_PERMISSIONS_VERIFIER_API
#   ifdef CRYPT_PERMISSIONS_VERIFIER_API_EXPORTS
#       define CRYPT_PERMISSIONS_VERIFIER_API ICE_DECLSPEC_EXPORT
#   else
#       define CRYPT_PERMISSIONS_VERIFIER_API /**/
#   endif
#endif

//
// Plug-in factory function.
//
extern "C"
{

CRYPT_PERMISSIONS_VERIFIER_API Ice::Plugin*
createCryptPermissionsVerifier(const CommunicatorPtr& communicator, const string& name, const StringSeq& args)
{
    if(args.size() > 0)
    {
        Error out(communicator->getLogger());
        out << "Plugin " << name << ": too many arguments";
        return 0;
    }

    return new CryptPermissionsVerifierPlugin(communicator);
}

}
