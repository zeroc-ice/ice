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

#include <Ice/LoggerF.h>
#include <IceSSL/SslTransceiver.h>
#include <IceSSL/ContextOpenSSLClient.h>
#include <IceSSL/PluginBaseIF.h>
#include <IceSSL/TraceLevelsF.h>

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
    SslClientTransceiver(const PluginBaseIPtr&, SOCKET, const OpenSSL::CertificateVerifierPtr&, SSL*);
    friend class ClientContext;
};

}

#endif
