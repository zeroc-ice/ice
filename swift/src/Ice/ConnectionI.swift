// Copyright (c) ZeroC, Inc.

import IceImpl

extension Connection {
    // CustomStringConvertible implementation
    public var description: String {
        return toString()
    }
}

final class ConnectionI: LocalObject<ICEConnection>, Connection, @unchecked Sendable {

    func abort() {
        handle.abort()
    }

    func close() async throws {
        try await handle.close()
    }

    func createProxy(_ id: Identity) throws -> ObjectPrx {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        return try autoreleasepool {
            let handle = try self.handle.createProxy(id.name, category: id.category)
            let communicator = handle.ice_getCommunicator().getCachedSwiftObject(CommunicatorI.self)
            return ObjectPrxI(handle: handle, communicator: communicator)
        }
    }

    func setAdapter(_ oa: ObjectAdapter?) throws {
        try autoreleasepool {
            try handle.setAdapter((oa as? ObjectAdapterI)?.handle)
        }
    }

    func getAdapter() -> ObjectAdapter? {
        guard let handle = handle.getAdapter() else {
            return nil
        }

        return handle.getCachedSwiftObject(ObjectAdapterI.self)
    }

    func getEndpoint() -> Endpoint {
        let handle = self.handle.getEndpoint()
        return handle.getSwiftObject(EndpointI.self) {
            EndpointI(handle: handle)
        }
    }

    func flushBatchRequests(_ compress: CompressBatch) async throws {
        return try await withCheckedThrowingContinuation { continuation in
            handle.flushBatchRequests(
                compress.rawValue,
                exception: { error in continuation.resume(throwing: error) },
                sent: { _ in
                    continuation.resume(returning: ())
                })
        }
    }

    func setCloseCallback(_ callback: CloseCallback?) throws {
        return try autoreleasepool {
            guard let cb = callback else {
                try handle.setCloseCallback(nil)
                return
            }

            try handle.setCloseCallback { c in
                precondition(c.getCachedSwiftObject(ConnectionI.self) === self)
                cb(self)
            }
        }
    }

    func disableInactivityCheck() {
        handle.disableInactivityCheck()
    }

    func type() -> String {
        return handle.type()
    }

    func toString() -> String {
        return handle.toString()
    }

    func getInfo() throws -> ConnectionInfo {
        // swiftlint:disable force_cast
        return try handle.getInfo() as! ConnectionInfo
        // swiftlint:enable force_cast
    }

    func setBufferSize(rcvSize: Int32, sndSize: Int32) throws {
        return try autoreleasepool {
            try handle.setBufferSize(rcvSize, sndSize: sndSize)
        }
    }

    func throwException() throws {
        return try autoreleasepool {
            try handle.throwException()
        }
    }
}
