// Copyright (c) ZeroC, Inc.

import Foundation
import IceImpl

/// The base protocol for all Ice proxies.
public protocol ObjectPrx: CustomStringConvertible, AnyObject {
    /// Returns the communicator that created this proxy.
    ///
    /// - returns: `Ice.Communicator` - The communicator that created this proxy.
    func ice_getCommunicator() -> Communicator

    /// Returns the identity embedded in this proxy.
    ///
    /// - returns: `Ice.Identity` - The identity of the target object.
    func ice_getIdentity() -> Identity

    /// Creates a new proxy that is identical to this proxy, except for the identity.
    ///
    /// - parameter id: `Ice.Identity` - The identity for the new proxy.
    ///
    /// - returns: A proxy with the new identity.
    func ice_identity(_ id: Identity) -> Self

    /// Returns the per-proxy context for this proxy.
    ///
    /// - returns: `Ice.Context` - The per-proxy context.
    func ice_getContext() -> Context

    /// Creates a new proxy that is identical to this proxy, except for the per-proxy context.
    ///
    /// - parameter context: `Ice.Context` - The context for the new proxy.
    ///
    /// - returns: The proxy with the new per-proxy context.
    func ice_context(_ context: Context) -> Self

    /// Returns the facet for this proxy.
    ///
    /// - returns: `String` - The facet for this proxy. If the proxy uses the default facet,
    ///   the return value is the empty string.
    func ice_getFacet() -> String

    /// Creates a new proxy that is identical to this proxy, except for the facet.
    ///
    /// - parameter facet: `String` - The facet for the new proxy.
    ///
    /// - returns: `Ice.ObjectPrx` - The proxy with the new facet.
    func ice_facet(_ facet: String) -> ObjectPrx

    /// Returns the adapter ID for this proxy.
    ///
    /// - returns: `String` - The adapter ID. If the proxy does not have an adapter ID, the return value is the
    ///   empty string.
    func ice_getAdapterId() -> String

    /// Creates a new proxy that is identical to this proxy, except for the adapter ID.
    ///
    /// - parameter id: `String` - The adapter ID for the new proxy.
    ///
    /// - returns: The proxy with the new adapter ID.
    func ice_adapterId(_ id: String) -> Self

    /// Returns the endpoints used by this proxy.
    ///
    /// - returns: `EndpointSeq` - The endpoints used by this proxy.
    func ice_getEndpoints() -> EndpointSeq

    /// Creates a new proxy that is identical to this proxy, except for the endpoints.
    ///
    /// - parameter endpoints: `EndpointSeq` - The endpoints for the new proxy.
    ///
    /// - returns: The proxy with the new endpoints.
    func ice_endpoints(_ endpoints: EndpointSeq) -> Self

    /// Returns the locator cache timeout of this proxy.
    ///
    /// - returns: `Int32` - The locator cache timeout value (in seconds).
    func ice_getLocatorCacheTimeout() -> Int32

    /// Creates a new proxy that is identical to this proxy, except for the locator cache timeout.
    ///
    /// - parameter timeout: `Int32` - The new locator cache timeout (in seconds).
    ///
    /// - returns: A new proxy with the specified cache timeout.
    func ice_locatorCacheTimeout(_ timeout: Int32) -> Self

    /// Returns the invocation timeout of this proxy.
    ///
    /// - returns: `Int32` - The invocation timeout value (in seconds).
    func ice_getInvocationTimeout() -> Int32

    /// Creates a new proxy that is identical to this proxy, except for the invocation timeout.
    ///
    /// - parameter timeout: `Int32` - The new invocation timeout (in seconds).
    ///
    /// - returns: A new proxy with the specified invocation timeout.
    func ice_invocationTimeout(_ timeout: Int32) -> Self

    /// Returns the connection id of this proxy.
    ///
    /// returns: `String` - The connection id.
    func ice_getConnectionId() -> String

    /// Creates a new proxy that is identical to this proxy, except for its connection ID.
    ///
    /// - parameter id: `String` - The connection ID for the new proxy. An empty string removes the
    ///   connection ID.
    ///
    /// - returns: A new proxy with the specified connection ID.
    func ice_connectionId(_ id: String) -> Self

    /// Returns whether this proxy caches connections.
    ///
    /// - returns: `Bool` - True if this proxy caches connections; false, otherwise.
    func ice_isConnectionCached() -> Bool

    /// Creates a new proxy that is identical to this proxy, except for connection caching.
    ///
    /// - parameter cached: `Bool` - True if the new proxy should cache connections; false, otherwise.
    ///
    /// - returns: The new proxy with the specified caching policy.
    func ice_connectionCached(_ cached: Bool) -> Self

