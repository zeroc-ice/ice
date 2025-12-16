// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Represents the response to an incoming request. It's returned by {@link Object#dispatch(IncomingRequest)}.
 */
public final class OutgoingResponse {
    /**
     * The exception ID of the response.
     *
     * <p>It's {@code null} when {@link #replyStatus} is {@link ReplyStatus#Ok} or {@link ReplyStatus#UserException}.
     * Otherwise, this ID is the value returned by {@link LocalException#ice_id}. For other exceptions,
     * this ID is the full name of the exception's type.
     */
    public final String exceptionId;

    /**
     * The full details of the exception marshaled into the response.
     *
     * <p>The exception details. It's {@code null} when {@link #replyStatus} is {@link ReplyStatus#Ok} or
     * {@link ReplyStatus#UserException}.
     */
    public final String exceptionDetails;

    /** The output stream buffer of the response. This output stream should not be written to after construction. */
    public final OutputStream outputStream;

    /** The reply status of the response, as an int. */
    public final int replyStatus;

    /** The number of bytes in the response's payload. */
    public final int size;

    /**
     * Constructs an OutgoingResponse object.
     *
     * @param replyStatus the reply status, as an int
     * @param exceptionId the type ID of the exception, when the response carries an exception other than a
     *     user exception
     * @param exceptionDetails the full details of the exception, when the response carries an exception other than a
     *     user exception
     * @param outputStream the output stream that holds the response
     * @see ReplyStatus
     */
    public OutgoingResponse(int replyStatus, String exceptionId, String exceptionDetails, OutputStream outputStream) {
        this.replyStatus = replyStatus;
        this.exceptionId = exceptionId;
        this.exceptionDetails = exceptionDetails;
        this.outputStream = outputStream;
        this.size = outputStream.isEmpty() ? 0 : outputStream.size() - Protocol.headerSize - 4;
    }

    /**
     * Constructs an OutgoingResponse object with the {@link ReplyStatus#Ok} status.
     *
     * @param outputStream the output stream that holds the response
     */
    public OutgoingResponse(OutputStream outputStream) {
        this(ReplyStatus.Ok.value(), null, null, outputStream);
    }
}
