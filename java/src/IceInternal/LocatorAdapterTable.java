// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class LocatorAdapterTable
{
    LocatorAdapterTable()
    {
    }

    synchronized IceInternal.Endpoint[]
    get(String adapter)
    {
	return (IceInternal.Endpoint[])_table.get(adapter);
    }

    synchronized void
    add(String adapter, IceInternal.Endpoint[] endpoints)
    {
	_table.put(adapter, endpoints);
    }

    synchronized void
    remove(String adapter)
    {
	Object object = _table.remove(adapter);
	assert(object != null);
    }

    private java.util.HashMap _table = new java.util.HashMap();
};

