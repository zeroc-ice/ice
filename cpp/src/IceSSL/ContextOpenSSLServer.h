// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONTEXT_OPENSSL_SERVER_H
#define ICE_SSL_CONTEXT_OPENSSL_SERVER_H

#include <IceSSL/ContextOpenSSL.h>

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

    // Takes a socket fd as the first parameter.
    virtual IceSSL::ConnectionPtr createConnection(int, const IceSSL::PluginBaseIPtr&);

protected:

    ServerContext(const IceSSL::TraceLevelsPtr&, const Ice::LoggerPtr&, const Ice::PropertiesPtr&);

    virtual void loadCertificateAuthority(const IceSSL::CertificateAuthority& certAuth);

    friend class IceSSL::OpenSSL::PluginI;
};

}

}

#endif
