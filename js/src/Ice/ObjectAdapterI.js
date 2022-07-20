//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/ArrayUtil");
require("../Ice/AsyncResultBase");
require("../Ice/Debug");
require("../Ice/Identity");
require("../Ice/LocalException");
require("../Ice/Promise");
require("../Ice/PropertyNames");
require("../Ice/Router");
require("../Ice/ServantManager");
require("../Ice/StringUtil");
require("../Ice/Timer");
require("../Ice/UUID");

const ArrayUtil = Ice.ArrayUtil;
const AsyncResultBase = Ice.AsyncResultBase;
const Debug = Ice.Debug;
const Identity = Ice.Identity;
const PropertyNames = Ice.PropertyNames;
const ServantManager = Ice.ServantManager;
const StringUtil = Ice.StringUtil;
const Timer = Ice.Timer;

const _suffixes =
[
    "ACM",
    "AdapterId",
    "Endpoints",
    "Locator",
    "Locator.EncodingVersion",
    "Locator.EndpointSelection",
    "Locator.ConnectionCached",
    "Locator.PreferSecure",
    "Locator.CollocationOptimized",
    "Locator.Router",
    "MessageSizeMax",
    "PublishedEndpoints",
    "ReplicaGroupId",
    "Router",
    "Router.EncodingVersion",
    "Router.EndpointSelection",
    "Router.ConnectionCached",
    "Router.PreferSecure",
    "Router.CollocationOptimized",
    "Router.Locator",
    "Router.Locator.EndpointSelection",
    "Router.Locator.ConnectionCached",
    "Router.Locator.PreferSecure",
    "Router.Locator.CollocationOptimized",
    "Router.Locator.LocatorCacheTimeout",
    "Router.Locator.InvocationTimeout",
    "Router.LocatorCacheTimeout",
    "Router.InvocationTimeout",
    "ProxyOptions",
    "ThreadPool.Size",
    "ThreadPool.SizeMax",
    "ThreadPool.SizeWarn",
    "ThreadPool.StackSize",
    "ThreadPool.Serialize"
];

const StateUninitialized = 0; // Just constructed.
const StateHeld = 1;
// const StateWaitActivate = 2;
const StateActive = 3;
// const StateDeactivating = 4;
const StateDeactivated = 5;
const StateDestroyed = 6;

