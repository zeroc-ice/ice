// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates a DNS problem. */
public final class DNSException extends SyscallException {
    /**
     * Constructs a {@code DNSException} with a message.
     *
     * @param host the host that could not be resolved
     */
    public DNSException(String host) {
        super("Cannot resolve host '" + host + "'");
    }

    /**
     * Constructs a {@code DNSException} with a message and a cause.
     *
     * @param host  the host that could not be resolved
     * @param cause the cause of this exception
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
