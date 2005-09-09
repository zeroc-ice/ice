// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    synchronized IceInternal.EndpointI[]
    getAdapterEndpoints(String adapter)
    {
	return (IceInternal.EndpointI[])_adapterEndpointsTable.get(adapter);
    }

    synchronized void
    addAdapterEndpoints(String adapter, IceInternal.EndpointI[] endpoints)
    {
	_adapterEndpointsTable.put(adapter, endpoints);
    }

    synchronized IceInternal.EndpointI[]
    removeAdapterEndpoints(String adapter)
    {
	return (IceInternal.EndpointI[])_adapterEndpointsTable.remove(adapter);
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

