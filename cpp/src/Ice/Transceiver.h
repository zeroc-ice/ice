// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
#include <Ice/Network.h>

namespace IceInternal
{

class Buffer;

class ICE_API Transceiver : virtual public ::IceUtil::Shared
{
public:
    
    virtual NativeInfoPtr getNativeInfo() = 0;
    virtual SocketOperation initialize(Buffer&, Buffer&) = 0;
    virtual void close() = 0;
    virtual bool write(Buffer&) = 0;
    virtual bool read(Buffer&) = 0;
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
    virtual bool startWrite(Buffer&) = 0;
    virtual void finishWrite(Buffer&) = 0;
    virtual void startRead(Buffer&) = 0;
    virtual void finishRead(Buffer&) = 0;
#endif
    virtual std::string type() const = 0;
    virtual std::string toString() const = 0;
    virtual Ice::ConnectionInfoPtr getInfo() const = 0;
    virtual void checkSendSize(const Buffer&, size_t) = 0;
};

}

#endif
