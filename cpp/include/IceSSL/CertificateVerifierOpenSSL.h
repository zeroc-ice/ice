// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_SSL_CERTIFICATE_VERIFIER_OPENSSL_H
#define ICE_SSL_CERTIFICATE_VERIFIER_OPENSSL_H

#include <IceSSL/CertificateVerifier.h>
#include <openssl/ssl.h>

#ifdef _WIN32
#   ifdef ICE_SSL_API_EXPORTS
#       define ICE_SSL_API __declspec(dllexport)
#   else
#       define ICE_SSL_API __declspec(dllimport)
#   endif
#else
#   define ICE_SSL_API /**/
#endif

namespace IceSSL
{

class ICE_SSL_API CertificateVerifierOpenSSL : public IceSSL::CertificateVerifier
{
public:

    virtual ~CertificateVerifierOpenSSL();

    virtual int verify(int, X509_STORE_CTX*, SSL*) = 0;
};

typedef IceInternal::Handle<IceSSL::CertificateVerifierOpenSSL> CertificateVerifierOpenSSLPtr;

}

namespace IceInternal
{

ICE_SSL_API void incRef(IceSSL::CertificateVerifierOpenSSL*);
ICE_SSL_API void decRef(IceSSL::CertificateVerifierOpenSSL*);

}


#endif
