// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

namespace OpenSSL
{

class ICE_SSL_API CertificateVerifier : public IceSSL::CertificateVerifier
{
public:

    virtual ~CertificateVerifier();

    virtual int verify(int, X509_STORE_CTX*, SSL*) = 0;
};

typedef IceInternal::Handle<IceSSL::OpenSSL::CertificateVerifier> CertificateVerifierPtr;

}

}

namespace IceInternal
{

ICE_SSL_API void incRef(IceSSL::OpenSSL::CertificateVerifier*);
ICE_SSL_API void decRef(IceSSL::OpenSSL::CertificateVerifier*);

}


#endif
