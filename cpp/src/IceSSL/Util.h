// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_UTIL_H
#define ICE_SSL_UTIL_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>

#include <IceSSL/Plugin.h>

#if defined(ICE_USE_OPENSSL)
#  include <openssl/ssl.h>
#  include <list>
#elif defined(ICE_USE_SECURE_TRANSPORT)
#  include <Security/Security.h>
#  include <CoreFoundation/CoreFoundation.h>
#elif defined(ICE_USE_SCHANNEL)
#  include <wincrypt.h>
#endif

namespace IceSSL
{

//
// Constants for X509 certificate alt names (AltNameOther, AltNameORAddress, AltNameEDIPartyName and
// AltNameObjectIdentifier) are not supported.
//

//const int AltNameOther = 0;
const int AltNameEmail = 1;
const int AltNameDNS = 2;
//const int AltNameORAddress = 3;
const int AltNameDirectory = 4;
//const int AltNameEDIPartyName = 5;
const int AltNameURL = 6;
const int AltNAmeIP = 7;
//const AltNameObjectIdentifier = 8;

#ifdef ICE_USE_OPENSSL

#  ifndef OPENSSL_NO_DH
class DHParams : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    DHParams();
    ~DHParams();

    bool add(int, const std::string&);
    DH* get(int);

private:

    typedef std::pair<int, DH*> KeyParamPair;
    typedef std::list<KeyParamPair> ParamList;
    ParamList _params;

    DH* _dh512;
    DH* _dh1024;
    DH* _dh2048;
    DH* _dh4096;
};
typedef IceUtil::Handle<DHParams> DHParamsPtr;
#  endif

//
// Accumulate the OpenSSL error stack into a string.
//
std::string getSslErrors(bool);

#elif defined(ICE_USE_SECURE_TRANSPORT)

template<typename T>
class UniqueRef
{
public:

    explicit UniqueRef(CFTypeRef ptr = 0) : _ptr((T)ptr)
    {
    }

    ~UniqueRef()
    {
        if(_ptr != 0)
        {
            CFRelease(_ptr);
        }
    }

    T release()
    {
        T r = _ptr;
        _ptr = 0;
        return r;
    }

    void reset(CFTypeRef ptr = 0)
    {
        if(_ptr == ptr)
        {
            return;
        }
        if(_ptr != 0)
        {
            CFRelease(_ptr);
        }
        _ptr = (T)ptr;
    }

    void retain(CFTypeRef ptr)
    {
        reset(ptr ? CFRetain(ptr) : ptr);
    }

    T get() const
    {
        return _ptr;
    }

    operator bool() const
    {
        return _ptr != 0;
    }

private:

    T _ptr;
};

//
// Helper functions to use by Secure Transport.
//

std::string fromCFString(CFStringRef);

inline CFStringRef
toCFString(const std::string& s)
{
    return CFStringCreateWithCString(NULL, s.c_str(), kCFStringEncodingUTF8);
}

std::string errorToString(CFErrorRef);
std::string errorToString(OSStatus);

//
// Retrieve a certificate property
//
CFDictionaryRef getCertificateProperty(SecCertificateRef, CFTypeRef);

//
// Read a private key from an file and associate it to the given certificate.
//
SecIdentityRef loadPrivateKey(const std::string&, SecCertificateRef, SecKeychainRef, const std::string&,
                              const PasswordPromptPtr&, int);

//
// Read certificate from a file.
//
CFArrayRef loadCertificateChain(const std::string&, const std::string&, SecKeychainRef, const std::string&,
                                const PasswordPromptPtr&, int);

SecCertificateRef loadCertificate(const std::string&);
CFArrayRef loadCACertificates(const std::string&);

SecCertificateRef findCertificate(SecKeychainRef, const std::string&);

#elif defined(ICE_USE_SCHANNEL)
std::vector<PCCERT_CONTEXT>
findCertificates(const std::string&, const std::string&, const std::string&, std::vector<HCERTSTORE>&);
#endif

//
// Read a file into memory buffer.
//
void readFile(const std::string&, std::vector<char>&);

//
// Determine if a file or directory exists, with an optional default
// directory.
//
bool checkPath(const std::string&, const std::string&, bool, std::string&);

}

#endif
