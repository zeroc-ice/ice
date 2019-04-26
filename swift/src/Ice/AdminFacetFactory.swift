//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc

class AdminFacetFacade: ICEBlobjectFacade {
    let communicator: Communicator
    let servant: Object

    init(communicator: Communicator, servant: Object) {
        self.communicator = communicator
        self.servant = servant
    }

    func facadeInvoke(_ adapter: ICEObjectAdapter, is: ICEInputStream, con: ICEConnection?,
                      name: String, category: String, facet: String, operation: String, mode: UInt8,
                      context: [String: String], requestId: Int32, encodingMajor: UInt8, encodingMinor: UInt8,
                      response: @escaping ICEBlobjectResponse,
                      exception: @escaping ICEBlobjectException) {
        let objectAdapter = adapter.fromLocalObject(to: ObjectAdapterI.self) {
            let queue = (communicator as! CommunicatorI).getDispatchQueue(adapter.getName(), forAdmin: true)
            let oa = ObjectAdapterI(handle: adapter, communicator: communicator, queue: queue)

            // Register the admin OA's id with the servant manager. This is used to distinguish between
            // ObjectNotExistException and FacetNotExistException when a servant is not found on
            // a Swift Admin OA.
            oa.servantManager.setAdminId(Identity(name: name, category: category))
            return oa
        }

        let connection = con != nil ? con!.fromLocalObject(to: ConnectionI.self) { ConnectionI(handle: con!) } : nil

        let current = Current(adapter: objectAdapter,
                              con: connection,
                              id: Identity(name: name, category: category),
                              facet: facet,
                              operation: operation,
                              mode: OperationMode(rawValue: mode)!,
                              ctx: context,
                              requestId: requestId,
                              encoding: EncodingVersion(major: encodingMajor, minor: encodingMinor))

        let incoming = Incoming(istr: InputStream(communicator: communicator, inputStream: `is`,
                                                  encoding: EncodingVersion(major: encodingMajor,
                                                                            minor: encodingMinor)),
                                response: response,
                                exception: exception,
                                current: current)

        let queue = objectAdapter.getDispatchQueue()

        //
        // Check if we are in a collocated dispatch (con == nil) on the OA's queue by
        // checking if this object adapter is in the current execution context's dispatch speceific data.
        // If so, we use the current thread, otherwise dispatch to the OA's queue.
        //
        if con == nil,
            let adapters = DispatchQueue.getSpecific(key: (communicator as! CommunicatorI).dispatchSpecificKey),
            adapters.contains(objectAdapter) {
            dispatchPrecondition(condition: .onQueue(queue))
            dispatch(incoming: incoming, current: current)
            return
        }

        dispatchPrecondition(condition: .notOnQueue(queue))
        queue.sync {
            dispatch(incoming: incoming, current: current)
        }
    }

    func dispatch(incoming: Incoming, current: Current) {
        // Dispatch directly to the servant. Do not call invoke on Incoming
        do {
            try servant.iceDispatch(incoming: incoming, current: current)
        } catch {
            incoming.exception(error)
        }
    }

    func facadeRemoved() {}
}

class UnsupportedAdminFacet: LocalObject<ICEUnsupportedAdminFacet>, Object {}

class AdminFacetFactory: ICEAdminFacetFactory {
    static func createProcess(_ communicator: ICECommunicator, handle: ICEProcess) -> ICEBlobjectFacade {
        let c = communicator.as(type: CommunicatorI.self)
        return AdminFacetFacade(communicator: c, servant: ProcessI(handle: handle))
    }

    static func createProperties(_ communicator: ICECommunicator, handle: ICEPropertiesAdmin) -> ICEBlobjectFacade {
        let c = communicator.as(type: CommunicatorI.self)
        return AdminFacetFacade(communicator: c, servant: PropertiesAdminI(communicator: c, handle: handle))
    }

    static func createUnsupported(_ communicator: ICECommunicator,
                                  handle: ICEUnsupportedAdminFacet) -> ICEBlobjectFacade {
        let c = communicator.as(type: CommunicatorI.self)
        return AdminFacetFacade(communicator: c, servant: UnsupportedAdminFacet(handle: handle))
    }
}
