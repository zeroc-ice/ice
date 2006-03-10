// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class RoutingTable
{
    public
    RoutingTable()
    {
    }

    //
    // Clear the contents of the routing table.
    //
    synchronized public void
    clear()
    {
	_table.clear();
    }

    //
    // Returns false if the Proxy exists already.
    //
    public boolean
    add(Ice.ObjectPrx prx)
    {
        if(prx == null)
        {
            return false;
        }

	//
	// We insert the proxy in its default form into the routing table.
	//
        Ice.ObjectPrx proxy = prx.ice_twoway().ice_secure(false);

        synchronized(this)
        {
            if(!_table.containsKey(proxy.ice_getIdentity()))
            {
                _table.put(proxy.ice_getIdentity(), proxy);
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    private java.util.HashMap _table = new java.util.HashMap();
}
