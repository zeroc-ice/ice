// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICESSL_CLIENT_CONTEXT_H
#define ICESSL_CLIENT_CONTEXT_H

#include <IceSSL/Context.h>

namespace IceSSL
{

class ClientContext : public Context
{
public:

    virtual void configure(const GeneralConfig&,
                           const CertificateAuthority&,
                           const BaseCertificates&);

    // Takes a socket fd as the first parameter.
    virtual SslTransceiverPtr createTransceiver(int, const OpenSSLPluginIPtr&);

protected:

    ClientContext(const TraceLevelsPtr&, const Ice::LoggerPtr&, const Ice::PropertiesPtr&);

    friend class OpenSSLPluginI;
};

}

#endif
