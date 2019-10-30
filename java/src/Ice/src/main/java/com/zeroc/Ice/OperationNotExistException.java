//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if an operation for a given object does
 * not exist on the server. Typically this is caused by either the
 * client or the server using an outdated Slice specification.
 **/
public class OperationNotExistException extends RequestFailedException
{
    public OperationNotExistException()
    {
        super();
    }

    public OperationNotExistException(Throwable cause)
    {
        super(cause);
    }

    public OperationNotExistException(Identity id, String facet, String operation)
    {
        super(id, facet, operation);
    }

    public OperationNotExistException(Identity id, String facet, String operation, Throwable cause)
    {
        super(id, facet, operation, cause);
    }

    public String ice_id()
    {
        return "::Ice::OperationNotExistException";
    }

    /** @hidden */
    public static final long serialVersionUID = 3973646568523472620L;
}
