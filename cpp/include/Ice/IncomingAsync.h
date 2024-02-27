//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_INCOMING_ASYNC_H
#define ICE_INCOMING_ASYNC_H

#include "IncomingAsyncF.h"
#include "Incoming.h"
#include <atomic>

namespace IceInternal
{

    // Represents an incoming request dispatched with AMD.
    class ICE_API IncomingAsync final : public IncomingBase, public std::enable_shared_from_this<IncomingAsync>
    {
    public:
        IncomingAsync(Incoming&);

        static std::shared_ptr<IncomingAsync> create(Incoming&);

        std::function<void()> response()
        {
            return [self = shared_from_this()]
            {
                self->writeEmptyParams();
                self->completed();
            };
        }

        template <class T> std::function<void(const T&)> response()
        {
            return [self = shared_from_this()](const T& marshaledResult)
            {
                self->setMarshaledResult(marshaledResult);
                self->completed();
            };
        }

        std::function<void(std::exception_ptr)> exception()
        {
            return [self = shared_from_this()](std::exception_ptr ex) { self->completed(ex); };
        }

        void kill(Incoming&);

        void completed();

        void completed(std::exception_ptr);

    private:
        void checkResponseSent();
        std::atomic_flag _responseSent = ATOMIC_FLAG_INIT;

        // We need a ResponseHandlerPtr, because IncomingBase only holds a pointer to its ResponseHandler as an
        // optimization.
        const ResponseHandlerPtr _responseHandlerCopy;
    };

}

#endif
