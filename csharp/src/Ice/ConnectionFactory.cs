// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Text;

    public class MultiDictionary<K, V> : Dictionary<K, ICollection<V>>
    {
        public void
        Add(K key, V value)
        {
            ICollection<V> list = null;
            if(!TryGetValue(key, out list))
            {
                list = new List<V>();
                Add(key, list);
            }
            list.Add(value);
        }

        public void
        Remove(K key, V value)
        {
            ICollection<V> list = this[key];
            list.Remove(value);
            if(list.Count == 0)
            {
                Remove(key);
            }
        }
    }

    public sealed class OutgoingConnectionFactory
    {
        public interface CreateConnectionCallback
        {
            void setConnection(Ice.ConnectionI connection, bool compress);
            void setException(Ice.LocalException ex);
        }

        public void destroy()
        {
            lock(this)
            {
                if(_destroyed)
                {
                    return;
                }

                foreach(ICollection<Ice.ConnectionI> connections in _connections.Values)
                {
                    foreach(Ice.ConnectionI c in connections)
                    {
                        c.destroy(Ice.ConnectionI.CommunicatorDestroyed);
                    }
                }

                _destroyed = true;
                _communicator = null;
                System.Threading.Monitor.PulseAll(this);
            }
        }

        public void updateConnectionObservers()
        {
            lock(this)
            {
                foreach(ICollection<Ice.ConnectionI> connections in _connections.Values)
                {
                    foreach(Ice.ConnectionI c in connections)
                    {
                        c.updateObserver();
                    }
                }
            }
        }

        public void waitUntilFinished()
        {
            Dictionary<Connector, ICollection<Ice.ConnectionI>> connections = null;
            lock(this)
            {
                //
                // First we wait until the factory is destroyed. We also
                // wait until there are no pending connections
                // anymore. Only then we can be sure the _connections
                // contains all connections.
                //
                while(!_destroyed || _pending.Count > 0 || _pendingConnectCount > 0)
                {
                    System.Threading.Monitor.Wait(this);
                }

                //
                // We want to wait until all connections are finished outside the
                // thread synchronization.
                //
                connections = new Dictionary<Connector, ICollection<Ice.ConnectionI>>(_connections);
            }

            //
            // Now we wait until the destruction of each connection is finished.
            //
            foreach(ICollection<Ice.ConnectionI> cl in connections.Values)
            {
                foreach(Ice.ConnectionI c in cl)
                {
                    c.waitUntilFinished();
                }
            }

            lock(this)
            {
                // Ensure all the connections are finished and reapable at this point.
                ICollection<Ice.ConnectionI> cons = _monitor.swapReapedConnections();
                if(cons != null)
                {
                    int size = 0;
                    foreach(ICollection<Ice.ConnectionI> cl in _connections.Values)
                    {
                        size += cl.Count;
                    }
                    Debug.Assert(cons.Count == size);
                    _connections.Clear();
                    _connectionsByEndpoint.Clear();
                }
                else
                {
                    Debug.Assert(_connections.Count == 0);
                    Debug.Assert(_connectionsByEndpoint.Count == 0);
                }
            }

            //
            // Must be destroyed outside the synchronization since this might block waiting for
            // a timer task to execute.
            //
            _monitor.destroy();
        }

        public void create(EndpointI[] endpts, bool hasMore, Ice.EndpointSelectionType selType,
                           CreateConnectionCallback callback)
        {
            Debug.Assert(endpts.Length > 0);

            //
            // Apply the overrides.
            //
            List<EndpointI> endpoints = applyOverrides(endpts);

            //
            // Try to find a connection to one of the given endpoints.
            //
            try
            {
                bool compress;
                Ice.ConnectionI connection = findConnection(endpoints, out compress);
                if(connection != null)
                {
                    callback.setConnection(connection, compress);
                    return;
                }
            }
            catch(Ice.LocalException ex)
            {
                callback.setException(ex);
                return;
            }

            ConnectCallback cb = new ConnectCallback(this, endpoints, hasMore, callback, selType);
            cb.getConnectors();
        }

        public void setRouterInfo(IceInternal.RouterInfo routerInfo)
        {
            Debug.Assert(routerInfo != null);
            Ice.ObjectAdapter adapter = routerInfo.getAdapter();
            EndpointI[] endpoints = routerInfo.getClientEndpoints(); // Must be called outside the synchronization

            lock(this)
            {
                if(_destroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                //
                // Search for connections to the router's client proxy
                // endpoints, and update the object adapter for such
                // connections, so that callbacks from the router can be
                // received over such connections.
                //
                DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
                for(int i = 0; i < endpoints.Length; i++)
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
                    // The Ice.ConnectionI object does not take the compression flag of
                    // endpoints into account, but instead gets the information
                    // about whether messages should be compressed or not from
                    // other sources. In order to allow connection sharing for
                    // endpoints that differ in the value of the compression flag
                    // only, we always set the compression flag to false here in
                    // this connection factory.
                    //
                    endpoint = endpoint.compress(false);

                    foreach(ICollection<Ice.ConnectionI> connections in _connections.Values)
                    {
                        foreach(Ice.ConnectionI connection in connections)
                        {
                            if(connection.endpoint().Equals(endpoint))
                            {
                                connection.setAdapter(adapter);
                            }
                        }
                    }
                }
            }
        }

        public void removeAdapter(Ice.ObjectAdapter adapter)
        {
            lock(this)
            {
                if(_destroyed)
                {
                    return;
                }

                foreach(ICollection<Ice.ConnectionI> connectionList in _connections.Values)
                {
                    foreach(Ice.ConnectionI connection in connectionList)
                    {
                        if(connection.getAdapter() == adapter)
                        {
                            connection.setAdapter(null);
                        }
                    }
                }
            }
        }

        public void flushAsyncBatchRequests(Ice.CompressBatch compressBatch, CommunicatorFlushBatchAsync outAsync)
        {
            ICollection<Ice.ConnectionI> c = new List<Ice.ConnectionI>();

            lock(this)
            {
                if(!_destroyed)
                {
                    foreach(ICollection<Ice.ConnectionI> connectionList in _connections.Values)
                    {
                        foreach(Ice.ConnectionI conn in connectionList)
                        {
                            if(conn.isActiveOrHolding())
                            {
                                c.Add(conn);
                            }
                        }
                    }
                }
            }

            foreach(Ice.ConnectionI conn in c)
            {
                try
                {
                    outAsync.flushConnection(conn, compressBatch);
                }
                catch(Ice.LocalException)
                {
                    // Ignore.
                }
            }
        }

        //
        // Only for use by Instance.
        //
        internal OutgoingConnectionFactory(Ice.Communicator communicator, Instance instance)
        {
            _communicator = communicator;
            _instance = instance;
            _destroyed = false;
            _monitor = new FactoryACMMonitor(instance, instance.clientACM());
            _pendingConnectCount = 0;
        }

        private List<EndpointI> applyOverrides(EndpointI[] endpts)
        {
            DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
            List<EndpointI> endpoints = new List<EndpointI>();
            for(int i = 0; i < endpts.Length; i++)
            {
                //
                // Modify endpoints with overrides.
                //
                if(defaultsAndOverrides.overrideTimeout)
                {
                    endpoints.Add(endpts[i].timeout(defaultsAndOverrides.overrideTimeoutValue));
                }
                else
                {
                    endpoints.Add(endpts[i]);
                }
            }

            return endpoints;
        }

        private Ice.ConnectionI findConnection(List<EndpointI> endpoints, out bool compress)
        {
            lock(this)
            {
                if(_destroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
                Debug.Assert(endpoints.Count > 0);

                foreach(EndpointI endpoint in endpoints)
                {
                    ICollection<Ice.ConnectionI> connectionList = null;
                    if(!_connectionsByEndpoint.TryGetValue(endpoint, out connectionList))
                    {
                        continue;
                    }

                    foreach(Ice.ConnectionI connection in connectionList)
                    {
                        if(connection.isActiveOrHolding()) // Don't return destroyed or unvalidated connections
                        {
                            if(defaultsAndOverrides.overrideCompress)
                            {
                                compress = defaultsAndOverrides.overrideCompressValue;
                            }
                            else
                            {
                                compress = endpoint.compress();
                            }
                            return connection;
                        }
                    }
                }

                compress = false; // Satisfy the compiler
                return null;
            }
        }

        //
        // Must be called while synchronized.
        //
        private Ice.ConnectionI findConnection(List<ConnectorInfo> connectors, out bool compress)
        {
            DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
            foreach(ConnectorInfo ci in connectors)
            {
                if(_pending.ContainsKey(ci.connector))
                {
                    continue;
                }

                ICollection<Ice.ConnectionI> connectionList = null;
                if(!_connections.TryGetValue(ci.connector, out connectionList))
                {
                    continue;
                }

                foreach(Ice.ConnectionI connection in connectionList)
                {
                    if(connection.isActiveOrHolding()) // Don't return destroyed or un-validated connections
                    {
                        if(defaultsAndOverrides.overrideCompress)
                        {
                            compress = defaultsAndOverrides.overrideCompressValue;
                        }
                        else
                        {
                            compress = ci.endpoint.compress();
                        }
                        return connection;
                    }
                }
            }

            compress = false; // Satisfy the compiler
            return null;
        }

        internal void incPendingConnectCount()
        {
            //
            // Keep track of the number of pending connects. The outgoing connection factory
            // waitUntilFinished() method waits for all the pending connects to terminate before
            // to return. This ensures that the communicator client thread pool isn't destroyed
            // too soon and will still be available to execute the ice_exception() callbacks for
            // the asynchronous requests waiting on a connection to be established.
            //

            lock(this)
            {
                if(_destroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                ++_pendingConnectCount;
            }
        }

        internal void decPendingConnectCount()
        {
            lock(this)
            {
                --_pendingConnectCount;
                Debug.Assert(_pendingConnectCount >= 0);
                if(_destroyed && _pendingConnectCount == 0)
                {
                    System.Threading.Monitor.PulseAll(this);
                }
            }
        }

        private Ice.ConnectionI getConnection(List<ConnectorInfo> connectors, ConnectCallback cb, out bool compress)
        {
            lock(this)
            {
                if(_destroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                //
                // Reap closed connections
                //
                ICollection<Ice.ConnectionI> cons = _monitor.swapReapedConnections();
                if(cons != null)
                {
                    foreach(Ice.ConnectionI c in cons)
                    {
                        _connections.Remove(c.connector(), c);
                        _connectionsByEndpoint.Remove(c.endpoint(), c);
                        _connectionsByEndpoint.Remove(c.endpoint().compress(true), c);
                    }
                }

                //
                // Try to get the connection. We may need to wait for other threads to
                // finish if one of them is currently establishing a connection to one
                // of our connectors.
                //
                while(true)
                {
                    if(_destroyed)
                    {
                        throw new Ice.CommunicatorDestroyedException();
                    }

                    //
                    // Search for a matching connection. If we find one, we're done.
                    //
                    Ice.ConnectionI connection = findConnection(connectors, out compress);
                    if(connection != null)
                    {
                        return connection;
                    }

                    if(addToPending(cb, connectors))
                    {
                        //
                        // If a callback is not specified we wait until another thread notifies us about a
                        // change to the pending list. Otherwise, if a callback is provided we're done:
                        // when the pending list changes the callback will be notified and will try to
                        // get the connection again.
                        //
                        if(cb == null)
                        {
                            System.Threading.Monitor.Wait(this);
                        }
                        else
                        {
                            return null;
                        }
                    }
                    else
                    {
                        //
                        // If no thread is currently establishing a connection to one of our connectors,
                        // we get out of this loop and start the connection establishment to one of the
                        // given connectors.
                        //
                        break;
                    }
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

            compress = false; // Satisfy the compiler
            return null;
        }

        private Ice.ConnectionI createConnection(Transceiver transceiver, ConnectorInfo ci)
        {
            lock(this)
            {
                Debug.Assert(_pending.ContainsKey(ci.connector) && transceiver != null);

                //
                // Create and add the connection to the connection map. Adding the connection to the map
                // is necessary to support the interruption of the connection initialization and validation
                // in case the communicator is destroyed.
                //
                Ice.ConnectionI connection;
                try
                {
                    if(_destroyed)
                    {
                        throw new Ice.CommunicatorDestroyedException();
                    }

                    connection = new Ice.ConnectionI(_communicator, _instance, _monitor, transceiver, ci.connector,
                                                     ci.endpoint.compress(false), null);
                }
                catch(Ice.LocalException)
                {
                    try
                    {
                        transceiver.close();
                    }
                    catch(Ice.LocalException)
                    {
                        // Ignore
                    }
                    throw;
                }

                _connections.Add(ci.connector, connection);
                _connectionsByEndpoint.Add(connection.endpoint(), connection);
                _connectionsByEndpoint.Add(connection.endpoint().compress(true), connection);
                return connection;
            }
        }

        private void finishGetConnection(List<ConnectorInfo> connectors,
                                         ConnectorInfo ci,
                                         Ice.ConnectionI connection,
                                         ConnectCallback cb)
        {
            HashSet<ConnectCallback> connectionCallbacks = new HashSet<ConnectCallback>();
            if(cb != null)
            {
                connectionCallbacks.Add(cb);
            }

            HashSet<ConnectCallback> callbacks = new HashSet<ConnectCallback>();
            lock(this)
            {
                foreach(ConnectorInfo c in connectors)
                {
                    HashSet<ConnectCallback> s = null;
                    if(_pending.TryGetValue(c.connector, out s))
                    {
                        foreach(ConnectCallback cc in s)
                        {
                            if(cc.hasConnector(ci))
                            {
                                connectionCallbacks.Add(cc);
                            }
                            else
                            {
                                callbacks.Add(cc);
                            }
                        }
                        _pending.Remove(c.connector);
                    }
                }

                foreach(ConnectCallback cc in connectionCallbacks)
                {
                    cc.removeFromPending();
                    callbacks.Remove(cc);
                }
                foreach(ConnectCallback cc in callbacks)
                {
                    cc.removeFromPending();
                }
                System.Threading.Monitor.PulseAll(this);
            }

            bool compress;
            DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
            if(defaultsAndOverrides.overrideCompress)
            {
                compress = defaultsAndOverrides.overrideCompressValue;
            }
            else
            {
                compress = ci.endpoint.compress();
            }

            foreach(ConnectCallback cc in callbacks)
            {
                cc.getConnection();
            }
            foreach(ConnectCallback cc in connectionCallbacks)
            {
                cc.setConnection(connection, compress);
            }
        }

        private void finishGetConnection(List<ConnectorInfo> connectors, Ice.LocalException ex, ConnectCallback cb)
        {
            HashSet<ConnectCallback> failedCallbacks = new HashSet<ConnectCallback>();
            if(cb != null)
            {
                failedCallbacks.Add(cb);
            }

            HashSet<ConnectCallback> callbacks = new HashSet<ConnectCallback>();
            lock(this)
            {
                foreach(ConnectorInfo c in connectors)
                {
                    HashSet<ConnectCallback> s = null;
                    if(_pending.TryGetValue(c.connector, out s))
                    {
                        foreach(ConnectCallback cc in s)
                        {
                            if(cc.removeConnectors(connectors))
                            {
                                failedCallbacks.Add(cc);
                            }
                            else
                            {
                                callbacks.Add(cc);
                            }
                        }
                        _pending.Remove(c.connector);
                    }
                }

                foreach(ConnectCallback cc in callbacks)
                {
                    Debug.Assert(!failedCallbacks.Contains(cc));
                    cc.removeFromPending();
                }
                System.Threading.Monitor.PulseAll(this);
            }

            foreach(ConnectCallback cc in callbacks)
            {
                cc.getConnection();
            }
            foreach(ConnectCallback cc in failedCallbacks)
            {
                cc.setException(ex);
            }
        }

        private void handleConnectionException(Ice.LocalException ex, bool hasMore)
        {
            TraceLevels traceLevels = _instance.traceLevels();
            if(traceLevels.network >= 2)
            {
                StringBuilder s = new StringBuilder();
                s.Append("connection to endpoint failed");
                if(ex is Ice.CommunicatorDestroyedException)
                {
                    s.Append("\n");
                }
                else
                {
                    if(hasMore)
                    {
                        s.Append(", trying next endpoint\n");
                    }
                    else
                    {
                        s.Append(" and no more endpoints to try\n");
                    }
                }
                s.Append(ex);
                _instance.initializationData().logger.trace(traceLevels.networkCat, s.ToString());
            }
        }

        private bool
        addToPending(ConnectCallback cb, List<ConnectorInfo> connectors)
        {
            //
            // Add the callback to each connector pending list.
            //
            bool found = false;
            foreach(ConnectorInfo ci in connectors)
            {
                HashSet<ConnectCallback> cbs = null;
                if(_pending.TryGetValue(ci.connector, out cbs))
                {
                    found = true;
                    if(cb != null)
                    {
                        cbs.Add(cb); // Add the callback to each pending connector.
                    }
                }
            }

            if(found)
            {
                return true;
            }

            //
            // If there's no pending connection for the given connectors, we're
            // responsible for its establishment. We add empty pending lists,
            // other callbacks to the same connectors will be queued.
            //
            foreach(ConnectorInfo ci in connectors)
            {
                if(!_pending.ContainsKey(ci.connector))
                {
                    _pending.Add(ci.connector, new HashSet<ConnectCallback>());
                }
            }
            return false;
        }

        private void
        removeFromPending(ConnectCallback cb, List<ConnectorInfo> connectors)
        {
            foreach(ConnectorInfo ci in connectors)
            {
                HashSet<ConnectCallback> cbs = null;
                if(_pending.TryGetValue(ci.connector, out cbs))
                {
                    cbs.Remove(cb);
                }
            }
        }

        internal void handleException(Ice.LocalException ex, bool hasMore)
        {
            TraceLevels traceLevels = _instance.traceLevels();
            if(traceLevels.network >= 2)
            {
                StringBuilder s = new StringBuilder();
                s.Append("couldn't resolve endpoint host");
                if(ex is Ice.CommunicatorDestroyedException)
                {
                    s.Append("\n");
                }
                else
                {
                    if(hasMore)
                    {
                        s.Append(", trying next endpoint\n");
                    }
                    else
                    {
                        s.Append(" and no more endpoints to try\n");
                    }
                }
                s.Append(ex);
                _instance.initializationData().logger.trace(traceLevels.networkCat, s.ToString());
            }
        }

        private class ConnectorInfo
        {
            internal ConnectorInfo(Connector c, EndpointI e)
            {
                connector = c;
                endpoint = e;
            }

            public override bool Equals(object obj)
            {
                ConnectorInfo r = (ConnectorInfo)obj;
                return connector.Equals(r.connector);
            }

            public override int GetHashCode()
            {
                return connector.GetHashCode();
            }

            public Connector connector;
            public EndpointI endpoint;
        }

        private class ConnectCallback : Ice.ConnectionI.StartCallback, EndpointI_connectors
        {
            internal ConnectCallback(OutgoingConnectionFactory f, List<EndpointI> endpoints, bool more,
                                     CreateConnectionCallback cb, Ice.EndpointSelectionType selType)
            {
                _factory = f;
                _endpoints = endpoints;
                _hasMore = more;
                _callback = cb;
                _selType = selType;
                _endpointsIter = 0;
            }

            //
            // Methods from ConnectionI.StartCallback
            //
            public void connectionStartCompleted(Ice.ConnectionI connection)
            {
                if(_observer != null)
                {
                    _observer.detach();
                }
                connection.activate();
                _factory.finishGetConnection(_connectors, _current, connection, this);
            }

            public void connectionStartFailed(Ice.ConnectionI connection, Ice.LocalException ex)
            {
                if(connectionStartFailedImpl(ex))
                {
                    nextConnector();
                }
            }

            //
            // Methods from EndpointI_connectors
            //
            public void connectors(List<Connector> cons)
            {
                foreach(Connector connector in cons)
                {
                    _connectors.Add(new ConnectorInfo(connector, _currentEndpoint));
                }

                if(_endpointsIter < _endpoints.Count)
                {
                    nextEndpoint();
                }
                else
                {
                    Debug.Assert(_connectors.Count > 0);

                    //
                    // We now have all the connectors for the given endpoints. We can try to obtain the
                    // connection.
                    //
                    _iter = 0;
                    getConnection();
                }
            }

            public void exception(Ice.LocalException ex)
            {
                _factory.handleException(ex, _hasMore || _endpointsIter < _endpoints.Count);
                if(_endpointsIter < _endpoints.Count)
                {
                    nextEndpoint();
                }
                else if(_connectors.Count > 0)
                {
                    //
                    // We now have all the connectors for the given endpoints. We can try to obtain the
                    // connection.
                    //
                    _iter = 0;
                    getConnection();
                }
                else
                {
                    _callback.setException(ex);
                    _factory.decPendingConnectCount(); // Must be called last.
                }
            }

            public void setConnection(Ice.ConnectionI connection, bool compress)
            {
                //
                // Callback from the factory: the connection to one of the callback
                // connectors has been established.
                //
                _callback.setConnection(connection, compress);
                _factory.decPendingConnectCount(); // Must be called last.
            }

            public void setException(Ice.LocalException ex)
            {
                //
                // Callback from the factory: connection establishment failed.
                //
                _callback.setException(ex);
                _factory.decPendingConnectCount(); // Must be called last.
            }

            public bool hasConnector(ConnectorInfo ci)
            {
                return _connectors.Contains(ci);
            }

            public bool removeConnectors(List<ConnectorInfo> connectors)
            {
                foreach(ConnectorInfo ci in connectors)
                {
                    while(_connectors.Remove(ci)); // Remove all of them.
                }
                return _connectors.Count == 0;
            }

            public void removeFromPending()
            {
                _factory.removeFromPending(this, _connectors);
            }

            public void getConnectors()
            {
                try
                {
                    //
                    // Notify the factory that there's an async connect pending. This is necessary
                    // to prevent the outgoing connection factory to be destroyed before all the
                    // pending asynchronous connects are finished.
                    //
                    _factory.incPendingConnectCount();
                }
                catch(Ice.LocalException ex)
                {
                    _callback.setException(ex);
                    return;
                }

                nextEndpoint();
            }

            void nextEndpoint()
            {
                try
                {
                    Debug.Assert(_endpointsIter < _endpoints.Count);
                    _currentEndpoint = _endpoints[_endpointsIter++];
                    _currentEndpoint.connectors_async(_selType, this);
                }
                catch(Ice.LocalException ex)
                {
                    exception(ex);
                }
            }

            internal void getConnection()
            {
                try
                {
                    //
                    // If all the connectors have been created, we ask the factory to get a
                    // connection.
                    //
                    bool compress;
                    Ice.ConnectionI connection = _factory.getConnection(_connectors, this, out compress);
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

                    _callback.setConnection(connection, compress);
                    _factory.decPendingConnectCount(); // Must be called last.
                }
                catch(Ice.LocalException ex)
                {
                    _callback.setException(ex);
                    _factory.decPendingConnectCount(); // Must be called last.
                }
            }

            internal void nextConnector()
            {
                while(true)
                {
                    try
                    {
                        Debug.Assert(_iter < _connectors.Count);
                        _current = _connectors[_iter++];

                        Ice.Instrumentation.CommunicatorObserver obsv = _factory._instance.initializationData().observer;
                        if(obsv != null)
                        {
                            _observer = obsv.getConnectionEstablishmentObserver(_current.endpoint,
                                                                                _current.connector.ToString());
                            if(_observer != null)
                            {
                                _observer.attach();
                            }
                        }

                        if(_factory._instance.traceLevels().network >= 2)
                        {
                            StringBuilder s = new StringBuilder("trying to establish ");
                            s.Append(_current.endpoint.protocol());
                            s.Append(" connection to ");
                            s.Append(_current.connector.ToString());
                            _factory._instance.initializationData().logger.trace(
                                                _factory._instance.traceLevels().networkCat, s.ToString());
                        }

                        Ice.ConnectionI connection = _factory.createConnection(_current.connector.connect(), _current);
                        connection.start(this);
                    }
                    catch(Ice.LocalException ex)
                    {
                        if(_factory._instance.traceLevels().network >= 2)
                        {
                            StringBuilder s = new StringBuilder("failed to establish ");
                            s.Append(_current.endpoint.protocol());
                            s.Append(" connection to ");
                            s.Append(_current.connector.ToString());
                            s.Append("\n");
                            s.Append(ex);
                            _factory._instance.initializationData().logger.trace(
                                                _factory._instance.traceLevels().networkCat, s.ToString());
                        }

                        if(connectionStartFailedImpl(ex))
                        {
                            continue;
                        }
                    }
                    break;
                }
            }

            private bool connectionStartFailedImpl(Ice.LocalException ex)
            {
                if(_observer != null)
                {
                    _observer.failed(ex.ice_id());
                    _observer.detach();
                }
                _factory.handleConnectionException(ex, _hasMore || _iter < _connectors.Count);
                if(ex is Ice.CommunicatorDestroyedException) // No need to continue.
                {
                    _factory.finishGetConnection(_connectors, ex, this);
                }
                else if(_iter < _connectors.Count) // Try the next connector.
                {
                    return true;
                }
                else
                {
                    _factory.finishGetConnection(_connectors, ex, this);
                }
                return false;
            }

            private OutgoingConnectionFactory _factory;
            private bool _hasMore;
            private CreateConnectionCallback _callback;
            private List<EndpointI> _endpoints;
            private Ice.EndpointSelectionType _selType;
            private int _endpointsIter;
            private EndpointI _currentEndpoint;
            private List<ConnectorInfo> _connectors = new List<ConnectorInfo>();
            private int _iter;
            private ConnectorInfo _current;
            private Ice.Instrumentation.Observer _observer;
        }

        private Ice.Communicator _communicator;
        private readonly Instance _instance;
        private FactoryACMMonitor _monitor;
        private bool _destroyed;

        private MultiDictionary<Connector, Ice.ConnectionI> _connections =
            new MultiDictionary<Connector, Ice.ConnectionI>();
        private MultiDictionary<EndpointI, Ice.ConnectionI> _connectionsByEndpoint =
            new MultiDictionary<EndpointI, Ice.ConnectionI>();
        private Dictionary<Connector, HashSet<ConnectCallback>> _pending =
            new Dictionary<Connector, HashSet<ConnectCallback>>();
        private int _pendingConnectCount;
    }

    public sealed class IncomingConnectionFactory : EventHandler, Ice.ConnectionI.StartCallback
    {
        private class StartAcceptor : TimerTask
        {
            public StartAcceptor(IncomingConnectionFactory factory)
            {
                _factory = factory;
            }

            public void runTimerTask()
            {
                _factory.startAcceptor();
            }

            private IncomingConnectionFactory _factory;
        }

        public void startAcceptor()
        {
            lock(this)
            {
                if(_state >= StateClosed || _acceptorStarted)
                {
                    return;
                }

                try
                {
                    createAcceptor();
                }
                catch(Exception ex)
                {
                    string s = "acceptor creation failed:\n" + ex + '\n' + _acceptor.ToString();
                    _instance.initializationData().logger.error(s);
                    _instance.timer().schedule(new StartAcceptor(this), 1000);
                }
            }
        }

        public void activate()
        {
            lock(this)
            {
                setState(StateActive);
            }
        }

        public void hold()
        {
            lock(this)
            {
                setState(StateHolding);
            }
        }

        public void destroy()
        {
            lock(this)
            {
                setState(StateClosed);
            }
        }

        public void updateConnectionObservers()
        {
            lock(this)
            {
                foreach(Ice.ConnectionI connection in _connections)
                {
                    connection.updateObserver();
                }
            }
        }

        public void waitUntilHolding()
        {
            ICollection<Ice.ConnectionI> connections;

            lock(this)
            {
                //
                // First we wait until the connection factory itself is in
                // holding state.
                //
                while(_state < StateHolding)
                {
                    System.Threading.Monitor.Wait(this);
                }

                //
                // We want to wait until all connections are in holding state
                // outside the thread synchronization.
                //
                connections = new List<Ice.ConnectionI>(_connections);
            }

            //
            // Now we wait until each connection is in holding state.
            //
            foreach(Ice.ConnectionI connection in connections)
            {
                connection.waitUntilHolding();
            }
        }

        public void waitUntilFinished()
        {
            ICollection<Ice.ConnectionI> connections = null;

            lock(this)
            {
                //
                // First we wait until the factory is destroyed. If we are using
                // an acceptor, we also wait for it to be closed.
                //
                while(_state != StateFinished)
                {
                    System.Threading.Monitor.Wait(this);
                }

                //
                // Clear the OA. See bug 1673 for the details of why this is necessary.
                //
                _adapter = null;

                //
                // We want to wait until all connections are finished outside the
                // thread synchronization.
                //
                connections = new List<Ice.ConnectionI>(_connections);
            }

            foreach(Ice.ConnectionI connection in connections)
            {
                connection.waitUntilFinished();
            }

            lock(this)
            {
                if(_transceiver != null)
                {
                    Debug.Assert(_connections.Count <= 1); // The connection isn't monitored or reaped.
                }
                else
                {
                    // Ensure all the connections are finished and reapable at this point.
                    ICollection<Ice.ConnectionI> cons = _monitor.swapReapedConnections();
                    Debug.Assert((cons == null ? 0 : cons.Count) == _connections.Count);
                    if(cons != null)
                    {
                        cons.Clear();
                    }
                }
                _connections.Clear();
            }

            //
            // Must be destroyed outside the synchronization since this might block waiting for
            // a timer task to execute.
            //
            _monitor.destroy();
        }

        public bool isLocal(EndpointI endpoint)
        {
            if(_publishedEndpoint != null && endpoint.equivalent(_publishedEndpoint))
            {
                return true;
            }
            lock(this)
            {
                return endpoint.equivalent(_endpoint);
            }
        }

        public EndpointI endpoint()
        {
            if(_publishedEndpoint != null)
            {
                return _publishedEndpoint;
            }
            lock(this)
            {
                return _endpoint;
            }
        }

        public ICollection<Ice.ConnectionI> connections()
        {
            lock(this)
            {
                ICollection<Ice.ConnectionI> connections = new List<Ice.ConnectionI>();

                //
                // Only copy connections which have not been destroyed.
                //
                foreach(Ice.ConnectionI connection in _connections)
                {
                    if(connection.isActiveOrHolding())
                    {
                        connections.Add(connection);
                    }
                }

                return connections;
            }
        }

        public void flushAsyncBatchRequests(Ice.CompressBatch compressBatch, CommunicatorFlushBatchAsync outAsync)
        {
            //
            // connections() is synchronized, no need to synchronize here.
            //
            foreach(Ice.ConnectionI connection in connections())
            {
                try
                {
                    outAsync.flushConnection(connection, compressBatch);
                }
                catch(Ice.LocalException)
                {
                    // Ignore.
                }
            }
        }

        //
        // Operations from EventHandler.
        //
        public override bool startAsync(int operation, AsyncCallback callback, ref bool completedSynchronously)
        {
            if(_state >= StateClosed)
            {
                return false;
            }

            Debug.Assert(_acceptor != null);
            try
            {
                completedSynchronously = _acceptor.startAccept(callback, this);
            }
            catch(Ice.LocalException ex)
            {
                _acceptorException = ex;
                completedSynchronously = true;
            }
            return true;
        }

        public override bool finishAsync(int unused)
        {
            try
            {
                if(_acceptorException != null)
                {
                    throw _acceptorException;
                }
                _acceptor.finishAccept();
            }
            catch(Ice.LocalException ex)
            {
                _acceptorException = null;

                string s = "couldn't accept connection:\n" + ex + '\n' + _acceptor.ToString();
                _instance.initializationData().logger.error(s);
                if(_acceptorStarted)
                {
                    _acceptorStarted = false;
                    _adapter.getThreadPool().finish(this);
                    closeAcceptor();
                }
            }
            return _state < StateClosed;
        }

        public override void message(ref ThreadPoolCurrent current)
        {
            Ice.ConnectionI connection = null;

            ThreadPoolMessage msg = new ThreadPoolMessage(this);

            lock(this)
            {
                if(!msg.startIOScope(ref current))
                {
                    return;
                }

                try
                {
                    if(_state >= StateClosed)
                    {
                        return;
                    }
                    else if(_state == StateHolding)
                    {
                        return;
                    }

                    //
                    // Reap closed connections
                    //
                    ICollection<Ice.ConnectionI> cons = _monitor.swapReapedConnections();
                    if(cons != null)
                    {
                        foreach(Ice.ConnectionI c in cons)
                        {
                            _connections.Remove(c);
                        }
                    }

                    if(!_acceptorStarted)
                    {
                        return;
                    }

                    //
                    // Now accept a new connection.
                    //
                    Transceiver transceiver = null;
                    try
                    {
                        transceiver = _acceptor.accept();

                        if(_instance.traceLevels().network >= 2)
                        {
                            StringBuilder s = new StringBuilder("trying to accept ");
                            s.Append(_endpoint.protocol());
                            s.Append(" connection\n");
                            s.Append(transceiver.ToString());
                            _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
                        }
                    }
                    catch(Ice.SocketException ex)
                    {
                        if(Network.noMoreFds(ex.InnerException))
                        {
                            string s = "can't accept more connections:\n" + ex + '\n' + _acceptor.ToString();
                            _instance.initializationData().logger.error(s);
                            Debug.Assert(_acceptorStarted);
                            _acceptorStarted = false;
                            _adapter.getThreadPool().finish(this);
                            closeAcceptor();
                        }

                        // Ignore socket exceptions.
                        return;
                    }
                    catch(Ice.LocalException ex)
                    {
                        // Warn about other Ice local exceptions.
                        if(_warn)
                        {
                            warning(ex);
                        }
                        return;
                    }

                    Debug.Assert(transceiver != null);

                    try
                    {
                        connection = new Ice.ConnectionI(_adapter.getCommunicator(), _instance, _monitor, transceiver,
                                                         null, _endpoint, _adapter);
                    }
                    catch(Ice.LocalException ex)
                    {
                        try
                        {
                            transceiver.close();
                        }
                        catch(Ice.LocalException)
                        {
                            // Ignore
                        }

                        if(_warn)
                        {
                            warning(ex);
                        }
                        return;
                    }

                    _connections.Add(connection);
                }
                finally
                {
                    msg.finishIOScope(ref current);
                }
            }

            Debug.Assert(connection != null);
            connection.start(this);
        }

        public override void finished(ref ThreadPoolCurrent current)
        {
            lock(this)
            {
                if(_state < StateClosed)
                {
                    //
                    // If the acceptor hasn't been explicitly stopped (which is the case if the acceptor got closed
                    // because of an unexpected error), try to restart the acceptor in 1 second.
                    //
                    _instance.timer().schedule(new StartAcceptor(this), 1000);
                    return;
                }

                Debug.Assert(_state >= StateClosed);
                setState(StateFinished);
            }
        }

        public override string ToString()
        {
            if(_transceiver != null)
            {
                return _transceiver.ToString();
            }
            return _acceptor.ToString();
        }

        //
        // Operations from ConnectionI.StartCallback
        //
        public void connectionStartCompleted(Ice.ConnectionI connection)
        {
            lock(this)
            {
                //
                // Initially, connections are in the holding state. If the factory is active
                // we activate the connection.
                //
                if(_state == StateActive)
                {
                    connection.activate();
                }
            }
        }

        public void connectionStartFailed(Ice.ConnectionI connection, Ice.LocalException ex)
        {
            lock(this)
            {
                if(_state >= StateClosed)
                {
                    return;
                }

                //
                // Do not warn about connection exceptions here. The connection is not yet validated.
                //
            }
        }

        public IncomingConnectionFactory(Instance instance, EndpointI endpoint, EndpointI publish,
                                         Ice.ObjectAdapterI adapter)
        {
            _instance = instance;
            _endpoint = endpoint;
            _publishedEndpoint = publish;
            _adapter = adapter;
            _warn = _instance.initializationData().properties.getPropertyAsInt("Ice.Warn.Connections") > 0;
            _connections = new HashSet<Ice.ConnectionI>();
            _state = StateHolding;
            _acceptorStarted = false;
            _monitor = new FactoryACMMonitor(instance, ((Ice.ObjectAdapterI)adapter).getACM());

            DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
            if(defaultsAndOverrides.overrideTimeout)
            {
                _endpoint = _endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
            }

            if(defaultsAndOverrides.overrideCompress)
            {
                _endpoint = _endpoint.compress(defaultsAndOverrides.overrideCompressValue);
            }

            try
            {
                _transceiver = _endpoint.transceiver();
                if(_transceiver != null)
                {
                    if(_instance.traceLevels().network >= 2)
                    {
                        StringBuilder s = new StringBuilder("attempting to bind to ");
                        s.Append(_endpoint.protocol());
                        s.Append(" socket\n");
                        s.Append(_transceiver.ToString());
                        _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
                    }
                    _endpoint = _transceiver.bind();

                    Ice.ConnectionI connection = new Ice.ConnectionI(_adapter.getCommunicator(), _instance, null,
                                                                     _transceiver, null, _endpoint, _adapter);
                    connection.startAndWait();
                    _connections.Add(connection);
                }
                else
                {
                    createAcceptor();
                }
            }
            catch(Exception ex)
            {
                //
                // Clean up.
                //
                if(_transceiver != null)
                {
                    try
                    {
                        _transceiver.close();
                    }
                    catch(Ice.LocalException)
                    {
                        // Ignore
                    }
                }

                _state = StateFinished;
                _monitor.destroy();
                _connections.Clear();

                if(ex is Ice.LocalException)
                {
                    throw;
                }
                else
                {
                    throw new Ice.SyscallException(ex);
                }
            }
        }

        private const int StateActive = 0;
        private const int StateHolding = 1;
        private const int StateClosed = 2;
        private const int StateFinished = 3;

        private void setState(int state)
        {
            if(_state == state) // Don't switch twice.
            {
                return;
            }

            switch (state)
            {
                case StateActive:
                {
                    if(_state != StateHolding) // Can only switch from holding to active.
                    {
                        return;
                    }
                    if(_acceptor != null)
                    {
                        if(_instance.traceLevels().network >= 1)
                        {
                            StringBuilder s = new StringBuilder("accepting ");
                            s.Append(_endpoint.protocol());
                            s.Append(" connections at ");
                            s.Append(_acceptor.ToString());
                            _instance.initializationData().logger.trace(_instance.traceLevels().networkCat,
                                                                        s.ToString());
                        }
                        _adapter.getThreadPool().register(this, SocketOperation.Read);
                    }

                    foreach(Ice.ConnectionI connection in _connections)
                    {
                        connection.activate();
                    }
                    break;
                }

                case StateHolding:
                {
                    if(_state != StateActive) // Can only switch from active to holding.
                    {
                        return;
                    }
                    if(_acceptor != null)
                    {
                        if(_instance.traceLevels().network >= 1)
                        {
                            StringBuilder s = new StringBuilder("holding ");
                            s.Append(_endpoint.protocol());
                            s.Append(" connections at ");
                            s.Append(_acceptor.ToString());
                            _instance.initializationData().logger.trace(_instance.traceLevels().networkCat,
                                                                        s.ToString());
                        }
                        _adapter.getThreadPool().unregister(this, SocketOperation.Read);
                    }

                    foreach(Ice.ConnectionI connection in _connections)
                    {
                        connection.hold();
                    }
                    break;
                }

                case StateClosed:
                {
                    if(_acceptorStarted)
                    {
                        _acceptorStarted = false;
                        _adapter.getThreadPool().finish(this);
                        closeAcceptor();
                    }
                    else
                    {
                        state = StateFinished;
                    }

                    foreach(Ice.ConnectionI connection in _connections)
                    {
                        connection.destroy(Ice.ConnectionI.ObjectAdapterDeactivated);
                    }
                    break;
                }

                case StateFinished:
                {
                    Debug.Assert(_state == StateClosed);
                    break;
                }
            }

            _state = state;
            System.Threading.Monitor.PulseAll(this);
        }

        private void createAcceptor()
        {
            try
            {
                Debug.Assert(!_acceptorStarted);
                _acceptor = _endpoint.acceptor(_adapter.getName());
                Debug.Assert(_acceptor != null);

                if(_instance.traceLevels().network >= 2)
                {
                    StringBuilder s = new StringBuilder("attempting to bind to ");
                    s.Append(_endpoint.protocol());
                    s.Append(" socket ");
                    s.Append(_acceptor.ToString());
                    _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
                }
                _endpoint = _acceptor.listen();

                if(_instance.traceLevels().network >= 1)
                {
                    StringBuilder s = new StringBuilder("listening for ");
                    s.Append(_endpoint.protocol());
                    s.Append(" connections\n");
                    s.Append(_acceptor.toDetailedString());
                    _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
                }

                _adapter.getThreadPool().initialize(this);

                if(_state == StateActive)
                {
                    _adapter.getThreadPool().register(this, SocketOperation.Read);
                }

                _acceptorStarted = true;
            }
            catch(SystemException)
            {
                if(_acceptor != null)
                {
                    _acceptor.close();
                }
                throw;
            }
        }

        private void closeAcceptor()
        {
            Debug.Assert(_acceptor != null);

            if(_instance.traceLevels().network >= 1)
            {
                StringBuilder s = new StringBuilder("stopping to accept ");
                s.Append(_endpoint.protocol());
                s.Append(" connections at ");
                s.Append(_acceptor.ToString());
                _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
            }

            Debug.Assert(!_acceptorStarted);
            _acceptor.close();
        }

        private void warning(Ice.LocalException ex)
        {
            _instance.initializationData().logger.warning("connection exception:\n" + ex + '\n' + _acceptor.ToString());
        }

        private Instance _instance;
        private FactoryACMMonitor _monitor;

        private Acceptor _acceptor;
        private readonly Transceiver _transceiver;
        private EndpointI _endpoint;
        private readonly EndpointI _publishedEndpoint;

        private Ice.ObjectAdapterI _adapter;

        private readonly bool _warn;

        private HashSet<Ice.ConnectionI> _connections;

        private int _state;
        private bool _acceptorStarted;
        private Ice.LocalException _acceptorException;
    }

}
