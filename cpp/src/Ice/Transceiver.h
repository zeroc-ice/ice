// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
