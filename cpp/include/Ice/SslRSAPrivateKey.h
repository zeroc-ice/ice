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
#include <Ice/SslRSAPrivateKeyF.h>

#ifdef WIN32
#   ifdef ICE_API_EXPORTS
#       define ICE_API __declspec(dllexport)
#   else
#       define ICE_API __declspec(dllimport)
#   endif
#else
#   define ICE_API /**/
#endif

namespace IceSecurity
{

namespace Ssl
{

namespace OpenSSL
{

using namespace std;
using IceUtil::Shared;

using std::string;
using Ice::ByteSeq;

class RSAPrivateKey : public Shared
{

public:
    // Construction from Base64 encoding
    RSAPrivateKey(const string&);

    // Construction from ByteSeq
    RSAPrivateKey(const ByteSeq&);

    // Construction from RSA Private Key structure
    RSAPrivateKey(RSA*);

    ~RSAPrivateKey();

    // Conversions to Base64 encoding
    void keyToBase64(string&);

    // Conversions to ByteSequence
    void keyToByteSeq(ByteSeq&);

    // Get the key structure
    RSA* getRSAPrivateKey() const;

private:
    void byteSeqToKey(const ByteSeq&);

    RSA* _privateKey;

};

}

}

}

#endif
