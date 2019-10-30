//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if no facet with the given name exists,
 * but at least one facet with the given identity exists.
 **/
public class FacetNotExistException extends RequestFailedException
{
    public FacetNotExistException()
    {
        super();
    }

    public FacetNotExistException(Throwable cause)
    {
        super(cause);
    }

    public FacetNotExistException(Identity id, String facet, String operation)
    {
        super(id, facet, operation);
    }

    public FacetNotExistException(Identity id, String facet, String operation, Throwable cause)
    {
        super(id, facet, operation, cause);
    }

    public String ice_id()
    {
        return "::Ice::FacetNotExistException";
    }

    /** @hidden */
    public static final long serialVersionUID = -584705670010603188L;
}
