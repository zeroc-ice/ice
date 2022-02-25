//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/ACM");
require("../Ice/AsyncResultBase");
require("../Ice/Debug");
require("../Ice/DefaultsAndOverrides");
require("../Ice/EndpointFactoryManager");
require("../Ice/EndpointInfo");
require("../Ice/Exception");
require("../Ice/IdentityUtil");
require("../Ice/ImplicitContextI");
require("../Ice/LocalException");
require("../Ice/LocatorManager");
require("../Ice/ObjectAdapterFactory");
require("../Ice/OutgoingConnectionFactory");
require("../Ice/ProcessLogger");
require("../Ice/Promise");
require("../Ice/Properties");
require("../Ice/ProtocolInstance");
require("../Ice/ProxyFactory");
require("../Ice/Reference");
require("../Ice/RequestHandlerFactory");
require("../Ice/RetryQueue");
require("../Ice/RouterManager");
require("../Ice/TcpEndpointFactory");
require("../Ice/Timer");
require("../Ice/ToStringMode");
require("../Ice/TraceLevels");
require("../Ice/ValueFactoryManagerI");
require("../Ice/WSEndpointFactory");

const ACMConfig = Ice.ACMConfig;
const AsyncResultBase = Ice.AsyncResultBase;
const Debug = Ice.Debug;
const DefaultsAndOverrides = Ice.DefaultsAndOverrides;
const EndpointFactoryManager = Ice.EndpointFactoryManager;
const ImplicitContextI = Ice.ImplicitContextI;
const LocatorManager = Ice.LocatorManager;
const ObjectAdapterFactory = Ice.ObjectAdapterFactory;
const OutgoingConnectionFactory = Ice.OutgoingConnectionFactory;
const Properties = Ice.Properties;
const ProxyFactory = Ice.ProxyFactory;
const ReferenceFactory = Ice.ReferenceFactory;
const RequestHandlerFactory = Ice.RequestHandlerFactory;
const RetryQueue = Ice.RetryQueue;
const RouterManager = Ice.RouterManager;
const Timer = Ice.Timer;
const TraceLevels = Ice.TraceLevels;
const ValueFactoryManagerI = Ice.ValueFactoryManagerI;

const StateActive = 0;
const StateDestroyInProgress = 1;
const StateDestroyed = 2;

//
// Instance - only for use by Communicator
//
class Instance
{
    constructor(initData)
    {
        this._state = StateActive;
        this._initData = initData;

        this._traceLevels = null;
        this._defaultsAndOverrides = null;
        this._messageSizeMax = 0;
        this._batchAutoFlushSize = 0;
        this._clientACM = null;
        this._toStringMode = Ice.ToStringMode.Unicode;
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
    }

    initializationData()
    {
        //
        // No check for destruction. It must be possible to access the
        // initialization data after destruction.
        //
        // This value is immutable.
        //
        return this._initData;
    }

    traceLevels()
    {
        // This value is immutable.
        Debug.assert(this._traceLevels !== null);
        return this._traceLevels;
    }

    defaultsAndOverrides()
    {
        // This value is immutable.
        Debug.assert(this._defaultsAndOverrides !== null);
        return this._defaultsAndOverrides;
    }

