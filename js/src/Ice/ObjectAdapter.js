// Copyright (c) ZeroC, Inc.

import { ArrayUtil } from "./ArrayUtil.js";
import { Ice as Ice_Identity } from "./Identity.js";
const { Identity } = Ice_Identity;
import { generateUUID } from "./UUID.js";
import {
    AlreadyRegisteredException,
    InitializationException,
    ObjectAdapterDestroyedException,
    ParseException,
} from "./LocalExceptions.js";
import { Ice as Ice_Router } from "./Router.js";
const { RouterPrx } = Ice_Router;
import { Promise } from "./Promise.js";
import { PropertyNames } from "./PropertyNames.js";
import { ServantManager } from "./ServantManager.js";
import { StringUtil } from "./StringUtil.js";
import { identityToString } from "./IdentityToString.js";
import { ObjectPrx } from "./ObjectPrx.js";
import { Logger } from "./Logger.js";
import { LoggerMiddleware } from "./LoggerMiddleware.js";
import { Properties } from "./Properties.js";

//
// Only for use by IceInternal.ObjectAdapterFactory
//
export class ObjectAdapter {
    constructor(instance, communicator, objectAdapterFactory, name, router, noConfig, promise) {
        this._instance = instance;
        this._communicator = communicator;
        this._objectAdapterFactory = objectAdapterFactory;
        this._servantManager = new ServantManager(instance, name);
        this._name = name;
        this._publishedEndpoints = [];
        this._routerInfo = null;
        this._isDestroyed = false;
        this._noConfig = noConfig;

        this._dispatchPipeline = null;
        this._middlewareStack = [];

        // Install default middleware depending on the communicator's configuration.
        const logger = instance.initializationData().logger;
        if (logger instanceof Logger) {
            const warningLevel = instance.initializationData().properties.getIcePropertyAsInt("Ice.Warn.Dispatch");
            if (instance.traceLevels().dispatch > 0 || warningLevel > 0) {
                this.use(
                    next =>
                        new LoggerMiddleware(
                            next,
                            logger,
                            instance.traceLevels().dispatch,
                            instance.traceLevels().dispatchCat,
                            warningLevel,
                            instance.toStringMode(),
                        ),
                );
            }
        }

        if (this._noConfig) {
            this._reference = this._instance.referenceFactory().createFromString("dummy -t", "");
            this._messageSizeMax = this._instance.messageSizeMax();
            promise.resolve(this);
            return;
        }

        const properties = this._instance.initializationData().properties;

        Properties.validatePropertiesWithPrefix(this._name, properties, PropertyNames.ObjectAdapterProps);

        //
        // Make sure named adapter has some configuration.
        //
        if (router === null && properties.getPropertiesForPrefix(`${this._name}.`).size === 0) {
            throw new InitializationException(`Object adapter '${this._name}' requires configuration.`);
        }

        //
        // Setup a reference to be used to get the default proxy options
        // when creating new proxies. By default, create twoway proxies.
        //
        const proxyOptions = properties.getPropertyWithDefault(this._name + ".ProxyOptions", "-t");
        try {
            this._reference = this._instance.referenceFactory().createFromString("dummy " + proxyOptions, "");
        } catch (ex) {
            if (ex instanceof ParseException) {
                throw new InitializationException(
                    `invalid proxy options '${proxyOptions}' for object adapter '${name}'`,
                    { cause: ex },
                );
            } else {
                throw ex;
            }
        }

        // The maximum size of an Ice protocol message in bytes. This is limited to 0x7fffffff, which corresponds to
        // the maximum value of a 32-bit signed integer.
        const messageSizeMaxUpperLimit = 0x7fffffff;
        const defaultMessageSizeMax = this._instance.messageSizeMax() / 1024;
        const messageSizeMax = properties.getPropertyAsIntWithDefault(
            `${this._name}.MessageSizeMax`,
            defaultMessageSizeMax,
        );
        if (messageSizeMax > messageSizeMaxUpperLimit / 1024) {
            throw new InitializationException(
                `${this._name}.MessageSizeMax '${messageSizeMax}' is too large, it must be less than or equal to '${messageSizeMaxUpperLimit / 1024}' KiB`,
            );
        } else if (messageSizeMax < 1) {
            this._messageSizeMax = messageSizeMaxUpperLimit;
        } else {
            // The property is specified in kibibytes (KiB); _messageSizeMax is stored in bytes.
            this._messageSizeMax = messageSizeMax * 1024;
        }

        try {
            if (router === null && properties.getProperty(this._name + ".Router").length > 0) {
                router = new RouterPrx(communicator.propertyToProxy(this._name + ".Router"));
            }
            let p;
            if (router !== null) {
                this._routerInfo = this._instance.routerManager().find(router);
                DEV: console.assert(this._routerInfo !== null);

                //
                // Make sure this router is not already registered with another adapter.
                //
                if (this._routerInfo.getAdapter() !== null) {
                    throw new AlreadyRegisteredException(
                        "object adapter with router",
                        identityToString(router.ice_getIdentity(), this._instance.toStringMode()),
                    );
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
            } else {
                p = Promise.resolve();
            }

            p.then(() => this.computePublishedEndpoints()).then(
                endpoints => {
                    this._publishedEndpoints = endpoints;
                    promise.resolve(this);
                },
                ex => {
                    this.destroy();
                    promise.reject(ex);
                },
            );
        } catch (ex) {
            this.destroy();
            throw ex;
        }
    }

    getName() {
        return this._noConfig ? "" : this._name;
    }

    getCommunicator() {
        return this._communicator;
    }

    destroy() {
        if (!this._isDestroyed) {
            this._isDestroyed = true;

            if (this._routerInfo !== null) {
                // Remove entry from the router manager.
                this._instance.routerManager().erase(this._routerInfo.getRouter());

                // Clear this object adapter with the router.
                this._routerInfo.setAdapter(null);
            }
            this._instance.outgoingConnectionFactory().removeAdapter(this);

            this._servantManager.destroy();
            this._objectAdapterFactory.removeObjectAdapter(this);
            this._publishedEndpoints = [];
        }
    }

    use(middleware) {
        if (this._dispatchPipeline !== null) {
            throw new Error("All middleware must be installed before the first dispatch.");
        }
        this._middlewareStack.push(middleware);
        return this;
    }

    add(object, ident) {
        return this.addFacet(object, ident, "");
    }

    addFacet(object, ident, facet) {
        this.checkForDestruction();
        ObjectAdapter.checkIdentity(ident);
        ObjectAdapter.checkServant(object);

        // Create a copy of the Identity argument, in case the caller reuses it.
        const id = ident.clone();

        this._servantManager.addServant(object, id, facet);

        return this.newProxy(id, facet);
    }

    addWithUUID(object) {
        return this.addFacetWithUUID(object, "");
    }

    addFacetWithUUID(object, facet) {
        return this.addFacet(object, new Identity(generateUUID(), ""), facet);
    }

    addDefaultServant(servant, category) {
        ObjectAdapter.checkServant(servant);
        this.checkForDestruction();

        this._servantManager.addDefaultServant(servant, category);
    }

    remove(ident) {
        return this.removeFacet(ident, "");
    }

    removeFacet(ident, facet) {
        this.checkForDestruction();
        ObjectAdapter.checkIdentity(ident);

        return this._servantManager.removeServant(ident, facet);
    }

    removeAllFacets(ident) {
        this.checkForDestruction();
        ObjectAdapter.checkIdentity(ident);

        return this._servantManager.removeAllFacets(ident);
    }

    removeDefaultServant(category) {
        this.checkForDestruction();

        return this._servantManager.removeDefaultServant(category);
    }

    find(ident) {
        return this.findFacet(ident, "");
    }

    findFacet(ident, facet) {
        this.checkForDestruction();
        ObjectAdapter.checkIdentity(ident);
        return this._servantManager.findServant(ident, facet);
    }

    findAllFacets(ident) {
        this.checkForDestruction();
        ObjectAdapter.checkIdentity(ident);
        return this._servantManager.findAllFacets(ident);
    }

    findByProxy(proxy) {
        this.checkForDestruction();
        const ref = proxy._getReference();
        return this.findFacet(ref.getIdentity(), ref.getFacet());
    }

    findDefaultServant(category) {
        this.checkForDestruction();
        return this._servantManager.findDefaultServant(category);
    }

    get dispatchPipeline() {
        if (this._dispatchPipeline === null) {
            let dispatchPipeline = this._servantManager; // the "final" dispatcher
            while (this._middlewareStack.length > 0) {
                const middleware = this._middlewareStack.pop();
                dispatchPipeline = middleware(dispatchPipeline);
            }
            this._dispatchPipeline = dispatchPipeline;
        }
        return this._dispatchPipeline;
    }

    addServantLocator(locator, prefix) {
        this.checkForDestruction();
        this._servantManager.addServantLocator(locator, prefix);
    }

    removeServantLocator(prefix) {
        this.checkForDestruction();
        return this._servantManager.removeServantLocator(prefix);
    }

    findServantLocator(prefix) {
        this.checkForDestruction();
        return this._servantManager.findServantLocator(prefix);
    }

    createProxy(ident) {
        this.checkForDestruction();
        ObjectAdapter.checkIdentity(ident);
        return this.newProxy(ident, "");
    }

    createDirectProxy(ident) {
        return this.createProxy(ident);
    }

    getEndpoints() {
        return [];
    }

    getPublishedEndpoints() {
        return ArrayUtil.clone(this._publishedEndpoints);
    }

    setPublishedEndpoints(newEndpoints) {
        this.checkForDestruction();
        if (this._routerInfo !== null) {
            throw new Error("can't set published endpoints on object adapter associated with a router");
        }
        this._publishedEndpoints = ArrayUtil.clone(newEndpoints);
    }

    getServantManager() {
        return this._servantManager;
    }

    messageSizeMax() {
        return this._messageSizeMax;
    }

    newProxy(ident, facet) {
        //
        // Now we also add the endpoints of the router's server proxy, if
        // any. This way, object references created by this object adapter
        // will also point to the router's server proxy endpoints.
        //
        //
        // Create a reference and return a proxy for this reference.
        //
        const reference = this._instance
            .referenceFactory()
            .create(ident, facet, this._reference, this._publishedEndpoints);
        return new ObjectPrx(reference);
    }

    checkForDestruction() {
        if (this._isDestroyed) {
            throw new ObjectAdapterDestroyedException(this.getName());
        }
    }

    static checkIdentity(ident) {
        if (ident.name === undefined || ident.name === null || ident.name.length === 0) {
            throw new TypeError("The name of an Ice object identity cannot be empty.");
        }

        if (ident.category === undefined || ident.category === null) {
            ident.category = "";
        }
    }

    static checkServant(servant) {
        if (servant === undefined || servant === null) {
            throw new TypeError("cannot add null servant to Object Adapter");
        }
    }

    async computePublishedEndpoints() {
        let endpoints = [];
        if (this._routerInfo !== null) {
            endpoints = await this._routerInfo.getServerEndpoints();
        } else {
            // Parse published endpoints. If set, these are used in proxies instead of the connection factory
            // endpoints.
            const s = this._instance.initializationData().properties.getProperty(this._name + ".PublishedEndpoints");
            const delim = " \t\n\r";

            let end = 0;
            let beg;
            while (end < s.length) {
                beg = StringUtil.findFirstNotOf(s, delim, end);
                if (beg === -1) {
                    if (s != "") {
                        throw new ParseException("invalid empty object adapter endpoint");
                    }
                    break;
                }

                end = beg;
                while (true) {
                    end = s.indexOf(":", end);
                    if (end == -1) {
                        end = s.length;
                        break;
                    } else {
                        let quoted = false;
                        let quote = beg;
                        while (true) {
                            quote = s.indexOf('"', quote);
                            if (quote == -1 || end < quote) {
                                break;
                            } else {
                                quote = s.indexOf('"', ++quote);
                                if (quote == -1) {
                                    break;
                                } else if (end < quote) {
                                    quoted = true;
                                    break;
                                }
                                ++quote;
                            }
                        }
                        if (!quoted) {
                            break;
                        }
                        ++end;
                    }
                }

                const endpointString = s.substring(beg, end);
                const endpoint = this._instance.endpointFactoryManager().create(endpointString, false);
                if (endpoint === null) {
                    throw new ParseException(`invalid object adapter endpoint '${s}'`);
                }
                endpoints.push(endpoint);
            }
        }

        if (this._instance.traceLevels().network >= 1 && endpoints.length > 0) {
            let msg = `published endpoints for object adapter '${this._name}':\n`;
            msg += endpoints.map(endpoint => endpoint.toString()).join(":");
            this._instance.initializationData().logger.trace(this._instance.traceLevels().networkCat, msg);
        }
        return endpoints;
    }
}
