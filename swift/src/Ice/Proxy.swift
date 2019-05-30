//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import IceObjc
import PromiseKit

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
    /// - parameter _: `Ice.Identity` - The identity for the new proxy.
    ///
    /// - returns: A proxy with the new identity.
    func ice_identity(_ id: Identity) -> Self

    /// Returns the per-proxy context for this proxy.
    ///
    /// - returns: `Ice.Context` - The per-proxy context.
    func ice_getContext() -> Context

    /// Creates a new proxy that is identical to this proxy, except for the per-proxy context.
    ///
    /// - parameter newContext: `Ice.Context` - The context for the new proxy.
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
    /// - parameter _: `String` - The facet for the new proxy.
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
    /// - parameter _: `String` - The adapter ID for the new proxy.
    ///
    /// - returns: The proxy with the new adapter ID.
    func ice_adapterId(_ id: String) -> Self

    /// Returns the endpoints used by this proxy.
    ///
    /// - returns: `EndpointSeq` - The endpoints used by this proxy.
    func ice_getEndpoints() -> EndpointSeq

    /// Creates a new proxy that is identical to this proxy, except for the endpoints.
    ///
    /// - parameter _: `EndpointSeq` - The endpoints for the new proxy.
    ///
    /// - returns: The proxy with the new endpoints.
    func ice_endpoints(_ endpoints: EndpointSeq) -> Self

    /// Returns the locator cache timeout of this proxy.
    ///
    /// - returns: `Int32` - The locator cache timeout value (in seconds).
    func ice_getLocatorCacheTimeout() -> Int32

    /// Creates a new proxy that is identical to this proxy, except for the locator cache timeout.
    ///
    /// - parameter _: `Int32` - The new locator cache timeout (in seconds).
    ///
    /// - returns: A new proxy with the specified cache timeout.
    func ice_locatorCacheTimeout(_ timeout: Int32) -> Self

    /// Returns the invocation timeout of this proxy.
    ///
    /// - returns: `Int32` - The invocation timeout value (in seconds).
    func ice_getInvocationTimeout() -> Int32

    /// Creates a new proxy that is identical to this proxy, except for the invocation timeout.
    ///
    /// - parameter _: `Int32` - The new invocation timeout (in seconds).
    ///
    /// - returns: A new proxy with the specified invocation timeout.
    func ice_invocationTimeout(_ timeout: Int32) -> Self

    /// Returns the connection id of this proxy.
    ///
    /// returns: `String` - The connection id.
    func ice_getConnectionId() -> String

    /// Creates a new proxy that is identical to this proxy, except for its connection ID.
    ///
    /// - parameter _: `String` - The connection ID for the new proxy. An empty string removes the
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
    /// - parameter _: `Bool` - True if the new proxy should cache connections; false, otherwise.
    ///
    /// - returns: The new proxy with the specified caching policy.
    func ice_connectionCached(_ cached: Bool) -> Self

    /// Returns how this proxy selects endpoints (randomly or ordered).
    ///
    /// - returns: `Ice.EndpointSelectionType` - The endpoint selection policy.
    func ice_getEndpointSelection() -> EndpointSelectionType

    /// Creates a new proxy that is identical to this proxy, except for the endpoint selection policy.
    ///
    /// - parameter _: `Ice.EndpointSelectionType` - The new endpoint selection policy.
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
    /// - parameter _: `Ice.EncodingVersion` - The encoding version to use to marshal requests parameters.
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
    /// - parameter _: `Ice.LocatorPrx` The locator for the new proxy.
    ///
    /// - returns: The new proxy with the specified locator.
    func ice_locator(_ locator: LocatorPrx?) -> Self

    /// Returns whether this proxy communicates only via secure endpoints.
    ///
    /// - returns: `Bool` - True if this proxy communicates only via secure endpoints; false, otherwise.
    func ice_isSecure() -> Bool

    /// Creates a new proxy that is identical to this proxy, except for how it selects endpoints.
    ///
    /// - parameter _: `Bool` - If true only endpoints that use a secure transport are used by the new proxy.
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
    /// - parameter _: `Bool` - If true, the new proxy will use secure endpoints for invocations
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
    /// - parameter _: `Bool` - True enables compression for the new proxy; false disables compression.
    ///
    /// - returns: A new proxy with the specified compression setting.
    func ice_compress(_ compress: Bool) -> Self

    /// Obtains the timeout override of this proxy.
    ///
    /// - returns: `Int32?` - The timeout override. If no optional value is present, no override is set.
    ///   Otherwise, returns the timeout override value.
    func ice_getTimeout() -> Int32?

    /// Creates a new proxy that is identical to this proxy, except for its timeout setting.
    ///
    /// - parameter _: `Int32` - The timeout for the new proxy in milliseconds.
    ///
    /// - returns: A new proxy with the specified timeout.
    func ice_timeout(_ timeout: Int32) -> Self

    /// Returns a proxy that is identical to this proxy, except it's a fixed proxy bound
    /// to the given connection.
    ///
    /// - parameter _: `Ice.Connection` - The fixed proxy connection.
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
    ///
    /// - throws: `CollocationOptimizationException` - If the proxy uses collocation optimization and denotes a
    ///   collocated object.
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
    /// - parameter _: `Bool` - True if the new proxy enables collocation optimization; false, otherwise.
    ///
    /// - returns: The new proxy the specified collocation optimization.
    func ice_collocationOptimized(_ collocated: Bool) -> Self
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
public func checkedCast(prx: Ice.ObjectPrx,
                        type _: ObjectPrx.Protocol,
                        facet: String? = nil,
                        context: Ice.Context? = nil) throws -> ObjectPrx? {
    return try ObjectPrxI.checkedCast(prx: prx, facet: facet, context: context) as ObjectPrxI?
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
public func uncheckedCast(prx: Ice.ObjectPrx,
                          type _: ObjectPrx.Protocol,
                          facet: String? = nil) -> ObjectPrx {
    return ObjectPrxI.uncheckedCast(prx: prx, facet: facet) as ObjectPrxI
}

/// Returns the Slice type id of the interface or class associated with this proxy class.
///
/// - returns: `String` - The type id, "::Ice::Object".
public func ice_staticId(_: ObjectPrx.Protocol) -> Swift.String {
    return ObjectPrxI.ice_staticId()
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

public extension ObjectPrx {
    /// Returns the underdlying implementation object (Ice internal).
    var _impl: ObjectPrxI {
        return self as! ObjectPrxI
    }

    /// Sends ping request to the target object.
    ///
    /// - parameter context: `Ice.Context` - The optional context dictionary for the invocation.
    ///
    /// - throws: `Ice.LocalException` such as `Ice.ObjectNotExistException` and
    ///   `Ice.ConnectionRefusedException`.
    func ice_ping(context: Context? = nil) throws {
        try _impl._invoke(operation: "ice_ping",
                          mode: OperationMode.Nonmutating,
                          context: context)
    }

    /// Sends ping request to the target object asynchronously.
    ///
    /// - parameter context: `Ice.Context` - The optional context dictionary for the invocation.
    ///
    /// - parameter sentOn: `Dispatch.DispatchQueue` - Optional dispatch queue used to
    ///   dispatch sent callback, the default is to use `PromiseKit.conf.Q.return` queue.
    ///
    /// - parameter sentFlags: `Dispatch.DispatchWorkItemFlags` - Optional dispatch flags used to
    ///   dispatch sent callback
    ///
    /// - parameter sent: `((Swift.Bool) -> Swift.Void)` - Optional sent callback.
    ///
    /// - returns: `PromiseKit.Promise<Void>` - A promise object that will be resolved with
    ///   the return values of invocation.
    func ice_pingAsync(context: Context? = nil,
                       sentOn: DispatchQueue? = PromiseKit.conf.Q.return,
                       sentFlags: DispatchWorkItemFlags? = nil,
                       sent: ((Bool) -> Void)? = nil) -> Promise<Void> {
        return _impl._invokeAsync(operation: "ice_ping",
                                  mode: .Nonmutating,
                                  context: context,
                                  sentOn: sentOn,
                                  sentFlags: sentFlags,
                                  sent: sent)
    }

    /// Tests whether this object supports a specific Slice interface.
    ///
    /// - parameter id: `String` - The type ID of the Slice interface to test against.
    ///
    /// - parameter context: `Ice.Context` - The optional context dictionary for the invocation.
    ///
    /// - returns: `Bool` - True if the target object has the interface specified by id or derives
    ///   from the interface specified by id.
    func ice_isA(id: String, context: Context? = nil) throws -> Bool {
        return try _impl._invoke(operation: "ice_isA",
                                 mode: .Nonmutating,
                                 write: { ostr in
                                     ostr.write(id)
                                 },
                                 read: { istr in try istr.read() as Bool },
                                 context: context)
    }

    /// Tests whether this object supports a specific Slice interface.
    ///
    /// - parameter id: `String` - The type ID of the Slice interface to test against.
    ///
    /// - parameter context: `Ice.Context` - The optional context dictionary for the invocation.
    ///
    /// - parameter sentOn: `Dispatch.DispatchQueue` - Optional dispatch queue used to
    ///   dispatch sent callback, the default is to use `PromiseKit.conf.Q.return` queue.
    ///
    /// - parameter sentFlags: `Dispatch.DispatchWorkItemFlags` - Optional dispatch flags used to
    ///   dispatch sent callback
    ///
    /// - parameter sent: `((Bool) -> Void)` - Optional sent callback.
    ///
    /// - returns: `PromiseKit.Promise<Bool>` - A promise object that will be resolved with
    ///   the return values of invocation.
    func ice_isAAsync(id: String, context: Context? = nil,
                      sentOn: DispatchQueue? = PromiseKit.conf.Q.return,
                      sentFlags: DispatchWorkItemFlags? = nil,
                      sent: ((Bool) -> Void)? = nil) -> Promise<Bool> {
        return _impl._invokeAsync(operation: "ice_isA",
                                  mode: .Nonmutating,
                                  write: { ostr in
                                      ostr.write(id)
                                  },
                                  read: { istr in try istr.read() as Bool },
                                  context: context,
                                  sentOn: sentOn,
                                  sentFlags: sentFlags,
                                  sent: sent)
    }

    /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
    ///
    /// - parameter context: `Ice.Context?` - The optional context dictionary for the invocation.
    ///
    /// - returns: `String` - The Slice type ID of the most-derived interface.
    func ice_id(context: Context? = nil) throws -> String {
        return try _impl._invoke(operation: "ice_id",
                                 mode: .Nonmutating,
                                 read: { istr in try istr.read() as String },
                                 context: context)
    }

    /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
    ///
    /// - parameter context: `Ice.Context?` - The optional context dictionary for the invocation.
    ///
    /// - parameter sentOn: `Dispatch.DispatchQueue` - Optional dispatch queue used to
    ///   dispatch sent callback, the default is to use `PromiseKit.conf.Q.return` queue.
    ///
    /// - parameter sentFlags: `Dispatch.DispatchWorkItemFlags` - Optional dispatch flags used to
    ///   dispatch sent callback
    ///
    /// - parameter sent: `((Bool) -> Void)` - Optional sent callback.
    ///
    /// - returns: `PromiseKit.Promise<String>` A promise object that will be resolved with
    ///   the return values of invocation.
    func ice_idAsync(context: Context? = nil,
                     sentOn: DispatchQueue? = PromiseKit.conf.Q.return,
                     sentFlags: DispatchWorkItemFlags? = nil,
                     sent: ((Bool) -> Void)? = nil) -> Promise<String> {
        return _impl._invokeAsync(operation: "ice_id",
                                  mode: .Nonmutating,
                                  read: { istr in try istr.read() as String },
                                  context: context,
                                  sentOn: sentOn,
                                  sentFlags: sentFlags,
                                  sent: sent)
    }

    /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
    ///
    /// - parameter context: `Ice.Context?` - The optional context dictionary for the invocation.
    ///
    /// - returns: `Ice.StringSeq` - The Slice type IDs of the interfaces supported by the target object,
    ///   in base-to-derived order. The first element of the returned array is always `::Ice::Object`.
    func ice_ids(context: Context? = nil) throws -> StringSeq {
        return try _impl._invoke(operation: "ice_ids",
                                 mode: .Nonmutating,
                                 read: { istr in try istr.read() as StringSeq },
                                 context: context)
    }

    /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
    ///
    /// - parameter context: `Ice.Context?` - The optional context dictionary for the invocation.
    ///
    /// - parameter sentOn: `Dispatch.DispatchQueue` - Optional dispatch queue used to
    ///   dispatch sent callback, the default is to use `PromiseKit.conf.Q.return` queue.
    ///
    /// - parameter sentFlags: `Dispatch.DispatchWorkItemFlags` - Optional dispatch flags used to
    ///   dispatch sent callback
    ///
    /// - parameter sent: `((Bool) -> Void)` - Optional sent callback.
    ///
    /// - returns: `PromiseKit.Promise<Ice.StringSeq>` - A promise object that will be resolved with
    ///   the return values of invocation.
    func ice_idsAsync(context: Context? = nil,
                      sentOn: DispatchQueue? = PromiseKit.conf.Q.return,
                      sentFlags: DispatchWorkItemFlags? = nil,
                      sent: ((Bool) -> Void)? = nil) -> Promise<StringSeq> {
        return _impl._invokeAsync(operation: "ice_ids",
                                  mode: .Nonmutating,
                                  read: { istr in try istr.read() as StringSeq },
                                  context: context,
                                  sentOn: sentOn,
                                  sentFlags: sentFlags,
                                  sent: sent)
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
    /// - returns: A tuple with the following fields:
    ///
    ///   - ok: `Bool` - If the operation completed successfully, the value
    ///     is set to true. If the operation raises a user exception, the return value
    ///     is false; in this case, outEncaps contains the encoded user exception. If
    ///     the operation raises a run-time exception, it throws it directly.
    ///
    ///   - outEncaps: `Data` - The encoded out-paramaters and return value for the operation.
    ///     The return value follows any out-parameters.
    func ice_invoke(operation: String,
                    mode: OperationMode,
                    inEncaps: Data,
                    context: Context? = nil) throws -> (ok: Bool, outEncaps: Data) {
        if _impl.isTwoway {
            var data: Data?
            var ok: Bool = false
            try _impl.handle.invoke(operation, mode: mode.rawValue, inParams: inEncaps, context: context) {
                                        ok = $0
                                        data = Data($1) // make a copy
                                    }
            return (ok, data!)
        } else {
            try _impl.handle.onewayInvoke(operation, mode: mode.rawValue, inParams: inEncaps, context: context)
            return (true, Data())
        }
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
    /// - parameter sentOn: `Dispatch.DispatchQueue` - Optional dispatch queue used to
    ///   dispatch sent callback, the default is to use `PromiseKit.conf.Q.return` queue.
    ///
    /// - parameter sentFlags: `Dispatch.DispatchWorkItemFlags` - Optional dispatch flags used to
    ///   dispatch sent callback.
    ///
    /// - parameter sent: `((Bool) -> Void)` - Optional sent callback.
    ///
    /// - returns: `PromiseKit.Promise<(ok: Bool, outEncaps: Data)>` - A promise object that will be
    ////  resolved with the return values of the invocation.
    func ice_invokeAsync(operation: String,
                         mode: OperationMode,
                         inEncaps: Data,
                         context: Context? = nil,
                         sentOn: DispatchQueue? = nil,
                         sentFlags: DispatchWorkItemFlags? = nil,
                         sent: ((Bool) -> Void)? = nil) -> Promise<(ok: Bool, outEncaps: Data)> {
        if _impl.isTwoway {
            return Promise<(ok: Bool, outEncaps: Data)> { seal in
                _impl.handle.invokeAsync(operation,
                                         mode: mode.rawValue,
                                         inParams: inEncaps,
                                         context: context,
                                         response: { ok, encaps in
                                             do {
                                                 let istr =
                                                     InputStream(communicator: self._impl.communicator,
                                                                 encoding: self._impl.encoding,
                                                                 bytes: Data(encaps)) // make a copy
                                                 seal.fulfill((ok, try istr.readEncapsulation().bytes))
                                             } catch {
                                                 seal.reject(error)
                                             }
                                         },
                                         exception: { error in
                                             seal.reject(error)
                                         },
                                         sent: createSentCallback(sentOn: sentOn,
                                                                  sentFlags: sentFlags,
                                                                  sent: sent))
            }
        } else {
            let sentCB = createSentCallback(sentOn: sentOn, sentFlags: sentFlags, sent: sent)
            return Promise<(ok: Bool, outEncaps: Data)> { seal in
                _impl.handle.invokeAsync(operation,
                                         mode: mode.rawValue,
                                         inParams: inEncaps,
                                         context: context,
                                         response: { _, _ in
                                             precondition(false)
                                         },
                                         exception: { error in
                                             seal.reject(error)
                                         },
                                         sent: {
                                             seal.fulfill((true, Data()))
                                             if let sentCB = sentCB {
                                                 sentCB($0)
                                             }
                })
            }
        }
    }

    /// Returns the connection for this proxy. If the proxy does not yet have an established connection,
    /// it first attempts to create a connection.
    ///
    /// - returns: `Ice.Connection?` - The Connection for this proxy.
    ///
    /// - throws: `Ice.CollocationOptimizationException` - If the proxy uses collocation optimization and denotes a
    ///   collocated object.
    func ice_getConnection() throws -> Connection? {
        return try autoreleasepool {
            //
            // Returns Any which is either NSNull or ICEConnection
            //
            guard let handle = try _impl.handle.ice_getConnection() as? ICEConnection else {
                return nil
            }
            return handle.getSwiftObject(ConnectionI.self) { ConnectionI(handle: handle) }
        }
    }

    /// Returns the connection for this proxy. If the proxy does not yet have an established connection,
    /// it first attempts to create a connection.
    ///
    /// - returns: `PromiseKit.Promise<Ice.Connection?>` - A promise object that will be resolved with
    ///   the return values of invocation.
    ///
    /// - throws: `Ice.CollocationOptimizationException` - If the proxy uses collocation optimization and denotes a
    ///   collocated object.
    func ice_getConnectionAsync() -> Promise<Connection?> {
        return Promise<Connection?> { seal in
            self._impl.handle.ice_getConnectionAsync({ conn in
                seal.fulfill(conn?.getSwiftObject(ConnectionI.self) {
                    ConnectionI(handle: conn!)
                })
            }, exception: { ex in seal.reject(ex) })
        }
    }

    /// Flushes any pending batched requests for this communicator. The call blocks until the flush is complete.
    func ice_flushBatchRequests() throws {
        return try autoreleasepool {
            try _impl.handle.ice_flushBatchRequests()
        }
    }

    /// Asynchronously flushes any pending batched requests for this proxy.
    ///
    /// - parameter sentOn: `Dispatch.DispatchQueue` - Optional dispatch queue used to
    ///   dispatch sent callback, the default is to use `PromiseKit.conf.Q.return` queue.
    ///
    /// - parameter sentFlags: `Dispatch.DispatchWorkItemFlags` Optional dispatch flags used to
    ///   dispatch sent callback.
    ///
    /// - parameter sent: `((Bool) -> Void)` - Optional sent callback.
    ///
    /// - returns: `PromiseKit.Promise<Void> - A promise object that will be resolved when
    ///   the flush is complete.
    func ice_flushBatchRequestsAsync(sentOn: DispatchQueue? = PromiseKit.conf.Q.return,
                                     sentFlags: DispatchWorkItemFlags? = nil.self,
                                     sent: ((Bool) -> Void)? = nil) -> Promise<Void> {
        let sentCB = createSentCallback(sentOn: sentOn, sentFlags: sentFlags, sent: sent)
        return Promise<Void> { seal in
            _impl.handle.ice_flushBatchRequestsAsync(
                exception: {
                    seal.reject($0)
                },
                sent: {
                    seal.fulfill(())
                    if let sentCB = sentCB {
                        sentCB($0)
                    }
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
    internal let handle: ICEObjectPrx
    internal let communicator: Communicator
    internal let encoding: EncodingVersion
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

    internal func fromICEObjectPrx<ObjectPrxType>(_ h: ICEObjectPrx) -> ObjectPrxType where ObjectPrxType: ObjectPrxI {
        return ObjectPrxType(handle: h, communicator: communicator)
    }

    internal func fromICEObjectPrx(_ h: ICEObjectPrx) -> Self {
        return type(of: self).init(handle: h, communicator: communicator)
    }

    internal static func fromICEObjectPrx(handle: ICEObjectPrx,
                                          communicator c: Communicator? = nil) -> Self {
        let communicator = c ?? handle.ice_getCommunicator().getCachedSwiftObject(CommunicatorI.self)
        return self.init(handle: handle, communicator: communicator)
    }

    public func ice_getCommunicator() -> Communicator {
        return communicator
    }

    open class func ice_staticId() -> String {
        return "::Ice::Object"
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
                try fromICEObjectPrx(handle.ice_identity(id.name, category: id.category))
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
                try fromICEObjectPrx(handle.ice_adapterId(id))
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
                try fromICEObjectPrx(handle.ice_endpoints(endpoints.toObjc()))
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
                try fromICEObjectPrx(handle.ice_locatorCacheTimeout(timeout))
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
                try fromICEObjectPrx(handle.ice_invocationTimeout(timeout))
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
                try fromICEObjectPrx(handle.ice_connectionId(id))
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
                try fromICEObjectPrx(handle.ice_connectionCached(cached))
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
                try fromICEObjectPrx(handle.ice_endpointSelection(type.rawValue))
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
                try fromICEObjectPrx(handle.ice_preferSecure(preferSecure))
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

    public func ice_getTimeout() -> Int32? {
        guard let timeout = handle.ice_getTimeout() as? Int32? else {
            preconditionFailure("Int32? type was expected")
        }
        return timeout
    }

    public func ice_timeout(_ timeout: Int32) -> Self {
        precondition(!ice_isFixed(), "Cannot create a fixed proxy with a connection timeout")
        precondition(timeout > 0 || timeout == -1, "Invalid connection timeout value")
        do {
            return try autoreleasepool {
                try fromICEObjectPrx(handle.ice_timeout(timeout))
            }
        } catch {
            fatalError("\(error)")
        }
    }

    public func ice_fixed(_ connection: Connection) -> Self {
        do {
            return try autoreleasepool {
                try fromICEObjectPrx(handle.ice_fixed((connection as! ConnectionI).handle))
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
        handle.ice_write(os, encodingMajor: os.currentEncoding.major, encodingMinor: os.currentEncoding.minor)
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
                try fromICEObjectPrx(handle.ice_collocationOptimized(collocated))
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
        var bytesRead: Int = 0
        let encoding = istr.currentEncoding
        let communicator = istr.communicator

        //
        // Returns Any which is either NSNull or ICEObjectPrx
        //
        let handleOpt = try ICEObjectPrx.ice_read(istr.data[istr.pos ..< istr.data.count],
                                                  communicator: (communicator as! CommunicatorI).handle,
                                                  encodingMajor: encoding.major,
                                                  encodingMinor: encoding.minor,
                                                  bytesRead: &bytesRead) as? ICEObjectPrx

        // Since the proxy was read in C++ we need to skip over the bytes which were read
        // We avoid using a defer statment for this since you can not throw from one
        try istr.skip(bytesRead)

        guard let handle = handleOpt else {
            return nil
        }

        return self.init(handle: handle, communicator: communicator)
    }

    public func _invoke(operation: String,
                        mode: OperationMode,
                        format: FormatType = FormatType.DefaultFormat,
                        write: ((OutputStream) -> Void)? = nil,
                        userException: ((UserException) throws -> Void)? = nil,
                        context: Context? = nil) throws {
        if userException != nil, !isTwoway {
            throw TwowayOnlyException(operation: operation)
        }

        let ostr = OutputStream(communicator: communicator, encoding: encoding)
        if let write = write {
            ostr.startEncapsulation(encoding: encoding, format: format)
            write(ostr)
            ostr.endEncapsulation()
        }

        if isTwoway {
            var uex: Error?
            try handle.invoke(operation, mode: mode.rawValue,
                              inParams: ostr.finished(), context: context,
                              response: { ok, encaps in
                                  do {
                                      let istr = InputStream(communicator: self.communicator,
                                                             encoding: self.encoding,
                                                             bytes: encaps)
                                      if ok == false {
                                          try ObjectPrxI.throwUserException(istr: istr,
                                                                            userException: userException)
                                      }
                                      try istr.skipEmptyEncapsulation()
                                  } catch {
                                      uex = error
                                  }
            })
            if let e = uex {
                throw e
            }
        } else {
            try handle.onewayInvoke(operation,
                                    mode: mode.rawValue,
                                    inParams: ostr.finished(),
                                    context: context)
        }
    }

    public func _invoke<T>(operation: String,
                           mode: OperationMode,
                           format: FormatType = FormatType.DefaultFormat,
                           write: ((OutputStream) -> Void)? = nil,
                           read: @escaping (InputStream) throws -> T,
                           userException: ((UserException) throws -> Void)? = nil,
                           context: Context? = nil) throws -> T {
        if !isTwoway {
            throw TwowayOnlyException(operation: operation)
        }
        let ostr = OutputStream(communicator: communicator, encoding: encoding)
        if let write = write {
            ostr.startEncapsulation(encoding: encoding, format: format)
            write(ostr)
            ostr.endEncapsulation()
        }
        var uex: Error?
        var ret: T!
        try handle.invoke(operation,
                          mode: mode.rawValue,
                          inParams: ostr.finished(),
                          context: context,
                          response: { ok, encaps in
                              do {
                                  let istr = InputStream(communicator: self.communicator,
                                                         encoding: self.encoding,
                                                         bytes: encaps)
                                  if ok == false {
                                      try ObjectPrxI.throwUserException(istr: istr,
                                                                        userException: userException)
                                  }
                                  try istr.startEncapsulation()
                                  ret = try read(istr)
                                  try istr.endEncapsulation()
                              } catch {
                                  uex = error
                              }
        })

        if let e = uex {
            throw e
        }

        precondition(ret != nil)
        return ret
    }

    public func _invokeAsync(operation: String,
                             mode: OperationMode,
                             format: FormatType = FormatType.DefaultFormat,
                             write: ((OutputStream) -> Void)? = nil,
                             userException: ((UserException) throws -> Void)? = nil,
                             context: Context? = nil,
                             sentOn: DispatchQueue? = PromiseKit.conf.Q.return,
                             sentFlags: DispatchWorkItemFlags? = nil,
                             sent: ((Bool) -> Void)? = nil) -> Promise<Void> {
        if userException != nil, !isTwoway {
            return Promise(error: TwowayOnlyException(operation: operation))
        }
        let ostr = OutputStream(communicator: communicator, encoding: encoding)
        if let write = write {
            ostr.startEncapsulation(encoding: encoding, format: format)
            write(ostr)
            ostr.endEncapsulation()
        }
        if isTwoway {
            return Promise<Void> { seal in
                handle.invokeAsync(operation,
                                   mode: mode.rawValue,
                                   inParams: ostr.finished(),
                                   context: context,
                                   response: { ok, encaps in
                                       do {
                                           let istr = InputStream(communicator: self.communicator,
                                                                  encoding: self.encoding,
                                                                  bytes: encaps)
                                           if ok == false {
                                               try ObjectPrxI.throwUserException(istr: istr,
                                                                                 userException: userException)
                                           }
                                           try istr.skipEmptyEncapsulation()
                                           seal.fulfill(())
                                       } catch {
                                           seal.reject(error)
                                       }
                                   },
                                   exception: { error in
                                       seal.reject(error)
                                   },
                                   sent: createSentCallback(sentOn: sentOn, sentFlags: sentFlags, sent: sent))
            }
        } else {
            if ice_isBatchOneway() || ice_isBatchDatagram() {
                return Promise<Void> { seal in
                    try handle.onewayInvoke(operation,
                                            mode: mode.rawValue,
                                            inParams: ostr.finished(),
                                            context: context)

                    seal.fulfill(())
                }
            } else {
                return Promise<Void> { seal in
                    let sentCB = createSentCallback(sentOn: sentOn, sentFlags: sentFlags, sent: sent)
                    handle.invokeAsync(operation,
                                       mode: mode.rawValue,
                                       inParams: ostr.finished(),
                                       context: context,
                                       response: { _, _ in
                                           precondition(false)
                                       },
                                       exception: { error in
                                           seal.reject(error)
                                       },
                                       sent: {
                                           seal.fulfill(())
                                           if let sentCB = sentCB {
                                               sentCB($0)
                                           }
                    })
                }
            }
        }
    }

    public func _invokeAsync<T>(operation: String,
                                mode: OperationMode,
                                format: FormatType = FormatType.DefaultFormat,
                                write: ((OutputStream) -> Void)? = nil,
                                read: @escaping (InputStream) throws -> T,
                                userException: ((UserException) throws -> Void)? = nil,
                                context: Context? = nil,
                                sentOn: DispatchQueue? = PromiseKit.conf.Q.return,
                                sentFlags: DispatchWorkItemFlags? = nil,
                                sent: ((Bool) -> Void)? = nil) -> Promise<T> {
        if !isTwoway {
            return Promise(error: TwowayOnlyException(operation: operation))
        }
        let ostr = OutputStream(communicator: communicator, encoding: encoding)
        if let write = write {
            ostr.startEncapsulation(encoding: encoding, format: format)
            write(ostr)
            ostr.endEncapsulation()
        }
        return Promise<T> { seal in
            handle.invokeAsync(operation,
                               mode: mode.rawValue,
                               inParams: ostr.finished(),
                               context: context,
                               response: { ok, encaps in
                                   do {
                                       let istr = InputStream(communicator: self.communicator,
                                                              encoding: self.encoding,
                                                              bytes: encaps)
                                       if ok == false {
                                           try ObjectPrxI.throwUserException(istr: istr,
                                                                             userException: userException)
                                       }
                                       try istr.startEncapsulation()
                                       let l = try read(istr)
                                       try istr.endEncapsulation()
                                       seal.fulfill(l)
                                   } catch {
                                       seal.reject(error)
                                   }
                               },
                               exception: { error in
                                   seal.reject(error)
                               },
                               sent: createSentCallback(sentOn: sentOn, sentFlags: sentFlags, sent: sent))
        }
    }

    private static func throwUserException(istr: InputStream, userException: ((UserException) throws -> Void)?) throws {
        do {
            try istr.startEncapsulation()
            try istr.throwException()
        } catch let error as UserException {
            try istr.endEncapsulation()
            if let userException = userException {
                try userException(error)
            }
            throw UnknownUserException(unknown: error.ice_id())
        }
        precondition(false)
    }

    public static func checkedCast<ProxyImpl>(prx: ObjectPrx,
                                              facet: String? = nil,
                                              context: Context? = nil) throws -> ProxyImpl?
        where ProxyImpl: ObjectPrxI {
        do {
            let objPrx = facet != nil ? prx.ice_facet(facet!) : prx

            // checkedCast always calls ice_isA - no optimization on purpose
            guard try objPrx.ice_isA(id: ProxyImpl.ice_staticId(), context: context) else {
                return nil
            }
            return ProxyImpl(from: objPrx)
        } catch is FacetNotExistException {
            return nil
        }
    }

    public static func uncheckedCast<ProxyImpl>(prx: ObjectPrx,
                                                facet: String? = nil) -> ProxyImpl where ProxyImpl: ObjectPrxI {
        if let f = facet {
            return ProxyImpl(from: prx.ice_facet(f))
        } else if let optimized = prx as? ProxyImpl {
            return optimized
        } else {
            return ProxyImpl(from: prx)
        }
    }
}
