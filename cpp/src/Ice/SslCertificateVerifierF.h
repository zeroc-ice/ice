// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CERTIFICATE_VERIFIER_F_H
#define ICE_SSL_CERTIFICATE_VERIFIER_F_H

#include <Ice/Handle.h>

namespace IceSecurity
{

namespace Ssl
{

class CertificateVerifier;
typedef IceInternal::Handle<CertificateVerifier> CertificateVerifierPtr;

}

}

namespace IceInternal
{

void incRef(::IceSecurity::Ssl::CertificateVerifier*);
void decRef(::IceSecurity::Ssl::CertificateVerifier*);

}

#endif


