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
        "../Ice/Instance",
        "../Ice/UUID",
        "../Ice/AsyncResultBase"
    ]);

const Instance = Ice.Instance;

//
// Ice.Communicator
//
class Communicator
{
    constructor(initData)
    {
        this._instance = new Instance(initData);
    }

    //
    // Certain initialization tasks need to be completed after the
    // constructor.
    //
    finishSetup(promise)
    {
        this._instance.finishSetup(this, promise);
    }

    destroy()
    {
        return this._instance.destroy();
    }

    shutdown()
    {
        this._instance.objectAdapterFactory().shutdown();
    }

    waitForShutdown()
    {
        return this._instance.objectAdapterFactory().waitForShutdown();
    }

    isShutdown()
    {
        return this._instance.objectAdapterFactory().isShutdown();
    }

    stringToProxy(s)
    {
        return this._instance.proxyFactory().stringToProxy(s);
    }

    proxyToString(proxy)
    {
        return this._instance.proxyFactory().proxyToString(proxy);
    }

    propertyToProxy(s)
    {
        return this._instance.proxyFactory().propertyToProxy(s);
    }

    proxyToProperty(proxy, prefix)
    {
        return this._instance.proxyFactory().proxyToProperty(proxy, prefix);
    }

    stringToIdentity(s)
    {
        return Ice.stringToIdentity(s);
    }

    identityToString(ident)
    {
        return Ice.identityToString(ident, this._instance.toStringMode());
    }

    createObjectAdapter(name)
    {
        const promise = new Ice.AsyncResultBase(this, "createObjectAdapter", this, null, null);
        this._instance.objectAdapterFactory().createObjectAdapter(name, null, promise);
        return promise;
    }

    createObjectAdapterWithEndpoints(name, endpoints)
    {
        if(name.length === 0)
        {
            name = Ice.generateUUID();
        }

        this.getProperties().setProperty(name + ".Endpoints", endpoints);
        const promise = new Ice.AsyncResultBase(this, "createObjectAdapterWithEndpoints", this, null, null);
        this._instance.objectAdapterFactory().createObjectAdapter(name, null, promise);
        return promise;
    }

    createObjectAdapterWithRouter(name, router)
    {
        if(name.length === 0)
        {
            name = Ice.generateUUID();
        }

        const promise = new Ice.AsyncResultBase(this, "createObjectAdapterWithRouter", this, null, null);

        //
        // We set the proxy properties here, although we still use the proxy supplied.
        //
        this.proxyToProperty(router, name + ".Router").forEach((value, key) =>
            {
                this.getProperties().setProperty(key, value);
            });

        this._instance.objectAdapterFactory().createObjectAdapter(name, router, promise);
        return promise;
    }

    addObjectFactory(factory, id)
    {
        this._instance.addObjectFactory(factory, id);
    }

    findObjectFactory(id)
    {
        return this._instance.findObjectFactory(id);
    }

    getValueFactoryManager()
    {
        return this._instance.initializationData().valueFactoryManager;
    }

    getImplicitContext()
    {
        return this._instance.getImplicitContext();
    }

    getProperties()
    {
        return this._instance.initializationData().properties;
    }

    getLogger()
    {
        return this._instance.initializationData().logger;
    }

    getDefaultRouter()
    {
        return this._instance.referenceFactory().getDefaultRouter();
    }

    setDefaultRouter(router)
    {
        this._instance.setDefaultRouter(router);
    }

    getDefaultLocator()
    {
        return this._instance.referenceFactory().getDefaultLocator();
    }

    setDefaultLocator(locator)
    {
        this._instance.setDefaultLocator(locator);
    }
    
    flushBatchRequests()
    {
        return this._instance.outgoingConnectionFactory().flushAsyncBatchRequests();
    }

    get instance()
    {
        return this._instance;
    }
}

Ice.Communicator = Communicator;
module.exports.Ice = Ice;
