// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public protocol Exception: Error {}

open class LocalException: Exception {
    let _file: String
    let _line: Int

    public init(file: String = #file, line: Int = #line) {
        _file = file
        _line = line
    }

    public func ice_file() -> String {
        return _file
    }

    public func ice_line() -> Int {
        return _line
    }
}

open class UserException: Exception {
    public required init() {}

    open func _iceReadImpl(from _: InputStream) throws {}
    open func _iceWriteImpl(to _: OutputStream) {}

    open func _usesClasses() -> Bool {
        return false
    }

    open func ice_id() -> String {
        return "::Ice::UserException"
    }

    open class func ice_staticId() -> String {
        return "::Ice::UserException"
    }

    open func _iceRead(from istr: InputStream) throws {
        istr.startException()
        try _iceReadImpl(from: istr)
        _ = try istr.endException(preserve: false)
    }

    open func _iceWrite(to ostr: OutputStream) {
        ostr.startException(data: nil)
        _iceWriteImpl(to: ostr)
        ostr.endException()
    }

    open func ice_getSlicedData() -> SlicedData? {
        return nil
    }
}

// Used to wrap C++ std::exception errors
public struct RuntimeError: Error, CustomStringConvertible {
    let message: String

    public var description: String {
        return message
    }

    init(_ message: String) {
        self.message = message
    }
}
