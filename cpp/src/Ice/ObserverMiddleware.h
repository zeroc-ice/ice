// Copyright (c) ZeroC, Inc.

#ifndef ICE_OBSERVER_MIDDLEWARE_H
#define ICE_OBSERVER_MIDDLEWARE_H

#include "Ice/Instrumentation.h"
#include "Ice/Object.h"

namespace IceInternal
{
    // A middleware that observes dispatches.
    class ObserverMiddleware final : public Ice::Object
    {
    public:
        ObserverMiddleware(Ice::ObjectPtr next, Ice::Instrumentation::CommunicatorObserverPtr communicatorObserver);

        void dispatch(Ice::IncomingRequest&, std::function<void(Ice::OutgoingResponse)>) final;

    private:
        Ice::ObjectPtr _next;
        Ice::Instrumentation::CommunicatorObserverPtr _communicatorObserver;
    };
}

#endif
