//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public class RetryException extends Exception
{
    public
    RetryException(Ice.LocalException ex)
    {
        _ex = ex;
    }

    public Ice.LocalException
    get()
    {
        return _ex;
    }

    private Ice.LocalException _ex;
}
