// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Config.h>
#include <IceUtil/Base64.h>
#include <Ice/RSAPrivateKey.h>
#include <Ice/SslIceUtils.h>
#include <assert.h>

void ::IceInternal::incRef(::IceSSL::OpenSSL::RSAPrivateKey* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSSL::OpenSSL::RSAPrivateKey* p) { p->__decRef(); }

using std::back_inserter;
using std::string;
using Ice::ByteSeq;
using IceUtil::Base64;

IceSSL::OpenSSL::RSAPrivateKey::RSAPrivateKey(const string& key)
{
    assert(!key.empty());

    _privateKey = 0;
    
    ByteSeq keySeq = Base64::decode(key);

    byteSeqToKey(keySeq);
}

IceSSL::OpenSSL::RSAPrivateKey::RSAPrivateKey(const ByteSeq& keySeq)
{
    assert(!keySeq.empty());

    _privateKey = 0;
    
    byteSeqToKey(keySeq);
}

IceSSL::OpenSSL::RSAPrivateKey::~RSAPrivateKey()
{
    if (_privateKey != 0)
    {
        RSA_free(_privateKey);
    }
}

void
IceSSL::OpenSSL::RSAPrivateKey::keyToBase64(string& b64Key)
{
    ByteSeq keySeq;
    keyToByteSeq(keySeq);
    b64Key = Base64::encode(keySeq);
}

void
IceSSL::OpenSSL::RSAPrivateKey::keyToByteSeq(ByteSeq& keySeq)
{
    assert(_privateKey);

    // Output the Private Key to a char buffer
    unsigned int privKeySize = i2d_RSAPrivateKey(_privateKey, 0);

    assert(privKeySize > 0);

    unsigned char* privateKeyBuffer = new unsigned char[privKeySize];
    assert(privateKeyBuffer != 0);

    // We have to do this because i2d_RSAPrivateKey changes the pointer.
    unsigned char* privKeyBuff = privateKeyBuffer;
    i2d_RSAPrivateKey(_privateKey, &privKeyBuff);

    IceSSL::ucharToByteSeq(privateKeyBuffer, privKeySize, keySeq);

    delete []privateKeyBuffer;
}

RSA*
IceSSL::OpenSSL::RSAPrivateKey::getRSAPrivateKey() const
{
    return _privateKey;
}

IceSSL::OpenSSL::RSAPrivateKey::RSAPrivateKey(RSA* rsa) :
                               _privateKey(rsa)
{
    assert(_privateKey != 0);
}

void
IceSSL::OpenSSL::RSAPrivateKey::byteSeqToKey(const ByteSeq& keySeq)
{
    unsigned char* privateKeyBuffer = byteSeqToUChar(keySeq);
    assert(privateKeyBuffer);

    unsigned char* privKeyBuff = privateKeyBuffer;
    unsigned char** privKeyBuffpp = &privKeyBuff;
    RSA** rsapp = &_privateKey;

    _privateKey = d2i_RSAPrivateKey(rsapp, privKeyBuffpp, (long)keySeq.size());
    assert(_privateKey);

    delete []privateKeyBuffer;
}

