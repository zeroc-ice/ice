//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc

class ObjectAdapterI: LocalObject<ICEObjectAdapter>, ObjectAdapter, ICEBlobjectFacade, Hashable {
    private let communicator: Communicator
    let servantManager: ServantManager
    private let queue: DispatchQueue
    private let dispatchSpecificKey: DispatchSpecificKey<Set<ObjectAdapterI>>

    init(handle: ICEObjectAdapter, communicator: Communicator, queue: DispatchQueue) {
        self.communicator = communicator
        servantManager = ServantManager(adapterName: handle.getName(), communicator: communicator)
        self.queue = queue
        dispatchSpecificKey = (communicator as! CommunicatorI).dispatchSpecificKey
        super.init(handle: handle)

        handle.registerDefaultServant(self)

        // Add self to the queue's dispatch specific data
        queue.async(flags: .barrier) {
            var adapters = queue.getSpecific(key: self.dispatchSpecificKey) ?? Set<ObjectAdapterI>()
            adapters.insert(self)
            queue.setSpecific(key: self.dispatchSpecificKey, value: adapters)
        }
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(ObjectIdentifier(self).hashValue)
    }

    static func == (lhs: ObjectAdapterI, rhs: ObjectAdapterI) -> Bool {
        return lhs === rhs
    }

    func getName() -> String {
        return _handle.getName()
    }

    func getCommunicator() -> Communicator {
        return communicator
    }

    func activate() throws {
        try autoreleasepool {
            try _handle.activate()
        }
    }

    func hold() {
        _handle.hold()
    }

    func waitForHold() {
        _handle.waitForHold()
    }

    func deactivate() {
        _handle.deactivate()
    }

    func waitForDeactivate() {
        _handle.waitForDeactivate()
    }

    func isDeactivated() -> Bool {
        return _handle.isDeactivated()
    }

    func destroy() {
        return _handle.destroy()
    }

    func add(servant: Object, id: Identity) throws -> ObjectPrx {
        return try addFacet(servant: servant, id: id, facet: "")
    }

    func addFacet(servant: Object, id: Identity, facet: String) throws -> ObjectPrx {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        try servantManager.addServant(servant: servant, id: id, facet: facet)
        return try createProxy(id).ice_facet(facet)
    }

    func addWithUUID(_ servant: Object) throws -> ObjectPrx {
        return try addFacetWithUUID(servant: servant, facet: "")
    }

    func addFacetWithUUID(servant: Object, facet: String) throws -> ObjectPrx {
        return try addFacet(servant: servant, id: Identity(name: UUID().uuidString, category: ""), facet: facet)
    }

    func addDefaultServant(servant: Object, category: String) throws {
        try servantManager.addDefaultServant(servant: servant, category: category)
    }

    func remove(_ id: Identity) throws -> Object {
        return try removeFacet(id: id, facet: "")
    }

    func removeFacet(id: Identity, facet: String) throws -> Object {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        return try servantManager.removeServant(id: id, facet: facet)
    }

    func removeAllFacets(_ id: Identity) throws -> FacetMap {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        return try servantManager.removeAllFacets(id: id)
    }

    func removeDefaultServant(_ category: String) throws -> Object {
        return try servantManager.removeDefaultServant(category: category)
    }

    func find(_ id: Identity) -> Object? {
        return findFacet(id: id, facet: "")
    }

    func findFacet(id: Identity, facet: String) -> Object? {
        return servantManager.findServant(id: id, facet: facet)
    }

    func findAllFacets(_ id: Identity) -> FacetMap {
        return servantManager.findAllFacets(id: id)
    }

    func findByProxy(_ proxy: ObjectPrx) -> Object? {
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

    func findDefaultServant(_ category: String) -> Object? {
        return servantManager.findDefaultServant(category: category)
    }

    func createProxy(_ id: Identity) throws -> ObjectPrx {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        return try _ObjectPrxI(handle: _handle.createProxy(name: id.name, category: id.category),
                               communicator: communicator)
    }

    func createDirectProxy(_ id: Identity) throws -> ObjectPrx {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        return try _ObjectPrxI(handle: _handle.createDirectProxy(name: id.name, category: id.category),
                               communicator: communicator)
    }

    func createIndirectProxy(_ id: Identity) throws -> ObjectPrx {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        return try _ObjectPrxI(handle: _handle.createIndirectProxy(name: id.name, category: id.category),
                               communicator: communicator)
    }

    func setLocator(_ locator: LocatorPrx?) {
        let l = locator as? _LocatorPrxI
        _handle.setLocator(l?._handle ?? nil)
    }

    func getLocator() -> LocatorPrx? {
        guard let locatorHandle = _handle.getLocator() else {
            return nil
        }
        return _LocatorPrxI.fromICEObjectPrx(handle: locatorHandle)
    }

    func getEndpoints() -> EndpointSeq {
        return _handle.getEndpoints().fromObjc()
    }

    func refreshPublishedEndpoints() throws {
        try autoreleasepool {
            try _handle.refreshPublishedEndpoints()
        }
    }

    func getPublishedEndpoints() -> EndpointSeq {
        return _handle.getPublishedEndpoints().fromObjc()
    }

    func setPublishedEndpoints(_ newEndpoints: EndpointSeq) throws {
        try _handle.setPublishedEndpoints(newEndpoints.toObjc())
    }

    func getDispatchQueue() -> DispatchQueue {
        return queue
    }

    func facadeInvoke(_ adapter: ICEObjectAdapter,
                      start: UnsafeRawPointer,
                      count: Int,
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
                      response: @escaping (Bool, UnsafeRawPointer?, Int) -> Void,
                      exception: @escaping (ICERuntimeException) -> Void) {
        precondition(_handle == adapter)

        let connection = con?.getSwiftObject(ConnectionI.self) { ConnectionI(handle: con!) } ?? nil

        let current = Current(adapter: self,
                              con: connection,
                              id: Identity(name: name, category: category),
                              facet: facet,
                              operation: operation,
                              mode: OperationMode(rawValue: mode)!,
                              ctx: context,
                              requestId: requestId,
                              encoding: EncodingVersion(major: encodingMajor, minor: encodingMinor))

        let incoming = Incoming(istr: InputStream(communicator: communicator,
                                                  start: start,
                                                  count: count,
                                                  encoding: EncodingVersion(major: encodingMajor,
                                                                            minor: encodingMinor)),
                                response: response,
                                exception: exception,
                                current: current)

        //
        // Check if we are in a collocated dispatch (con == nil) on the OA's queue by
        // checking if this object adapter is in the current execution context's dispatch speceific data.
        // If so, we use the current thread, otherwise dispatch to the OA's queue.
        //
        if con == nil, let adapters = DispatchQueue.getSpecific(key: dispatchSpecificKey), adapters.contains(self) {
            dispatchPrecondition(condition: .onQueue(queue))
            incoming.invoke(servantManager)
            return
        }
        dispatchPrecondition(condition: .notOnQueue(queue))
        queue.sync {
            incoming.invoke(servantManager)
        }
    }

    func facadeRemoved() {
        servantManager.destroy()
        queue.async(flags: .barrier) {
            guard var adapters = self.queue.getSpecific(key: self.dispatchSpecificKey) else {
                preconditionFailure("ObjectAdapter missing from dispatch specific data")
            }
            adapters.remove(self)
            self.queue.setSpecific(key: self.dispatchSpecificKey, value: adapters)
        }
    }
}
