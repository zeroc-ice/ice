// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice._ModuleRegistry.require(module,
    [
        "../Ice/AsyncResultBase",
        "../Ice/Debug",
        "../Ice/Identity",
        "../Ice/LocalException",
        "../Ice/PropertyNames",
        "../Ice/Router",
        "../Ice/ServantManager",
        "../Ice/StringUtil",
        "../Ice/UUID"
    ]);

const AsyncResultBase = Ice.AsyncResultBase;
const Debug = Ice.Debug;
const Identity = Ice.Identity;
const PropertyNames = Ice.PropertyNames;
const ServantManager = Ice.ServantManager;
const StringUtil = Ice.StringUtil;

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
//const StateHeld = 1;
//const StateWaitActivate = 2;
//const StateActive = 3;
//const StateDeactivating = 4;
const StateDeactivated = 5;
const StateDestroyed  = 6;

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
        this._routerEndpoints = [];
        this._routerInfo = null;
        this._state = StateUninitialized;
        this._noConfig = noConfig;

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
                // Add the router's server proxy endpoints to this object
                // adapter.
                //
                this._routerInfo.getServerEndpoints().then(
                    (endpoints) =>
                    {
                        endpoints.forEach(endpoint => this._routerEndpoints.push(endpoint));
                        this._routerEndpoints.sort((e1, e2) => e1.compareTo(e2));    // Must be sorted.

                        //
                        // Remove duplicate endpoints, so we have a list of unique
                        // endpoints.
                        //
                        for(let i = 0; i < this._routerEndpoints.length - 1;)
                        {
                            if(this._routerEndpoints[i].equals(this._routerEndpoints[i + 1]))
                            {
                                this._routerEndpoints.splice(i, 1);
                            }
                            else
                            {
                                ++i;
                            }
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
                        return this._instance.outgoingConnectionFactory().setRouterInfo(this._routerInfo);
                    }
                ).then(() => promise.resolve(this), promise.reject);
            }
            else
            {
                const endpoints = properties.getProperty(this._name + ".Endpoints");
                if(endpoints.length > 0)
                {
                    throw new Ice.FeatureNotSupportedException("object adapter endpoints not supported");
                }
                promise.resolve(this);
            }
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
    }

    hold()
    {
        this.checkForDeactivation();
    }

    waitForHold()
    {
        const promise = new AsyncResultBase(this._communicator, "waitForHold", null, null, this);
        return this.checkForDeactivation(promise) ? promise : promise.resolve();
    }

    deactivate()
    {
        const promise = new AsyncResultBase(this._communicator, "deactivate", null, null, this);
        if(this._state < StateDeactivated)
        {
            this._state = StateDeactivated;
            this._instance.outgoingConnectionFactory().removeAdapter(this);
        }
        promise.resolve();
        return promise;
    }

    waitForDeactivate()
    {
        return new AsyncResultBase(this._communicator, "deactivate", null, null, this).resolve();
    }

    isDeactivated()
    {
        return this._state >= StateDeactivated;
    }

    destroy()
    {
        const promise = new AsyncResultBase(this._communicator, "destroy", null, null, this);
        const destroyInternal = () =>
        {
            if(this._state < StateDestroyed)
            {
                this._state = StateDestroyed;
                this._servantManager.destroy();
                this._objectAdapterFactory.removeObjectAdapter(this);
            }
            return promise.resolve();
        };

        return this._state < StateDeactivated ? this.deactivate().then(destroyInternal) : destroyInternal();
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
        throw new Ice.FeatureNotSupportedException("refreshPublishedEndpoints not supported");
    }

    getPublishedEndpoints()
    {
        return [];
    }

    setPublishedEndpoints(newEndpoints)
    {
        throw new Ice.FeatureNotSupportedException("setPublishedEndpoints not supported");
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
        connection.setAdapterAndServantManager(this, _servantManager);
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
            this._instance.referenceFactory().create(ident, facet, this._reference,
                                                     Array.from(this._routerEndpoints)));
    }

    checkForDeactivation(promise)
    {
        if(this._state >= StateDeactivated)
        {
            const ex = new Ice.ObjectAdapterDeactivatedException();
            ex.name = this.getName();

            if(promise !== undefined)
            {
                promise.reject(ex);
                return true;
            }
            else
            {
                throw ex;
            }
        }

        return false;
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
        let props = this._instance.initializationData().properties.getPropertiesForPrefix(prefix);
        for(let [key, value] of props)
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
}

Ice.ObjectAdapterI = ObjectAdapterI;
module.exports.Ice = Ice;
