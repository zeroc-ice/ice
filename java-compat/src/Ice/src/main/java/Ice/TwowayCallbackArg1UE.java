//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

public interface TwowayCallbackArg1UE<T>  extends TwowayCallbackArg1<T>
{
    /**
     * Called when the invocation raises an user exception.
     *
     * @param ex The user exception raised by the operation.
     **/
    void exception(Ice.UserException ex);
}
