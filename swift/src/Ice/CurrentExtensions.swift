// Copyright (c) ZeroC, Inc.

import Foundation

extension Current {
    /// Creates an outgoing response with reply status `ok`.
    /// - Parameters:
    ///   - result: The result to marshal into the response payload.
    ///   - formatType: The class format.
    ///   - marshal: The action that marshals result into an output stream.
    /// - Returns: The outgoing response.
    public func makeOutgoingResponse<T>(
        _ result: T, formatType: FormatType?, marshal: (OutputStream, T) -> Void
    )
        -> OutgoingResponse
    {
        precondition(requestId != 0, "A one-way request cannot return a response")
        let ostr = startReplyStream()
        ostr.startEncapsulation(encoding: encoding, format: formatType)
        marshal(ostr, result)
        ostr.endEncapsulation()
        return OutgoingResponse(ostr)
    }

    /// Creates an empty outgoing response with reply status `ok`.
    /// - Returns: The outgoing response.
    public func makeEmptyOutgoingResponse() -> OutgoingResponse {
        let ostr = startReplyStream()
        if requestId != 0 {
            ostr.writeEmptyEncapsulation(encoding)
        }
        return OutgoingResponse(ostr)
    }

    /// Creates an outgoing response that marshals an exception.
    /// - Parameter error: The exception to marshal into the response payload.
    /// - Returns: The outgoing response.
    public func makeOutgoingResponse(error: Error) -> OutgoingResponse {
        let ostr = OutputStream(
            communicator: adapter.getCommunicator(), encoding: currentProtocolEncoding)

        if requestId != 0 {
            ostr.writeBlob(replyHdr)
            ostr.write(requestId)
        }

        var replyStatus: ReplyStatus
        var exceptionId: String
        var exceptionDetails: String? = nil
        var unknownExceptionMessage: String? = nil

        switch error {
        case let rfe as RequestFailedException:
            exceptionId = rfe.ice_id()

            replyStatus =
                switch rfe {
                case is ObjectNotExistException:
                    .objectNotExist
                case is FacetNotExistException:
                    .facetNotExist
                case is OperationNotExistException:
                    .operationNotExist
                default:
                    fatalError("Unexpected RequestFailedException subclass")
                }

            var id = rfe.id
            var facet = rfe.facet
            var operation = rfe.operation

            if id.name.isEmpty {
                id = self.id
                facet = self.facet
            }

            if operation.isEmpty {
                operation = self.operation
            }

            exceptionDetails = "" // not use

            if requestId != 0 {
                ostr.write(replyStatus.rawValue)
                ostr.write(id)
                if facet.isEmpty {
                    ostr.write(size: 0)
                } else {
                    ostr.write([facet])
                }
                ostr.write(operation)
            }

        case let ex as UserException:
            exceptionId = ex.ice_id()
            replyStatus = .userException

            if requestId != 0 {
                ostr.write(replyStatus.rawValue)
                ostr.startEncapsulation(encoding: encoding, format: .SlicedFormat)
                ostr.write(ex)
                ostr.endEncapsulation()
            }

        case let ex as UnknownLocalException:
            exceptionId = ex.ice_id()
            replyStatus = .unknownLocalException
            unknownExceptionMessage = ex.message

        case let ex as UnknownUserException:
            exceptionId = ex.ice_id()
            replyStatus = .unknownUserException
            unknownExceptionMessage = ex.message

        case let ex as UnknownException:
            exceptionId = ex.ice_id()
            replyStatus = .unknownException
            unknownExceptionMessage = ex.message

        case let ex as LocalException:
            exceptionId = ex.ice_id()
            replyStatus = .unknownLocalException
            unknownExceptionMessage = "dispatch failed with \(exceptionId): \(ex.message)"

        default:
            replyStatus = .unknownException
            exceptionId = "\(type(of: error))"
            unknownExceptionMessage = "dispatch failed with \(exceptionId)"
        }

        if requestId != 0,
            replyStatus == .unknownUserException || replyStatus == .unknownLocalException
                || replyStatus == .unknownException
        {
            ostr.write(replyStatus.rawValue)
            ostr.write(unknownExceptionMessage!)
        }

        return OutgoingResponse(
            replyStatus: replyStatus, exceptionId: exceptionId,
            exceptionDetails: exceptionDetails ?? "\(error)",
            outputStream: ostr)
    }

    /// Starts the output stream for a reply, with everything up to and including the reply status. When the request ID
    /// is 0 (one-way request), the returned output stream is empty.
    /// - Parameter replyStatus: The reply status.
    /// - Returns: The output stream.
    private func startReplyStream(replyStatus: ReplyStatus = .ok) -> OutputStream {
        let ostr = OutputStream(
            communicator: adapter.getCommunicator(), encoding: currentProtocolEncoding)
        if requestId != 0 {
            ostr.writeBlob(replyHdr)
            ostr.write(requestId)
            ostr.write(replyStatus.rawValue)
        }
        return ostr
    }
}

private let currentProtocolEncoding = EncodingVersion(major: 1, minor: 0)

private let replyHdr: [UInt8] = [
    0x49, 0x63, 0x65, 0x50,  // IceP magic
    1, 0,  // Protocol version (1.0)
    1, 0,  // Protocol encoding version (1.0)
    2,  // Reply message
    0,  // Compression status (not compressed)
    0, 0, 0, 0,  // Message size (placeholder)
]
