// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Base class for all Slice classes.
 **/
public abstract class ObjectImpl implements Object, java.lang.Cloneable, java.io.Serializable
{
    /**
     * Instantiates an Ice object.
     **/
    public
    ObjectImpl()
    {
    }

    /**
     * Returns a copy of the object. The cloned object contains field-for-field copies
     * of the state.
     *
     * @return The cloned object.
     **/
    @Override
    public ObjectImpl
    clone()
    {
	ObjectImpl c = null;

	try
	{
	   c = (ObjectImpl)super.clone();
	}
	catch(CloneNotSupportedException ex)
	{
	    assert false;
	}
	return c;
    }

    public final static String[] __ids =
    {
        "::Ice::Object"
    };

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param s The type ID of the Slice interface to test against.
     * @return The return value is <code>true</code> if <code>s</code> is
     * <code>::Ice::Object</code>.
     **/
    @Override
    public boolean
    ice_isA(String s)
    {
        return s.equals(__ids[0]);
    }

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param s The type ID of the Slice interface to test against.
     * @param current The current object for the invocation.
     * @return The return value is <code>true</code> if <code>s</code> is
     * <code>::Ice::Object</code>.
     **/
    @Override
    public boolean
    ice_isA(String s, Current current)
    {
        return s.equals(__ids[0]);
    }

    public static DispatchStatus
    ___ice_isA(Ice.Object __obj, IceInternal.Incoming __inS, Current __current)
    {
        IceInternal.BasicStream __is = __inS.startReadParams();
        String __id = __is.readString();
        __inS.endReadParams();
        boolean __ret = __obj.ice_isA(__id, __current);
        IceInternal.BasicStream __os = __inS.__startWriteParams(Ice.FormatType.DefaultFormat);
        __os.writeBool(__ret);
        __inS.__endWriteParams(true);
        return DispatchStatus.DispatchOK;
    }

    /**
     * Tests whether this object can be reached.
     **/
    @Override
    public void
    ice_ping()
    {
        // Nothing to do.
    }

    /**
     * Tests whether this object can be reached.
     *
     * @param current The current object for the invocation.
     **/
    @Override
    public void
    ice_ping(Current current)
    {
        // Nothing to do.
    }

    public static DispatchStatus
    ___ice_ping(Ice.Object __obj, IceInternal.Incoming __inS, Current __current)
    {
        __inS.readEmptyParams();
        __obj.ice_ping(__current);
        __inS.__writeEmptyParams();
        return DispatchStatus.DispatchOK;
    }

    /**
     * Returns the Slice type IDs of the interfaces supported by this object.
     *
     * @return An array whose only element is <code>::Ice::Object</code>.
     **/
    @Override
    public String[]
    ice_ids()
    {
        return __ids;
    }

    /**
     * Returns the Slice type IDs of the interfaces supported by this object.
     *
     * @param current The current object for the invocation.
     * @return An array whose only element is <code>::Ice::Object</code>.
     **/
    @Override
    public String[]
    ice_ids(Current current)
    {
        return __ids;
    }

    public static DispatchStatus
    ___ice_ids(Ice.Object __obj, IceInternal.Incoming __inS, Current __current)
    {
        __inS.readEmptyParams();
        String[] __ret = __obj.ice_ids(__current);
        IceInternal.BasicStream __os = __inS.__startWriteParams(Ice.FormatType.DefaultFormat);
        __os.writeStringSeq(__ret);
        __inS.__endWriteParams(true);
        return DispatchStatus.DispatchOK;
    }

    /**
     * Returns the Slice type ID of the most-derived interface supported by this object.
     *
     * @return The return value is always <code>::Ice::Object</code>.
     **/
    @Override
    public String
    ice_id()
    {
        return __ids[0];
    }

    /**
     * Returns the Slice type ID of the most-derived interface supported by this object.
     *
     * @param current The current object for the invocation.
     * @return A Slice type ID.
     **/
    @Override
    public String
    ice_id(Current current)
    {
        return __ids[0];
    }

    public static DispatchStatus
    ___ice_id(Ice.Object __obj, IceInternal.Incoming __inS, Current __current)
    {
        __inS.readEmptyParams();
        String __ret = __obj.ice_id(__current);
        IceInternal.BasicStream __os = __inS.__startWriteParams(Ice.FormatType.DefaultFormat);
        __os.writeString(__ret);
        __inS.__endWriteParams(true);
        return DispatchStatus.DispatchOK;
    }

