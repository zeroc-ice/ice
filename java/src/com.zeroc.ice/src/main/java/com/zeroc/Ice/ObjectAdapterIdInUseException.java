// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised if an {@link ObjectAdapter} cannot be activated. This happens if the
 * {@link Locator} detects another active {@link ObjectAdapter} with the same adapter id.
 */
public final class ObjectAdapterIdInUseException extends LocalException {
    public ObjectAdapterIdInUseException(String id) {
        super("An object adapter with adapter ID '" + id + "' is already active.");
        this.id = id;
    }

    public String ice_id() {
        return "::Ice::ObjectAdapterIdInUseException";
    }

    /** Adapter ID. */
    public final String id;

    private static final long serialVersionUID = 609699148378869554L;
}
