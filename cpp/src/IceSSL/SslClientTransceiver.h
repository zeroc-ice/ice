// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CLIENT_TRANSCEIVER_H
#define ICE_SSL_CLIENT_TRANSCEIVER_H

#include <IceSSL/SslTransceiver.h>

namespace IceSSL
{

class ClientContext;

class SslClientTransceiver : public SslTransceiver
{
public:
    virtual int handshake(int timeout = 0);
    virtual void write(IceInternal::Buffer&, int);

protected:
    virtual void showConnectionInfo();
    SslClientTransceiver(const OpenSSLPluginIPtr&, SOCKET, const CertificateVerifierPtr&, SSL*);
    friend class ClientContext;
};

}

#endif
