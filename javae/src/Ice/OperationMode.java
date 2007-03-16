// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 2.1.0

package Ice;

public final class OperationMode
{
    private static OperationMode[] __values = new OperationMode[3];
    private int __value;

    public static final int _Normal = 0;
    public static final OperationMode Normal = new OperationMode(_Normal);
    public static final int _Nonmutating = 1;
    public static final OperationMode Nonmutating = new OperationMode(_Nonmutating);
    public static final int _Idempotent = 2;
    public static final OperationMode Idempotent = new OperationMode(_Idempotent);

    public static OperationMode
    convert(int val)
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(val < 3);
	}
	return __values[val];
    }

    public int
    value()
    {
	return __value;
    }

    public String
    toString()
    {
	return __T[__value];
    }

    private
    OperationMode(int val)
    {
	__value = val;
	__values[val] = this;
    }

    public void
    __write(IceInternal.BasicStream __os)
    {
	__os.writeByte((byte)__value);
    }

    public static OperationMode
    __read(IceInternal.BasicStream __is)
    {
	int __v = __is.readByte();
	if(__v < 0 || __v >= 3)
	{
	    throw new Ice.MarshalException();
	}
	return OperationMode.convert(__v);
    }

    final static private String[] __T =
    {
	"Normal",
	"Nonmutating",
	"Idempotent"
    };
}
