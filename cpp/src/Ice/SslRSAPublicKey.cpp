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
#include <Ice/SslRSAPublicKey.h>
#include <Ice/SslIceUtils.h>
#include <assert.h>

void ::IceInternal::incRef(::IceSecurity::Ssl::OpenSSL::RSAPublicKey* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSecurity::Ssl::OpenSSL::RSAPublicKey* p) { p->__decRef(); }

using std::back_inserter;
using std::string;
using IceUtil::Base64;

IceSecurity::Ssl::OpenSSL::RSAPublicKey::RSAPublicKey(const string& cert)
{
    _publicKey = 0;

    ByteSeq certSeq = Base64::decode(cert);

    byteSeqToCert(certSeq);
}

IceSecurity::Ssl::OpenSSL::RSAPublicKey::RSAPublicKey(const ByteSeq& certSeq)
{
    _publicKey = 0;
    byteSeqToCert(certSeq);
}

IceSecurity::Ssl::OpenSSL::RSAPublicKey::~RSAPublicKey()
{
    X509_free(_publicKey);
}

void
IceSecurity::Ssl::OpenSSL::RSAPublicKey::certToBase64(string& b64Cert)
{
    ByteSeq certSeq;
    certToByteSeq(certSeq);
    b64Cert = Base64::encode(certSeq);
}

void
IceSecurity::Ssl::OpenSSL::RSAPublicKey::certToByteSeq(ByteSeq& certSeq)
{
    assert(_publicKey);

    // Output the Public Key to a char buffer
    unsigned int pubKeySize = i2d_X509(_publicKey, 0);

    assert(pubKeySize > 0);

    unsigned char* publicKeyBuffer = new unsigned char[pubKeySize];

    // We have to do this because i2d_X509_PUBKEY changes the pointer.
    unsigned char* pubKeyBuff = publicKeyBuffer;
    i2d_X509(_publicKey, &pubKeyBuff);

    IceSecurity::Ssl::ucharToByteSeq(publicKeyBuffer, pubKeySize, certSeq);

    delete []publicKeyBuffer;
}

X509*
IceSecurity::Ssl::OpenSSL::RSAPublicKey::getX509PublicKey() const
{
    return _publicKey;
}

IceSecurity::Ssl::OpenSSL::RSAPublicKey::RSAPublicKey(X509* x509) :
                                      _publicKey(x509)
{
}

void
IceSecurity::Ssl::OpenSSL::RSAPublicKey::byteSeqToCert(const ByteSeq& certSeq)
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


