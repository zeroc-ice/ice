// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_SERVER_TRANSCEIVER_H
#define ICE_SSL_SERVER_TRANSCEIVER_H

#include <IceSSL/SslTransceiver.h>

namespace IceSSL
{

class SslServerTransceiver : public SslTransceiver
{
public:

    virtual int handshake(int timeout = 0);
    virtual void write(IceInternal::Buffer&, int);

protected:

    virtual void showConnectionInfo();

    SslServerTransceiver(const OpenSSLPluginIPtr&, SOCKET, const CertificateVerifierPtr&, SSL*);
    friend class ServerContext;
};

}

#endif

