//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.interceptor;

public class RetryException extends com.zeroc.Ice.LocalException
{
    public RetryException()
    {
    }

    public RetryException(Throwable cause)
    {
        super(cause);
    }

    public String ice_id()
    {
        return "::Test::RetryException";
    }

    /** @hidden */
    public static final long serialVersionUID = 2178002139837551118L;
}
