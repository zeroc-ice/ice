//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { CommunicatorDestroyedException } from "./LocalExceptions.js";
import { generateUUID } from "./UUID.js";
import { identityToString } from "./IdentityToString.js";
import { Promise } from "./Promise.js";
import { Debug } from "./Debug.js";
import { ObjectPrx } from "./ObjectPrx.js";

//
// Ice.Communicator
//
export class Communicator {
    constructor(initData) {
        this._instance = this.createInstance(initData);
    }

    //
    // Certain initialization tasks need to be completed after the constructor.
    //
    finishSetup(promise) {
        this._instance.finishSetup(this, promise);
    }

    destroy() {
        return this._instance.destroy();
    }

    shutdown() {
        try {
            return this._instance.objectAdapterFactory().shutdown();
        } catch (ex) {
            Debug.assert(ex instanceof CommunicatorDestroyedException);
            return Promise.resolve();
        }
    }

    waitForShutdown() {
        try {
            return this._instance.objectAdapterFactory().waitForShutdown();
        } catch (ex) {
            Debug.assert(ex instanceof CommunicatorDestroyedException);
            return Promise.resolve();
        }
    }

    isShutdown() {
        try {
            return this._instance.objectAdapterFactory().isShutdown();
        } catch (ex) {
            if (!(ex instanceof CommunicatorDestroyedException)) {
                throw ex;
            }
            return true;
        }
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
        const promise = this.createAsyncResultBase(this, "createObjectAdapter", this, null, null);
        this._instance.objectAdapterFactory().createObjectAdapter(name, null, promise);
        return promise;
    }

    createObjectAdapterWithEndpoints(name, endpoints) {
        if (name.length === 0) {
            name = generateUUID();
        }

        this.getProperties().setProperty(name + ".Endpoints", endpoints);
        const promise = this.createAsyncResultBase(this, "createObjectAdapterWithEndpoints", this, null, null);
        this._instance.objectAdapterFactory().createObjectAdapter(name, null, promise);
        return promise;
    }

    createObjectAdapterWithRouter(name, router) {
        if (name.length === 0) {
            name = generateUUID();
        }

        const promise = this.createAsyncResultBase(this, "createObjectAdapterWithRouter", this, null, null);

        //
        // We set the proxy properties here, although we still use the proxy supplied.
        //
        this.proxyToProperty(router, name + ".Router").forEach((value, key) => {
            this.getProperties().setProperty(key, value);
        });

        this._instance.objectAdapterFactory().createObjectAdapter(name, router, promise);
        return promise;
    }

    getValueFactoryManager() {
        return this._instance.initializationData().valueFactoryManager;
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
