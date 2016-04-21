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
        "../Ice/DefaultsAndOverrides",
        "../Ice/EndpointFactoryManager",
        "../Ice/HashMap",
        "../Ice/ImplicitContextI",
        "../Ice/IdentityUtil",
        "../Ice/LocatorManager",
        "../Ice/Logger",
        "../Ice/ObjectAdapterFactory",
        "../Ice/ValueFactoryManagerI",
        "../Ice/OutgoingConnectionFactory",
        "../Ice/Promise",
        "../Ice/Properties",
        "../Ice/ProtocolInstance",
        "../Ice/ProxyFactory",
        "../Ice/RetryQueue",
        "../Ice/RouterManager",
        "../Ice/Timer",
        "../Ice/TraceLevels",
        "../Ice/TcpEndpointFactory",
        "../Ice/WSEndpointFactory",
        "../Ice/Reference",
        "../Ice/RequestHandlerFactory",
        "../Ice/LocalException",
        "../Ice/Exception",
        "../Ice/ProcessLogger",
        "../Ice/ACM"
    ]);

var IceSSL = Ice.__M.require(module, ["../Ice/EndpointInfo"]).IceSSL;

var AsyncResultBase = Ice.AsyncResultBase;
var Debug = Ice.Debug;
var DefaultsAndOverrides = Ice.DefaultsAndOverrides;
var EndpointFactoryManager = Ice.EndpointFactoryManager;
var HashMap = Ice.HashMap;
var ImplicitContextI = Ice.ImplicitContextI;
var LocatorManager = Ice.LocatorManager;
var Logger = Ice.Logger;
var ObjectAdapterFactory = Ice.ObjectAdapterFactory;
var ValueFactoryManagerI = Ice.ValueFactoryManagerI;
var OutgoingConnectionFactory = Ice.OutgoingConnectionFactory;
var Promise = Ice.Promise;
var Properties = Ice.Properties;
var ProxyFactory = Ice.ProxyFactory;
var RetryQueue = Ice.RetryQueue;
var RouterManager = Ice.RouterManager;
var Timer = Ice.Timer;
var TraceLevels = Ice.TraceLevels;
var ReferenceFactory = Ice.ReferenceFactory;
var RequestHandlerFactory = Ice.RequestHandlerFactory;
var ACMConfig = Ice.ACMConfig;

var StateActive = 0;
var StateDestroyInProgress = 1;
var StateDestroyed = 2;

