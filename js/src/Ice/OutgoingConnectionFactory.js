//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { ConnectionI } from "./ConnectionI.js";
import { HashMap } from "./HashMap.js";
import { Promise } from "./Promise.js";
import { LocalException } from "./LocalException.js";
import { CommunicatorDestroyedException } from "./LocalExceptions.js";
import { Debug } from "./Debug.js";

//
// Only for use by Instance.
//
export class OutgoingConnectionFactory {
    constructor(communicator, instance) {
        this._communicator = communicator;
        this._instance = instance;
        this._destroyed = false;
        this._defaultObjectAdapter = null;
        this._connectionOptions = instance.clientConnectionOptions;

        this._connectionsByEndpoint = new ConnectionListMap(); // map<EndpointI, Array<Ice.ConnectionI>>
        this._pending = new HashMap(HashMap.compareEquals); // map<EndpointI, Array<ConnectCallback>>
        this._pendingConnectCount = 0;

        this._waitPromise = null;
    }

    destroy() {
        if (this._destroyed) {
            return;
        }

        this._connectionsByEndpoint.forEach(connection => connection.destroy());

        this._destroyed = true;
        this._communicator = null;
        this._defaultObjectAdapter = null;
        this.checkFinished();
    }

    waitUntilFinished() {
        this._waitPromise = new Promise();
        this.checkFinished();
        return this._waitPromise;
    }

    //
    // Returns a promise, success callback receives the connection
    //
    create(endpoints, hasMore, selType) {
        DEV: console.assert(endpoints.length > 0);

        //
        // Try to find a connection to one of the given endpoints.
        //
        try {
            const connection = this.findConnectionByEndpoint(endpoints);
            if (connection !== null) {
                return Promise.resolve(connection);
            }
        } catch (ex) {
            return Promise.reject(ex);
        }

        return new ConnectCallback(this, endpoints, hasMore, selType).start();
    }

    setRouterInfo(routerInfo) {
        return Promise.resolve()
            .then(() => {
                if (this._destroyed) {
                    throw new CommunicatorDestroyedException();
                }
                return routerInfo.getClientEndpoints();
            })
            .then(endpoints => {
                //
                // Search for connections to the router's client proxy
                // endpoints, and update the object adapter for such
                // connections, so that callbacks from the router can be
                // received over such connections.
                //
                const adapter = routerInfo.getAdapter();
                endpoints.forEach(endpoint => {
                    //
                    // The Connection object does not take the compression flag of
                    // endpoints into account, but instead gets the information
                    // about whether messages should be compressed or not from
                    // other sources. In order to allow connection sharing for
                    // endpoints that differ in the value of the compression flag
                    // only, we always set the compression flag to false here in
                    // this connection factory. We also clear the timeout as it is
                    // no longer used for Ice 3.8.
                    //
                    endpoint = endpoint.changeCompress(false).changeTimeout(-1);

                    this._connectionsByEndpoint.forEach(connection => {
                        if (connection.endpoint().equals(endpoint)) {
                            connection.setAdapter(adapter);
                        }
                    });
                });
            });
    }

    removeAdapter(adapter) {
        if (this._destroyed) {
            return;
        }
        this._connectionsByEndpoint.forEach(connection => {
            if (connection.getAdapter() === adapter) {
                connection.setAdapter(null);
            }
        });
    }

    async flushAsyncBatchRequests() {
        if (!this._destroyed) {
            await Promise.all(
                this._connectionsByEndpoint.map(connection => {
                    if (connection.isActiveOrHolding()) {
                        return connection.flushBatchRequests().catch(ex => {
                            if (ex instanceof LocalException) {
                                // Ignore
                            } else {
                                throw ex;
                            }
                        });
                    }
                }),
            );
        }
    }

    getDefaultObjectAdapter() {
        return this._defaultObjectAdapter;
    }

    setDefaultObjectAdapter(adapter) {
        this._defaultObjectAdapter = adapter;
    }

    findConnectionByEndpoint(endpoints) {
        if (this._destroyed) {
            throw new CommunicatorDestroyedException();
        }

        DEV: console.assert(endpoints.length > 0);

        for (const endpoint of endpoints) {
            const proxyEndpoint = endpoint.changeTimeout(-1);

            if (this._pending.has(proxyEndpoint)) {
                continue;
            }

            const connectionList = this._connectionsByEndpoint.get(proxyEndpoint);
            if (connectionList === undefined) {
                continue;
            }

            for (let j = 0; j < connectionList.length; ++j) {
                if (connectionList[j].isActiveOrHolding()) {
                    // Don't return destroyed or un-validated connections
                    return connectionList[j];
                }
            }
        }

        return null;
    }

