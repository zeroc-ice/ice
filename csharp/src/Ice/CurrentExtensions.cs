// Copyright (c) ZeroC, Inc.

#nullable enable

using Ice.Internal;
using System.Diagnostics;

namespace Ice;

/// <summary>
/// Provides extension methods for <see cref="Current"/>.
/// </summary>
public static class CurrentExtensions
{
    /// <summary>
    /// Creates an outgoing response with reply status <see cref="ReplyStatus.Ok"/>.
    /// </summary>
    /// <typeparam name="TResult">The type of result.</typeparam>
    /// <param name="current">The current object of the corresponding incoming request.</param>
    /// <param name="result">The result to marshal into the response payload.</param>
    /// <param name="marshal">The action that marshals result into an output stream.</param>
    /// <param name="formatType">The class format.</param>
    /// <returns>A new outgoing response.</returns>
    public static OutgoingResponse createOutgoingResponse<TResult>(
        this Current current,
        TResult result,
        Action<OutputStream, TResult> marshal,
        FormatType formatType = FormatType.DefaultFormat)
    {
        OutputStream ostr = current.startReplyStream();
        if (current.requestId != 0)
        {
            try
            {
                ostr.startEncapsulation(current.encoding, formatType);
                marshal(ostr, result);
                ostr.endEncapsulation();
                return new OutgoingResponse(ostr);
            }
            catch (System.Exception exception)
            {
                return current.createOutgoingResponse(exception);
            }
        }
        else
        {
            Debug.Fail("A one-way request cannot return a response");
            return new OutgoingResponse(ostr);
        }
    }

    /// <summary>
    /// Creates an empty outgoing response with reply status <see cref="ReplyStatus.Ok"/>.
    /// </summary>
    /// <param name="current">The current object of the corresponding incoming request.</param>
    /// <returns>An outgoing response with an empty payload.</returns>
    public static OutgoingResponse createEmptyOutgoingResponse(this Current current)
    {
        OutputStream ostr = current.startReplyStream();
        if (current.requestId != 0)
        {
            try
            {
                ostr.writeEmptyEncapsulation(current.encoding);
            }
            catch (System.Exception ex)
            {
                return current.createOutgoingResponse(ex);
            }
        }
        return new OutgoingResponse(ostr);
    }

    /// <summary>
    /// Creates an outgoing response with the specified payload.
    /// </summary>
    /// <param name="current">The current object of the corresponding incoming request.</param>
    /// <param name="ok">When true, the reply status of the response is <see cref="ReplyStatus.Ok" />; otherwise, it's
    /// <see cref="ReplyStatus.UserException" />.</param>
    /// <param name="encapsulation">The payload of the response.</param>
    /// <returns>A new outgoing response.</returns>
    public static OutgoingResponse createOutgoingResponse(this Current current, bool ok, byte[] encapsulation)
    {
        // For compatibility with the Ice 3.7 and earlier.
        encapsulation ??= [];

        OutputStream ostr = current.startReplyStream(ok ? ReplyStatus.Ok : ReplyStatus.UserException);

        if (current.requestId != 0)
        {
            try
            {
                if (encapsulation.Length > 0)
                {
                    ostr.writeEncapsulation(encapsulation);
                }
                else
                {
                    ostr.writeEmptyEncapsulation(current.encoding);
                }
            }
            catch (System.Exception ex)
            {
                return current.createOutgoingResponse(ex);
            }
        }
        return new OutgoingResponse(ostr);
    }

