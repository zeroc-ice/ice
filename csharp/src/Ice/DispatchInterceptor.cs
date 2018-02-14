// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

namespace Ice
{
    /// <summary>
    /// Base class that allows a server intercept incoming requests.
    /// The application must derive a concrete class from DispatchInterceptor
    /// that implements the DispatchInterceptor.dispatch operation. An instance of this derived
    /// class can be registered with an object adapter like any other servant.
    /// A dispatch interceptor is useful particularly to automatically retry requests
    /// that have failed due to a recoverable error condition.
    /// </summary>
    public abstract class DispatchInterceptor : Ice.ObjectImpl
    {
        /// <summary>
        /// Called by the Ice run time to dispatch an incoming request. The implementation
        /// of <code>dispatch</code> must dispatch the request to the actual servant.
        /// </summary>
        /// <param name="request">The details of the incoming request.</param>
        /// <returns>For synchronous dispatch, the return value must be whatever is
        /// returned ice_dispatch. For asynchronous dispatch, the return
        /// value must be DispatchAsync.</returns>
        public abstract DispatchStatus 
        dispatch(Request request);

        public override DispatchStatus
        dispatch__(IceInternal.Incoming inc, Current current)
        {
            try
            {
                DispatchStatus status = dispatch(inc);
                if(status != DispatchStatus.DispatchAsync)
                {
                    //
                    // Make sure 'inc' owns the connection etc.
                    //
                    inc.killAsync();
                }
                return status;
            }
            catch(ResponseSentException)
            {
                return DispatchStatus.DispatchAsync;
            }
            catch(System.Exception)
            {
                try
                {
                    inc.killAsync();
                    throw;
                }
                catch(ResponseSentException)
                {
                    return DispatchStatus.DispatchAsync;
                }
            }
        }
    }
}
