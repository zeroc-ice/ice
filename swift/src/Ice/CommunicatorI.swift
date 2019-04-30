//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc
import PromiseKit

class CommunicatorI: LocalObject<ICECommunicator>, Communicator {
    private let valueFactoryManager: ValueFactoryManager = ValueFactoryManagerI()
    let defaultsAndOverrides: DefaultsAndOverrides
    let initData: InitializationData
    private let serverQueue = DispatchQueue(label: "com.zeroc.ice.server", attributes: .concurrent)
    let dispatchSpecificKey = DispatchSpecificKey<Set<ObjectAdapterI>>()
    let classGraphDepthMax: Int32

    private var mutex: Mutex = Mutex()
    private var adminDispatchQueue: Dispatch.DispatchQueue

    init(handle: ICECommunicator, initData: InitializationData) {
        defaultsAndOverrides = DefaultsAndOverrides(handle: handle)
        self.initData = initData
        adminDispatchQueue = serverQueue
        let num = initData.properties!.getPropertyAsIntWithDefault(key: "Ice.ClassGraphDepthMax", value: 50)
        if num < 1 || num > 0x7FFF_FFFF {
            classGraphDepthMax = 0x7FFF_FFFF
        } else {
            classGraphDepthMax = num
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

            return ObjectAdapterI(handle: handle, communicator: self, queue: getDispatchQueue(name))
        }
    }

    func createObjectAdapterWithEndpoints(name: String, endpoints: String) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try _handle.createObjectAdapterWithEndpoints(name: name, endpoints: endpoints)
            return ObjectAdapterI(handle: handle, communicator: self, queue: getDispatchQueue(name))
        }
    }

    func createObjectAdapterWithRouter(name: String, rtr: RouterPrx) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try _handle.createObjectAdapterWithRouter(name: name, router: rtr._impl._handle)
            return ObjectAdapterI(handle: handle, communicator: self, queue: getDispatchQueue(name))
        }
    }

    func getImplicitContext() -> ImplicitContext {
        let handle = _handle.getImplicitContext()
        return handle.getSwiftObject(ImplicitContextI.self) {
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

                mutex.sync {
                    adminDispatchQueue = adapter.getDispatchQueue()
                }
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
            adminDispatchQueue
        }
    }

    func getDispatchQueue(_ name: String, forAdmin: Bool = false) -> Dispatch.DispatchQueue {
        if !name.isEmpty {
            if initData.properties!.getPropertyAsInt(name + ".ThreadPool.Size") > 0 ||
                initData.properties!.getPropertyAsInt(name + ".ThreadPool.SizeMax") > 0 ||
                !initData.properties!.getProperty(name + ".ThreadPool.ThreadPriority").isEmpty {
                // This OA has its own thread pool
                let queue = Dispatch.DispatchQueue(label: "com.zeroc.ice.oa." + name, attributes: .concurrent)
                if forAdmin {
                    mutex.sync {
                        adminDispatchQueue = queue
                    }
                }
                return queue
            }
        }
        return serverQueue
    }
}

public extension Communicator {
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
