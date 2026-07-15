// Copyright (c) ZeroC, Inc.

#ifndef GLACIER2_FORWARD_OBSERVER_H
#define GLACIER2_FORWARD_OBSERVER_H

#include "Instrumentation.h"

#include <memory>
#include <mutex>

namespace Glacier2
{
    // A thread-safe holder for the session observer, used to report forwarded requests. The RouterI and its
    // blobjects share this object, so it stays valid and consistently synchronized while a request is being
    // forwarded, even if the session is destroyed concurrently.
    class ForwardObserver final
    {
    public:
        void update(std::shared_ptr<Instrumentation::SessionObserver> observer)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _observer = std::move(observer);
        }

        void forwarded(bool client)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_observer)
            {
                _observer->forwarded(client);
            }
        }

    private:
        std::shared_ptr<Instrumentation::SessionObserver> _observer;
        std::mutex _mutex;
    };
}

#endif
