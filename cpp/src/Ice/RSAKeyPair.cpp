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
#include <Ice/RSAKeyPair.h>
#include <Ice/RSAPrivateKey.h>
#include <Ice/RSAPublicKey.h>
#include <assert.h>

void ::IceInternal::incRef(::IceSSL::OpenSSL::RSAKeyPair* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSSL::OpenSSL::RSAKeyPair* p) { p->__decRef(); }

using std::back_inserter;
using std::string;
using Ice::ByteSeq;
using IceUtil::Base64;

IceSSL::OpenSSL::RSAKeyPair::RSAKeyPair(const string& key, const string& cert) :
                            _privateKey(new RSAPrivateKey(key)),
                            _publicKey(new RSAPublicKey(cert))
{
    assert(_privateKey != 0);
    assert(_publicKey != 0);
}

IceSSL::OpenSSL::RSAKeyPair::RSAKeyPair(const ByteSeq& keySeq, const ByteSeq& certSeq) :
                            _privateKey(new RSAPrivateKey(keySeq)),
                            _publicKey(new RSAPublicKey(certSeq))
{
    assert(_privateKey != 0);
    assert(_publicKey != 0);
}

IceSSL::OpenSSL::RSAKeyPair::~RSAKeyPair()
{
}

void
IceSSL::OpenSSL::RSAKeyPair::keyToBase64(string& b64Key)
{
    _privateKey->keyToBase64(b64Key);
}

void
IceSSL::OpenSSL::RSAKeyPair::certToBase64(string& b64Cert)
{
    _publicKey->certToBase64(b64Cert);
}

void
IceSSL::OpenSSL::RSAKeyPair::keyToByteSeq(ByteSeq& keySeq)
{
    _privateKey->keyToByteSeq(keySeq);
}

void
IceSSL::OpenSSL::RSAKeyPair::certToByteSeq(ByteSeq& certSeq)
{
    _publicKey->certToByteSeq(certSeq);
}

RSA*
IceSSL::OpenSSL::RSAKeyPair::getRSAPrivateKey() const
{
    return _privateKey->get();
}

X509*
IceSSL::OpenSSL::RSAKeyPair::getX509PublicKey() const
{
    return _publicKey->getX509PublicKey();
}

IceSSL::OpenSSL::RSAKeyPair::RSAKeyPair(const RSAPrivateKeyPtr& rsa, const RSAPublicKeyPtr& x509) :
                            _privateKey(rsa),
                            _publicKey(x509)
{
}

