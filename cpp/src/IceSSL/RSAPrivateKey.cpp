// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Config.h>
#include <IceUtil/Base64.h>
#include <IceSSL/RSAPrivateKey.h>
#include <IceSSL/Convert.h>
#include <IceSSL/OpenSSLUtils.h>
#include <IceSSL/Exception.h>
#include <assert.h>

void IceInternal::incRef(::IceSSL::RSAPrivateKey* p) { p->__incRef(); }
void IceInternal::decRef(::IceSSL::RSAPrivateKey* p) { p->__decRef(); }

using std::back_inserter;
using std::string;
using Ice::ByteSeq;
using IceUtil::Base64;

IceSSL::RSAPrivateKey::RSAPrivateKey(const string& key)
{
    assert(!key.empty());

    _privateKey = 0;
    
    ByteSeq keySeq = Base64::decode(key);

    byteSeqToKey(keySeq);
}

IceSSL::RSAPrivateKey::RSAPrivateKey(const ByteSeq& keySeq)
{
    assert(!keySeq.empty());

    _privateKey = 0;
    
    byteSeqToKey(keySeq);
}

IceSSL::RSAPrivateKey::RSAPrivateKey(RSA* rsa) :
                               _privateKey(rsa)
{
    assert(_privateKey != 0);
}

IceSSL::RSAPrivateKey::~RSAPrivateKey()
{
    if(_privateKey != 0)
    {
        RSA_free(_privateKey);
    }
}

void
IceSSL::RSAPrivateKey::keyToBase64(string& b64Key)
{
    ByteSeq keySeq;
    keyToByteSeq(keySeq);
    b64Key = Base64::encode(keySeq);
}

void
IceSSL::RSAPrivateKey::keyToByteSeq(ByteSeq& keySeq)
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

    delete [] privateKeyBuffer;
}

RSA*
IceSSL::RSAPrivateKey::get() const
{
    return _privateKey;
}

void
IceSSL::RSAPrivateKey::byteSeqToKey(const ByteSeq& keySeq)
{
    unsigned char* privateKeyBuffer = byteSeqToUChar(keySeq);
    assert(privateKeyBuffer != 0);

    unsigned char* privKeyBuff = privateKeyBuffer;
    unsigned char** privKeyBuffpp = &privKeyBuff;
    RSA** rsapp = &_privateKey;

#if OPENSSL_VERSION_NUMBER < 0x0090700fL
    _privateKey = d2i_RSAPrivateKey(rsapp, privKeyBuffpp, (long)keySeq.size());
#else
    _privateKey = d2i_RSAPrivateKey(rsapp, (const unsigned char **)privKeyBuffpp, (long)keySeq.size());
#endif

    delete [] privateKeyBuffer;

    if(_privateKey == 0)
    {
        IceSSL::PrivateKeyParseException pkParseException(__FILE__, __LINE__);

        pkParseException.message = "unable to parse provided private key\n" + sslGetErrors();

        throw pkParseException;
    }
}

