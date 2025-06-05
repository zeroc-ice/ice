// Copyright (c) ZeroC, Inc.

/// Base class for Ice local exceptions.
public class LocalException: Exception, CustomStringConvertible, @unchecked Sendable {
    /// The error message.
    public let message: String
    /// The name of the file where this exception was thrown.
    public let file: String
    /// The line number where this exception was thrown.
    public let line: Int32

    /// A textual representation of this Ice exception.
    public var description: String { "\(file):\(line) \(ice_id().dropFirst(2)) \(message)" }

    /// Creates a LocalException.
    /// - Parameters:
    ///   - message: The exception message.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public required init(_ message: String, file: String = #fileID, line: Int32 = #line) {
        self.message = message
        self.file = file
        self.line = line
    }

    /// Gets the type ID of the class, for example "::Ice::CommunicatorDestroyedException".
    /// This type ID is provided for informational purposes.
    /// - Returns: The type ID of the class.
    public func ice_id() -> String {
        "::" + String(reflecting: type(of: self)).replacingOccurrences(of: ".", with: "::")
    }
}
