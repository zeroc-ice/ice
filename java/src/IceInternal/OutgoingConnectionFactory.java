// **********************************************************************
//
// Copyright (c) 2002
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

public class OutgoingConnectionFactory
{
    public synchronized void
    destroy()
    {
        if(_instance == null)
        {
            return;
        }

        java.util.Iterator p = _connections.values().iterator();
        while(p.hasNext())
        {
            Connection connection = (Connection)p.next();
            connection.destroy(Connection.CommunicatorDestroyed);
        }

        _instance = null;
    }

    public synchronized void
    waitUntilFinished()
    {
	//
	// First we wait until the factory is destroyed.
	//
	while(_instance != null)
	{
	    try
	    {
		wait();
	    }
	    catch(InterruptedException ex)
	    {
	    }
	}
	
	//
	// Now we wait for until the destruction of each connection is
	// finished.
	//
        java.util.Iterator p = _connections.values().iterator();
        while(p.hasNext())
        {
            Connection connection = (Connection)p.next();
            connection.waitUntilFinished();
        }

	//
	// We're done, now we can throw away all connections.
	//
        _connections.clear();
    }

    public synchronized Connection
    create(Endpoint[] endpoints)
    {
        if(_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(endpoints.length > 0);

	//
	// Reap connections for which destruction has completed.
	//
        java.util.Iterator p = _connections.values().iterator();
        while(p.hasNext())
        {
            Connection connection = (Connection)p.next();
            if(connection.isFinished())
            {
                p.remove();
            }
        }

        //
        // Search for existing connections.
        //
	DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        for(int i = 0; i < endpoints.length; i++)
        {
	    Endpoint endpoint = endpoints[i];
	    if(defaultsAndOverrides.overrideTimeout)
	    {
		endpoint = endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
	    }

            Connection connection = (Connection)_connections.get(endpoint);
            if(connection != null)
            {
		//
		// Don't return connections for which destruction has
		// been initiated.
		//
		if(!connection.isDestroyed())
		{
		    return connection;
		}
            }
        }

        //
        // No connections exist, try to create one.
        //
        TraceLevels traceLevels = _instance.traceLevels();
        Ice.Logger logger = _instance.logger();

        Connection connection = null;
        Ice.LocalException exception = null;
        for(int i = 0; i < endpoints.length; i++)
        {
  	    Endpoint endpoint = endpoints[i];
	    if(defaultsAndOverrides.overrideTimeout)
	    {
		endpoint = endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
	    }
	    
	    try
            {
                Transceiver transceiver = endpoint.clientTransceiver();
                if(transceiver == null)
                {
                    Connector connector = endpoint.connector();
                    assert(connector != null);
                    transceiver = connector.connect(endpoint.timeout());
                    assert(transceiver != null);
                }
                connection = new Connection(_instance, transceiver, endpoint, null);
		connection.validate();
                connection.activate();
                _connections.put(endpoint, connection);
                break;
            }
            catch(Ice.LocalException ex)
            {
                exception = ex;
            }

            if(traceLevels.retry >= 2)
            {
                StringBuffer s = new StringBuffer();
                s.append("connection to endpoint failed");
                if(i < endpoints.length - 1)
                {
                    s.append(", trying next endpoint\n");
                }
                else
                {
                    s.append(" and no more endpoints to try\n");
                }
                s.append(exception.toString());
                logger.trace(traceLevels.retryCat, s.toString());
            }
        }

        if(connection == null)
        {
            assert(exception != null);
            throw exception;
        }

        return connection;
    }

    public synchronized void
    setRouter(Ice.RouterPrx router)
    {
        if(_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        RouterInfo routerInfo = _instance.routerManager().get(router);
        if(routerInfo != null)
        {
            //
            // Search for connections to the router's client proxy
            // endpoints, and update the object adapter for such
            // connections, so that callbacks from the router can be
            // received over such connections.
            //
            Ice.ObjectPrx proxy = routerInfo.getClientProxy();
            Ice.ObjectAdapter adapter = routerInfo.getAdapter();
	    DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
            Endpoint[] endpoints = ((Ice.ObjectPrxHelper)proxy).__reference().endpoints;
            for(int i = 0; i < endpoints.length; i++)
            {
		Endpoint endpoint = endpoints[i];
		if(defaultsAndOverrides.overrideTimeout)
		{
		    endpoint = endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
		}

                Connection connection = (Connection)_connections.get(endpoint);
                if(connection != null)
                {
                    connection.setAdapter(adapter);
                }
            }
        }
    }

    public synchronized void
    removeAdapter(Ice.ObjectAdapter adapter)
    {
        if(_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        java.util.Iterator p = _connections.values().iterator();
        while(p.hasNext())
        {
            Connection connection = (Connection)p.next();
            if(connection.getAdapter() == adapter)
            {
                connection.setAdapter(null);
            }
        }
    }

    //
    // Only for use by Instance.
    //
    OutgoingConnectionFactory(Instance instance)
    {
        _instance = instance;
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_instance == null);

        super.finalize();
    }

    private Instance _instance;
    private java.util.HashMap _connections = new java.util.HashMap();
}
