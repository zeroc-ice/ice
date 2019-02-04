//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

    private final static String[] _ids =
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
        return s.equals(_ids[0]);
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
        return s.equals(_ids[0]);
    }

    public static boolean
    _iceD_ice_isA(Ice.Object obj, IceInternal.Incoming inS, Current current)
    {
        InputStream istr = inS.startReadParams();
        String id = istr.readString();
        inS.endReadParams();
        boolean ret = obj.ice_isA(id, current);
        OutputStream ostr = inS.startWriteParams();
        ostr.writeBool(ret);
        inS.endWriteParams();
        return true;
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

    public static boolean
    _iceD_ice_ping(Ice.Object obj, IceInternal.Incoming inS, Current current)
    {
        inS.readEmptyParams();
        obj.ice_ping(current);
        inS.writeEmptyParams();
        return true;
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
        return _ids;
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
        return _ids;
    }

    public static boolean
    _iceD_ice_ids(Ice.Object obj, IceInternal.Incoming inS, Current current)
    {
        inS.readEmptyParams();
        String[] ret = obj.ice_ids(current);
        OutputStream ostr = inS.startWriteParams();
        ostr.writeStringSeq(ret);
        inS.endWriteParams();
        return true;
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
        return _ids[0];
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
        return _ids[0];
    }

    public static boolean
    _iceD_ice_id(Ice.Object obj, IceInternal.Incoming inS, Current current)
    {
        inS.readEmptyParams();
        String ret = obj.ice_id(current);
        OutputStream ostr = inS.startWriteParams();
        ostr.writeString(ret);
        inS.endWriteParams();
        return true;
    }

    /**
     * Returns the Slice type ID of the interface supported by this object.
     *
     * @return The return value is always ::Ice::Object.
     **/
    public static String
    ice_staticId()
    {
        return _ids[0];
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
     *
     * Refer to the Freeze manual for more information on the TransactionalEvictor.
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

    /**
     * Returns the sliced data if the value has a preserved-slice base class and has been sliced during
     * un-marshaling of the value, null is returned otherwise.
     *
     * @return The sliced data or null.
     **/
    @Override
    public SlicedData
    ice_getSlicedData()
    {
        return null;
    }

    private final static String[] _all =
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
     * @return True for asynchronous dispatch, false otherwise
     *
     * @see DispatchInterceptor
     * @see DispatchInterceptorAsyncCallback
     **/
    @Override
    public boolean
    ice_dispatch(Request request, DispatchInterceptorAsyncCallback cb)
        throws Ice.UserException
    {
        IceInternal.Incoming in = (IceInternal.Incoming)request;
        in.startOver(); // may raise ResponseSentException
        if(cb != null)
        {
            in.push(cb);
            try
            {
                return _iceDispatch(in, in.getCurrent());
            }
            finally
            {
                in.pop();
            }
        }
        else
        {
            return _iceDispatch(in, in.getCurrent());
        }
    }

    /**
     * Dispatches an invocation to a servant. This method is used by dispatch interceptors to forward an invocation
     * to a servant (or to another interceptor).
     *
     * @param request The details of the invocation.
     * @return True for asynchronous dispatch, false otherwise.
     *
     * @see DispatchInterceptor
     **/
    @Override
    public boolean
    ice_dispatch(Request request)
        throws Ice.UserException
    {
        return ice_dispatch(request, null);
    }

    @Override
    public boolean
    _iceDispatch(IceInternal.Incoming in, Current current)
        throws Ice.UserException
    {
        int pos = java.util.Arrays.binarySearch(_all, current.operation);
        if(pos < 0)
        {
            throw new Ice.OperationNotExistException(current.id, current.facet, current.operation);
        }

        switch(pos)
        {
            case 0:
            {
                return _iceD_ice_id(this, in, current);
            }
            case 1:
            {
                return _iceD_ice_ids(this, in, current);
            }
            case 2:
            {
                return _iceD_ice_isA(this, in, current);
            }
            case 3:
            {
                return _iceD_ice_ping(this, in, current);
            }
        }

        assert(false);
        throw new Ice.OperationNotExistException(current.id, current.facet, current.operation);
    }

    @Override
    public void
    _iceWrite(OutputStream os)
    {
         os.startValue(null);
         _iceWriteImpl(os);
         os.endValue();
    }

    @Override
    public void
    _iceRead(InputStream is)
    {
         is.startValue();
         _iceReadImpl(is);
         is.endValue(false);
    }

    protected void
    _iceWriteImpl(OutputStream os)
    {
    }

    protected void
    _iceReadImpl(InputStream is)
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
    _iceCheckMode(OperationMode expected, OperationMode received)
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
