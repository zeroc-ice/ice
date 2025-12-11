// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Represents the request context associated with a communicator.
/// </summary>
/// <remarks>When you make a remote invocation without an explicit request context parameter, Ice uses the per-proxy
/// request context (if any) combined with the <c>ImplicitContext</c> associated with your communicator. The property
/// <c>Ice.ImplicitContext</c> controls if your communicator has an associated implicit context, and when it does,
/// whether this implicit context is per-thread or shared by all threads.</remarks>
public interface ImplicitContext
{
    /// <summary>
    /// Gets a copy of the request context maintained by this object.
    /// </summary>
    /// <returns>A copy of the request context.</returns>
    Dictionary<string, string> getContext();

    /// <summary>
    /// Sets the request context.
    /// </summary>
    /// <param name="newContext">The new request context.</param>
    void setContext(Dictionary<string, string> newContext);

    /// <summary>
    /// Checks if the specified key has an associated value in the request context.
    /// </summary>
    /// <param name="key">The key.</param>
    /// <returns><see langword="true"/> if the key has an associated value, <see langword="false"/> otherwise.</returns>
    bool containsKey(string key);

    /// <summary>
    /// Gets the value associated with the specified key in the request context.
    /// </summary>
    /// <param name="key">The key.</param>
    /// <returns>The value associated with the key, or the empty string if no value is associated with the key.
    /// <see cref="containsKey"/> allows you to distinguish between an empty-string value and no value at all.</returns>
    string get(string key);

    /// <summary>
    /// Creates or updates a key/value entry in the request context.
    /// </summary>
    /// <param name="key">The key.</param>
    /// <param name="value">The value.</param>
    /// <returns>The previous value associated with the key, if any.</returns>
    string put(string key, string value);

    /// <summary>
    /// Removes the entry for the specified key in the request context.
    /// </summary>
    /// <param name="key">The key.</param>
    /// <returns>The value associated with the key, if any.</returns>
    string remove(string key);
}
