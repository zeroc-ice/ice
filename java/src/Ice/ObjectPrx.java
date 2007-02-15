// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    boolean ice_isA(String __id, java.util.Map __context);

    void ice_ping();
    void ice_ping(java.util.Map __context);

    String[] ice_ids();
    String[] ice_ids(java.util.Map __context);

    String ice_id();
    String ice_id(java.util.Map __context);

    // Returns true if ok, false if user exception.
    boolean ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams);
    boolean ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams,
                       java.util.Map __context);

    void ice_invoke_async(AMI_Object_ice_invoke cb, String operation, OperationMode mode, byte[] inParams);
    void ice_invoke_async(AMI_Object_ice_invoke cb, String operation, OperationMode mode, byte[] inParams,
                          java.util.Map context);

    Identity ice_getIdentity();
    /**
     * @deprecated This method has been replaced with ice_identity.
     **/
    ObjectPrx ice_newIdentity(Identity newIdentity);
    ObjectPrx ice_identity(Identity newIdentity);

    java.util.Map ice_getContext();
    /**
     * @deprecated This method has been replaced with ice_context.
     **/
    ObjectPrx ice_newContext(java.util.Map newContext);
    ObjectPrx ice_context(java.util.Map newContext);
    /**
     * @deprecated This method has been replaced with ice_context.
     **/
    ObjectPrx ice_defaultContext();

    String ice_getFacet(); 
    /**
     * @deprecated This method has been replaced with ice_facet.
     **/
    ObjectPrx ice_newFacet(String newFacet);
    ObjectPrx ice_facet(String newFacet);

    String ice_getAdapterId();
    /**
     * @deprecated This method has been replaced with ice_adapterId.
     **/
    ObjectPrx ice_newAdapterId(String newAdapterId);
    ObjectPrx ice_adapterId(String newAdapterId);

    Endpoint[] ice_getEndpoints();
    /**
     * @deprecated This method has been replaced with ice_endpoints.
     **/
    ObjectPrx ice_newEndpoints(Endpoint[] newEndpoints);
    ObjectPrx ice_endpoints(Endpoint[] newEndpoints);

    int ice_getLocatorCacheTimeout();
    ObjectPrx ice_locatorCacheTimeout(int newTimeout);

    boolean ice_isConnectionCached();
    ObjectPrx ice_connectionCached(boolean newCache);

    EndpointSelectionType ice_getEndpointSelection();
    ObjectPrx ice_endpointSelection(EndpointSelectionType newType);

    boolean ice_isSecure();
    ObjectPrx ice_secure(boolean b);

    boolean ice_isPreferSecure();
    ObjectPrx ice_preferSecure(boolean b);

    Ice.RouterPrx ice_getRouter();
    ObjectPrx ice_router(Ice.RouterPrx router);

    Ice.LocatorPrx ice_getLocator();
    ObjectPrx ice_locator(Ice.LocatorPrx locator);

    boolean ice_isCollocationOptimized();
    /**
     * @deprecated This method has been replaced with ice_collocationOptimized.
     **/
    ObjectPrx ice_collocationOptimization(boolean b);
    ObjectPrx ice_collocationOptimized(boolean b);

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

    ObjectPrx ice_compress(boolean co);
    ObjectPrx ice_timeout(int t);
    ObjectPrx ice_connectionId(String connectionId);

    boolean ice_isThreadPerConnection();
    ObjectPrx ice_threadPerConnection(boolean tpc);

    /**
     * @deprecated This method has been replaced with ice_getConnection.
     **/
    Connection ice_connection();
    Connection ice_getConnection();
    Connection ice_getCachedConnection();

    boolean equals(java.lang.Object r);
}
