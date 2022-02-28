//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/Debug");
require("../Ice/Exception");
require("../Ice/HashMap");
require("../Ice/IdentityUtil");
require("../Ice/LocalException");
require("../Ice/Locator");
require("../Ice/Promise");
require("../Ice/Protocol");

const Debug = Ice.Debug;
const HashMap = Ice.HashMap;
const LocatorRegistryPrx = Ice.LocatorRegisterPrx;
const Protocol = Ice.Protocol;

class LocatorInfo
{
    constructor(locator, table, background)
    {
        this._locator = locator;
        this._locatorRegistry = null;
        this._table = table;
        this._background = background;

        this._adapterRequests = new Map(); // Map<String, Request>
        this._objectRequests = new HashMap(HashMap.compareEquals); // Map<Ice.Identity, Request>
    }

    destroy()
    {
        this._locatorRegistry = null;
        this._table.clear();
    }

    equals(rhs)
    {
        if(this === rhs)
        {
            return true;
        }

        if(rhs instanceof LocatorInfo)
        {
            return this._locator.equals(rhs._locator);
        }

        return false;
    }

    hashCode()
    {
        return this._locator.hashCode();
    }

    getLocator()
    {
        return this._locator;
    }

    getLocatorRegistry()
    {
        if(this._locatorRegistry !== null)
        {
            return Ice.Promise.resolve(this._locatorRegistry);
        }

        return this._locator.getRegistry().then(reg =>
            {
                //
                // The locator registry can't be located. We use ordered
                // endpoint selection in case the locator returned a proxy
                // with some endpoints which are prefered to be tried first.
                //
                this._locatorRegistry = LocatorRegistryPrx.uncheckedCast(reg.ice_locator(null).ice_endpointSelection(
                    Ice.EndpointSelectionType.Ordered));
                return this._locatorRegistry;
            });
    }

    getEndpoints(ref, wellKnownRef, ttl, p)
    {
        const promise = p || new Ice.Promise(); // success callback receives (endpoints, cached)

        Debug.assert(ref.isIndirect());
        let endpoints = null;
        const cached = {value: false};
        if(!ref.isWellKnown())
        {
            endpoints = this._table.getAdapterEndpoints(ref.getAdapterId(), ttl, cached);
            if(!cached.value)
            {
                if(this._background && endpoints !== null)
                {
                    this.getAdapterRequest(ref).addCallback(ref, wellKnownRef, ttl, null);
                }
                else
                {
                    this.getAdapterRequest(ref).addCallback(ref, wellKnownRef, ttl, promise);
                    return promise;
                }
            }
        }
        else
        {
            const r = this._table.getObjectReference(ref.getIdentity(), ttl, cached);
            if(!cached.value)
            {
                if(this._background && r !== null)
                {
                    this.getObjectRequest(ref).addCallback(ref, null, ttl, null);
                }
                else
                {
                    this.getObjectRequest(ref).addCallback(ref, null, ttl, promise);
                    return promise;
                }
            }

            if(!r.isIndirect())
            {
                endpoints = r.getEndpoints();
            }
            else if(!r.isWellKnown())
            {
                if(ref.getInstance().traceLevels().location >= 1)
                {
                    this.traceWellKnown("found adapter for well-known object in locator cache", ref, r);
                }
                this.getEndpoints(r, ref, ttl, promise);
                return promise;
            }
        }

        Debug.assert(endpoints !== null);
        if(ref.getInstance().traceLevels().location >= 1)
        {
            this.getEndpointsTrace(ref, endpoints, true);
        }
        promise.resolve([endpoints, true]);

        return promise;
    }

