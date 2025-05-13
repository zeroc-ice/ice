// Copyright (c) ZeroC, Inc.

import Foundation

/// Determines the order in which the Ice run time uses the endpoints in a proxy when establishing a connection.
public enum EndpointSelectionType: UInt8 {
    /// Random Random causes the endpoints to be arranged in a random order.
    case Random = 0
    /// Ordered Ordered forces the Ice run time to use the endpoints in the order they appeared in the proxy.
    case Ordered = 1
    public init() {
        self = .Random
    }
}

/// An `Ice.InputStream` extension to read `EndpointSelectionType` enumerated values from the stream.
extension InputStream {
    /// Read an enumerated value.
    ///
    /// - returns: `EndpointSelectionType` - The enumarated value.
    public func read() throws -> EndpointSelectionType {
        let rawValue: UInt8 = try read(enumMaxValue: 1)
        guard let val = EndpointSelectionType(rawValue: rawValue) else {
            throw MarshalException("invalid enum value")
        }
        return val
    }

    /// Read an optional enumerated value from the stream.
    ///
    /// - parameter tag: `Int32` - The numeric tag associated with the value.
    ///
    /// - returns: `EndpointSelectionType` - The enumerated value.
    public func read(tag: Int32) throws -> EndpointSelectionType? {
        guard try readOptional(tag: tag, expectedFormat: .Size) else {
            return nil
        }
        return try read() as EndpointSelectionType
    }
}

/// An `Ice.OutputStream` extension to write `EndpointSelectionType` enumerated values to the stream.
extension OutputStream {
    /// Writes an enumerated value to the stream.
    ///
    /// - parameter value: `EndpointSelectionType` - The enumerator to write.
    public func write(_ value: EndpointSelectionType) {
        write(enum: value.rawValue, maxValue: 1)
    }

    /// Writes an optional enumerated value to the stream.
    ///
    /// - parameter tag: `Int32` - The numeric tag associated with the value.
    ///
    /// - parameter value: `EndpointSelectionType` - The enumerator to write.
    public func write(tag: Int32, value: EndpointSelectionType?) {
        guard let v = value else {
            return
        }
        write(tag: tag, val: v.rawValue, maxValue: 1)
    }
}
