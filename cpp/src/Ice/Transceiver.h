// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TRANSCEIVER_H
#define ICE_TRANSCEIVER_H

#include <IceUtil/Shared.h>
#include <Ice/TransceiverF.h>

#ifdef _WIN32
typedef int ssize_t;
#else
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
