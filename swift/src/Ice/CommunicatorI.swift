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
    let valueFactoryManager: ValueFactoryManager = ValueFactoryManagerI()
    let defaultsAndOverrides: DefaultsAndOverrides
    var initData: InitializationData
    let serialQueue = DispatchQueue(label: "com.zeroc.ice.serial")
    let dispatchSpecificKey = DispatchSpecificKey<Set<ObjectAdapterI>>()

    var mutex: Mutex = Mutex()

    init(handle: ICECommunicator, initData: InitializationData) {
        defaultsAndOverrides = DefaultsAndOverrides(handle: handle)
        self.initData = initData
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
        return try _handle.proxy(toString: obj._impl._handle)
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
        return try _handle.proxy(toProperty: proxy._impl._handle, property: property)
    }

    func stringToIdentity(_ str: String) throws -> Identity {
        return try autoreleasepool {
            try Ice.stringToIdentity(str)
        }
    }

    func identityToString(_ id: Identity) throws -> String {
        return try autoreleasepool {
            try Ice.identityToString(id: id)
        }
    }

    func createObjectAdapter(_ name: String) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try _handle.createObjectAdapter(name)
            return ObjectAdapterI(handle: handle, communicator: self, queue: serialQueue)
        }
    }

    func createObjectAdapterWithEndpoints(name: String, endpoints: String) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try _handle.createObjectAdapterWithEndpoints(name: name, endpoints: endpoints)
            return ObjectAdapterI(handle: handle, communicator: self, queue: serialQueue)
        }
    }

    func createObjectAdapterWithRouter(name: String, rtr: RouterPrx) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try _handle.createObjectAdapterWithRouter(name: name, router: rtr._impl._handle)
            return ObjectAdapterI(handle: handle, communicator: self, queue: serialQueue)
        }
    }

    func getImplicitContext() -> ImplicitContext {
        let handle = _handle.getImplicitContext()
        return handle.fromLocalObject(to: ImplicitContextI.self) {
            ImplicitContextI(handle: handle)
        }
    }

    func getProperties() -> Properties {
        return initData.properties!
    }

    func getLogger() -> Logger {
        return initData.logger!
    }

    func getDefaultRouter() -> RouterPrx? {
        guard let handle = _handle.getDefaultRouter() else {
            return nil
        }
        return _RouterPrxI.fromICEObjectPrx(handle: handle, communicator: self)
    }

    func setDefaultRouter(_ rtr: RouterPrx?) throws {
        try autoreleasepool {
            try _handle.setDefaultRouter((rtr as? _RouterPrxI)?._handle)
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
            try _handle.setDefaultRouter((loc as? _LocatorPrxI)?._handle)
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

    func createAdmin(adminAdapter: ObjectAdapter?, adminId: Identity) throws -> ObjectPrx {
        return try autoreleasepool {
            let handle = try _handle.createAdmin((adminAdapter as? ObjectAdapterI)?._handle,
                                                 name: adminId.name,
                                                 category: adminId.category)
            // Replace the iniData.adminDispatchQueue with the dispatch queue from this adapter
            mutex.sync {
                initData.adminDispatchQueue = adminAdapter?.getDispatchQueue()
            }

            return _ObjectPrxI(handle: handle, communicator: self)
        }
    }

    func getAdmin() throws -> ObjectPrx? {
        return try autoreleasepool {
            guard let handle = try _handle.getAdmin() as? ICEObjectPrx else {
                return nil
            }

            return _ObjectPrxI(handle: handle, communicator: self)
        }
    }

    func addAdminFacet(servant: Object, facet: String) throws {
        try autoreleasepool {
            try _handle.addAdminFacet(AdminFacetFacade(communicator: self, servant: servant), facet: facet)
        }
    }

    func removeAdminFacet(_ facet: String) throws -> Object {
        return try autoreleasepool {
            guard let facade = try _handle.removeAdminFacet(facet) as? AdminFacetFacade else {
                preconditionFailure()
            }

            return facade.servant
        }
    }

    func findAdminFacet(_ facet: String) throws -> Object? {
        return try autoreleasepool {
            guard let facade = try _handle.findAdminFacet(facet) as? AdminFacetFacade else {
                return nil
            }

            return facade.servant
        }
    }

    func findAllAdminFacets() throws -> FacetMap {
        return try autoreleasepool {
            try _handle.findAllAdminFacets().mapValues { facade in
                (facade as! AdminFacetFacade).servant
            }
        }
    }

    func getAdminDispatchQueue() -> DispatchQueue {
        return mutex.sync {
            initData.adminDispatchQueue ?? serialQueue
        }
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
                                                                                            router: rtr._impl._handle)
            return ObjectAdapterI(handle: handle, communicator: self, queue: queue)
        }
    }
}

class DefaultsAndOverrides {
    init(handle: ICECommunicator) {
        var defaultEncoding = EncodingVersion()
        handle.getDefaultEncoding(major: &defaultEncoding.major, minor: &defaultEncoding.minor)
        self.defaultEncoding = defaultEncoding
        defaultFormat = FormatType(rawValue: handle.getDefaultFormat())!
    }

    let defaultEncoding: EncodingVersion
    let defaultFormat: FormatType
}
