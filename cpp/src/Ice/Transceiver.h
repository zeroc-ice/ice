// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TRANSCEIVER_H
#define ICE_TRANSCEIVER_H

#include <IceUtil/Shared.h>
#include <Ice/TransceiverF.h>
#include <Ice/ConnectionF.h>
#include <Ice/EndpointIF.h>
#include <Ice/Network.h>

namespace IceInternal
{

class Buffer;

class ICE_API Transceiver : virtual public ::IceUtil::Shared
{
public:

    virtual NativeInfoPtr getNativeInfo() = 0;

    virtual SocketOperation initialize(Buffer&, Buffer&, bool&) = 0;
    virtual SocketOperation closing(bool, const Ice::LocalException&) = 0;
    virtual void close() = 0;
    virtual EndpointIPtr bind();
    virtual SocketOperation write(Buffer&) = 0;
    virtual SocketOperation read(Buffer&, bool&) = 0;
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
    virtual bool startWrite(Buffer&) = 0;
    virtual void finishWrite(Buffer&) = 0;
    virtual void startRead(Buffer&) = 0;
    virtual void finishRead(Buffer&, bool&) = 0;
#endif

    virtual std::string protocol() const = 0;
    virtual std::string toString() const = 0;
    virtual std::string toDetailedString() const = 0;
    virtual Ice::ConnectionInfoPtr getInfo() const = 0;
    virtual void checkSendSize(const Buffer&) = 0;
    virtual void setBufferSize(int rcvSize, int sndSize) = 0;
};

}

#endif
