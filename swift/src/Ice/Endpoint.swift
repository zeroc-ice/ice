// Copyright (c) ZeroC, Inc.

import Foundation

/// An endpoint specifies the address of the server-end of an Ice connection: an object adapter listens on one or more
/// endpoints and a client establishes a connection to an endpoint.
public protocol Endpoint: AnyObject, CustomStringConvertible {
    /// Returns a string representation of this endpoint.
    ///
    /// - Returns: The string representation of this endpoint.
    func toString() -> String

    /// Returns this endpoint's information.
    ///
    /// - Returns: This endpoint's information class.
    func getInfo() -> EndpointInfo?
}

public typealias EndpointSeq = [Endpoint]

/// Base class for the endpoint info classes.
open class EndpointInfo {
    /// The information of the underlying endpoint or nil if there's no underlying endpoint.
    public let underlying: EndpointInfo?

    /// Specifies whether or not compression should be used if available when using this endpoint.
    public let compress: Bool

    /// Returns the type of the endpoint.
    ///
    /// - Returns: The endpoint type.
    public func type() -> Int16 {
        underlying?.type() ?? -1
    }

    /// Returns `true` if this endpoint's transport is a datagram transport (namely, UDP), `false` otherwise.
    ///
    /// - Returns: `true` for a UDP endpoint, `false` otherwise.
    public func datagram() -> Bool {
        underlying?.datagram() ?? false
    }

    /// Returns `true` if this endpoint's transport uses SSL, `false` otherwise.
    ///
    /// - Returns: `true` for SSL and SSL-based transports, `false` otherwise.
    public func secure() -> Bool {
        underlying?.secure() ?? false
    }

    public init(underlying: EndpointInfo) {
        self.underlying = underlying
        self.compress = underlying.compress
    }

    public init(compress: Bool) {
        self.underlying = nil
        self.compress = compress
    }
}

/// Provides access to the address details of an IP endpoint.
open class IPEndpointInfo: EndpointInfo {
    /// The host or address configured with the endpoint.
    public let host: String

    /// The port number.
    public let port: Int32

    /// The source IP address.
    public let sourceAddress: String

    public init(compress: Bool, host: String, port: Int32, sourceAddress: String) {
        self.host = host
        self.port = port
        self.sourceAddress = sourceAddress
        super.init(compress: compress)
    }
}

/// Provides access to a TCP endpoint information.
public final class TCPEndpointInfo: IPEndpointInfo {
    private let _type: Int16
    private let _secure: Bool

    public override func type() -> Int16 {
        _type
    }

    public override func secure() -> Bool {
        _secure
    }

    internal init(compress: Bool, host: String, port: Int32, sourceAddress: String, type: Int16, secure: Bool) {
        self._type = type
        self._secure = secure
        super.init(compress: compress, host: host, port: port, sourceAddress: sourceAddress)
    }
}

/// Provides access to an SSL endpoint information.
public final class SSLEndpointInfo: EndpointInfo {
}

/// Provides access to a UDP endpoint information.
public final class UDPEndpointInfo: IPEndpointInfo {
    /// The multicast interface.
    public let mcastInterface: String

    /// The multicast time-to-live (or hops).
    public let mcastTtl: Int32

    public override func type() -> Int16 {
        UDPEndpointType
    }

    public override func datagram() -> Bool {
        true
    }

    internal init(
        compress: Bool, host: String, port: Int32, sourceAddress: String, mcastInterface: String,
        mcastTtl: Int32
    ) {
        self.mcastInterface = mcastInterface
        self.mcastTtl = mcastTtl
        super.init(compress: compress, host: host, port: port, sourceAddress: sourceAddress)
    }
}

/// Provides access to a WebSocket endpoint information.
public final class WSEndpointInfo: EndpointInfo {
    /// The URI configured with the endpoint.
    public let resource: String

    internal init(underlying: EndpointInfo, resource: String) {
        self.resource = resource
        super.init(underlying: underlying)
    }
}

/// Provides access to an IAP endpoint information.
public final class IAPEndpointInfo: EndpointInfo {
    /// The accessory manufacturer. Can be empty.
    public let manufacturer: String

    /// The accessory model number. Can be empty.
    public let modelNumber: String

    /// The accessory name. Can be empty.
    public let name: String

    /// The protocol supported by the accessory.
    public let `protocol`: String

    private let _type: Int16
    private let _secure: Bool

    public override func type() -> Int16 {
        _type
    }

    public override func secure() -> Bool {
        _secure
    }

    internal init(
        compress: Bool, manufacturer: String, modelNumber: String, name: String, protocol: String,
        type: Int16, secure: Bool
    ) {
        self.manufacturer = manufacturer
        self.modelNumber = modelNumber
        self.name = name
        self.`protocol` = `protocol`
        self._type = type
        self._secure = secure
        super.init(compress: compress)
    }
}

/// Provides access to the details of an opaque endpoint.
public final class OpaqueEndpointInfo: EndpointInfo {
    /// The encoding version of the opaque endpoint (to decode or encode the rawBytes).
    public let rawEncoding: EncodingVersion

    /// The raw encoding of the opaque endpoint.
    public let rawBytes: ByteSeq

    private let _type: Int16

    public override func type() -> Int16 {
        _type
    }

    internal init(type: Int16, rawEncoding: EncodingVersion, rawBytes: ByteSeq) {
        self.rawEncoding = rawEncoding
        self.rawBytes = rawBytes
        self._type = type
        super.init(compress: false)
    }
}
