// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_RSA_PRIVATE_KEY_H
#define ICE_RSA_PRIVATE_KEY_H

#include <IceUtil/Config.h>
#include <IceUtil/Shared.h>
#include <string>
#include <vector>
#include <openssl/ssl.h>
#include <Ice/BuiltinSequences.h>
#include <Ice/RSAPrivateKeyF.h>

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

class ICE_API RSAPrivateKey : public IceUtil::Shared
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
