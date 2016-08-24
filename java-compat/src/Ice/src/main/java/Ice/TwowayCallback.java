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
 * Base interface for generated twoway operation callback.
 **/
public interface TwowayCallback
{
    /**
     * Called when the invocation raises an Ice run-time exception.
     *
     * @param __ex The Ice run-time exception raised by the operation.
     **/
    public void exception(LocalException __ex);

    /**
     * Called when the invocation raises an Ice system exception.
     *
     * @param __ex The Ice system exception raised by the operation.
     **/
    public void exception(SystemException __ex);
}
