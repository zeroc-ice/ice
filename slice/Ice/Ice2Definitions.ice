// Copyright (c) ZeroC, Inc. All rights reserved.

#pragma once

[[cpp:dll-export(ICE_API)]]
[[cpp:doxygen:include(Ice/Ice.h)]]
[[cpp:header-ext(h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(Ice)]]

#include <Ice/BuiltinSequences.ice>
#include <Ice/Context.ice>
#include <Ice/Identity.ice>

[[java:package(com.zeroc)]]
[cs:namespace(ZeroC)]
module Ice
{
    // These definitions help with the encoding of ice2 frames.

#ifdef __SLICE2CS__

    enum Ice2FrameType : byte
    {
        Initialize = 0,
        Request = 1,
        Response = 2,
        GoAway = 3
    }

    dictionary<varint, ByteSeq> BinaryContext;

    /// The priority of this request.
    // TODO: describe semantics.
    unchecked enum Priority : byte
    {
    }

    /// The keys for supported ice2 connection parameters.
    unchecked enum Ice2ParameterKey : int
    {
        IncomingFrameMaxSize = 0
    }

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
        Context? context;        // null equivalent to empty context
    }

    /// Each ice2 request frame has:
    /// - a frame prologue, with the frame type and the overall frame size.
    /// - a request header, with the header size, body and binary context.
    /// - a request payload, whose size is frame size less length of header size less header size.
    struct Ice2RequestHeader
    {
        varulong headerSize;
        Ice2RequestHeaderBody body;
        BinaryContext binaryContext;
    }

    /// Each ice2 response frame has:
    /// - a frame prologue, with the frame type and the overall frame size.
    /// - a response header, with the header size and binary context.
    /// - a response payload, whose size is frame size less length of header size less header size.
    struct Ice2ResponseHeader
    {
        varulong headerSize;
        BinaryContext binaryContext;
    }

    /// The type of result carried by an ice2 response frame. The values Success and Failure match the values of OK and
    /// UserException in {@see ReplyStatus}.
    /// The result type is always the first byte of an ice2 reponse frame payload.
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

    struct Ice2GoAwayBody
    {
        varulong lastBidirectionalStreamId;
        varulong lastUnidirectionalStreamId;
        string message;
    }
#endif
}
