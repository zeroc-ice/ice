// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ACCEPTOR_H
#define ICE_ACCEPTOR_H

#include <IceUtil/Shared.h>
#include <Ice/AcceptorF.h>
#include <Ice/TransceiverF.h>
#include <Ice/EndpointIF.h>
#include <Ice/Network.h>

namespace IceInternal
{

class ICE_API Acceptor : virtual public ::IceUtil::Shared
{
public:

    virtual NativeInfoPtr getNativeInfo() = 0;
    virtual void close() = 0;
    virtual EndpointIPtr listen() = 0;
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
    virtual void startAccept() = 0;
    virtual void finishAccept() = 0;
#endif
    virtual TransceiverPtr accept() = 0;
    virtual std::string protocol() const = 0;
    virtual std::string toString() const = 0;
    virtual std::string toDetailedString() const = 0;
};

}

#endif
