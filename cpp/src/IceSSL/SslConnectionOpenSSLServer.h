// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONNECTION_OPENSSL_SERVER_H
#define ICE_SSL_CONNECTION_OPENSSL_SERVER_H

#include <IceSSL/SslConnectionOpenSSL.h>

namespace IceSSL
{

namespace OpenSSL
{

class ServerConnection : public Connection
{
public:

    ServerConnection(const IceSSL::CertificateVerifierPtr&,
                     SSL*,
                     const IceSSL::PluginBaseIPtr&);
    virtual ~ServerConnection();

    virtual int handshake(int timeout = 0);

    virtual int write(IceInternal::Buffer&, int);

protected:

    virtual void showConnectionInfo();
};

}

}

#endif
