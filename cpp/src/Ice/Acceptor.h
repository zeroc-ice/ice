// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ACCEPTOR_H
#define ICE_ACCEPTOR_H

#include <IceUtil/Shared.h>
#include <Ice/AcceptorF.h>
#include <Ice/TransceiverF.h>

#ifndef _WIN32
#   define SOCKET int
#endif

namespace IceInternal
{

class ICE_PROTOCOL_API Acceptor : public ::IceUtil::Shared
{
public:

    virtual SOCKET fd() = 0;
    virtual void close() = 0;
    virtual void listen() = 0;
    virtual TransceiverPtr accept(int) = 0;
    virtual std::string toString() const = 0;
};

}

#endif
