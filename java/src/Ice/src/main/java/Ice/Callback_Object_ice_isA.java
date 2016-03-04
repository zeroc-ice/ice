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
 * Callback object for {@link ObjectPrx#begin_ice_isA}.
 **/
public abstract class Callback_Object_ice_isA extends IceInternal.TwowayCallback implements Ice.TwowayCallbackBool
{
    /**
     * Called when the invocation completes successfully.
     *
     * @param __ret True if the target object supports the given interface, false otherwise.
     **/
    @Override
    public abstract void response(boolean __ret);

    @Override
    public final void __completed(AsyncResult __result)
    {
        ObjectPrxHelperBase.__ice_isA_completed(this, __result);
    }
}
