//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * Base class for all Ice system exceptions.
 **/
public abstract class SystemException extends Exception
{
    public SystemException()
    {
    }

    public SystemException(Throwable cause)
    {
        super(cause);
    }
}
