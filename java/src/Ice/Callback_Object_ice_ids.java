// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Callback object for {@link ObjectPrx#.begin_ice_ids}.
 **/
public abstract class Callback_Object_ice_ids extends TwowayCallback
{
    /**
     * Called when the invocation completes successfully.
     *
     * @param __ret The Slice type ids of the interfaces supported by the target object.
     **/
    public abstract void response(String[] __ret);

    public final void __completed(AsyncResult __result)
    {
        String[] __ret = null;
        try
        {
            __ret = __result.getProxy().end_ice_ids(__result);
        }
        catch(LocalException __ex)
        {
            exception(__ex);
            return;
        }
        response(__ret);
    }
}
