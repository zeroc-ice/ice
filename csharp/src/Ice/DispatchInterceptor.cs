//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    public abstract class DispatchInterceptor : ObjectImpl
    {
        /// <summary>
        /// Called by the Ice run time to dispatch an incoming request. The implementation
        /// of <code>dispatch</code> must dispatch the request to the actual servant.
        /// </summary>
        /// <param name="request">The details of the incoming request.</param>
        /// <returns>The task if dispatched asynchronously, null otherwise.</returns>
        public abstract System.Threading.Tasks.Task<OutputStream>
        dispatch(Request request);

        public override System.Threading.Tasks.Task<OutputStream>
        iceDispatch(IceInternal.Incoming inc, Current current)
        {
            try
            {
                return dispatch(inc);
            }
            catch(Exception)
            {
                //
                // If the input parameters weren't read, make sure we skip them here. It's needed to read the
                // encoding version used by the client to eventually marshal the user exception. It's also needed
                // if we dispatch a batch oneway request to read the next batch request.
                //
                if(current.encoding == null || (current.encoding.major == 0 && current.encoding.minor == 0))
                {
                    inc.skipReadParams();
                }
                throw;
            }
        }
    }
}
