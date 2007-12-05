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
    interface CreateConnectionCallback
    {
        void setConnection(Ice.ConnectionI connection, boolean compress);
        void setException(Ice.LocalException ex);
    }

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
        java.util.HashMap connections = null;

        synchronized(this)
        {
            //
            // First we wait until the factory is destroyed. We also
            // wait until there are no pending connections
            // anymore. Only then we can be sure the _connections
            // contains all connections.
            //
            while(!_destroyed || !_pending.isEmpty() || !_pendingEndpoints.isEmpty())
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
            if(_connections != null)
            {
                connections = new java.util.HashMap(_connections);
            }
        }
        
        //
        // Now we wait until the destruction of each connection is finished.
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

        synchronized(this)
        {
            //
            // For consistency with C#, we set _connections to null rather than to a
            // new empty list so that our finalizer does not try to invoke any
            // methods on member objects.
            //
            _connections = null;
        }
    }

    public Ice.ConnectionI
    create(EndpointI[] endpts, boolean hasMore, boolean tpc, Ice.EndpointSelectionType selType, 
           Ice.BooleanHolder compress)
    {
        assert(endpts.length > 0);

        //
        // TODO: Remove when we no longer support SSL for JDK 1.4. We can also remove
        //       the threadPerConnection argument.
        //
        for(int i = 0; i < endpts.length; i++)
        {
            if(!tpc && endpts[i].requiresThreadPerConnection())
            {
                Ice.FeatureNotSupportedException ex = new Ice.FeatureNotSupportedException();
                ex.unsupportedFeature = "endpoint requires thread-per-connection:\n" + endpts[i].toString();
                throw ex;
            }
        }

        //
        // Apply the overrides.
        //
        java.util.List endpoints = applyOverrides(endpts);

        //
        // Try to find a connection to one of the given endpoints.
        //
        Ice.ConnectionI connection = findConnection(endpoints, tpc, compress);
        if(connection != null)
        {
            return connection;
        }

        Ice.LocalException exception = null;

        //
        // If we didn't find a connection with the endpoints, we create the connectors
        // for the endpoints.
        //
        java.util.ArrayList connectors = new java.util.ArrayList();
        java.util.Iterator p = endpoints.iterator();
        while(p.hasNext())
        {
            EndpointI endpoint = (EndpointI)p.next();

            //
            // Create connectors for the endpoint.
            //
            try
            {
                java.util.List cons = endpoint.connectors();
                assert(cons.size() > 0);
                
                //
                // Shuffle connectors if endpoint selection type is Random.
                //
                if(selType == Ice.EndpointSelectionType.Random)
                {
                    java.util.Collections.shuffle(cons);
                }
                
                java.util.Iterator q = cons.iterator();
                while(q.hasNext())
                {
                    connectors.add(new ConnectorInfo((Connector)q.next(), endpoint, tpc));
                }
            }
            catch(Ice.LocalException ex)
            {
                exception = ex;
                handleException(exception, hasMore || p.hasNext());
            }
        }
        
        if(connectors.isEmpty())
        {
            assert(exception != null);
            throw exception;
        }
        
        //
        // Try to get a connection to one of the connectors. A null result indicates that no
        // connection was found and that we should try to establish the connection (and that
        // the connectors were added to _pending to prevent other threads from establishing
        // the connection).
        //
        connection = getConnection(connectors, null, compress);
        if(connection != null)
        {
            return connection;
        }

        //
        // Try to establish the connection to the connectors.
        //
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        p = connectors.iterator();
        while(p.hasNext())
        {
            ConnectorInfo ci = (ConnectorInfo)p.next();
            try
            {
                int timeout;
                if(defaultsAndOverrides.overrideConnectTimeout)
                {
                    timeout = defaultsAndOverrides.overrideConnectTimeoutValue;
                }
                else
                {
                    //
                    // It is not necessary to check for overrideTimeout, the endpoint has already 
                    // been modified with this override, if set.
                    //
                    timeout = ci.endpoint.timeout();
                }

                connection = createConnection(ci.connector.connect(timeout), ci);
                connection.start(null);

                if(defaultsAndOverrides.overrideCompress)
                {
                    compress.value = defaultsAndOverrides.overrideCompressValue;
                }
                else
                {
                    compress.value = ci.endpoint.compress();
                }

                break;
            }
            catch(Ice.CommunicatorDestroyedException ex)
            {
                exception = ex;
                handleException(exception, ci, connection, hasMore || p.hasNext());
                connection = null;
                break; // No need to continue
            }
            catch(Ice.LocalException ex)
            {
                exception = ex;
                handleException(exception, ci, connection, hasMore || p.hasNext());
                connection = null;
            }
        }

        //
        // Finish creating the connection (this removes the connectors from the _pending
        // list and notifies any waiting threads).
        //
        finishGetConnection(connectors, null, connection);

        if(connection == null)
        {
            assert(exception != null);
            throw exception;
        }

        return connection;
    }

    public void
    create(EndpointI[] endpts, boolean hasMore, boolean tpc, Ice.EndpointSelectionType selType,
           CreateConnectionCallback callback)
    {
        assert(endpts.length > 0);

        //
        // TODO: Remove when we no longer support SSL for JDK 1.4. We can also remove
        //       the threadPerConnection argument.
        //
        for(int i = 0; i < endpts.length; i++)
        {
            if(!tpc && endpts[i].requiresThreadPerConnection())
            {
                Ice.FeatureNotSupportedException ex = new Ice.FeatureNotSupportedException();
                ex.unsupportedFeature = "endpoint requires thread-per-connection:\n" + endpts[i].toString();
                throw ex;
            }
        }

        //
        // Apply the overrides.
        //
        java.util.List endpoints = applyOverrides(endpts);

        //
        // Try to find a connection to one of the given endpoints.
        //
        Ice.BooleanHolder compress = new Ice.BooleanHolder();
        Ice.ConnectionI connection = findConnection(endpoints, tpc, compress);
        if(connection != null)
        {
            callback.setConnection(connection, compress.value);
            return;
        }

        ConnectCallback cb = new ConnectCallback(this, endpoints, hasMore, callback, selType, tpc);
        cb.getConnection();
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

    private java.util.List
    applyOverrides(EndpointI[] endpts)
    {
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        java.util.ArrayList endpoints = new java.util.ArrayList();
        for(int i = 0; i < endpts.length; i++)
        {
            //
            // Modify endpoints with overrides.
            //
            if(defaultsAndOverrides.overrideTimeout)
            {
                endpoints.add(endpts[i].timeout(defaultsAndOverrides.overrideTimeoutValue));
            }
            else
            {
                endpoints.add(endpts[i]);
            }
        }

        return endpoints;
    }

    synchronized private Ice.ConnectionI
    findConnection(java.util.List endpoints, boolean tpc, Ice.BooleanHolder compress)
    {
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        assert(!endpoints.isEmpty());

        java.util.Iterator p = endpoints.iterator();
        while(p.hasNext())
        {
            EndpointI endpoint = (EndpointI)p.next();
            java.util.LinkedList connectionList = (java.util.LinkedList)_connectionsByEndpoint.get(endpoint);
            if(connectionList == null)
            {
                continue;
            }
            
            java.util.Iterator q = connectionList.iterator();
            while(q.hasNext())
            {
                Ice.ConnectionI connection = (Ice.ConnectionI)q.next();
                if(connection.isActiveOrHolding() &&
                   connection.threadPerConnection() == tpc) // Don't return destroyed or un-validated connections
                {
                    if(defaultsAndOverrides.overrideCompress)
                    {
                        compress.value = defaultsAndOverrides.overrideCompressValue;
                    }
                    else
                    {
                        compress.value = endpoint.compress();
                    }
                    return connection;
                }
            }
        }
        
        return null;
    }

    //
    // Must be called while synchronized.
    //
    private Ice.ConnectionI
    findConnection(java.util.List connectors, Ice.BooleanHolder compress)
    {
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        java.util.Iterator p = connectors.iterator();
        while(p.hasNext())
        {
            ConnectorInfo ci = (ConnectorInfo)p.next();
            java.util.LinkedList connectionList = (java.util.LinkedList)_connections.get(ci);
            if(connectionList == null)
            {
                continue;
            }
            
            java.util.Iterator q = connectionList.iterator();
            while(q.hasNext())
            {
                Ice.ConnectionI connection = (Ice.ConnectionI)q.next();
                if(connection.isActiveOrHolding()) // Don't return destroyed or un-validated connections
                {
                    if(connection.endpoint().equals(ci.endpoint))
                    {
                        java.util.List conList = (java.util.LinkedList)_connectionsByEndpoint.get(ci.endpoint);
                        if(conList == null)
                        {
                            conList = new java.util.LinkedList();
                            _connectionsByEndpoint.put(ci.endpoint, conList);
                        }
                        conList.add(connection);
                    }
                    
                    if(defaultsAndOverrides.overrideCompress)
                    {
                        compress.value = defaultsAndOverrides.overrideCompressValue;
                    }
                    else
                    {
                        compress.value = ci.endpoint.compress();
                    }
                    return connection;
                }
            }
        }
        
        return null;
    }

    synchronized private void
    addPendingEndpoints(java.util.List endpoints)
    {
        if(_destroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }
        _pendingEndpoints.addAll(endpoints);
    }

    synchronized private void
    removePendingEndpoints(java.util.List endpoints)
    {
        java.util.Iterator p = endpoints.iterator();
        while(p.hasNext())
        {
            _pendingEndpoints.remove(p.next());
        }

        if(_destroyed)
        {
            notifyAll();
        }
    }

    private Ice.ConnectionI
    getConnection(java.util.List connectors, ConnectCallback cb, Ice.BooleanHolder compress)
    {
        synchronized(this)
        {
            if(_destroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
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

            p = _connectionsByEndpoint.values().iterator();
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
            
            //
            // Try to get the connection. We may need to wait for other threads to
            // finish if one of them is currently establishing a connection to one
            // of our connectors.
            //
            while(!_destroyed)
            {
                //
                // Search for a matching connection. If we find one, we're done.
                //
                Ice.ConnectionI connection = findConnection(connectors, compress);
                if(connection != null)
                {
                    if(cb != null)
                    {
                        //
                        // This might not be the first getConnection call for the callback. We need
                        // to ensure that the callback isn't registered with any other pending 
                        // connectors since we just found a connection and therefore don't need to
                        // wait anymore for other pending connectors.
                        // 
                        p = connectors.iterator();
                        while(p.hasNext())
                        {
                            java.util.Set cbs = (java.util.Set)_pending.get(p.next());
                            if(cbs != null)
                            {
                                cbs.remove(cb);
                            }
                        }
                    }
                    return connection;
                }

                //
                // Determine whether another thread is currently attempting to connect to one of our endpoints;
                // if so we wait until it's done.
                //
                p = connectors.iterator();
                boolean found = false;
                while(p.hasNext())
                {
                    java.util.Set cbs = (java.util.Set)_pending.get(p.next());
                    if(cbs != null)
                    {
                        found = true;
                        if(cb != null)
                        {
                            cbs.add(cb); // Add the callback to each pending connector.
                        }
                    }
                }
                
                if(!found)
                {
                    //
                    // If no thread is currently establishing a connection to one of our connectors,
                    // we get out of this loop and start the connection establishment to one of the
                    // given connectors.
                    //
                    break;
                }
                else
                {
                    //
                    // If a callback is not specified we wait until another thread notifies us about a 
                    // change to the pending list. Otherwise, if a callback is provided we're done: 
                    // when the pending list changes the callback will be notified and will try to 
                    // get the connection again.
                    //
                    if(cb == null)
                    {
                        try
                        {
                            wait();
                        }
                        catch(InterruptedException ex)
                        {
                        }
                    }
                    else
                    {
                        return null;
                    }
                }
            }
            
            if(_destroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }
            
            //
            // No connection to any of our endpoints exists yet; we add the given connectors to
            // the _pending set to indicate that we're attempting connection establishment to 
            // these connectors.
            //
            p = connectors.iterator();
            while(p.hasNext())
            {
                _pending.put(p.next(), new java.util.HashSet());
            }
        }

        //
        // At this point, we're responsible for establishing the connection to one of 
        // the given connectors. If it's a non-blocking connect, calling nextConnector
        // will start the connection establishment. Otherwise, we return null to get
        // the caller to establish the connection.
        //
        if(cb != null)
        {
            cb.nextConnector();
        }

        return null;
    }

    private synchronized Ice.ConnectionI
    createConnection(Transceiver transceiver, ConnectorInfo ci)
    {
        assert(_pending.containsKey(ci) && transceiver != null);

        //
        // Create and add the connection to the connection map. Adding the connection to the map
        // is necessary to support the interruption of the connection initialization and validation
        // in case the communicator is destroyed.
        //
	try
	{
            if(_destroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

	    Ice.ConnectionI connection = new Ice.ConnectionI(_instance, transceiver, ci.endpoint.compress(false),
                                                             null, ci.threadPerConnection);

            java.util.LinkedList connectionList = (java.util.LinkedList)_connections.get(ci);
            if(connectionList == null)
            {
                connectionList = new java.util.LinkedList();
                _connections.put(ci, connectionList);
            }
            connectionList.add(connection);
            return connection;
	}
	catch(Ice.LocalException ex)
	{
	    try
	    {
		transceiver.close();
	    }
	    catch(Ice.LocalException exc)
	    {
		// Ignore
	    }
            throw ex;
	}
    }

    private void
    finishGetConnection(java.util.List connectors, ConnectCallback cb, Ice.ConnectionI connection)
    {
        java.util.Set callbacks = new java.util.HashSet();

        synchronized(this)
        {
            //
            // We're done trying to connect to the given connectors so we remove the 
            // connectors from the pending list and notify waiting threads. We also 
            // notify the pending connect callbacks (outside the synchronization).
            //

            java.util.Iterator p = connectors.iterator();
            while(p.hasNext())
            {
                callbacks.addAll((java.util.Set)_pending.remove(p.next()));
            }
            notifyAll();

            //
            // If the connect attempt succeeded and the communicator is not destroyed,
            // activate the connection!
            //
            if(connection != null && !_destroyed)
            {
                connection.activate();
            }
        }
        
        //
        // Notify any waiting callbacks.
        //
        java.util.Iterator p = callbacks.iterator();
        while(p.hasNext())
        {
            ((ConnectCallback)p.next()).getConnection();
        }
    }

    private void
    handleException(Ice.LocalException ex, ConnectorInfo ci, Ice.ConnectionI connection, boolean hasMore)
    {
        TraceLevels traceLevels = _instance.traceLevels();
        if(traceLevels.retry >= 2)
        {
            StringBuffer s = new StringBuffer();
            s.append("connection to endpoint failed");
            if(ex instanceof Ice.CommunicatorDestroyedException)
            {
                s.append("\n");
            }
            else
            {
                if(hasMore)
                {
                    s.append(", trying next endpoint\n");
                }
                else
                {
                    s.append(" and no more endpoints to try\n");
                }
            }
            s.append(ex.toString());
            _instance.initializationData().logger.trace(traceLevels.retryCat, s.toString());
        }

        if(connection != null && connection.isFinished())
        {
            //
            // If the connection is finished, we remove it right away instead of
            // waiting for the reaping.
            //
            // NOTE: it's possible for the connection to not be finished yet. That's
            // for instance the case when using thread per connection and if it's the
            // thread which is calling back the outgoing connection factory to notify
            // it of the failure.
            //
            synchronized(this)
            {
                java.util.LinkedList connectionList = (java.util.LinkedList)_connections.get(ci);
                if(connectionList != null) // It might have already been reaped!
                {
                    connectionList.remove(connection);
                    if(connectionList.isEmpty())
                    {
                        _connections.remove(ci);
                    }
                }
            }
        }
    }

    private void
    handleException(Ice.LocalException ex, boolean hasMore)
    {
        TraceLevels traceLevels = _instance.traceLevels();
        if(traceLevels.retry >= 2)
        {
            StringBuffer s = new StringBuffer();
            s.append("couldn't resolve endpoint host");
            if(ex instanceof Ice.CommunicatorDestroyedException)
            {
                s.append("\n");
            }
            else
            {
                if(hasMore)
                {
                    s.append(", trying next endpoint\n");
                }
                else
                {
                    s.append(" and no more endpoints to try\n");
                }
            }
            s.append(ex.toString());
            _instance.initializationData().logger.trace(traceLevels.retryCat, s.toString());
        }
    }

    private static class ConnectorInfo
    {
        public ConnectorInfo(Connector c, EndpointI e, boolean t)
        {
            connector = c;
            endpoint = e;
            threadPerConnection = t;
        }

        public boolean 
        equals(Object obj)
        {
            ConnectorInfo r = (ConnectorInfo)obj;
            if(threadPerConnection != r.threadPerConnection)
            {
                return false;
            }
            return connector.equals(r.connector);
        }

        public int
        hashCode()
        {
            return 2 * connector.hashCode() + (threadPerConnection ? 0 : 1);
        }

        public Connector connector;
        public EndpointI endpoint;
        public boolean threadPerConnection;
    }

    private static class ConnectCallback implements Ice.ConnectionI.StartCallback, EndpointI_connectors,
                                         ThreadPoolWorkItem
    {
        ConnectCallback(OutgoingConnectionFactory f, java.util.List endpoints, boolean more, 
                        CreateConnectionCallback cb, Ice.EndpointSelectionType selType, boolean threadPerConnection)
        {
            _factory = f;
            _endpoints = endpoints;
            _hasMore = more;
            _callback = cb;
            _selType = selType;
            _threadPerConnection = threadPerConnection;
            _endpointsIter = _endpoints.iterator();
        }

        //
        // Methods from ConnectionI.StartCallback
        //
        public synchronized void 
        connectionStartCompleted(Ice.ConnectionI connection)
        {
            assert(_exception == null && connection == _connection);

            boolean compress;
            DefaultsAndOverrides defaultsAndOverrides = _factory._instance.defaultsAndOverrides();
            if(defaultsAndOverrides.overrideCompress)
            {
                compress = defaultsAndOverrides.overrideCompressValue;
            }
            else
            {
                compress = _current.endpoint.compress();
            }
            
            _factory.finishGetConnection(_connectors, this, connection);
            _factory.removePendingEndpoints(_endpoints);
            _callback.setConnection(connection, compress);
        }

        public synchronized void
        connectionStartFailed(Ice.ConnectionI connection, Ice.LocalException ex)
        {
            assert(_exception == null && connection == _connection);

            _exception = ex;
            handleException();
        }

        //
        // Methods from EndpointI_connectors
        //
        public void
        connectors(java.util.List cons)
        {
            //
            // Shuffle connectors if endpoint selection type is Random.
            //
            if(_selType == Ice.EndpointSelectionType.Random)
            {
                java.util.Collections.shuffle(cons);
            }
                
            java.util.Iterator q = cons.iterator();
            while(q.hasNext())
            {
                _connectors.add(new ConnectorInfo((Connector)q.next(), _currentEndpoint, _threadPerConnection));
            }

            if(_endpointsIter.hasNext())
            {
                _currentEndpoint = (EndpointI)_endpointsIter.next();
                _currentEndpoint.connectors_async(this);
            }
            else
            {
                assert(!_connectors.isEmpty());
                
                //
                // We now have all the connectors for the given endpoints. We can try to obtain the
                // connection.
                //
                _iter = _connectors.iterator();
                getConnection();
            }
        }
        
        public void
        exception(Ice.LocalException ex)
        {
            _factory.handleException(ex, _hasMore || _endpointsIter.hasNext());
            if(_endpointsIter.hasNext())
            {
                _currentEndpoint = (EndpointI)_endpointsIter.next();
                _currentEndpoint.connectors_async(this);
            }
            else if(!_connectors.isEmpty())
            {
                //
                // We now have all the connectors for the given endpoints. We can try to obtain the
                // connection.
                //
                _iter = _connectors.iterator();
                getConnection();
            }
            else
            {
                _exception = ex;
                _factory._instance.clientThreadPool().execute(this);
            }
        }

        //
        // Methods from ThreadPoolWorkItem
        //
        public void
        execute(ThreadPool threadPool)
        {
            threadPool.promoteFollower();
            assert(_exception != null);
            _factory.removePendingEndpoints(_endpoints);
            _callback.setException(_exception);
        }

        void
        getConnection()
        {
            //
            // First, get the connectors for all the endpoints.
            //
            if(_endpointsIter.hasNext())
            {
                try
                {
                    _factory.addPendingEndpoints(_endpoints);
                    _currentEndpoint = (EndpointI)_endpointsIter.next();
                    _currentEndpoint.connectors_async(this);
                }
                catch(Ice.LocalException ex)
                {
                    _callback.setException(ex);
                }
                return;
            }

            try
            {
                Ice.BooleanHolder compress = new Ice.BooleanHolder();
                Ice.ConnectionI connection = _factory.getConnection(_connectors, this, compress);
                if(connection == null)
                {
                    //
                    // A null return value from getConnection indicates that the connection
                    // is being established and that everthing has been done to ensure that
                    // the callback will be notified when the connection establishment is 
                    // done.
                    // 
                    return;
                }
                
                _factory.removePendingEndpoints(_endpoints);
                _callback.setConnection(connection, compress.value);
            }
            catch(Ice.LocalException ex)
            {
                _exception = ex;
                _factory._instance.clientThreadPool().execute(this);
            }
        }

        void
        nextConnector()
        {
            _current = (ConnectorInfo)_iter.next();
            try
            {
                _exception = null;
                _connection = _factory.createConnection(_current.connector.connect(0), _current);
                _connection.start(this);
            }
            catch(Ice.LocalException ex)
            {
                _exception = ex;
                handleException();
            }
        }

        private void
        handleException()
        {
            assert(_current != null && _exception != null);

            _factory.handleException(_exception, _current, _connection, _hasMore || _iter.hasNext());
            if(_exception instanceof Ice.CommunicatorDestroyedException) // No need to continue.
            {
                _factory.finishGetConnection(_connectors, this, null);
                _factory.removePendingEndpoints(_endpoints);
                _callback.setException(_exception);                
            }
            else if(_iter.hasNext()) // Try the next connector.
            {
                nextConnector();
            }
            else
            {
                _factory.finishGetConnection(_connectors, this, null);
                _factory.removePendingEndpoints(_endpoints);
                _callback.setException(_exception);
            }
        }

        private final OutgoingConnectionFactory _factory;
        private final boolean _hasMore;
        private final CreateConnectionCallback _callback;
        private final java.util.List _endpoints;
        private final Ice.EndpointSelectionType _selType;
        private final boolean _threadPerConnection;
        private java.util.Iterator _endpointsIter;
        private EndpointI _currentEndpoint;
        private java.util.List _connectors = new java.util.ArrayList();
        private java.util.Iterator _iter;
        private ConnectorInfo _current;
        private Ice.LocalException _exception;
        private Ice.ConnectionI _connection;
    }

    private final Instance _instance;
    private boolean _destroyed;

    private java.util.HashMap _connections = new java.util.HashMap();
    private java.util.HashMap _pending = new java.util.HashMap();

    private java.util.HashMap _connectionsByEndpoint = new java.util.HashMap();
    private java.util.LinkedList _pendingEndpoints = new java.util.LinkedList();
}
