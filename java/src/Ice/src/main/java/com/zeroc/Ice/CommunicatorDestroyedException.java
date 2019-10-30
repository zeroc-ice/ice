//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if the {@link Communicator} has been destroyed.
 *
 * @see Communicator#destroy
 **/
public class CommunicatorDestroyedException extends LocalException
{
    public CommunicatorDestroyedException()
    {
    }

    public CommunicatorDestroyedException(Throwable cause)
    {
        super(cause);
    }

    public String ice_id()
    {
        return "::Ice::CommunicatorDestroyedException";
    }

    /** @hidden */
    public static final long serialVersionUID = 752535633703849L;
}
