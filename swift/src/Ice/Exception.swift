//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public protocol Exception: Error {
    func ice_id() -> String
    static func ice_staticId() -> String
}

public extension Exception {
    func ice_id() -> String {
        return type(of: self).ice_staticId()
    }
}

open class LocalException: Exception, CustomStringConvertible {
    public let file: String
    public let line: Int

    public var description: String {
        return "\(file): \(line): \(ice_id())\(ice_print())"
    }

    public init(file: String = #file, line: Int = #line) {
        self.file = file
        self.line = line
    }

    open class func ice_staticId() -> String {
        return "::Ice::LocalException"
    }

    open func ice_print() -> String {
        return ""
    }
}

open class UserException: Exception {
    public required init() {}

    open func _iceReadImpl(from _: InputStream) throws {}
    open func _iceWriteImpl(to _: OutputStream) {}

    open func _usesClasses() -> Bool {
        return false
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

    public init(_ message: String) {
        self.message = message
    }
}
