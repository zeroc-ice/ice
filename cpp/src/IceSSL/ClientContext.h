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

    // Takes a socket fd as the first parameter, and the initial handshake timeout as the final.
    virtual SslTransceiverPtr createTransceiver(int, const OpenSSLPluginIPtr&, int);

protected:

    ClientContext(const TraceLevelsPtr&, const Ice::LoggerPtr&, const Ice::PropertiesPtr&);

    friend class OpenSSLPluginI;
};

}

#endif
