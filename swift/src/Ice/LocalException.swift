// Copyright (c) ZeroC, Inc.

/// Base class for all Ice exceptions not defined in Slice.
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
    ///
    /// - Parameters:
    ///   - message: The exception message.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public required init(_ message: String, file: String = #fileID, line: Int32 = #line) {
        self.message = message
        self.file = file
        self.line = line
    }

    /// Returns a type ID for this exception based on its fully qualified name.
    ///
    /// - Returns: The type ID of this exception.
    public func ice_id() -> String {
        "::" + String(reflecting: type(of: self)).replacingOccurrences(of: ".", with: "::")
    }
}
