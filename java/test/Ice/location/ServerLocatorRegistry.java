// **********************************************************************
//
// Copyright (c) 2001
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
