// **********************************************************************
//
// Copyright (c) 2003
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

    // Takes a socket fd as the first parameter, and the initial handshake timeout as the final.
    virtual SslTransceiverPtr createTransceiver(int, const OpenSSLPluginIPtr&, int);

protected:

    ServerContext(const TraceLevelsPtr&, const Ice::CommunicatorPtr&);

    virtual void loadCertificateAuthority(const CertificateAuthority& certAuth);

    friend class OpenSSLPluginI;
};

}

#endif
