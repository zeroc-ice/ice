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
