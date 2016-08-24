// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

import java.util.concurrent.CompletionStage;

import com.zeroc.IceInternal.Incoming;

/**
 * The base interface for servants.
 **/
public interface Object
{
    public static final String ice_staticId = "::Ice::Object";

    public final static String[] __ids =
    {
        ice_staticId
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
        return __ids;
    }

    /**
     * Returns the Slice type ID of the most-derived interface supported by this object.
     *
     * @param current The {@link Current} object for the invocation.
     * @return The Slice type ID of the most-derived interface.
     **/
    default String ice_id(Current current)
    {
        return __ids[0];
    }

    /**
     * Returns the Slice type ID of the interface supported by this object.
     *
     * @return The return value is always ::Ice::Object.
     **/
    public static String ice_staticId()
    {
        return ice_staticId;
    }

    /**
     * Returns the Freeze metadata attributes for an operation.
     *
     * @param operation The name of the operation.
     * @return The least significant bit indicates whether the operation is a read
     * or write operation. If the bit is set, the operation is a write operation.
     * The expression <code>ice_operationAttributes("op") & 0x1</code> is true if
     * the operation has a <code>["freeze:write"]</code> metadata directive.
     * <p>
     * The second- and third least significant bit indicate the transactional mode
     * of the operation. The expression <code>ice_operationAttributes("op") & 0x6 >> 1</code>
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
    default int ice_operationAttributes(String operation)
    {
        return 0;
    }

    final static String[] __ops =
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
     *
     * @see DispatchInterceptor
     **/
    default CompletionStage<OutputStream> ice_dispatch(Request request)
        throws UserException
    {
        Incoming in = (Incoming)request;
        in.startOver();
        return __dispatch(in, in.getCurrent());
    }

    default CompletionStage<OutputStream> __dispatch(Incoming in, Current current)
        throws UserException
    {
        int pos = java.util.Arrays.binarySearch(__ops, current.operation);
        if(pos < 0)
        {
            throw new OperationNotExistException(current.id, current.facet, current.operation);
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
        throw new OperationNotExistException(current.id, current.facet, current.operation);
    }

    default void __write(OutputStream __os)
    {
         __os.startValue(null);
         __writeImpl(__os);
         __os.endValue();
    }

    default void __writeImpl(OutputStream __os)
    {
    }

    default void __read(InputStream __is)
    {
         __is.startValue();
         __readImpl(__is);
         __is.endValue(false);
    }

    default void __readImpl(InputStream __is)
    {
    }

    static CompletionStage<OutputStream> ___ice_isA(Object __obj, Incoming __inS, Current __current)
    {
        InputStream __is = __inS.startReadParams();
        String __id = __is.readString();
        __inS.endReadParams();
        boolean __ret = __obj.ice_isA(__id, __current);
        OutputStream __os = __inS.startWriteParams();
        __os.writeBool(__ret);
        __inS.endWriteParams(__os);
        return __inS.setResult(__os);
    }

    static CompletionStage<OutputStream> ___ice_ping(Object __obj, Incoming __inS, Current __current)
    {
        __inS.readEmptyParams();
        __obj.ice_ping(__current);
        return __inS.setResult(__inS.writeEmptyParams());
    }

    static CompletionStage<OutputStream> ___ice_ids(Object __obj, Incoming __inS, Current __current)
    {
        __inS.readEmptyParams();
        String[] __ret = __obj.ice_ids(__current);
        OutputStream __os = __inS.startWriteParams();
        __os.writeStringSeq(__ret);
        __inS.endWriteParams(__os);
        return __inS.setResult(__os);
    }

    static CompletionStage<OutputStream> ___ice_id(Object __obj, Incoming __inS, Current __current)
    {
        __inS.readEmptyParams();
        String __ret = __obj.ice_id(__current);
        OutputStream __os = __inS.startWriteParams();
        __os.writeString(__ret);
        __inS.endWriteParams(__os);
        return __inS.setResult(__os);
    }

    static String __operationModeToString(OperationMode mode)
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

    static void __checkMode(OperationMode expected, OperationMode received)
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
                    + __operationModeToString(expected) + " received = "
                    + __operationModeToString(received);
                throw ex;
            }
        }
    }
}
