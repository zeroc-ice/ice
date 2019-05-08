//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc
import PromiseKit

class CommunicatorI: LocalObject<ICECommunicator>, Communicator {
    private let valueFactoryManager: ValueFactoryManager = ValueFactoryManagerI()
    let defaultsAndOverrides: DefaultsAndOverrides
    let initData: InitializationData
    let classGraphDepthMax: Int32

    init(handle: ICECommunicator, initData: InitializationData) {
        defaultsAndOverrides = DefaultsAndOverrides(handle: handle)
        self.initData = initData
        let num = initData.properties!.getPropertyAsIntWithDefault(key: "Ice.ClassGraphDepthMax", value: 50)
        if num < 1 || num > 0x7FFF_FFFF {
            classGraphDepthMax = 0x7FFF_FFFF
        } else {
            classGraphDepthMax = num
        }
        super.init(handle: handle)
    }

    func destroy() {
        handle.destroy()
    }

    func shutdown() {
        handle.shutdown()
    }

    func waitForShutdown() {
        handle.waitForShutdown()
    }

    func isShutdown() -> Bool {
        return handle.isShutdown()
    }

    func stringToProxy(_ str: String) throws -> ObjectPrx? {
        return try autoreleasepool {
            guard let prxHandle = try handle.stringToProxy(str: str) as? ICEObjectPrx else {
                return nil
            }
            return ObjectPrxI(handle: prxHandle, communicator: self)
        }
    }

    func proxyToString(_ obj: ObjectPrx?) -> String {
        return obj?.ice_toString() ?? ""
    }

    func propertyToProxy(_ property: String) throws -> ObjectPrx? {
        return try autoreleasepool {
            guard let handle = try handle.propertyToProxy(property: property) as? ICEObjectPrx else {
                return nil
            }
            return ObjectPrxI(handle: handle, communicator: self)
        }
    }

    func proxyToProperty(proxy: ObjectPrx, property: String) -> PropertyDict {
        precondition(!proxy.ice_isFixed(), "Cannot create property for fixed proxy")
        do {
            return try autoreleasepool {
                try handle.proxyToProperty(prx: proxy._impl.handle, property: property)
            }
        } catch is CommunicatorDestroyedException {
            return PropertyDict()
        } catch {
            fatalError("\(error)")
        }
    }

    func identityToString(_ id: Identity) -> String {
        return Ice.identityToString(id: id)
    }

    func createObjectAdapter(_ name: String) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try self.handle.createObjectAdapter(name)

            return ObjectAdapterI(handle: handle, communicator: self)
        }
    }

    func createObjectAdapterWithEndpoints(name: String, endpoints: String) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try self.handle.createObjectAdapterWithEndpoints(name: name, endpoints: endpoints)
            return ObjectAdapterI(handle: handle, communicator: self)
        }
    }

    func createObjectAdapterWithRouter(name: String, rtr: RouterPrx) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try self.handle.createObjectAdapterWithRouter(name: name, router: rtr._impl.handle)
            return ObjectAdapterI(handle: handle, communicator: self)
        }
    }

    func getImplicitContext() -> ImplicitContext {
        let handle = self.handle.getImplicitContext()
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
        guard let handle = handle.getDefaultRouter() else {
            return nil
        }
        return RouterPrxI.fromICEObjectPrx(handle: handle, communicator: self)
    }

    func setDefaultRouter(_ rtr: RouterPrx?) {
        do {
            try autoreleasepool {
                try handle.setDefaultRouter((rtr as? ObjectPrxI)?.handle)
            }
        } catch is CommunicatorDestroyedException {
            // Ignored
        } catch {
            fatalError("\(error)")
        }
    }

    func getDefaultLocator() -> LocatorPrx? {
        guard let handle = handle.getDefaultLocator() else {
            return nil
        }
        return LocatorPrxI.fromICEObjectPrx(handle: handle, communicator: self)
    }

    func setDefaultLocator(_ loc: LocatorPrx?) {
        do {
            try autoreleasepool {
                try handle.setDefaultLocator((loc as? ObjectPrxI)?.handle)
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
            try handle.flushBatchRequests(compress.rawValue)
        }
    }

    func createAdmin(adminAdapter: ObjectAdapter?, adminId: Identity) throws -> ObjectPrx {
        return try autoreleasepool {
            let handle = try self.handle.createAdmin((adminAdapter as? ObjectAdapterI)?.handle,
                                                     name: adminId.name,
                                                     category: adminId.category)
            if let adapter = adminAdapter {
                // Register the admin OA's id with the servant manager. This is used to distingish between
                // ObjectNotExistException and FacetNotExistException when a servant is not found on
                // a Swift Admin OA.
                (adapter as! ObjectAdapterI).servantManager.setAdminId(adminId)
            }

            return ObjectPrxI(handle: handle, communicator: self)
        }
    }

    func getAdmin() throws -> ObjectPrx? {
        return try autoreleasepool {
            guard let handle = try handle.getAdmin() as? ICEObjectPrx else {
                return nil
            }

            return ObjectPrxI(handle: handle, communicator: self)
        }
    }

    func addAdminFacet(servant: Object, facet: String) throws {
        try autoreleasepool {
            try handle.addAdminFacet(AdminFacetFacade(communicator: self, servant: servant), facet: facet)
        }
    }

    func removeAdminFacet(_ facet: String) throws -> Object {
        return try autoreleasepool {
            guard let facade = try handle.removeAdminFacet(facet) as? AdminFacetFacade else {
                preconditionFailure()
            }

            return facade.servant
        }
    }

    func findAdminFacet(_ facet: String) -> Object? {
        do {
            return try autoreleasepool {
                guard let facade = try handle.findAdminFacet(facet) as? AdminFacetFacade else {
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
                try handle.findAllAdminFacets().mapValues { facade in
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

    func getClientDispatchQueue() throws -> DispatchQueue {
        return try autoreleasepool {
            try handle.getClientDispatchQueue()
        }
    }

    func getServerDispatchQueue() throws -> DispatchQueue {
        return try autoreleasepool {
            try handle.getServerDispatchQueue()
        }
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
                try impl.handle.flushBatchRequestsAsync(compress.rawValue,
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

    func setSslPasswordPrompt(prompt: @escaping (() -> String)) {
        (self as! CommunicatorI).handle.setSslPasswordPrompt(prompt)
    }

    func setSslCertificateVerifier(verifier: @escaping ((SSLConnectionInfo) -> Bool)) {
        (self as! CommunicatorI).handle.setSslCertificateVerifier { info in
            verifier(info as! SSLConnectionInfo)
        }
    }

    func initializePlugins() throws {
        try (self as! CommunicatorI).handle.initializePlugins()
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
