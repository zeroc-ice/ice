 // **********************************************************************
// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{

    public interface Object : System.ICloneable
    {
	int ice_hash();
	bool ice_isA(string s, Current current);
	void ice_ping(Current current);
	string[] ice_ids(Current current);
	string ice_id(Current current);
	FacetPath ice_facets(Current current);
	void ice_preMarshal();
	void ice_postUnmarshal();
	IceInternal.DispatchStatus __dispatch(IceInternal.Incoming inc, Current current);
	void __write(IceInternal.BasicStream __os, bool __marshalFacets);
	void __read(IceInternal.BasicStream __is, bool __rid);
	void ice_addFacet(Object facet, string name);
	Object ice_removeFacet(string name);
	void ice_removeAllFacets();
	Object ice_findFacet(string name);
	Object ice_findFacetPath(FacetPath path, int start);
    }

}
