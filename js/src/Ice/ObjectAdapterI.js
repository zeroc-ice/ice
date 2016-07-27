// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/AsyncResultBase",
        "../Ice/Debug",
        "../Ice/Identity",
        "../Ice/LocalException",
        "../Ice/Promise",
        "../Ice/PropertyNames",
        "../Ice/Router",
        "../Ice/ServantManager",
        "../Ice/StringUtil",
        "../Ice/UUID"
    ]);

var AsyncResultBase = Ice.AsyncResultBase;
var Debug = Ice.Debug;
var Identity = Ice.Identity;
var Promise = Ice.Promise;
var PropertyNames = Ice.PropertyNames;
var ServantManager = Ice.ServantManager;
var StringUtil = Ice.StringUtil;

var _suffixes =
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

var StateUninitialized = 0; // Just constructed.
//var StateHeld = 1;
//var StateWaitActivate = 2;
//var StateActive = 3;
//var StateDeactivating = 4;
var StateDeactivated = 5;
var StateDestroyed  = 6;

//
// Only for use by IceInternal.ObjectAdapterFactory
//
var ObjectAdapterI = Ice.Class({
    __init__: function(instance, communicator, objectAdapterFactory, name, router, noConfig, promise)
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
            promise.succeed(this, promise);
            return;
        }

        var properties = this._instance.initializationData().properties;
        var unknownProps = [];
        var noProps = this.filterProperties(unknownProps);

        //
        // Warn about unknown object adapter properties.
        //
        if(unknownProps.length !== 0 && properties.getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
        {
            var message = ["found unknown properties for object adapter `" + name + "':"];
            for(var i = 0; i < unknownProps.length; ++i)
            {
                message.push("\n    " + unknownProps[i]);
            }
            this._instance.initializationData().logger.warning(message.join(""));
        }

        //
        // Make sure named adapter has some configuration.
        //
        if(router === null && noProps)
        {
            var ex = new Ice.InitializationException();
            ex.reason = "object adapter `" + this._name + "' requires configuration";
            throw ex;
        }

        //
        // Setup a reference to be used to get the default proxy options
        // when creating new proxies. By default, create twoway proxies.
        //
        var proxyOptions = properties.getPropertyWithDefault(this._name + ".ProxyOptions", "-t");
        try
        {
            this._reference = this._instance.referenceFactory().createFromString("dummy " + proxyOptions, "");
        }
        catch(e)
        {
            if(e instanceof Ice.ProxyParseException)
            {
                var ex = new Ice.InitializationException();
                ex.reason = "invalid proxy options `" + proxyOptions + "' for object adapter `" + name + "'";
                throw ex;
            }
            else
            {
                throw e;
            }
        }

        {
            var defaultMessageSizeMax = this._instance.messageSizeMax() / 1024;
            var num = properties.getPropertyAsIntWithDefault(this._name + ".MessageSizeMax", defaultMessageSizeMax);
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
                        Ice.identityToString(router.ice_getIdentity()));
                }

                //
                // Add the router's server proxy endpoints to this object
                // adapter.
                //
                var self = this;
                this._routerInfo.getServerEndpoints().then(
                    function(endpoints)
                    {
                        var i;

                        for(i = 0; i < endpoints.length; ++i)
                        {
                            self._routerEndpoints.push(endpoints[i]);
                        }
                        self._routerEndpoints.sort(     // Must be sorted.
                            function(e1, e2)
                            {
                                return e1.compareTo(e2);
                            });

                        //
                        // Remove duplicate endpoints, so we have a list of unique
                        // endpoints.
                        //
                        for(i = 0; i < self._routerEndpoints.length - 1;)
                        {
                            var e1 = self._routerEndpoints[i];
                            var e2 = self._routerEndpoints[i + 1];
                            if(e1.equals(e2))
                            {
                                self._routerEndpoints.splice(i, 1);
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
                        self._routerInfo.setAdapter(self);

                        //
                        // Also modify all existing outgoing connections to the
                        // router's client proxy to use this object adapter for
                        // callbacks.
                        //
                        return self._instance.outgoingConnectionFactory().setRouterInfo(self._routerInfo);
                    }
                ).then(
                    function()
                    {
                        promise.succeed(self, promise);
                    },
                    function(ex)
                    {
                        promise.fail(ex, promise);
                    });
            }
            else
            {
                var endpoints = properties.getProperty(this._name + ".Endpoints");
                if(endpoints.length > 0)
                {
                    throw new Ice.FeatureNotSupportedException("object adapter endpoints not supported");
                }
                promise.succeed(this, promise);
            }
        }
        catch(ex)
        {
            this.destroy();
            throw ex;
        }
    },
    getName: function()
    {
        //
        // No mutex lock necessary, _name is immutable.
        //
        return this._noConfig ? "" : this._name;
    },
    getCommunicator: function()
    {
        return this._communicator;
    },
    activate: function()
    {
    },
    hold: function()
    {
        this.checkForDeactivation();
    },
    waitForHold: function()
    {
        var promise = new AsyncResultBase(this._communicator, "waitForHold", null, null, this);
        if(this.checkForDeactivation(promise))
        {
            return promise;
        }
        return promise.succeed(promise);
    },
    deactivate: function()
    {
        var promise = new AsyncResultBase(this._communicator, "deactivate", null, null, this);
        if(this._state < StateDeactivated)
        {
            this._state = StateDeactivated;
            this._instance.outgoingConnectionFactory().removeAdapter(this);
        }
        return promise.succeed(promise);
    },
    waitForDeactivate: function()
    {
        var promise = new AsyncResultBase(this._communicator, "deactivate", null, null, this);
        return promise.succeed(promise);
    },
    isDeactivated: function()
    {
        return this._state >= StateDeactivated;
    },
    destroy: function()
    {
        var promise = new AsyncResultBase(this._communicator, "destroy", null, null, this);
        var self = this;
        var destroyInternal = function()
        {
            if(self._state < StateDestroyed)
            {
                self._state = StateDestroyed;
                self._servantManager.destroy();
                self._objectAdapterFactory.removeObjectAdapter(self);
            }
            return promise.succeed(promise);
        };

        return this._state < StateDeactivated ? this.deactivate().then(destroyInternal) : destroyInternal();
    },
    add: function(object, ident)
    {
        return this.addFacet(object, ident, "");
    },
    addFacet: function(object, ident, facet)
    {
        this.checkForDeactivation();
        this.checkIdentity(ident);
        this.checkServant(object);

        //
        // Create a copy of the Identity argument, in case the caller
        // reuses it.
        //
        var id = ident.clone();

        this._servantManager.addServant(object, id, facet);

        return this.newProxy(id, facet);
    },
    addWithUUID: function(object)
    {
        return this.addFacetWithUUID(object, "");
    },
    addFacetWithUUID: function(object, facet)
    {
        return this.addFacet(object, new Identity(Ice.generateUUID(), ""), facet);
    },
    addDefaultServant: function(servant, category)
    {
        this.checkServant(servant);
        this.checkForDeactivation();

        this._servantManager.addDefaultServant(servant, category);
    },
    remove: function(ident)
    {
        return this.removeFacet(ident, "");
    },
    removeFacet: function(ident, facet)
    {
        this.checkForDeactivation();
        this.checkIdentity(ident);

        return this._servantManager.removeServant(ident, facet);
    },
    removeAllFacets: function(ident)
    {
        this.checkForDeactivation();
        this.checkIdentity(ident);

        return this._servantManager.removeAllFacets(ident);
    },
    removeDefaultServant: function(category)
    {
        this.checkForDeactivation();

        return this._servantManager.removeDefaultServant(category);
    },
    find: function(ident)
    {
        return this.findFacet(ident, "");
    },
    findFacet: function(ident, facet)
    {
        this.checkForDeactivation();
        this.checkIdentity(ident);

        return this._servantManager.findServant(ident, facet);
    },
    findAllFacets: function(ident)
    {
        this.checkForDeactivation();
        this.checkIdentity(ident);

        return this._servantManager.findAllFacets(ident);
    },
    findByProxy: function(proxy)
    {
        this.checkForDeactivation();

        var ref = proxy.__reference();
        return this.findFacet(ref.getIdentity(), ref.getFacet());
    },
    findDefaultServant: function(category)
    {
        this.checkForDeactivation();

        return this._servantManager.findDefaultServant(category);
    },
    addServantLocator: function(locator, prefix)
    {
        this.checkForDeactivation();

        this._servantManager.addServantLocator(locator, prefix);
    },
    removeServantLocator: function(prefix)
    {
        this.checkForDeactivation();

        return this._servantManager.removeServantLocator(prefix);
    },
    findServantLocator: function(prefix)
    {
        this.checkForDeactivation();

        return this._servantManager.findServantLocator(prefix);
    },
    createProxy: function(ident)
    {
        this.checkForDeactivation();
        this.checkIdentity(ident);

        return this.newProxy(ident, "");
    },
    createDirectProxy: function(ident)
    {
        return this.createProxy(ident);
    },
    createIndirectProxy: function(ident)
    {
        throw new Ice.FeatureNotSupportedException("setLocator not supported");
    },
    setLocator: function(locator)
    {
        throw new Ice.FeatureNotSupportedException("setLocator not supported");
    },
    refreshPublishedEndpoints: function()
    {
        throw new Ice.FeatureNotSupportedException("refreshPublishedEndpoints not supported");
    },
    getEndpoints: function()
    {
        return [];
    },
    getPublishedEndpoints: function()
    {
        return [];
    },
    getServantManager: function()
    {
        //
        // _servantManager is immutable.
        //
        return this._servantManager;
    },
    messageSizeMax: function()
    {
        return this._messageSizeMax;
    },
    newProxy: function(ident, facet)
    {
        var endpoints = [];

        //
        // Now we also add the endpoints of the router's server proxy, if
        // any. This way, object references created by this object adapter
        // will also point to the router's server proxy endpoints.
        //
        for(var i = 0; i < this._routerEndpoints.length; ++i)
        {
            endpoints.push(this._routerEndpoints[i]);
        }

        //
        // Create a reference and return a proxy for this reference.
        //
        var ref = this._instance.referenceFactory().create(ident, facet, this._reference, endpoints);
        return this._instance.proxyFactory().referenceToProxy(ref);
    },
    checkForDeactivation: function(promise)
    {
        if(this._state >= StateDeactivated)
        {
            var ex = new Ice.ObjectAdapterDeactivatedException();
            ex.name = this.getName();

            if(promise !== undefined)
            {
                promise.fail(ex, promise);
                return true;
            }
            else
            {
                throw ex;
            }
        }

        return false;
    },
    checkIdentity: function(ident)
    {
        if(ident.name === undefined || ident.name === null || ident.name.length === 0)
        {
            throw new Ice.IllegalIdentityException(ident);
        }

        if(ident.category === undefined || ident.category === null)
        {
            ident.category = "";
        }
    },
    checkServant: function(servant)
    {
        if(servant === undefined || servant === null)
        {
            throw new Ice.IllegalServantException("cannot add null servant to Object Adapter");
        }
    },
    filterProperties: function(unknownProps)
    {
        //
        // Do not create unknown properties list if Ice prefix, i.e., Ice, Glacier2, etc.
        //
        var addUnknown = true, i;
        var prefix = this._name + ".";
        for(i = 0; i < PropertyNames.clPropNames.length; ++i)
        {
            if(prefix.indexOf(PropertyNames.clPropNames[i] + ".") === 0)
            {
                addUnknown = false;
                break;
            }
        }

        var noProps = true;
        var props = this._instance.initializationData().properties.getPropertiesForPrefix(prefix);
        for(var e = props.entries; e !== null; e = e.next)
        {
            var valid = false;
            for(i = 0; i < _suffixes.length; ++i)
            {
                if(e.key === prefix + _suffixes[i])
                {
                    noProps = false;
                    valid = true;
                    break;
                }
            }

            if(!valid && addUnknown)
            {
                unknownProps.push(e.key);
            }
        }

        return noProps;
    }
});

Ice.ObjectAdapterI = ObjectAdapterI;
module.exports.Ice = Ice;
