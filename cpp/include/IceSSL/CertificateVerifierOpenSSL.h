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

#include <IceSSL/CertificateVerifier.h>
#include <openssl/ssl.h>

#ifdef _WIN32
#   ifdef ICE_API_EXPORTS
#       define ICE_API __declspec(dllexport)
#   else
#       define ICE_API __declspec(dllimport)
#   endif
#else
#   define ICE_API /**/
#endif

namespace IceSSL
{

namespace OpenSSL
{

class ICE_API CertificateVerifier : public IceSSL::CertificateVerifier
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

ICE_API void incRef(IceSSL::OpenSSL::CertificateVerifier*);
ICE_API void decRef(IceSSL::OpenSSL::CertificateVerifier*);

}


#endif
