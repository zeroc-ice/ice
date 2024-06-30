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
