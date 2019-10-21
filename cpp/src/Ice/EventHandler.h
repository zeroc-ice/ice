//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_EVENT_HANDLER_H
#define ICE_EVENT_HANDLER_H

#include <IceUtil/Shared.h>
#include <Ice/EventHandlerF.h>
#include <Ice/InstanceF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/Network.h>
#include <Ice/VirtualShared.h>

namespace IceInternal
{

class ICE_API EventHandler :
#ifdef ICE_CPP11_MAPPING
        public EnableSharedFromThis<EventHandler>
#else
        public virtual Ice::LocalObject
#endif
{
public:

#if defined(ICE_USE_IOCP)
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

#if defined(ICE_USE_IOCP)
    SocketOperation _pending;
    SocketOperation _started;
    SocketOperation _completed;
    bool _finish;
#else
    SocketOperation _disabled;
#endif
    SocketOperation _ready;
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