    /// <summary>
    /// Creates an outgoing response that marshals an exception.
    /// </summary>
    /// <param name="current">The current object of the corresponding incoming request.</param>
    /// <param name="exception">The exception to marshal into the response payload.</param>
    /// <returns>A new outgoing response.</returns>
    public static OutgoingResponse createOutgoingResponse(this Current current, System.Exception exception)
    {
        try
        {
            return createOutgoingResponseCore(exception);
        }
        catch (System.Exception ex)
        {
            // Try a second time with the marshal exception. This should not fail.
            return createOutgoingResponseCore(ex);
        }

        OutgoingResponse createOutgoingResponseCore(System.Exception exc)
        {
            OutputStream ostr;

            if (current.requestId != 0)
            {
                ostr = new OutputStream(current.adapter.getCommunicator(), Util.currentProtocolEncoding);
                ostr.writeBlob(Protocol.replyHdr);
                ostr.writeInt(current.requestId);
            }
            else
            {
                ostr = new OutputStream();
            }

            ReplyStatus replyStatus;
            string exceptionId;
            string exceptionMessage;

            switch (exc)
            {
                case RequestFailedException rfe:
                    exceptionId = rfe.ice_id();

                    replyStatus = rfe switch
                    {
                        ObjectNotExistException _ => ReplyStatus.ObjectNotExist,
                        FacetNotExistException _ => ReplyStatus.FacetNotExist,
                        OperationNotExistException _ => ReplyStatus.OperationNotExist,
                        _ => throw new Ice.MarshalException("Unexpected exception type")
                    };

                    if (rfe.id.name.Length == 0)
                    {
                        rfe.id = current.id;
                    }

                    if (rfe.facet.Length == 0 && current.facet.Length > 0)
                    {
                        rfe.facet = current.facet;
                    }

                    if (rfe.operation.Length == 0 && current.operation.Length > 0)
                    {
                        rfe.operation = current.operation;
                    }

                    // Called after fixing id, facet and operation.
                    exceptionMessage = rfe.ToString();

                    if (current.requestId != 0)
                    {
                        ostr.writeByte((byte)replyStatus);
                        Identity.ice_write(ostr, rfe.id);

                        if (rfe.facet.Length == 0)
                        {
                            ostr.writeStringSeq([]);
                        }
                        else
                        {
                            ostr.writeStringSeq([rfe.facet]);
                        }

                        ostr.writeString(rfe.operation);
                    }
                    break;

                case UserException ex:
                    exceptionId = ex.ice_id();
                    exceptionMessage = ex.ToString();

                    replyStatus = ReplyStatus.UserException;

                    if (current.requestId != 0)
                    {
                        ostr.writeByte((byte)replyStatus);
                        ostr.startEncapsulation(current.encoding, FormatType.SlicedFormat);
                        ostr.writeException(ex);
                        ostr.endEncapsulation();
                    }
                    break;

                case UnknownLocalException ex:
                    exceptionId = ex.ice_id();
                    replyStatus = ReplyStatus.UnknownLocalException;
                    exceptionMessage = ex.unknown;
                    break;

                case UnknownUserException ex:
                    exceptionId = ex.ice_id();
                    replyStatus = ReplyStatus.UnknownUserException;
                    exceptionMessage = ex.unknown;
                    break;

                case UnknownException ex:
                    exceptionId = ex.ice_id();
                    replyStatus = ReplyStatus.UnknownException;
                    exceptionMessage = ex.unknown;
                    break;

                case LocalException ex:
                    exceptionId = ex.ice_id();
                    replyStatus = ReplyStatus.UnknownLocalException;
                    exceptionMessage = ex.ToString();
                    break;

                case Ice.Exception ex:
                    exceptionId = ex.ice_id();
                    replyStatus = ReplyStatus.UnknownException;
                    exceptionMessage = ex.ToString();
                    break;

                default:
                    replyStatus = ReplyStatus.UnknownException;
                    exceptionId = exc.GetType().FullName ?? "System.Exception";
                    exceptionMessage = exc.ToString();
                    break;
            }

            if (current.requestId != 0 &&
                (replyStatus is
                    ReplyStatus.UnknownUserException or
                    ReplyStatus.UnknownLocalException or
                    ReplyStatus.UnknownException))
            {
                ostr.writeByte((byte)replyStatus);
                ostr.writeString(exceptionMessage);
            }

            return new OutgoingResponse(
                replyStatus,
                exceptionId,
                exceptionMessage,
                ostr);
        }
    }

    /// <summary>
    /// Starts the output stream for a reply, with everything up to and including the reply status. When the request ID
    /// is 0 (one-way request), the returned output stream is empty.
    /// </summary>
    /// <param name="communicator">The communicator.</param>
    /// <param name="requestId">The request ID.</param>
    /// <param name="replyStatus">The reply status</param>
    /// <returns>The new output stream.</returns>
    public static OutputStream startReplyStream(this Current current, ReplyStatus replyStatus = ReplyStatus.Ok)
    {
        if (current.requestId == 0)
        {
            return new OutputStream();
        }
        else
        {
            var ostr = new OutputStream(current.adapter.getCommunicator(), Util.currentProtocolEncoding);
            ostr.writeBlob(Protocol.replyHdr);
            ostr.writeInt(current.requestId);
            ostr.writeByte((byte)replyStatus);
            return ostr;
        }
    }
}
