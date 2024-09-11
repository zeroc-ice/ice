// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates a DNS problem. */
public final class DNSException extends SyscallException {
    public DNSException(String host) {
        super("Cannot resolve host '" + host + "'");
    }

    public DNSException(String host, Throwable cause) {
        super("Cannot resolve host '" + host + "'", cause);
    }

    public String ice_id() {
        return "::Ice::DNSException";
    }

    private static final long serialVersionUID = 824453629913156786L;
}
