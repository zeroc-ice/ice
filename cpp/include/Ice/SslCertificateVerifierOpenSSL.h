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

#ifdef WIN32
#   ifdef ICE_API_EXPORTS
#       define ICE_API __declspec(dllexport)
#   else
#       define ICE_API __declspec(dllimport)
#   endif
#else
#   define ICE_API /**/
#endif

namespace IceSecurity
{

namespace Ssl
{

namespace OpenSSL
{

class ICE_API CertificateVerifier : public IceSecurity::Ssl::CertificateVerifier
{

public:
    virtual ~CertificateVerifier();

    virtual int verify(int, X509_STORE_CTX*, SSL*) = 0;
};

typedef ::IceInternal::Handle< ::IceSecurity::Ssl::OpenSSL::CertificateVerifier> CertificateVerifierPtr;

}

}

}

namespace IceInternal
{

ICE_API void incRef(::IceSecurity::Ssl::OpenSSL::CertificateVerifier*);
ICE_API void decRef(::IceSecurity::Ssl::OpenSSL::CertificateVerifier*);

}


#endif


