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

package IceInternal;

final class LocatorTable
{
    LocatorTable()
    {
    }

    synchronized void
    clear()
    {
	_adapterEndpointsTable.clear();
	_objectTable.clear();
    }

    synchronized IceInternal.Endpoint[]
    getAdapterEndpoints(String adapter)
    {
	return (IceInternal.Endpoint[])_adapterEndpointsTable.get(adapter);
    }

    synchronized void
    addAdapterEndpoints(String adapter, IceInternal.Endpoint[] endpoints)
    {
	_adapterEndpointsTable.put(adapter, endpoints);
    }

    synchronized IceInternal.Endpoint[]
    removeAdapterEndpoints(String adapter)
    {
	return (IceInternal.Endpoint[])_adapterEndpointsTable.remove(adapter);
    }

    synchronized Ice.ObjectPrx
    getProxy(Ice.Identity id)
    {
	return (Ice.ObjectPrx)_objectTable.get(id);
    }

    synchronized void
    addProxy(Ice.Identity id, Ice.ObjectPrx proxy)
    {
	_objectTable.put(id, proxy);
    }

    synchronized Ice.ObjectPrx
    removeProxy(Ice.Identity id)
    {
	return (Ice.ObjectPrx)_objectTable.remove(id);
    }

    private java.util.HashMap _adapterEndpointsTable = new java.util.HashMap();
    private java.util.HashMap _objectTable = new java.util.HashMap();
}

