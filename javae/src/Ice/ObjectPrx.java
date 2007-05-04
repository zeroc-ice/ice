// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public interface ObjectPrx
{
    /**
     * @deprecated This method has been replaced with ice_getHash.
     **/
    int ice_hash();
    int ice_getHash();

    /**
     * @deprecated This method has been replaced with ice_getCommunicator.
     **/
    Communicator ice_communicator();
    Communicator ice_getCommunicator();

    String ice_toString();

    boolean ice_isA(String __id);
    boolean ice_isA(String __id, java.util.Hashtable __context);

    void ice_ping();
    void ice_ping(java.util.Hashtable __context);

    String[] ice_ids();
    String[] ice_ids(java.util.Hashtable __context);

    String ice_id();
    String ice_id(java.util.Hashtable __context);

    Identity ice_getIdentity();
    /**
     * @deprecated This method has been replaced with ice_identity.
     **/
    ObjectPrx ice_newIdentity(Identity newIdentity);
    ObjectPrx ice_identity(Identity newIdentity);

    java.util.Hashtable ice_getContext();
    /**
     * @deprecated This method has been replaced with ice_context.
     **/
    ObjectPrx ice_newContext(java.util.Hashtable newContext);
    ObjectPrx ice_context(java.util.Hashtable newContext);

    String ice_getFacet();
    /**
     * @deprecated This method has been replaced with ice_facet.
     **/
    ObjectPrx ice_newFacet(String newFacet);
    ObjectPrx ice_facet(String newFacet);

    String ice_getAdapterId();
    ObjectPrx ice_adapterId(String newAdapterId);

    boolean ice_isSecure();
    ObjectPrx ice_secure(boolean b);

    ObjectPrx ice_router(Ice.RouterPrx router);
    Ice.RouterPrx ice_getRouter();
    
    ObjectPrx ice_locator(Ice.LocatorPrx locator);
    Ice.LocatorPrx ice_getLocator();

    ObjectPrx ice_twoway();
    boolean ice_isTwoway();
    ObjectPrx ice_oneway();
    boolean ice_isOneway();
    ObjectPrx ice_batchOneway();
    boolean ice_isBatchOneway();
    ObjectPrx ice_datagram();
    boolean ice_isDatagram();
    ObjectPrx ice_batchDatagram();
    boolean ice_isBatchDatagram();

    ObjectPrx ice_timeout(int t);

    /**
     * @deprecated This method has been replaced with ice_getConnection.
     **/
    Connection ice_connection();
    Connection ice_getConnection();
    Connection ice_getCachedConnection();

    //
    // javac 1.1 emits a bogus error about this method causing ambiguity with
    // the declaration in java.lang.Object.
    //
    //boolean equals(java.lang.Object r);
}
