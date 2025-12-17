// Copyright (c) ZeroC, Inc.

import Foundation

/// Represents the request context associated with a communicator. When you make a remote invocation without an
/// explicit request context parameter, Ice uses the per-proxy request context (if any) combined with the
/// `ImplicitContext` associated with your communicator.
/// The property `Ice.ImplicitContext` controls if your communicator has an associated implicit context,
/// and when it does, whether this implicit context is per-thread or shared by all threads:
/// - `None`: No implicit context at all.
/// - `PerThread`: The implementation maintains a context per thread.
/// - `Shared`: The implementation maintains a single context shared by all threads.
public protocol ImplicitContext: AnyObject {
    /// Gets a copy of the request context maintained by this object.
    ///
    /// - Returns: A copy of the request context.
    func getContext() -> Context

    /// Sets the request context.
    ///
    /// - Parameter newContext: The new request context.
    func setContext(_ newContext: Context)

    /// Checks if the specified key has an associated value in the request context.
    ///
    /// - Parameter key: The key.
    /// - Returns: `true` if the key has an associated value, `false` otherwise.
    func containsKey(_ key: String) -> Bool

    /// Gets the value associated with the specified key in the request context.
    ///
    /// - Parameter key: The key.
    /// - Returns: The value associated with the key, or the empty string if no value is associated with the key.
    ///   ``containsKey(_:)`` allows you to distinguish between an empty-string value and no value at all.
    func get(_ key: String) -> String

    /// Creates or updates a key/value entry in the request context.
    ///
    /// - Parameters:
    ///   - key: The key.
    ///   - value: The value.
    /// - Returns: The previous value associated with the key, if any.
    @discardableResult
    func put(key: String, value: String) -> String

    /// Removes the entry for the specified key in the request context.
    ///
    /// - Parameter key: The key.
    /// - Returns: The value associated with the key, if any.
    func remove(_ key: String) -> String
}
