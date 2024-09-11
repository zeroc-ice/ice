// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * An attempt was made to find or deregister something that is not registered with the Ice run time
 * or Ice locator. This exception is raised if an attempt is made to remove a servant, servant
 * locator, facet, value factory, plug-in, object adapter, object, or user exception factory that is
 * not currently registered. It's also raised if the Ice locator can't find an object or object
 * adapter when resolving an indirect proxy or when an object adapter is activated.
 */
public final class NotRegisteredException extends LocalException {
    public NotRegisteredException(String kindOfObject, String id) {
        super("No " + kindOfObject + " is registered with ID '" + id + "'.");
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    public String ice_id() {
        return "::Ice::NotRegisteredException";
    }

    /**
     * The kind of object that could not be removed: "servant", "facet", "object", "default
     * servant", "servant locator", "value factory", "plugin", "object adapter", "object adapter
     * with router", "replica group".
     */
    public final String kindOfObject;

    /** The ID (or name) of the object that could not be removed. */
    public final String id;

    private static final long serialVersionUID = 3335358291266771447L;
}
