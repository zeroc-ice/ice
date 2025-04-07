// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.CompletionException;
import java.util.function.BiConsumer;

/** Provides information about an incoming request being dispatched. */
public final class Current implements Cloneable {
    /** The object adapter that received the request. */
    public final ObjectAdapter adapter;

    /**
     * The connection that received the request. It's null when the invocation and dispatch are
     * collocated.
     */
    public final Connection con;

    /** The identity of the target Ice object. */
    public final Identity id;

    /** The facet of the target Ice object. */
    public String facet;

    /** The name of the operation. */
    public String operation;

    /** The operation mode (idempotent or not). */
    public OperationMode mode;

    /** The request context. */
    public final Map<String, String> ctx;

    /** The request ID. 0 means the request is a one-way request. */
    public final int requestId;

    /** The encoding of the request payload. */
    public EncodingVersion encoding;

    /**
     * Creates a new Current object.
     *
     * @param adapter The adapter.
     * @param con The connection. Can be null.
     * @param id The identity of the target object.
     * @param facet The facet of the target object.
     * @param operation The name of the operation.
     * @param mode The operation mode.
     * @param ctx The request context.
     * @param requestId The request ID.
     * @param encoding The encoding of the payload.
     */
    public Current(
            ObjectAdapter adapter,
            Connection con,
            Identity id,
            String facet,
            String operation,
            OperationMode mode,
            Map<String, String> ctx,
            int requestId,
            EncodingVersion encoding) {
        // We may occasionally construct a Current with a null adapter, however we never
        // return such a current to the application code.
        Objects.requireNonNull(id);
        Objects.requireNonNull(facet);
        Objects.requireNonNull(operation);
        Objects.requireNonNull(mode);
        Objects.requireNonNull(ctx);
        // Objects.requireNonNull(encoding);
        this.adapter = adapter;
        this.con = con;
        this.id = id;
        this.facet = facet;
        this.operation = operation;
        this.mode = mode;
        this.ctx = ctx;
        this.requestId = requestId;
        this.encoding = encoding;
    }

    /**
     * Ensures the operation mode of an incoming request is not idempotent. The generated code calls
     * this method to ensure that when an operation's mode is not idempotent (locally), the incoming
     * request's operation mode is not idempotent.
     *
     * @throws MarshalException Thrown when the request's operation mode is {@link
     *     OperationMode#Idempotent} or {@link OperationMode#Nonmutating}.
     */
    public void checkNonIdempotent() {
        if (mode != OperationMode.Normal) {
            throw new MarshalException(
                    String.format(
                            "Operation mode mismatch for operation '%s': received %s for non-idempotent operation",
                            operation, mode));
        }
    }

    @Override
    public Current clone() {
        Current clone = null;
        try {
            clone = (Current) super.clone();
        } catch (CloneNotSupportedException ex) {
            assert false; // impossible
        }
        return clone;
    }

    /**
     * Creates an outgoing response with reply status {@link ReplyStatus#Ok}.
     *
     * @param <TResult> The type of result.
     * @param result The result to marshal into the response payload.
     * @param marshal The action that marshals result into an output stream.
     * @param formatType The class format.
     * @return A new outgoing response.
     */
    public <TResult> OutgoingResponse createOutgoingResponse(
            TResult result, BiConsumer<OutputStream, TResult> marshal, FormatType formatType) {
        OutputStream ostr = startReplyStream();
        if (requestId != 0) {
            try {
                ostr.startEncapsulation(encoding, formatType);
                marshal.accept(ostr, result);
                ostr.endEncapsulation();
                return new OutgoingResponse(ostr);
            } catch (Exception exception) {
                return createOutgoingResponse(exception);
            }
        } else {
            assert false : "A one-way request cannot return a response";
            return new OutgoingResponse(ostr);
        }
    }

    /**
     * Creates an empty outgoing response with reply status {@link ReplyStatus#Ok}.
     *
     * @return An outgoing response with an empty payload.
     */
    public OutgoingResponse createEmptyOutgoingResponse() {
        OutputStream ostr = startReplyStream();
        if (requestId != 0) {
            try {
                ostr.writeEmptyEncapsulation(encoding);
            } catch (Exception ex) {
                return createOutgoingResponse(ex);
            }
        }
        return new OutgoingResponse(ostr);
    }

    /**
     * Creates an outgoing response with the specified payload.
     *
     * @param ok When true, the reply status of the response is {@link ReplyStatus#Ok}; otherwise,
     *     it's {@link ReplyStatus#UserException}.
     * @param encapsulation The payload of the response.
     * @return A new outgoing response.
     */
    public OutgoingResponse createOutgoingResponse(boolean ok, byte[] encapsulation) {
        // For compatibility with the Ice 3.7 and earlier.
        encapsulation = encapsulation != null ? encapsulation : new byte[0];

        OutputStream ostr = startReplyStream(ok ? ReplyStatus.Ok : ReplyStatus.UserException);

        if (requestId != 0) {
            try {
                if (encapsulation.length > 0) {
                    ostr.writeEncapsulation(encapsulation);
                } else {
                    ostr.writeEmptyEncapsulation(encoding);
                }
            } catch (Throwable ex) {
                return createOutgoingResponse(ex);
            }
        }
        return new OutgoingResponse(ostr);
    }

