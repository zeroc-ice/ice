// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when you attempt to find or deregister something that is not registered with the
 * Ice runtime or Ice locator.
 */
public final class NotRegisteredException extends LocalException {
    /**
     * Constructs a NotRegisteredException with the object kind and ID.
     *
     * @param kindOfObject the kind of object that was not found
     * @param id           the ID of the object that was not found
     */
    public NotRegisteredException(String kindOfObject, String id) {
        super("No " + kindOfObject + " is registered with ID '" + id + "'.");
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    @Override
    public String ice_id() {
        return "::Ice::NotRegisteredException";
    }

    /**
     * The kind of object that could not be removed: "servant", "facet", "object", "default
     * servant", "servant locator", "plugin", "object adapter", "object adapter
     * with router", "replica group".
     */
    public final String kindOfObject;

    /** The ID (or name) of the object that could not be removed. */
    public final String id;

    private static final long serialVersionUID = 3335358291266771447L;
}
