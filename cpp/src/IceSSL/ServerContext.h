// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICESSL_SERVER_CONTEXT_H
#define ICESSL_SERVER_CONTEXT_H

#include <IceSSL/Context.h>

namespace IceSSL
{

class ServerContext : public Context
{
public:

    virtual void configure(const GeneralConfig&,
                           const CertificateAuthority&,
                           const BaseCertificates&);

    // Takes a socket fd as the first parameter.
    virtual SslTransceiverPtr createTransceiver(int, const OpenSSLPluginIPtr&);

protected:

    ServerContext(const TraceLevelsPtr&, const Ice::LoggerPtr&, const Ice::PropertiesPtr&);

    virtual void loadCertificateAuthority(const CertificateAuthority& certAuth);

    friend class OpenSSLPluginI;
};

}

#endif
