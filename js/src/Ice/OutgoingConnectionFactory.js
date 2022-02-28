//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/ACM");
require("../Ice/AsyncResultBase");
require("../Ice/ConnectionI");
require("../Ice/Debug");
require("../Ice/EndpointTypes");
require("../Ice/Exception");
require("../Ice/HashMap");
require("../Ice/LocalException");
require("../Ice/Promise");

const AsyncResultBase = Ice.AsyncResultBase;
const ConnectionI = Ice.ConnectionI;
const Debug = Ice.Debug;
const FactoryACMMonitor = Ice.FactoryACMMonitor;
const HashMap = Ice.HashMap;

//
// Only for use by Instance.
//
class OutgoingConnectionFactory
{
    constructor(communicator, instance)
    {
        this._communicator = communicator;
        this._instance = instance;
        this._destroyed = false;

        this._monitor = new FactoryACMMonitor(this._instance, this._instance.clientACM());

        this._connectionsByEndpoint = new ConnectionListMap(); // map<EndpointI, Array<Ice.ConnectionI>>
        this._pending = new HashMap(HashMap.compareEquals); // map<EndpointI, Array<ConnectCallback>>
        this._pendingConnectCount = 0;

        this._waitPromise = null;
    }

    destroy()
    {
        if(this._destroyed)
        {
            return;
        }

        this._connectionsByEndpoint.forEach(connection => connection.destroy(ConnectionI.CommunicatorDestroyed));

        this._destroyed = true;
        this._communicator = null;
        this.checkFinished();
    }

    waitUntilFinished()
    {
        this._waitPromise = new Ice.Promise();
        this.checkFinished();
        return this._waitPromise;
    }

    //
    // Returns a promise, success callback receives the connection
    //
    create(endpts, hasMore, selType)
    {
        Debug.assert(endpts.length > 0);

        //
        // Apply the overrides.
        //
        const endpoints = this.applyOverrides(endpts);

        //
        // Try to find a connection to one of the given endpoints.
        //
        try
        {
            const connection = this.findConnectionByEndpoint(endpoints);
            if(connection !== null)
            {
                return Ice.Promise.resolve(connection);
            }
        }
        catch(ex)
        {
            return Ice.Promise.reject(ex);
        }

        return new ConnectCallback(this, endpoints, hasMore, selType).start();
    }

