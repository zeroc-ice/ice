// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class ServerLocatorRegistry extends Ice._LocatorRegistryDisp
{
    public void
    setAdapterDirectProxy(String adapter, Ice.ObjectPrx object, Ice.Current current)
    {
	_adapters.put(adapter, object);
    }

    public Ice.ObjectPrx
    getAdapter(String adapter)
    {
	return (Ice.ObjectPrx)_adapters.get(adapter);
    }

    private java.util.HashMap _adapters = new java.util.HashMap();
}
