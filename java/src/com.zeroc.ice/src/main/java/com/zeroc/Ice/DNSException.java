// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown to report a DNS resolution failure. */
public final class DNSException extends SyscallException {
    /**
     * Constructs a DNSException.
     *
     * @param host the host that could not be resolved
     */
    public DNSException(String host) {
        super("Cannot resolve host '" + host + "'");
    }

    /**
     * Constructs a DNSException with a cause.
     *
     * @param host the host that could not be resolved
     * @param cause the cause
     */
    public DNSException(String host, Throwable cause) {
        super("Cannot resolve host '" + host + "'", cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::DNSException";
    }

    private static final long serialVersionUID = 824453629913156786L;
}
