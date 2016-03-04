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
 * Callback object for {@link ObjectPrx#begin_ice_id}.
 **/
public abstract class Callback_Object_ice_id extends IceInternal.TwowayCallback
    implements Ice.TwowayCallbackArg1<String>
{
    /**
     * Called when the invocation completes successfully.
     *
     * @param __ret The Slice type id of the most-derived interface supported by the target object.
     **/
    @Override
    public abstract void response(String __ret);

    @Override
    public final void __completed(AsyncResult __result)
    {
        ObjectPrxHelperBase.__ice_id_completed(this, __result);
    }
}