    clearCache(ref)
    {
        Debug.assert(ref.isIndirect());

        if(!ref.isWellKnown())
        {
            const endpoints = this._table.removeAdapterEndpoints(ref.getAdapterId());

            if(endpoints !== null && ref.getInstance().traceLevels().location >= 2)
            {
                this.trace("removed endpoints for adapter from locator cache", ref, endpoints);
            }
        }
        else
        {
            const r = this._table.removeObjectReference(ref.getIdentity());
            if(r !== null)
            {
                if(!r.isIndirect())
                {
                    if(ref.getInstance().traceLevels().location >= 2)
                    {
                        this.trace("removed endpoints for well-known object from locator cache", ref, r.getEndpoints());
                    }
                }
                else if(!r.isWellKnown())
                {
                    if(ref.getInstance().traceLevels().location >= 2)
                    {
                        this.traceWellKnown("removed adapter for well-known object from locator cache", ref, r);
                    }
                    this.clearCache(r);
                }
            }
        }
    }

    trace(msg, ref, endpoints)
    {
        Debug.assert(ref.isIndirect());

        const s = [];
        s.push(msg);
        s.push("\n");
        if(!ref.isWellKnown())
        {
            s.push("adapter = ");
            s.push(ref.getAdapterId());
            s.push("\n");
        }
        else
        {
            s.push("well-known proxy = ");
            s.push(ref.toString());
            s.push("\n");
        }

        s.push("endpoints = ");
        s.push(endpoints.map(e => e.toString()).join(":"));
        ref.getInstance().initializationData().logger.trace(ref.getInstance().traceLevels().locationCat, s.join(""));
    }

    traceWellKnown(msg, ref, resolved)
    {
        Debug.assert(ref.isWellKnown());

        const s = [];
        s.push(msg);
        s.push("\n");
        s.push("well-known proxy = ");
        s.push(ref.toString());
        s.push("\n");

        s.push("adapter = ");
        s.push(resolved.getAdapterId());
        ref.getInstance().initializationData().logger.trace(ref.getInstance().traceLevels().locationCat, s.join(""));
    }

