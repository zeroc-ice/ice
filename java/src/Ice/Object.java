// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public interface Object
{
    boolean equals(java.lang.Object rhs);

    java.lang.Object clone() throws java.lang.CloneNotSupportedException;

    int ice_hash();

    boolean ice_isA(String s, Current current);

    void ice_ping(Current current);

    String[] ice_ids(Current current);

    String ice_id(Current current);

    String[] ice_facets(Current current);

    IceInternal.DispatchStatus __dispatch(IceInternal.Incoming in, Current current);

    void __write(IceInternal.BasicStream __os);

    void __read(IceInternal.BasicStream __is);

    void __marshal(Ice.Stream __os);

    void __unmarshal(Ice.Stream __is);

    void ice_marshal(String name, Ice.Stream stream);

    void ice_addFacet(Object facet, String name);

    Object ice_removeFacet(String name);

    void ice_removeAllFacets();

    Object ice_findFacet(String name);

    Object ice_findFacetPath(String[] path, int start);
}
