// Copyright (c) ZeroC, Inc.

import { HashMap } from "./HashMap.js";
import { NoEndpointException } from "./LocalExceptions.js";
import { ObjectPrx } from "./ObjectPrx.js";

export class RouterInfo {
    constructor(router) {
        this._router = router;

        DEV: console.assert(this._router !== null);

        this._clientEndpoints = null;
        this._adapter = null;
        this._identities = new HashMap(HashMap.compareEquals); // Set<Identity> = Map<Identity, 1>
        this._evictedIdentities = [];
        this._hasRoutingTable = false;
    }

    destroy() {
        this._clientEndpoints = [];
        this._adapter = null;
        this._identities.clear();
    }

    equals(rhs) {
        if (this === rhs) {
            return true;
        }

        if (rhs instanceof RouterInfo) {
            return this._router.equals(rhs._router);
        }

        return false;
    }

    hashCode() {
        return this._router.hashCode();
    }

    getRouter() {
        return this._router;
    }

    async getClientEndpoints() {
        if (this._clientEndpoints === null) {
            const [clientProxy, hasRoutingTable] = await this._router.getClientProxy();
            this._hasRoutingTable = hasRoutingTable ?? true;
            this._clientEndpoints =
                clientProxy === null
                    ? this._router._getReference().getEndpoints()
                    : clientProxy._getReference().getEndpoints();
        }
        return this._clientEndpoints;
    }

    async getServerEndpoints() {
        let serverProxy = await this._router.getServerProxy();
        if (serverProxy === null) {
            throw new NoEndpointException("Router::getServerProxy returned a null proxy.");
        }
        return serverProxy._getReference().getEndpoints();
    }

    async addProxy(reference) {
        DEV: console.assert(reference !== null);
        const identity = reference.getIdentity();
        // If the router maintains a routing table, and the proxy is not already in our local map,
        // add it to the router.
        if (this._hasRoutingTable && !this._identities.has(identity)) {
            const evictedProxies = await this._router.addProxies([new ObjectPrx(reference)]);
            this.addAndEvictProxies(identity, evictedProxies);
        }
    }

    setAdapter(adapter) {
        this._adapter = adapter;
    }

    getAdapter() {
        return this._adapter;
    }

    clearCache(ref) {
        this._identities.delete(ref.getIdentity());
    }

    addAndEvictProxies(identity, evictedProxies) {
        // Check if the proxy hasn't already been evicted by a concurrent addProxies call. If it's the case, don't add
        // it to our local map.
        const index = this._evictedIdentities.findIndex(e => e.equals(identity));
        if (index >= 0) {
            this._evictedIdentities.splice(index, 1);
        } else {
            // If we successfully added the proxy to the router, we add it to our local map.
            this._identities.set(identity, 1);
        }

        // Remove the evicted proxies from our local map.
        for (const proxy of evictedProxies) {
            this._identities.delete(proxy.ice_getIdentity());
        }
    }
}
