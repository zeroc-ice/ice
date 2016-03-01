// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_EVENT_HANDLER_H
#define ICE_EVENT_HANDLER_H

#include <IceUtil/Shared.h>
#include <Ice/EventHandlerF.h>
#include <Ice/InstanceF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/BasicStream.h>
#include <Ice/Network.h>

namespace IceInternal
{

class ICE_API EventHandler : virtual public ::IceUtil::Shared
{
public:

#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
    //
    // Called to start a new asynchronous read or write operation.
    //
    virtual bool startAsync(SocketOperation) = 0;
    virtual bool finishAsync(SocketOperation) = 0;
#endif

    //
    // Called when there's a message ready to be processed.
    //
    virtual void message(ThreadPoolCurrent&) = 0;

    //
    // Called when the event handler is unregistered.
    //
    virtual void finished(ThreadPoolCurrent&, bool) = 0;

    //
    // Get a textual representation of the event handler.
    //
    virtual std::string toString() const = 0;

    //
    // Get the native information of the handler, this is used by the selector.
    //
    virtual NativeInfoPtr getNativeInfo() = 0;

protected:
    
    EventHandler();
    virtual ~EventHandler();

#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
    SocketOperation _ready;
    SocketOperation _pending;
    SocketOperation _started;
    bool _finish;
#else
    SocketOperation _disabled;
#endif
    bool _hasMoreData;
    SocketOperation _registered;

    friend class ThreadPool;
    friend class ThreadPoolCurrent;
    friend class Selector;
#ifdef ICE_USE_CFSTREAM
    friend class EventHandlerWrapper;
#endif
};

}

#endif