    routerManager()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._routerManager !== null);
        return this._routerManager;
    }

    locatorManager()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._locatorManager !== null);
        return this._locatorManager;
    }

    referenceFactory()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._referenceFactory !== null);
        return this._referenceFactory;
    }

    requestHandlerFactory()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._requestHandlerFactory !== null);
        return this._requestHandlerFactory;
    }

    proxyFactory()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._proxyFactory !== null);
        return this._proxyFactory;
    }

    outgoingConnectionFactory()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._outgoingConnectionFactory !== null);
        return this._outgoingConnectionFactory;
    }

    objectAdapterFactory()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._objectAdapterFactory !== null);
        return this._objectAdapterFactory;
    }

    retryQueue()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._retryQueue !== null);
        return this._retryQueue;
    }

    timer()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._timer !== null);
        return this._timer;
    }

    endpointFactoryManager()
    {
        if(this._state === StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Debug.assert(this._endpointFactoryManager !== null);
        return this._endpointFactoryManager;
    }

    messageSizeMax()
    {
        // This value is immutable.
        return this._messageSizeMax;
    }

    batchAutoFlushSize()
    {
        // This value is immutable.
        return this._batchAutoFlushSize;
    }

    clientACM()
    {
        // This value is immutable.
        return this._clientACM;
    }

    toStringMode()
    {
        // this value is immutable
        return this._toStringMode;
    }

    getImplicitContext()
    {
        return this._implicitContext;
    }

    setDefaultLocator(locator)
    {
        if(this._state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        this._referenceFactory = this._referenceFactory.setDefaultLocator(locator);
    }

    setDefaultRouter(router)
    {
        if(this._state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        this._referenceFactory = this._referenceFactory.setDefaultRouter(router);
    }

    setLogger(logger)
    {
        this._initData.logger = logger;
    }

    finishSetup(communicator, promise)
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

            if(Ice._oneOfDone === undefined)
            {
                Ice._printStackTraces =
                    this._initData.properties.getPropertyAsIntWithDefault("Ice.PrintStackTraces", 0) > 0;

                Ice._oneOfDone = true;
            }

            if(this._initData.logger === null)
            {
                this._initData.logger = Ice.getProcessLogger();
            }

            this._traceLevels = new TraceLevels(this._initData.properties);

            this._defaultsAndOverrides = new DefaultsAndOverrides(this._initData.properties, this._initData.logger);

            const defMessageSizeMax = 1024;
            let num = this._initData.properties.getPropertyAsIntWithDefault("Ice.MessageSizeMax", defMessageSizeMax);
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

            const toStringModeStr = this._initData.properties.getPropertyWithDefault("Ice.ToStringMode", "Unicode");
            if(toStringModeStr === "ASCII")
            {
                this._toStringMode = Ice.ToStringMode.ASCII;
            }
            else if(toStringModeStr === "Compat")
            {
                this._toStringMode = Ice.ToStringMode.Compat;
            }
            else if(toStringModeStr !== "Unicode")
            {
                throw new Ice.InitializationException("The value for Ice.ToStringMode must be Unicode, ASCII or Compat");
            }

            this._implicitContext =
                ImplicitContextI.create(this._initData.properties.getProperty("Ice.ImplicitContext"));

            this._routerManager = new RouterManager();

            this._locatorManager = new LocatorManager(this._initData.properties);

            this._referenceFactory = new ReferenceFactory(this, communicator);

            this._requestHandlerFactory = new RequestHandlerFactory(this, communicator);

            this._proxyFactory = new ProxyFactory(this);

            this._endpointFactoryManager = new EndpointFactoryManager(this);

            const tcpInstance = new Ice.ProtocolInstance(this, Ice.TCPEndpointType, "tcp", false);
            const tcpEndpointFactory = new Ice.TcpEndpointFactory(tcpInstance);
            this._endpointFactoryManager.add(tcpEndpointFactory);

            const wsInstance = new Ice.ProtocolInstance(this, Ice.WSEndpointType, "ws", false);
            const wsEndpointFactory = new Ice.WSEndpointFactory(wsInstance, tcpEndpointFactory.clone(wsInstance));
            this._endpointFactoryManager.add(wsEndpointFactory);

            const sslInstance = new Ice.ProtocolInstance(this, Ice.SSLEndpointType, "ssl", true);
            const sslEndpointFactory = new Ice.TcpEndpointFactory(sslInstance);
            this._endpointFactoryManager.add(sslEndpointFactory);

            const wssInstance = new Ice.ProtocolInstance(this, Ice.WSSEndpointType, "wss", true);
            const wssEndpointFactory = new Ice.WSEndpointFactory(wssInstance, sslEndpointFactory.clone(wssInstance));
            this._endpointFactoryManager.add(wssEndpointFactory);

            this._outgoingConnectionFactory = new OutgoingConnectionFactory(communicator, this);

            if(this._initData.valueFactoryManager === null)
            {
                this._initData.valueFactoryManager = new ValueFactoryManagerI();
            }

            this._objectAdapterFactory = new ObjectAdapterFactory(this, communicator);

            this._retryQueue = new RetryQueue(this);
            this._timer = new Timer(this._initData.logger);

            const router = Ice.RouterPrx.uncheckedCast(this._proxyFactory.propertyToProxy("Ice.Default.Router"));
            if(router !== null)
            {
                this._referenceFactory = this._referenceFactory.setDefaultRouter(router);
            }

            const loc = Ice.LocatorPrx.uncheckedCast(this._proxyFactory.propertyToProxy("Ice.Default.Locator"));
            if(loc !== null)
            {
                this._referenceFactory = this._referenceFactory.setDefaultLocator(loc);
            }

            if(promise !== null)
            {
                promise.resolve(communicator);
            }
        }
        catch(ex)
        {
            if(promise !== null)
            {
                if(ex instanceof Ice.LocalException)
                {
                    this.destroy().finally(() => promise.reject(ex));
                }
                else
                {
                    promise.reject(ex);
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
    }

    //
    // Only for use by Ice.CommunicatorI
    //
    destroy()
    {
        const promise = new AsyncResultBase(null, "destroy", null, this, null);

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
        Ice.Promise.try(() =>
            {
                if(this._objectAdapterFactory)
                {
                    return this._objectAdapterFactory.shutdown();
                }
            }
        ).then(() =>
            {
                if(this._outgoingConnectionFactory !== null)
                {
                    this._outgoingConnectionFactory.destroy();
                }

                if(this._objectAdapterFactory !== null)
                {
                    return this._objectAdapterFactory.destroy();
                }
            }
        ).then(() =>
            {
                if(this._outgoingConnectionFactory !== null)
                {
                    return this._outgoingConnectionFactory.waitUntilFinished();
                }
            }
        ).then(() =>
            {
                if(this._retryQueue)
                {
                    this._retryQueue.destroy();
                }
                if(this._timer)
                {
                    this._timer.destroy();
                }

                if(this._objectFactoryMap !== null)
                {
                    this._objectFactoryMap.forEach(factory => factory.destroy());
                    this._objectFactoryMap.clear();
                }

                if(this._routerManager)
                {
                    this._routerManager.destroy();
                }
                if(this._locatorManager)
                {
                    this._locatorManager.destroy();
                }
                if(this._endpointFactoryManager)
                {
                    this._endpointFactoryManager.destroy();
                }

                if(this._initData.properties.getPropertyAsInt("Ice.Warn.UnusedProperties") > 0)
                {
                    const unusedProperties = this._initData.properties.getUnusedProperties();
                    if(unusedProperties.length > 0)
                    {
                        const message = [];
                        message.push("The following properties were set but never read:");
                        unusedProperties.forEach(p => message.push("\n    ", p));
                        this._initData.logger.warning(message.join(""));
                    }
                }

                this._objectAdapterFactory = null;
                this._outgoingConnectionFactory = null;
                this._retryQueue = null;
                this._timer = null;

                this._referenceFactory = null;
                this._requestHandlerFactory = null;
                this._proxyFactory = null;
                this._routerManager = null;
                this._locatorManager = null;
                this._endpointFactoryManager = null;

                this._state = StateDestroyed;

                if(this._destroyPromises)
                {
                    this._destroyPromises.forEach(p => p.resolve());
                }
                promise.resolve();
            }
        ).catch(ex =>
            {
                if(this._destroyPromises)
                {
                    this._destroyPromises.forEach(p => p.reject(ex));
                }
                promise.reject(ex);
            });
        return promise;
    }

    addObjectFactory(factory, id)
    {
        //
        // Create a ValueFactory wrapper around the given ObjectFactory and register the wrapper
        // with the value factory manager. This may raise AlreadyRegisteredException.
        //
        this._initData.valueFactoryManager.add(typeId => factory.create(typeId), id);

        if(this._objectFactoryMap === null)
        {
            this._objectFactoryMap = new Map();
        }

        this._objectFactoryMap.set(id, factory);
    }

    findObjectFactory(id)
    {
        let factory = null;
        if(this._objectFactoryMap !== null)
        {
            factory = this._objectFactoryMap.get(id);
        }
        return factory !== undefined ? factory : null;
    }
}

Ice.Instance = Instance;
module.exports.Ice = Ice;