    /// Returns how this proxy selects endpoints (randomly or ordered).
    ///
    /// - returns: `Ice.EndpointSelectionType` - The endpoint selection policy.
    func ice_getEndpointSelection() -> EndpointSelectionType

    /// Creates a new proxy that is identical to this proxy, except for the endpoint selection policy.
    ///
    /// - parameter type: `Ice.EndpointSelectionType` - The new endpoint selection policy.
    ///
    /// - returns: The new proxy with the specified endpoint selection policy.
    func ice_endpointSelection(_ type: EndpointSelectionType) -> Self

    /// Returns the encoding version used to marshal requests parameters.
    ///
    /// - returns: `Ice.EncodingVersion` - The encoding version.
    func ice_getEncodingVersion() -> EncodingVersion

    /// Creates a new proxy that is identical to this proxy, except for the encoding used to marshal
    /// parameters.
    ///
    /// - parameter encoding: `Ice.EncodingVersion` - The encoding version to use to marshal requests parameters.
    ///
    /// - returns: The new proxy with the specified encoding version.
    func ice_encodingVersion(_ encoding: EncodingVersion) -> Self

    /// Returns the router for this proxy.
    ///
    /// - returns: `Ice.RouterPrx?` - The router for the proxy. If no router is configured for the proxy,
    ///   the return value is nil.
    func ice_getRouter() -> RouterPrx?

    /// Creates a new proxy that is identical to this proxy, except for the router.
    ///
    /// - parameter router: `Ice.RouterPrx?` - The router for the new proxy.
    ///
    /// - returns: The new proxy with the specified router.
    func ice_router(_ router: RouterPrx?) -> Self

    /// Returns the locator for this proxy.
    ///
    /// - returns: `Ice.LocatorPrx?` - The locator for this proxy. If no locator is configured, the
    ///   return value is nil.
    func ice_getLocator() -> LocatorPrx?

    /// Creates a new proxy that is identical to this proxy, except for the locator.
    ///
    /// - parameter locator: `Ice.LocatorPrx` The locator for the new proxy.
    ///
    /// - returns: The new proxy with the specified locator.
    func ice_locator(_ locator: LocatorPrx?) -> Self

    /// Returns whether this proxy communicates only via secure endpoints.
    ///
    /// - returns: `Bool` - True if this proxy communicates only via secure endpoints; false, otherwise.
    func ice_isSecure() -> Bool

    /// Creates a new proxy that is identical to this proxy, except for how it selects endpoints.
    ///
    /// - parameter secure: `Bool` - If true only endpoints that use a secure transport are used by the new proxy.
    ///   otherwise the returned proxy uses both secure and insecure endpoints.
    ///
    /// - returns: The new proxy with the specified selection policy.
    func ice_secure(_ secure: Bool) -> Self

    /// Returns whether this proxy prefers secure endpoints.
    ///
    /// - returns: `Bool` - True if the proxy always attempts to invoke via secure endpoints before it
    ///   attempts to use insecure endpoints; false, otherwise.
    func ice_isPreferSecure() -> Bool

    /// Creates a new proxy that is identical to this proxy, except for its endpoint selection policy.
    ///
    /// - parameter preferSecure: `Bool` - If true, the new proxy will use secure endpoints for invocations
    ///   and only use insecure endpoints if an invocation cannot be made via secure endpoints. Otherwise
    ///   the proxy prefers insecure endpoints to secure ones.
    ///
    /// - returns: The new proxy with the new endpoint selection policy.
    func ice_preferSecure(_ preferSecure: Bool) -> Self

    /// Returns whether this proxy uses twoway invocations.
    ///
    /// - returns: `Bool` - True if this proxy uses twoway invocations; false, otherwise.
    func ice_isTwoway() -> Bool

    /// Creates a new proxy that is identical to this proxy, but uses twoway invocations.
    ///
    /// - returns: A new proxy that uses twoway invocations.
    func ice_twoway() -> Self

    /// Returns whether this proxy uses oneway invocations.
    ///
    /// - returns: `Bool` - True if this proxy uses oneway invocations; false, otherwise.
    func ice_isOneway() -> Bool

    /// Creates a new proxy that is identical to this proxy, but uses oneway invocations.
    ///
    /// - returns: A new proxy that uses oneway invocations.
    func ice_oneway() -> Self

    /// Returns whether this proxy uses batch oneway invocations.
    ///
    /// - returns: `Bool` - True if this proxy uses batch oneway invocations; false, otherwise.
    func ice_isBatchOneway() -> Bool

    /// Creates a new proxy that is identical to this proxy, but uses batch oneway invocations.
    ///
    /// - returns: A new proxy that uses batch oneway invocations.
    func ice_batchOneway() -> Self

