// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
