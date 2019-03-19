// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc
import PromiseKit

public protocol ObjectPrx: CustomStringConvertible, AnyObject {
    func ice_getCommunicator() -> Communicator
    func ice_getIdentity() -> Identity
    func ice_identity(_ id: Identity) throws -> Self
    func ice_getContext() -> Context
    func ice_context(_ context: Context) -> Self
    func ice_getFacet() -> String
    func ice_facet(_ facet: String) -> ObjectPrx
    func ice_getAdapterId() -> String
    func ice_adapterId(_ id: String) throws -> Self
    func ice_getEndpoints() -> [Endpoint]
    func ice_endpoints(_ endpoints: [Endpoint]) throws -> Self
    func ice_getLocatorCacheTimeout() -> Int32
    func ice_locatorCacheTimeout(_ timeout: Int32) throws -> Self
    func ice_getInvocationTimeout() -> Int32
    func ice_invocationTimeout(_ timeout: Int32) throws -> Self
    func ice_getConnectionId() -> String
    func ice_connectionId(_ id: String) throws -> Self
    func ice_isConnectionCached() -> Bool
    func ice_connectionCached(_ cached: Bool) throws -> Self
    func ice_getEndpointSelection() -> EndpointSelectionType
    func ice_endpointSelection(_ type: EndpointSelectionType) throws -> Self
    func ice_getEncodingVersion() -> EncodingVersion
    func ice_encodingVersion(_ encoding: EncodingVersion) -> Self
    func ice_getRouter() -> RouterPrx?
    func ice_router(_ router: RouterPrx?) throws -> Self
    func ice_getLocator() -> LocatorPrx?
    func ice_locator(_ locator: LocatorPrx?) throws -> Self
    func ice_isSecure() -> Bool
    func ice_secure(_ secure: Bool) -> Self
    func ice_isPreferSecure() -> Bool
    func ice_preferSecure(_ preferSecure: Bool) throws -> Self
    func ice_isTwoway() -> Bool
    func ice_twoway() -> Self
    func ice_isOneway() -> Bool
    func ice_oneway() -> Self
    func ice_isBatchOneway() -> Bool
    func ice_batchOneway() -> Self
    func ice_isDatagram() -> Bool
    func ice_datagram() -> Self
    func ice_isBatchDatagram() -> Bool
    func ice_batchDatagram() -> Self
    func ice_getCompress() -> Bool?
    func ice_compress(_ compress: Bool) -> Self
    func ice_getTimeout() -> Int32?
    func ice_timeout(_ timeout: Int32) throws -> Self
    func ice_fixed(_ connection: Connection) throws -> ObjectPrx?
    func ice_getConnection() throws -> Connection?
    func ice_getCachedConnection() -> Connection?
    func ice_flushBatchRequests() throws
    func ice_toString() -> String
    func ice_isCollocationOptimized() -> Bool
    func ice_collocationOptimized(_ collocated: Bool) throws -> ObjectPrx?

    func ice_invoke(operation: String,
                    mode: OperationMode,
                    inEncaps: [UInt8],
                    context: Context?) throws -> (Bool, [UInt8])
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
        let lhsI = lhs as! _ObjectPrxI
        let rhsI = rhs as! _ObjectPrxI
        return lhsI.handle.isEqual(rhsI.handle)
    }
}

public func proxyIdentityAndFacetLess(lhs: ObjectPrx?, rhs: ObjectPrx?) -> Bool {
    if lhs === rhs {
        return false
    } else if lhs === nil && rhs !== nil {
        return true
    } else if lhs !== nil || rhs === nil {
        return false
    } else {
        let lhsI = lhs as! _ObjectPrxI
        let rhsI = rhs as! _ObjectPrxI
        return lhsI.handle.proxyIdentityAndFacetLess(rhsI.handle)
    }
}

public func proxyIdentityAndFacetEqual(lhs: ObjectPrx?, rhs: ObjectPrx?) -> Bool {
    if lhs === rhs {
        return true
    } else if lhs === nil && rhs === nil {
        return true
    } else if lhs === nil || rhs === nil {
        return false
    } else {
        let lhsI = lhs as! _ObjectPrxI
        let rhsI = rhs as! _ObjectPrxI
        return lhsI.handle.proxyIdentityAndFacetEqual(rhsI.handle)
    }
}

