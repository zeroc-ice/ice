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
using IceUtil::Base64;

IceSecurity::Ssl::OpenSSL::RSAKeyPair::RSAKeyPair(const string& key, const string& cert) :
                                      _privateKey(new RSAPrivateKey(key)),
                                      _publicKey(new RSAPublicKey(cert))
{
/*
    _privateKey = 0;
    _publicKey = 0;

    ByteSeq keySeq = Base64::decode(key);
    ByteSeq certSeq = Base64::decode(cert);

    byteSeqToKey(keySeq);
    byteSeqToCert(certSeq);
*/
}

IceSecurity::Ssl::OpenSSL::RSAKeyPair::RSAKeyPair(const ByteSeq& keySeq, const ByteSeq& certSeq) :
                                      _privateKey(new RSAPrivateKey(keySeq)),
                                      _publicKey(new RSAPublicKey(certSeq))
{
/*
    _privateKey = 0;
    _publicKey = 0;
    byteSeqToKey(keySeq);
    byteSeqToCert(certSeq);
*/
}

IceSecurity::Ssl::OpenSSL::RSAKeyPair::~RSAKeyPair()
{
/*
    RSA_free(_privateKey);
    X509_free(_publicKey);
*/
}

void
IceSecurity::Ssl::OpenSSL::RSAKeyPair::keyToBase64(string& b64Key)
{
    _privateKey->keyToBase64(b64Key);

/*
    ByteSeq keySeq;
    keyToByteSeq(keySeq);
    b64Key = Base64::encode(keySeq);
*/
}

void
IceSecurity::Ssl::OpenSSL::RSAKeyPair::certToBase64(string& b64Cert)
{
    _publicKey->certToBase64(b64Cert);
/*
    ByteSeq certSeq;
    certToByteSeq(certSeq);
    b64Cert = Base64::encode(certSeq);
*/
}

void
IceSecurity::Ssl::OpenSSL::RSAKeyPair::keyToByteSeq(ByteSeq& keySeq)
{
    _privateKey->keyToByteSeq(keySeq);
/*
    assert(_privateKey);

    // Output the Private Key to a char buffer
    unsigned int privKeySize = i2d_RSAPrivateKey(_privateKey, 0);

    assert(privKeySize > 0);

    unsigned char* privateKeyBuffer = new unsigned char[privKeySize];

    // We have to do this because i2d_RSAPrivateKey changes the pointer.
    unsigned char* privKeyBuff = privateKeyBuffer;
    i2d_RSAPrivateKey(_privateKey, &privKeyBuff);

    ucharToByteSeq(privateKeyBuffer, privKeySize, keySeq);

    delete []privateKeyBuffer;
*/
}

void
IceSecurity::Ssl::OpenSSL::RSAKeyPair::certToByteSeq(ByteSeq& certSeq)
{
    _publicKey->certToByteSeq(certSeq);

/*
    assert(_publicKey);

    // Output the Public Key to a char buffer
    unsigned int pubKeySize = i2d_X509(_publicKey, 0);

    assert(pubKeySize > 0);

    unsigned char* publicKeyBuffer = new unsigned char[pubKeySize];

    // We have to do this because i2d_X509_PUBKEY changes the pointer.
    unsigned char* pubKeyBuff = publicKeyBuffer;
    i2d_X509(_publicKey, &pubKeyBuff);

    ucharToByteSeq(publicKeyBuffer, pubKeySize, certSeq);

    delete []publicKeyBuffer;
*/
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

// IceSecurity::Ssl::OpenSSL::RSAKeyPair::RSAKeyPair(RSA* rsa, X509* x509) :

IceSecurity::Ssl::OpenSSL::RSAKeyPair::RSAKeyPair(const RSAPrivateKeyPtr& rsa, const RSAPublicKeyPtr& x509) :
                                      _privateKey(rsa),
                                      _publicKey(x509)
{
}

/*
void
IceSecurity::Ssl::OpenSSL::RSAKeyPair::byteSeqToKey(const ByteSeq& keySeq)
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

void
IceSecurity::Ssl::OpenSSL::RSAKeyPair::byteSeqToCert(const ByteSeq& certSeq)
{
    unsigned char* publicKeyBuffer = byteSeqToUChar(certSeq);
    assert(publicKeyBuffer);

    // We have to do this because d2i_X509 changes the pointer.
    unsigned char* pubKeyBuff = publicKeyBuffer;
    unsigned char** pubKeyBuffpp = &pubKeyBuff;

    X509** x509pp = &_publicKey;

    _publicKey = d2i_X509(x509pp, pubKeyBuffpp, (long)certSeq.size());
    assert(_publicKey);

    delete []publicKeyBuffer;
}


void
IceSecurity::Ssl::OpenSSL::RSAKeyPair::ucharToByteSeq(unsigned char* ucharBuffer, int length, ByteSeq& destBuffer)
{
    destBuffer.reserve(length);
    std::copy(ucharBuffer, (ucharBuffer + length), back_inserter(destBuffer));
}


unsigned char*
IceSecurity::Ssl::OpenSSL::RSAKeyPair::byteSeqToUChar(const ByteSeq& sequence)
{
    int seqSize = sequence.size();

    assert(seqSize > 0);

    unsigned char* ucharSeq = new unsigned char[seqSize];
    unsigned char* ucharPtr = ucharSeq;
    std::copy(sequence.begin(), sequence.end(), ucharPtr);

    return ucharSeq;
}
*/

