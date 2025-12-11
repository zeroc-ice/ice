// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Provides information about an incoming request being dispatched.
/// </summary>
/// <param name="adapter">The object adapter that received the request.</param>
/// <param name="con">The connection that received the request. It's null for collocation-optimized dispatches.</param>
/// <param name="id">The identity of the target Ice object.</param>
/// <param name="facet">The facet of the target Ice object.</param>
/// <param name="operation">The name of the operation.</param>
/// <param name="mode">The operation mode (idempotent or not).</param>
/// <param name="ctx">The request context.</param>
/// <param name="requestId">The request ID. <c>0</c> means the request is one-way.</param>
/// <param name="encoding">The Slice encoding version used to marshal the payload of the request.</param>
public sealed record class Current(
    ObjectAdapter adapter,
    Connection? con,
    Identity id,
    string facet,
    string operation,
    OperationMode mode,
    Dictionary<string, string> ctx,
    int requestId,
    EncodingVersion encoding);
