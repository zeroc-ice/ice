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

public interface Object : System.ICloneable
{
    int ice_hash();
    bool ice_isA(string s, Current current);
    void ice_ping(Current current);
    StringSeq ice_ids(Current current);
    string ice_id(Current current);
    FacetPath ice_facets(Current current);
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
