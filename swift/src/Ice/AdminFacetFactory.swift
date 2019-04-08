// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
                      response: @escaping (Bool, UnsafeRawPointer?, Int) -> Void,
                      exception: @escaping (ICERuntimeException) -> Void) {

        let objectAdapter = adapter.fromLocalObject(to: ObjectAdapterI.self) {
            ObjectAdapterI(handle: adapter, communicator: communicator, queue: communicator.getAdminDispatchQueue())
        }

        let connection = con.fromLocalObject(to: ConnectionI.self) { ConnectionI(handle: con!) }

        let current = Current(adapter: objectAdapter,
                              con: connection,
                              id: Identity(name: name, category: category),
                              facet: facet,
                              operation: operation,
                              mode: OperationMode(rawValue: mode)!,
                              ctx: context,
                              requestId: requestId,
                              encoding: EncodingVersion(major: encodingMajor, minor: encodingMinor))

        current.adapter!.getDispatchQueue().sync {
            let istr = InputStream(communicator: communicator, inputStream: `is`)
            let inc = Incoming(istr: istr, response: response, exception: exception, current: current)
            // Dispatch directly to the servant. Do not call invoke on Incoming
            do {
                try servant.iceDispatch(incoming: inc, current: current)
            } catch let err {
                exception(inc.convertException(err))
            }
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
