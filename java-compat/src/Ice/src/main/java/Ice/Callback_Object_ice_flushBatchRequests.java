// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package Ice;

/**
 * Callback object for {@link ObjectPrx#begin_ice_flushBatchRequests}.
 **/
public abstract class Callback_Object_ice_flushBatchRequests extends OnewayCallback
{
    @Override
    public final void response()
    {
        // Not used.
    }
}
