// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public interface ObjectPrx
{
    public int ice_hash();

    public boolean ice_isA(String __id);
    public boolean ice_isA(String __id, java.util.HashMap __context);

    public void ice_ping();
    public void ice_ping(java.util.HashMap __context);

    public byte[] ice_invoke(String operation, boolean nonmutating,
                             byte[] inParams);
    public byte[] ice_invoke(String operation, boolean nonmutating,
                             byte[] inParams, java.util.HashMap __context);

    public String ice_getIdentity();

    public ObjectPrx ice_newIdentity(String newIdentity);

    public String ice_getFacet();

    public ObjectPrx ice_newFacet(String newFacet);

    public ObjectPrx ice_twoway();

    public ObjectPrx ice_oneway();

    public ObjectPrx ice_batchOneway();

    public ObjectPrx ice_datagram();

    public ObjectPrx ice_batchDatagram();

    public ObjectPrx ice_secure(boolean b);

    public ObjectPrx ice_timeout(int t);

    public void ice_flush();

    public boolean equals(java.lang.Object r);
}
