// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class ObjectImpl implements Object, java.lang.Cloneable
{
    public
    ObjectImpl()
    {
    }

    public java.lang.Object
    clone()
    {
        java.lang.Object o = null;
        try
        {
            o = super.clone();
        }
        catch(java.lang.CloneNotSupportedException ex)
        {
            assert false; // Impossible
        }
        return o;
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

    public static DispatchStatus
    ___ice_isA(Ice.Object __obj, IceInternal.Incoming __inS, Current __current)
    {
        IceInternal.BasicStream __is = __inS.is();
        IceInternal.BasicStream __os = __inS.os();
        String __id = __is.readString();
        boolean __ret = __obj.ice_isA(__id, __current);
        __os.writeBool(__ret);
        return DispatchStatus.DispatchOK;
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

    public static DispatchStatus
    ___ice_ping(Ice.Object __obj, IceInternal.Incoming __inS, Current __current)
    {
        __obj.ice_ping(__current);
        return DispatchStatus.DispatchOK;
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

    public static DispatchStatus
    ___ice_ids(Ice.Object __obj, IceInternal.Incoming __inS, Current __current)
    {
        IceInternal.BasicStream __os = __inS.os();
        String[] __ret = __obj.ice_ids(__current);
        __os.writeStringSeq(__ret);
        return DispatchStatus.DispatchOK;
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

    public static DispatchStatus
    ___ice_id(Ice.Object __obj, IceInternal.Incoming __inS, Current __current)
    {
        IceInternal.BasicStream __os = __inS.os();
        String __ret = __obj.ice_id(__current);
        __os.writeString(__ret);
        return DispatchStatus.DispatchOK;
    }

    public static String
    ice_staticId()
    {
        return __ids[0];
    }

    public int ice_operationAttributes(String operation)
    {
        return 0;
    }

    public void
    ice_preMarshal()
    {
    }

    public void
    ice_postUnmarshal()
    {
    }

    private final static String[] __all =
    {
        "ice_id",
        "ice_ids",
        "ice_isA",
        "ice_ping"
    };

    public DispatchStatus
    ice_dispatch(Request request, DispatchInterceptorAsyncCallback cb)
    {
        if(request.isCollocated())
        {
            return __collocDispatch((IceInternal.Direct)request);
        }
        else
        {
            IceInternal.Incoming in = (IceInternal.Incoming)request;
            if(cb != null)
            {
                in.push(cb);
            }
            try
            {
                in.startOver(); // may raise ResponseSentException
                return __dispatch(in, in.getCurrent());
            }
            finally
            {
                if(cb != null)
                {
                    in.pop();
                }
            }
        }
    }

    public DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        int pos = java.util.Arrays.binarySearch(__all, current.operation);
        if(pos < 0)
        {
            throw new Ice.OperationNotExistException(current.id, current.facet, current.operation);
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

        assert(false);
        throw new Ice.OperationNotExistException(current.id, current.facet, current.operation);
    }
    
    public DispatchStatus
    __collocDispatch(IceInternal.Direct request)
    {
        return request.run(this);
    }
    

    public void
    __write(IceInternal.BasicStream __os)
    {
        __os.writeTypeId(ice_staticId());
        __os.startWriteSlice();
        __os.writeSize(0); // For compatibility with the old AFM.
        __os.endWriteSlice();
    }

    public void
    __read(IceInternal.BasicStream __is, boolean __rid)
    {
        if(__rid)
        {
            String myId = __is.readTypeId();
        }

        __is.startReadSlice();

        // For compatibility with the old AFM.
        int sz = __is.readSize();
        if(sz != 0)
        {
            throw new MarshalException();
        }

        __is.endReadSlice();
    }

    public void
    __write(Ice.OutputStream __outS)
    {
        __outS.writeTypeId(ice_staticId());
        __outS.startSlice();
        __outS.writeSize(0); // For compatibility with the old AFM.
        __outS.endSlice();
    }

    public void
    __read(Ice.InputStream __inS, boolean __rid)
    {
        if(__rid)
        {
            String myId = __inS.readTypeId();
        }

        __inS.startSlice();

        // For compatibility with the old AFM.
        int sz = __inS.readSize();
        if(sz != 0)
        {
            throw new MarshalException();
        }

        __inS.endSlice();
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
            if(expected == Ice.OperationMode.Idempotent 
               && received == Ice.OperationMode.Nonmutating)
            {
                //
                // Fine: typically an old client still using the 
                // deprecated nonmutating keyword
                //
            }
            else
            {
                Ice.MarshalException ex = new Ice.MarshalException();
                ex.reason = "unexpected operation mode. expected = "
                    + operationModeToString(expected) + " received = "
                    + operationModeToString(received);
                throw ex;
            }
        }
    }
}
