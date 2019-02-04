//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * AMDCallback is the interface from which all AMD callbacks are derived.
 **/
public interface AMDCallback
{
    /**
     * ice_exception indicates to the Ice run time that
     * the operation completed with an exception.
     * @param ex The exception to be raised.
     **/
    public void ice_exception(java.lang.Exception ex);
}
