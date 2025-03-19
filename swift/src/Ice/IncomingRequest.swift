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