public func proxyIdentityLess(lhs: ObjectPrx?, rhs: ObjectPrx?) -> Bool {
    if lhs === rhs {
        return false
    } else if lhs === nil && rhs !== nil {
        return true
    } else if lhs !== nil || rhs === nil {
        return false
    } else {
        let lhsI = lhs as! _ObjectPrxI
        let rhsI = rhs as! _ObjectPrxI
        return lhsI.handle.proxyIdentityLess(rhsI.handle)
    }
}

public func proxyIdentityEqual(lhs: ObjectPrx?, rhs: ObjectPrx?) -> Bool {
    if lhs === rhs {
        return true
    } else if lhs === nil && rhs === nil {
        return true
    } else if lhs === nil || rhs === nil {
        return false
    } else {
        let lhsI = lhs as! _ObjectPrxI
        let rhsI = rhs as! _ObjectPrxI
        return lhsI.handle.proxyIdentityEqual(rhsI.handle)
    }
}

public extension ObjectPrx {
    var impl: _ObjectPrxI {
        // swiftlint:disable force_cast
        return self as! _ObjectPrxI
    }

    func ice_ping() throws {
        _ = try impl._invoke(operation: "ice_ping",
                             mode: OperationMode.Nonmutating,
                             twowayOnly: false,
                             inParams: nil,
                             hasOutParams: false)
    }

    func ice_pingAsync() -> Promise<Void> {
        return impl._invokeAsync(operation: "ice_ping",
                                        mode: OperationMode.Nonmutating,
                                        twowayOnly: false,
                                        inParams: nil,
                                        hasOutParams: false) { _ in }
    }

    func ice_isA(id: String, context: Context? = nil) throws -> Bool {
        let impl = self as! _ObjectPrxI
        let os = impl._createOutputStream()
        os.startEncapsulation()
        os.write(id)
        os.endEncapsulation()
        let istr = try impl._invoke(operation: "ice_isA",
                                   mode: .Nonmutating,
                                   twowayOnly: true,
                                   inParams: os,
                                   hasOutParams: true,
                                   context: context)
        try istr.startEncapsulation()
        let r: Bool = try istr.read()
        try istr.endEncapsulation()
        return r
    }

    func ice_isAAsync(id: String, context: Context? = nil) -> Promise<Bool> {
        let impl = self as! _ObjectPrxI
        let os = impl._createOutputStream()
        os.startEncapsulation()
        os.write(id)
        os.endEncapsulation()
        return impl._invokeAsync(operation: "ice_isA",
                                 mode: .Nonmutating,
                                 twowayOnly: true,
                                 inParams: os,
                                 hasOutParams: true,
                                 context: context) { istr in
                                    try istr.startEncapsulation()
                                    let r: Bool = try istr.read()
                                    try istr.endEncapsulation()
                                    return r
        }
    }

    func ice_id(context: Context? = nil) throws -> String {
        let istr = try impl._invoke(operation: "ice_id",
                                   mode: .Nonmutating,
                                   twowayOnly: true,
                                   inParams: nil,
                                   hasOutParams: true,
                                   context: context)
        try istr.startEncapsulation()
        let id: String = try istr.read()
        try istr.endEncapsulation()
        return id
    }

    func ice_idAsync(context: Context? = nil) -> Promise<String> {
        return impl._invokeAsync(operation: "ice_id",
                                   mode: .Nonmutating,
                                   twowayOnly: true,
                                   inParams: nil,
                                   hasOutParams: true,
                                   context: context) { istr in
            try istr.startEncapsulation()
            let id: String = try istr.read()
            try istr.endEncapsulation()
            return id
        }
    }

    func ice_ids(context: Context? = nil) throws -> StringSeq {
        let istr = try impl._invoke(operation: "ice_ids",
                                   mode: .Nonmutating,
                                   twowayOnly: true,
                                   inParams: nil,
                                   hasOutParams: true,
                                   context: context)
        try istr.startEncapsulation()
        let id: StringSeq = try istr.read()
        try istr.endEncapsulation()
        return id
    }

