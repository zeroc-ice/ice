// Copyright (c) ZeroC, Inc.

/// Base protocol for Ice exceptions.
public protocol Exception: Error {
    /// Gets the type ID of this Ice exception.
    /// - Returns: The type ID of this Ice exception.
    func ice_id() -> String

    /// Gets the type ID of the class.
    /// - Returns: The type ID of the class.
    static func ice_staticId() -> String
}

/// Base class for Ice local exceptions.
open class LocalException: Exception, CustomStringConvertible {
    public let message: String
    public let file: String
    public let line: Int32
    private let cxxDescription: String?

    /// A textual representation of this Ice exception.
    public var description: String {
        cxxDescription ?? "\(file): \(line) \(ice_id()) \(message)"
    }

    /// Creates a LocalException.
    /// - Parameters:
    ///   - message: The exception message.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public init(_ message: String, file: String = #file, line: Int32 = #line) {
        self.message = message
        self.file = file
        self.line = line
        self.cxxDescription = nil
    }

    /// Creates a LocalException from an Ice C++ local exception.
    /// - Parameters:
    ///   - message: The exception message.
    ///   - cxxDescription: The C++ exception description.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public required init(message: String, cxxDescription: String, file: String, line: Int32) {
        self.message = message
        self.file = file
        self.line = line
        self.cxxDescription = cxxDescription
    }

    public func ice_id() -> String {
        return type(of: self).ice_staticId()
    }

    public class func ice_staticId() -> String {
        return "::Ice::LocalException"
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

    open class func ice_staticId() -> String {
        return "::Ice::UserException"
    }

    open func _iceRead(from istr: InputStream) throws {
        istr.startException()
        try _iceReadImpl(from: istr)
        try istr.endException()
    }

    open func _iceWrite(to ostr: OutputStream) {
        ostr.startException()
        _iceWriteImpl(to: ostr)
        ostr.endException()
    }
}

extension UserException {
    public func ice_id() -> String {
        return type(of: self).ice_staticId()
    }
}

/// Represents a C++ local exception or a std::exception without its own corresponding Swift class.
internal final class CxxLocalException: LocalException {
    private let typeId: String

    internal init(typeId: String, message: String, cxxDescription: String, file: String, line: Int32) {
        self.typeId = typeId
        super.init(message: message, cxxDescription: cxxDescription, file: file, line: line)
    }

    internal required init(message: String, cxxDescription: String, file: String, line: Int32) {
        fatalError("CxxLocalException must be initialized with a typeId")
    }

    override public func ice_id() -> String {
        typeId
    }
}
