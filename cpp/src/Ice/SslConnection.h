// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONNECTION_H
#define ICE_SSL_CONNECTION_H

#include <IceUtil/Shared.h>
#include <Ice/Buffer.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/SslConnectionF.h>
#include <Ice/CertificateVerifierF.h>

namespace IceSSL
{

class Connection : public IceUtil::Shared
{
public:
    Connection(const IceInternal::TraceLevelsPtr&,
               const Ice::LoggerPtr&,
               const IceSSL::CertificateVerifierPtr&);
    virtual ~Connection();

    virtual void shutdown() = 0;

    virtual int read(IceInternal::Buffer&, int) = 0;
    virtual int write(IceInternal::Buffer&, int) = 0;

protected:
    IceInternal::TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;
    CertificateVerifierPtr _certificateVerifier;
};

}

#endif
