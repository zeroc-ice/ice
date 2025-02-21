// Copyright (c) ZeroC, Inc.

import IceImpl

class AdminFacetFacade: ICEDispatchAdapter {
    private let communicator: Communicator
    var dispatcher: Dispatcher

    init(communicator: Communicator, dispatcher: Dispatcher) {
        self.communicator = communicator
        self.dispatcher = dispatcher
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
        let objectAdapter = adapter.getSwiftObject(ObjectAdapterI.self) {
            let oa = ObjectAdapterI(handle: adapter, communicator: communicator)

            // Register the admin OA's id with the servant manager. This is used to distinguish between
            // ObjectNotExistException and FacetNotExistException when a servant is not found on
            // a Swift Admin OA.
            oa.servantManager.setAdminId(Identity(name: name, category: category))
            return oa
        }

        let connection = con?.getSwiftObject(ConnectionI.self) { ConnectionI(handle: con!) }
        let encoding = EncodingVersion(major: encodingMajor, minor: encodingMinor)

        let current = Current(
            adapter: objectAdapter,
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
            do {
                response = try await dispatcher.dispatch(request)
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

    func complete() {}
}

final class UnsupportedAdminFacet: LocalObject<ICEUnsupportedAdminFacet>, Object {
    func ice_id(current _: Current) -> String {
        return ObjectTraits.staticId
    }

    func ice_ids(current _: Current) -> [String] {
        return ObjectTraits.staticIds
    }

    func ice_isA(id: String, current _: Current) -> Bool {
        return id == ObjectTraits.staticId
    }

    func ice_ping(current _: Current) {}
}

class AdminFacetFactory: ICEAdminFacetFactory {
    static func createProcess(_ communicator: ICECommunicator, handle: ICEProcess)
        -> ICEDispatchAdapter
    {
        let c = communicator.getCachedSwiftObject(CommunicatorI.self)
        return AdminFacetFacade(
            communicator: c,
            dispatcher: ProcessDisp(
                handle.getSwiftObject(ProcessI.self) {
                    ProcessI(handle: handle)
                }))
    }

    static func createProperties(_ communicator: ICECommunicator, handle: ICEPropertiesAdmin)
        -> ICEDispatchAdapter
    {
        let c = communicator.getCachedSwiftObject(CommunicatorI.self)

        return AdminFacetFacade(
            communicator: c,
            dispatcher: PropertiesAdminDisp(
                handle.getSwiftObject(PropertiesAdminI.self) {
                    PropertiesAdminI(communicator: c, handle: handle)
                }))
    }

    static func createUnsupported(_ communicator: ICECommunicator, handle: ICEUnsupportedAdminFacet)
        -> ICEDispatchAdapter
    {
        let c = communicator.getCachedSwiftObject(CommunicatorI.self)
        return AdminFacetFacade(
            communicator: c,
            dispatcher: ObjectDisp(
                handle.getSwiftObject(UnsupportedAdminFacet.self) {
                    UnsupportedAdminFacet(handle: handle)
                }))
    }
}
