// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONNECTION_OPENSSL_CLIENT_H
#define ICE_SSL_CONNECTION_OPENSSL_CLIENT_H

#include <Ice/SslConnectionOpenSSL.h>

namespace IceSecurity
{

namespace Ssl
{

namespace OpenSSL
{

class ClientConnection : public Connection
{

public:
    ClientConnection(const CertificateVerifierPtr&, SSL*, const IceSecurity::Ssl::SystemPtr&);
    virtual ~ClientConnection();
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
