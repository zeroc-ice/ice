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

public final class LocatorManager
{
    LocatorManager()
    {
    }

    synchronized void
    destroy()
    {
        _table.clear();
	_adapterTables.clear();
    }

    //
    // Returns locator info for a given locator. Automatically creates
    // the locator info if it doesn't exist yet.
    //
    public LocatorInfo
    get(Ice.LocatorPrx locator)
    {
        if(locator == null)
        {
            return null;
        }

	//
	// TODO: reap unused locator info objects?
	//

        synchronized (this)
        {
            LocatorInfo info = (LocatorInfo)_table.get(locator);
            if(info == null)
            {
		//
		// Rely on locator identity for the adapter table. We want to
		// have only one table per locator (not one per locator
		// proxy).
		//
		LocatorAdapterTable adapterTable = (LocatorAdapterTable)_adapterTables.get(locator.ice_getIdentity());
		if(adapterTable == null)
		{
		    adapterTable = new LocatorAdapterTable();
		    _adapterTables.put(locator.ice_getIdentity(), adapterTable);
		}

                info = new LocatorInfo(locator, adapterTable);
                _table.put(locator, info);
            }

            return info;
        }
    }

    private java.util.HashMap _table = new java.util.HashMap();
    private java.util.HashMap _adapterTables = new java.util.HashMap();
}
