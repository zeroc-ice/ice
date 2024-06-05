// Copyright (c) ZeroC, Inc.

import Foundation

/// Represents the response to an incoming request. It's returned by Dispatcher.dispatch.
public final class OutgoingResponse {
    /// Gets the exception ID of the response.
    /// It's nil when replyStatus is ok. Otherwise, this ID is the Slice type ID of the exception marshaled into this
    /// response if this exception was defined in Slice or is derived from LocalException.
    /// For other exceptions, this ID is the full name of the exception's type.
    public let exceptionId: String?

    /// Gets the exception message of the response.
    /// It's nil when replyStatus is ok.
    public let exceptionMessage: String?

    /// Gets the output stream buffer of the response. This output stream should not be written to after construction.
    public let outputStream: OutputStream

    /// Gets the reply status of the response.
    public let replyStatus: ReplyStatus

    /// Constructs an OutgoingResponse object.
    /// - Parameters:
    ///   - replyStatus: The reply status.
    ///   - exceptionId: The ID of the exception, when the response carries an exception.
    ///   - exceptionMessage: The message of the exception, when the response carries an exception.
    ///   - outputStream: The output stream that holds the response.
    public init(replyStatus: ReplyStatus, exceptionId: String?, exceptionMessage: String?, outputStream: OutputStream) {
        self.replyStatus = replyStatus
        self.exceptionId = exceptionId
        self.exceptionMessage = exceptionMessage
        self.outputStream = outputStream
    }

    /// Constructs an OutgoingResponse object with the ok status.
    /// - Parameter outputStream: The output stream that holds the response.
    public convenience init(_ outputStream: OutputStream) {
        self.init(replyStatus: .ok, exceptionId: nil, exceptionMessage: nil, outputStream: outputStream)
    }
}
