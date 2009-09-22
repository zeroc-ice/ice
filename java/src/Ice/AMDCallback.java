// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public interface AMDCallback
{
    /**
     * ice_exception indicates to the Ice run time that
     * the operation completed with an exception.
     * @param ex The exception to be raised.
     **/
    public void ice_exception(java.lang.Exception ex);
}