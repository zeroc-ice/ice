// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_RSA_PUBLIC_KEY_H
#define ICE_SSL_RSA_PUBLIC_KEY_H

#include <IceUtil/Shared.h>

#include <Ice/BuiltinSequences.h>

#include <IceSSL/RSAPublicKeyF.h>

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

class ICE_SSL_API RSAPublicKey : public IceUtil::Shared
{
public:

    // Construction from Base64 encoding.
    RSAPublicKey(const std::string&);

    // Construction from binary DER encoding ByteSeq.
    RSAPublicKey(const Ice::ByteSeq&);

    // Construction from X509 structure (simple initialization).
    RSAPublicKey(X509*);

    virtual ~RSAPublicKey();

    // Conversion to Base64 encoding.
    void certToBase64(std::string&);

    // Conversion to binary DER encoding.
    void certToByteSeq(Ice::ByteSeq&);

    // Get the internal key structure as per the OpenSSL implementation.
    X509* getX509PublicKey() const;

private:

    void byteSeqToCert(const Ice::ByteSeq&);

    X509* _publicKey;
};

}

#endif
