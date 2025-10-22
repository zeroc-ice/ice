// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * An attempt was made to find or deregister something that is not registered with Ice.
 */
public final class NotRegisteredException extends LocalException {
    /**
     * Constructs a NotRegisteredException with the object kind and ID.
     *
     * @param kindOfObject the kind of object that is not registered
     * @param id the ID of the object that is not registered
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
     * The kind of object that is not registered.
     */
    public final String kindOfObject;

    /** The ID (or name) of the object that is not registered. */
    public final String id;

    private static final long serialVersionUID = 3335358291266771447L;
}
