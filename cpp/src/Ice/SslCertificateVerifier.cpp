// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SslCertificateVerifier.h>

void ::IceInternal::incRef(::IceSecurity::Ssl::CertificateVerifier* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSecurity::Ssl::CertificateVerifier* p) { p->__decRef(); }

IceSecurity::Ssl::CertificateVerifier::~CertificateVerifier()
{
}


