// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc
import PromiseKit

class ConnectionI: LocalObject<ICEConnection>, Connection {
    public var description: String {
        return toString()
    }

    public func close(_ mode: ConnectionClose) {
        _handle.close(mode.rawValue)
    }

    public func createProxy(_ id: Identity) throws -> ObjectPrx? {
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

    public func setAdapter(_ oa: ObjectAdapter?) throws {
        try autoreleasepool {
            try _handle.setAdapter((oa as! ObjectAdapterI)._handle)
        }
    }

    public func getAdapter() -> ObjectAdapter? {
        guard let handle = _handle.getAdapter() else {
            return nil
        }

        return handle.assign(to: ObjectAdapterI.self) {
            ObjectAdapterI(handle: handle)
        }
    }

    public func getEndpoint() -> Endpoint {
        return EndpointI(handle: _handle.getEndpoint())
    }

    public func flushBatchRequests(_ compress: CompressBatch) throws {
        return try autoreleasepool {
            try _handle.flushBatchRequests(compress.rawValue)
        }
    }

    public func flushBatchRequestsAsync(_ compress: CompressBatch,
                                        sent: ((Bool) -> Void)? = nil,
                                        sentOn: DispatchQueue? = PromiseKit.conf.Q.return,
                                        sentFlags: DispatchWorkItemFlags? = nil) -> Promise<Void> {
        return Promise<Void> { seal in
            try autoreleasepool {
                try _handle.flushBatchRequestsAsync(compress.rawValue,
                                                    exception: { error in seal.reject(error) },
                                                    sent: createSentCallback(sent: sent,
                                                                             sentOn: sentOn,
                                                                             sentFlags: sentFlags))
            }
        }
    }

    public func setCloseCallback(_ callback: CloseCallback?) throws {
        return try autoreleasepool {
            guard let cb = callback else {
                try _handle.setCloseCallback(nil)
                return
            }

            try _handle.setCloseCallback {
                let connection = $0.to(type: ConnectionI.self)
                cb(connection)
            }
        }
    }

    public func setHeartbeatCallback(_ callback: HeartbeatCallback?) throws {
        return try autoreleasepool {
            guard let cb = callback else {
                try _handle.setHeartbeatCallback(nil)
                return
            }
            try _handle.setHeartbeatCallback {
                let connection = $0.to(type: ConnectionI.self)
                cb(connection)
            }
        }
    }

    public func heartbeat() throws {
        return try autoreleasepool {
            try _handle.heartbeat()
        }
    }

    func heartbeatAsync(sent: ((Bool) -> Void)? = nil,
                        sentOn: DispatchQueue? = PromiseKit.conf.Q.return,
                        sentFlags: DispatchWorkItemFlags? = nil) -> Promise<Void> {
        return Promise<Void> { seal in
            try autoreleasepool {
                try _handle.heartbeatAsync(exception: { error in seal.reject(error) },
                                           sent: createSentCallback(sent: sent, sentOn: sentOn, sentFlags: sentFlags))
            }
        }
    }

    public func setACM(timeout: Int32?, close: ACMClose?, heartbeat: ACMHeartbeat?) throws {
        return try autoreleasepool {
            try _handle.setACM(timeout as Any, close: close as Any, heartbeat: heartbeat as Any)
        }
    }

    public func getACM() -> ACM {
        var timeout = Int32()
        var close = UInt8()
        var heartbeat = UInt8()
        _handle.getACM(&timeout, close: &close, heartbeat: &heartbeat)
        return ACM(timeout: timeout, close: ACMClose(rawValue: close)!, heartbeat: ACMHeartbeat(rawValue: heartbeat)!)
    }

    public func type() -> String {
        return _handle.type()
    }

    public func timeout() -> Int32 {
        return _handle.timeout()
    }

    public func toString() -> String {
        return _handle.toString()
    }

    public func getInfo() throws -> ConnectionInfo {
        // swiftlint:disable force_cast
        return try _handle.getInfo() as! ConnectionInfo
    }

    public func setBufferSize(rcvSize: Int32, sndSize: Int32) throws {
        return try autoreleasepool {
            try _handle.setBufferSize(rcvSize, sndSize: sndSize)
        }
    }

    public func throwException() throws {
        return try autoreleasepool {
            try _handle.throwException()
        }
    }
}
