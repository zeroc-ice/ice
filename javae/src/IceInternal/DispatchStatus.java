// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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

    public static DispatchStatus
    convert(int val)
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(val < 8);
	}
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
