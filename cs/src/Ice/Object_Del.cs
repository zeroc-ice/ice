// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************
namespace Ice
{
	
    public interface Object_Del
    {
	bool ice_isA(string id, Ice.Context context);
	void  ice_ping(Ice.Context context);
	StringSeq ice_ids(Ice.Context context);
	string ice_id(Ice.Context context);
	FacetPath ice_facets(Ice.Context context);
	bool ice_invoke(string operation, Ice.OperationMode mode, Ice.ByteSeq inParams, out Ice.ByteSeq outParams,
			Ice.Context context);
	void ice_invoke_async(AMI_Object_ice_invoke cb, string operation, Ice.OperationMode mode, Ice.ByteSeq inParams,
			      Ice.Context context);
    }

}
