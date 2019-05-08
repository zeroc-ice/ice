//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc
import PromiseKit

public extension Connection {
    func flushBatchRequestsAsync(_ compress: CompressBatch,
                                 sent: ((Bool) -> Void)? = nil,
                                 sentOn: DispatchQueue? = PromiseKit.conf.Q.return,
                                 sentFlags: DispatchWorkItemFlags? = nil) -> Promise<Void> {
        let impl = self as! ConnectionI
        let sentCB = createSentCallback(sent: sent, sentOn: sentOn, sentFlags: sentFlags)
        return Promise<Void> { seal in
            try autoreleasepool {
                try impl.handle.flushBatchRequestsAsync(compress.rawValue,
                                                        exception: { error in seal.reject(error) },
                                                        sent: {
                                                            seal.fulfill(())
                                                            if let sentCB = sentCB {
                                                                sentCB($0)
                                                            }
                })
            }
        }
    }

    func heartbeatAsync(sent: ((Bool) -> Void)? = nil,
                        sentOn: DispatchQueue? = PromiseKit.conf.Q.return,
                        sentFlags: DispatchWorkItemFlags? = nil) -> Promise<Void> {
        let impl = self as! ConnectionI
        return Promise<Void> { seal in
            try autoreleasepool {
                try impl.handle.heartbeatAsync(exception: { error in seal.reject(error) },
                                               sent: createSentCallback(sent: sent, sentOn: sentOn,
                                                                        sentFlags: sentFlags))
            }
        }
    }
}

class ConnectionI: LocalObject<ICEConnection>, Connection {
    var description: String {
        return toString()
    }

    func close(_ mode: ConnectionClose) {
        handle.close(mode.rawValue)
    }

    func createProxy(_ id: Identity) throws -> ObjectPrx {
        precondition(!id.name.isEmpty, "Identity cannot have an empty name")
        return try autoreleasepool {
            let handle = try self.handle.createProxy(id.name, category: id.category)
            let communicator = handle.ice_getCommunicator().getCachedSwiftObject(CommunicatorI.self)
            return _ObjectPrxI(handle: handle, communicator: communicator)
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
        return EndpointI(handle: handle.getEndpoint())
    }

    func flushBatchRequests(_ compress: CompressBatch) throws {
        return try autoreleasepool {
            try handle.flushBatchRequests(compress.rawValue)
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

    func setHeartbeatCallback(_ callback: HeartbeatCallback?) {
        guard let cb = callback else {
            handle.setHeartbeatCallback(nil)
            return
        }

        handle.setHeartbeatCallback { c in
            precondition(c.getCachedSwiftObject(ConnectionI.self) === self)
            cb(self)
        }
    }

    func heartbeat() throws {
        return try autoreleasepool {
            try handle.heartbeat()
        }
    }

    func setACM(timeout: Int32?, close: ACMClose?, heartbeat: ACMHeartbeat?) {
        precondition(timeout ?? 0 >= 0, "Invalid negative ACM timeout value")
        handle.setACM(timeout as NSNumber?,
                      close: close != nil ? close.unsafelyUnwrapped.rawValue as NSNumber : nil,
                      heartbeat: heartbeat != nil ? heartbeat.unsafelyUnwrapped.rawValue as NSNumber : nil)
    }

    func getACM() -> ACM {
        var timeout = Int32()
        var close = UInt8()
        var heartbeat = UInt8()
        handle.getACM(&timeout, close: &close, heartbeat: &heartbeat)
        return ACM(timeout: timeout, close: ACMClose(rawValue: close)!, heartbeat: ACMHeartbeat(rawValue: heartbeat)!)
    }

    func type() -> String {
        return handle.type()
    }

    func timeout() -> Int32 {
        return handle.timeout()
    }

    func toString() -> String {
        return handle.toString()
    }

    func getInfo() throws -> ConnectionInfo {
        // swiftlint:disable force_cast
        return try handle.getInfo() as! ConnectionInfo
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
