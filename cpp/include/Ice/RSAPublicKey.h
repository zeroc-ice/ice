// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_RSA_PUBLIC_KEY_H
#define ICE_RSA_PUBLIC_KEY_H

#include <IceUtil/Config.h>
#include <IceUtil/Shared.h>
#include <string>
#include <vector>
#include <openssl/ssl.h>
#include <Ice/BuiltinSequences.h>
#include <Ice/RSAPublicKeyF.h>

#ifdef WIN32
#   ifdef ICE_API_EXPORTS
#       define ICE_API __declspec(dllexport)
#   else
#       define ICE_API __declspec(dllimport)
#   endif
#else
#   define ICE_API /**/
#endif

namespace IceSSL
{

namespace OpenSSL
{

class ICE_API RSAPublicKey : public IceUtil::Shared
{
public:

    // Construction from Base64 encoding.
    RSAPublicKey(const std::string&);

    // Construction from binary DER encoding ByteSeq.
    RSAPublicKey(const Ice::ByteSeq&);

    // Construction from X509 structure (simple initialization).
    RSAPublicKey(X509*);

    ~RSAPublicKey();

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

}

#endif
