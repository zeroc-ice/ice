// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONTEXT_OPENSSL_CLIENT_H
#define ICE_SSL_CONTEXT_OPENSSL_CLIENT_H

#include <IceSSL/ContextOpenSSL.h>

namespace IceSSL
{

namespace OpenSSL
{

class ClientContext : public Context
{
public:

    virtual void configure(const IceSSL::GeneralConfig&,
                           const IceSSL::CertificateAuthority&,
                           const IceSSL::BaseCertificates&);

    // Takes a socket fd as the first parameter.
    virtual IceSSL::ConnectionPtr createConnection(int, const IceSSL::PluginBaseIPtr&);

protected:

    ClientContext(const IceInternal::InstancePtr&);

    friend class IceSSL::OpenSSL::PluginI;
};

}

}

#endif
