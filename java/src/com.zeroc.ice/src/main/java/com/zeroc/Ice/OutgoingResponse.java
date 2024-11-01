// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Represents the response to an incoming request. It's returned by {@link
 * Object#dispatch(IncomingRequest)}.
 */
public final class OutgoingResponse {
    /**
     * Gets the exception ID of the response.
     *
     * <p>It's null when replyStatus is {@link ReplyStatus#Ok}. Otherwise, this ID is the Slice type
     * ID of the exception marshaled into this response if this exception was defined in Slice or is
     * derived from {@link LocalException}. For other exceptions, this ID is the full name of the
     * exception's type.
     */
    public final String exceptionId;

    /**
     * Gets the full details of the exception marshaled into the response.
     *
     * <p>The exception details. It's null when replyStatus is {@link ReplyStatus#Ok}.
     */
    public final String exceptionDetails;

    /**
     * Gets the output stream buffer of the response. This output stream should not be written to
     * after construction.
     */
    public final OutputStream outputStream;

    /** Gets the reply status of the response. */
    public final ReplyStatus replyStatus;

    /** Gets the number of bytes in the response's payload. */
    public final int size;

    /**
     * Constructs an OutgoingResponse object.
     *
     * @param replyStatus The reply status.
     * @param exceptionId The ID of the exception, when the response carries an exception.
     * @param exceptionDetails The full details of the exception, when the response carries an
     *     exception.
     * @param outputStream The output stream that holds the response.
     */
    public OutgoingResponse(
            ReplyStatus replyStatus,
            String exceptionId,
            String exceptionDetails,
            OutputStream outputStream) {
        this.replyStatus = replyStatus;
        this.exceptionId = exceptionId;
        this.exceptionDetails = exceptionDetails;
        this.outputStream = outputStream;
        this.size = outputStream.isEmpty() ? 0 : outputStream.size() - Protocol.headerSize - 4;
    }

    /**
     * Constructs an OutgoingResponse object with the {@link ReplyStatus#Ok} status.
     *
     * @param outputStream The output stream that holds the response.
     */
    public OutgoingResponse(OutputStream outputStream) {
        this(ReplyStatus.Ok, null, null, outputStream);
    }
}