    /// Returns whether this proxy uses datagram invocations.
    ///
    /// - returns: `Bool` - True if this proxy uses datagram invocations; false, otherwise.
    func ice_isDatagram() -> Bool

    /// Creates a new proxy that is identical to this proxy, but uses datagram invocations.
    ///
    /// - returns: A new proxy that uses datagram invocations.
    func ice_datagram() -> Self

    /// Returns whether this proxy uses batch datagram invocations.
    ///
    /// - returns: `Bool` - True if this proxy uses batch datagram invocations; false, otherwise.
    func ice_isBatchDatagram() -> Bool

    /// Creates a new proxy that is identical to this proxy, but uses batch datagram invocations.
    ///
    /// - returns: A new proxy that uses batch datagram invocations.
    func ice_batchDatagram() -> Self

    /// Obtains the compression override setting of this proxy.
    ///
    /// - returns: `Bool` - The compression override setting. If no optional value is present, no override is
    ///   set. Otherwise, true if compression is enabled, false otherwise.
    func ice_getCompress() -> Bool?

    /// Creates a new proxy that is identical to this proxy, except for compression.
    ///
    /// - parameter compress: `Bool` - True enables compression for the new proxy; false disables compression.
    ///
    /// - returns: A new proxy with the specified compression setting.
    func ice_compress(_ compress: Bool) -> Self

    /// Returns a proxy that is identical to this proxy, except it's a fixed proxy bound
    /// to the given connection.
    ///
    /// - parameter connection: `Ice.Connection` - The fixed proxy connection.
    ///
    /// - returns: A fixed proxy bound to the given connection.
    func ice_fixed(_ connection: Connection) -> Self

    /// Returns whether this proxy is a fixed proxy.
    ///
    /// - returns: `Bool` - True if this is a fixed proxy, false otherwise.
    func ice_isFixed() -> Bool

    /// Returns the cached Connection for this proxy. If the proxy does not yet have an established
    /// connection, it does not attempt to create a connection.
    ///
    /// - returns: `Ice.Connection?` - The cached Connection for this proxy (nil if the proxy does not have
    ///   an established connection).
    func ice_getCachedConnection() -> Connection?

    /// Returns the stringified form of this proxy.
    ///
    /// - returns: `String` - The stringified proxy
    func ice_toString() -> String

    /// Returns whether this proxy uses collocation optimization.
    ///
    /// - returns: `Bool` - True if the proxy uses collocation optimization; false, otherwise.
    func ice_isCollocationOptimized() -> Bool

    /// Creates a new proxy that is identical to this proxy, except for collocation optimization.
    ///
    /// - parameter collocated: `Bool` - True if the new proxy enables collocation optimization; false, otherwise.
    ///
    /// - returns: The new proxy the specified collocation optimization.
    func ice_collocationOptimized(_ collocated: Bool) -> Self
}

/// Makes a new proxy from a communicator and a proxy string.
///
/// - Parameters:
///    - communicator: The communicator of the new proxy.
///    - proxyString: The proxy string to parse.
///    - type: The type of the new proxy.
/// - Throws: `Ice.ParseException` if the proxy string is invalid.
/// - Returns: A new proxy with the requested type.
public func makeProxy(communicator: Ice.Communicator, proxyString: String, type: ObjectPrx.Protocol)
    throws -> ObjectPrx
{
    try communicator.makeProxyImpl(proxyString) as ObjectPrxI
}

/// Casts a proxy to `Ice.ObjectPrx`. This call contacts the server and will throw an Ice run-time exception
/// if the target object does not exist or the server cannot be reached.
///
/// - parameter prx: `Ice.ObjectPrx` - The proxy to cast to `Ice.ObjectPrx`.
///
/// - parameter type: `Ice.ObjectPrx.Protocol` - The proxy type to cast to.
///
/// - parameter facet: `String?` - The optional facet for the new proxy.
///
/// - parameter context: `Ice.Context?` - The optional context dictionary for the invocation.
///
/// - throws: Throws an Ice run-time exception if the target object does not exist, the specified facet
///   does not exist, or the server cannot be reached.
///
/// - returns: The new proxy with the specified facet or nil if the target object does not support the specified
///   interface.
public func checkedCast(
    prx: Ice.ObjectPrx,
    type _: ObjectPrx.Protocol,
    facet: String? = nil,
    context: Ice.Context? = nil
) async throws -> ObjectPrx? {
    return try await ObjectPrxI.checkedCast(prx: prx, facet: facet, context: context) as ObjectPrxI?
}

