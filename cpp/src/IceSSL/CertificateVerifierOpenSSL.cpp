// **********************************************************************
//
// Copyright (c) 2001
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

