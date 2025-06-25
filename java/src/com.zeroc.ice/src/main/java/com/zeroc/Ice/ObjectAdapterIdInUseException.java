// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised if an {@link ObjectAdapter} cannot be activated. This happens if the
 * {@link Locator} detects another active {@link ObjectAdapter} with the same adapter id.
 */
public final class ObjectAdapterIdInUseException extends LocalException {
    /**
     * Constructs an ObjectAdapterIdInUseException with the adapter ID.
     *
     * @param id the adapter ID that is already in use
     */
    public ObjectAdapterIdInUseException(String id) {
        super("An object adapter with adapter ID '" + id + "' is already active.");
        this.id = id;
    }

    @Override
    public String ice_id() {
        return "::Ice::ObjectAdapterIdInUseException";
    }

    /** Adapter ID. */
    public final String id;

    private static final long serialVersionUID = 609699148378869554L;
}