/// Creates a new proxy that is identical to the passed proxy, except for its facet. This call does
/// not contact the server and always succeeds.
///
/// - parameter prx: `Ice.ObjectPrx` - The proxy to cast to `Ice.ObjectPrx`.
///
/// - parameter type: `Ice.ObjectPrx.Protocol` - The proxy type to cast to.
///
/// - parameter facet: `String?` - The optional facet for the new proxy.
///
/// - returns: The new proxy with the specified facet.
public func uncheckedCast(
    prx: Ice.ObjectPrx,
    type _: ObjectPrx.Protocol,
    facet: String? = nil
) -> ObjectPrx {
    return ObjectPrxI.uncheckedCast(prx: prx, facet: facet) as ObjectPrxI
}

/// Returns the Slice type id of the interface or class associated with this proxy class.
///
/// - returns: `String` - The type id, "::Ice::Object".
public func ice_staticId(_: ObjectPrx.Protocol) -> String {
    return ObjectTraits.staticId
}

public func != (lhs: ObjectPrx?, rhs: ObjectPrx?) -> Bool {
    return !(lhs == rhs)
}

public func == (lhs: ObjectPrx?, rhs: ObjectPrx?) -> Bool {
    if lhs === rhs {
        return true
    } else if lhs === nil && rhs === nil {
        return true
    } else if lhs === nil || rhs === nil {
        return false
    } else {
        let lhsI = lhs as! ObjectPrxI
        let rhsI = rhs as! ObjectPrxI
        return lhsI.handle.isEqual(rhsI.handle)
    }
}

extension ObjectPrx {
    /// Returns the underlying implementation object (Ice internal).
    public var _impl: ObjectPrxI {
        return self as! ObjectPrxI
    }

    /// Sends ping request to the target object.
    ///
    /// - parameter context: `Ice.Context` - The optional context dictionary for the invocation.
    public func ice_ping(
        context: Context? = nil
    ) async throws {
        return try await _impl._invoke(
            operation: "ice_ping",
            mode: .idempotent,
            context: context)
    }

    /// Tests whether this object supports a specific Slice interface.
    ///
    /// - parameter id: `String` - The type ID of the Slice interface to test against.
    ///
    /// - parameter context: `Ice.Context` - The optional context dictionary for the invocation.
    ///
    /// - returns: `Bool` - The result of the invocation.
    public func ice_isA(
        id: String, context: Context? = nil
    ) async throws -> Bool {
        return try await _impl._invoke(
            operation: "ice_isA",
            mode: .idempotent,
            write: { ostr in
                ostr.write(id)
            },
            read: { istr in try istr.read() as Bool },
            context: context)
    }

    /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
    ///
    /// - parameter context: `Ice.Context?` - The optional context dictionary for the invocation.
    ///
    /// - returns: `String` The result of the invocation.
    public func ice_id(
        context: Context? = nil
    ) async throws -> String {
        return try await _impl._invoke(
            operation: "ice_id",
            mode: .idempotent,
            read: { istr in try istr.read() as String },
            context: context)
    }

    /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
    ///
    /// - parameter context: `Ice.Context?` - The optional context dictionary for the invocation.
    ///
    /// - returns: `Ice.StringSeq` - The result of the invocation.
    public func ice_ids(
        context: Context? = nil
    ) async throws -> StringSeq {
        return try await _impl._invoke(
            operation: "ice_ids",
            mode: .idempotent,
            read: { istr in try istr.read() as StringSeq },
            context: context)
    }

    /// Invokes an operation dynamically.
    ///
    /// - parameter operation: `String` - The name of the operation to invoke.
    ///
    /// - parameter mode: `Ice.OperationMode` - The operation mode (normal or idempotent).
    ///
    /// - parameter inEncaps: `Data` - The encoded in-parameters for the operation.
    ///
    /// - parameter context: `Ice.Context` - The context dictionary for the invocation.
    ///
    /// - returns: `(ok: Bool, outEncaps: Data)` - The result of the invocation.
    public func ice_invoke(
        operation: String,
        mode: OperationMode,
        inEncaps: Data,
        context: Context? = nil
    ) async throws -> (ok: Bool, outEncaps: Data) {
        if _impl.isTwoway {
            return try await withCheckedThrowingContinuation { continuation in
                _impl.handle.invoke(
                    operation,
                    mode: mode.rawValue,
                    inParams: inEncaps,
                    context: context,
                    response: { ok, bytes, count in
                        do {
                            let istr =
                                InputStream(
                                    communicator: self._impl.communicator,
                                    encoding: self._impl.encoding,
                                    bytes: Data(bytes: bytes, count: count))  // make a copy
                            try continuation.resume(returning: (ok, istr.readEncapsulation().bytes))
                        } catch {
                            continuation.resume(throwing: error)
                        }
                    },
                    exception: { error in
                        continuation.resume(throwing: error)
                    })
            }
        } else if ice_isBatchOneway() || ice_isBatchDatagram() {
            return try autoreleasepool {
                try _impl.handle.enqueueBatch(
                    operation,
                    mode: mode.rawValue,
                    inParams: inEncaps,
                    context: context)

                return (true, Data())
            }
        } else {
            return try await withCheckedThrowingContinuation { continuation in
                _impl.handle.invoke(
                    operation,
                    mode: mode.rawValue,
                    inParams: inEncaps,
                    context: context,
                    response: { _, _, _ in
                        fatalError("unexpected response")
                    },
                    exception: { error in
                        continuation.resume(throwing: error)
                    },
                    sent: { _ in
                        continuation.resume(returning: (true, Data()))
                    })
            }
        }
    }

