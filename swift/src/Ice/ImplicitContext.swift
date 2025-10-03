// Copyright (c) ZeroC, Inc.

import Foundation

/// An interface to associate implicit contexts with communicators. When you make a remote invocation without an
/// explicit context parameter, Ice uses the per-proxy context (if any) combined with the ImplicitContext
/// associated with the communicator.
/// Ice provides several implementations of ImplicitContext. The implementation used depends on the value
/// of the Ice.ImplicitContext property.
///
/// None (default)
/// No implicit context at all.
/// PerThread
/// The implementation maintains a context per thread.
/// Shared
/// The implementation maintains a single context shared by all threads.
///
/// ImplicitContext also provides a number of operations to create, update or retrieve an entry in the
/// underlying context without first retrieving a copy of the entire context.
public protocol ImplicitContext: AnyObject {
    /// Get a copy of the underlying context.
    ///
    /// - Returns: A copy of the underlying context.
    func getContext() -> Context

    /// Set the underlying context.
    ///
    /// - Parameter newContext: The new context.
    func setContext(_ newContext: Context)

    /// Check if this key has an associated value in the underlying context.
    ///
    /// - Parameter key: The key.
    /// - Returns: True if the key has an associated value, False otherwise.
    func containsKey(_ key: String) -> Bool

    /// Get the value associated with the given key in the underlying context. Returns an empty string if no value is
    /// associated with the key. containsKey allows you to distinguish between an empty-string value and no
    /// value at all.
    ///
    /// - Parameter key: The key.
    /// - Returns: The value associated with the key.
    func get(_ key: String) -> String

    /// Create or update a key/value entry in the underlying context.
    ///
    /// - Parameters:
    ///   - key: The key.
    ///   - value: The value.
    /// - Returns: The previous value associated with the key, if any.
    @discardableResult
    func put(key: String, value: String) -> String

    /// Remove the entry for the given key in the underlying context.
    ///
    /// - Parameter key: The key.
    /// - Returns: The value associated with the key, if any.
    func remove(_ key: String) -> String
}
