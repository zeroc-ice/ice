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
#include <IceSSL/RSAPublicKey.h>
#include <IceSSL/Convert.h>
#include <IceSSL/OpenSSLUtils.h>
#include <IceSSL/Exception.h>
#include <assert.h>

void ::IceInternal::incRef(::IceSSL::OpenSSL::RSAPublicKey* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSSL::OpenSSL::RSAPublicKey* p) { p->__decRef(); }

using std::back_inserter;
using std::string;
using Ice::ByteSeq;
using IceUtil::Base64;

IceSSL::OpenSSL::RSAPublicKey::RSAPublicKey(const string& cert)
{
    assert(!cert.empty());

    _publicKey = 0;

    ByteSeq certSeq = Base64::decode(cert);

    byteSeqToCert(certSeq);
}

IceSSL::OpenSSL::RSAPublicKey::RSAPublicKey(const ByteSeq& certSeq)
{
    assert(!certSeq.empty());

    _publicKey = 0;

    byteSeqToCert(certSeq);
}

IceSSL::OpenSSL::RSAPublicKey::~RSAPublicKey()
{
    if (_publicKey != 0)
    {
        X509_free(_publicKey);
    }
}

void
IceSSL::OpenSSL::RSAPublicKey::certToBase64(string& b64Cert)
{
    ByteSeq certSeq;
    certToByteSeq(certSeq);
    b64Cert = Base64::encode(certSeq);
}

void
IceSSL::OpenSSL::RSAPublicKey::certToByteSeq(ByteSeq& certSeq)
{
    assert(_publicKey);

    // Output the Public Key to a char buffer
    unsigned int pubKeySize = i2d_X509(_publicKey, 0);

    assert(pubKeySize > 0);

    unsigned char* publicKeyBuffer = new unsigned char[pubKeySize];
    assert(publicKeyBuffer != 0);

    // We have to do this because i2d_X509_PUBKEY changes the pointer.
    unsigned char* pubKeyBuff = publicKeyBuffer;
    i2d_X509(_publicKey, &pubKeyBuff);

    IceSSL::ucharToByteSeq(publicKeyBuffer, pubKeySize, certSeq);

    delete []publicKeyBuffer;
}

X509*
IceSSL::OpenSSL::RSAPublicKey::getX509PublicKey() const
{
    return _publicKey;
}

IceSSL::OpenSSL::RSAPublicKey::RSAPublicKey(X509* x509) :
                              _publicKey(x509)
{
}

void
IceSSL::OpenSSL::RSAPublicKey::byteSeqToCert(const ByteSeq& certSeq)
{
    unsigned char* publicKeyBuffer = byteSeqToUChar(certSeq);
    assert(publicKeyBuffer != 0);

    // We have to do this because d2i_X509 changes the pointer.
    unsigned char* pubKeyBuff = publicKeyBuffer;
    unsigned char** pubKeyBuffpp = &pubKeyBuff;

    X509** x509pp = &_publicKey;

    _publicKey = d2i_X509(x509pp, pubKeyBuffpp, (long)certSeq.size());

    if (_publicKey == 0)
    {
        IceSSL::CertificateParseException certParseException(__FILE__, __LINE__);

        certParseException._message = "unable to parse provided public key\n" + sslGetErrors();

        throw certParseException;
    }

    delete []publicKeyBuffer;
}


