// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when an {@link ObjectAdapter} cannot be activated. This can happen when a
 * {@link Locator} implementation detects another active {@link ObjectAdapter} with the same adapter ID.
 */
public final class ObjectAdapterIdInUseException extends LocalException {
    /**
     * Constructs an ObjectAdapterIdInUseException.
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
