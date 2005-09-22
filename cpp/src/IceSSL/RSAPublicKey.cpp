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
#include <IceSSL/RSAPublicKey.h>
#include <IceSSL/Convert.h>
#include <IceSSL/OpenSSLUtils.h>
#include <IceSSL/Exception.h>
#include <assert.h>

void IceInternal::incRef(::IceSSL::RSAPublicKey* p) { p->__incRef(); }
void IceInternal::decRef(::IceSSL::RSAPublicKey* p) { p->__decRef(); }

using std::back_inserter;
using std::string;
using Ice::ByteSeq;
using IceUtil::Base64;

IceSSL::RSAPublicKey::RSAPublicKey(const string& cert)
{
    assert(!cert.empty());

    _publicKey = 0;

    ByteSeq certSeq = Base64::decode(cert);

    byteSeqToCert(certSeq);
}

IceSSL::RSAPublicKey::RSAPublicKey(const ByteSeq& certSeq)
{
    assert(!certSeq.empty());

    _publicKey = 0;

    byteSeqToCert(certSeq);
}

IceSSL::RSAPublicKey::~RSAPublicKey()
{
    if(_publicKey != 0)
    {
        X509_free(_publicKey);
    }
}

void
IceSSL::RSAPublicKey::certToBase64(string& b64Cert)
{
    ByteSeq certSeq;
    certToByteSeq(certSeq);
    b64Cert = Base64::encode(certSeq);
}

void
IceSSL::RSAPublicKey::certToByteSeq(ByteSeq& certSeq)
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
IceSSL::RSAPublicKey::getX509PublicKey() const
{
    return _publicKey;
}

IceSSL::RSAPublicKey::RSAPublicKey(X509* x509) :
                              _publicKey(x509)
{
}

void
IceSSL::RSAPublicKey::byteSeqToCert(const ByteSeq& certSeq)
{
    const unsigned char* publicKeyBuffer = byteSeqToUChar(certSeq);
    assert(publicKeyBuffer != 0);

    // We have to do this because d2i_X509 changes the pointer.
    const unsigned char* pubKeyBuff = publicKeyBuffer;
    const unsigned char** pubKeyBuffpp = &pubKeyBuff;

    X509** x509pp = &_publicKey;
#if OPENSSL_VERSION_NUMBER < 0x0090800fL 
    _publicKey = d2i_X509(x509pp, const_cast<unsigned char**>(pubKeyBuffpp), (long)certSeq.size());
#else
    _publicKey = d2i_X509(x509pp, pubKeyBuffpp, (long)certSeq.size());
#endif
			  
    delete [] const_cast<unsigned char*>(publicKeyBuffer);

    if(_publicKey == 0)
    {
        IceSSL::CertificateParseException certParseException(__FILE__, __LINE__);

        certParseException.message = "unable to parse provided public key\n" + sslGetErrors();

        throw certParseException;
    }
}


