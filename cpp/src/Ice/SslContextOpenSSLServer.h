// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONTEXT_OPENSSL_SERVER_H
#define ICE_SSL_CONTEXT_OPENSSL_SERVER_H

#include <Ice/SslContextOpenSSL.h>

namespace IceSSL
{

namespace OpenSSL
{

class ServerContext : public Context
{

public:
    virtual void configure(const IceSSL::GeneralConfig&,
                           const IceSSL::CertificateAuthority&,
                           const IceSSL::BaseCertificates&);

    // Takes a socket fd.
    virtual IceSSL::ConnectionPtr createConnection(int, const IceSSL::SystemInternalPtr&);

protected:
    ServerContext(const IceInternal::InstancePtr&);

    virtual void loadCertificateAuthority(const IceSSL::CertificateAuthority& certAuth);

    friend class IceSSL::OpenSSL::System;
};

}

}

#endif