    /// Returns the connection for this proxy. If the proxy does not yet have an established connection,
    /// it first attempts to create a connection.
    ///
    /// - returns: `Ice.Connection?` - The result of the invocation.
    public func ice_getConnection() async throws -> Connection? {
        return try await withCheckedThrowingContinuation { continuation in
            self._impl.handle.ice_getConnection(
                { connection in
                    continuation.resume(
                        returning:
                            connection?.getSwiftObject(ConnectionI.self) {
                                ConnectionI(handle: connection!)
                            })
                }, exception: { ex in continuation.resume(throwing: ex) })
        }
    }

    /// Flushes any pending batched requests for this proxy.
    public func ice_flushBatchRequests() async throws {
        return try await withCheckedThrowingContinuation { continuation in
            _impl.handle.ice_flushBatchRequests(
                exception: {
                    continuation.resume(throwing: $0)
                },
                sent: { _ in
                    continuation.resume(returning: ())
                }
            )
        }
    }
}

//
// ObjectPrxI, the base proxy implementation class is an Ice-internal class used in the
// generated code - this is why we give it the open access level.
//
open class ObjectPrxI: ObjectPrx {
    let handle: ICEObjectPrx
    let communicator: Communicator
    let encoding: EncodingVersion
    fileprivate let isTwoway: Bool

    public var description: String {
        return handle.ice_toString()
    }

    public required init(handle: ICEObjectPrx, communicator: Communicator) {
        self.handle = handle
        self.communicator = communicator
        var encoding = EncodingVersion()
        handle.ice_getEncodingVersion(&encoding.major, minor: &encoding.minor)
        self.encoding = encoding
        isTwoway = handle.ice_isTwoway()
    }

    public required init(from prx: ObjectPrx) {
        let impl = prx as! ObjectPrxI
        handle = impl.handle
        communicator = impl.communicator
        encoding = impl.encoding
        isTwoway = impl.isTwoway
    }

    private func fromICEObjectPrx<ProxyImpl>(_ h: ICEObjectPrx) -> ProxyImpl
    where ProxyImpl: ObjectPrxI {
        return ProxyImpl(handle: h, communicator: communicator)
    }

    static func fromICEObjectPrx(
        handle: ICEObjectPrx,
        communicator c: Communicator? = nil
    ) -> Self {
        let communicator = c ?? handle.ice_getCommunicator().getCachedSwiftObject(CommunicatorI.self)
        return self.init(handle: handle, communicator: communicator)
    }

    public func ice_getCommunicator() -> Communicator {
        return communicator
    }

    open class func ice_staticId() -> String {
        return ObjectTraits.staticId
    }

    public func ice_getIdentity() -> Identity {
        var name = NSString()
        var category = NSString()
        handle.ice_getIdentity(&name, category: &category)
        return Identity(name: name as String, category: category as String)
    }

    public func ice_identity(_ id: Identity) -> Self {
        precondition(!id.name.isEmpty, "Identity name cannot be empty")
        do {
            return try autoreleasepool {
                try fromICEObjectPrx(handle.ice_identity(id.name, category: id.category)) as Self
            }
        } catch {
            fatalError("\(error)")
        }
    }

    public func ice_getContext() -> Context {
        return handle.ice_getContext() as Context
    }

    public func ice_context(_ context: Context) -> Self {
        return fromICEObjectPrx(handle.ice_context(context))
    }

    public func ice_getFacet() -> String {
        return handle.ice_getFacet()
    }

    public func ice_facet(_ facet: String) -> ObjectPrx {
        return fromICEObjectPrx(handle.ice_facet(facet))
    }

    public func ice_getAdapterId() -> String {
        return handle.ice_getAdapterId()
    }

    public func ice_adapterId(_ id: String) -> Self {
        precondition(!ice_isFixed(), "Cannot create a fixed proxy with an adapterId")
        do {
            return try autoreleasepool {
                try fromICEObjectPrx(handle.ice_adapterId(id)) as Self
            }
        } catch {
            fatalError("\(error)")
        }
    }

