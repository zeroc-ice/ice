// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class DispatchStatus
{
    private static DispatchStatus[] __values = new DispatchStatus[2];
    private int __value;

    public static final int _DispatchOK = 0;
    public static final DispatchStatus DispatchOK =
        new DispatchStatus(_DispatchOK);

    public static final int _DispatchUserException = 1;
    public static final DispatchStatus DispatchUserException =
        new DispatchStatus(_DispatchUserException);

    public static DispatchStatus
    convert(int val)
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(val < 2);
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
