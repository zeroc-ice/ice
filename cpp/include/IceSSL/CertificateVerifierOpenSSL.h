// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
