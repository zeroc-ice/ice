//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

import java.util.concurrent.CompletionStage;

import com.zeroc.IceInternal.Incoming;

/**
 * The base interface for servants.
 **/
public interface Object
{
    /** @hidden */
    final static String[] _iceIds =
    {
        "::Ice::Object"
    };

    /**
     * Holds the results of a call to <code>ice_invoke</code>.
     **/
    public class Ice_invokeResult
    {
        /**
         * Default initializes the members.
         **/
        public Ice_invokeResult()
        {
        }

        /**
         * One-shot constructor to initialize the members.
         *
         * @param returnValue True for a succesful invocation with any results encoded in <code>outParams</code>.
         * False if a user exception occurred with the exception encoded in <code>outParams</code>.
         * @param outParams The encoded results.
         **/
        public Ice_invokeResult(boolean returnValue, byte[] outParams)
        {
            this.returnValue = returnValue;
            this.outParams = outParams;
        }

        /**
         * If the operation completed successfully, the return value
         * is <code>true</code>. If the operation raises a user exception,
         * the return value is <code>false</code>; in this case, <code>outParams</code>
         * contains the encoded user exception. If the operation raises a run-time exception,
         * it throws it directly.
         **/
        public boolean returnValue;

        /**
         * The encoded out-paramaters and return value for the operation. The return value
         * follows any out-parameters.
         **/
        public byte[] outParams;
    }

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param s The type ID of the Slice interface to test against.
     * @param current The {@link Current} object for the invocation.
     * @return <code>true</code> if this object has the interface
     * specified by <code>s</code> or derives from the interface
     * specified by <code>s</code>.
     **/
    default boolean ice_isA(String s, Current current)
    {
        return java.util.Arrays.binarySearch(ice_ids(current), s) >= 0;
    }

    /**
     * Tests whether this object can be reached.
     *
     * @param current The {@link Current} object for the invocation.
     **/
    default void ice_ping(Current current)
    {
        // Nothing to do.
    }

    /**
     * Returns the Slice type IDs of the interfaces supported by this object.
     *
     * @param current The {@link Current} object for the invocation.
     * @return The Slice type IDs of the interfaces supported by this object, in base-to-derived
     * order. The first element of the returned array is always <code>::Ice::Object</code>.
     **/
    default String[] ice_ids(Current current)
    {
        return _iceIds;
    }

    /**
     * Returns the Slice type ID of the most-derived interface supported by this object.
     *
     * @param current The {@link Current} object for the invocation.
     * @return The Slice type ID of the most-derived interface.
     **/
    default String ice_id(Current current)
    {
        return ice_staticId();
    }

    /**
     * Returns the Slice type ID of the interface supported by this object.
     *
     * @return The return value is always ::Ice::Object.
     **/
    public static String ice_staticId()
    {
        return _iceIds[0];
    }

    /** @hidden */
    final static String[] _iceOps =
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
     * @return A completion stage if the dispatched asynchronously, null otherwise.
     * @throws UserException A user exception that propagates out of this method will be marshaled as the result.
     *
     * @see DispatchInterceptor
     **/
    default CompletionStage<OutputStream> ice_dispatch(Request request)
        throws UserException
    {
        Incoming in = (Incoming)request;
        in.startOver();
        return _iceDispatch(in, in.getCurrent());
    }

    /**
     * @hidden
     * @param in -
     * @param current -
     * @return -
     * @throws UserException -
     **/
    default CompletionStage<OutputStream> _iceDispatch(Incoming in, Current current)
        throws UserException
    {
        int pos = java.util.Arrays.binarySearch(_iceOps, current.operation);
        if(pos < 0)
        {
            throw new OperationNotExistException(current.id, current.facet, current.operation);
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
        throw new OperationNotExistException(current.id, current.facet, current.operation);
    }

    /**
     * @hidden
     * @param ostr -
     **/
    default void _iceWrite(OutputStream ostr)
    {
         ostr.startValue(null);
         _iceWriteImpl(ostr);
         ostr.endValue();
    }

    /**
     * @hidden
     * @param ostr -
     **/
    default void _iceWriteImpl(OutputStream ostr)
    {
    }

    /**
     * @hidden
     * @param istr -
     **/
    default void _iceRead(InputStream istr)
    {
         istr.startValue();
         _iceReadImpl(istr);
         istr.endValue(false);
    }

    /**
     * @hidden
     * @param istr -
     **/
    default void _iceReadImpl(InputStream istr)
    {
    }

    /**
     * @hidden
     * @param obj -
     * @param inS -
     * @param current -
     * @return -
     **/
    static CompletionStage<OutputStream> _iceD_ice_isA(Object obj, Incoming inS, Current current)
    {
        InputStream istr = inS.startReadParams();
        String id = istr.readString();
        inS.endReadParams();
        boolean ret = obj.ice_isA(id, current);
        OutputStream ostr = inS.startWriteParams();
        ostr.writeBool(ret);
        inS.endWriteParams(ostr);
        return inS.setResult(ostr);
    }

    /**
     * @hidden
     * @param obj -
     * @param inS -
     * @param current -
     * @return -
     **/
    static CompletionStage<OutputStream> _iceD_ice_ping(Object obj, Incoming inS, Current current)
    {
        inS.readEmptyParams();
        obj.ice_ping(current);
        return inS.setResult(inS.writeEmptyParams());
    }

    /**
     * @hidden
     * @param obj -
     * @param inS -
     * @param current -
     * @return -
     **/
    static CompletionStage<OutputStream> _iceD_ice_ids(Object obj, Incoming inS, Current current)
    {
        inS.readEmptyParams();
        String[] ret = obj.ice_ids(current);
        OutputStream ostr = inS.startWriteParams();
        ostr.writeStringSeq(ret);
        inS.endWriteParams(ostr);
        return inS.setResult(ostr);
    }

    /**
     * @hidden
     * @param obj -
     * @param inS -
     * @param current -
     * @return -
     **/
    static CompletionStage<OutputStream> _iceD_ice_id(Object obj, Incoming inS, Current current)
    {
        inS.readEmptyParams();
        String ret = obj.ice_id(current);
        OutputStream ostr = inS.startWriteParams();
        ostr.writeString(ret);
        inS.endWriteParams(ostr);
        return inS.setResult(ostr);
    }

    /**
     * @hidden
     * @param mode -
     * @return -
     **/
    static String _iceOperationModeToString(OperationMode mode)
    {
        if(mode == OperationMode.Normal)
        {
            return "::Ice::Normal";
        }
        if(mode == OperationMode.Nonmutating)
        {
            return "::Ice::Nonmutating";
        }

        if(mode == OperationMode.Idempotent)
        {
            return "::Ice::Idempotent";
        }

        return "???";
    }

    /**
     * @hidden
     * @param expected -
     * @param received -
     **/
    static void _iceCheckMode(OperationMode expected, OperationMode received)
    {
        if(expected == null)
        {
            expected = OperationMode.Normal;
        }

        if(expected != received)
        {
            if(expected == OperationMode.Idempotent && received == OperationMode.Nonmutating)
            {
                //
                // Fine: typically an old client still using the
                // deprecated nonmutating keyword
                //
            }
            else
            {
                MarshalException ex = new MarshalException();
                ex.reason = "unexpected operation mode. expected = "
                    + _iceOperationModeToString(expected) + " received = "
                    + _iceOperationModeToString(received);
                throw ex;
            }
        }
    }
}