    incPendingConnectCount() {
        //
        // Keep track of the number of pending connects. The outgoing connection factory
        // waitUntilFinished() method waits for all the pending connects to terminate before
        // to return. This ensures that the communicator client thread pool isn't destroyed
        // too soon and will still be available to execute the ice_exception() callbacks for
        // the asynchronous requests waiting on a connection to be established.
        //

        if (this._destroyed) {
            throw new CommunicatorDestroyedException();
        }
        ++this._pendingConnectCount;
    }

    decPendingConnectCount() {
        --this._pendingConnectCount;
        DEV: console.assert(this._pendingConnectCount >= 0);
        if (this._destroyed && this._pendingConnectCount === 0) {
            this.checkFinished();
        }
    }

    getConnection(endpoints, cb) {
        if (this._destroyed) {
            throw new CommunicatorDestroyedException();
        }

        // Search for an existing connections matching one of the given endpoints.
        const connection = this.findConnectionByEndpoint(endpoints);
        if (connection !== null) {
            return connection;
        }

        if (!this.addToPending(cb, endpoints)) {
            // No connection is currently pending to one of our endpoints, call nextEndpoint to start the connection
            // establishment.
            cb.nextEndpoint();
        }
        // Return null to indicate to the caller that the connection is being established.
        return null;
    }

    createConnection(transceiver, endpoint) {
        DEV: console.assert(this._pending.has(endpoint) && transceiver !== null);

        //
        // Create and add the connection to the connection map. Adding the connection to the map
        // is necessary to support the interruption of the connection initialization and validation
        // in case the communicator is destroyed.
        //
        let connection = null;
        try {
            if (this._destroyed) {
                throw new CommunicatorDestroyedException();
            }

            connection = new ConnectionI(
                this._communicator,
                this._instance,
                transceiver,
                endpoint.changeCompress(false).changeTimeout(-1),
                this._defaultObjectAdapter,
                connection => this.removeConnection(connection),
                this._connectionOptions,
            );
        } catch (ex) {
            Debug.assert(ex instanceof LocalException);
            transceiver.close();
            throw ex;
        }

        this._connectionsByEndpoint.set(connection.endpoint(), connection);
        this._connectionsByEndpoint.set(connection.endpoint().changeCompress(true), connection);
        return connection;
    }

    removeConnection(connection) {
        this._connectionsByEndpoint.removeConnection(connection.endpoint(), connection);
        this._connectionsByEndpoint.removeConnection(connection.endpoint().changeCompress(true), connection);
    }

    finishGetConnection(endpoints, endpoint, connection, cb) {
        // cb is-a ConnectCallback

        const connectionCallbacks = [];
        if (cb !== null) {
            connectionCallbacks.push(cb);
        }

        const callbacks = [];
        endpoints.forEach(endpt => {
            const cbs = this._pending.get(endpt);
            if (cbs !== undefined) {
                this._pending.delete(endpt);
                cbs.forEach(cc => {
                    if (cc.hasEndpoint(endpoint)) {
                        if (connectionCallbacks.indexOf(cc) === -1) {
                            connectionCallbacks.push(cc);
                        }
                    } else if (callbacks.indexOf(cc) === -1) {
                        callbacks.push(cc);
                    }
                });
            }
        });

        connectionCallbacks.forEach(cc => {
            cc.removeFromPending();
            const idx = callbacks.indexOf(cc);
            if (idx !== -1) {
                callbacks.splice(idx, 1);
            }
        });

        callbacks.forEach(cc => cc.removeFromPending());

        callbacks.forEach(cc => cc.getConnection());
        connectionCallbacks.forEach(cc => cc.setConnection(connection));

        this.checkFinished();
    }

