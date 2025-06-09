// Copyright (c) ZeroC, Inc.

import IceImpl

final class AdminFacetFacade: ICEDispatchAdapter {
    private let communicator: Communicator
    let servant: Dispatcher

    init(communicator: Communicator, servant: Dispatcher) {
        self.communicator = communicator
        self.servant = servant
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
            oa.setAdminId(Identity(name: name, category: category))
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
        let servant = self.servant

        Task {
            let response: OutgoingResponse

            // TODO: the request is in the Task capture and we need to send it. Is there a better syntax?
            nonisolated(unsafe) let request = request
            do {
                response = try await servant.dispatch(request)
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

struct UnsupportedAdminFacet: Dispatcher {
    func dispatch(_ request: sending IncomingRequest) throws -> OutgoingResponse {
        throw Ice.OperationNotExistException()
    }
}

final class AdminFacetFactory: ICEAdminFacetFactory {
    static func createProcess(_ communicator: ICECommunicator, handle: ICEProcess) -> ICEDispatchAdapter {
        // We create a new ProcessI each time, which does not really matter since users are not expected
        // to compare the address of these servants.

        let c = communicator.getCachedSwiftObject(CommunicatorI.self)
        return AdminFacetFacade(
            communicator: c,
            servant: ProcessI(handle: handle)
        )
    }

    static func createProperties(_ communicator: ICECommunicator, handle: ICEPropertiesAdmin) -> ICEDispatchAdapter {
        let c = communicator.getCachedSwiftObject(CommunicatorI.self)

        // We create a new NativePropertiesAdmin each time, which does not really matter since users are not expected
        // to compare the address of these servants.

        return AdminFacetFacade(
            communicator: c,
            servant: NativePropertiesAdmin(handle: handle)
        )
    }

    static func createUnsupported(_ communicator: ICECommunicator) -> ICEDispatchAdapter {
        let c = communicator.getCachedSwiftObject(CommunicatorI.self)
        return AdminFacetFacade(
            communicator: c,
            servant: UnsupportedAdminFacet()
        )
    }
}
