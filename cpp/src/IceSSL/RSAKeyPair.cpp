// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Config.h>
#include <IceUtil/Base64.h>
#include <IceSSL/RSAKeyPair.h>
#include <IceSSL/RSAPrivateKey.h>
#include <IceSSL/RSAPublicKey.h>
#include <assert.h>

void IceInternal::incRef(::IceSSL::RSAKeyPair* p) { p->__incRef(); }
void IceInternal::decRef(::IceSSL::RSAKeyPair* p) { p->__decRef(); }

using std::back_inserter;
using std::string;
using Ice::ByteSeq;
using IceUtil::Base64;

IceSSL::RSAKeyPair::RSAKeyPair(const string& key, const string& cert) :
    _privateKey(new RSAPrivateKey(key)),
    _publicKey(new RSAPublicKey(cert))
{
    assert(_privateKey != 0);
    assert(_publicKey != 0);
}

IceSSL::RSAKeyPair::RSAKeyPair(const ByteSeq& keySeq, const ByteSeq& certSeq) :
    _privateKey(new RSAPrivateKey(keySeq)),
    _publicKey(new RSAPublicKey(certSeq))
{
    assert(_privateKey != 0);
    assert(_publicKey != 0);
}

IceSSL::RSAKeyPair::~RSAKeyPair()
{
}

void
IceSSL::RSAKeyPair::keyToBase64(string& b64Key)
{
    _privateKey->keyToBase64(b64Key);
}

void
IceSSL::RSAKeyPair::certToBase64(string& b64Cert)
{
    _publicKey->certToBase64(b64Cert);
}

void
IceSSL::RSAKeyPair::keyToByteSeq(ByteSeq& keySeq)
{
    _privateKey->keyToByteSeq(keySeq);
}

void
IceSSL::RSAKeyPair::certToByteSeq(ByteSeq& certSeq)
{
    _publicKey->certToByteSeq(certSeq);
}

RSA*
IceSSL::RSAKeyPair::getRSAPrivateKey() const
{
    return _privateKey->get();
}

X509*
IceSSL::RSAKeyPair::getX509PublicKey() const
{
    return _publicKey->getX509PublicKey();
}

IceSSL::RSAKeyPair::RSAKeyPair(const RSAPrivateKeyPtr& rsa, const RSAPublicKeyPtr& x509) :
    _privateKey(rsa),
    _publicKey(x509)
{
}