    finishGetConnectionEx(endpoints, ex, cb) {
        // cb is-a ConnectCallback

        const failedCallbacks = [];
        if (cb !== null) {
            failedCallbacks.push(cb);
        }

        const callbacks = [];
        endpoints.forEach(endpt => {
            const cbs = this._pending.get(endpt);
            if (cbs !== undefined) {
                this._pending.delete(endpt);
                cbs.forEach(cc => {
                    if (cc.removeEndpoints(endpoints)) {
                        if (failedCallbacks.indexOf(cc) === -1) {
                            failedCallbacks.push(cc);
                        }
                    } else if (callbacks.indexOf(cc) === -1) {
                        callbacks.push(cc);
                    }
                });
            }
        });

        callbacks.forEach(cc => {
            DEV: console.assert(failedCallbacks.indexOf(cc) === -1);
            cc.removeFromPending();
        });
        this.checkFinished();
        callbacks.forEach(cc => cc.getConnection());
        failedCallbacks.forEach(cc => cc.setException(ex));
    }

    addToPending(cb, endpoints) {
        DEV: console.assert(cb !== null);

        // Add the callback to each pending list.
        let found = false;
        endpoints.forEach(p => {
            const cbs = this._pending.get(p);
            if (cbs !== undefined) {
                found = true;
                if (cbs.indexOf(cb) === -1) {
                    cbs.push(cb); // Add the callback to each pending endpoint.
                }
            }
        });

        if (found) {
            return true;
        }

        // If there's no pending connection for the given endpoints, we're responsible for its establishment. We add an
        // empty pending lists, other callbacks to the same endpoints will be queued.
        endpoints.forEach(p => {
            if (!this._pending.has(p)) {
                this._pending.set(p, []);
            }
        });

        return false;
    }

    removeFromPending(cb, endpoints) {
        // cb is-a ConnectCallback
        endpoints.forEach(p => {
            const cbs = this._pending.get(p);
            if (cbs !== undefined) {
                const idx = cbs.indexOf(cb);
                if (idx !== -1) {
                    cbs.splice(idx, 1);
                }
            }
        });
    }

    handleConnectionException(ex, hasMore) {
        const traceLevels = this._instance.traceLevels();
        if (traceLevels.network >= 2) {
            const s = [];
            s.push("connection to endpoint failed");
            if (ex instanceof CommunicatorDestroyedException) {
                s.push("\n");
            } else if (hasMore) {
                s.push(", trying next endpoint\n");
            } else {
                s.push(" and no more endpoints to try\n");
            }
            s.push(ex.toString());
            this._instance.initializationData().logger.trace(traceLevels.networkCat, s.join(""));
        }
    }

    handleException(ex, hasMore) {
        const traceLevels = this._instance.traceLevels();
        if (traceLevels.network >= 2) {
            const s = [];
            s.push("couldn't resolve endpoint host");
            if (ex instanceof CommunicatorDestroyedException) {
                s.push("\n");
            } else if (hasMore) {
                s.push(", trying next endpoint\n");
            } else {
                s.push(" and no more endpoints to try\n");
            }
            s.push(ex.toString());
            this._instance.initializationData().logger.trace(traceLevels.networkCat, s.join(""));
        }
    }

    async checkFinished() {
        //
        // Can't continue until the factory is destroyed and there are no pending connections.
        //
        if (!this._waitPromise || !this._destroyed || this._pending.size > 0 || this._pendingConnectCount > 0) {
            return;
        }

        await Promise.all(
            this._connectionsByEndpoint.map(async connection => {
                try {
                    await connection.waitUntilFinished();
                } catch (ex) {
                    DEV: console.assert(false);
                }
            }),
        );

        DEV: console.assert(this._waitPromise !== null);
        this._waitPromise.resolve();
    }
}

//
// Value is a Vector<Ice.ConnectionI>
//
class ConnectionListMap extends HashMap {
    constructor(h) {
        super(h || HashMap.compareEquals);
    }

    set(key, value) {
        let list = this.get(key);
        if (list === undefined) {
            list = [];
            super.set(key, list);
        }
        DEV: console.assert(value instanceof ConnectionI);
        list.push(value);
        return undefined;
    }

    removeConnection(key, conn) {
        const list = this.get(key);
        DEV: console.assert(list !== null);
        const idx = list.indexOf(conn);
        DEV: console.assert(idx !== -1);
        list.splice(idx, 1);
        if (list.length === 0) {
            this.delete(key);
        }
    }

    map(fn) {
        const arr = [];
        this.forEach(c => arr.push(fn(c)));
        return arr;
    }

    forEach(fn) {
        for (const connections of this.values()) {
            connections.forEach(fn);
        }
    }
}

