// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
