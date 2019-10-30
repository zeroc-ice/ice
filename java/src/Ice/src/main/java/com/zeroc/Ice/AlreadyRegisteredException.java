//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * An attempt was made to register something more than once with
 * the Ice run time.
 *
 * This exception is raised if an attempt is made to register a
 * servant, servant locator, facet, value factory, plug-in, object
 * adapter, object, or user exception factory more than once for the
 * same ID.
 **/
public class AlreadyRegisteredException extends LocalException
{
    public AlreadyRegisteredException()
    {
        this.kindOfObject = "";
        this.id = "";
    }

    public AlreadyRegisteredException(Throwable cause)
    {
        super(cause);
        this.kindOfObject = "";
        this.id = "";
    }

    public AlreadyRegisteredException(String kindOfObject, String id)
    {
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    public AlreadyRegisteredException(String kindOfObject, String id, Throwable cause)
    {
        super(cause);
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    public String ice_id()
    {
        return "::Ice::AlreadyRegisteredException";
    }

    /**
     * The kind of object that could not be removed: "servant", "facet",
     * "object", "default servant", "servant locator", "value factory", "plugin",
     * "object adapter", "object adapter with router", "replica group".
     **/
    public String kindOfObject;

    /**
     * The ID (or name) of the object that is registered already.
     **/
    public String id;

    /** @hidden */
    public static final long serialVersionUID = 392587231034664196L;
}