    public func ice_getEndpoints() -> EndpointSeq {
        return handle.ice_getEndpoints().fromObjc()
    }

    public func ice_endpoints(_ endpoints: EndpointSeq) -> Self {
        precondition(!ice_isFixed(), "Cannot create a fixed proxy with endpoints")
        do {
            return try autoreleasepool {
                try fromICEObjectPrx(handle.ice_endpoints(endpoints.toObjc())) as Self
            }
        } catch {
            fatalError("\(error)")
        }
    }

    public func ice_getLocatorCacheTimeout() -> Int32 {
        return handle.ice_getLocatorCacheTimeout()
    }

    public func ice_locatorCacheTimeout(_ timeout: Int32) -> Self {
        precondition(!ice_isFixed(), "Cannot create a fixed proxy with a locatorCacheTimeout")
        precondition(timeout >= -1, "Invalid locator cache timeout value")
        do {
            return try autoreleasepool {
                try fromICEObjectPrx(handle.ice_locatorCacheTimeout(timeout)) as Self
            }
        } catch {
            fatalError("\(error)")
        }
    }

    public func ice_getInvocationTimeout() -> Int32 {
        return handle.ice_getInvocationTimeout()
    }

    public func ice_invocationTimeout(_ timeout: Int32) -> Self {
        precondition(timeout >= 1 || timeout == -1 || timeout == -2, "Invalid invocation timeout value")
        do {
            return try autoreleasepool {
                try fromICEObjectPrx(handle.ice_invocationTimeout(timeout)) as Self
            }
        } catch {
            fatalError("\(error)")
        }
    }

    public func ice_getConnectionId() -> String {
        return handle.ice_getConnectionId()
    }

    public func ice_connectionId(_ id: String) -> Self {
        precondition(!ice_isFixed(), "Cannot create a fixed proxy with a connectionId")
        do {
            return try autoreleasepool {
                try fromICEObjectPrx(handle.ice_connectionId(id)) as Self
            }
        } catch {
            fatalError("\(error)")
        }
    }

    public func ice_isConnectionCached() -> Bool {
        return handle.ice_isConnectionCached()
    }

    public func ice_connectionCached(_ cached: Bool) -> Self {
        precondition(!ice_isFixed(), "Cannot create a fixed proxy with a cached connection")
        do {
            return try autoreleasepool {
                try fromICEObjectPrx(handle.ice_connectionCached(cached)) as Self
            }
        } catch {
            fatalError("\(error)")
        }
    }

    public func ice_getEndpointSelection() -> EndpointSelectionType {
        return EndpointSelectionType(rawValue: handle.ice_getEndpointSelection())!
    }

    public func ice_endpointSelection(_ type: EndpointSelectionType) -> Self {
        precondition(!ice_isFixed(), "Cannot create a fixed proxy with an endpointSelectionType")
        do {
            return try autoreleasepool {
                try fromICEObjectPrx(handle.ice_endpointSelection(type.rawValue)) as Self
            }
        } catch {
            fatalError("\(error)")
        }
    }

    public func ice_getEncodingVersion() -> EncodingVersion {
        return encoding
    }

    public func ice_encodingVersion(_ encoding: EncodingVersion) -> Self {
        return fromICEObjectPrx(handle.ice_encodingVersion(encoding.major, minor: encoding.minor))
    }

    public func ice_getRouter() -> RouterPrx? {
        guard let routerHandle = handle.ice_getRouter() else {
            return nil
        }
        return fromICEObjectPrx(routerHandle) as RouterPrxI
    }

    public func ice_router(_ router: RouterPrx?) -> Self {
        precondition(!ice_isFixed(), "Cannot create a fixed proxy with a router")
        do {
            return try autoreleasepool {
                let r = router as? ObjectPrxI
                return try fromICEObjectPrx(handle.ice_router(r?.handle ?? nil))
            }
        } catch {
            fatalError("\(error)")
        }
    }

    public func ice_getLocator() -> LocatorPrx? {
        guard let locatorHandle = handle.ice_getLocator() else {
            return nil
        }
        return fromICEObjectPrx(locatorHandle) as LocatorPrxI
    }

    public func ice_locator(_ locator: LocatorPrx?) -> Self {
        precondition(!ice_isFixed(), "Cannot create a fixed proxy with a locator")
        do {
            return try autoreleasepool {
                let l = locator as? ObjectPrxI
                return try fromICEObjectPrx(handle.ice_locator(l?.handle ?? nil))
            }
        } catch {
            fatalError("\(error)")
        }
    }

