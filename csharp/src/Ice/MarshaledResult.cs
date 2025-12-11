// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Base interface for marshaled result structures, which are generated for operations marked with
/// "marshaled-result" metadata.
/// </summary>
public interface MarshaledResult
{
    /// <summary>
    /// Gets the output stream used to marshal the results.
    /// </summary>
    OutputStream outputStream { get; }
}