//
// Only for use by IceInternal.ObjectAdapterFactory
//
class ObjectAdapterI
{
    constructor(instance, communicator, objectAdapterFactory, name, router, noConfig, promise)
    {
        this._instance = instance;
        this._communicator = communicator;
        this._objectAdapterFactory = objectAdapterFactory;
        this._servantManager = new ServantManager(instance, name);
        this._name = name;
        this._publishedEndpoints = [];
        this._routerInfo = null;
        this._state = StateUninitialized;
        this._noConfig = noConfig;
        this._statePromises = [];

        if(this._noConfig)
        {
            this._reference = this._instance.referenceFactory().createFromString("dummy -t", "");
            this._messageSizeMax = this._instance.messageSizeMax();
            promise.resolve(this);
            return;
        }

        const properties = this._instance.initializationData().properties;
        const unknownProps = [];
        const noProps = this.filterProperties(unknownProps);

        //
        // Warn about unknown object adapter properties.
        //
        if(unknownProps.length !== 0 && properties.getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
        {
            const message = ["found unknown properties for object adapter `" + name + "':"];
            unknownProps.forEach(unknownProp => message.push("\n    " + unknownProp));
            this._instance.initializationData().logger.warning(message.join(""));
        }

        //
        // Make sure named adapter has some configuration.
        //
        if(router === null && noProps)
        {
            throw new Ice.InitializationException(`object adapter \`${this._name}' requires configuration`);
        }

        //
        // Setup a reference to be used to get the default proxy options
        // when creating new proxies. By default, create twoway proxies.
        //
        const proxyOptions = properties.getPropertyWithDefault(this._name + ".ProxyOptions", "-t");
        try
        {
            this._reference = this._instance.referenceFactory().createFromString("dummy " + proxyOptions, "");
        }
        catch(e)
        {
            if(e instanceof Ice.ProxyParseException)
            {
                throw new Ice.InitializationException(
                    `invalid proxy options \`${proxyOptions}' for object adapter \`${name}'`);
            }
            else
            {
                throw e;
            }
        }

        {
            const defaultMessageSizeMax = this._instance.messageSizeMax() / 1024;
            const num = properties.getPropertyAsIntWithDefault(this._name + ".MessageSizeMax", defaultMessageSizeMax);
            if(num < 1 || num > 0x7fffffff / 1024)
            {
                this._messageSizeMax = 0x7fffffff;
            }
            else
            {
                this._messageSizeMax = num * 1024; // Property is in kilobytes, _messageSizeMax in bytes
            }
        }

        try
        {
            if(router === null)
            {
                router = Ice.RouterPrx.uncheckedCast(
                    this._instance.proxyFactory().propertyToProxy(this._name + ".Router"));
            }
            let p;
            if(router !== null)
            {
                this._routerInfo = this._instance.routerManager().find(router);
                Debug.assert(this._routerInfo !== null);

                //
                // Make sure this router is not already registered with another adapter.
                //
                if(this._routerInfo.getAdapter() !== null)
                {
                    throw new Ice.AlreadyRegisteredException(
                        "object adapter with router",
                        Ice.identityToString(router.ice_getIdentity(), this._instance.toStringMode()));
                }

                //
                // Associate this object adapter with the router. This way,
                // new outgoing connections to the router's client proxy will
                // use this object adapter for callbacks.
                //
                this._routerInfo.setAdapter(this);

                //
                // Also modify all existing outgoing connections to the
                // router's client proxy to use this object adapter for
                // callbacks.
                //
                p = this._instance.outgoingConnectionFactory().setRouterInfo(this._routerInfo);
            }
            else
            {
                const endpoints = properties.getProperty(this._name + ".Endpoints");
                if(endpoints.length > 0)
                {
                    throw new Ice.FeatureNotSupportedException("object adapter endpoints not supported");
                }
                p = Ice.Promise.resolve();
            }

            p.then(() => this.computePublishedEndpoints()).then(endpoints =>
            {
                this._publishedEndpoints = endpoints;
                promise.resolve(this);
            },
            ex =>
            {
                this.destroy();
                promise.reject(ex);
            });
        }
        catch(ex)
        {
            this.destroy();
            throw ex;
        }
    }

    getName()
    {
        //
        // No mutex lock necessary, _name is immutable.
        //
        return this._noConfig ? "" : this._name;
    }

    getCommunicator()
    {
        return this._communicator;
    }

    activate()
    {
        const promise = new AsyncResultBase(this._communicator, "activate", null, null, this);
        this.setState(StateActive);
        promise.resolve();
        return promise;
    }

    hold()
    {
        this.checkForDeactivation();
        this.setState(StateHeld);
    }

    waitForHold()
    {
        const promise = new AsyncResultBase(this._communicator, "waitForHold", null, null, this);
        try
        {
            this.checkForDeactivation();
            this.waitState(StateHeld, promise);
        }
        catch(ex)
        {
            promise.reject(ex);
        }
        return promise;
    }

    deactivate()
    {
        const promise = new AsyncResultBase(this._communicator, "deactivate", null, null, this);
        if(this._state < StateDeactivated)
        {
            this.setState(StateDeactivated);
            this._instance.outgoingConnectionFactory().removeAdapter(this);
        }
        promise.resolve();
        return promise;
    }

    waitForDeactivate()
    {
        const promise = new AsyncResultBase(this._communicator, "waitForDeactivate", null, null, this);
        this.waitState(StateDeactivated, promise);
        return promise;
    }

    isDeactivated()
    {
        return this._state >= StateDeactivated;
    }

    destroy()
    {
        // NOTE: we don't call waitForDeactivate since it's currently a no-op.
        return this.deactivate().then(() =>
        {
            if(this._state < StateDestroyed)
            {
                this.setState(StateDestroyed);
                this._servantManager.destroy();
                this._objectAdapterFactory.removeObjectAdapter(this);
                this._publishedEndpoints = [];
            }
            const promise = new AsyncResultBase(this._communicator, "destroy", null, null, this);
            promise.resolve();
            return promise;
        });
    }

    add(object, ident)
    {
        return this.addFacet(object, ident, "");
    }

    addFacet(object, ident, facet)
    {
        this.checkForDeactivation();
        this.checkIdentity(ident);
        this.checkServant(object);

        //
        // Create a copy of the Identity argument, in case the caller
        // reuses it.
        //
        const id = ident.clone();

        this._servantManager.addServant(object, id, facet);

        return this.newProxy(id, facet);
    }

    addWithUUID(object)
    {
        return this.addFacetWithUUID(object, "");
    }

    addFacetWithUUID(object, facet)
    {
        return this.addFacet(object, new Identity(Ice.generateUUID(), ""), facet);
    }

    addDefaultServant(servant, category)
    {
        this.checkServant(servant);
        this.checkForDeactivation();

        this._servantManager.addDefaultServant(servant, category);
    }

    remove(ident)
    {
        return this.removeFacet(ident, "");
    }

    removeFacet(ident, facet)
    {
        this.checkForDeactivation();
        this.checkIdentity(ident);

        return this._servantManager.removeServant(ident, facet);
    }

    removeAllFacets(ident)
    {
        this.checkForDeactivation();
        this.checkIdentity(ident);

        return this._servantManager.removeAllFacets(ident);
    }

    removeDefaultServant(category)
    {
        this.checkForDeactivation();

        return this._servantManager.removeDefaultServant(category);
    }

    find(ident)
    {
        return this.findFacet(ident, "");
    }

    findFacet(ident, facet)
    {
        this.checkForDeactivation();
        this.checkIdentity(ident);
        return this._servantManager.findServant(ident, facet);
    }

    findAllFacets(ident)
    {
        this.checkForDeactivation();
        this.checkIdentity(ident);
        return this._servantManager.findAllFacets(ident);
    }

    findByProxy(proxy)
    {
        this.checkForDeactivation();
        const ref = proxy._getReference();
        return this.findFacet(ref.getIdentity(), ref.getFacet());
    }

    findDefaultServant(category)
    {
        this.checkForDeactivation();
        return this._servantManager.findDefaultServant(category);
    }

    addServantLocator(locator, prefix)
    {
        this.checkForDeactivation();
        this._servantManager.addServantLocator(locator, prefix);
    }

    removeServantLocator(prefix)
    {
        this.checkForDeactivation();
        return this._servantManager.removeServantLocator(prefix);
    }

    findServantLocator(prefix)
    {
        this.checkForDeactivation();
        return this._servantManager.findServantLocator(prefix);
    }

    createProxy(ident)
    {
        this.checkForDeactivation();
        this.checkIdentity(ident);
        return this.newProxy(ident, "");
    }

    createDirectProxy(ident)
    {
        return this.createProxy(ident);
    }

    createIndirectProxy(ident)
    {
        throw new Ice.FeatureNotSupportedException("createIndirectProxy not supported");
    }

    setLocator(locator)
    {
        throw new Ice.FeatureNotSupportedException("setLocator not supported");
    }

    getEndpoints()
    {
        return [];
    }

    refreshPublishedEndpoints()
    {
        this.checkForDeactivation();
        return this.computePublishedEndpoints().then(
            endpoints =>
                {
                    this._publishedEndpoints = endpoints;
                });
    }

    getPublishedEndpoints()
    {
        return ArrayUtil.clone(this._publishedEndpoints);
    }

    setPublishedEndpoints(newEndpoints)
    {
        this.checkForDeactivation();
        if(this._routerInfo !== null)
        {
            throw new Error("can't set published endpoints on object adapter associated with a router");
        }
        this._publishedEndpoints = ArrayUtil.clone(newEndpoints);
    }

    getServantManager()
    {
        //
        // _servantManager is immutable.
        //
        return this._servantManager;
    }

    setAdapterOnConnection(connection)
    {
        this.checkForDeactivation();
        connection.setAdapterAndServantManager(this, this._servantManager);
    }

    messageSizeMax()
    {
        return this._messageSizeMax;
    }

    newProxy(ident, facet)
    {
        //
        // Now we also add the endpoints of the router's server proxy, if
        // any. This way, object references created by this object adapter
        // will also point to the router's server proxy endpoints.
        //
        //
        // Create a reference and return a proxy for this reference.
        //
        return this._instance.proxyFactory().referenceToProxy(
            this._instance.referenceFactory().create(ident, facet, this._reference, this._publishedEndpoints));
    }

    checkForDeactivation()
    {
        if(this._state >= StateDeactivated)
        {
            const ex = new Ice.ObjectAdapterDeactivatedException();
            ex.name = this.getName();
            throw ex;
        }
    }

    checkIdentity(ident)
    {
        if(ident.name === undefined || ident.name === null || ident.name.length === 0)
        {
            throw new Ice.IllegalIdentityException(ident);
        }

        if(ident.category === undefined || ident.category === null)
        {
            ident.category = "";
        }
    }

    checkServant(servant)
    {
        if(servant === undefined || servant === null)
        {
            throw new Ice.IllegalServantException("cannot add null servant to Object Adapter");
        }
    }

    computePublishedEndpoints()
    {
        let p;
        if(this._routerInfo !== null)
        {
            p = this._routerInfo.getServerEndpoints().then(
                endpts =>
                    {
                        //
                        // Remove duplicate endpoints, so we have a list of unique endpoints.
                        //
                        const endpoints = [];
                        endpts.forEach(endpoint =>
                                       {
                                           if(endpoints.findIndex(value => endpoint.equals(value)) === -1)
                                           {
                                               endpoints.push(endpoint);
                                           }
                                       });
                        return endpoints;
                    });
        }
        else
        {

            //
            // Parse published endpoints. If set, these are used in proxies
            // instead of the connection factory Endpoints.
            //
            const endpoints = [];
            const s = this._instance.initializationData().properties.getProperty(this._name + ".PublishedEndpoints");
            const delim = " \t\n\r";

            let end = 0;
            let beg;
            while(end < s.length)
            {
                beg = StringUtil.findFirstNotOf(s, delim, end);
                if(beg === -1)
                {
                    if(s != "")
                    {
                        throw new Ice.EndpointParseException("invalid empty object adapter endpoint");
                    }
                    break;
                }

                end = beg;
                while(true)
                {
                    end = s.indexOf(':', end);
                    if(end == -1)
                    {
                        end = s.length;
                        break;
                    }
                    else
                    {
                        let quoted = false;
                        let quote = beg;
                        while(true)
                        {
                            quote = s.indexOf("\"", quote);
                            if(quote == -1 || end < quote)
                            {
                                break;
                            }
                            else
                            {
                                quote = s.indexOf("\"", ++quote);
                                if(quote == -1)
                                {
                                    break;
                                }
                                else if(end < quote)
                                {
                                    quoted = true;
                                    break;
                                }
                                ++quote;
                            }
                        }
                        if(!quoted)
                        {
                            break;
                        }
                        ++end;
                    }
                }

                const es = s.substring(beg, end);
                const endp = this._instance.endpointFactoryManager().create(es, false);
                if(endp === null)
                {
                    throw new Ice.EndpointParseException("invalid object adapter endpoint `" + s + "'");
                }
                endpoints.push(endp);
            }

            p = Ice.Promise.resolve(endpoints);
        }

        return p.then(
            endpoints =>
                {
                    if(this._instance.traceLevels().network >= 1 && endpoints.length > 0)
                    {
                        const s = [];
                        s.push("published endpoints for object adapter `");
                        s.push(this._name);
                        s.push("':\n");
                        let first = true;
                        endpoints.forEach(endpoint =>
                                          {
                                              if(!first)
                                              {
                                                  s.push(":");
                                              }
                                              s.push(endpoint.toString());
                                              first = false;
                                          });
                        this._instance.initializationData().logger.trace(this._instance.traceLevels().networkCat,
                                                                         s.toString());
                    }
                    return endpoints;
                });
    }

    filterProperties(unknownProps)
    {
        //
        // Do not create unknown properties list if Ice prefix, i.e., Ice, Glacier2, etc.
        //
        let addUnknown = true;
        const prefix = this._name + ".";
        for(let i = 0; i < PropertyNames.clPropNames.length; ++i)
        {
            if(prefix.indexOf(PropertyNames.clPropNames[i] + ".") === 0)
            {
                addUnknown = false;
                break;
            }
        }

        let noProps = true;
        const props = this._instance.initializationData().properties.getPropertiesForPrefix(prefix);
        for(const key of props.keys())
        {
            let valid = false;
            for(let i = 0; i < _suffixes.length; ++i)
            {
                if(key === prefix + _suffixes[i])
                {
                    noProps = false;
                    valid = true;
                    break;
                }
            }

            if(!valid && addUnknown)
            {
                unknownProps.push(key);
            }
        }

        return noProps;
    }

    setState(state)
    {
        if(this._state === state)
        {
            return;
        }
        this._state = state;

        let promises = [];
        (state < StateDeactivated ? [state] : [StateHeld, StateDeactivated]).forEach(s =>
        {
            if(this._statePromises[s])
            {
                promises = promises.concat(this._statePromises[s]);
                delete this._statePromises[s];
            }
        });
        if(promises.length > 0)
        {
            Timer.setImmediate(() => promises.forEach(p => p.resolve()));
        }
    }

    waitState(state, promise)
    {
        if(this._state < StateDeactivated &&
           (state === StateHeld && this._state !== StateHeld || state === StateDeactivated))
        {
            if(this._statePromises[state])
            {
                this._statePromises[state].push(promise);
            }
            else
            {
                this._statePromises[state] = [promise];
            }
        }
        else
        {
            promise.resolve();
        }
    }
}

Ice.ObjectAdapterI = ObjectAdapterI;
module.exports.Ice = Ice;
