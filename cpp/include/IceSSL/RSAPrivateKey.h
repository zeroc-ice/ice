// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_RSA_PRIVATE_KEY_H
#define ICE_SSL_RSA_PRIVATE_KEY_H

#include <IceUtil/Shared.h>

#include <Ice/BuiltinSequences.h>

#include <IceSSL/Config.h>
#include <IceSSL/RSAPrivateKeyF.h>

#include <openssl/ssl.h>



namespace IceSSL
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

#endif
