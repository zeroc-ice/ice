// Copyright (c) ZeroC, Inc.

#nullable enable

using Ice.Internal;
using System.Diagnostics;

namespace Ice;

/// <summary>
/// Provides extension methods for <see cref="Current"/> to create <see cref="OutgoingResponse"/> objects.
/// </summary>
public static class OutgoingResponseCurrentExtensions
{
    public static OutgoingResponse createOutgoingResponse(
        this Current current,
        Action<OutputStream> marshal,
        FormatType formatType = FormatType.DefaultFormat)
    {
        var ostr = new OutputStream(current.adapter.getCommunicator(), Util.currentProtocolEncoding);
        if (current.requestId != 0)
        {
            try
            {
                ostr.writeBlob(Protocol.replyHdr);
                ostr.writeInt(current.requestId);
                ostr.writeByte((byte)ReplyStatus.Ok);
                ostr.startEncapsulation(current.encoding, formatType);
                marshal(ostr);
                ostr.endEncapsulation();
                return new OutgoingResponse(ostr, current);
            }
            catch (System.Exception exception)
            {
                return current.createOutgoingResponse(exception);
            }
        }
        else
        {
            Debug.Fail("A one-way request cannot return a response");
            return new OutgoingResponse(ostr, current);
        }
    }

    public static OutgoingResponse createEmptyOutgoingResponse(this Current current) =>
        new(new OutputStream(current.adapter.getCommunicator(), Util.currentProtocolEncoding), current);

    public static OutgoingResponse createOutgoingResponse(this Current current, bool ok, byte[] encapsulation)
    {
        var ostr = new OutputStream(current.adapter.getCommunicator(), Util.currentProtocolEncoding);
        if (current.requestId != 0)
        {
            try
            {
                ostr.writeBlob(Protocol.replyHdr);
                ostr.writeInt(current.requestId);
                ostr.writeByte((byte)(ok ? ReplyStatus.Ok : ReplyStatus.UserException));
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
        return new OutgoingResponse(ostr, current);
    }

    public static OutgoingResponse createOutgoingResponse(this Current current, System.Exception exception)
    {
        try
        {
            return current.createOutgoingResponseCore(exception);
        }
        catch (System.Exception ex)
        {
            return current.createOutgoingResponseCore(ex);
        }
    }

    private static OutgoingResponse createOutgoingResponseCore(this Current current, System.Exception exc)
    {
        var ostr = new OutputStream(current.adapter.getCommunicator(), Util.currentProtocolEncoding);

         if (current.requestId != 0)
        {
            ostr.writeBlob(Protocol.replyHdr);
            ostr.writeInt(current.requestId);
        }
        ReplyStatus replyStatus;
        string exceptionId;
        string exceptionMessage;

        try
        {
            throw exc;
        }
        catch (RequestFailedException rfe)
        {
            exceptionId = rfe.ice_id();
            exceptionMessage = rfe.ToString();

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
        }
        catch (UserException ex)
        {
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
        }
        catch (UnknownLocalException ex)
        {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus.UnknownLocalException;
            exceptionMessage = ex.unknown;
        }
        catch (UnknownUserException ex)
        {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus.UnknownUserException;
            exceptionMessage = ex.unknown;
        }
        catch (UnknownException ex)
        {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus.UnknownException;
            exceptionMessage = ex.unknown;
        }
        catch (LocalException ex)
        {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus.UnknownLocalException;
            exceptionMessage = ex.ToString();
        }
        catch (Ice.Exception ex)
        {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus.UnknownException;
            exceptionMessage = ex.ToString();
        }
        catch (System.Exception ex)
        {
            replyStatus = ReplyStatus.UnknownException;
            exceptionId = ex.GetType().FullName ?? "System.Exception";
            exceptionMessage = ex.ToString();
        }

        if ((current.requestId != 0) &&
            (replyStatus == ReplyStatus.UnknownUserException || replyStatus == ReplyStatus.UnknownLocalException ||
             replyStatus == ReplyStatus.UnknownException))
        {
            ostr.writeByte((byte)replyStatus);
            ostr.writeString(exceptionMessage);
        }

        return new OutgoingResponse(
            replyStatus,
            exceptionId,
            exceptionMessage,
            ostr,
            current);
    }
}
