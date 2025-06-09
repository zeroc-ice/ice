// Copyright (c) ZeroC, Inc.

import IceImpl
import Synchronization

final class ObjectAdapterI: LocalObject<ICEObjectAdapter>, ObjectAdapter, ICEDispatchAdapter, Hashable,
    @unchecked Sendable
{
    var dispatchPipeline: Dispatcher {
        dispatchPipelineValue.withLock {
            guard let value = $0 else {
                var value: Dispatcher = servantManager
                for factory in middlewareFactoryList.reversed() {
                    value = factory(value)
                }
                middlewareFactoryList.removeAll()  // we're done with the factories, release their memory
                $0 = value
                return value
            }
            return value
        }
    }

    private let communicator: Communicator
    private let servantManager: ServantManager
    private let dispatchPipelineValue = Mutex<Dispatcher?>(nil)
    private var middlewareFactoryList: [(Dispatcher) -> Dispatcher] = []  // not thread-safe

    init(handle: ICEObjectAdapter, communicator: Communicator) {
        self.communicator = communicator
        servantManager = ServantManager(adapterName: handle.getName(), communicator: communicator)
        super.init(handle: handle)
        handle.registerDispatchAdapter(self)
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(ObjectIdentifier(self).hashValue)
    }

    static func == (lhs: ObjectAdapterI, rhs: ObjectAdapterI) -> Bool {
        return lhs === rhs
    }

    func getName() -> String {
        return handle.getName()
    }

    func getCommunicator() -> Communicator {
        return communicator
    }

    func activate() throws {
        try autoreleasepool {
            try handle.activate()
        }
    }

    func hold() {
        handle.hold()
    }

    func waitForHold() {
        handle.waitForHold()
    }

    func deactivate() {
        handle.deactivate()
    }

    func waitForDeactivate() {
        handle.waitForDeactivate()
    }

    func isDeactivated() -> Bool {
        return handle.isDeactivated()
    }

    func destroy() {
        return handle.destroy()
    }

    @discardableResult
    func use(_ middlewareFactory: @escaping (_ next: Dispatcher) -> Dispatcher) -> Self {
        precondition(
            dispatchPipelineValue.withLock {
                $0 == nil
            },
            "All middleware must be installed before the first dispatch.")
        middlewareFactoryList.append(middlewareFactory)
        return self
    }

    func add(servant: Dispatcher, id: Identity) throws -> ObjectPrx {
        return try addFacet(servant: servant, id: id, facet: "")
    }

    func addFacet(servant: Dispatcher, id: Identity, facet: String) throws -> ObjectPrx {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        try servantManager.addServant(servant: servant, id: id, facet: facet)
        return try createProxy(id).ice_facet(facet)
    }

    func addWithUUID(_ servant: Dispatcher) throws -> ObjectPrx {
        return try addFacetWithUUID(servant: servant, facet: "")
    }

    func addFacetWithUUID(servant: Dispatcher, facet: String) throws -> ObjectPrx {
        return try addFacet(
            servant: servant, id: Identity(name: UUID().uuidString, category: ""), facet: facet)
    }

    func addDefaultServant(servant: Dispatcher, category: String) throws {
        try servantManager.addDefaultServant(servant: servant, category: category)
    }

    func remove(_ id: Identity) throws -> Dispatcher {
        return try removeFacet(id: id, facet: "")
    }

    func removeFacet(id: Identity, facet: String) throws -> Dispatcher {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        return try servantManager.removeServant(id: id, facet: facet)
    }

    func removeAllFacets(_ id: Identity) throws -> FacetMap {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        return try servantManager.removeAllFacets(id: id)
    }

    func removeDefaultServant(_ category: String) throws -> Dispatcher {
        return try servantManager.removeDefaultServant(category: category)
    }

    func find(_ id: Identity) -> Dispatcher? {
        return findFacet(id: id, facet: "")
    }

    func findFacet(id: Identity, facet: String) -> Dispatcher? {
        return servantManager.findServant(id: id, facet: facet)
    }

    func findAllFacets(_ id: Identity) -> FacetMap {
        return servantManager.findAllFacets(id: id)
    }

    func findByProxy(_ proxy: ObjectPrx) -> Dispatcher? {
        return findFacet(id: proxy.ice_getIdentity(), facet: proxy.ice_getFacet())
    }

    func addServantLocator(locator: ServantLocator, category: String) throws {
        try servantManager.addServantLocator(locator: locator, category: category)
    }

    func removeServantLocator(_ category: String) throws -> ServantLocator {
        return try servantManager.removeServantLocator(category: category)
    }

    func findServantLocator(_ category: String) -> ServantLocator? {
        return servantManager.findServantLocator(category: category)
    }

    func findDefaultServant(_ category: String) -> Dispatcher? {
        return servantManager.findDefaultServant(category: category)
    }

    func createProxy(_ id: Identity) throws -> ObjectPrx {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        return try autoreleasepool {
            try ObjectPrxI(
                handle: handle.createProxy(name: id.name, category: id.category),
                communicator: communicator)
        }
    }

    func createDirectProxy(_ id: Identity) throws -> ObjectPrx {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        return try autoreleasepool {
            try ObjectPrxI(
                handle: handle.createDirectProxy(name: id.name, category: id.category),
                communicator: communicator)
        }
    }

    func createIndirectProxy(_ id: Identity) throws -> ObjectPrx {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        return try autoreleasepool {
            try ObjectPrxI(
                handle: handle.createIndirectProxy(name: id.name, category: id.category),
                communicator: communicator)
        }
    }

    func setLocator(_ locator: LocatorPrx?) {
        let l = locator as? LocatorPrxI
        handle.setLocator(l?.handle ?? nil)
    }

    func getLocator() -> LocatorPrx? {
        guard let locatorHandle = handle.getLocator() else {
            return nil
        }
        return LocatorPrxI.fromICEObjectPrx(handle: locatorHandle)
    }

    func getEndpoints() -> EndpointSeq {
        return handle.getEndpoints().fromObjc()
    }

    func getPublishedEndpoints() -> EndpointSeq {
        return handle.getPublishedEndpoints().fromObjc()
    }

    func setPublishedEndpoints(_ newEndpoints: EndpointSeq) throws {
        try autoreleasepool {
            try handle.setPublishedEndpoints(newEndpoints.toObjc())
        }
    }

    func dispatch(
        _ adapter: ICEObjectAdapter,
        inEncapsBytes: UnsafeMutableRawPointer,
        inEncapsCount: Int,
        con: ICEConnection?,
        name: String,
        category: String,
        facet: String,
        operation: String,
        mode: UInt8,
        context: [String: String],
        requestId: Int32,
        encodingMajor: UInt8,
        encodingMinor: UInt8,
        outgoingResponseHandler: @escaping ICEOutgoingResponse
    ) {
        precondition(handle == adapter)

        let connection = con?.getSwiftObject(ConnectionI.self) { ConnectionI(handle: con!) }
        let encoding = EncodingVersion(major: encodingMajor, minor: encodingMinor)

        let current = Current(
            adapter: self,
            con: connection,
            id: Identity(name: name, category: category),
            facet: facet,
            operation: operation,
            mode: OperationMode(rawValue: mode)!,
            ctx: context,
            requestId: requestId,
            encoding: encoding)

        let istr = InputStream(
            communicator: communicator,
            encoding: encoding,
            bytes: Data(bytesNoCopy: inEncapsBytes, count: inEncapsCount, deallocator: .none))

        let request = IncomingRequest(current: current, inputStream: istr)

        Task {
            let response: OutgoingResponse

            // TODO: the request is in the Task capture and we need to send it. Is there a better syntax?
            nonisolated(unsafe) let request = request
            do {
                response = try await dispatchPipeline.dispatch(request)
            } catch {
                response = current.makeOutgoingResponse(error: error)
            }

            response.outputStream.finished().withUnsafeBytes {
                outgoingResponseHandler(
                    response.replyStatus,
                    response.exceptionId,
                    response.exceptionDetails,
                    $0.baseAddress!,
                    $0.count)
            }
        }
    }

    func complete() {
        servantManager.destroy()
    }

    func setAdminId(_ adminId: Identity) {
        servantManager.setAdminId(adminId)
    }
}
