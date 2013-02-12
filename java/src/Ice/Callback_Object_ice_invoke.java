// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Callback object for {@link ObjectPrx#.begin_ice_invoke}.
 **/
public abstract class Callback_Object_ice_invoke extends TwowayCallback
{
    /**
     * The Ice run time calls <code>response</code> when an asynchronous operation invocation
     * completes successfully or raises a user exception.
     *
     * @param __ret Indicates the result of the invocation. If <code>true</code>, the operation
     * completed succesfully; if <code>false</code>, the operation raised a user exception.
     * @param outParams Contains the encoded out-parameters of the operation (if any) if <code>ok</code>
     * is <code>true</code>; otherwise, if <code>ok</code> is <code>false</code>, contains the
     * encoded user exception raised by the operation.
     **/
    public abstract void response(boolean __ret, byte[] outParams);

    public final void __completed(AsyncResult __result)
    {
        ByteSeqHolder outParams = new ByteSeqHolder();
        boolean __ret = false;
        try
        {
            __ret = __result.getProxy().end_ice_invoke(outParams, __result);
        }
        catch(LocalException __ex)
        {
            exception(__ex);
            return;
        }
        response(__ret, outParams.value);
    }
}
