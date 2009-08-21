// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
#include <Ice/Network.h>

namespace IceInternal
{

class ICE_API Acceptor : virtual public ::IceUtil::Shared
{
public:

    virtual NativeInfoPtr getNativeInfo() = 0;
    virtual void close() = 0;
    virtual void listen() = 0;
#ifdef ICE_USE_IOCP
    virtual void startAccept() = 0;
    virtual void finishAccept() = 0;
#endif
    virtual TransceiverPtr accept() = 0;
    virtual std::string toString() const = 0;
};

}

#endif
