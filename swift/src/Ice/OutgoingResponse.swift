// Copyright (c) ZeroC, Inc.

import Foundation

/// Represents the response to an incoming request. It's returned by Dispatcher.dispatch.
public struct OutgoingResponse {
    /// Gets the exception ID of the response.
    /// It's nil when replyStatus is ok or userException. Otherwise, this ID is the value returned by `ice_id` for
    /// Ice local exceptions. For other exceptions, this ID is the full name of the exception's type.
    public let exceptionId: String?

    /// Gets the full details of the exception marshaled into this response.
    /// It's nil when replyStatus is ok.
    public let exceptionDetails: String?

    /// Gets the output stream buffer of the response. This output stream should not be written to after construction.
    public let outputStream: OutputStream

    /// Gets the reply status raw value. It may not correspond to a valid `ReplyStatus` value.
    public let replyStatus: UInt8

    /// Creates an OutgoingResponse object.
    /// - Parameters:
    ///   - replyStatus: The reply status.
    ///   - exceptionId: The ID of the exception, when the response carries an exception other than a user exception.
    ///   - exceptionDetails: The full details of the exception, when the response carries an exception other than a
    ///   user exception.
    ///   - outputStream: The output stream that holds the response.
    public init(
        replyStatus: UInt8, exceptionId: String?, exceptionDetails: String?, outputStream: OutputStream
    ) {
        self.replyStatus = replyStatus
        self.exceptionId = exceptionId
        self.exceptionDetails = exceptionDetails
        self.outputStream = outputStream
    }

    /// Creates an OutgoingResponse object with the ok status.
    /// - Parameter outputStream: The output stream that holds the response.
    internal init(_ outputStream: OutputStream) {
        self.init(
            replyStatus: ReplyStatus.ok.rawValue, exceptionId: nil, exceptionDetails: nil, outputStream: outputStream)
    }
}
