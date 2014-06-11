// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_UTIL_H
#define ICE_SSL_UTIL_H

#include <IceSSL/Config.h>
#include <IceSSL/UtilF.h>
#include <Ice/Network.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <IceUtil/ScopedArray.h>

#include <IceSSL/Plugin.h>

#include <list>

#ifdef ICE_USE_OPENSSL
#  include <openssl/ssl.h>
#else
#  include <Security/Security.h>
#  include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef ICE_USE_OPENSSL
namespace IceSSL
{

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
#  endif

//
// Accumulate the OpenSSL error stack into a string.
//
std::string getSslErrors(bool);

}
#elif defined(ICE_USE_SECURE_TRANSPORT)

namespace IceSSL
{

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
// Read a while file into memory buffer and return the number of bytes read.
//
int readFile(const std::string&, IceUtil::ScopedArray<char>&);


//
// Retrieve a certificate property
//
CFDictionaryRef
getCertificateProperty(SecCertificateRef, CFTypeRef);


std::string keyLabel(SecCertificateRef);

//
// Read a private key from an file and optionaly import into a keychain.
//
void loadPrivateKey(SecKeyRef*, const std::string&, CFDataRef, SecKeychainRef,
                    const std::string&, const std::string&, const PasswordPromptPtr&, 
                    int);

//
// Read a certificate and key from an file and optionaly import then into a 
// keychain.
//
void loadCertificate(SecCertificateRef*, CFDataRef*, SecKeyRef*, SecKeychainRef, 
                     const std::string&, const std::string& = "",
                     const PasswordPromptPtr& = 0, int = 0);

CFArrayRef loadCACertificates(const std::string&, const std::string& = "", const PasswordPromptPtr& = 0, 
                              int = 0);

}

#endif

namespace IceSSL
{

//
// Determine if a file or directory exists, with an optional default directory.
//
bool checkPath(std::string&, const std::string&, bool);

}

#endif
