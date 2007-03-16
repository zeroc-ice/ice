// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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

    private java.util.Hashtable _adapterEndpointsTable = new java.util.Hashtable();
    private java.util.Hashtable _objectTable = new java.util.Hashtable();
}

