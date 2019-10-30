//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
//

package com.zeroc.Ice;

/**
 * This exception is raised if an object does not exist on the server,
 * that is, if no facets with the given identity exist.
 **/
public class ObjectNotExistException extends RequestFailedException
{
    public ObjectNotExistException()
    {
        super();
    }

    public ObjectNotExistException(Throwable cause)
    {
        super(cause);
    }

    public ObjectNotExistException(Identity id, String facet, String operation)
    {
        super(id, facet, operation);
    }

    public ObjectNotExistException(Identity id, String facet, String operation, Throwable cause)
    {
        super(id, facet, operation, cause);
    }

    public String ice_id()
    {
        return "::Ice::ObjectNotExistException";
    }

    /** @hidden */
    public static final long serialVersionUID = 5680607485723114764L;
}
