// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceSSL/CertificateVerifierOpenSSL.h>

IceSSL::CertificateVerifierOpenSSL::~CertificateVerifierOpenSSL()
{
}

void
IceInternal::incRef(::IceSSL::CertificateVerifierOpenSSL* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceSSL::CertificateVerifierOpenSSL* p)
{
    p->__decRef();
}

