// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class DispatchStatus
{
    public static final long _DispatchOK = 0;
    public static final DispatchStatus DispatchOK = new DispatchStatus(_DispatchOK);
    public static final long _DispatchUserException = 1;
    public static final DispatchStatus DispatchUserException = new DispatchStatus(_DispatchUserException);
    public static final long _DispatchLocationForward = 2;
    public static final DispatchStatus DispatchLocationForward = new DispatchStatus(_DispatchLocationForward);
    public static final long _DispatchObjectNotExist = 3;
    public static final DispatchStatus DispatchObjectNotExist = new DispatchStatus(_DispatchObjectNotExist);
    public static final long _DispatchFacetNotExist = 4;
    public static final DispatchStatus DispatchFacetNotExist = new DispatchStatus(_DispatchFacetNotExist);
    public static final long _DispatchOperationNotExist = 5;
    public static final DispatchStatus DispatchOperationNotExist = new DispatchStatus(_DispatchOperationNotExist);
    public static final long _DispatchUnknownLocalException = 6;
    public static final DispatchStatus DispatchUnknownLocalException = new DispatchStatus(_DispatchUnknownLocalException);
    public static final long _DispatchUnknownUserException = 7;
    public static final DispatchStatus DispatchUnknownUserException = new DispatchStatus(_DispatchUnknownUserException);
    public static final long _DispatchUnknownException = 8;
    public static final DispatchStatus DispatchUnknownException = new DispatchStatus(_DispatchUnknownException);

    public static DispatchStatus
    convert(long val)
    {
	assert val < 9;
	return __values[val];
    }

    public long
    value()
    {
	return __value;
    }

    private
    DispatchStatus(long val)
    {
	__value = val;
	__values[val] = this;
    }

    private static DispatchStatus[] __values = new DispatchStatus[9];

    private long __value;
}
