//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc

class ObjectAdapterI: LocalObject<ICEObjectAdapter>, ObjectAdapter, ICEBlobjectFacade, Hashable {
    private let communicator: Communicator
    let servantManager: ServantManager

    init(handle: ICEObjectAdapter, communicator: Communicator) {
        self.communicator = communicator
        servantManager = ServantManager(adapterName: handle.getName(), communicator: communicator)
        super.init(handle: handle)

        handle.registerDefaultServant(self)
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
        return try autoreleasepool {
            return try ObjectPrxI(handle: handle.createProxy(name: id.name, category: id.category),
                                  communicator: communicator)
        }
    }

    func createDirectProxy(_ id: Identity) throws -> ObjectPrx {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        return try autoreleasepool {
            return try ObjectPrxI(handle: handle.createDirectProxy(name: id.name, category: id.category),
                                  communicator: communicator)
        }
    }

    func createIndirectProxy(_ id: Identity) throws -> ObjectPrx {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        return try autoreleasepool {
            return try ObjectPrxI(handle: handle.createIndirectProxy(name: id.name, category: id.category),
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

    func refreshPublishedEndpoints() throws {
        try autoreleasepool {
            try handle.refreshPublishedEndpoints()
        }
    }

    func getPublishedEndpoints() -> EndpointSeq {
        return handle.getPublishedEndpoints().fromObjc()
    }

    func setPublishedEndpoints(_ newEndpoints: EndpointSeq) throws {
        try autoreleasepool {
            try handle.setPublishedEndpoints(newEndpoints.toObjc())
        }
    }

    func getDispatchQueue() throws -> DispatchQueue {
        return try autoreleasepool {
            try handle.getDispatchQueue()
        }
    }

    func facadeInvoke(_ adapter: ICEObjectAdapter,
                      inEncaps: Data,
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
                      response: @escaping (Bool, Data) -> Void,
                      exception: @escaping (ICERuntimeException) -> Void) {
        precondition(handle == adapter)

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
                                                  encoding: EncodingVersion(major: encodingMajor,
                                                                            minor: encodingMinor),
                                                  bytes: inEncaps),
                                response: response,
                                exception: exception,
                                current: current)

        incoming.invoke(servantManager)
    }

    func facadeRemoved() {
        servantManager.destroy()
    }
}