    getEndpointsException(ref, exc)
    {
        Debug.assert(ref.isIndirect());

        const instance = ref.getInstance();
        try
        {
            throw exc;
        }
        catch(ex)
        {
            if(ex instanceof Ice.AdapterNotFoundException)
            {
                if(instance.traceLevels().location >= 1)
                {
                    const s = [];
                    s.push("adapter not found\n");
                    s.push("adapter = ");
                    s.push(ref.getAdapterId());
                    instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.join(""));
                }

                const e = new Ice.NotRegisteredException();
                e.kindOfObject = "object adapter";
                e.id = ref.getAdapterId();
                throw e;
            }
            else if(ex instanceof Ice.ObjectNotFoundException)
            {
                if(instance.traceLevels().location >= 1)
                {
                    const s = [];
                    s.push("object not found\n");
                    s.push("object = ");
                    s.push(Ice.identityToString(ref.getIdentity(), instance.toStringMode()));
                    instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.join(""));
                }

                const e = new Ice.NotRegisteredException();
                e.kindOfObject = "object";
                e.id = Ice.identityToString(ref.getIdentity(), instance.toStringMode());
                throw e;
            }
            else if(ex instanceof Ice.NotRegisteredException)
            {
                throw ex;
            }
            else if(ex instanceof Ice.LocalException)
            {
                if(instance.traceLevels().location >= 1)
                {
                    const s = [];
                    s.push("couldn't contact the locator to retrieve endpoints\n");
                    if(ref.getAdapterId().length > 0)
                    {
                        s.push("adapter = ");
                        s.push(ref.getAdapterId());
                        s.push("\n");
                    }
                    else
                    {
                        s.push("well-known proxy = ");
                        s.push(ref.toString());
                        s.push("\n");
                    }
                    s.push("reason = " + ex.toString());
                    instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.join(""));
                }
                throw ex;
            }
            else
            {
                Debug.assert(false);
            }
        }
    }

    getEndpointsTrace(ref, endpoints, cached)
    {
        if(endpoints !== null && endpoints.length > 0)
        {
            if(cached)
            {
                if(ref.isWellKnown())
                {
                    this.trace("found endpoints for well-known proxy in locator cache", ref, endpoints);
                }
                else
                {
                    this.trace("found endpoints for adapter in locator cache", ref, endpoints);
                }
            }
            else if(ref.isWellKnown())
            {
                this.trace("retrieved endpoints for well-known proxy from locator, adding to locator cache",
                           ref, endpoints);
            }
            else
            {
                this.trace("retrieved endpoints for adapter from locator, adding to locator cache",
                           ref, endpoints);
            }
        }
        else
        {
            const instance = ref.getInstance();
            const s = [];
            s.push("no endpoints configured for ");
            if(ref.getAdapterId().length > 0)
            {
                s.push("adapter\n");
                s.push("adapter = ");
                s.push(ref.getAdapterId());
                s.push("\n");
            }
            else
            {
                s.push("well-known object\n");
                s.push("well-known proxy = ");
                s.push(ref.toString());
                s.push("\n");
            }
            instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.join(""));
        }
    }

    getAdapterRequest(ref)
    {
        if(ref.getInstance().traceLevels().location >= 1)
        {
            const instance = ref.getInstance();
            const s = [];
            s.push("searching for adapter by id\n");
            s.push("adapter = ");
            s.push(ref.getAdapterId());
            instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.join(""));
        }

        let request = this._adapterRequests.get(ref.getAdapterId());
        if(request !== undefined)
        {
            return request;
        }
        request = new AdapterRequest(this, ref);
        this._adapterRequests.set(ref.getAdapterId(), request);
        return request;
    }

    getObjectRequest(ref)
    {
        if(ref.getInstance().traceLevels().location >= 1)
        {
            const instance = ref.getInstance();
            const s = [];
            s.push("searching for well-known object\n");
            s.push("well-known proxy = ");
            s.push(ref.toString());
            instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.join(""));
        }

        let request = this._objectRequests.get(ref.getIdentity());
        if(request !== undefined)
        {
            return request;
        }
        request = new ObjectRequest(this, ref);
        this._objectRequests.set(ref.getIdentity(), request);
        return request;
    }

    finishRequest(ref, wellKnownRefs, proxy, notRegistered)
    {
        if(proxy === null || proxy._getReference().isIndirect())
        {
            //
            // Remove the cached references of well-known objects for which we tried
            // to resolved the endpoints if these endpoints are empty.
            //
            for(let i = 0; i < wellKnownRefs.length; ++i)
            {
                this._table.removeObjectReference(wellKnownRefs[i].getIdentity());
            }
        }

        if(!ref.isWellKnown())
        {
            if(proxy !== null && !proxy._getReference().isIndirect())
            {
                // Cache the adapter endpoints.
                this._table.addAdapterEndpoints(ref.getAdapterId(), proxy._getReference().getEndpoints());
            }
            else if(notRegistered) // If the adapter isn't registered anymore, remove it from the cache.
            {
                this._table.removeAdapterEndpoints(ref.getAdapterId());
            }

            Debug.assert(this._adapterRequests.has(ref.getAdapterId()));
            this._adapterRequests.delete(ref.getAdapterId());
        }
        else
        {
            if(proxy !== null && !proxy._getReference().isWellKnown())
            {
                // Cache the well-known object reference.
                this._table.addObjectReference(ref.getIdentity(), proxy._getReference());
            }
            else if(notRegistered) // If the well-known object isn't registered anymore, remove it from the cache.
            {
                this._table.removeObjectReference(ref.getIdentity());
            }

            Debug.assert(this._objectRequests.has(ref.getIdentity()));
            this._objectRequests.delete(ref.getIdentity());
        }
    }
}

Ice.LocatorInfo = LocatorInfo;

class RequestCallback
{
    constructor(ref, ttl, promise)
    {
        this._ref = ref;
        this._ttl = ttl;
        this._promise = promise;
    }

