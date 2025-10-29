// Copyright (c) ZeroC, Inc.

import { CompositeSliceLoader } from "./CompositeSliceLoader.js";
import { InitializationData } from "./InitializationData.js";
import { InitializationException } from "./LocalExceptions.js";
import { ObjectPrx } from "./ObjectPrx.js";
import { Promise } from "./Promise.js";
import { Properties } from "./Properties.js";
import { defaultSliceLoaderInstance } from "./DefaultSliceLoader.js";
import { generateUUID } from "./UUID.js";
import { identityToString } from "./IdentityToString.js";

//
// Ice.Communicator
//
export class Communicator {
    constructor() {
        this._isShutdown = false;
        this._shutdownPromise = new Promise();

        let initData = null;
        if (arguments.length === 0) {
            initData = new InitializationData();
        } else if (arguments.length === 1) {
            const arg0 = arguments[0];
            if (arg0 instanceof Array) {
                initData = new InitializationData();
                initData.properties = new Properties(arg0);
            } else if (arg0 instanceof InitializationData) {
                initData = arg0.clone();
            } else {
                throw new InitializationException("invalid argument provided to the Communicator constructor");
            }
        } else {
            throw new InitializationException("invalid number of arguments provided to the Communicator constructor");
        }

        if (initData.sliceLoader === null || initData.sliceLoader === undefined) {
            initData.sliceLoader = defaultSliceLoaderInstance;
        } else {
            initData.sliceLoader = new CompositeSliceLoader([initData.sliceLoader, defaultSliceLoaderInstance]);
        }

        this._instance = this.createInstance(initData);
        this._instance.finishSetup(this);
    }

    destroy() {
        this.shutdown();
        return this._instance.destroy();
    }

    [Symbol.asyncDispose]() {
        return this.destroy();
    }

    shutdown() {
        if (!this._isShutdown) {
            this._isShutdown = true;
            this._shutdownPromise.resolve();
        }
    }

    waitForShutdown() {
        return this._shutdownPromise;
    }

    isShutdown() {
        return this._isShutdown;
    }

    stringToProxy(str) {
        const reference = this._instance.referenceFactory().createFromString(str, "");
        return reference == null ? null : new ObjectPrx(reference);
    }

    proxyToString(proxy) {
        return proxy == null ? "" : proxy._reference.toString();
    }

    propertyToProxy(property) {
        const proxy = this._instance.initializationData().properties.getProperty(property);
        const reference = this._instance.referenceFactory().createFromString(proxy, property);
        return reference == null ? null : new ObjectPrx(reference);
    }

    proxyToProperty(proxy, prefix) {
        return proxy._reference.toProperty(prefix);
    }

    identityToString(ident) {
        return identityToString(ident, this._instance.toStringMode());
    }

    createObjectAdapter(name) {
        const promise = new Promise();
        this._instance.objectAdapterFactory().createObjectAdapter(name, null, promise);
        return promise;
    }

    createObjectAdapterWithRouter(name, router) {
        if (name.length === 0) {
            name = generateUUID();
        }

        const promise = new Promise();

        //
        // We set the proxy properties here, although we still use the proxy supplied.
        //
        this.proxyToProperty(router, name + ".Router").forEach((value, key) => {
            this.getProperties().setProperty(key, value);
        });

        this._instance.objectAdapterFactory().createObjectAdapter(name, router, promise);
        return promise;
    }

    getDefaultObjectAdapter() {
        return this._instance.outgoingConnectionFactory().getDefaultObjectAdapter();
    }

    setDefaultObjectAdapter(adapter) {
        this._instance.outgoingConnectionFactory().setDefaultObjectAdapter(adapter);
    }

    getImplicitContext() {
        return this._instance.getImplicitContext();
    }

    getProperties() {
        return this._instance.initializationData().properties;
    }

    getLogger() {
        return this._instance.initializationData().logger;
    }

    getDefaultRouter() {
        return this._instance.referenceFactory().getDefaultRouter();
    }

    setDefaultRouter(router) {
        this._instance.setDefaultRouter(router);
    }

    getDefaultLocator() {
        return this._instance.referenceFactory().getDefaultLocator();
    }

    setDefaultLocator(locator) {
        this._instance.setDefaultLocator(locator);
    }

    flushBatchRequests() {
        return this._instance.outgoingConnectionFactory().flushAsyncBatchRequests();
    }

    get instance() {
        return this._instance;
    }
}
