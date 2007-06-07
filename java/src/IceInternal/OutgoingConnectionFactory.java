// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class OutgoingConnectionFactory
{

    public synchronized void
    destroy()
    {
        if(_destroyed)
        {
            return;
        }

        java.util.Iterator p = _connections.values().iterator();
        while(p.hasNext())
        {
            java.util.LinkedList connectionList = (java.util.LinkedList)p.next();
                
            java.util.Iterator q = connectionList.iterator();
            while(q.hasNext())
            {
                Ice.ConnectionI connection = (Ice.ConnectionI)q.next();
                connection.destroy(Ice.ConnectionI.CommunicatorDestroyed);
            }
        }

        _destroyed = true;
        notifyAll();
    }

    public void
    waitUntilFinished()
    {
        java.util.HashMap connections;

        synchronized(this)
        {
            //
            // First we wait until the factory is destroyed. We also
            // wait until there are no pending connections
            // anymore. Only then we can be sure the _connections
            // contains all connections.
            //
            while(!_destroyed || !_pending.isEmpty())
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
            // We want to wait until all connections are finished outside the
            // thread synchronization.
            //
            // For consistency with C#, we set _connections to null rather than to a
            // new empty list so that our finalizer does not try to invoke any
            // methods on member objects.
            //
            connections = _connections;
            _connections = null;
        }
        
        //
        // Now we wait for until the destruction of each connection is
        // finished.
        //
        java.util.Iterator p = connections.values().iterator();
        while(p.hasNext())
        {
            java.util.LinkedList connectionList = (java.util.LinkedList)p.next();
                
            java.util.Iterator q = connectionList.iterator();
            while(q.hasNext())
            {
                Ice.ConnectionI connection = (Ice.ConnectionI)q.next();
                connection.waitUntilFinished();
            }
        }
    }

    public Ice.ConnectionI
    create(EndpointI[] endpts, boolean hasMore, boolean threadPerConnection, Ice.EndpointSelectionType selType,
           Ice.BooleanHolder compress)
    {
        class ConnectorEndpointPair
        {
             public ConnectorEndpointPair(Connector c, EndpointI e)
             {
                 connector = c;
                 endpoint = e;
             }

             public Connector connector;
             public EndpointI endpoint;
        }

        assert(endpts.length > 0);
        java.util.ArrayList connectors = new java.util.ArrayList();

        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();

        synchronized(this)
        {
            if(_destroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            //
            // TODO: Remove when we no longer support SSL for JDK 1.4.
            //
            for(int i = 0; i < endpts.length; i++)
            {
                if(!threadPerConnection && endpts[i].requiresThreadPerConnection())
                {
                    Ice.FeatureNotSupportedException ex = new Ice.FeatureNotSupportedException();
                    ex.unsupportedFeature = "endpoint requires thread-per-connection:\n" + endpts[i].toString();
                    throw ex;
                }
            }

            //
            // Reap connections for which destruction has completed.
            //
            java.util.Iterator p = _connections.values().iterator();
            while(p.hasNext())
            {
                java.util.LinkedList connectionList = (java.util.LinkedList)p.next();
                
                java.util.Iterator q = connectionList.iterator();
                while(q.hasNext())
                {
                    Ice.ConnectionI con = (Ice.ConnectionI)q.next();
                    if(con.isFinished())
                    {
                        q.remove();
                    }
                }

                if(connectionList.isEmpty())
                {
                    p.remove();
                }
            }

            EndpointI[] endpoints = new EndpointI[endpts.length];
            System.arraycopy(endpts, 0, endpoints, 0, endpts.length);
            for(int i = 0; i < endpoints.length; i++)
            {
                //
                // Modify endpoints with overrides.
                //
                if(defaultsAndOverrides.overrideTimeout)
                {
                    endpoints[i] = endpoints[i].timeout(defaultsAndOverrides.overrideTimeoutValue);
                }

                //
                // Create connectors for the endpoint.
                //
                java.util.ArrayList cons = endpoints[i].connectors();
                assert(cons.size() > 0);

                //
                // Shuffle connectors is endpoint selection type is Random.
                //
                if(selType == Ice.EndpointSelectionType.Random)
                {
                    java.util.Collections.shuffle(cons);
                }

                p = cons.iterator();
                while(p.hasNext())
                {
                    connectors.add(new ConnectorEndpointPair((Connector)p.next(), endpoints[i]));
                }

            }

            //
            // Search for existing connections.
            //
            p = connectors.iterator();
            while(p.hasNext())
            {
                ConnectorEndpointPair cep = (ConnectorEndpointPair)p.next();


                java.util.LinkedList connectionList = (java.util.LinkedList)_connections.get(cep.connector);
                if(connectionList != null)
                {
                    java.util.Iterator q = connectionList.iterator();
                        
                    while(q.hasNext())
                    {
                        Ice.ConnectionI connection = (Ice.ConnectionI)q.next();
                        
                        //
                        // Don't return connections for which destruction has
                        // been initiated. The connection must also match the
                        // requested thread-per-connection setting.
                        //
                        if(!connection.isDestroyed() && connection.threadPerConnection() == threadPerConnection)
                        {
                            if(defaultsAndOverrides.overrideCompress)
                            {
                                compress.value = defaultsAndOverrides.overrideCompressValue;
                            }
                            else
                            {
                                compress.value = cep.endpoint.compress();
                            }

                            return connection;
                        }
                    }
                }
            }

            //
            // If some other thread is currently trying to establish a
            // connection to any of our endpoints, we wait until this
            // thread is finished.
            //
            boolean searchAgain = false;
            while(!_destroyed)
            {
                boolean found = false;
                p = connectors.iterator();
                while(p.hasNext())
                {
                    ConnectorEndpointPair cep = (ConnectorEndpointPair)p.next();
                    if(_pending.contains(cep.connector))
                    { 
                        found = true;
                        break;
                    }
                }
                
                if(!found)
                {
                    break;
                }
                
                searchAgain = true;

                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            if(_destroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            //
            // Search for existing connections again if we waited
            // above, as new connections might have been added in the
            // meantime.
            //
            if(searchAgain)
            {
                p = connectors.iterator();
                while(p.hasNext())
                {
                    ConnectorEndpointPair cep = (ConnectorEndpointPair)p.next();

                    java.util.LinkedList connectionList = (java.util.LinkedList)_connections.get(cep.connector);
                    if(connectionList != null)
                    {
                        java.util.Iterator q = connectionList.iterator();
                        
                        while(q.hasNext())
                        {
                            Ice.ConnectionI connection = (Ice.ConnectionI)q.next();
                            
                            //
                            // Don't return connections for which destruction has
                            // been initiated. The connection must also match the
                            // requested thread-per-connection setting.
                            //
                            if(!connection.isDestroyed() && connection.threadPerConnection() == threadPerConnection)
                            {
                                if(defaultsAndOverrides.overrideCompress)
                                {
                                    compress.value = defaultsAndOverrides.overrideCompressValue;
                                }
                                else
                                {
                                    compress.value = cep.endpoint.compress();
                                }

                                return connection;
                            }
                        }
                    }
                }
            }

            //
            // No connection to any of our endpoints exists yet, so we
            // will try to create one. To avoid that other threads try
            // to create connections to the same endpoints, we add our
            // endpoints to _pending.
            //
            p = connectors.iterator();
            while(p.hasNext())
            {
                ConnectorEndpointPair cep = (ConnectorEndpointPair)p.next();
                _pending.add(cep.connector);
            }
        }

        Connector connector = null;
        Ice.ConnectionI connection = null;
        Ice.LocalException exception = null;

        java.util.Iterator p = connectors.iterator();
        while(p.hasNext())
        {
            ConnectorEndpointPair cep = (ConnectorEndpointPair)p.next();
            connector = cep.connector;
            EndpointI endpoint = cep.endpoint;

            try
            {
                int timeout;
                if(defaultsAndOverrides.overrideConnectTimeout)
                {
                    timeout = defaultsAndOverrides.overrideConnectTimeoutValue;
                }
                // It is not necessary to check for overrideTimeout,
                // the endpoint has already been modified with this
                // override, if set.
                else
                {
                    timeout = endpoint.timeout();
                }

                Transceiver transceiver = connector.connect(timeout);
                assert(transceiver != null);
                
                connection =
                    new Ice.ConnectionI(_instance, transceiver, endpoint.compress(false), null, threadPerConnection);
                connection.start();
                connection.validate();

                if(defaultsAndOverrides.overrideCompress)
                {
                    compress.value = defaultsAndOverrides.overrideCompressValue;
                }
                else
                {
                    compress.value = endpoint.compress();
                }
                break;
            }
            catch(Ice.LocalException ex)
            {
                exception = ex;

                //
                // If a connection object was constructed, then validate()
                // must have raised the exception.
                //
                if(connection != null)
                {
                    connection.waitUntilFinished(); // We must call waitUntilFinished() for cleanup.
                    connection = null;
                }
            }
            
            TraceLevels traceLevels = _instance.traceLevels();
            if(traceLevels.retry >= 2)
            {
                StringBuffer s = new StringBuffer();
                s.append("connection to endpoint failed");
                if(hasMore || p.hasNext())
                {
                    s.append(", trying next endpoint\n");
                }
                else
                {
                    s.append(" and no more endpoints to try\n");
                }
                s.append(exception.toString());
                _instance.initializationData().logger.trace(traceLevels.retryCat, s.toString());
            }
        }
        
        synchronized(this)
        {
            //
            // Signal other threads that we are done with trying to
            // establish connections to our endpoints.
            //
            p = connectors.iterator();
            while(p.hasNext())
            {
                ConnectorEndpointPair cep = (ConnectorEndpointPair)p.next();
                _pending.remove(cep.connector);
            }
            notifyAll();
            
            if(connection == null)
            {
                assert(exception != null);
                throw exception;
            }
            else
            {
                java.util.LinkedList connectionList = (java.util.LinkedList)_connections.get(connector);
                if(connectionList == null)
                {
                    connectionList = new java.util.LinkedList();
                    _connections.put(connector, connectionList);
                }
                connectionList.add(connection);

                if(_destroyed)
                {
                    connection.destroy(Ice.ConnectionI.CommunicatorDestroyed);
                    throw new Ice.CommunicatorDestroyedException();
                }
                else
                {
                    connection.activate();
                }
            }
        }
        
        assert(connection != null);
        return connection;
    }

    public synchronized void
    setRouterInfo(IceInternal.RouterInfo routerInfo)
    {
        if(_destroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(routerInfo != null);

        //
        // Search for connections to the router's client proxy
        // endpoints, and update the object adapter for such
        // connections, so that callbacks from the router can be
        // received over such connections.
        //
        Ice.ObjectAdapter adapter = routerInfo.getAdapter();
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        EndpointI[] endpoints = routerInfo.getClientEndpoints();
        for(int i = 0; i < endpoints.length; i++)
        {
            EndpointI endpoint = endpoints[i];

            //
            // Modify endpoints with overrides.
            //
            if(defaultsAndOverrides.overrideTimeout)
            {
                endpoint = endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
            }

            //
            // The Connection object does not take the compression flag of
            // endpoints into account, but instead gets the information
            // about whether messages should be compressed or not from
            // other sources. In order to allow connection sharing for
            // endpoints that differ in the value of the compression flag
            // only, we always set the compression flag to false here in
            // this connection factory.
            //
            endpoint = endpoint.compress(false);

            java.util.Iterator p = _connections.values().iterator();
            while(p.hasNext())
            {
                java.util.LinkedList connectionList = (java.util.LinkedList)p.next();
                
                java.util.Iterator q = connectionList.iterator();
                while(q.hasNext())
                {
                    Ice.ConnectionI connection = (Ice.ConnectionI)q.next();
                    if(connection.endpoint() == endpoint)
                    {
                        try
                        {
                            connection.setAdapter(adapter);
                        }
                        catch(Ice.LocalException ex)
                        {
                            //
                            // Ignore, the connection is being closed or closed.
                            //
                        }
                    }
                }
            }
        }
    }

    public synchronized void
    removeAdapter(Ice.ObjectAdapter adapter)
    {
        if(_destroyed)
        {
            return;
        }

        java.util.Iterator p = _connections.values().iterator();
        while(p.hasNext())
        {
            java.util.LinkedList connectionList = (java.util.LinkedList)p.next();
                
            java.util.Iterator q = connectionList.iterator();
            while(q.hasNext())
            {
                Ice.ConnectionI connection = (Ice.ConnectionI)q.next();
                if(connection.getAdapter() == adapter)
                {
                    try
                    {
                        connection.setAdapter(null);
                    }
                    catch(Ice.LocalException ex)
                    {
                        //
                        // Ignore, the connection is being closed or closed.
                        //
                    }
                }
            }
        }
    }

    public void
    flushBatchRequests()
    {
        java.util.LinkedList c = new java.util.LinkedList();

        synchronized(this)
        {
            java.util.Iterator p = _connections.values().iterator();
            while(p.hasNext())
            {
                java.util.LinkedList connectionList = (java.util.LinkedList)p.next();
                java.util.Iterator q = connectionList.iterator();
                while(q.hasNext())
                {
                    c.add(q.next());
                }
            }
        }

        java.util.Iterator p = c.iterator();
        while(p.hasNext())
        {
            Ice.ConnectionI conn = (Ice.ConnectionI)p.next();
            try
            {
                conn.flushBatchRequests();
            }
            catch(Ice.LocalException ex)
            {
                // Ignore.
            }
        }
    }

    //
    // Only for use by Instance.
    //
    OutgoingConnectionFactory(Instance instance)
    {
        _instance = instance;
        _destroyed = false;
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        IceUtil.Assert.FinalizerAssert(_destroyed);
        IceUtil.Assert.FinalizerAssert(_connections == null);

        super.finalize();
    }

    private final Instance _instance;
    private boolean _destroyed;
    private java.util.HashMap _connections = new java.util.HashMap();
    private java.util.HashSet _pending = new java.util.HashSet();
}
