// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/CertificateVerifierOpenSSL.h>

IceSSL::CertificateVerifierOpenSSL::~CertificateVerifierOpenSSL()
{
}

void
IceSSL::CertificateVerifierOpenSSL::setContext(::IceSSL::ContextType contextType)
{
    _contextType = contextType;
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

