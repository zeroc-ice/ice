//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if a request failed. This exception, and
 * all exceptions derived from {@link RequestFailedException}, are
 * transmitted by the Ice protocol, even though they are declared
 * <code>local</code>.
 **/
public class RequestFailedException extends LocalException
{
    public RequestFailedException()
    {
        this.id = new Identity();
        this.facet = "";
        this.operation = "";
    }

    public RequestFailedException(Throwable cause)
    {
        super(cause);
        this.id = new Identity();
        this.facet = "";
        this.operation = "";
    }

    public RequestFailedException(Identity id, String facet, String operation)
    {
        this.id = id;
        this.facet = facet;
        this.operation = operation;
    }

    public RequestFailedException(Identity id, String facet, String operation, Throwable cause)
    {
        super(cause);
        this.id = id;
        this.facet = facet;
        this.operation = operation;
    }

    public String ice_id()
    {
        return "::Ice::RequestFailedException";
    }

    /**
     * The identity of the Ice Object to which the request was sent.
     **/
    public Identity id;

    /**
     * The facet to which the request was sent.
     **/
    public String facet;

    /**
     * The operation name of the request.
     **/
    public String operation;

    /** @hidden */
    public static final long serialVersionUID = 4181164754424262091L;
}
