// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_RSA_KEY_PAIR_H
#define ICE_RSA_KEY_PAIR_H

#include <IceUtil/Config.h>
#include <IceUtil/Shared.h>
#include <string>
#include <vector>
#include <openssl/ssl.h>
#include <Ice/BuiltinSequences.h>
#include <Ice/RSAKeyPairF.h>
#include <Ice/RSACertificateGenF.h>
#include <Ice/RSAPrivateKeyF.h>
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

class ICE_API RSAKeyPair : public IceUtil::Shared
{
public:

    // Construction from Base64 encodings.
    RSAKeyPair(const std::string&, const std::string&);

    // Construction from binary DER encoding ByteSeq's.
    RSAKeyPair(const Ice::ByteSeq&, const Ice::ByteSeq&);

    ~RSAKeyPair();

    // Conversions to Base64 encodings.
    void keyToBase64(std::string&);
    void certToBase64(std::string&);

    // Conversions to binary DER encodings.
    void keyToByteSeq(Ice::ByteSeq&);
    void certToByteSeq(Ice::ByteSeq&);

    // Get the internal key structures as per the OpenSSL implementation.
    RSA* getRSAPrivateKey() const;
    X509* getX509PublicKey() const;

private:

    RSAKeyPair(const RSAPrivateKeyPtr&, const RSAPublicKeyPtr&);

    friend class RSACertificateGen;

    RSAPrivateKeyPtr _privateKey;
    RSAPublicKeyPtr _publicKey;
};

}

}

#endif
