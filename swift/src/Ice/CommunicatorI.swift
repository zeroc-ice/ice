//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc
import PromiseKit

class CommunicatorI: LocalObject<ICECommunicator>, Communicator {
    let valueFactoryManager: ValueFactoryManager = ValueFactoryManagerI()
    let defaultsAndOverrides: DefaultsAndOverrides
    var initData: InitializationData
    let serverQueue = DispatchQueue(label: "com.zeroc.ice.server", attributes: .concurrent)
    let dispatchSpecificKey = DispatchSpecificKey<Set<ObjectAdapterI>>()
    let _classGraphDepthMax: Int32
    var mutex: Mutex = Mutex()

    init(handle: ICECommunicator, initData: InitializationData) {
        defaultsAndOverrides = DefaultsAndOverrides(handle: handle)
        self.initData = initData
        let num = initData.properties!.getPropertyAsIntWithDefault(key: "Ice.ClassGraphDepthMax", value: 50)
        if num < 1 || num > 0x7fffffff {
            _classGraphDepthMax = 0x7fffffff
        } else {
            _classGraphDepthMax = num
        }
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

    func proxyToString(_ obj: ObjectPrx?) -> String {
        return obj?.ice_toString() ?? ""
    }

    func propertyToProxy(_ property: String) throws -> ObjectPrx? {
        return try autoreleasepool {
            guard let handle = try _handle.propertyToProxy(property: property) as? ICEObjectPrx else {
                return nil
            }
            return _ObjectPrxI(handle: handle, communicator: self)
        }
    }

    func proxyToProperty(proxy: ObjectPrx, property: String) -> PropertyDict {
        precondition(!proxy.ice_isFixed(), "Cannot create property for fixed proxy")
        do {
            return try autoreleasepool {
                try _handle.proxy(toProperty: proxy._impl._handle, property: property)
            }
        } catch is CommunicatorDestroyedException {
            return PropertyDict()
        } catch {
            fatalError("\(error)")
        }
    }

    func stringToIdentity(_ str: String) throws -> Identity {
        return try autoreleasepool {
            try Ice.stringToIdentity(str)
        }
    }

    func identityToString(_ id: Identity) -> String {
        return Ice.identityToString(id: id)
    }

    func createObjectAdapter(_ name: String) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try _handle.createObjectAdapter(name)
            return ObjectAdapterI(handle: handle, communicator: self, queue: serverQueue)
        }
    }

    func createObjectAdapterWithEndpoints(name: String, endpoints: String) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try _handle.createObjectAdapterWithEndpoints(name: name, endpoints: endpoints)
            return ObjectAdapterI(handle: handle, communicator: self, queue: serverQueue)
        }
    }

    func createObjectAdapterWithRouter(name: String, rtr: RouterPrx) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try _handle.createObjectAdapterWithRouter(name: name, router: rtr._impl._handle)
            return ObjectAdapterI(handle: handle, communicator: self, queue: serverQueue)
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

    func setDefaultRouter(_ rtr: RouterPrx?) {
        do {
            try autoreleasepool {
                try _handle.setDefaultRouter((rtr as? _ObjectPrxI)?._handle)
            }
        } catch is CommunicatorDestroyedException {
            // Ignored
        } catch {
            fatalError("\(error)")
        }
    }

    func getDefaultLocator() -> LocatorPrx? {
        guard let handle = _handle.getDefaultLocator() else {
            return nil
        }
        return _LocatorPrxI.fromICEObjectPrx(handle: handle, communicator: self)
    }

    func setDefaultLocator(_ loc: LocatorPrx?) {
        do {
            try autoreleasepool {
                try _handle.setDefaultLocator((loc as? _ObjectPrxI)?._handle)
            }
        } catch is CommunicatorDestroyedException {
            // Ignored
        } catch {
            fatalError("\(error)")
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

    func createAdmin(adminAdapter: ObjectAdapter?, adminId: Identity) throws -> ObjectPrx {
        return try autoreleasepool {
            let handle = try _handle.createAdmin((adminAdapter as? ObjectAdapterI)?._handle,
                                                 name: adminId.name,
                                                 category: adminId.category)
            if let adapter = adminAdapter {
                // Register the admin OA's id with the servant manager. This is used to distingish between
                // ObjectNotExistException and FacetNotExistException when a servant is not found on
                // a Swift Admin OA.
                (adapter as! ObjectAdapterI).servantManager.setAdminId(adminId)
            }

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

    func findAdminFacet(_ facet: String) -> Object? {
        do {
            return try autoreleasepool {
                guard let facade = try _handle.findAdminFacet(facet) as? AdminFacetFacade else {
                    return nil
                }
                return facade.servant
            }
        } catch is CommunicatorDestroyedException {
            // Ignored
            return nil
        } catch {
            fatalError("\(error)")
        }
    }

    func findAllAdminFacets() -> FacetMap {
        do {
            return try autoreleasepool {
                try _handle.findAllAdminFacets().mapValues { facade in
                    (facade as! AdminFacetFacade).servant
                }
            }
        } catch is CommunicatorDestroyedException {
            // Ignored
            return FacetMap()
        } catch {
            fatalError("\(error)")
        }
    }

    func getAdminDispatchQueue() -> DispatchQueue {
        return mutex.sync {
            initData.adminDispatchQueue ?? serverQueue
        }
    }

    func classGraphDepthMax() -> Int32 {
        // No mutex lock, immutable.
        return _classGraphDepthMax
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

    func flushBatchRequestsAsync(_ compress: CompressBatch,
                                 sent: ((Bool) -> Void)? = nil,
                                 sentOn: DispatchQueue? = PromiseKit.conf.Q.return,
                                 sentFlags: DispatchWorkItemFlags? = nil) -> Promise<Void> {
        let impl = self as! CommunicatorI
        let sentCB = createSentCallback(sent: sent, sentOn: sentOn, sentFlags: sentFlags)
        return Promise<Void> { seal in
            try autoreleasepool {
                try impl._handle.flushBatchRequestsAsync(compress.rawValue,
                                                         exception: { seal.reject($0) },
                                                         sent: {
                                                             seal.fulfill(())
                                                             if let sentCB = sentCB {
                                                                 sentCB($0)
                                                             }
                })
            }
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

struct DefaultsAndOverrides {
    init(handle: ICECommunicator) {
        var defaultEncoding = EncodingVersion()
        handle.getDefaultEncoding(major: &defaultEncoding.major, minor: &defaultEncoding.minor)
        self.defaultEncoding = defaultEncoding
        defaultFormat = FormatType(rawValue: handle.getDefaultFormat())!
    }

    let defaultEncoding: EncodingVersion
    let defaultFormat: FormatType
}
