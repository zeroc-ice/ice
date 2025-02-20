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

        var replyStatusByte: UInt8
        var exceptionId: String
        var dispatchExceptionMessage: String

        switch error {
            case let ex as UserException:
                exceptionId = ex.ice_id()
                replyStatusByte = ReplyStatus.userException.rawValue
                dispatchExceptionMessage = "" // not used

                if requestId != 0 {
                    ostr.write(replyStatusByte)
                    ostr.startEncapsulation(encoding: encoding, format: .SlicedFormat)
                    ostr.write(ex)
                    ostr.endEncapsulation()
                }
                break;

            case let ex as DispatchException:
                exceptionId = ex.ice_id()
                replyStatusByte = ex.replyStatus
                dispatchExceptionMessage = ex.message
                break;

            case let ex as LocalException:
                exceptionId = ex.ice_id();
                replyStatusByte = ReplyStatus.unknownLocalException.rawValue
                 // We don't include the stack trace in this message.
                dispatchExceptionMessage = "Dispatch failed with \(exceptionId): \(ex.message)"
                break;

            default:
                replyStatusByte = ReplyStatus.unknownException.rawValue
                exceptionId = "\(type(of: error))"
                // We don't include the stack trace in this message.
                dispatchExceptionMessage = "Dispatch failed with \(exceptionId)."
        }

        if replyStatusByte > ReplyStatus.userException.rawValue && requestId != 0 {
            // two-way, so we marshal a reply
            ostr.write(replyStatusByte)

            if replyStatusByte == ReplyStatus.objectNotExist.rawValue ||
                replyStatusByte == ReplyStatus.facetNotExist.rawValue ||
                replyStatusByte == ReplyStatus.operationNotExist.rawValue {
                let rfe = error as? RequestFailedException
                var id = rfe?.id ?? Identity()
                var facet = rfe?.facet ?? ""
                if id.name.isEmpty {
                    id = self.id
                    facet = self.facet
                }
                var operation = rfe?.operation ?? ""
                if operation.isEmpty {
                    operation = self.operation
                }

                ostr.write(id)
                if facet.isEmpty {
                    ostr.write(size: 0)
                } else {
                    ostr.write([facet])
                }
                ostr.write(operation)
                // We don't use the dispatchException message in this case.
            } else {
                ostr.write(dispatchExceptionMessage);
            }
        }

        return OutgoingResponse(replyStatus: replyStatusByte, exceptionId: exceptionId, exceptionDetails: "\(error)", outputStream: ostr)
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
