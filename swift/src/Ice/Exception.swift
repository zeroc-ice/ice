//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/// Base protocol for Ice exceptions.
public protocol Exception: Error {
    /// Returns the type id of this exception.
    ///
    /// - returns: `String` - The type id of this exception.
    func ice_id() -> String
    static func ice_staticId() -> String
}

public extension Exception {
    func ice_id() -> String {
        return type(of: self).ice_staticId()
    }
}

/// Base class for Ice run-time exceptions.
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

    /// Returns a stringified description of this exception.
    ///
    /// - returns: `String` - The exception description.
    open func ice_print() -> String {
        return ""
    }
}

/// Base class for Ice user exceptions.
open class UserException: Exception {
    public required init() {}

    open func _iceReadImpl(from _: InputStream) throws {}
    open func _iceWriteImpl(to _: OutputStream) {}

    open func _usesClasses() -> Bool {
        return false
    }

    /// Returns the Slice type ID of the exception.
    ///
    /// - returns: `String` The Slice type ID.
    open class func ice_staticId() -> String {
        return "::Ice::UserException"
    }

    open func _iceRead(from istr: InputStream) throws {
        istr.startException()
        try _iceReadImpl(from: istr)
        try istr.endException(preserve: false)
    }

    open func _iceWrite(to ostr: OutputStream) {
        ostr.startException(data: nil)
        _iceWriteImpl(to: ostr)
        ostr.endException()
    }

    /// Returns the sliced data if the exception has a preserved-slice base class and has been sliced during
    /// un-marshaling, nil is returned otherwise.
    ///
    /// - returns: `SlicedData?` - The sliced data or nil.
    open func ice_getSlicedData() -> SlicedData? {
        return nil
    }
}

/// Error used to wrap C++ std::exception errors.
public class RuntimeError: LocalException {
    private let message: String

    override public var description: String {
        return message
    }

    open override class func ice_staticId() -> String {
        return "::Ice::RuntimeError"
    }

    public init(_ message: String) {
        self.message = message
    }
}
