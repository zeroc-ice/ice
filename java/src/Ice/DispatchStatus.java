// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Indicates the status of operation dispatch.
 *
 * @see DispatchInterceptor
 **/
public final class DispatchStatus
{
    private static DispatchStatus[] __values = new DispatchStatus[3];
    private int __value;

    public static final int _DispatchOK = 0;

    /**
     * Indicates that an operation was dispatched synchronously and successfully.
     **/
    public static final DispatchStatus DispatchOK =
        new DispatchStatus(_DispatchOK);

    public static final int _DispatchUserException = 1;

    /**
     * Indicates that an operation was dispatched synchronously and raised a user exception.
     **/
    public static final DispatchStatus DispatchUserException =
        new DispatchStatus(_DispatchUserException);

    public static final int _DispatchAsync = 2;

    /**
     * Indicates that an operation was dispatched asynchronously.
     **/
    public static final DispatchStatus DispatchAsync =
        new DispatchStatus(_DispatchAsync);

    /**
     * Converts an ordinal value to the corresponding enumerator.
     **/
    public static DispatchStatus
    convert(int val)
    {
        assert val < 3;
        return __values[val];
    }

    /**
     * Returns the ordinal value of the current enumerator.
     **/
    public int
    value()
    {
        return __value;
    }

    private
    DispatchStatus(int val)
    {
        __value = val;
        __values[val] = this;
    }
}
