// Copyright (c) ZeroC, Inc. All rights reserved.

#pragma once

[[cpp:dll-export(ICE_API)]]
[[cpp:doxygen:include(Ice/Ice.h)]]
[[cpp:header-ext(h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(Ice)]]

[[java:package(com.zeroc)]]
[cs:namespace(ZeroC)]
module Ice
{
#ifdef __SLICE2CS__
    // These definitions help with the encoding of Slic frames.
    module Slic
    {
        /// The keys for supported Slic connection parameters.
        unchecked enum ParameterKey : int
        {
            MaxBidirectionalStreams = 0,
            MaxUnidirectionalStreams = 1,
            IdleTimeout = 2,
        }

        /// The header of the Slic initialize frame body. This header is followed by connection parameters encoded
        /// as a binary context.
        [cs:readonly]
        struct InitializeHeaderBody
        {
            /// The supported Slic version.
            varuint slicVersion;

            /// The application protocol name.
            string applicationProtocolName;
        }

        sequence<varuint> VersionSeq;

        /// The body of a Slic version frame. This frame is sent in response to an initialize frame if the Slic version
        /// from the initialize frame is not supported by the receiver. Upon receiving the Version frame the receiver
        /// should send back a new Initialize frame with a version matching one of the versions provided by the Version
        /// frame body.
        [cs:readonly]
        struct VersionBody
        {
            /// The supported Slic versions.
            VersionSeq versions;
        }

        /// The body of the Stream reset frame. This frame is sent to notify the peer that sender is no longer
        /// interested in the stream. The error code is application protocol specific.
        [cs:readonly]
        struct StreamResetBody
        {
            /// The application protocol error code indicating the reason of the reset.
            varulong applicationProtocolErrorCode;
        }
    }
#endif
}
