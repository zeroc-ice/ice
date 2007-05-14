//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public interface DispatchInterceptorAsyncCallback
{
    boolean response(boolean ok);
    boolean exception(java.lang.Exception ex);
}
