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
#include <Ice/SslRSAKeyPairF.h>
#include <Ice/SslRSACertificateGenF.h>

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

class RSAKeyPair : public Shared
{

public:
    // Construction from Base64 encodings
    RSAKeyPair(const string&, const string&);

    // Construction from ByteSeq
    RSAKeyPair(const ByteSeq&, const ByteSeq&);

    ~RSAKeyPair();

    // Conversions to Base64 encodings
    void keyToBase64(string&);
    void certToBase64(string&);

    // Conversions to ByteSequences
    void keyToByteSeq(ByteSeq&);
    void certToByteSeq(ByteSeq&);

    // Get the key structures
    RSA* getRSAPrivateKey() const;
    X509* getX509PublicKey() const;

private:
    RSAKeyPair(RSA*, X509*);

    friend class RSACertificateGen;

    void byteSeqToKey(const ByteSeq&);
    void byteSeqToCert(const ByteSeq&);
    void ucharToByteSeq(unsigned char*, int, ByteSeq&);
    unsigned char* byteSeqToUChar(const ByteSeq&);

    RSA* _privateKey;
    X509* _publicKey;

};

}

}

}

#endif
