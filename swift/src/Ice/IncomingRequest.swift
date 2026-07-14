// Copyright (c) ZeroC, Inc.

/// Represents a request received by a connection. It's the argument given to ``Dispatcher/dispatch(_:)``.
public struct IncomingRequest {
    /// The Current object of the request.
    public let current: Current

    /// The input stream buffer of the request.
    public let inputStream: InputStream

    /// Constructs an incoming request.
    ///
    /// - Parameters:
    ///   - current: The Current object for the request.
    ///   - inputStream: The input stream buffer over the incoming Ice protocol request message. The stream is
    ///   positioned at the beginning of the encapsulation carrying the in-parameters.
    public init(current: Current, inputStream: InputStream) {
        self.current = current
        self.inputStream = inputStream
    }
}
