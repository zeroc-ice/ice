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

namespace IceSSL
{

namespace OpenSSL
{

class ClientConnection : public Connection
{
public:

    ClientConnection(const IceInternal::TraceLevelsPtr&,
                     const Ice::LoggerPtr&,
                     const IceSSL::CertificateVerifierPtr&,
                     SSL*,
                     const IceSSL::SystemInternalPtr&);
    virtual ~ClientConnection();

    virtual void shutdown();
    virtual int init(int timeout = 0);

    virtual int write(IceInternal::Buffer&, int);

protected:

    virtual void showConnectionInfo();
};

}

}

#endif
