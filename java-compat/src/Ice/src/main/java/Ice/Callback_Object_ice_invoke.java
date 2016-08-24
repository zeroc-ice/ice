// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Callback object for {@link ObjectPrx#begin_ice_invoke}.
 **/
public abstract class Callback_Object_ice_invoke
    extends IceInternal.TwowayCallback implements _Callback_Object_ice_invoke
{
    /**
     * The Ice run time calls <code>response</code> when an asynchronous operation invocation
     * completes successfully or raises a user exception.
     *
     * @param ret Indicates the result of the invocation. If <code>true</code>, the operation
     * completed succesfully; if <code>false</code>, the operation raised a user exception.
     * @param outParams Contains the encoded out-parameters of the operation (if any) if <code>ok</code>
     * is <code>true</code>; otherwise, if <code>ok</code> is <code>false</code>, contains the
     * encoded user exception raised by the operation.
     **/
    @Override
    public abstract void response(boolean ret, byte[] outParams);

    @Override
    public final void __completed(AsyncResult __result)
    {
        ObjectPrxHelperBase.__ice_invoke_completed(this, __result);
    }
}
