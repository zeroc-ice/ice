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
	
    public interface Object_Del
    {
	bool ice_isA(string id, Ice.Context context);
	void ice_ping(Ice.Context context);
	string[] ice_ids(Ice.Context context);
	string ice_id(Ice.Context context);
	bool ice_invoke(string operation, Ice.OperationMode mode, byte[] inParams, out byte[] outParams,
			Ice.Context context);
    }

}
