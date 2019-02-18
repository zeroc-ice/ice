// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

class ConnectionI: LocalObject<ICEConnection>, Connection {

    public var description: String {
        return toString()
    }

    public func close(mode: ConnectionClose) {
        _handle.close(mode.rawValue)
    }

    public func createProxy(id: Identity) throws -> ObjectPrx? {
        return try autoreleasepool {
            return try _handle.createProxy(id.name, category: id.category) as? ObjectPrx ?? nil
        }
    }

    public func setAdapter(adapter: ObjectAdapter?) throws {
        return autoreleasepool {
            preconditionFailure("not implemented yet")
        }
    }

    public func getAdapter() -> ObjectAdapter? {
        preconditionFailure("not implemented yet")
    }

    // TODO should this be non-optional
    public func getEndpoint() -> Endpoint? {
        return EndpointI(handle: _handle.getEndpoint())
    }

    public func flushBatchRequests(compress: CompressBatch) throws {
        return try autoreleasepool {
            try _handle.flushBatchRequests(compress.rawValue)
        }
    }

    public func setCloseCallback(callback: CloseCallback?) throws {
        return try autoreleasepool {
            guard let cb = callback else {
                try _handle.setCloseCallback(nil)
                return
            }

            try _handle.setCloseCallback {
                guard let connection = $0.to(type: ConnectionI.self) else {
                    preconditionFailure("Expected ConnectionI")
                }
                cb(connection)
            }
        }
    }

    public func setHeartbeatCallback(callback: HeartbeatCallback?) throws {
        return try autoreleasepool {
            guard let cb = callback else {
                try _handle.setHeartbeatCallback(nil)
                return
            }
            try _handle.setHeartbeatCallback {
                guard let connection = $0.to(type: ConnectionI.self) else {
                    preconditionFailure("Expected ConnectionI")
                }
                cb(connection)
            }
        }
    }

    public func heartbeat() throws {
        return try autoreleasepool {
            try _handle.heartbeat()
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

    //TODO should this be non-optional
    public func getInfo() throws -> ConnectionInfo? {
        return try _handle.getInfo() as? ConnectionInfo
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
