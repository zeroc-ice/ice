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
    public static final int _DispatchOK = 0;
    public static final DispatchStatus DispatchOK = new DispatchStatus(_DispatchOK);
    public static final int _DispatchUserException = 1;
    public static final DispatchStatus DispatchUserException = new DispatchStatus(_DispatchUserException);
    public static final int _DispatchLocationForward = 2;
    public static final DispatchStatus DispatchLocationForward = new DispatchStatus(_DispatchLocationForward);
    public static final int _DispatchProxyRequested = 3;
    public static final DispatchStatus DispatchProxyRequested = new DispatchStatus(_DispatchProxyRequested);
    public static final int _DispatchObjectNotExist = 4;
    public static final DispatchStatus DispatchObjectNotExist = new DispatchStatus(_DispatchObjectNotExist);
    public static final int _DispatchFacetNotExist = 5;
    public static final DispatchStatus DispatchFacetNotExist = new DispatchStatus(_DispatchFacetNotExist);
    public static final int _DispatchOperationNotExist = 6;
    public static final DispatchStatus DispatchOperationNotExist = new DispatchStatus(_DispatchOperationNotExist);
    public static final int _DispatchUnknownLocalException = 7;
    public static final DispatchStatus DispatchUnknownLocalException = new DispatchStatus(_DispatchUnknownLocalException);
    public static final int _DispatchUnknownUserException = 8;
    public static final DispatchStatus DispatchUnknownUserException = new DispatchStatus(_DispatchUnknownUserException);
    public static final int _DispatchUnknownException = 9;
    public static final DispatchStatus DispatchUnknownException = new DispatchStatus(_DispatchUnknownException);

    public static DispatchStatus
    convert(int val)
    {
        assert val < 10;
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

    private static DispatchStatus[] __values = new DispatchStatus[10];

    private int __value;
}
