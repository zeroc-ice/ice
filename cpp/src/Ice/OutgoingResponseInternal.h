// Copyright (c) ZeroC, Inc.

#ifndef ICE_OUTGOING_RESPONSE_INTERNAL_H
#define ICE_OUTGOING_RESPONSE_INTERNAL_H

#include "Ice/OutgoingResponse.h"

namespace Ice
{
    /// @private
    /// The "core" implementation of makeOutgoingResponse for exceptions. Note that it can throw an exception.
    /// @param exception The exception to marshal into the response.
    /// @param current A reference to the Current object of the request. @c current.adapter can be null here; the
    /// implementation uses @p instance instead.
    /// @param instance The IceInternal::Instance object. Not null.
    /// @return The new response.
    OutgoingResponse
    makeOutgoingResponseCore(std::exception_ptr exception, const Current& current, IceInternal::Instance* instance);
}

#endif
