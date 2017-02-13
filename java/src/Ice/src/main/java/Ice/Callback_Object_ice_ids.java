// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Callback object for {@link ObjectPrx#begin_ice_ids}.
 **/
public abstract class Callback_Object_ice_ids extends IceInternal.TwowayCallback
    implements Ice.TwowayCallbackArg1<String[]>
{
    /**
     * Called when the invocation completes successfully.
     *
     * @param __ret The Slice type ids of the interfaces supported by the target object.
     **/
    @Override
    public abstract void response(String[] __ret);

    @Override
    public final void __completed(AsyncResult __result)
    {
        ObjectPrxHelperBase.__ice_ids_completed(this, __result);
    }
}
