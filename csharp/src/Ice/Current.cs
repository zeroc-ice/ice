// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Provides information about an incoming request being dispatched.
/// </summary>
/// <param name="adapter">The object adapter that received the request.</param>
/// <param name="con">The connection that received the request. It's null when the invocation and dispatch are
/// collocated.</param>
/// <param name="id">The identity of the target Ice object.</param>
/// <param name="facet">The facet of the target Ice object.</param>
/// <param name="operation">The name of the operation.</param>
/// <param name="mode">The operation mode (idempotent or not).</param>
/// <param name="ctx">The request context.</param>
/// <param name="requestId">The request ID. 0 means the request is a one-way request.</param>
/// <param name="encoding">The encoding of the request payload.</param>
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
