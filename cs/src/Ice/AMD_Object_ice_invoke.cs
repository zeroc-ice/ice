// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{

    public interface AMD_Object_ice_invoke
    {
	void ice_response(bool ok, byte[] outParams);
	void ice_exception(System.Exception ex);
    }

}
