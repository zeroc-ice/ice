// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

private enum State {
    case alive
    case dead
}

class ObjectAdapterI: LocalObject<ICEObjectAdapter>, ObjectAdapter, ICEBlobjectFacade, Hashable {
    let communicator: Communicator
    var servantManager: ServantManager
    var locator: LocatorPrx?
    var queue: DispatchQueue

    var mutex = Mutex()
    private var state: State

    init(handle: ICEObjectAdapter, communicator: Communicator, queue: DispatchQueue) {
        self.communicator = communicator
        servantManager = ServantManager(adapterName: handle.getName(), communicator: communicator)
        state = .alive
        self.queue = queue
        super.init(handle: handle)

        handle.registerDefaultServant(self)

        // Add self to the queue's dispatch specific data
        queue.async(flags: .barrier) {
            let key = (communicator as! CommunicatorI).dispatchSpecificKey
            guard var adapters = queue.getSpecific(key: key) else {
                queue.setSpecific(key: key, value: Set<ObjectAdapterI>())
                return
            }

            adapters.insert(self)
            queue.setSpecific(key: key, value: adapters)
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

    func hold() throws {
        try autoreleasepool {
            try _handle.hold()
        }
    }

    func waitForHold() throws {
        try autoreleasepool {
            try _handle.waitForHold()
        }
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
        return try mutex.sync {
            try checkForDeactivation()
            try checkIdentity(id)

            try servantManager.addServant(servant: servant, id: id, facet: facet)

            return try createProxy(id).ice_facet(facet)
        }
    }

    func addWithUUID(_ servant: Object) throws -> ObjectPrx {
        return try addFacetWithUUID(servant: servant, facet: "")
    }

    func addFacetWithUUID(servant: Object, facet: String) throws -> ObjectPrx {
        return try addFacet(servant: servant, id: Identity(name: UUID().uuidString, category: ""), facet: facet)
    }

    func addDefaultServant(servant: Object, category: String) throws {
        try mutex.sync {
            try checkForDeactivation()

            try servantManager.addDefaultServant(servant: servant, category: category)
        }
    }

    func remove(_ id: Identity) throws -> Object {
        return try removeFacet(id: id, facet: "")
    }

    func removeFacet(id: Identity, facet: String) throws -> Object {
        return try mutex.sync {
            try checkForDeactivation()
            try checkIdentity(id)

            return try servantManager.removeServant(id: id, facet: facet)
        }
    }

    func removeAllFacets(_ id: Identity) throws -> FacetMap {
        return try mutex.sync {
            try checkForDeactivation()
            try checkIdentity(id)

            return try servantManager.removeAllFacets(id: id)
        }
    }

    func removeDefaultServant(_ category: String) throws -> Object {
        return try mutex.sync {
            try checkForDeactivation()
            return try servantManager.removeDefaultServant(category: category)
        }
    }

    func find(_ id: Identity) throws -> Object? {
        return try findFacet(id: id, facet: "")
    }

    func findFacet(id: Identity, facet: String) throws -> Object? {
        return try mutex.sync {
            try checkForDeactivation()
            try checkIdentity(id)

            return servantManager.findServant(id: id, facet: facet)
        }
    }

    func findAllFacets(_ id: Identity) throws -> FacetMap {
        return try mutex.sync {
            try checkForDeactivation()
            try checkIdentity(id)

            return servantManager.findAllFacets(id: id)
        }
    }

    func findByProxy(_ proxy: ObjectPrx) throws -> Object? {
        return try mutex.sync {
            try checkForDeactivation()

            return try findFacet(id: proxy.ice_getIdentity(), facet: proxy.ice_getFacet())
        }
    }

    func addServantLocator(locator: ServantLocator, category: String) throws {
        try mutex.sync {
            try checkForDeactivation()

            try servantManager.addServantLocator(locator: locator, category: category)
        }
    }

    func removeServantLocator(_ category: String) throws -> ServantLocator {
        return try mutex.sync {
            try checkForDeactivation()

            return try servantManager.removeServantLocator(category: category)
        }
    }

    func findServantLocator(_ category: String) throws -> ServantLocator? {
        return try mutex.sync {
            try checkForDeactivation()

            return servantManager.findServantLocator(category: category)
        }
    }

    func findDefaultServant(_ category: String) throws -> Object? {
        return try mutex.sync {
            try checkForDeactivation()

            return servantManager.findDefaultServant(category: category)
        }
    }

    func createProxy(_ id: Identity) throws -> ObjectPrx {
        return try _ObjectPrxI(handle: _handle.createProxy(name: id.name, category: id.category),
                               communicator: communicator)
    }

    func createDirectProxy(_ id: Identity) throws -> ObjectPrx {
        return try _ObjectPrxI(handle: _handle.createDirectProxy(name: id.name, category: id.category),
                               communicator: communicator)
    }

    func createIndirectProxy(_ id: Identity) throws -> ObjectPrx {
        return try _ObjectPrxI(handle: _handle.createIndirectProxy(name: id.name, category: id.category),
                               communicator: communicator)
    }

    func setLocator(_ locator: LocatorPrx?) throws {
        try mutex.sync {
            try checkForDeactivation()

            self.locator = locator
        }
    }

    func getLocator() -> LocatorPrx? {
        return mutex.sync {
            locator
        }
    }

    func getEndpoints() -> EndpointSeq {
        return _handle.getEndpoints().map { objcEndpt in EndpointI(handle: objcEndpt) }
    }

    func refreshPublishedEndpoints() throws {
        try autoreleasepool {
            try _handle.refreshPublishedEndpoints()
        }
    }

    func getPublishedEndpoints() -> EndpointSeq {
        return _handle.getPublishedEndpoints().map { objcEndpt in EndpointI(handle: objcEndpt) }
    }

    func setPublishedEndpoints(_ newEndpoints: EndpointSeq) throws {
        try _handle.setPublishedEndpoints(newEndpoints.map { ($0 as! EndpointI)._handle })
    }

    func getDispatchQueue() -> DispatchQueue {
        return queue
    }

    func facadeInvoke(_ adapter: ICEObjectAdapter,
                      is istr: ICEInputStream,
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

        let current = Current(adapter: self,
                              con: con.fromLocalObject(to: ConnectionI.self) { ConnectionI(handle: con!) },
                              id: Identity(name: name, category: category),
                              facet: facet,
                              operation: operation,
                              mode: OperationMode(rawValue: mode)!,
                              ctx: context,
                              requestId: requestId,
                              encoding: EncodingVersion(major: encodingMajor, minor: encodingMinor))

        let incoming = Incoming(istr: InputStream(communicator: communicator, inputStream: istr),
                                response: response,
                                exception: exception,
                                current: current)

        //
        // Check if we are in a collocated dispatch (con == nil) on the OA's queue by
        // checking if this object adapter is in the current execution context's dispatch speceific data.
        // If so, we use the current thread, otherwise dispatch to the OA's queue.
        //
        if con == nil {
            let key = (communicator as! CommunicatorI).dispatchSpecificKey
            if let adapters = DispatchQueue.getSpecific(key: key), adapters.contains(self) {
                dispatchPrecondition(condition: .onQueue(queue))
                incoming.invoke(servantManager)
                return
            }
        }
        dispatchPrecondition(condition: .notOnQueue(queue))
        queue.sync {
            incoming.invoke(servantManager)
        }
    }

    func facadeRemoved() {
        mutex.sync {
            self.state = .dead
            servantManager.destroy()
            locator = nil
            queue.async(flags: .barrier) {
                let key = (self.communicator as! CommunicatorI).dispatchSpecificKey
                guard var adapters = self.queue.getSpecific(key: key) else {
                    preconditionFailure("ObjectAdapter missing from dispatch specific data")
                }
                adapters.remove(self)
            }
        }
    }

    private func checkForDeactivation() throws {
        guard state == .alive else {
            throw ObjectAdapterDeactivatedException(name: getName())
        }
    }

    private func checkIdentity(_ id: Identity) throws {
        guard !id.name.isEmpty else {
            throw IllegalIdentityException(id: id)
        }
    }
}
