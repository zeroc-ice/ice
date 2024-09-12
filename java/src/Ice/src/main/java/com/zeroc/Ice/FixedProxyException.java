// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception indicates that an attempt has been made to change the connection properties of a
 * fixed proxy.
 */
public final class FixedProxyException extends LocalException {
    public FixedProxyException() {
        super("Cannot change the connection properties of a fixed proxy.");
    }

    public String ice_id() {
        return "::Ice::FixedProxyException";
    }

    private static final long serialVersionUID = 3198117120780643493L;
}
