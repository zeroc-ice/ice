// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef ICE_SSL_DEFAULT_CERTIFICATE_VERIFIER_H
#define ICE_SSL_DEFAULT_CERTIFICATE_VERIFIER_H

#include <Ice/LoggerF.h>
#include <IceSSL/TraceLevelsF.h>
#include <IceSSL/CertificateVerifierOpenSSL.h>

namespace IceSSL
{

class DefaultCertificateVerifier : public IceSSL::CertificateVerifierOpenSSL
{
public:

    DefaultCertificateVerifier(const IceSSL::TraceLevelsPtr&, const Ice::LoggerPtr&);

    virtual int verify(int, X509_STORE_CTX*, SSL*);

private:

    IceSSL::TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;
};

}

#endif
