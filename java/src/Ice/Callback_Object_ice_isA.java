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
 * Callback object for {@link ObjectPrx#.begin_ice_isA}.
 **/
public abstract class Callback_Object_ice_isA extends TwowayCallback
{
    /**
     * Called when the invocation completes successfully.
     *
     * @param __ret True if the target object supports the given interface, false otherwise.
     **/
    public abstract void response(boolean __ret);

    public final void __completed(AsyncResult __result)
    {
        boolean __ret = false;
        try
        {
            __ret = __result.getProxy().end_ice_isA(__result);
        }
        catch(LocalException __ex)
        {
            exception(__ex);
            return;
        }
        response(__ret);
    }
}
