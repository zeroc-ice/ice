// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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

std::string keyLabel(SecCertificateRef);

//
// Read a private key from an file and optionaly import into a keychain.
//
void loadPrivateKey(SecKeyRef*, const std::string&, CFDataRef, SecKeychainRef, const std::string&, const std::string&, 
                    const PasswordPromptPtr&, int);

//
// Read a certificate and key from an file and optionaly import then
// into a keychain.
//
void loadCertificate(SecCertificateRef*, CFDataRef*, SecKeyRef*, SecKeychainRef, const std::string&, 
                     const std::string& = "", const PasswordPromptPtr& = 0, int = 0);

CFArrayRef loadCACertificates(const std::string&, const std::string& = "", const PasswordPromptPtr& = 0, int = 0);
SecCertificateRef findCertificates(SecKeychainRef, const std::string&, const std::string&);
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
bool checkPath(std::string&, const std::string&, bool);

}

#endif
