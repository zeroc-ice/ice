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

#include <Ice/Buffer.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>

namespace IceSecurity
{

namespace Ssl
{

using IceInternal::Buffer;
using IceInternal::TraceLevelsPtr;
using Ice::LoggerPtr;

class Connection
{

public:

    virtual ~Connection() {};

    virtual void shutdown() = 0;

    virtual int read(Buffer&, int) = 0;
    virtual int write(Buffer&, int) = 0;

    virtual void setTrace(TraceLevelsPtr) = 0;
    virtual void setLogger(LoggerPtr) = 0;
};

}

}

#endif
