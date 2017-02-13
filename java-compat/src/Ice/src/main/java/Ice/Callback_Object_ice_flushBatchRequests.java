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
