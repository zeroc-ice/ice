// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public interface ObjectPrx
{
    int ice_hash();

    boolean ice_isA(String __id);
    boolean ice_isA(String __id, java.util.Map __context);

    void ice_ping();
    void ice_ping(java.util.Map __context);

    String[] ice_ids();
    String[] ice_ids(java.util.Map __context);

    String ice_id();
    String ice_id(java.util.Map __context);

    String[] ice_facets();
    String[] ice_facets(java.util.Map __context);

    // Returns true if ok, false if user exception.
    boolean ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams);
    boolean ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams,
                       java.util.Map __context);

    Identity ice_getIdentity();
    ObjectPrx ice_newIdentity(Identity newIdentity);

    String[] ice_getFacet();
    ObjectPrx ice_newFacet(String[] newFacet);
    ObjectPrx ice_appendFacet(String f);

    ObjectPrx ice_twoway();
    ObjectPrx ice_oneway();
    ObjectPrx ice_batchOneway();
    ObjectPrx ice_datagram();
    ObjectPrx ice_batchDatagram();
    ObjectPrx ice_secure(boolean b);
    ObjectPrx ice_timeout(int t);
    ObjectPrx ice_router(Ice.RouterPrx router);
    ObjectPrx ice_default();

    void ice_flush();

    boolean equals(java.lang.Object r);
}
