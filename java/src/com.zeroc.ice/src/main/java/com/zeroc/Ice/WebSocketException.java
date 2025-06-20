// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

// TODO: should this derive from 'LocalException'?
/**
 * Exception thrown to indicate a WebSocket protocol error or failure.
 */
final class WebSocketException extends RuntimeException {
    /**
     * Constructs a {@code WebSocketException} with the specified reason.
     *
     * @param reason the reason for this exception
     */
    public WebSocketException(String reason) {
        super(reason);
    }

    /**
     * Constructs a {@code WebSocketException} with a cause.
     *
     * @param cause the cause of this exception
     */
    public WebSocketException(Throwable cause) {
        super(cause);
    }

    private static final long serialVersionUID = 133989672864895760L;
}
