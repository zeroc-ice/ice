// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SslCertificateVerifierOpenSSL.h>

IceSecurity::Ssl::OpenSSL::CertificateVerifier::~CertificateVerifier()
{
}

void
IceInternal::incRef(::IceSecurity::Ssl::OpenSSL::CertificateVerifier* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceSecurity::Ssl::OpenSSL::CertificateVerifier* p)
{
    p->__decRef();
}