    func ice_idsAsync(context: Context? = nil) -> Promise<StringSeq> {
        return impl._invokeAsync(operation: "ice_ids",
                                 mode: .Nonmutating,
                                 twowayOnly: true,
                                 inParams: nil,
                                 hasOutParams: true,
                                 context: context) { istr in
                                    try istr.startEncapsulation()
                                    let id: StringSeq = try istr.read()
                                    try istr.endEncapsulation()
                                    return id
        }
    }

    public func ice_invoke(operation: String,
                           mode: OperationMode,
                           inEncaps: [UInt8],
                           context: Context? = nil) throws -> (Bool, [UInt8]) {
        return try inEncaps.withUnsafeBufferPointer {
            var ok = Bool()
            let ins = try InputStream(communicator: impl.communicator,
                                      inputStream: impl.handle.iceInvoke(operation, mode: Int(mode.rawValue),
                                                                    inParams: $0.baseAddress,
                                                                    inSize: inEncaps.count,
                                                                    context: context,
                                                                    returnValue: &ok))
            return impl.isTwoway ? (ok, try ins.readEncapsulation().bytes) : (ok, [UInt8]())
        }
    }
}

open class _ObjectPrxI: ObjectPrx {
    let handle: ICEObjectPrx
    fileprivate let communicator: Communicator
    private let encoding: EncodingVersion
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
        let impl = prx as! _ObjectPrxI
        communicator = impl.communicator
        encoding = impl.encoding
        isTwoway = impl.isTwoway
        handle = impl.handle
    }

    internal func fromICEObjectPrx<ObjectPrxType>(_ h: ICEObjectPrx) -> ObjectPrxType where ObjectPrxType: _ObjectPrxI {
        return ObjectPrxType(handle: h,
                             communicator: communicator)
    }

    internal func fromICEObjectPrx(_ h: ICEObjectPrx) -> Self {
        return type(of: self).init(handle: h, communicator: communicator)
    }

    internal static func fromICEObjectPrx(handle: ICEObjectPrx,
                                          communicator c: Communicator? = nil) -> Self {
        let communicator = c ?? handle.ice_getCommunicator().to(type: CommunicatorI.self)
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

    public func ice_identity(_ id: Identity) throws -> Self {
        return try autoreleasepool {
            try fromICEObjectPrx(handle.ice_identity(id.name, category: id.category))
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

    public func ice_adapterId(_ id: String) throws -> Self {
        return try autoreleasepool {
            try fromICEObjectPrx(handle.ice_adapterId(id))
        }
    }

    public func ice_getEndpoints() -> [Endpoint] {
        return handle.ice_getEndpoints().map { objcEndpt in
            EndpointI(handle: objcEndpt)
        }
    }

    public func ice_endpoints(_ endpoints: [Endpoint]) throws -> Self {
        return try autoreleasepool {
            try fromICEObjectPrx(handle.ice_endpoints(endpoints.map { ($0 as! EndpointI)._handle }))
        }
    }

    public func ice_getLocatorCacheTimeout() -> Int32 {
        return handle.ice_getLocatorCacheTimeout()
    }

    public func ice_locatorCacheTimeout(_ timeout: Int32) throws -> Self {
        return try autoreleasepool {
            try fromICEObjectPrx(handle.ice_locatorCacheTimeout(timeout))
        }
    }

    public func ice_getInvocationTimeout() -> Int32 {
        return handle.ice_getInvocationTimeout()
    }

    public func ice_invocationTimeout(_ timeout: Int32) throws -> Self {
        return try autoreleasepool {
            try fromICEObjectPrx(handle.ice_invocationTimeout(timeout))
        }
    }

    public func ice_getConnectionId() -> String {
        return handle.ice_getConnectionId()
    }

    public func ice_connectionId(_ id: String) throws -> Self {
        return try autoreleasepool {
            try fromICEObjectPrx(handle.ice_connectionId(id))
        }
    }

    public func ice_isConnectionCached() -> Bool {
        return handle.ice_isConnectionCached()
    }

    public func ice_connectionCached(_ cached: Bool) throws -> Self {
        return try autoreleasepool {
            try fromICEObjectPrx(handle.ice_connectionCached(cached))
        }
    }

    public func ice_getEndpointSelection() -> EndpointSelectionType {
        return EndpointSelectionType(rawValue: handle.ice_getEndpointSelection())!
    }

    public func ice_endpointSelection(_ type: EndpointSelectionType) throws -> Self {
        return try autoreleasepool {
            try fromICEObjectPrx(handle.ice_endpointSelection(type.rawValue))
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
        return fromICEObjectPrx(routerHandle) as _RouterPrxI
    }

    public func ice_router(_ router: RouterPrx?) throws -> Self {
        return try autoreleasepool {
            let r = router as? _RouterPrxI
            return try fromICEObjectPrx(handle.ice_router(r?.handle ?? nil))
        }
    }

    public func ice_getLocator() -> LocatorPrx? {
        guard let locatorHandle = handle.ice_getLocator() else {
            return nil
        }
        return fromICEObjectPrx(locatorHandle) as _LocatorPrxI
    }

    public func ice_locator(_ locator: LocatorPrx?) throws -> Self {
        return try autoreleasepool {
            let l = locator as? _LocatorPrxI
            return try fromICEObjectPrx(handle.ice_locator(l?.handle ?? nil))
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

    public func ice_preferSecure(_ preferSecure: Bool) throws -> Self {
        return try autoreleasepool {
            try fromICEObjectPrx(handle.ice_preferSecure(preferSecure))
        }
    }

    public func ice_isTwoway() -> Bool {
        return handle.ice_isTwoway()
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

    public func ice_timeout(_ timeout: Int32) throws -> Self {
        return try fromICEObjectPrx(handle.ice_timeout(timeout))
    }

    public func ice_fixed(_ connection: Connection) throws -> ObjectPrx? {
        return try autoreleasepool {
            try fromICEObjectPrx(handle.ice_fixed((connection as! ConnectionI)._handle)) as _ObjectPrxI
        }
    }

    public func ice_getConnection() throws -> Connection? {
        return try autoreleasepool {
            //
            // Returns Any which is either NSNull or ICEConnection
            //
            guard let handle = try handle.ice_getConnection() as? ICEConnection else {
                return nil
            }
            return handle.assign(to: ConnectionI.self) { ConnectionI(handle: handle) }
        }
    }

    public func ice_getCachedConnection() -> Connection? {
        guard let handle = handle.ice_getCachedConnection() else {
            return nil
        }
        return handle.assign(to: ConnectionI.self) { ConnectionI(handle: handle) }
    }

    public func ice_flushBatchRequests() throws {
        return try autoreleasepool {
            try handle.ice_flushBatchRequests()
        }
    }

    public func ice_write(to os: OutputStream) {
        handle.iceWrite(os)
    }

    public func ice_toString() -> String {
        return handle.ice_toString()
    }

    public func ice_isCollocationOptimized() -> Bool {
        return handle.ice_isCollocationOptimized()
    }

    public func ice_collocationOptimized(_ collocated: Bool) throws -> ObjectPrx? {
        return try fromICEObjectPrx(handle.ice_collocationOptimized(collocated))
    }

    public static func ice_read(from istr: InputStream) throws -> Self? {
        //
        // Unmarshaling of proxies is done in C++. Since we don't know how big this proxy will
        // be we pass the current buffer position and remaining buffer capacity.
        //

        // The number of bytes consumed reading the proxy
        var bytesRead: Int = 0

        let buf = istr.getBuffer()
        let encoding = istr.getEncoding()
        let communicator = istr.getCommunicator()

        //
        // Returns Any which is either NSNull or ICEObjectPrx
        //
        let handleOpt = try ICEObjectPrx.iceRead(buf.baseAddress!.advanced(by: buf.position()),
                                                    size: buf.capacity - buf.position(),
                                                    communicator: (communicator as! CommunicatorI)._handle,
                                                    encodingMajor: encoding.major,
                                                    encodingMinor: encoding.minor,
                                                    bytesRead: &bytesRead) as? ICEObjectPrx

        // Since the proxy was read in C++ we need to skip over the bytes which were read
        // We avoid using a defer statment for this since you can not throw from one
        try buf.skip(bytesRead)

        guard let handle = handleOpt else {
            return nil
        }

        return self.init(handle: handle, communicator: communicator)
    }

    public func _createOutputStream() -> OutputStream {
        return OutputStream(communicator: communicator, encoding: encoding)
    }

    public func _invoke(operation op: String,
                        mode: OperationMode,
                        twowayOnly: Bool,
                        inParams: OutputStream?,
                        hasOutParams: Bool,
                        exceptions: [UserException.Type] = [],
                        context: Context? = nil) throws -> InputStream {
        return try autoreleasepool {
            if twowayOnly, !self.isTwoway {
                throw TwowayOnlyException(operation: op)
            }

            var ok = Bool()
            let istrHandle = try handle.iceInvoke(op, mode: Int(mode.rawValue),
                                              inParams: inParams?.getConstBytes(),
                                              inSize: inParams?.getCount() ?? 0,
                                              context: context,
                                              returnValue: &ok)

            let istr = InputStream(communicator: self.communicator, inputStream: istrHandle)
            if self.isTwoway {
                guard ok else {
                    throw try _ObjectPrxI.unmarshalUserException(stream: istr, exceptions: exceptions)
                }
                if !hasOutParams {
                    try istr.skipEmptyEncapsulation()
                }
            }
            return istr
        }
    }

    public func _invokeAsync<T>(operation op: String,
                                mode: OperationMode,
                                twowayOnly: Bool,
                                inParams: OutputStream?,
                                hasOutParams: Bool,
                                exceptions: [UserException.Type] = [],
                                context: Context? = nil,
                                sent sendCallback: ((Bool) -> Void)? = nil,
                                sentOn: DispatchQueue? = PromiseKit.conf.Q.map,
                                unmarshalResult: @escaping ((InputStream) throws -> T)) -> Promise<T> {

        if twowayOnly, !self.isTwoway {
            return Promise(error: TwowayOnlyException(operation: op))
        }

        return Promise<T> { seal in
            //
            // Response callback
            //
            func response(ok: Bool, inputStream istrHandle: ICEInputStream) {
                do {
                    let istr = InputStream(communicator: self.communicator, inputStream: istrHandle)
                    if self.isTwoway {
                        guard ok else {
                            throw try _ObjectPrxI.unmarshalUserException(stream: istr, exceptions: exceptions)
                        }
                        if !hasOutParams {
                            try istr.skipEmptyEncapsulation()
                        }
                    }
                    try seal.fulfill(unmarshalResult(istr))
                } catch let error {
                    seal.reject(error)
                }
            }

            //
            // Exception callback
            //
            func exception(error: Error) {
                seal.reject(error)
            }

            //
            // Sent callback (optional)
            //
            var sent: ((Bool) -> Void)?

            if let s = sendCallback {
                sent = { (sentSynchronously: Bool) -> Void in
                    //
                    // Use PromiseKit's map queue if not nil, otherwise use call with the current thread
                    //
                    if let queue = PromiseKit.conf.Q.map {
                        queue.async {
                            s(sentSynchronously)
                        }
                    }
                    s(sentSynchronously)
                }
            }

            do {
                try autoreleasepool {
                    try handle.iceInvokeAsync(op, mode: Int(mode.rawValue),
                                              inParams: inParams?.getBytes(),
                                              inSize: inParams?.getCount() ?? 0,
                                              context: context,
                                              response: response, exception: exception, sent: sent)
                }
            } catch let error {
                seal.reject(error)
            }
        }
    }

    static func unmarshalUserException(stream istr: InputStream,
                                       exceptions: [UserException.Type]) throws -> Error {
        do {
            try istr.startEncapsulation()
            try istr.throwException()
        } catch let error as UserException {
            try istr.endEncapsulation()
            for exceptionType in exceptions {
                if exceptionType.isBase(of: type(of: error)) {
                    return error
                }
            }
            return UnknownUserException(unknown: error.ice_id())
        }
        return UnknownUserException(unknown: "")
    }

    public static func checkedCast<ProxyImpl>(prx: ObjectPrx,
                                              facet: String? = nil,
                                              context: Context? = nil) throws -> ProxyImpl?
        where ProxyImpl: _ObjectPrxI {
        let objPrx = facet != nil ? prx.ice_facet(facet!) : prx
        guard try objPrx.ice_isA(id: ProxyImpl.ice_staticId(), context: context) else {
            return nil
        }
        return ProxyImpl(from: objPrx)
    }

    public static func uncheckedCast<ProxyImpl>(prx: ObjectPrx,
                                                facet: String? = nil) -> ProxyImpl where ProxyImpl: _ObjectPrxI {
        let objPrx = facet != nil ? prx.ice_facet(facet!) : prx
        return ProxyImpl(from: objPrx)
    }
}