    public func ice_isSecure() -> Bool {
        return handle.ice_isSecure()
    }

    public func ice_secure(_ secure: Bool) -> Self {
        return fromICEObjectPrx(handle.ice_secure(secure))
    }

    public func ice_isPreferSecure() -> Bool {
        return handle.ice_isPreferSecure()
    }

    public func ice_preferSecure(_ preferSecure: Bool) -> Self {
        precondition(!ice_isFixed(), "Cannot create a fixed proxy with preferSecure")
        do {
            return try autoreleasepool {
                try fromICEObjectPrx(handle.ice_preferSecure(preferSecure)) as Self
            }
        } catch {
            fatalError("\(error)")
        }
    }

    public func ice_isTwoway() -> Bool {
        return isTwoway
    }

    public func ice_twoway() -> Self {
        return fromICEObjectPrx(handle.ice_twoway())
    }

    public func ice_isOneway() -> Bool {
        return handle.ice_isOneway()
    }

    public func ice_oneway() -> Self {
        return fromICEObjectPrx(handle.ice_oneway())
    }

    public func ice_isBatchOneway() -> Bool {
        return handle.ice_isBatchOneway()
    }

    public func ice_batchOneway() -> Self {
        return fromICEObjectPrx(handle.ice_batchOneway())
    }

    public func ice_isDatagram() -> Bool {
        return handle.ice_isDatagram()
    }

    public func ice_datagram() -> Self {
        return fromICEObjectPrx(handle.ice_datagram())
    }

    public func ice_isBatchDatagram() -> Bool {
        return handle.ice_isBatchDatagram()
    }

    public func ice_batchDatagram() -> Self {
        return fromICEObjectPrx(handle.ice_batchDatagram())
    }

    public func ice_getCompress() -> Bool? {
        guard let compress = handle.ice_getCompress() as? Bool? else {
            preconditionFailure("Bool? type was expected")
        }
        return compress
    }

    public func ice_compress(_ compress: Bool) -> Self {
        return fromICEObjectPrx(handle.ice_compress(compress))
    }

    public func ice_fixed(_ connection: Connection) -> Self {
        do {
            return try autoreleasepool {
                try fromICEObjectPrx(handle.ice_fixed((connection as! ConnectionI).handle)) as Self
            }
        } catch {
            fatalError("\(error)")
        }
    }

    public func ice_isFixed() -> Bool {
        return handle.ice_isFixed()
    }

    public func ice_getCachedConnection() -> Connection? {
        guard let handle = handle.ice_getCachedConnection() else {
            return nil
        }
        return handle.getSwiftObject(ConnectionI.self) { ConnectionI(handle: handle) }
    }

    public func ice_write(to os: OutputStream) {
        handle.ice_write(
            os, encodingMajor: os.currentEncoding.major, encodingMinor: os.currentEncoding.minor)
    }

    public func ice_toString() -> String {
        return handle.ice_toString()
    }

    public func ice_isCollocationOptimized() -> Bool {
        return handle.ice_isCollocationOptimized()
    }

    public func ice_collocationOptimized(_ collocated: Bool) -> Self {
        precondition(!ice_isFixed(), "Cannot create a fixed proxy with collocation optimization")
        do {
            return try autoreleasepool {
                try fromICEObjectPrx(handle.ice_collocationOptimized(collocated)) as Self
            }
        } catch {
            fatalError("\(error)")
        }
    }

    public static func ice_read(from istr: InputStream) throws -> Self? {
        //
        // Unmarshaling of proxies is done in C++. Since we don't know how big this proxy will
        // be we pass the current buffer position and remaining buffer capacity.
        //

        // The number of bytes consumed reading the proxy
        var bytesRead = 0
        let encoding = istr.currentEncoding
        let communicator = istr.communicator

        //
        // Returns Any which is either NSNull or ICEObjectPrx
        //
        let handleOpt =
            try ICEObjectPrx.ice_read(
                istr.data[istr.pos..<istr.data.count],
                communicator: (communicator as! CommunicatorI).handle,
                encodingMajor: encoding.major,
                encodingMinor: encoding.minor,
                bytesRead: &bytesRead) as? ICEObjectPrx

        // Since the proxy was read in C++ we need to skip over the bytes which were read
        // We avoid using a defer statement for this since you can not throw from one
        try istr.skip(bytesRead)

        guard let handle = handleOpt else {
            return nil
        }

        return self.init(handle: handle, communicator: communicator)
    }

