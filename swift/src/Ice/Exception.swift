// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public protocol Exception: Error {
}

public class LocalException: Exception {
    let _file: String
    let _line: Int

    public init(file: String = #file, line: Int = #line) {
        self._file = file
        self._line = line
    }

    public func ice_file() -> String {
        return _file
    }

    public func ice_line() -> Int {
        return _line
    }
}

public protocol UserException: Exception, CustomStringConvertible {
    var description: String { get }

    init()

    func _iceReadImpl(from: InputStream) throws
    func _iceWriteImpl(to: OutputStream)
    func _usesClasses() -> Bool

    func ice_id() -> String
    static func ice_staticId() -> String
}

public extension UserException {
    var description: String {
        return Self.ice_staticId()
    }

    internal static func isBase(of ex: UserException.Type) -> Bool {
        return ex is Self
    }

    func _iceRead(from istr: InputStream) throws {
        istr.startException()
        try _iceReadImpl(from: istr)
        _ = try istr.endException(preserve: false)
    }

    func _iceWrite(to ostr: OutputStream) {
        ostr.startException(data: nil)
        _iceWriteImpl(to: ostr)
        ostr.endException()
    }

    func _usesClasses() -> Bool {
        return false
    }
}

// Used to wrap C++ std::exception messages
public struct RuntimeError: Error, CustomStringConvertible {
    let message: String

    public var description: String {
        return message
    }

    init(_ message: String) {
        self.message = message
    }
}

#warning("TODO: CustomStringConvertible impl")
// TODO: All LocalExceptions should be CustomStringConvertible and print detailed messages
// like C++ (by calling IceUtilInternal::errorToString from ObjC)
