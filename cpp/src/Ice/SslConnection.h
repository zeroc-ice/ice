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
#include <Ice/SslCertificateVerifierF.h>

namespace IceSecurity
{

namespace Ssl
{

class Connection : public IceUtil::Shared
{
public:
    Connection(const CertificateVerifierPtr&);
    virtual ~Connection();

    virtual void shutdown() = 0;

    virtual int read(IceInternal::Buffer&, int) = 0;
    virtual int write(IceInternal::Buffer&, int) = 0;

    virtual void setTrace(const IceInternal::TraceLevelsPtr&) = 0;
    virtual void setLogger(const Ice::LoggerPtr&) = 0;

protected:
    CertificateVerifierPtr _certificateVerifier;
};

}

}

#endif
