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

using namespace Ice;

using IceSecurity::Ssl::SystemPtr;

class ServerConnection : public Connection
{

public:
    ServerConnection(const CertificateVerifierPtr&, SSL*, const SystemPtr&);
    virtual ~ServerConnection();
    virtual void shutdown();
    virtual int init(int timeout = 0);

    virtual int read(Buffer&, int);
    virtual int write(Buffer&, int);

protected:

    virtual void showConnectionInfo();
};

}

}

}

#endif
