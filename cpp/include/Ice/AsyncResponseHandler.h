//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ASYNC_RESPONSE_HANDLER_H
#define ICE_ASYNC_RESPONSE_HANDLER_H

#include "Current.h"
#include "OutgoingResponse.h"
#include "LocalException.h"

#include <atomic>

namespace IceInternal
{
    // This class helps with the implementation of the AMD response and exception callbacks. It allows the dispatch
    // thread and these two callbacks to share the same sendResponse and Current objects, and ensures sendResponse is
    // called exactly once.
    class AsyncResponseHandler final
    {
    public:
        // This class typically holds a _copy_ of the incoming request current object.
        AsyncResponseHandler(std::function<void(Ice::OutgoingResponse)> sendResponse, Ice::Current current)
            : _sendResponse(std::move(sendResponse)),
              _current(std::move(current))
        {
        }

        void sendEmptyResponse() noexcept
        {
            if (!_responseSent.test_and_set())
            {
                _sendResponse(makeEmptyOutgoingResponse(_current));
            }
            // else we ignore this call.
        }

        void sendResponse(Ice::MarshaledResult marshaledResult) noexcept
        {
            if (!_responseSent.test_and_set())
            {
                _sendResponse(Ice::OutgoingResponse{marshaledResult.outputStream(), _current});
            }
            // else we ignore this call.
        }

        void sendResponse(bool ok, const std::pair<const uint8_t*, const uint8_t*>& encaps) noexcept
        {
            if (!_responseSent.test_and_set())
            {
                _sendResponse(makeOutgoingResponse(ok, encaps, _current));
            }
            // else we ignore this call.
        }

        void sendResponse(
            std::function<void(Ice::OutputStream*)> marshal,
            Ice::FormatType format = Ice::FormatType::DefaultFormat) noexcept
        {
            // It is critical to only call the _sendResponse function only once. Calling it multiple times results in an
            // incorrect dispatch count.
            if (!_responseSent.test_and_set())
            {
                _sendResponse(makeOutgoingResponse(std::move(marshal), _current, format));
            }
            // else we ignore this call.
        }

        void sendException(std::exception_ptr ex) noexcept
        {
            if (!_responseSent.test_and_set())
            {
                _sendResponse(makeOutgoingResponse(ex, _current));
            }
            // else we ignore this call.
        }

        const Ice::Current& current() const noexcept { return _current; }

    private:
        const std::function<void(Ice::OutgoingResponse)> _sendResponse;
        const Ice::Current _current;
        std::atomic_flag _responseSent = ATOMIC_FLAG_INIT;
    };
}

#endif