    /**
     * Returns the Slice type ID of the interface supported by this object.
     *
     * @return The return value is always ::Ice::Object.
     **/
    public static String
    ice_staticId()
    {
        return __ids[0];
    }

    /**
     * Returns the Freeze metadata attributes for an operation.
     *
     * @param operation The name of the operation.
     * @return The least significant bit indicates whether the operation is a read
     * or write operation. If the bit is set, the operation is a write operation.
     * The expression <code>ice_operationAttributes("op") &amp; 0x1</code> is true if
     * the operation has a <code>["freeze:write"]</code> metadata directive.
     * <p>
     * The second and third least significant bit indicate the transactional mode
     * of the operation. The expression <code>ice_operationAttributes("op") &amp; 0x6 &gt;&gt; 1</code>
     * indicates the transactional mode as follows:
     * <dl>
     *   <dt>0</dt>
     *   <dd><code>["freeze:read:supports"]</code></dd>
     *   <dt>1</dt>
     *   <dd><code>["freeze:read:mandatory"]</code> or <code>["freeze:write:mandatory"]</code></dd>
     *   <dt>2</dt>
     *   <dd><code>["freeze:read:required"]</code> or <code>["freeze:write:required"]</code></dd>
     *   <dt>3</dt>
     *   <dd><code>["freeze:read:never"]</code></dd>
     * </dl>
     **/
    @Override
    public int ice_operationAttributes(String operation)
    {
        return 0;
    }

    /**
     * The Ice run time invokes this method prior to marshaling an object's data members. This allows a subclass
     * to override this method in order to validate its data members. This default implementation does nothing.
     **/
    @Override
    public void
    ice_preMarshal()
    {
    }

    /**
     * This Ice run time invokes this method vafter unmarshaling an object's data members. This allows a
     * subclass to override this method in order to perform additional initialization. This default
     * implementation does nothing.
     **/
    @Override
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

    /**
     * Dispatches an invocation to a servant. This method is used by dispatch interceptors to forward an invocation
     * to a servant (or to another interceptor).
     *
     * @param request The details of the invocation.
     * @param cb The callback object for asynchchronous dispatch. For synchronous dispatch, the callback object must
     * be <code>null</code>.
     * @return The dispatch status for the operation.
     *
     * @see DispatchInterceptor
     * @see DispatchInterceptorAsyncCallback
     * @see DispatchStatus
     **/
    @Override
    public DispatchStatus
    ice_dispatch(Request request, DispatchInterceptorAsyncCallback cb)
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

    /**
     * Dispatches an invocation to a servant. This method is used by dispatch interceptors to forward an invocation
     * to a servant (or to another interceptor).
     *
     * @param request The details of the invocation.
     * @return The dispatch status for the operation.
     *
     * @see DispatchInterceptor
     * @see DispatchStatus
     **/
    @Override
    public DispatchStatus
    ice_dispatch(Request request)
    {
        return ice_dispatch(request, null);
    }

    @Override
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

    @Override
    public void
    __write(IceInternal.BasicStream os)
    {
         os.startWriteObject(null);
         __writeImpl(os);
         os.endWriteObject();
    }

    @Override
    public void
    __read(IceInternal.BasicStream is)
    {
         is.startReadObject();
         __readImpl(is);
         is.endReadObject(false);
    }

    @Override
    public void
    __write(OutputStream os)
    {
        os.startObject(null);
        __writeImpl(os);
        os.endObject();
    }

    @Override
    public void
    __read(InputStream is)
    {
        is.startObject();
        __readImpl(is);
        is.endObject(false);
    }

    protected void
    __writeImpl(IceInternal.BasicStream os)
    {
    }

    protected void
    __readImpl(IceInternal.BasicStream is)
    {
    }

     protected void
    __writeImpl(OutputStream os)
    {
        throw new MarshalException("class was not generated with stream support");
    }

    protected void
    __readImpl(InputStream is)
    {
        throw new MarshalException("class was not generated with stream support");
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

    public static final long serialVersionUID = 0L;
}
