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

public class OutgoingConnectionFactory
{
    public synchronized Connection
    create(Endpoint[] endpoints)
    {
        if (_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(endpoints.length > 0);

        //
        // Reap destroyed connections.
        //
        java.util.Iterator p = _connections.values().iterator();
        while (p.hasNext())
        {
            Connection connection = (Connection)p.next();
            if (connection.destroyed())
            {
                p.remove();
            }
        }

        //
        // Search for existing connections.
        //
	DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        for (int i = 0; i < endpoints.length; i++)
        {
	    Endpoint endpoint = endpoints[i];
	    if (defaultsAndOverrides.overrideTimeout)
	    {
		endpoint = endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
	    }

            Connection connection = (Connection)_connections.get(endpoint);
            if (connection != null)
            {
                return connection;
            }
        }

        //
        // No connections exist, try to create one
        //
        TraceLevels traceLevels = _instance.traceLevels();
        Ice.Logger logger = _instance.logger();

        Connection connection = null;
        Ice.LocalException exception = null;
        for (int i = 0; i < endpoints.length; i++)
        {
  	    Endpoint endpoint = endpoints[i];
	    if (defaultsAndOverrides.overrideTimeout)
	    {
		endpoint = endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
	    }
	    
	    try
            {
                Transceiver transceiver = endpoint.clientTransceiver();
                if (transceiver == null)
                {
                    Connector connector = endpoint.connector();
                    assert(connector != null);
                    transceiver = connector.connect(endpoint.timeout());
                    assert(transceiver != null);
                }
                connection = new Connection(_instance, transceiver, endpoint, null);
                connection.activate();
                _connections.put(endpoint, connection);
                break;
            }
            catch (Ice.SocketException ex)
            {
                exception = ex;
            }
            catch (Ice.DNSException ex)
            {
                exception = ex;
            }
            catch (Ice.TimeoutException ex)
            {
                exception = ex;
            }

            if (traceLevels.retry >= 2)
            {
                StringBuffer s = new StringBuffer();
                s.append("connection to endpoint failed");
                if (i < endpoints.length - 1)
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

        if (connection == null)
        {
            assert(exception != null);
            throw exception;
        }

        return connection;
    }

    public synchronized void
    setRouter(Ice.RouterPrx router)
    {
        if (_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        RouterInfo routerInfo = _instance.routerManager().get(router);
        if (routerInfo != null)
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
            for (int i = 0; i < endpoints.length; i++)
            {
		Endpoint endpoint = endpoints[i];
		if (defaultsAndOverrides.overrideTimeout)
		{
		    endpoint = endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
		}

                Connection connection = (Connection)_connections.get(endpoint);
                if (connection != null)
                {
                    connection.setAdapter(adapter);
                }
            }
        }
    }

    public synchronized void
    removeAdapter(Ice.ObjectAdapter adapter)
    {
        if (_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        java.util.Iterator p = _connections.values().iterator();
        while (p.hasNext())
        {
            Connection connection = (Connection)p.next();
            if (connection.getAdapter() == adapter)
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

    public synchronized void
    destroy()
    {
        if (_instance == null)
        {
            return;
        }

        java.util.Iterator p = _connections.values().iterator();
        while (p.hasNext())
        {
            Connection connection = (Connection)p.next();
            connection.destroy(Connection.CommunicatorDestroyed);
        }
        _connections.clear();
        _instance = null;
    }

    private Instance _instance;
    private java.util.HashMap _connections = new java.util.HashMap();
}
