// Copyright (c) ZeroC, Inc.

import Foundation

/// Provides information about an incoming request being dispatched.
public struct Current: Sendable {
    /// The object adapter.
    public let adapter: ObjectAdapter
    /// The connection that received the request. It's `nil` for collocation-optimized dispatches.
    public let con: Connection?
    /// The identity of the target Ice object.
    public let id: Identity
    /// The facet of the target Ice object.
    public let facet: String
    /// The name of the operation.
    public let operation: String
    /// The mode of the operation (see ``Current/checkNonIdempotent()``).
    public let mode: OperationMode
    /// The request context.
    public var ctx: Context
    /// The request ID. `0` means the request is one-way.
    public let requestId: Int32
    /// The Slice encoding version used to marshal the payload of the request.
    public let encoding: EncodingVersion

    public init(
        adapter: ObjectAdapter,
        con: Connection?,
        id: Identity,
        facet: String,
        operation: String,
        mode: OperationMode,
        ctx: Context,
        requestId: Int32,
        encoding: EncodingVersion
    ) {
        self.adapter = adapter
        self.con = con
        self.id = id
        self.facet = facet
        self.operation = operation
        self.mode = mode
        self.ctx = ctx
        self.requestId = requestId
        self.encoding = encoding
    }
}
