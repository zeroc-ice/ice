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

package IceInternal;

public final class LocatorAdapterTable
{
    LocatorAdapterTable()
    {
    }

    synchronized void
    clear()
    {
	_table.clear();
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

    synchronized IceInternal.Endpoint[]
    remove(String adapter)
    {
	return (IceInternal.Endpoint[])_table.remove(adapter);
    }

    private java.util.HashMap _table = new java.util.HashMap();
};

