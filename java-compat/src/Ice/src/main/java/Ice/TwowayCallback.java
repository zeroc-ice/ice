//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * Base interface for generated twoway operation callback.
 **/
public interface TwowayCallback
{
    /**
     * Called when the invocation raises an Ice run-time exception.
     *
     * @param ex The Ice run-time exception raised by the operation.
     **/
    public void exception(LocalException ex);

    /**
     * Called when the invocation raises an Ice system exception.
     *
     * @param ex The Ice system exception raised by the operation.
     **/
    public void exception(SystemException ex);
}
