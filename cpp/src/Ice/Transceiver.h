// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_TRANSCEIVER_H
#define ICE_TRANSCEIVER_H

#include <IceUtil/Shared.h>
#include <Ice/TransceiverF.h>

#ifndef _WIN32
#   define SOCKET int
#endif

namespace IceInternal
{

class Buffer;

class ICE_PROTOCOL_API Transceiver : public ::IceUtil::Shared
{
public:

    virtual SOCKET fd() = 0;
    virtual void close() = 0;
    virtual void shutdown() = 0;
    virtual void write(Buffer&, int) = 0;
    virtual void read(Buffer&, int) = 0;
    virtual std::string toString() const = 0;
};

}

#endif
