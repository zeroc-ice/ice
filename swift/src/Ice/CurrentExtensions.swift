// Copyright (c) ZeroC, Inc.

import Foundation

extension Current {
    /// Creates an outgoing response with reply status `ok`.
    /// - Parameters:
    ///   - result: The result to marshal into the response payload.
    ///   - formatType: The class format.
    ///   - marshal: The action that marshals result into an output stream.
    /// - Returns: The outgoing response.
    public func makeOutgoingResponse<T>(_ result: T, formatType: FormatType, marshal: (OutputStream, T) -> Void)
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

    /// Creates an outgoing response with the specified payload.
    /// - Parameters:
    ///   - ok: When true, the reply status of the response is `ok`; otherwise, it's `userException`.
    ///   - encapsulation: The payload of the response.
    /// - Returns: The outgoing response.
    public func makeOutgoingResponse(ok: Bool, encapsulation: Data) -> OutgoingResponse {
        let ostr = startReplyStream(replyStatus: ok ? .ok : .userException)

        if requestId != 0 {
            if encapsulation.isEmpty {
                ostr.writeEmptyEncapsulation(encoding)
            } else {
                ostr.writeEncapsulation(encapsulation)
            }
        }
        return OutgoingResponse(ostr)
    }

    /// Creates an outgoing response that marshals an exception.
    /// - Parameter error: The exception to marshal into the response payload.
    /// - Returns: The outgoing response.
    public func makeOutgoingResponse(error: Error) -> OutgoingResponse {
        let ostr = OutputStream(communicator: adapter.getCommunicator(), encoding: currentProtocolEncoding)

        if requestId != 0 {
            ostr.writeBlob(replyHdr)
            ostr.write(requestId)
        }

        var replyStatus: ReplyStatus
        var exceptionId: String?
        var exceptionMessage: String?

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

            if rfe.id.name.isEmpty {
                rfe.id = id
            }

            if rfe.facet.isEmpty {
                rfe.facet = facet
            }

            if rfe.operation.isEmpty {
                rfe.operation = operation
            }

            // We must call ice_print _after_ setting the properties above.
            exceptionMessage = rfe.ice_print()

            if requestId != 0 {
                ostr.write(replyStatus.rawValue)
                ostr.write(rfe.id)
                if rfe.facet.isEmpty {
                    ostr.write(size: 0)
                } else {
                    ostr.write([rfe.facet])
                }
                ostr.write(rfe.operation)
            }

        case let ex as UserException:
            exceptionId = ex.ice_id()
            exceptionMessage = "\(ex)"
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
            exceptionMessage = ex.unknown

        case let ex as UnknownUserException:
            exceptionId = ex.ice_id()
            replyStatus = .unknownUserException
            exceptionMessage = ex.unknown

        case let ex as UnknownException:
            exceptionId = ex.ice_id()
            replyStatus = .unknownException
            exceptionMessage = ex.unknown

        case let ex as LocalException:
            exceptionId = ex.ice_id()
            replyStatus = .unknownLocalException
            exceptionMessage = "\(ex)"

        case let ex as Exception:
            exceptionId = ex.ice_id()
            replyStatus = .unknownException
            exceptionMessage = "\(ex)"

        default:
            replyStatus = .unknownException
            exceptionId = "\(type(of: error))"
            exceptionMessage = "\(error)"
        }

        if requestId != 0,
            replyStatus == .unknownUserException || replyStatus == .unknownLocalException
                || replyStatus == .unknownException
        {
            ostr.write(replyStatus.rawValue)
            ostr.write(exceptionMessage!)
        }

        return OutgoingResponse(
            replyStatus: replyStatus, exceptionId: exceptionId, exceptionMessage: exceptionMessage, outputStream: ostr)
    }

    /// Starts the output stream for a reply, with everything up to and including the reply status. When the request ID
    /// is 0 (one-way request), the returned output stream is empty.
    /// - Parameter replyStatus: The reply status.
    /// - Returns: The output stream.
    public func startReplyStream(replyStatus: ReplyStatus = .ok) -> OutputStream {
        let ostr = OutputStream(communicator: adapter.getCommunicator(), encoding: currentProtocolEncoding)
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
