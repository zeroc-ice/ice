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

class CommunicatorI: LocalObject<ICECommunicator>, Communicator {
    let properties: Properties
    let logger: Logger
    let valueFactoryManager: ValueFactoryManager = ValueFactoryManagerI()
    let defaultsAndOverrides: DefaultsAndOverrides

    init(handle: ICECommunicator, properties: Properties, logger: Logger) {
        self.properties = properties
        self.logger = logger
        self.defaultsAndOverrides = DefaultsAndOverrides(handle: handle)
        super.init(handle: handle)
    }

    func destroy() {
        _handle.destroy()
    }

    func shutdown() {
        _handle.shutdown()
    }

    func waitForShutdown() {
        _handle.waitForShutdown()
    }

    func isShutdown() -> Bool {
        return _handle.isShutdown()
    }

    func stringToProxy(_ str: String) throws -> ObjectPrx? {
        return try autoreleasepool {
            guard let prxHandle = try _handle.string(toProxy: str) as? ICEObjectPrx else {
                return nil
            }
            return _ObjectPrxI(handle: prxHandle, communicator: self)
        }
    }

    func proxyToString(_ obj: ObjectPrx) throws -> String {
        return try _handle.proxy(toString: obj._impl.handle)
    }

    func propertyToProxy(_ property: String) throws -> ObjectPrx? {
        return try autoreleasepool {
            guard let handle = try _handle.propertyToProxy(property: property) as? ICEObjectPrx else {
                return nil
            }
            return _ObjectPrxI(handle: handle, communicator: self)
        }
    }

    func proxyToProperty(proxy: ObjectPrx, property: String) throws -> PropertyDict {
        return try _handle.proxy(toProperty: proxy._impl.handle, property: property)
    }

    func stringToIdentity(_ str: String) throws -> Identity {
        return try autoreleasepool {
            return try Ice.stringToIdentity(str)
        }
    }

    func identityToString(_ id: Identity) throws -> String {
        return try autoreleasepool {
            return try Ice.identityToString(id: id)
        }
    }

    func createObjectAdapter(_ name: String) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try _handle.createObjectAdapter(name)
            return ObjectAdapterI(handle: handle, communicator: self)
        }
    }

    func createObjectAdapterWithEndpoints(name: String, endpoints: String) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try _handle.createObjectAdapterWithEndpoints(name: name, endpoints: endpoints)
            return ObjectAdapterI(handle: handle, communicator: self)
        }
    }

    func createObjectAdapterWithRouter(name: String, rtr: RouterPrx) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try _handle.createObjectAdapterWithRouter(name: name, router: rtr._impl.handle)
            return ObjectAdapterI(handle: handle, communicator: self)
        }
    }

    func getImplicitContext() -> ImplicitContext {
        let handle = _handle.getImplicitContext()
        return handle.assign(to: ImplicitContextI.self) {
            ImplicitContextI(handle: handle)
        }
    }

    func getProperties() -> Properties {
        return properties
    }

    func getLogger() -> Logger {
        return logger
    }

    func getDefaultRouter() -> RouterPrx? {
        guard let handle = _handle.getDefaultRouter() else {
            return nil
        }
        return _RouterPrxI.fromICEObjectPrx(handle: handle, communicator: self)
    }

    func setDefaultRouter(_ rtr: RouterPrx?) throws {
        try autoreleasepool {
            try _handle.setDefaultRouter((rtr as? _RouterPrxI)?.handle)
        }
    }

    func getDefaultLocator() -> LocatorPrx? {
        guard let handle = _handle.getDefaultRouter() else {
            return nil
        }
        return _LocatorPrxI.fromICEObjectPrx(handle: handle, communicator: self)
    }

    func setDefaultLocator(_ loc: LocatorPrx?) throws {
        try autoreleasepool {
            try _handle.setDefaultRouter((loc as? _LocatorPrxI)?.handle)
        }
    }

    func getValueFactoryManager() -> ValueFactoryManager {
        return valueFactoryManager
    }

    func flushBatchRequests(_ compress: CompressBatch) throws {
        try autoreleasepool {
            try _handle.flushBatchRequests(compress.rawValue)
        }
    }

    func flushBatchRequestsAsync(_ compress: CompressBatch,
                                 sent: ((Bool) -> Void)? = nil,
                                 sentOn: DispatchQueue? = PromiseKit.conf.Q.return,
                                 sentFlags: DispatchWorkItemFlags? = nil) -> Promise<Void> {
        return Promise<Void> { seal in
            try autoreleasepool {
                try _handle.flushBatchRequestsAsync(compress.rawValue,
                                                    exception: { seal.reject($0) },
                                                    sent: createSentCallback(sent: sent,
                                                                             sentOn: sentOn,
                                                                             sentFlags: sentFlags))
            }
        }
    }

    func createAdmin(adminAdapter _: ObjectAdapter, adminId _: Identity) throws -> ObjectPrx {
        // TODO:
        preconditionFailure("Not yet implemented")
    }

    func getAdmin() throws -> ObjectPrx? {
        // TODO:
        preconditionFailure("Not yet implemented")
    }

    func addAdminFacet(servant _: Object, facet _: String) throws {
        // TODO:
        preconditionFailure("Not yet implemented")
    }

    func removeAdminFacet(_ facet: String) throws -> Object {
        // TODO:
        preconditionFailure("Not yet implemented")
    }

    func findAdminFacet(_ facet: String) throws -> Object? {
        // TODO:
        preconditionFailure("Not yet implemented")
    }

    func findAllAdminFacets() throws -> FacetMap {
        // TODO:
        preconditionFailure("Not yet implemented")
    }
}

public extension Communicator {
    func createObjectAdapter(name: String, queue: DispatchQueue) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try (self as! CommunicatorI)._handle.createObjectAdapter(name)
            return ObjectAdapterI(handle: handle, communicator: self, queue: queue)
        }
    }

    func createObjectAdapterWithEndpoints(name: String,
                                          endpoints: String, queue: DispatchQueue) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try (self as! CommunicatorI)._handle.createObjectAdapterWithEndpoints(name: name,
                                                                                               endpoints: endpoints)
            return ObjectAdapterI(handle: handle, communicator: self, queue: queue)
        }
    }

    func createObjectAdapterWithRouter(name: String, rtr: RouterPrx, queue: DispatchQueue) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try (self as! CommunicatorI)._handle.createObjectAdapterWithRouter(name: name,
                                                                                            router: rtr._impl.handle)
            return ObjectAdapterI(handle: handle, communicator: self, queue: queue)
        }
    }
}

public class DefaultsAndOverrides {

    public init(handle: ICECommunicator) {

        var defaultEncoding = EncodingVersion()
        handle.getDefaultEncoding(major: &defaultEncoding.major, minor: &defaultEncoding.minor)
        self.defaultEncoding = defaultEncoding
        self.defaultFormat = FormatType(rawValue: handle.getDefaultFormat())!
    }

    public let defaultEncoding: EncodingVersion
    public let defaultFormat: FormatType
}
