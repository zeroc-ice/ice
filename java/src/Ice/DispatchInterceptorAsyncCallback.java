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
     * Called when the operation succeeded or raised a user exception,
     * as indicated by the <code>ok</code> parameter.
     *
     * @param ok True if the operation succeeded, or false if the
     * operation raised a user exception.
     * @return True to allow the Ice run time to handle the result
     * as it normally would, or false if the interceptor has handled
     * the operation.
     **/
    boolean response(boolean ok);

    /**
     * Called when the operation failed with a run-time exception.
     *
     * @param ex The exception raised by the operation.
     * @return True to allow the Ice run time to handle the result
     * as it normally would, or false if the interceptor has handled
     * the operation.
     **/
    boolean exception(java.lang.Exception ex);
}
