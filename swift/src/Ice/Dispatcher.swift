// Copyright (c) ZeroC, Inc.

/// A dispatcher accepts incoming requests and returns outgoing responses.
public protocol Dispatcher: Sendable {
    /// Dispatches an incoming request and returns the corresponding outgoing response.
    /// - Parameter request: The incoming request.
    /// - Returns: The outgoing response.
    func dispatch(_ request: sending IncomingRequest) async throws -> OutgoingResponse
}

@available(*, deprecated, renamed: "Dispatcher")
public typealias Disp = Dispatcher
