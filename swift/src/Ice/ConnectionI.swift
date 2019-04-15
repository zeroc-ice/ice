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
                try impl._handle.flushBatchRequestsAsync(compress.rawValue,
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
                try impl._handle.heartbeatAsync(exception: { error in seal.reject(error) },
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
        _handle.close(mode.rawValue)
    }

    func createProxy(_ id: Identity) throws -> ObjectPrx? {
        return try autoreleasepool {
            //
            // Returns Any which is either NSNull or ICEObjectPrx
            //
            guard let handle = try _handle.createProxy(id.name, category: id.category) as? ICEObjectPrx else {
                return nil
            }
            return _ObjectPrxI.fromICEObjectPrx(handle: handle)
        }
    }

    func setAdapter(_ oa: ObjectAdapter?) throws {
        try autoreleasepool {
            try _handle.setAdapter((oa as? ObjectAdapterI)?._handle)
        }
    }

    func getAdapter() -> ObjectAdapter? {
        guard let handle = _handle.getAdapter() else {
            return nil
        }

        return handle.as(type: ObjectAdapterI.self)
    }

    func getEndpoint() -> Endpoint {
        return EndpointI(handle: _handle.getEndpoint())
    }

    func flushBatchRequests(_ compress: CompressBatch) throws {
        return try autoreleasepool {
            try _handle.flushBatchRequests(compress.rawValue)
        }
    }

    func setCloseCallback(_ callback: CloseCallback?) throws {
        return try autoreleasepool {
            guard let cb = callback else {
                try _handle.setCloseCallback(nil)
                return
            }

            try _handle.setCloseCallback {
                precondition($0.as(type: ConnectionI.self) === self)
                cb(self)
            }
        }
    }

    func setHeartbeatCallback(_ callback: HeartbeatCallback?) {
        guard let cb = callback else {
            _handle.setHeartbeatCallback(nil)
            return
        }
        _handle.setHeartbeatCallback {
            precondition($0.as(type: ConnectionI.self) === self)
            cb(self)
        }
    }

    func heartbeat() throws {
        return try autoreleasepool {
            try _handle.heartbeat()
        }
    }

    func setACM(timeout: Int32?, close: ACMClose?, heartbeat: ACMHeartbeat?) {
        _handle.setACM(timeout as NSNumber?,
                       close: close != nil ? close.unsafelyUnwrapped.rawValue as NSNumber : nil,
                       heartbeat: heartbeat != nil ? heartbeat.unsafelyUnwrapped.rawValue as NSNumber : nil)
    }

    func getACM() -> ACM {
        var timeout = Int32()
        var close = UInt8()
        var heartbeat = UInt8()
        _handle.getACM(&timeout, close: &close, heartbeat: &heartbeat)
        return ACM(timeout: timeout, close: ACMClose(rawValue: close)!, heartbeat: ACMHeartbeat(rawValue: heartbeat)!)
    }

    func type() -> String {
        return _handle.type()
    }

    func timeout() -> Int32 {
        return _handle.timeout()
    }

    func toString() -> String {
        return _handle.toString()
    }

    func getInfo() throws -> ConnectionInfo {
        // swiftlint:disable force_cast
        return try _handle.getInfo() as! ConnectionInfo
    }

    func setBufferSize(rcvSize: Int32, sndSize: Int32) throws {
        return try autoreleasepool {
            try _handle.setBufferSize(rcvSize, sndSize: sndSize)
        }
    }

    func throwException() throws {
        return try autoreleasepool {
            try _handle.throwException()
        }
    }
}
