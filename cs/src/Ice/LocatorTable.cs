// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


namespace IceInternal
{

    using System.Collections;

    sealed class LocatorTable
    {
	internal LocatorTable()
	{
	    _adapterEndpointsTable = new Hashtable();
	    _objectTable = new Hashtable();
	}
	
	internal void clear()
	{
	    lock(this)
	    {
		_adapterEndpointsTable.Clear();
		_objectTable.Clear();
	    }
	}
	
	internal IceInternal.Endpoint[] getAdapterEndpoints(string adapter)
	{
	    lock(this)
	    {
		return (IceInternal.Endpoint[])_adapterEndpointsTable[adapter];
	    }
	}
	
	internal void addAdapterEndpoints(string adapter, IceInternal.Endpoint[] endpoints)
	{
	    lock(this)
	    {
		_adapterEndpointsTable[adapter] = endpoints;
	    }
	}
	
	internal IceInternal.Endpoint[] removeAdapterEndpoints(string adapter)
	{
	    lock(this)
	    {
		IceInternal.Endpoint[] endpoints = (IceInternal.Endpoint[])_adapterEndpointsTable[adapter];
		_adapterEndpointsTable.Remove(adapter);
		return endpoints;
	    }
	}
	
	internal Ice.ObjectPrx getProxy(Ice.Identity id)
	{
	    lock(this)
	    {
		return (Ice.ObjectPrx)_objectTable[id];
	    }
	}
	
	internal void addProxy(Ice.Identity id, Ice.ObjectPrx proxy)
	{
	    lock(this)
	    {
		_objectTable[id] = proxy;
	    }
	}
	
	internal Ice.ObjectPrx removeProxy(Ice.Identity id)
	{
	    lock(this)
	    {
		Ice.ObjectPrx obj = (Ice.ObjectPrx)_objectTable[id];
		_objectTable.Remove(id);
		return obj;
	    }
	}
	
	private Hashtable _adapterEndpointsTable;
	private Hashtable _objectTable;
    }
    
}
