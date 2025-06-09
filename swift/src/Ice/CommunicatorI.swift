// Copyright (c) ZeroC, Inc.

import IceImpl

final class CommunicatorI: LocalObject<ICECommunicator>, Communicator, @unchecked Sendable {
    let defaultsAndOverrides: DefaultsAndOverrides
    let initData: InitializationData
    let classGraphDepthMax: Int32
    let traceSlicing: Bool
    let acceptClassCycles: Bool

    init(handle: ICECommunicator, initData: InitializationData) {

        self.initData = initData
        do {
            defaultsAndOverrides = try DefaultsAndOverrides(initData.properties!)
            classGraphDepthMax = try initData.properties!.getIcePropertyAsInt("Ice.ClassGraphDepthMax")
            precondition(
                classGraphDepthMax >= 1 && classGraphDepthMax <= 0x7FFF_FFFF,
                "Ice.ClassGraphDepthMax must be >= 0 and <= 0x7FFF_FFFF")
            traceSlicing = try initData.properties!.getIcePropertyAsInt("Ice.Trace.Slicing") > 0
            acceptClassCycles = try initData.properties!.getIcePropertyAsInt("Ice.AcceptClassCycles") > 0
        } catch {
            fatalError("\(error)")
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

    func shutdownCompleted() async {
        return await withCheckedContinuation { continuation in
            handle.waitForShutdownAsync {
                continuation.resume()
            }
        }
    }

    func isShutdown() -> Bool {
        return handle.isShutdown()
    }

    func stringToProxy(_ str: String) throws -> ObjectPrx? {
        try stringToProxyImpl(str)
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
            let handle = try self.handle.createObjectAdapterWithEndpoints(
                name: name, endpoints: endpoints)
            return ObjectAdapterI(handle: handle, communicator: self)
        }
    }

    func createObjectAdapterWithRouter(name: String, rtr: RouterPrx) throws -> ObjectAdapter {
        return try autoreleasepool {
            let handle = try self.handle.createObjectAdapterWithRouter(
                name: name, router: rtr._impl.handle)
            return ObjectAdapterI(handle: handle, communicator: self)
        }
    }

    func getDefaultObjectAdapter() -> ObjectAdapter? {
        guard let handle = self.handle.getDefaultObjectAdapter() else {
            return nil
        }
        return handle.getCachedSwiftObject(ObjectAdapterI.self)
    }

    func setDefaultObjectAdapter(_ adapter: ObjectAdapter?) {
        handle.setDefaultObjectAdapter((adapter as? ObjectAdapterI)?.handle)
    }

    func getImplicitContext() -> ImplicitContext? {
        guard let handle = self.handle.getImplicitContext() else {
            return nil
        }
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

    func flushBatchRequests(
        _ compress: CompressBatch
    ) async throws {
        return try await withCheckedThrowingContinuation { continuation in
            handle.flushBatchRequests(
                compress.rawValue,
                exception: { continuation.resume(throwing: $0) },
                sent: { _ in
                    continuation.resume(returning: ())
                }
            )
        }
    }

    func createAdmin(adminAdapter: ObjectAdapter?, adminId: Identity) throws -> ObjectPrx {
        return try autoreleasepool {
            let handle = try self.handle.createAdmin(
                (adminAdapter as? ObjectAdapterI)?.handle,
                name: adminId.name,
                category: adminId.category)
            if let adapter = adminAdapter {
                // Register the admin OA's id with the servant manager. This is used to distinguish between
                // ObjectNotExistException and FacetNotExistException when a servant is not found on
                // a Swift Admin OA.
                (adapter as! ObjectAdapterI).setAdminId(adminId)
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

    func addAdminFacet(servant: Dispatcher, facet: String) throws {
        try autoreleasepool {
            try handle.addAdminFacet(
                AdminFacetFacade(communicator: self, servant: servant), facet: facet)
        }
    }

    func removeAdminFacet(_ facet: String) throws -> Dispatcher {
        return try autoreleasepool {
            guard let facade = try handle.removeAdminFacet(facet) as? AdminFacetFacade else {
                preconditionFailure()
            }

            return facade.servant
        }
    }

    func findAdminFacet(_ facet: String) -> Dispatcher? {
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

    func makeProxyImpl<ProxyImpl>(_ proxyString: String) throws -> ProxyImpl
    where ProxyImpl: ObjectPrxI {
        guard let proxy: ProxyImpl = try stringToProxyImpl(proxyString) else {
            throw ParseException("invalid empty proxy string")
        }
        return proxy
    }

    private func stringToProxyImpl<ProxyImpl>(_ str: String) throws -> ProxyImpl?
    where ProxyImpl: ObjectPrxI {
        return try autoreleasepool {
            guard let prxHandle = try handle.stringToProxy(str: str) as? ICEObjectPrx else {
                return nil
            }
            return ProxyImpl(handle: prxHandle, communicator: self)
        }
    }
}

extension Communicator {
    /// Initialize the configured plug-ins. The communicator automatically initializes
    /// the plug-ins by default, but an application may need to interact directly with
    /// a plug-in prior to initialization. In this case, the application must set
    /// `Ice.InitPlugins=0` and then invoke `initializePlugins` manually. The plug-ins are
    /// initialized in the order in which they are loaded. If a plug-in raises an exception
    /// during initialization, the communicator invokes destroy on the plug-ins that have
    /// already been initialized.
    ///
    /// - throws: `InitializationException` Raised if the plug-ins have already been
    ///           initialized.
    public func initializePlugins() throws {
        try autoreleasepool {
            try (self as! CommunicatorI).handle.initializePlugins()
        }
    }
}

struct DefaultsAndOverrides {
    init(_ properties: Properties) throws {
        let defaultEncodingValue = properties.getIceProperty("Ice.Default.EncodingVersion")
        switch defaultEncodingValue {
        case "1.0":
            defaultEncoding = EncodingVersion(major: 1, minor: 0)
        case "1.1":
            defaultEncoding = EncodingVersion(major: 1, minor: 1)
        default:
            fatalError("Invalid Ice.Default.EncodingVersion value: \(defaultEncodingValue)")
        }

        let defaultFormatValue = try properties.getIcePropertyAsInt("Ice.Default.SlicedFormat")

        switch defaultFormatValue {
        case 0:
            defaultFormat = .compactFormat
        default:
            defaultFormat = .slicedFormat
        }
    }

    let defaultEncoding: EncodingVersion
    let defaultFormat: FormatType
}