    setRouterInfo(routerInfo)
    {
        return Ice.Promise.try(() =>
            {
                if(this._destroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                return routerInfo.getClientEndpoints();
            }
        ).then(
            endpoints =>
            {
                //
                // Search for connections to the router's client proxy
                // endpoints, and update the object adapter for such
                // connections, so that callbacks from the router can be
                // received over such connections.
                //
                const adapter = routerInfo.getAdapter();
                const defaultsAndOverrides = this._instance.defaultsAndOverrides();
                endpoints.forEach(endpoint =>
                {
                    //
                    // Modify endpoints with overrides.
                    //
                    if(defaultsAndOverrides.overrideTimeout)
                    {
                        endpoint = endpoint.changeTimeout(defaultsAndOverrides.overrideTimeoutValue);
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
                    endpoint = endpoint.changeCompress(false);

                    this._connectionsByEndpoint.forEach(connection =>
                                                        {
                                                            if(connection.endpoint().equals(endpoint))
                                                            {
                                                                connection.setAdapter(adapter);
                                                            }
                                                        });
                });
            });
    }

    removeAdapter(adapter)
    {
        if(this._destroyed)
        {
            return;
        }
        this._connectionsByEndpoint.forEach(connection =>
                                            {
                                                if(connection.getAdapter() === adapter)
                                                {
                                                    connection.setAdapter(null);
                                                }
                                            });
    }

    flushAsyncBatchRequests()
    {
        const promise = new AsyncResultBase(this._communicator, "flushBatchRequests", null, null, null);
        if(this._destroyed)
        {
            promise.resolve();
            return promise;
        }

        Ice.Promise.all(
            this._connectionsByEndpoint.map(
                connection =>
                {
                    if(connection.isActiveOrHolding())
                    {
                        return connection.flushBatchRequests().catch(
                            ex =>
                            {
                                if(ex instanceof Ice.LocalException)
                                {
                                    // Ignore
                                }
                                else
                                {
                                    throw ex;
                                }
                            });
                    }
                })).then(promise.resolve, promise.reject);
        return promise;
    }

    applyOverrides(endpts)
    {
        const defaultsAndOverrides = this._instance.defaultsAndOverrides();
        return endpts.map(
            endpoint =>
                {
                    if(defaultsAndOverrides.overrideTimeout)
                    {
                        return endpoint.changeTimeout(defaultsAndOverrides.overrideTimeoutValue);
                    }
                    else
                    {
                        return endpoint;
                    }
                });
    }

    findConnectionByEndpoint(endpoints)
    {
        if(this._destroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(endpoints.length > 0);

        for(let i = 0; i < endpoints.length; ++i)
        {
            const endpoint = endpoints[i];

            if(this._pending.has(endpoint))
            {
                continue;
            }

            const connectionList = this._connectionsByEndpoint.get(endpoint);
            if(connectionList === undefined)
            {
                continue;
            }

            for(let j = 0; j < connectionList.length; ++j)
            {
                if(connectionList[j].isActiveOrHolding()) // Don't return destroyed or un-validated connections
                {
                    return connectionList[j];
                }
            }
        }

        return null;
    }

    incPendingConnectCount()
    {
        //
        // Keep track of the number of pending connects. The outgoing connection factory
        // waitUntilFinished() method waits for all the pending connects to terminate before
        // to return. This ensures that the communicator client thread pool isn't destroyed
        // too soon and will still be available to execute the ice_exception() callbacks for
        // the asynchronous requests waiting on a connection to be established.
        //

        if(this._destroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }
        ++this._pendingConnectCount;
    }

    decPendingConnectCount()
    {
        --this._pendingConnectCount;
        Debug.assert(this._pendingConnectCount >= 0);
        if(this._destroyed && this._pendingConnectCount === 0)
        {
            this.checkFinished();
        }
    }

    getConnection(endpoints, cb)
    {
        if(this._destroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        //
        // Reap closed connections
        //
        const cons = this._monitor.swapReapedConnections();
        if(cons !== null)
        {
            cons.forEach(c =>
                {
                    this._connectionsByEndpoint.removeConnection(c.endpoint(), c);
                    this._connectionsByEndpoint.removeConnection(c.endpoint().changeCompress(true), c);
                });
        }

        //
        // Try to get the connection.
        //
        while(true)
        {
            if(this._destroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            //
            // Search for a matching connection. If we find one, we're done.
            //
            const connection = this.findConnectionByEndpoint(endpoints);
            if(connection !== null)
            {
                return connection;
            }

            if(this.addToPending(cb, endpoints))
            {
                //
                // A connection is already pending.
                //
                return null;
            }
            else
            {
                //
                // No connection is currently pending to one of our endpoints, so we
                // get out of this loop and start the connection establishment to one of the
                // given endpoints.
                //
                break;
            }
        }

        //
        // At this point, we're responsible for establishing the connection to one of
        // the given endpoints. If it's a non-blocking connect, calling nextEndpoint
        // will start the connection establishment. Otherwise, we return null to get
        // the caller to establish the connection.
        //
        cb.nextEndpoint();

        return null;
    }

    createConnection(transceiver, endpoint)
    {
        Debug.assert(this._pending.has(endpoint) && transceiver !== null);

        //
        // Create and add the connection to the connection map. Adding the connection to the map
        // is necessary to support the interruption of the connection initialization and validation
        // in case the communicator is destroyed.
        //
        let connection = null;
        try
        {
            if(this._destroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            connection = new ConnectionI(this._communicator, this._instance, this._monitor, transceiver,
                                         endpoint.changeCompress(false), false, null);
        }
        catch(ex)
        {
            if(ex instanceof Ice.LocalException)
            {
                try
                {
                    transceiver.close();
                }
                catch(exc)
                {
                    // Ignore
                }
            }
            throw ex;
        }

        this._connectionsByEndpoint.set(connection.endpoint(), connection);
        this._connectionsByEndpoint.set(connection.endpoint().changeCompress(true), connection);
        return connection;
    }

    finishGetConnection(endpoints, endpoint, connection, cb)
    {
        // cb is-a ConnectCallback

        const connectionCallbacks = [];
        if(cb !== null)
        {
            connectionCallbacks.push(cb);
        }

        const callbacks = [];
        endpoints.forEach(endpt =>
            {
                const cbs = this._pending.get(endpt);
                if(cbs !== undefined)
                {
                    this._pending.delete(endpt);
                    cbs.forEach(cc =>
                        {
                            if(cc.hasEndpoint(endpoint))
                            {
                                if(connectionCallbacks.indexOf(cc) === -1)
                                {
                                    connectionCallbacks.push(cc);
                                }
                            }
                            else if(callbacks.indexOf(cc) === -1)
                            {
                                callbacks.push(cc);
                            }
                        });
                }
            });

        connectionCallbacks.forEach(cc =>
            {
                cc.removeFromPending();
                const idx = callbacks.indexOf(cc);
                if(idx !== -1)
                {
                    callbacks.splice(idx, 1);
                }
            });

        callbacks.forEach(cc => cc.removeFromPending());

        callbacks.forEach(cc => cc.getConnection());
        connectionCallbacks.forEach(cc => cc.setConnection(connection));

        this.checkFinished();
    }

    finishGetConnectionEx(endpoints, ex, cb)
    {
        // cb is-a ConnectCallback

        const failedCallbacks = [];
        if(cb !== null)
        {
            failedCallbacks.push(cb);
        }

        const callbacks = [];
        endpoints.forEach(endpt =>
            {
                const cbs = this._pending.get(endpt);
                if(cbs !== undefined)
                {
                    this._pending.delete(endpt);
                    cbs.forEach(cc =>
                        {
                            if(cc.removeEndpoints(endpoints))
                            {
                                if(failedCallbacks.indexOf(cc) === -1)
                                {
                                    failedCallbacks.push(cc);
                                }
                            }
                            else if(callbacks.indexOf(cc) === -1)
                            {
                                callbacks.push(cc);
                            }
                        });
                }
            });

        callbacks.forEach(cc =>
            {
                Debug.assert(failedCallbacks.indexOf(cc) === -1);
                cc.removeFromPending();
            });
        this.checkFinished();
        callbacks.forEach(cc => cc.getConnection());
        failedCallbacks.forEach(cc => cc.setException(ex));
    }

    addToPending(cb, endpoints)
    {
        // cb is-a ConnectCallback

        //
        // Add the callback to each pending list.
        //
        let found = false;
        if(cb !== null)
        {
            endpoints.forEach(p =>
                {
                    const cbs = this._pending.get(p);
                    if(cbs !== undefined)
                    {
                        found = true;
                        if(cbs.indexOf(cb) === -1)
                        {
                            cbs.push(cb); // Add the callback to each pending endpoint.
                        }
                    }
                });
        }

        if(found)
        {
            return true;
        }

        //
        // If there's no pending connection for the given endpoints, we're
        // responsible for its establishment. We add empty pending lists,
        // other callbacks to the same endpoints will be queued.
        //
        endpoints.forEach(p =>
            {
                if(!this._pending.has(p))
                {
                    this._pending.set(p, []);
                }
            });

        return false;
    }

    removeFromPending(cb, endpoints)
    {
        // cb is-a ConnectCallback
        endpoints.forEach(p =>
            {
                const cbs = this._pending.get(p);
                if(cbs !== undefined)
                {
                    const idx = cbs.indexOf(cb);
                    if(idx !== -1)
                    {
                        cbs.splice(idx, 1);
                    }
                }
            });
    }

    handleConnectionException(ex, hasMore)
    {
        const traceLevels = this._instance.traceLevels();
        if(traceLevels.network >= 2)
        {
            const s = [];
            s.push("connection to endpoint failed");
            if(ex instanceof Ice.CommunicatorDestroyedException)
            {
                s.push("\n");
            }
            else if(hasMore)
            {
                s.push(", trying next endpoint\n");
            }
            else
            {
                s.push(" and no more endpoints to try\n");
            }
            s.push(ex.toString());
            this._instance.initializationData().logger.trace(traceLevels.networkCat, s.join(""));
        }
    }

    handleException(ex, hasMore)
    {
        const traceLevels = this._instance.traceLevels();
        if(traceLevels.network >= 2)
        {
            const s = [];
            s.push("couldn't resolve endpoint host");
            if(ex instanceof Ice.CommunicatorDestroyedException)
            {
                s.push("\n");
            }
            else if(hasMore)
            {
                s.push(", trying next endpoint\n");
            }
            else
            {
                s.push(" and no more endpoints to try\n");
            }
            s.push(ex.toString());
            this._instance.initializationData().logger.trace(traceLevels.networkCat, s.join(""));
        }
    }

    checkFinished()
    {
        //
        // Can't continue until the factory is destroyed and there are no pending connections.
        //
        if(!this._waitPromise || !this._destroyed || this._pending.size > 0 || this._pendingConnectCount > 0)
        {
            return;
        }

        Ice.Promise.all(
            this._connectionsByEndpoint.map(
                connection => connection.waitUntilFinished().catch(ex => Debug.assert(false)))
        ).then(
            () =>
            {
                const cons = this._monitor.swapReapedConnections();
                if(cons !== null)
                {
                    const arr = [];
                    this._connectionsByEndpoint.forEach(connection =>
                    {
                        if(arr.indexOf(connection) === -1)
                        {
                            arr.push(connection);
                        }
                    });
                    Debug.assert(cons.length === arr.length);
                    this._connectionsByEndpoint.clear();
                }
                else
                {
                    Debug.assert(this._connectionsByEndpoint.size === 0);
                }

                Debug.assert(this._waitPromise !== null);
                this._waitPromise.resolve();
                this._monitor.destroy();
            });
    }
}

Ice.OutgoingConnectionFactory = OutgoingConnectionFactory;
module.exports.Ice = Ice;

//
// Value is a Vector<Ice.ConnectionI>
//
class ConnectionListMap extends HashMap
{
    constructor(h)
    {
        super(h || HashMap.compareEquals);
    }

    set(key, value)
    {
        let list = this.get(key);
        if(list === undefined)
        {
            list = [];
            super.set(key, list);
        }
        Debug.assert(value instanceof ConnectionI);
        list.push(value);
        return undefined;
    }

    removeConnection(key, conn)
    {
        const list = this.get(key);
        Debug.assert(list !== null);
        const idx = list.indexOf(conn);
        Debug.assert(idx !== -1);
        list.splice(idx, 1);
        if(list.length === 0)
        {
            this.delete(key);
        }
    }

    map(fn)
    {
        const arr = [];
        this.forEach(c => arr.push(fn(c)));
        return arr;
    }

    forEach(fn)
    {
        for(const connections of this.values())
        {
            connections.forEach(fn);
        }
    }
}

class ConnectCallback
{
    constructor(f, endpoints, more, selType)
    {
        this._factory = f;
        this._endpoints = endpoints;
        this._hasMore = more;
        this._selType = selType;
        this._promise = new Ice.Promise();
        this._index = 0;
        this._current = null;
    }

    //
    // Methods from ConnectionI_StartCallback
    //
    connectionStartCompleted(connection)
    {
        connection.activate();
        this._factory.finishGetConnection(this._endpoints, this._current, connection, this);
    }

    connectionStartFailed(connection, ex)
    {
        Debug.assert(this._current !== null);
        if(this.connectionStartFailedImpl(ex))
        {
            this.nextEndpoint();
        }
    }

    setConnection(connection)
    {
        //
        // Callback from the factory: the connection to one of the callback
        // connectors has been established.
        //
        this._promise.resolve(connection);
        this._factory.decPendingConnectCount(); // Must be called last.
    }

    setException(ex)
    {
        //
        // Callback from the factory: connection establishment failed.
        //
        this._promise.reject(ex);
        this._factory.decPendingConnectCount(); // Must be called last.
    }

    hasEndpoint(endpoint)
    {
        return this.findEndpoint(endpoint) !== -1;
    }

    findEndpoint(endpoint)
    {
        return this._endpoints.findIndex(value => endpoint.equals(value));
    }

    removeEndpoints(endpoints)
    {
        endpoints.forEach(endpoint =>
            {
                const idx = this.findEndpoint(endpoint);
                if(idx !== -1)
                {
                    this._endpoints.splice(idx, 1);
                }
            });
        this._index = 0;
        return this._endpoints.length === 0;
    }

    removeFromPending()
    {
        this._factory.removeFromPending(this, this._endpoints);
    }

    start()
    {
        try
        {
            //
            // Notify the factory that there's an async connect pending. This is necessary
            // to prevent the outgoing connection factory to be destroyed before all the
            // pending asynchronous connects are finished.
            //
            this._factory.incPendingConnectCount();
        }
        catch(ex)
        {
            this._promise.reject(ex);
            return;
        }

        this.getConnection();
        return this._promise;
    }

    getConnection()
    {
        try
        {
            //
            // Ask the factory to get a connection.
            //
            const connection = this._factory.getConnection(this._endpoints, this);
            if(connection === null)
            {
                //
                // A null return value from getConnection indicates that the connection
                // is being established and that everthing has been done to ensure that
                // the callback will be notified when the connection establishment is
                // done.
                //
                return;
            }

            this._promise.resolve(connection);
            this._factory.decPendingConnectCount(); // Must be called last.
        }
        catch(ex)
        {
            this._promise.reject(ex);
            this._factory.decPendingConnectCount(); // Must be called last.
        }
    }

    nextEndpoint()
    {

        const start = connection =>
            {
                connection.start().then(
                    () =>
                    {
                        this.connectionStartCompleted(connection);
                    },
                    ex =>
                    {
                        this.connectionStartFailed(connection, ex);
                    });
            };

        while(true)
        {
            const traceLevels = this._factory._instance.traceLevels();
            try
            {
                Debug.assert(this._index < this._endpoints.length);
                this._current = this._endpoints[this._index++];

                if(traceLevels.network >= 2)
                {
                    const s = [];
                    s.push("trying to establish ");
                    s.push(this._current.protocol());
                    s.push(" connection to ");
                    s.push(this._current.toConnectorString());
                    this._factory._instance.initializationData().logger.trace(traceLevels.networkCat, s.join(""));
                }

                start(this._factory.createConnection(this._current.connect(), this._current));
            }
            catch(ex)
            {
                if(traceLevels.network >= 2)
                {
                    const s = [];
                    s.push("failed to establish ");
                    s.push(this._current.protocol());
                    s.push(" connection to ");
                    s.push(this._current.toString());
                    s.push("\n");
                    s.push(ex.toString());
                    this._factory._instance.initializationData().logger.trace(traceLevels.networkCat, s.join(""));
                }

                if(this.connectionStartFailedImpl(ex))
                {
                    continue;
                }
            }
            break;
        }
    }

    connectionStartFailedImpl(ex)
    {
        if(ex instanceof Ice.LocalException)
        {
            this._factory.handleConnectionException(ex, this._hasMore || this._index < this._endpoints.length);
            if(ex instanceof Ice.CommunicatorDestroyedException) // No need to continue.
            {
                this._factory.finishGetConnectionEx(this._endpoints, ex, this);
            }
            else if(this._index < this._endpoints.length) // Try the next endpoint.
            {
                return true;
            }
            else
            {
                this._factory.finishGetConnectionEx(this._endpoints, ex, this);
            }
        }
        else
        {
            this._factory.finishGetConnectionEx(this._endpoints, ex, this);
        }
        return false;
    }
}
