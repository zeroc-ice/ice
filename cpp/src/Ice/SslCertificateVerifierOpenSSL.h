// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CERTIFICATE_VERIFIER_OPENSSL_H
#define ICE_SSL_CERTIFICATE_VERIFIER_OPENSSL_H

#include <Ice/SslCertificateVerifier.h>
#include <openssl/ssl.h>

namespace IceSecurity
{

namespace Ssl
{

namespace OpenSSL
{

class CertificateVerifier : public IceSecurity::Ssl::CertificateVerifier
{

public:
    virtual ~CertificateVerifier();

    virtual int verify(int, X509_STORE_CTX*, SSL*) = 0;
};

class DefaultCertificateVerifier : public CertificateVerifier
{

public:
    virtual int verify(int, X509_STORE_CTX*, SSL*);
};

}

}

}

#endif


