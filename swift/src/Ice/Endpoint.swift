// Copyright (c) ZeroC, Inc.

import Foundation

/// Base class providing access to the endpoint details.
public protocol EndpointInfo: AnyObject {
    /// The information of the underyling endpoint or null if there's no underlying endpoint.
    var underlying: EndpointInfo? { get set }
    /// The timeout for the endpoint in milliseconds. 0 means non-blocking, -1 means no timeout.
    var timeout: Int32 { get set }
    /// Specifies whether or not compression should be used if available when using this endpoint.
    var compress: Bool { get set }

    /// Returns the type of the endpoint.
    ///
    /// - returns: `Int16` - The endpoint type.
    func type() -> Int16

    /// Returns true if this endpoint is a datagram endpoint.
    ///
    /// - returns: `Bool` - True for a datagram endpoint.
    func datagram() -> Bool

    ///
    /// - returns: `Bool` - True for a secure endpoint.
    func secure() -> Bool
}

/// The user-level interface to an endpoint.
public protocol Endpoint: AnyObject, CustomStringConvertible {
    /// Return a string representation of the endpoint.
    ///
    /// - returns: `String` - The string representation of the endpoint.
    func toString() -> String

    /// Returns the endpoint information.
    ///
    /// - returns: `EndpointInfo?` - The endpoint information class.
    func getInfo() -> EndpointInfo?
}

/// Provides access to the address details of a IP endpoint.
public protocol IPEndpointInfo: EndpointInfo {
    /// The host or address configured with the endpoint.
    var host: String { get set }
    /// The port number.
    var port: Int32 { get set }
    /// The source IP address.
    var sourceAddress: String { get set }
}

/// Provides access to a TCP endpoint information.
public protocol TCPEndpointInfo: IPEndpointInfo {}

/// Provides access to an UDP endpoint information.
public protocol UDPEndpointInfo: IPEndpointInfo {
    /// The multicast interface.
    var mcastInterface: String { get set }
    /// The multicast time-to-live (or hops).
    var mcastTtl: Int32 { get set }
}

/// Provides access to a WebSocket endpoint information.
public protocol WSEndpointInfo: EndpointInfo {
    /// The URI configured with the endpoint.
    var resource: String { get set }
}

/// Provides access to the details of an opaque endpoint.
public protocol OpaqueEndpointInfo: EndpointInfo {
    /// The encoding version of the opaque endpoint (to decode or encode the rawBytes).
    var rawEncoding: EncodingVersion { get set }
    /// The raw encoding of the opaque endpoint.
    var rawBytes: ByteSeq { get set }
}

public typealias EndpointSeq = [Endpoint]
