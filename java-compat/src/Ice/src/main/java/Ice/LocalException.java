//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * Base class for all Ice run-time exceptions.
 * System exceptions are currently Ice internal, non-documented
 * exceptions.
 **/
public abstract class LocalException extends Exception
{
    public LocalException()
    {
    }

    public LocalException(Throwable cause)
    {
        super(cause);
    }
}
