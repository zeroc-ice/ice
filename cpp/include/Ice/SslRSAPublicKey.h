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
#include <Ice/SslRSAPublicKeyF.h>

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

class RSAPublicKey : public Shared
{

public:
    // Construction from Base64 encodings
    RSAPublicKey(const string&);

    // Construction from ByteSeq
    RSAPublicKey(const ByteSeq&);

    RSAPublicKey(X509*);

    ~RSAPublicKey();

    // Conversions to Base64 encodings
    void certToBase64(string&);

    // Conversions to ByteSequences
    void certToByteSeq(ByteSeq&);

    // Get the key structures
    X509* getX509PublicKey() const;

private:

    void byteSeqToCert(const ByteSeq&);

    X509* _publicKey;

};

}

}

}

#endif
