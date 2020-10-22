// Copyright (c) ZeroC, Inc. All rights reserved.

#pragma once

[[cpp:dll-export(ICE_API)]]
[[cpp:doxygen:include(Ice/Ice.h)]]
[[cpp:header-ext(h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(Ice)]]

#include <Ice/BuiltinSequences.ice>
#include <Ice/Identity.ice>

[[java:package(com.zeroc)]]
[cs:namespace(ZeroC)]
module Ice
{
    // These definitions help with the encoding of ice2 frames.

#ifdef __SLICE2CS__

    /// The priority of this request.
    // TODO: describe semantics.
    unchecked enum Priority : byte
    {
    }

    /// The body of an ice2 request header. A request header consists of two parts: the frame prologue which contains
    /// the frame type and frame size, and the request header body that contains the identity, the operation name and
    /// other optional information.
    [cs:readonly]
    struct Ice2RequestHeaderBody
    {
        Identity identity;
        string? facet = "";      // null equivalent to empty string
        StringSeq? location;     // null equivalent to empty sequence
        string operation;
        bool? \idempotent;       // null equivalent to false
        Priority? priority;      // null equivalent to 0
        varlong deadline;
    }

    /// The type of result carried by an ice2 response frame. The values Success and Failure match the values of OK and
    /// UserException in {@see ReplyStatus}.
    enum ResultType : byte
    {
        /// The request succeeded.
        Success = 0,

        /// The request failed.
        Failure = 1
    }

    // The possible error codes to describe the reason of a stream reset.
    enum StreamResetErrorCode : byte
    {
        /// The caller canceled the request.
        RequestCanceled = 0,

        /// The peer no longer wants to receive data from the stream.
        StopStreamingData = 1,
    }
#endif
}