class ConnectCallback {
    constructor(f, endpoints, more, selType) {
        this._factory = f;
        this._endpoints = endpoints;
        this._hasMore = more;
        this._selType = selType;
        this._promise = new Promise();
        this._index = 0;
        this._current = null;
    }

    //
    // Methods from ConnectionI_StartCallback
    //
    connectionStartCompleted(connection) {
        connection.activate();
        this._factory.finishGetConnection(this._endpoints, this._current, connection, this);
    }

    connectionStartFailed(connection, ex) {
        DEV: console.assert(this._current !== null);
        if (this.connectionStartFailedImpl(ex)) {
            this.nextEndpoint();
        }
    }

    setConnection(connection) {
        //
        // Callback from the factory: the connection to one of the callback
        // connectors has been established.
        //
        this._promise.resolve(connection);
        this._factory.decPendingConnectCount(); // Must be called last.
    }

    setException(ex) {
        //
        // Callback from the factory: connection establishment failed.
        //
        this._promise.reject(ex);
        this._factory.decPendingConnectCount(); // Must be called last.
    }

    hasEndpoint(endpoint) {
        return this.findEndpoint(endpoint) !== -1;
    }

    findEndpoint(endpoint) {
        return this._endpoints.findIndex(value => endpoint.equals(value));
    }

    removeEndpoints(endpoints) {
        endpoints.forEach(endpoint => {
            const idx = this.findEndpoint(endpoint);
            if (idx !== -1) {
                this._endpoints.splice(idx, 1);
            }
        });
        this._index = 0;
        return this._endpoints.length === 0;
    }

    removeFromPending() {
        this._factory.removeFromPending(this, this._endpoints);
    }

    start() {
        try {
            //
            // Notify the factory that there's an async connect pending. This is necessary
            // to prevent the outgoing connection factory to be destroyed before all the
            // pending asynchronous connects are finished.
            //
            this._factory.incPendingConnectCount();
        } catch (ex) {
            this._promise.reject(ex);
            return;
        }

        this.getConnection();
        return this._promise;
    }

    getConnection() {
        try {
            const connection = this._factory.getConnection(this._endpoints, this);
            if (connection === null) {
                // A null return value from getConnection indicates that the connection is being established, and the
                // callback will be notified when the connection establishment is done.
                return;
            }

            this._promise.resolve(connection);
            this._factory.decPendingConnectCount(); // Must be called last.
        } catch (ex) {
            this._promise.reject(ex);
            this._factory.decPendingConnectCount(); // Must be called last.
        }
    }

    nextEndpoint() {
        const start = connection => {
            connection.start().then(
                () => {
                    this.connectionStartCompleted(connection);
                },
                ex => {
                    this.connectionStartFailed(connection, ex);
                },
            );
        };

        while (true) {
            const traceLevels = this._factory._instance.traceLevels();
            try {
                DEV: console.assert(this._index < this._endpoints.length);
                this._current = this._endpoints[this._index++];

                if (traceLevels.network >= 2) {
                    const s = [];
                    s.push("trying to establish ");
                    s.push(this._current.protocol());
                    s.push(" connection to ");
                    s.push(this._current.toConnectorString());
                    this._factory._instance.initializationData().logger.trace(traceLevels.networkCat, s.join(""));
                }

                start(this._factory.createConnection(this._current.connect(), this._current));
            } catch (ex) {
                if (traceLevels.network >= 2) {
                    const s = [];
                    s.push("failed to establish ");
                    s.push(this._current.protocol());
                    s.push(" connection to ");
                    s.push(this._current.toString());
                    s.push("\n");
                    s.push(ex.toString());
                    this._factory._instance.initializationData().logger.trace(traceLevels.networkCat, s.join(""));
                }

                if (this.connectionStartFailedImpl(ex)) {
                    continue;
                }
            }
            break;
        }
    }

    connectionStartFailedImpl(ex) {
        if (ex instanceof LocalException) {
            this._factory.handleConnectionException(ex, this._hasMore || this._index < this._endpoints.length);
            if (ex instanceof CommunicatorDestroyedException) {
                // No need to continue.
                this._factory.finishGetConnectionEx(this._endpoints, ex, this);
            } else if (this._index < this._endpoints.length) {
                // Try the next endpoint.
                return true;
            } else {
                this._factory.finishGetConnectionEx(this._endpoints, ex, this);
            }
        } else {
            this._factory.finishGetConnectionEx(this._endpoints, ex, this);
        }
        return false;
    }
}
