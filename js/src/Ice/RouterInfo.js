//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/Debug");
require("../Ice/Exception");
require("../Ice/HashMap");
require("../Ice/LocalException");
require("../Ice/Promise");

const Debug = Ice.Debug;
const HashMap = Ice.HashMap;

class RouterInfo
{
    constructor(router)
    {
        this._router = router;

        Debug.assert(this._router !== null);

        this._clientEndpoints = null;
        this._adapter = null;
        this._identities = new HashMap(HashMap.compareEquals); // Set<Identity> = Map<Identity, 1>
        this._evictedIdentities = [];
        this._hasRoutingTable = false;
    }

    destroy()
    {
        this._clientEndpoints = [];
        this._adapter = null;
        this._identities.clear();
    }

    equals(rhs)
    {
        if(this === rhs)
        {
            return true;
        }

        if(rhs instanceof RouterInfo)
        {
            return this._router.equals(rhs._router);
        }

        return false;
    }

    hashCode()
    {
        return this._router.hashCode();
    }

    getRouter()
    {
        //
        // No mutex lock necessary, _router is immutable.
        //
        return this._router;
    }

    getClientEndpoints()
    {
        const promise = new Ice.Promise();
        if(this._clientEndpoints !== null)
        {
            promise.resolve(this._clientEndpoints);
        }
        else
        {
            this._router.getClientProxy().then(result =>
                           this.setClientEndpoints(result[0],
                                                   result[1] !== undefined ? result[1] : true,
                                                   promise)).catch(promise.reject);
        }
        return promise;
    }

    getServerEndpoints()
    {
        return this._router.getServerProxy().then(serverProxy => {
            if(serverProxy === null)
            {
                throw new Ice.NoEndpointException();
            }
            serverProxy = serverProxy.ice_router(null); // The server proxy cannot be routed.
            return serverProxy._getReference().getEndpoints();
        });
    }

    addProxy(proxy)
    {
        Debug.assert(proxy !== null);
        if(!this._hasRoutingTable)
        {
            return Ice.Promise.resolve(); // The router implementation doesn't maintain a routing table.
        }
        else if(this._identities.has(proxy.ice_getIdentity()))
        {
            //
            // Only add the proxy to the router if it's not already in our local map.
            //
            return Ice.Promise.resolve();
        }
        else
        {
            return this._router.addProxies([proxy]).then(
                evictedProxies =>
                {
                    this.addAndEvictProxies(proxy, evictedProxies);
                });
        }
    }

    setAdapter(adapter)
    {
        this._adapter = adapter;
    }

    getAdapter()
    {
        return this._adapter;
    }

    clearCache(ref)
    {
        this._identities.delete(ref.getIdentity());
    }

    setClientEndpoints(clientProxy, hasRoutingTable, promise)
    {
        if(this._clientEndpoints === null)
        {
            this._hasRoutingTable = hasRoutingTable;
            if(clientProxy === null)
            {
                //
                // If getClientProxy() return nil, use router endpoints.
                //
                this._clientEndpoints = this._router._getReference().getEndpoints();
                promise.resolve(this._clientEndpoints);
            }
            else
            {
                clientProxy = clientProxy.ice_router(null); // The client proxy cannot be routed.

                //
                // In order to avoid creating a new connection to the
                // router, we must use the same timeout as the already
                // existing connection.
                //
                this._router.ice_getConnection().then(
                    con =>
                    {
                        this._clientEndpoints = clientProxy.ice_timeout(con.timeout())._getReference().getEndpoints();
                        promise.resolve(this._clientEndpoints);
                    }).catch(promise.reject);
            }
        }
        else
        {
            promise.resolve(this._clientEndpoints);
        }
    }

    addAndEvictProxies(proxy, evictedProxies)
    {
        //
        // Check if the proxy hasn't already been evicted by a
        // concurrent addProxies call. If it's the case, don't
        // add it to our local map.
        //
        const index = this._evictedIdentities.findIndex(e => e.equals(proxy.ice_getIdentity()));
        if(index >= 0)
        {
            this._evictedIdentities.splice(index, 1);
        }
        else
        {
            //
            // If we successfully added the proxy to the router,
            // we add it to our local map.
            //
            this._identities.set(proxy.ice_getIdentity(), 1);
        }

        //
        // We also must remove whatever proxies the router evicted.
        //
        evictedProxies.forEach(proxy =>
            {
                this._identities.delete(proxy.ice_getIdentity());
            });
    }
}

Ice.RouterInfo = RouterInfo;
module.exports.Ice = Ice;
