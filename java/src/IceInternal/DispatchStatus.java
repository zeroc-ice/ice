// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class DispatchStatus
{
    private static DispatchStatus[] __values = new DispatchStatus[9];
    private int __value;

    public static final int _DispatchOK = 0;
    public static final DispatchStatus DispatchOK =
        new DispatchStatus(_DispatchOK);

    public static final int _DispatchUserException = 1;
    public static final DispatchStatus DispatchUserException =
        new DispatchStatus(_DispatchUserException);

    public static final int _DispatchObjectNotExist = 2;
    public static final DispatchStatus DispatchObjectNotExist =
        new DispatchStatus(_DispatchObjectNotExist);

    public static final int _DispatchFacetNotExist = 3;
    public static final DispatchStatus DispatchFacetNotExist =
        new DispatchStatus(_DispatchFacetNotExist);

    public static final int _DispatchOperationNotExist = 4;
    public static final DispatchStatus DispatchOperationNotExist =
        new DispatchStatus(_DispatchOperationNotExist);

    public static final int _DispatchUnknownLocalException = 5;
    public static final DispatchStatus DispatchUnknownLocalException =
        new DispatchStatus(_DispatchUnknownLocalException);

    public static final int _DispatchUnknownUserException = 6;
    public static final DispatchStatus DispatchUnknownUserException =
        new DispatchStatus(_DispatchUnknownUserException);

    public static final int _DispatchUnknownException = 7;
    public static final DispatchStatus DispatchUnknownException =
        new DispatchStatus(_DispatchUnknownException);

    //
    // "Pseudo dispatch status", used internally only to indicate
    // async dispatch.
    //
    public static final int _DispatchAsync = 8;
    public static final DispatchStatus DispatchAsync =
        new DispatchStatus(_DispatchAsync);

    public static DispatchStatus
    convert(int val)
    {
        assert val < 9;
        return __values[val];
    }

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
