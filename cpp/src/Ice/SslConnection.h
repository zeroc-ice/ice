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

using IceUtil::Shared;
using IceInternal::Buffer;
using IceInternal::TraceLevelsPtr;
using Ice::LoggerPtr;

class Connection : public Shared
{
public:
    Connection(const CertificateVerifierPtr&);
    virtual ~Connection();

    virtual void shutdown() = 0;

    virtual int read(Buffer&, int) = 0;
    virtual int write(Buffer&, int) = 0;

    virtual void setTrace(const TraceLevelsPtr&) = 0;
    virtual void setLogger(const LoggerPtr&) = 0;

protected:
    CertificateVerifierPtr _certificateVerifier;
};

}

}

#endif
