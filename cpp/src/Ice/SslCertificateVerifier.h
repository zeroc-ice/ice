// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CERTIFICATE_VERIFIER_H
#define ICE_SSL_CERTIFICATE_VERIFIER_H

#include <IceUtil/Shared.h>
#include <Ice/SslCertificateVerifierF.h>

namespace IceSecurity
{

namespace Ssl
{

using IceUtil::Shared;

class CertificateVerifier : public Shared
{

public:
    virtual ~CertificateVerifier();

};

}

}

#endif


