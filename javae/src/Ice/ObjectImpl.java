// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class ObjectImpl implements Object
{
    public
    ObjectImpl()
    {
    }

    public java.lang.Object
    ice_clone()
        throws IceUtil.CloneException
    {
	try
	{
	    ObjectImpl obj = (ObjectImpl)getClass().newInstance();
	    obj.__copyFrom(this);
	    return obj;
	}
	catch(java.lang.IllegalAccessException ex)
	{
	    throw new IceUtil.CloneException(ex.getMessage());
	}
	catch(java.lang.InstantiationException ex)
	{
	    throw new IceUtil.CloneException(ex.getMessage());
	}
    }

    public int
    ice_hash()
    {
        return hashCode();
    }

    public final static String[] __ids =
    {
        "::Ice::Object"
    };

    public boolean
    ice_isA(String s)
    {
        return s.equals(__ids[0]);
    }

    public boolean
    ice_isA(String s, Current current)
    {
        return s.equals(__ids[0]);
    }

    public static IceInternal.DispatchStatus
    ___ice_isA(Ice.Object __obj, IceInternal.Incoming __inS, Current __current)
    {
        IceInternal.BasicStream __is = __inS.is();
        IceInternal.BasicStream __os = __inS.os();
        String __id = __is.readString();
        boolean __ret = __obj.ice_isA(__id, __current);
        __os.writeBool(__ret);
        return IceInternal.DispatchStatus.DispatchOK;
    }

    public void
    ice_ping()
    {
        // Nothing to do.
    }

    public void
    ice_ping(Current current)
    {
        // Nothing to do.
    }

    public static IceInternal.DispatchStatus
    ___ice_ping(Ice.Object __obj, IceInternal.Incoming __inS, Current __current)
    {
        __obj.ice_ping(__current);
        return IceInternal.DispatchStatus.DispatchOK;
    }

    public String[]
    ice_ids()
    {
        return __ids;
    }

    public String[]
    ice_ids(Current current)
    {
        return __ids;
    }

    public static IceInternal.DispatchStatus
    ___ice_ids(Ice.Object __obj, IceInternal.Incoming __inS, Current __current)
    {
        IceInternal.BasicStream __os = __inS.os();
        String[] __ret = __obj.ice_ids(__current);
        __os.writeStringSeq(__ret);
        return IceInternal.DispatchStatus.DispatchOK;
    }

    public String
    ice_id()
    {
        return __ids[0];
    }

    public String
    ice_id(Current current)
    {
        return __ids[0];
    }

    public static IceInternal.DispatchStatus
    ___ice_id(Ice.Object __obj, IceInternal.Incoming __inS, Current __current)
    {
        IceInternal.BasicStream __os = __inS.os();
        String __ret = __obj.ice_id(__current);
        __os.writeString(__ret);
        return IceInternal.DispatchStatus.DispatchOK;
    }

    public static String
    ice_staticId()
    {
        return __ids[0];
    }

    private final static String[] __all =
    {
        "ice_id",
        "ice_ids",
        "ice_isA",
        "ice_ping"
    };

    public IceInternal.DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        int pos = IceUtil.Arrays.search(__all, current.operation);
        if(pos < 0)
        {
            return IceInternal.DispatchStatus.DispatchOperationNotExist;
        }

        switch(pos)
        {
            case 0:
            {
                return ___ice_id(this, in, current);
            }
            case 1:
            {
                return ___ice_ids(this, in, current);
            }
            case 2:
            {
                return ___ice_isA(this, in, current);
            }
            case 3:
            {
                return ___ice_ping(this, in, current);
            }
        }

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(false);
	}
        return IceInternal.DispatchStatus.DispatchOperationNotExist;
    }

    protected void
    __copyFrom(java.lang.Object obj)
    {
    }

    private static String
    operationModeToString(OperationMode mode)
    {
	if(mode == Ice.OperationMode.Normal)
	{
	    return "::Ice::Normal";
	}
	if(mode == Ice.OperationMode.Nonmutating)
	{
	    return "::Ice::Nonmutating";
	}

	if(mode == Ice.OperationMode.Idempotent)
	{
	    return "::Ice::Idempotent";
	}

	return "???";
    }

    protected static void
    __checkMode(OperationMode expected, OperationMode received)
    {
	if(expected != received)
	{
	    Ice.MarshalException ex = new Ice.MarshalException();
	    ex.reason = "unexpected operation mode. expected = "
			+ operationModeToString(expected) + " received = "
			+ operationModeToString(received);
	    throw ex;
	}
    }
}
