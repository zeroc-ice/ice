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

#include <IceUtil/Config.h>
#include <Ice/Logger.h>
#include <Ice/TraceLevels.h>
#include <Ice/Instance.h>
#include <Ice/SslCertificateVerifierOpenSSL.h>

namespace IceSSL
{

namespace OpenSSL
{

class DefaultCertificateVerifier : public IceSSL::OpenSSL::CertificateVerifier
{

public:
    DefaultCertificateVerifier(const IceInternal::InstancePtr&);

    virtual int verify(int, X509_STORE_CTX*, SSL*);

private:
    IceInternal::TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;
};

}

}

#endif
