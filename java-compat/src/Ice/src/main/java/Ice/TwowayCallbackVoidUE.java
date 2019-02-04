//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

public interface TwowayCallbackVoidUE extends TwowayCallback
{
    /**
     * Called when the invocation response is received.
     **/
    void response();

    /**
     * Called when the invocation raises an user exception.
     *
     * @param ex The user exception raised by the operation.
     **/
    void exception(Ice.UserException ex);
}
