// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONNECTION_OPENSSL_SERVER_H
#define ICE_SSL_CONNECTION_OPENSSL_SERVER_H

#include <Ice/SslConnectionOpenSSL.h>

namespace IceSecurity
{

namespace Ssl
{

namespace OpenSSL
{

class ServerConnection : public Connection
{

public:
    ServerConnection(const CertificateVerifierPtr&, SSL*, const IceSecurity::Ssl::SystemPtr&);
    virtual ~ServerConnection();
    virtual void shutdown();
    virtual int init(int timeout = 0);

    virtual int read(IceInternal::Buffer&, int);
    virtual int write(IceInternal::Buffer&, int);

protected:

    virtual void showConnectionInfo();
};

}

}

}

#endif
