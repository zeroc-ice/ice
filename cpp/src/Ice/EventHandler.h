// Copyright (c) ZeroC, Inc.

#ifndef ICE_EVENT_HANDLER_H
#define ICE_EVENT_HANDLER_H

#include "EventHandlerF.h"
#include "Ice/InstanceF.h"
#include "Network.h"
#include "ThreadPoolF.h"

#include <memory>

namespace IceInternal
{
    class ThreadPoolCurrent;

    class ICE_API EventHandler : public std::enable_shared_from_this<EventHandler>
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
        [[nodiscard]] virtual std::string toString() const = 0;

        //
        // Get the native information of the handler, this is used by the selector.
        //
        virtual NativeInfoPtr getNativeInfo() = 0;

    protected:
        virtual ~EventHandler();

#if defined(ICE_USE_IOCP)
        SocketOperation _pending{SocketOperationNone};
        SocketOperation _started{SocketOperationNone};
        SocketOperation _completed{SocketOperationNone};
        bool _finish{false};
#else
        SocketOperation _disabled{SocketOperationNone};
#endif
        SocketOperation _ready{SocketOperationNone};
        SocketOperation _registered{SocketOperationNone};

        friend class ThreadPool;
        friend class ThreadPoolCurrent;
        friend class Selector;
#ifdef ICE_USE_CFSTREAM
        friend class EventHandlerWrapper;
#endif
    };
}

#endif
