// Copyright (c) ZeroC, Inc.

#ifndef ICE_CURRENT_H
#define ICE_CURRENT_H

#include "ConnectionF.h"
#include "Ice/Context.h"
#include "Ice/Identity.h"
#include "Ice/OperationMode.h"
#include "Ice/Version.h"
#include "ObjectAdapterF.h"

namespace Ice
{
    /// Provides information about an incoming request being dispatched.
    /// @headerfile Ice/Ice.h
    struct Current
    {
        /// The object adapter. This value is never nullptr when this Current is provided by an object adapter.
        ObjectAdapterPtr adapter;

        /// The connection that received the request. It's nullptr for collocation-optimized dispatches.
        ConnectionPtr con;

        /// The identity of the target Ice object.
        Identity id;

        /// The facet of the target Ice object.
        std::string facet;

        /// The operation name.
        std::string operation;

        /// The mode of the operation (see ::Ice::checkNonIdempotent).
        OperationMode mode;

        /// The request context.
        Context ctx;

        /// The request ID. `0` means the request is one-way.
        int requestId;

        /// The Ice encoding version used to marshal the payload of the request.
        EncodingVersion encoding;
    };

    /// A default-initialized Current instance.
    [[deprecated("Avoid calling generated functions with a trailing Current parameter.")]]
    ICE_API extern const Current emptyCurrent;

    /// Makes sure the operation mode of an incoming request is not idempotent.
    /// @param current The Current object of the incoming request to check.
    /// @throws MarshalException Thrown when the request's operation mode is OperationMode::Idempotent or
    /// OperationMode::Nonmutating.
    /// @remark The generated code calls this function to ensure that when an operation's mode is not idempotent
    /// (locally), the incoming request's operation mode is not idempotent.
    ICE_API void checkNonIdempotent(const Current& current);
}

#endif
