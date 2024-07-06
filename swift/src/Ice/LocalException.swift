// Copyright (c) ZeroC, Inc.

/// Base class for Ice local exceptions.
public class LocalException: Exception, CustomStringConvertible {
    /// The error message.
    public let message: String
    /// The name of the file where this exception was thrown.
    public let file: String
    /// The line number where this exception was thrown.
    public let line: Int32
    /// The C++ exception description, when this exception is a C++ exception converted to Swift.
    private let cxxDescription: String?

    /// A textual representation of this Ice exception.
    public var description: String { cxxDescription ?? "\(file):\(line) \(ice_id()) \(message)" }

    /// Creates a LocalException.
    /// - Parameters:
    ///   - message: The exception message.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public init(_ message: String, file: String = #fileID, line: Int32 = #line) {
        self.message = message
        self.file = file
        self.line = line
        self.cxxDescription = nil
    }

    /// Gets the type ID of the class, for example "::Ice::CommunicatorDestroyedException".
    /// This type ID is provided for informational purposes.
    /// - Returns: The type ID of the class.
    public func ice_id() -> String {
        "::" + String(reflecting: type(of: self)).replacingOccurrences(of: ".", with: "::")
    }

    /// Creates a LocalException from an Ice C++ local exception.
    /// - Parameters:
    ///   - message: The exception message.
    ///   - cxxDescription: The C++ exception description.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    internal required init(message: String, cxxDescription: String, file: String, line: Int32) {
        self.message = message
        self.file = file
        self.line = line
        self.cxxDescription = cxxDescription
    }
}