    /**
     * Creates an outgoing response that marshals an exception.
     *
     * @param exception The exception to marshal into the response payload.
     * @return A new outgoing response.
     */
    public OutgoingResponse createOutgoingResponse(Throwable exception) {
        assert exception != null;
        try {
            if (exception instanceof CompletionException completionException) {
                // Unwrap the completion exception.
                exception = completionException.getCause();
                assert exception != null;
            }

            return createOutgoingResponseCore(exception);
        } catch (Throwable ex) {
            // Try a second time with the marshal exception. This should not fail.
            return createOutgoingResponseCore(ex);
        }
    }

    private OutgoingResponse createOutgoingResponseCore(Throwable exc) {
        OutputStream ostr;

        if (requestId != 0) {
            // The default class format doesn't matter since we always encode user exceptions in
            // Sliced format.;
            ostr =
                    new OutputStream(
                            Protocol.currentProtocolEncoding, FormatType.SlicedFormat, false);
            ostr.writeBlob(Protocol.replyHdr);
            ostr.writeInt(requestId);
        } else {
            ostr = new OutputStream();
        }

        int replyStatus;
        String exceptionId;
        String dispatchExceptionMessage = null;

        // TODO: replace by switch statement with Java 21
        if (exc instanceof UserException ex) {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus.UserException.value();

            if (requestId != 0) {
                ReplyStatus.ice_write(ostr, ReplyStatus.UserException);
                ostr.startEncapsulation(encoding, FormatType.SlicedFormat);
                ostr.writeException(ex);
                ostr.endEncapsulation();
            }
        } else if (exc instanceof DispatchException ex) {
            exceptionId = ex.ice_id();
            replyStatus = ex.replyStatus;
            dispatchExceptionMessage = ex.getMessage();
        } else if (exc instanceof LocalException ex) {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus.UnknownLocalException.value();
        } else {
            replyStatus = ReplyStatus.UnknownException.value();
            exceptionId =
                    exc.getClass().getName() != null
                            ? exc.getClass().getName()
                            : "java.lang.Exception";
        }

        if (replyStatus > ReplyStatus.UserException.value() && requestId != 0) {
            // two-way, so we marshal a reply

            // We can't use ReplyStatus to marshal a possibly unknown reply status value.
            ostr.writeByte((byte) replyStatus);

            if (replyStatus >= ReplyStatus.ObjectNotExist.value()
                    && replyStatus <= ReplyStatus.OperationNotExist.value()) {

                Identity objectId = new Identity();
                String objectFacet = "";
                String operationName = "";
                if (exc instanceof RequestFailedException rfe) {
                    objectId = rfe.id;
                    objectFacet = rfe.facet;
                    operationName = rfe.operation;
                }

                if (objectId.name.isEmpty()) {
                    objectId = this.id;
                    objectFacet = this.facet;
                }
                if (operationName.isEmpty()) {
                    operationName = this.operation;
                }
                Identity.ice_write(ostr, objectId);

                if (objectFacet.isEmpty()) {
                    ostr.writeStringSeq(new String[]{});
                } else {
                    ostr.writeStringSeq(new String[]{objectFacet});
                }
                ostr.writeString(operationName);
                // and we don't use the dispatchExceptionMessage.
            } else {
                // If the exception is a DispatchException, we keep its message as-is; otherwise, we
                // create a custom message. This message doesn't include the stack trace.
                if (dispatchExceptionMessage == null) {
                    dispatchExceptionMessage = "Dispatch failed with " + exc.toString();
                }
                ostr.writeString(dispatchExceptionMessage);
            }
        }

        var stringWriter = new StringWriter();
        var printWriter = new PrintWriter(stringWriter);
        exc.printStackTrace(printWriter);
        printWriter.flush();

        return new OutgoingResponse(replyStatus, exceptionId, stringWriter.toString(), ostr);
    }

    /**
     * Starts the output stream for a successful reply, with everything up to and including the
     * reply status. When the request ID is 0 (one-way request), the returned output stream is
     * empty.
     *
     * @return The new output stream with status ReplyStatus.Ok.
     */
    public OutputStream startReplyStream() {
        return startReplyStream(ReplyStatus.Ok);
    }

    /**
     * Starts the output stream for a reply, with everything up to and including the reply status.
     * When the request ID is 0 (one-way request), the returned output stream is empty.
     *
     * @param replyStatus The reply status.
     * @return The new output stream.
     */
    private OutputStream startReplyStream(ReplyStatus replyStatus) {
        if (requestId == 0) {
            return new OutputStream();
        } else {
            assert (adapter != null);
            var ostr =
                    new OutputStream(
                            Protocol.currentProtocolEncoding,
                            adapter.getCommunicator()
                                    .getInstance()
                                    .defaultsAndOverrides()
                                    .defaultFormat,
                            false);
            ostr.writeBlob(Protocol.replyHdr);
            ostr.writeInt(requestId);
            ostr.writeByte((byte) replyStatus.value());
            return ostr;
        }
    }
}
