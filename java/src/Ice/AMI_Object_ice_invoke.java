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
 * Callback object for {@link ObjectPrx#.ice_invoke_async}.
 **/
public abstract class AMI_Object_ice_invoke extends Callback_Object_ice_invoke
{
    /**
     * Called when an asynchronous operation invocation completes successfully or raises a user exception.
     *
     * @param ok Indicates the result of the invocation. If <code>true</code>, the operation
     * completed succesfully; if <code>false</code>, the operation raised a user exception.
     * @param outEncaps Contains the encoded out-parameters of the operation (if any) if <code>ok</code>
     * is <code>true</code>; otherwise, if <code>ok</code> is <code>false</code>, contains the
     * encoded user exception raised by the operation.
     **/
    public abstract void ice_response(boolean ok, byte[] outEncaps);

    /**
     * Called when the invocation raises an Ice run-time exception.
     *
     * @param ex The Ice run-time exception raised by the operation.
     **/
    public abstract void ice_exception(LocalException ex);

    public final void response(boolean ok, byte[] outEncaps)
    {
        ice_response(ok, outEncaps);
    }

    public final void exception(LocalException ex)
    {
        ice_exception(ex);
    }

    @Override
    public final void sent(boolean sentSynchronously)
    {
        if(!sentSynchronously && this instanceof AMISentCallback)
        {
            ((AMISentCallback)this).ice_sent();
        }
    }
}