    public func _invoke(
        operation: String,
        mode: OperationMode,
        format: FormatType? = nil,
        write: ((OutputStream) -> Void)? = nil,
        userException: ((UserException) throws -> Void)? = nil,
        context: Context? = nil
    ) async throws {

        if userException != nil, !isTwoway {
            throw TwowayOnlyException(operation: operation)
        }
        let ostr = OutputStream(communicator: communicator)
        if let write = write {
            ostr.startEncapsulation(encoding: encoding, format: format)
            write(ostr)
            ostr.endEncapsulation()
        }
        if isTwoway {
            return try await withCheckedThrowingContinuation { continuation in
                handle.invoke(
                    operation,
                    mode: mode.rawValue,
                    inParams: ostr.finished(),
                    context: context,
                    response: { ok, bytes, count in
                        do {
                            let istr = InputStream(
                                communicator: self.communicator,
                                encoding: self.encoding,
                                bytes: Data(
                                    bytesNoCopy: bytes, count: count,
                                    deallocator: .none))
                            if ok == false {
                                try ObjectPrxI.throwUserException(
                                    istr: istr,
                                    userException: userException)
                            }
                            try istr.skipEmptyEncapsulation()
                            continuation.resume(returning: ())
                        } catch {
                            continuation.resume(throwing: error)
                        }
                    },
                    exception: { error in
                        continuation.resume(throwing: error)
                    })
            }
        } else if ice_isBatchOneway() || ice_isBatchDatagram() {
            return try autoreleasepool {
                try _impl.handle.enqueueBatch(
                    operation,
                    mode: mode.rawValue,
                    inParams: ostr.finished(),
                    context: context)
            }
        } else {
            return try await withCheckedThrowingContinuation { continuation in
                handle.invoke(
                    operation,
                    mode: mode.rawValue,
                    inParams: ostr.finished(),
                    context: context,
                    response: { _, _, _ in
                        fatalError("unexpected response")
                    },
                    exception: { error in
                        continuation.resume(throwing: error)
                    },
                    sent: { _ in
                        continuation.resume(returning: ())
                    })
            }
        }
    }

    public func _invoke<T>(
        operation: String,
        mode: OperationMode,
        format: FormatType? = nil,
        write: ((OutputStream) -> Void)? = nil,
        read: @escaping (InputStream) throws -> T,
        userException: ((UserException) throws -> Void)? = nil,
        context: Context? = nil
    ) async throws -> T {
        if !isTwoway {
            throw TwowayOnlyException(operation: operation)
        }

        let ostr = OutputStream(communicator: communicator)
        if let write = write {
            ostr.startEncapsulation(encoding: encoding, format: format)
            write(ostr)
            ostr.endEncapsulation()
        }

        return try await withCheckedThrowingContinuation { continuation in
            handle.invoke(
                operation,
                mode: mode.rawValue,
                inParams: ostr.finished(),
                context: context,
                response: { ok, bytes, count in
                    do {
                        let istr = InputStream(
                            communicator: self.communicator,
                            encoding: self.encoding,
                            bytes: Data(
                                bytesNoCopy: bytes, count: count,
                                deallocator: .none))
                        if ok == false {
                            try ObjectPrxI.throwUserException(
                                istr: istr,
                                userException: userException)
                        }
                        try istr.startEncapsulation()
                        let l = try read(istr)
                        try istr.endEncapsulation()
                        continuation.resume(returning: l)
                    } catch {
                        continuation.resume(throwing: error)
                    }
                },
                exception: { error in
                    continuation.resume(throwing: error)
                })
        }
    }

    private static func throwUserException(
        istr: InputStream, userException: ((UserException) throws -> Void)?
    ) throws {
        do {
            try istr.startEncapsulation()
            try istr.throwException()
        } catch let error as UserException {
            try istr.endEncapsulation()
            if let userException = userException {
                try userException(error)
            }
            throw UnknownUserException(badTypeId: error.ice_id())
        }
        fatalError("failed to throw user exception")
    }

    public static func checkedCast<ProxyImpl>(
        prx: ObjectPrx,
        facet: String? = nil,
        context: Context? = nil
    ) async throws -> ProxyImpl?
    where ProxyImpl: ObjectPrxI {
        let objPrx = facet != nil ? prx.ice_facet(facet!) : prx

        // checkedCast always calls ice_isA - no optimization on purpose
        guard try await objPrx.ice_isA(id: ProxyImpl.ice_staticId(), context: context) else {
            return nil
        }
        return ProxyImpl(from: objPrx)
    }

    public static func uncheckedCast<ProxyImpl>(
        prx: ObjectPrx,
        facet: String? = nil
    ) -> ProxyImpl where ProxyImpl: ObjectPrxI {
        if let f = facet {
            return ProxyImpl(from: prx.ice_facet(f))
        } else if let optimized = prx as? ProxyImpl {
            return optimized
        } else {
            return ProxyImpl(from: prx)
        }
    }
}
