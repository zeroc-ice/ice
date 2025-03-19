// Copyright (c) ZeroC, Inc.
/// Represents a request received by a connection. It's the argument given to `Dispatcher.dispatch`.
public final class IncomingRequest {
    /// Gets the current object for the request.
    public let current: Current

    /// Gets the incoming stream buffer of the request.
    public let inputStream: InputStream

    /// Constructs an incoming request.
    /// - parameter current: The current object for the request.
    /// - parameter inputStream: The input stream buffer over the incoming Ice protocol request message. The stream is
    /// positioned at the beginning of the encapsulation in the request.
    public init(current: Current, inputStream: InputStream) {
        self.current = current
        self.inputStream = inputStream
    }
}

extension IncomingRequest {
    /// Makes sure the operation mode received with the request is not idempotent.
    /// - Throws: `MarshalException` if the operation mode is idempotent or nonmutating.
    public func checkNonIdempotent() throws {
        if current.mode != .normal {  // i.e. idempotent or non-mutating
            // The caller believes the operation is idempotent or non-mutating, but the implementation (the local code)
            // doesn't. This is a problem, as the Ice runtime could retry automatically when it shouldn't.
            throw MarshalException(
                "Operation mode mismatch for operation '\(current.operation)': received \(current.mode) for non-idempotent operation"
            )
        }
    }
}
