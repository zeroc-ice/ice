// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONNECTION_H
#define ICE_SSL_CONNECTION_H

#include <Ice/Buffer.h>
#include <Ice/LoggerF.h>
#include <IceSSL/SslConnectionF.h>
#include <IceSSL/TraceLevelsF.h>
#include <IceSSL/CertificateVerifierF.h>

namespace IceSSL
{

class Connection : public IceUtil::Shared
{
public:

    Connection(const TraceLevelsPtr&,
               const Ice::LoggerPtr&,
               const CertificateVerifierPtr&);
    virtual ~Connection();

    virtual int shutdown(int timeout = 0) = 0;

    virtual int read(IceInternal::Buffer&, int) = 0;
    virtual int write(IceInternal::Buffer&, int) = 0;

protected:

    TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;
    CertificateVerifierPtr _certificateVerifier;
};

}

#endif
