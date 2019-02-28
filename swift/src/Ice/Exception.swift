// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public protocol Exception: Error {
    init()
}

public class LocalException: Exception {
    public required init() {
    }
}

public protocol UserException: Exception, CustomStringConvertible {
    var description: String { get }

    func _iceReadImpl(from: InputStream) throws
    func _iceWriteImpl(to: OutputStream)

    static func ice_staticId() -> String
}

public extension UserException {
    var description: String {
        return Self.ice_staticId()
    }

    internal func isBase(of ex: UserException) -> Bool {
        return ex is Self
    }

    func _iceRead(from ins: InputStream) throws {
        ins.startException()
        try _iceReadImpl(from: ins)
        _ = try ins.endException(preserve: false)
    }

    func _iceWrite(to os: OutputStream) {
        os.startException(data: nil)
        _iceWriteImpl(to: os)
        os.endException()
    }

}

#warning("TODO: Add proper LocalException  CustomStringConvertible impl")
// TODO: All LocalExceptions should be CustomStringConvertible and print detailed messages
// like C++ (by calling IceUtilInternal::errorToString from ObjC)
