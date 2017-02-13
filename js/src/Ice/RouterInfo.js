// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/ArrayUtil",
        "../Ice/Debug",
        "../Ice/HashMap",
        "../Ice/Promise",
        "../Ice/LocalException",
        "../Ice/Exception"
    ]);

var ArrayUtil = Ice.ArrayUtil;
var Debug = Ice.Debug;
var HashMap = Ice.HashMap;
var Promise = Ice.Promise;

var RouterInfo = Ice.Class({
    __init__: function(router)
    {
        this._router = router;

        Debug.assert(this._router !== null);

        this._clientEndpoints = null;
        this._serverEndpoints = null;
        this._adapter = null;
        this._identities = new HashMap(HashMap.compareEquals); // Set<Identity> = Map<Identity, 1>
        this._evictedIdentities = [];
    },
    destroy: function()
    {
        this._clientEndpoints = [];
        this._serverEndpoints = [];
        this._adapter = null;
        this._identities.clear();
    },
    equals: function(rhs)
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
    },
    hashCode: function()
    {
        return this._router.hashCode();
    },
    getRouter: function()
    {
        //
        // No mutex lock necessary, _router is immutable.
        //
        return this._router;
    },
    getClientEndpoints: function()
    {
        var promise = new Promise();

        if(this._clientEndpoints !== null)
        {
            promise.succeed(this._clientEndpoints);
        }
        else
        {
            var self = this;
            this._router.getClientProxy().then(
                function(clientProxy)
                {
                    self.setClientEndpoints(clientProxy, promise);
                }).exception(
                    function(ex)
                    {
                        promise.fail(ex);
                    });
        }

        return promise;
    },
    getServerEndpoints: function()
    {
        if(this._serverEndpoints !== null) // Lazy initialization.
        {
            return new Promise().succeed(this._serverEndpoints);
        }
        else
        {
            var self = this;
            return this._router.getServerProxy().then(
                function(proxy)
                {
                    return self.setServerEndpoints(proxy);
                });
        }
    },
    addProxy: function(proxy)
    {
        Debug.assert(proxy !== null);

        if(this._identities.has(proxy.ice_getIdentity()))
        {
            //
            // Only add the proxy to the router if it's not already in our local map.
            //
            return new Promise().succeed();
        }
        else
        {
            var self = this;
            return this._router.addProxies([ proxy ]).then(
                function(evictedProxies)
                {
                    self.addAndEvictProxies(proxy, evictedProxies);
                });
        }
    },
    setAdapter: function(adapter)
    {
        this._adapter = adapter;
    },
    getAdapter: function()
    {
        return this._adapter;
    },
    clearCache: function(ref)
    {
        this._identities.delete(ref.getIdentity());
    },
    setClientEndpoints: function(clientProxy, promise)
    {
        if(this._clientEndpoints === null)
        {
            if(clientProxy === null)
            {
                //
                // If getClientProxy() return nil, use router endpoints.
                //
                this._clientEndpoints = this._router.__reference().getEndpoints();
                promise.succeed(this._clientEndpoints);
            }
            else
            {
                clientProxy = clientProxy.ice_router(null); // The client proxy cannot be routed.

                //
                // In order to avoid creating a new connection to the
                // router, we must use the same timeout as the already
                // existing connection.
                //
                var self = this;
                this._router.ice_getConnection().then(
                    function(con)
                    {
                        var proxy = clientProxy.ice_timeout(con.timeout());
                        self._clientEndpoints = proxy.__reference().getEndpoints();
                        promise.succeed(self._clientEndpoints);
                    }).exception(
                        function(ex)
                        {
                            promise.fail(ex);
                        });
            }
        }
        else
        {
            promise.succeed(this._clientEndpoints);
        }
    },
    setServerEndpoints: function(serverProxy)
    {
        if(serverProxy === null)
        {
            throw new Ice.NoEndpointException();
        }

        serverProxy = serverProxy.ice_router(null); // The server proxy cannot be routed.
        this._serverEndpoints = serverProxy.__reference().getEndpoints();
        return this._serverEndpoints;
    },
    addAndEvictProxies: function(proxy, evictedProxies)
    {
        //
        // Check if the proxy hasn't already been evicted by a
        // concurrent addProxies call. If it's the case, don't
        // add it to our local map.
        //
        var index = ArrayUtil.indexOf(this._evictedIdentities, proxy.ice_getIdentity(),
                                    function(i1, i2) { return i1.equals(i2); });
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
        for(var i = 0; i < evictedProxies.length; ++i)
        {
            this._identities.delete(evictedProxies[i].ice_getIdentity());
        }
    }
});

Ice.RouterInfo = RouterInfo;
module.exports.Ice = Ice;
