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
#include <Ice/SslRSAKeyPair.h>
#include <Ice/SslRSAPrivateKey.h>
#include <Ice/SslRSAPublicKey.h>
#include <assert.h>

void ::IceInternal::incRef(::IceSecurity::Ssl::OpenSSL::RSAKeyPair* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSecurity::Ssl::OpenSSL::RSAKeyPair* p) { p->__decRef(); }

using std::back_inserter;
using std::string;
using Ice::ByteSeq;
using IceUtil::Base64;

IceSecurity::Ssl::OpenSSL::RSAKeyPair::RSAKeyPair(const string& key, const string& cert) :
                                      _privateKey(new RSAPrivateKey(key)),
                                      _publicKey(new RSAPublicKey(cert))
{
}

IceSecurity::Ssl::OpenSSL::RSAKeyPair::RSAKeyPair(const ByteSeq& keySeq, const ByteSeq& certSeq) :
                                      _privateKey(new RSAPrivateKey(keySeq)),
                                      _publicKey(new RSAPublicKey(certSeq))
{
}

IceSecurity::Ssl::OpenSSL::RSAKeyPair::~RSAKeyPair()
{
}

void
IceSecurity::Ssl::OpenSSL::RSAKeyPair::keyToBase64(string& b64Key)
{
    _privateKey->keyToBase64(b64Key);
}

void
IceSecurity::Ssl::OpenSSL::RSAKeyPair::certToBase64(string& b64Cert)
{
    _publicKey->certToBase64(b64Cert);
}

void
IceSecurity::Ssl::OpenSSL::RSAKeyPair::keyToByteSeq(ByteSeq& keySeq)
{
    _privateKey->keyToByteSeq(keySeq);
}

void
IceSecurity::Ssl::OpenSSL::RSAKeyPair::certToByteSeq(ByteSeq& certSeq)
{
    _publicKey->certToByteSeq(certSeq);
}

RSA*
IceSecurity::Ssl::OpenSSL::RSAKeyPair::getRSAPrivateKey() const
{
    return _privateKey->getRSAPrivateKey();
}

X509*
IceSecurity::Ssl::OpenSSL::RSAKeyPair::getX509PublicKey() const
{
    return _publicKey->getX509PublicKey();
}

IceSecurity::Ssl::OpenSSL::RSAKeyPair::RSAKeyPair(const RSAPrivateKeyPtr& rsa, const RSAPublicKeyPtr& x509) :
                                      _privateKey(rsa),
                                      _publicKey(x509)
{
}

