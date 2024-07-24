//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { HashMap } from "./HashMap.js";
import { Promise } from "./Promise.js";
import { NoEndpointException } from "./LocalException.js";
import { Debug } from "./Debug.js";
import { ObjectPrx } from "./ObjectPrx.js";

export class RouterInfo {
    constructor(router) {
        this._router = router;

        Debug.assert(this._router !== null);

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

    getClientEndpoints() {
        const promise = new Promise();
        if (this._clientEndpoints !== null) {
            promise.resolve(this._clientEndpoints);
        } else {
            this._router
                .getClientProxy()
                .then(result => this.setClientEndpoints(result[0], result[1] !== undefined ? result[1] : true, promise))
                .catch(promise.reject);
        }
        return promise;
    }

    getServerEndpoints() {
        return this._router.getServerProxy().then(serverProxy => {
            if (serverProxy === null) {
                throw new NoEndpointException();
            }
            serverProxy = serverProxy.ice_router(null); // The server proxy cannot be routed.
            return serverProxy._getReference().getEndpoints();
        });
    }

    addProxy(reference) {
        const identity = reference.getIdentity();
        Debug.assert(reference !== null);
        if (!this._hasRoutingTable) {
            return Promise.resolve(); // The router implementation doesn't maintain a routing table.
        } else if (this._identities.has(identity)) {
            // Only add the proxy to the router if it's not already in our local map.
            return Promise.resolve();
        } else {
            return this._router.addProxies([new ObjectPrx(reference)]).then(evictedProxies => {
                this.addAndEvictProxies(identity, evictedProxies);
            });
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

    setClientEndpoints(clientProxy, hasRoutingTable, promise) {
        if (this._clientEndpoints === null) {
            this._hasRoutingTable = hasRoutingTable;
            this._clientEndpoints =
                clientProxy === null
                    ? this._router._getReference().getEndpoints()
                    : clientProxy._getReference().getEndpoints();
        }
        promise.resolve(this._clientEndpoints);
    }

    addAndEvictProxies(identity, evictedProxies) {
        //
        // Check if the proxy hasn't already been evicted by a
        // concurrent addProxies call. If it's the case, don't
        // add it to our local map.
        //
        const index = this._evictedIdentities.findIndex(e => e.equals(identity));
        if (index >= 0) {
            this._evictedIdentities.splice(index, 1);
        } else {
            //
            // If we successfully added the proxy to the router,
            // we add it to our local map.
            //
            this._identities.set(identity, 1);
        }

        //
        // We also must remove whatever proxies the router evicted.
        //
        evictedProxies.forEach(proxy => {
            this._identities.delete(proxy.ice_getIdentity());
        });
    }
}
