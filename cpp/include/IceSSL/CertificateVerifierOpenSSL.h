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

#include <IceSSL/Config.h>
#include <IceSSL/CertificateVerifier.h>
#include <IceSSL/Plugin.h>
#include <openssl/ssl.h>

namespace IceSSL
{

class ICE_SSL_API CertificateVerifierOpenSSL : public IceSSL::CertificateVerifier
{
public:

    virtual ~CertificateVerifierOpenSSL();

    void setContext(ContextType);

    virtual int verify(int, X509_STORE_CTX*, SSL*) = 0;

protected:

    ContextType _contextType;
};

typedef IceInternal::Handle<IceSSL::CertificateVerifierOpenSSL> CertificateVerifierOpenSSLPtr;

}

namespace IceInternal
{

ICE_SSL_API void incRef(IceSSL::CertificateVerifierOpenSSL*);
ICE_SSL_API void decRef(IceSSL::CertificateVerifierOpenSSL*);

}


#endif
