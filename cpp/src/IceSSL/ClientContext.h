// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_CLIENT_CONTEXT_H
#define ICESSL_CLIENT_CONTEXT_H

#include <IceSSL/OpenSSLPluginIF.h>
#include <IceSSL/Context.h>

namespace IceSSL
{

class ClientContext : public Context
{
public:

    virtual void configure(const GeneralConfig&,
                           const CertificateAuthority&,
                           const BaseCertificates&);

    SslTransceiverPtr createTransceiver(int, const OpenSSLPluginIPtr&, int);

protected:

    ClientContext(const TraceLevelsPtr&, const Ice::CommunicatorPtr&);

    friend class OpenSSLPluginI;
};

}

#endif
