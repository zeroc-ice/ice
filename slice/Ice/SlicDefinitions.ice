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
        /// The body of a Slic initialize frame.
        /// TODO: should we use tagged data members instead for parameters? If we do, we can no longer use structs
        /// however
        [cs:readonly]
        struct InitializeBody
        {
            /// The supported Slic version.
            varuint slicVersion;

            /// The application protocol name.
            string applicationProtocolName;

            /// The maximum number of concurrent bidirectional streams that the peer is allowed to open.
            varulong maxBidirectionalStreams;

            /// The maximum number of concurrent undirectional streams that the peer is allowed to open.
            varulong maxUnidirectionalStreams;

            /// The configured idle timeout.
            varulong idleTimeout;
        }

        /// The body of a Slic initialize acknowledgment frame. This frame is sent in response to an initialize frame
        /// if the Slic version from the initialize frame is supported by the receiver.
        [cs:readonly]
        struct InitializeAckBody
        {
            /// The maximum number of concurrent bidirectional streams that the peer is allowed to open.
            varulong maxBidirectionalStreams;

            /// The maximum number of concurrent undirectional streams that the peer is allowed to open.
            varulong maxUnidirectionalStreams;

            /// The configured idle timeout.
            varulong idleTimeout;
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
