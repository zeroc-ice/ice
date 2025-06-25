// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

// TODO: should this derive from 'LocalException'?
/**
 * Exception thrown to indicate a WebSocket protocol error or failure.
 */
final class WebSocketException extends RuntimeException {
    /**
     * Constructs a WebSocketException with the specified message.
     *
     * @param message the detail message
     */
    public WebSocketException(String message) {
        super(message);
    }

    /**
     * Constructs a WebSocketException with a cause.
     *
     * @param cause the cause
     */
    public WebSocketException(Throwable cause) {
        super(cause);
    }

    private static final long serialVersionUID = 133989672864895760L;
}
