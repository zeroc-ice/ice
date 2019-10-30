//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised to reject an illegal servant (typically
 * a null servant)
 **/
public class IllegalServantException extends LocalException
{
    public IllegalServantException()
    {
        this.reason = "";
    }

    public IllegalServantException(Throwable cause)
    {
        super(cause);
        this.reason = "";
    }

    public IllegalServantException(String reason)
    {
        this.reason = reason;
    }

    public IllegalServantException(String reason, Throwable cause)
    {
        super(cause);
        this.reason = reason;
    }

    public String ice_id()
    {
        return "::Ice::IllegalServantException";
    }

    /**
     * Describes why this servant is illegal.
     **/
    public String reason;

    /** @hidden */
    public static final long serialVersionUID = 1134807368810099935L;
}
