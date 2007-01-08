// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public interface AMD_Object_ice_invoke
{
    void ice_response(boolean ok, byte[] outParams);
    void ice_exception(java.lang.Exception ex);
}
