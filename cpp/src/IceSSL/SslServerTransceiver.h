// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

