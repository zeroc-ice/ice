// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceSSL/CertificateVerifierOpenSSL.h>

IceSSL::OpenSSL::CertificateVerifier::~CertificateVerifier()
{
}

void
IceInternal::incRef(::IceSSL::OpenSSL::CertificateVerifier* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceSSL::OpenSSL::CertificateVerifier* p)
{
    p->__decRef();
}

