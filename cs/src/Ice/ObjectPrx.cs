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
	
    using System.Collections;

    public interface ObjectPrx
    {
	int ice_hash();
	bool ice_isA(string __id);
	bool ice_isA(string __id, Context __context);
	void ice_ping();
	void ice_ping(Context __context);
	string[] ice_ids();
	string[] ice_ids(Context __context);
	string ice_id();
	string ice_id(Context __context);
	FacetPath ice_facets();
	FacetPath ice_facets(Context __context);

	// Returns true if ok, false if user exception.
	bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams);
	bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams,
			Context __context);
	void ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inParams);
	void ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inParams,
			      Context context);
	Identity ice_getIdentity();
	ObjectPrx ice_newIdentity(Identity newIdentity);
	Context ice_getContext();
	ObjectPrx ice_newContext(Context newContext);
	FacetPath ice_getFacet();
	ObjectPrx ice_newFacet(FacetPath newFacet);
	ObjectPrx ice_appendFacet(string f);
	ObjectPrx ice_twoway();
	bool ice_isTwoway();
	ObjectPrx ice_oneway();
	bool ice_isOneway();
	ObjectPrx ice_batchOneway();
	bool ice_isBatchOneway();
	ObjectPrx ice_datagram();
	bool ice_isDatagram();
	ObjectPrx ice_batchDatagram();
	bool ice_isBatchDatagram();
	ObjectPrx ice_secure(bool b);
	ObjectPrx ice_compress(bool co);
	ObjectPrx ice_timeout(int t);
	ObjectPrx ice_router(Ice.RouterPrx router);
	ObjectPrx ice_locator(Ice.LocatorPrx locator);
	ObjectPrx ice_collocationOptimization(bool b);
	ObjectPrx ice_default();
    }

}
