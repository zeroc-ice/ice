// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_DEFAULT_CERTIFICATE_VERIFIER_H
#define ICE_SSL_DEFAULT_CERTIFICATE_VERIFIER_H

#include <Ice/LoggerF.h>
#include <IceSSL/TraceLevelsF.h>
#include <IceSSL/CertificateVerifierOpenSSL.h>

namespace IceSSL
{

namespace OpenSSL
{

class DefaultCertificateVerifier : public IceSSL::OpenSSL::CertificateVerifier
{
public:

    DefaultCertificateVerifier(const IceSSL::TraceLevelsPtr&, const Ice::LoggerPtr&);

    virtual int verify(int, X509_STORE_CTX*, SSL*);

private:

    IceSSL::TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;
};

}

}

#endif
