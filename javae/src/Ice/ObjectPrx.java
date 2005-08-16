// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public interface ObjectPrx
{
    int ice_hash();

    Communicator ice_communicator();

    String ice_toString();

    boolean ice_isA(String __id);
    boolean ice_isA(String __id, java.util.Hashtable __context);

    void ice_ping();
    void ice_ping(java.util.Hashtable __context);

    String[] ice_ids();
    String[] ice_ids(java.util.Hashtable __context);

    String ice_id();
    String ice_id(java.util.Hashtable __context);

    // Returns true if ok, false if user exception.
    boolean ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams);
    boolean ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams,
                       java.util.Hashtable __context);

    Identity ice_getIdentity();
    ObjectPrx ice_newIdentity(Identity newIdentity);

    java.util.Hashtable ice_getContext();
    ObjectPrx ice_newContext(java.util.Hashtable newContext);
    ObjectPrx ice_defaultContext();

    String ice_getFacet();
    ObjectPrx ice_newFacet(String newFacet);

    ObjectPrx ice_twoway();
    boolean ice_isTwoway();
    ObjectPrx ice_oneway();
    boolean ice_isOneway();
    ObjectPrx ice_batchOneway();
    boolean ice_isBatchOneway();

    ObjectPrx ice_timeout(int t);
    ObjectPrx ice_router(Ice.RouterPrx router);
    ObjectPrx ice_locator(Ice.LocatorPrx locator);
    ObjectPrx ice_default();

    Connection ice_connection();

    boolean equals(java.lang.Object r);
}
