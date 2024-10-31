// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

// TODO: should this derive from 'LocalException'?
final class WebSocketException extends RuntimeException {
    public WebSocketException(String reason) {
        super(reason);
    }

    public WebSocketException(Throwable cause) {
        super(cause);
    }

    private static final long serialVersionUID = 133989672864895760L;
}
