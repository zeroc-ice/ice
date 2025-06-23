// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception indicates that an attempt has been made to change the connection properties of a
 * fixed proxy.
 */
public final class FixedProxyException extends LocalException {
    /**
     * Constructs a {@code FixedProxyException}.
     */
    public FixedProxyException() {
        super("Cannot change the connection properties of a fixed proxy.");
    }

    @Override
    public String ice_id() {
        return "::Ice::FixedProxyException";
    }

    private static final long serialVersionUID = 3198117120780643493L;
}
