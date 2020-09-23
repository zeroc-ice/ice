//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[cpp:dll-export:ICE_API]]
[[cpp:doxygen:include:Ice/Ice.h]]
[[cpp:header-ext:h]]

[[suppress-warning:reserved-identifier]]
[[js:module:ice]]

[[python:pkgdir:Ice]]

#include <Ice/BuiltinSequences.ice>
#include <Ice/Encoding.ice>
#include <Ice/Identity.ice>
#include <Ice/Protocol.ice>

[[java:package:com.zeroc]]
[cs:namespace:ZeroC]
module Ice
{
    // These definitions help with the encoding of proxies.

    /// The InvocationMode is carried by proxies that use the ice1 protocol, and it specifies the behavior when sending
    /// a request using such a proxy.
    enum InvocationMode : byte
    {
        /// This is the default invocation mode; a request using this mode always expects a response.
        Twoway,

        /// A request using oneway mode returns control to the application code as soon as it has been accepted by the
        /// local transport.
        Oneway,

        /// The batch oneway invocation mode is no longer supported, it was supported with Ice versions up to 3.7.
        BatchOneway,

        /// Invocation mode used by datagram based transports.
        Datagram,

        /// The batch datagram invocation mode is no longer supported, it was supported with Ice versions up to 3.7.
        BatchDatagram,
    }

    /// With the 1.1 encoding, a proxy is encoded as a kind of discriminated union with:
    /// - Identity
    /// - if Identity is not the null identity:
    ///     - ProxyData11
    ///     - a sequence of endpoints that can be empty
    ///     - an adapter ID string (renamed location in Ice 4.0) present only when the sequence of endpoints is empty
    struct ProxyData11
    {
        StringSeq facetPath;
        InvocationMode invocationMode;
        bool secure;        // ignored
        Protocol protocol;
        byte protocolMinor; // always 0
        Encoding encoding;
    }

    /// The kind of proxy being marshaled/unmarshaled (2.0 encoding only)
    enum ProxyKind : byte
    {
        /// A null proxy.
        Null,

        /// A proxy with one or more endpoints; its URI scheme is ice+transport, where transport corresponds to the
        /// transport of the first endpoint.
        Direct,

        /// A proxy with no endpoint; its URI scheme is ice.
        Indirect,
    }

    /// With the 2.0 encoding, a proxy is encoded as a discrimated union with:
    /// - ProxyKind (the discriminant)
    /// - if ProxyKind is not Null:
    ///    - ProxyData20
    ///    - If ProxyKind is Direct, a sequence of one or more endpoints
    struct ProxyData20
    {
        Identity identity;
        Protocol? protocol;
        Encoding? encoding;
        StringSeq? location;
        InvocationMode? invocationMode;
        string? facet;
    }
}
