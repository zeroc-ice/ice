// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_RSA_PRIVATE_KEY_H
#define ICE_SSL_RSA_PRIVATE_KEY_H

#include <IceUtil/Shared.h>

#include <Ice/BuiltinSequences.h>

#include <IceSSL/RSAPrivateKeyF.h>

#include <openssl/ssl.h>

#ifdef _WIN32
#   ifdef ICE_SSL_API_EXPORTS
#       define ICE_SSL_API __declspec(dllexport)
#   else
#       define ICE_SSL_API __declspec(dllimport)
#   endif
#else
#   define ICE_SSL_API /**/
#endif

namespace IceSSL
{

namespace OpenSSL
{

class ICE_SSL_API RSAPrivateKey : public IceUtil::Shared
{
public:

    // Construction from Base64 encoding.
    RSAPrivateKey(const std::string&);

    // Construction from binary DER encoding ByteSeq.
    RSAPrivateKey(const Ice::ByteSeq&);

    // Construction from RSA Private Key structure (simple initialization).
    RSAPrivateKey(RSA*);

    ~RSAPrivateKey();

    // Conversion to Base64 encoding.
    void keyToBase64(std::string&);

    // Conversion to binary DER encoding.
    void keyToByteSeq(Ice::ByteSeq&);

    // Get the internal key structure as per the OpenSSL implementation.
    RSA* get() const;

private:

    void byteSeqToKey(const Ice::ByteSeq&);

    RSA* _privateKey;
};

}

}

#endif
