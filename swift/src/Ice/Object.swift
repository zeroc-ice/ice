// Copyright (c) ZeroC, Inc.

import IceImpl

/// A SliceTraits struct describes a Slice interface.
public protocol SliceTraits {
    /// List of all type-ids.
    static var staticIds: [String] { get }

    /// Most derived type-id.
    static var staticId: String { get }
}

/// The common interface implemented by all Ice objects.
// Note that it does not inherit from Dispatcher on purpose.
public protocol Object {
    /// Returns the Slice type ID of the most-derived interface supported by this object.
    ///
    /// - parameter current: `Ice.Current` - The Current object for the dispatch.
    ///
    /// - returns: `String` - The Slice type ID of the most-derived interface.
    func ice_id(current: Current) async throws -> String

    /// Returns the Slice type IDs of the interfaces supported by this object.
    ///
    /// - parameter current: `Ice.Current` - The Current object for the dispatch.
    ///
    /// - returns: `[String]` The Slice type IDs of the interfaces supported by this object, in alphabetical order.
    func ice_ids(current: Current) async throws -> [String]

    /// Tests whether this object supports a specific Slice interface.
    ///
    /// - parameter id: `String` - The type ID of the Slice interface to test against.
    ///
    /// - parameter current: `Ice.Current` - The Current object for the dispatch.
    ///
    /// - returns: `Bool` - True if this object has the interface specified by s or
    ///   derives from the interface specified by s.
    func ice_isA(id: String, current: Current) async throws -> Bool

    /// Tests whether this object can be reached.
    ///
    /// - parameter current: The Current object for the dispatch.
    func ice_ping(current: Current) async throws
}

extension Object {
    public func _iceD_ice_id(_ request: IncomingRequest) async throws -> OutgoingResponse {
        _ = try request.inputStream.skipEmptyEncapsulation()
        let returnValue = try await ice_id(current: request.current)
        return request.current.makeOutgoingResponse(returnValue, formatType: .compactFormat) {
            ostr, value in
            ostr.write(value)
        }
    }

    public func _iceD_ice_ids(_ request: IncomingRequest) async throws -> OutgoingResponse {
        _ = try request.inputStream.skipEmptyEncapsulation()
        let returnValue = try await ice_ids(current: request.current)
        return request.current.makeOutgoingResponse(returnValue, formatType: .compactFormat) {
            ostr, value in
            ostr.write(value)
        }
    }

    public func _iceD_ice_isA(_ request: IncomingRequest) async throws -> OutgoingResponse {
        let istr = request.inputStream
        _ = try istr.startEncapsulation()
        let identity: String = try istr.read()
        let returnValue = try await ice_isA(id: identity, current: request.current)
        return request.current.makeOutgoingResponse(returnValue, formatType: .compactFormat) {
            ostr, value in
            ostr.write(value)
        }
    }

    public func _iceD_ice_ping(_ request: IncomingRequest) async throws -> OutgoingResponse {
        _ = try request.inputStream.skipEmptyEncapsulation()
        try await ice_ping(current: request.current)
        return request.current.makeEmptyOutgoingResponse()
    }
}

/// Traits for Object.
public struct ObjectTraits: SliceTraits {
    public static let staticIds = ["::Ice::Object"]
    public static let staticId = "::Ice::Object"
}

/// DefaultObject provides the default implementation of Object operations (ice_id,
/// ice_ping etc.) for a given Slice interface.
public struct DefaultObject<T: SliceTraits>: Object {
    public init() {}

    public func ice_id(current _: Current) -> String {
        return T.staticId
    }

    public func ice_ids(current _: Current) -> [String] {
        return T.staticIds
    }

    public func ice_isA(id: String, current _: Current) -> Bool {
        return T.staticIds.contains(id)
    }

    public func ice_ping(current _: Current) {
        // Do nothing
    }
}
