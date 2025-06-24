// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * An exception wrapper, which is used to notify that the request handler should be cleared
 * and the invocation retried. It wraps a {@link LocalException} that caused the retry condition.
 */
class RetryException extends java.lang.Exception {
    /**
     * Constructs a RetryException with the specified LocalException cause.
     *
     * @param ex the LocalException that caused this retry condition
     */
    public RetryException(LocalException ex) {
        _ex = ex;
    }

    /**
     * Returns the LocalException that caused this retry condition.
     *
     * @return the LocalException cause
     */
    public LocalException get() {
        return _ex;
    }

    private final LocalException _ex;

    private static final long serialVersionUID = -8555917196921366848L;
}
