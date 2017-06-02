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
 * The callback object for asynchronous dispatch.
 **/
public interface DispatchInterceptorAsyncCallback
{
    /**
     * Called when the operation succeeded.
     *
     * @return True to allow the Ice run time to handle the result
     * as it normally would, or false if the interceptor has handled
     * the operation.
     **/
    boolean response();

    /**
     * Called when the operation failed with an exception.
     *
     * @param ex The exception raised by the operation.
     * @return True to allow the Ice run time to handle the result
     * as it normally would, or false if the interceptor has handled
     * the operation.
     **/
    boolean exception(java.lang.Exception ex);
}