    response(locatorInfo, proxy)
    {
        let endpoints = null;
        if(proxy !== null)
        {
            const r = proxy._getReference();
            if(this._ref.isWellKnown() && !Protocol.isSupported(this._ref.getEncoding(), r.getEncoding()))
            {
                //
                // If a well-known proxy and the returned proxy
                // encoding isn't supported, we're done: there's
                // no compatible endpoint we can use.
                //
            }
            else if(!r.isIndirect())
            {
                endpoints = r.getEndpoints();
            }
            else if(this._ref.isWellKnown() && !r.isWellKnown())
            {
                //
                // We're resolving the endpoints of a well-known object and the proxy returned
                // by the locator is an indirect proxy. We now need to resolve the endpoints
                // of this indirect proxy.
                //
                if(this._ref.getInstance().traceLevels().location >= 1)
                {
                    locatorInfo.traceWellKnown("retrieved adapter for well-known object from locator, " +
                                               "adding to locator cache", this._ref, r);
                }
                locatorInfo.getEndpoints(r, this._ref, this._ttl).then(
                    values =>
                    {
                        if(this._promise !== null)
                        {
                            this._promise.resolve(values);
                        }
                    },
                    ex =>
                    {
                        if(this._promise !== null)
                        {
                            this._promise.reject(ex);
                        }
                    });
                return;
            }
        }

        if(this._ref.getInstance().traceLevels().location >= 1)
        {
            locatorInfo.getEndpointsTrace(this._ref, endpoints, false);
        }

        if(this._promise !== null)
        {
            this._promise.resolve(endpoints === null ? [[], false] : [endpoints, false]);
        }
    }

    exception(locatorInfo, exc)
    {
        try
        {
            locatorInfo.getEndpointsException(this._ref, exc); // This throws.
        }
        catch(ex)
        {
            if(this._promise !== null)
            {
                this._promise.reject(ex);
            }
        }
    }
}

class Request
{
    constructor(locatorInfo, ref)
    {
        this._locatorInfo = locatorInfo;
        this._ref = ref;

        this._callbacks = []; // Array<RequestCallback>
        this._wellKnownRefs = []; // Array<Reference>
        this._sent = false;
        this._response = false;
        this._proxy = null;
        this._exception = null;
    }

    addCallback(ref, wellKnownRef, ttl, promise)
    {
        const callback = new RequestCallback(ref, ttl, promise);
        if(this._response)
        {
            callback.response(this._locatorInfo, this._proxy);
        }
        else if(this._exception !== null)
        {
            callback.exception(this._locatorInfo, this._exception);
        }
        else
        {
            this._callbacks.push(callback);
            if(wellKnownRef !== null) // This request is to resolve the endpoints of a cached well-known object ref
            {
                this._wellKnownRefs.push(wellKnownRef);
            }
            if(!this._sent)
            {
                this._sent = true;
                this.send();
            }
        }
    }

    response(proxy)
    {
        this._locatorInfo.finishRequest(this._ref, this._wellKnownRefs, proxy, false);
        this._response = true;
        this._proxy = proxy;
        for(let i = 0; i < this._callbacks.length; ++i)
        {
            this._callbacks[i].response(this._locatorInfo, proxy);
        }
    }

    exception(ex)
    {
        this._locatorInfo.finishRequest(this._ref, this._wellKnownRefs, null, ex instanceof Ice.UserException);
        this._exception = ex;
        for(let i = 0; i < this._callbacks.length; ++i)
        {
            this._callbacks[i].exception(this._locatorInfo, ex);
        }
    }
}

class ObjectRequest extends Request
{
    constructor(locatorInfo, reference)
    {
        super(locatorInfo, reference);
        Debug.assert(reference.isWellKnown());
    }

    send()
    {
        try
        {
            this._locatorInfo.getLocator().findObjectById(this._ref.getIdentity()).then(
                proxy => this.response(proxy),
                ex => this.exception(ex));
        }
        catch(ex)
        {
            this.exception(ex);
        }
    }
}

class AdapterRequest extends Request
{
    constructor(locatorInfo, reference)
    {
        super(locatorInfo, reference);
        Debug.assert(reference.isIndirect());
    }

    send()
    {
        try
        {
            this._locatorInfo.getLocator().findAdapterById(this._ref.getAdapterId()).then(
                proxy => this.response(proxy),
                ex => this.exception(ex));
        }
        catch(ex)
        {
            this.exception(ex);
        }
    }
}

module.exports.Ice = Ice;