//
// Instance - only for use by Communicator
//
var Instance = Ice.Class({
    __init__: function(initData)
    {
        this._state = StateActive;
        this._initData = initData;

        this._traceLevels = null;
        this._defaultsAndOverrides = null;
        this._messageSizeMax = 0;
        this._batchAutoFlushSize = 0;
        this._clientACM = null;
        this._implicitContext = null;
        this._routerManager = null;
        this._locatorManager = null;
        this._referenceFactory = null;
        this._requestHandlerFactory = null;
        this._proxyFactory = null;
        this._outgoingConnectionFactory = null;
        this._objectAdapterFactory = null;
        this._retryQueue = null;
        this._endpointHostResolver = null;
        this._endpointFactoryManager = null;
        this._objectFactoryMap = null;
    },
    initializationData: function()
    {
        //
        // No check for destruction. It must be possible to access the
        // initialization data after destruction.
        //
        // This value is immutable.
        //
        return this._initData;
    },
    traceLevels: function()
    {
        // This value is immutable.
        Debug.assert(this._traceLevels !== null);
        return this._traceLevels;
    },
    defaultsAndOverrides: function()
    {
        // This value is immutable.
        Debug.assert(this._defaultsAndOverrides !== null);
        return this._defaultsAndOverrides;
    },
    routerManager: function()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._routerManager !== null);
        return this._routerManager;
    },
    locatorManager: function()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._locatorManager !== null);
        return this._locatorManager;
    },
    referenceFactory: function()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._referenceFactory !== null);
        return this._referenceFactory;
    },
    requestHandlerFactory: function()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._requestHandlerFactory !== null);
        return this._requestHandlerFactory;
    },
    proxyFactory: function()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._proxyFactory !== null);
        return this._proxyFactory;
    },
    outgoingConnectionFactory: function()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._outgoingConnectionFactory !== null);
        return this._outgoingConnectionFactory;
    },
    objectAdapterFactory: function()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._objectAdapterFactory !== null);
        return this._objectAdapterFactory;
    },
    retryQueue: function()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._retryQueue !== null);
        return this._retryQueue;
    },
    timer: function()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._timer !== null);
        return this._timer;
    },
    endpointFactoryManager: function()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._endpointFactoryManager !== null);
        return this._endpointFactoryManager;
    },
    messageSizeMax: function()
    {
        // This value is immutable.
        return this._messageSizeMax;
    },
    batchAutoFlushSize: function()
    {
        // This value is immutable.
        return this._batchAutoFlushSize;
    },
    clientACM: function()
    {
        // This value is immutable.
        return this._clientACM;
    },
    getImplicitContext: function()
    {
        return this._implicitContext;
    },
    stringToIdentity: function(s)
    {
        return Ice.stringToIdentity(s);
    },
    identityToString: function(ident)
    {
        return Ice.identityToString(ident);
    },

    setDefaultLocator: function(locator)
    {
        if(this._state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        this._referenceFactory = this._referenceFactory.setDefaultLocator(locator);
    },
    setDefaultRouter: function(router)
    {
        if(this._state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        this._referenceFactory = this._referenceFactory.setDefaultRouter(router);
    },
    setLogger: function(logger)
    {
        this._initData.logger = logger;
    },
    finishSetup: function(communicator, promise)
    {
        //
        // If promise == null, it means the caller is requesting a synchronous setup.
        // Otherwise, we resolve the promise after all initialization is complete.
        //

        try
        {
            if(this._initData.properties === null)
            {
                this._initData.properties = Properties.createProperties();
            }

            if(Ice.__oneOfDone === undefined)
            {
                Ice.__printStackTraces =
                    this._initData.properties.getPropertyAsIntWithDefault("Ice.PrintStackTraces", 0) > 0;

                Ice.__oneOfDone = true;
            }

            if(this._initData.logger === null)
            {
                this._initData.logger = Ice.getProcessLogger();
            }

            this._traceLevels = new TraceLevels(this._initData.properties);

            this._defaultsAndOverrides = new DefaultsAndOverrides(this._initData.properties, this._initData.logger);

            var defMessageSizeMax = 1024;
            var num = this._initData.properties.getPropertyAsIntWithDefault("Ice.MessageSizeMax", defMessageSizeMax);
            if(num < 1 || num > 0x7fffffff / 1024)
            {
                this._messageSizeMax = 0x7fffffff;
            }
            else
            {
                this._messageSizeMax = num * 1024; // Property is in kilobytes, _messageSizeMax in bytes
            }

            if(this._initData.properties.getProperty("Ice.BatchAutoFlushSize").length === 0 &&
               this._initData.properties.getProperty("Ice.BatchAutoFlush").length > 0)
            {
                if(this._initData.properties.getPropertyAsInt("Ice.BatchAutoFlush") > 0)
                {
                    this._batchAutoFlushSize = this._messageSizeMax;
                }
            }
            else
            {
                num = this._initData.properties.getPropertyAsIntWithDefault("Ice.BatchAutoFlushSize", 1024); // 1MB
                if(num < 1)
                {
                    this._batchAutoFlushSize = num;
                }
                else if(num > 0x7fffffff / 1024)
                {
                    this._batchAutoFlushSize = 0x7fffffff;
                }
                else
                {
                    this._batchAutoFlushSize = num * 1024; // Property is in kilobytes, _batchAutoFlushSize in bytes
                }
            }

            this._clientACM = new ACMConfig(this._initData.properties, this._initData.logger, "Ice.ACM.Client",
                                            new ACMConfig(this._initData.properties, this._initData.logger,
                                                            "Ice.ACM", new ACMConfig()));

            this._implicitContext =
                ImplicitContextI.create(this._initData.properties.getProperty("Ice.ImplicitContext"));

            this._routerManager = new RouterManager();

            this._locatorManager = new LocatorManager(this._initData.properties);

            this._referenceFactory = new ReferenceFactory(this, communicator);

            this._requestHandlerFactory = new RequestHandlerFactory(this, communicator);

            this._proxyFactory = new ProxyFactory(this);

            this._endpointFactoryManager = new EndpointFactoryManager(this);

            var tcpInstance = new Ice.ProtocolInstance(this, Ice.TCPEndpointType, "tcp", false);
            var tcpEndpointFactory = new Ice.TcpEndpointFactory(tcpInstance);
            this._endpointFactoryManager.add(tcpEndpointFactory);

            var wsInstance = new Ice.ProtocolInstance(this, Ice.WSEndpointType, "ws", false);
            var wsEndpointFactory = new Ice.WSEndpointFactory(wsInstance, tcpEndpointFactory.clone(wsInstance));
            this._endpointFactoryManager.add(wsEndpointFactory);

            var sslInstance = new Ice.ProtocolInstance(this, IceSSL.EndpointType, "ssl", true);
            var sslEndpointFactory = new Ice.TcpEndpointFactory(sslInstance);
            this._endpointFactoryManager.add(sslEndpointFactory);

            var wssInstance = new Ice.ProtocolInstance(this, Ice.WSSEndpointType, "wss", true);
            var wssEndpointFactory = new Ice.WSEndpointFactory(wssInstance, sslEndpointFactory.clone(wssInstance));
            this._endpointFactoryManager.add(wssEndpointFactory);

            this._outgoingConnectionFactory = new OutgoingConnectionFactory(communicator, this);

            if(this._initData.valueFactoryManager === null)
            {
                this._initData.valueFactoryManager = new ValueFactoryManagerI();
            }

            this._objectAdapterFactory = new ObjectAdapterFactory(this, communicator);

            this._retryQueue = new RetryQueue(this);
            this._timer = new Timer(this._initData.logger);

            var router = Ice.RouterPrx.uncheckedCast(this._proxyFactory.propertyToProxy("Ice.Default.Router"));
            if(router !== null)
            {
                this._referenceFactory = this._referenceFactory.setDefaultRouter(router);
            }

            var loc = Ice.LocatorPrx.uncheckedCast(this._proxyFactory.propertyToProxy("Ice.Default.Locator"));
            if(loc !== null)
            {
                this._referenceFactory = this._referenceFactory.setDefaultLocator(loc);
            }

            if(promise !== null)
            {
                promise.succeed(communicator);
            }
        }
        catch(ex)
        {
            if(promise !== null)
            {
                if(ex instanceof Ice.LocalException)
                {
                    this.destroy().finally(function()
                                            {
                                                promise.fail(ex);
                                            });
                }
                else
                {
                    promise.fail(ex);
                }
            }
            else
            {
                if(ex instanceof Ice.LocalException)
                {
                    this.destroy();
                }
                throw ex;
            }
        }
    },
    //
    // Only for use by Ice.CommunicatorI
    //
    destroy: function()
    {
        var promise = new AsyncResultBase(null, "destroy", null, this, null);

        //
        // If destroy is in progress, wait for it to be done. This is
        // necessary in case destroy() is called concurrently by
        // multiple threads.
        //
        if(this._state == StateDestroyInProgress)
        {
            if(!this._destroyPromises)
            {
                this._destroyPromises = [];
            }
            this._destroyPromises.push(promise);
            return promise;
        }
        this._state = StateDestroyInProgress;

        //
        // Shutdown and destroy all the incoming and outgoing Ice
        // connections and wait for the connections to be finished.
        //
        var self = this;
        Ice.Promise.try(
            function()
            {
                if(self._objectAdapterFactory)
                {
                    return self._objectAdapterFactory.shutdown();
                }
            }
        ).then(
            function()
            {
                if(self._outgoingConnectionFactory !== null)
                {
                    self._outgoingConnectionFactory.destroy();
                }

                if(self._objectAdapterFactory !== null)
                {
                    return self._objectAdapterFactory.destroy();
                }
            }
        ).then(
            function()
            {
                if(self._outgoingConnectionFactory !== null)
                {
                    return self._outgoingConnectionFactory.waitUntilFinished();
                }
            }
        ).then(
            function()
            {
                if(self._retryQueue)
                {
                    self._retryQueue.destroy();
                }
                if(self._timer)
                {
                    self._timer.destroy();
                }

                if(self._objectFactoryMap !== null)
                {
                    self._objectFactoryMap.forEach(function(arg, id, factory)
                        {
                            factory.destroy();
                        });
                    self._objectFactoryMap.clear();
                }

                if(self._routerManager)
                {
                    self._routerManager.destroy();
                }
                if(self._locatorManager)
                {
                    self._locatorManager.destroy();
                }
                if(self._endpointFactoryManager)
                {
                    self._endpointFactoryManager.destroy();
                }

                var i;
                if(self._initData.properties.getPropertyAsInt("Ice.Warn.UnusedProperties") > 0)
                {
                    var unusedProperties = self._initData.properties.getUnusedProperties();
                    if(unusedProperties.length > 0)
                    {
                        var message = [];
                        message.push("The following properties were set but never read:");
                        for(i = 0; i < unusedProperties.length; ++i)
                        {
                            message.push("\n    ");
                            message.push(unusedProperties[i]);
                        }
                        self._initData.logger.warning(message.join(""));
                    }
                }

                self._objectAdapterFactory = null;
                self._outgoingConnectionFactory = null;
                self._retryQueue = null;
                self._timer = null;

                self._referenceFactory = null;
                self._requestHandlerFactory = null;
                self._proxyFactory = null;
                self._routerManager = null;
                self._locatorManager = null;
                self._endpointFactoryManager = null;

                self._state = StateDestroyed;

                if(this._destroyPromises)
                {
                    for(i = 0; i < this._destroyPromises.length; ++i)
                    {
                        this._destroyPromises[i].succeed(this._destroyPromises[i]);
                    }
                }
                promise.succeed(promise);
            }
        ).exception(
            function(ex)
            {
                if(this._destroyPromises)
                {
                    for(var i = 0; i < this._destroyPromises.length; ++i)
                    {
                        this._destroyPromises[i].fail(ex, this._destroyPromises[i]);
                    }
                }
                promise.fail(ex, promise);
            }
        );
        return promise;
    },
    addObjectFactory: function(factory, id)
    {
        //
        // Create a ValueFactory wrapper around the given ObjectFactory and register the wrapper
        // with the value factory manager. This may raise AlreadyRegisteredException.
        //
        this._initData.valueFactoryManager.add(function(typeId)
            {
                return factory.create(typeId);
            }, id);

        if(this._objectFactoryMap === null)
        {
            this._objectFactoryMap = new HashMap();
        }

        this._objectFactoryMap.set(id, factory);
    },
    findObjectFactory: function(id)
    {
        var factory = null;
        if(this._objectFactoryMap !== null)
        {
            factory = this._objectFactoryMap.get(id);
        }
        return factory !== undefined ? factory : null;
    }
});

Ice.Instance = Instance;
module.exports.Ice = Ice;
