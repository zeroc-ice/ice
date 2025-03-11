// Copyright (c) ZeroC, Inc.

import { ArrayUtil } from "./ArrayUtil.js";
import { BatchRequestQueue } from "./BatchRequestQueue.js";
import { ConnectionRequestHandler } from "./ConnectionRequestHandler.js";
import { EndpointSelectionType } from "./EndpointSelectionType.js";
import { HashUtil } from "./HashUtil.js";
import { Ice as Ice_Identity } from "./Identity.js";
const { Identity } = Ice_Identity;
import { identityToString } from "./IdentityToString.js";
import { MapUtil } from "./MapUtil.js";
import { OpaqueEndpointI } from "./OpaqueEndpoint.js";
import { ReferenceMode } from "./ReferenceMode.js";
import { StringUtil } from "./StringUtil.js";
import {
    Encoding_1_0,
    Encoding_1_1,
    Protocol_1_0,
    encodingVersionToString,
    protocolVersionToString,
} from "./Protocol.js";
import { FixedProxyException, NoEndpointException } from "./LocalExceptions.js";
import { Promise } from "./Promise.js";
import { ConnectRequestHandler } from "./ConnectRequestHandler.js";

const _emptyContext = new Map();
const _emptyEndpoints = [];

export class Reference {
    constructor(instance, communicator, identity, facet, mode, secure, protocol, encoding, invocationTimeout, context) {
        //
        // Validate string arguments.
        //
        DEV: console.assert(identity === undefined || identity.name !== null);
        DEV: console.assert(identity === undefined || identity.category !== null);
        DEV: console.assert(facet === undefined || facet !== null);

        this._instance = instance;
        this._communicator = communicator;
        this._mode = mode;
        this._secure = secure;
        this._identity = identity;
        this._context = context === undefined || context === null ? _emptyContext : context;
        this._facet = facet;
        this._protocol = protocol;
        this._encoding = encoding;
        this._invocationTimeout = invocationTimeout;
    }

    getMode() {
        return this._mode;
    }

    get isBatch() {
        return this._mode == ReferenceMode.ModeBatchOneway || this._mode == ReferenceMode.ModeBatchDatagram;
    }

    get isTwoway() {
        return this._mode == ReferenceMode.ModeTwoway;
    }

    getSecure() {
        return this._secure;
    }

    getProtocol() {
        return this._protocol;
    }

    getEncoding() {
        return this._encoding;
    }

    getIdentity() {
        return this._identity;
    }

    getFacet() {
        return this._facet;
    }

    getInstance() {
        return this._instance;
    }

    getContext() {
        return this._context; // Map
    }

    getInvocationTimeout() {
        return this._invocationTimeout;
    }

    getCommunicator() {
        return this._communicator;
    }

    getEndpoints() {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    getAdapterId() {
        // Abstract
        DEV: console.assert(false);
        return "";
    }

    getRouterInfo() {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    getLocatorInfo() {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    getCacheConnection() {
        // Abstract
        DEV: console.assert(false);
        return false;
    }

    getPreferSecure() {
        // Abstract
        DEV: console.assert(false);
        return false;
    }

    getEndpointSelection() {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    getLocatorCacheTimeout() {
        // Abstract
        DEV: console.assert(false);
        return 0;
    }

    getConnectionId() {
        // Abstract
        DEV: console.assert(false);
        return "";
    }

    //
    // The change* methods (here and in derived classes) create
    // a new reference based on the existing one, with the
    // corresponding value changed.
    //
    changeContext(newContext) {
        if (newContext === undefined || newContext === null) {
            newContext = _emptyContext;
        }
        const r = this._instance.referenceFactory().copy(this);
        if (newContext.size === 0) {
            r._context = _emptyContext;
        } else {
            r._context = new Map(newContext);
        }
        return r;
    }

    changeMode(newMode) {
        const r = this._instance.referenceFactory().copy(this);
        r._mode = newMode;
        return r;
    }

    changeSecure(newSecure) {
        const r = this._instance.referenceFactory().copy(this);
        r._secure = newSecure;
        return r;
    }

    changeIdentity(newIdentity) {
        const r = this._instance.referenceFactory().copy(this);
        r._identity = new Identity(newIdentity.name, newIdentity.category);
        return r;
    }

    changeFacet(newFacet) {
        const r = this._instance.referenceFactory().copy(this);
        r._facet = newFacet;
        return r;
    }

    changeInvocationTimeout(newInvocationTimeout) {
        const r = this._instance.referenceFactory().copy(this);
        r._invocationTimeout = newInvocationTimeout;
        return r;
    }

    changeEncoding(newEncoding) {
        const r = this._instance.referenceFactory().copy(this);
        r._encoding = newEncoding;
        return r;
    }

    changeAdapterId(newAdapterId) {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    changeEndpoints(newEndpoints) {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    changeLocator(newLocator) {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    changeRouter(newRouter) {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    changeCacheConnection(newCache) {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    changePreferSecure(newPreferSecure) {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    changeEndpointSelection(newType) {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    changeLocatorCacheTimeout(newTimeout) {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    changeConnectionId(connectionId) {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    changeConnection(connection) {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    hashCode() {
        let h = 5381;
        h = HashUtil.addNumber(h, this._mode);
        h = HashUtil.addBoolean(h, this._secure);
        h = HashUtil.addHashable(h, this._identity);
        if (this._context !== null && this._context !== undefined) {
            for (const [key, value] of this._context) {
                h = HashUtil.addString(h, key);
                h = HashUtil.addString(h, value);
            }
        }
        h = HashUtil.addString(h, this._facet);
        h = HashUtil.addHashable(h, this._protocol);
        h = HashUtil.addHashable(h, this._encoding);
        h = HashUtil.addNumber(h, this._invocationTimeout);

        return h;
    }

    //
    // Utility methods
    //
    isFixed() {
        return false;
    }

    isIndirect() {
        // Abstract
        DEV: console.assert(false);
        return false;
    }

    isWellKnown() {
        // Abstract
        DEV: console.assert(false);
        return false;
    }

    //
    // Marshal the reference.
    //
    streamWrite(s) {
        //
        // Don't write the identity here. Operations calling streamWrite
        // write the identity.
        //

        //
        // For compatibility with the old FacetPath.
        //
        if (this._facet.length === 0) {
            s.writeSize(0); // Empty string sequence
        } else {
            s.writeSize(1); // String sequence with one element
            s.writeString(this._facet);
        }

        s.writeByte(this._mode);

        s.writeBool(this._secure);

        if (!s.getEncoding().equals(Encoding_1_0)) {
            this._protocol._write(s);
            this._encoding._write(s);
        }

        // Derived class writes the remainder of the reference.
    }

    //
    // Convert the reference to its string form.
    //
    toString() {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        const s = [];

        const toStringMode = this._instance.toStringMode();

        //
        // If the encoded identity string contains characters which
        // the reference parser uses as separators, then we enclose
        // the identity string in quotes.
        //

        const id = identityToString(this._identity, toStringMode);
        if (id.search(/[ :@]/) != -1) {
            s.push('"');
            s.push(id);
            s.push('"');
        } else {
            s.push(id);
        }

        if (this._facet.length > 0) {
            //
            // If the encoded facet string contains characters which
            // the reference parser uses as separators, then we enclose
            // the facet string in quotes.
            //
            s.push(" -f ");
            const fs = StringUtil.escapeString(this._facet, "", toStringMode);
            if (fs.search(/[ :@]/) != -1) {
                s.push('"');
                s.push(fs);
                s.push('"');
            } else {
                s.push(fs);
            }
        }

        switch (this._mode) {
            case ReferenceMode.ModeTwoway: {
                // Don't print the default mode.
                break;
            }

            case ReferenceMode.ModeOneway: {
                s.push(" -o");
                break;
            }

            case ReferenceMode.ModeBatchOneway: {
                s.push(" -O");
                break;
            }

            case ReferenceMode.ModeDatagram: {
                s.push(" -d");
                break;
            }

            case ReferenceMode.ModeBatchDatagram: {
                s.push(" -D");
                break;
            }

            default: {
                DEV: console.assert(false);
                break;
            }
        }

        if (this._secure) {
            s.push(" -s");
        }

        if (!this._protocol.equals(Protocol_1_0)) {
            // We print the protocol unless it's 1.0.
            s.push(" -p ");
            s.push(protocolVersionToString(this._protocol));
        }

        // We print the encoding if it's not 1.1 or if Ice.Default.EncodingVersion is set to something other than 1.1.
        if (
            !this._encoding.equals(Encoding_1_1) ||
            !this._instance.defaultsAndOverrides().defaultEncoding.equals(Encoding_1_1)
        ) {
            s.push(" -e ");
            s.push(encodingVersionToString(this._encoding));
        }

        return s.join("");

        // Derived class writes the remainder of the string.
    }

    //
    // Convert the reference to its property form.
    //
    toProperty(prefix) {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    getRequestHandler(proxy) {
        // Abstract
        DEV: console.assert(false);
    }

    equals(r) {
        //
        // Note: if(this === r) and type test are performed by each non-abstract derived class.
        //

        if (this._mode !== r._mode) {
            return false;
        }

        if (this._secure !== r._secure) {
            return false;
        }

        if (!this._identity.equals(r._identity)) {
            return false;
        }

        if (!MapUtil.equals(this._context, r._context)) {
            return false;
        }

        if (this._facet !== r._facet) {
            return false;
        }

        if (!this._protocol.equals(r._protocol)) {
            return false;
        }

        if (!this._encoding.equals(r._encoding)) {
            return false;
        }

        if (this._invocationTimeout !== r._invocationTimeout) {
            return false;
        }

        return true;
    }

    clone() {
        // Abstract
        DEV: console.assert(false);
        return null;
    }

    copyMembers(r) {
        //
        // Copy the members that are not passed to the constructor.
        //
        r._context = this._context;
    }
}

export class FixedReference extends Reference {
    constructor(
        instance,
        communicator,
        identity,
        facet,
        mode,
        secure,
        protocol,
        encoding,
        connection,
        invocationTimeout,
        context,
    ) {
        super(instance, communicator, identity, facet, mode, secure, protocol, encoding, invocationTimeout, context);
        this._fixedConnection = connection;
    }

    getEndpoints() {
        return _emptyEndpoints;
    }

    getAdapterId() {
        return "";
    }

    getRouterInfo() {
        return null;
    }

    getLocatorInfo() {
        return null;
    }

    getCacheConnection() {
        return true;
    }

    getPreferSecure() {
        return false;
    }

    getEndpointSelection() {
        return EndpointSelectionType.Random;
    }

    getLocatorCacheTimeout() {
        return 0;
    }

    getConnectionId() {
        return "";
    }

    changeAdapterId(newAdapterId) {
        throw new FixedProxyException();
    }

    changeEndpoints(newEndpoints) {
        throw new FixedProxyException();
    }

    changeLocator(newLocator) {
        throw new FixedProxyException();
    }

    changeRouter(newRouter) {
        throw new FixedProxyException();
    }

    changeCacheConnection(newCache) {
        throw new FixedProxyException();
    }

    changePreferSecure(prefSec) {
        throw new FixedProxyException();
    }

    changeEndpointSelection(newType) {
        throw new FixedProxyException();
    }

    changeLocatorCacheTimeout(newTimeout) {
        throw new FixedProxyException();
    }

    changeConnectionId(connectionId) {
        throw new FixedProxyException();
    }

    changeConnection(newConnection) {
        const r = this.getInstance().referenceFactory().copy(this);
        r._fixedConnection = newConnection;
        return r;
    }

    isFixed() {
        return true;
    }

    isIndirect() {
        return false;
    }

    isWellKnown() {
        return false;
    }

    streamWrite(s) {
        throw new FixedProxyException();
    }

    toProperty(prefix) {
        throw new FixedProxyException();
    }

    clone() {
        const r = new FixedReference(
            this.getInstance(),
            this.getCommunicator(),
            this.getIdentity(),
            this.getFacet(),
            this.getMode(),
            this.getSecure(),
            this.getProtocol(),
            this.getEncoding(),
            this._fixedConnection,
            this.getInvocationTimeout(),
            this.getContext(),
        );
        this.copyMembers(r);
        return r;
    }

    getRequestHandler() {
        switch (this.getMode()) {
            case ReferenceMode.ModeTwoway:
            case ReferenceMode.ModeOneway:
            case ReferenceMode.ModeBatchOneway: {
                if (this._fixedConnection.endpoint().datagram()) {
                    throw new NoEndpointException(this.toString());
                }
                break;
            }

            case ReferenceMode.ModeDatagram:
            case ReferenceMode.ModeBatchDatagram: {
                if (!this._fixedConnection.endpoint().datagram()) {
                    throw new NoEndpointException(this.toString());
                }
                break;
            }

            default: {
                DEV: console.assert(false);
                break;
            }
        }

        //
        // If a secure connection is requested or secure overrides is set,
        // check if the connection is secure.
        //
        const defaultsAndOverrides = this.getInstance().defaultsAndOverrides();
        const secure = defaultsAndOverrides.overrideSecure
            ? defaultsAndOverrides.overrideSecureValue
            : this.getSecure();
        if (secure && !this._fixedConnection.endpoint().secure()) {
            throw new NoEndpointException(this.toString());
        }

        this._fixedConnection.throwException(); // Throw in case our connection is already destroyed.

        // TODO: rename ConnectionRequestHandler to FixedRequestHandler
        return new ConnectionRequestHandler(this, this._fixedConnection);
    }

    get batchRequestQueue() {
        return this._fixedConnection.getBatchRequestQueue();
    }

    equals(rhs) {
        if (this === rhs) {
            return true;
        }
        if (!(rhs instanceof FixedReference)) {
            return false;
        }
        if (!super.equals(rhs)) {
            return false;
        }
        return this._fixedConnection == rhs._fixedConnection;
    }
}

export class RoutableReference extends Reference {
    constructor(
        instance,
        communicator,
        identity,
        facet,
        mode,
        secure,
        protocol,
        encoding,
        endpoints,
        adapterId,
        locatorInfo,
        routerInfo,
        cacheConnection,
        preferSecure,
        endpointSelection,
        locatorCacheTimeout,
        invocationTimeout,
        context,
    ) {
        super(instance, communicator, identity, facet, mode, secure, protocol, encoding, invocationTimeout, context);
        this._endpoints = endpoints;
        this._adapterId = adapterId;
        this._locatorInfo = locatorInfo;
        this._routerInfo = routerInfo;
        this._cacheConnection = cacheConnection;
        this._preferSecure = preferSecure;
        this._endpointSelection = endpointSelection;
        this._locatorCacheTimeout = locatorCacheTimeout;

        if (this._endpoints === null) {
            this._endpoints = _emptyEndpoints;
        }
        if (this._adapterId === null) {
            this._adapterId = "";
        }
        this._connectionId = "";
        this.setBatchRequestQueue();
        DEV: console.assert(this._adapterId.length === 0 || this._endpoints.length === 0);
    }

    getEndpoints() {
        return this._endpoints;
    }

    getAdapterId() {
        return this._adapterId;
    }

    getRouterInfo() {
        return this._routerInfo;
    }

    getLocatorInfo() {
        return this._locatorInfo;
    }

    getCacheConnection() {
        return this._cacheConnection;
    }

    getPreferSecure() {
        return this._preferSecure;
    }

    getEndpointSelection() {
        return this._endpointSelection;
    }

    getLocatorCacheTimeout() {
        return this._locatorCacheTimeout;
    }

    getConnectionId() {
        return this._connectionId;
    }

    changeMode(newMode) {
        const r = super.changeMode(newMode);
        r.setBatchRequestQueue();
        return r;
    }

    changeEncoding(newEncoding) {
        const r = super.changeEncoding(newEncoding);
        if (r !== this) {
            if (r._locatorInfo !== null && !r._locatorInfo.getLocator().ice_getEncodingVersion().equals(newEncoding)) {
                r._locatorInfo = this.getInstance()
                    .locatorManager()
                    .find(r._locatorInfo.getLocator().ice_encodingVersion(newEncoding));
            }
        }
        return r;
    }

    changeAdapterId(newAdapterId) {
        if (this._adapterId === newAdapterId) {
            return this;
        }
        const r = this.getInstance().referenceFactory().copy(this);
        r._adapterId = newAdapterId;
        r._endpoints = _emptyEndpoints;
        return r;
    }

    changeEndpoints(newEndpoints) {
        if (ArrayUtil.equals(newEndpoints, this._endpoints, (e1, e2) => e1.equals(e2))) {
            return this;
        }
        const r = this.getInstance().referenceFactory().copy(this);
        r._endpoints = newEndpoints;
        r._adapterId = "";
        r.applyOverrides(r._endpoints);
        return r;
    }

    changeLocator(newLocator) {
        const newLocatorInfo = this.getInstance().locatorManager().find(newLocator);
        if (newLocatorInfo !== null && this._locatorInfo !== null && newLocatorInfo.equals(this._locatorInfo)) {
            return this;
        }
        const r = this.getInstance().referenceFactory().copy(this);
        r._locatorInfo = newLocatorInfo;
        return r;
    }

    changeRouter(newRouter) {
        const newRouterInfo = this.getInstance().routerManager().find(newRouter);
        if (newRouterInfo !== null && this._routerInfo !== null && newRouterInfo.equals(this._routerInfo)) {
            return this;
        }
        const r = this.getInstance().referenceFactory().copy(this);
        r._routerInfo = newRouterInfo;
        return r;
    }

    changeCacheConnection(newCache) {
        const r = this.getInstance().referenceFactory().copy(this);
        r._cacheConnection = newCache;
        return r;
    }

    changePreferSecure(newPreferSecure) {
        const r = this.getInstance().referenceFactory().copy(this);
        r._preferSecure = newPreferSecure;
        return r;
    }

    changeEndpointSelection(newType) {
        const r = this.getInstance().referenceFactory().copy(this);
        r._endpointSelection = newType;
        return r;
    }

    changeLocatorCacheTimeout(newTimeout) {
        const r = this.getInstance().referenceFactory().copy(this);
        r._locatorCacheTimeout = newTimeout;
        return r;
    }

    changeConnectionId(id) {
        const r = this.getInstance().referenceFactory().copy(this);
        r._connectionId = id;
        r._endpoints = this._endpoints.map(endpoint => endpoint.changeConnectionId(id));
        return r;
    }

    changeConnection(newConnection) {
        return new FixedReference(
            this.getInstance(),
            this.getCommunicator(),
            this.getIdentity(),
            this.getFacet(),
            this.getMode(),
            this.getSecure(),
            this.getProtocol(),
            this.getEncoding(),
            newConnection,
            this.getInvocationTimeout(),
            this.getContext(),
        );
    }

    isIndirect() {
        return this._endpoints.length === 0;
    }

    isWellKnown() {
        return this._endpoints.length === 0 && this._adapterId.length === 0;
    }

    streamWrite(s) {
        super.streamWrite(s);

        s.writeSize(this._endpoints.length);
        if (this._endpoints.length > 0) {
            DEV: console.assert(this._adapterId.length === 0);
            this._endpoints.forEach(endpoint => {
                s.writeShort(endpoint.type());
                endpoint.streamWrite(s);
            });
        } else {
            s.writeString(this._adapterId); // Adapter id.
        }
    }

    toString() {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        const s = [];
        s.push(super.toString());
        if (this._endpoints.length > 0) {
            this._endpoints.forEach(endpoint => {
                const endp = endpoint.toString();
                if (endp !== null && endp.length > 0) {
                    s.push(":");
                    s.push(endp);
                }
            });
        } else if (this._adapterId.length > 0) {
            s.push(" @ ");

            //
            // If the encoded adapter id string contains characters which
            // the reference parser uses as separators, then we enclose
            // the adapter id string in quotes.
            //
            const a = StringUtil.escapeString(this._adapterId, null, this._instance.toStringMode());
            if (a.search(/[ :@]/) != -1) {
                s.push('"');
                s.push(a);
                s.push('"');
            } else {
                s.push(a);
            }
        }
        return s.join("");
    }

    toProperty(prefix) {
        const properties = new Map();

        properties.set(prefix, this.toString());
        properties.set(prefix + ".ConnectionCached", this._cacheConnection ? "1" : "0");
        properties.set(prefix + ".PreferSecure", this._preferSecure ? "1" : "0");
        properties.set(
            prefix + ".EndpointSelection",
            this._endpointSelection === EndpointSelectionType.Random ? "Random" : "Ordered",
        );

        properties.set(prefix + ".LocatorCacheTimeout", String(this._locatorCacheTimeout));
        properties.set(prefix + ".InvocationTimeout", String(this.getInvocationTimeout()));

        if (this._routerInfo !== null) {
            this._routerInfo
                .getRouter()
                ._getReference()
                .toProperty(prefix + ".Router")
                .forEach((value, key) => properties.set(key, value));
        }

        if (this._locatorInfo !== null) {
            this._locatorInfo
                .getLocator()
                ._getReference()
                .toProperty(prefix + ".Locator")
                .forEach((value, key) => properties.set(key, value));
        }

        return properties;
    }

    hashCode() {
        let h = super.hashCode();
        h = HashUtil.addString(h, this._adapterId);
        return h;
    }

    equals(rhs) {
        if (this === rhs) {
            return true;
        }
        if (!(rhs instanceof RoutableReference)) {
            return false;
        }

        if (!super.equals(rhs)) {
            return false;
        }

        if (this._locatorInfo === null ? rhs._locatorInfo !== null : !this._locatorInfo.equals(rhs._locatorInfo)) {
            return false;
        }
        if (this._routerInfo === null ? rhs._routerInfo !== null : !this._routerInfo.equals(rhs._routerInfo)) {
            return false;
        }
        if (this._cacheConnection !== rhs._cacheConnection) {
            return false;
        }
        if (this._preferSecure !== rhs._preferSecure) {
            return false;
        }
        if (this._endpointSelection !== rhs._endpointSelection) {
            return false;
        }
        if (this._locatorCacheTimeout !== rhs._locatorCacheTimeout) {
            return false;
        }
        if (this._connectionId !== rhs._connectionId) {
            return false;
        }
        if (!ArrayUtil.equals(this._endpoints, rhs._endpoints, (e1, e2) => e1.equals(e2))) {
            return false;
        }
        if (this._adapterId !== rhs._adapterId) {
            return false;
        }
        return true;
    }

    getRequestHandler() {
        const handler = new ConnectRequestHandler(this);
        this.getConnection(handler);
        return handler;
    }

    get batchRequestQueue() {
        return this._batchRequestQueue;
    }

    // Sets or resets _batchRequestQueue based on _mode.
    setBatchRequestQueue() {
        this._batchRequestQueue = this.isBatch
            ? new BatchRequestQueue(this._instance, this._mode === ReferenceMode.ModeBatchDatagram)
            : null;
    }

    getConnection(handler) {
        const p = new Promise(); // success callback receives (connection)

        if (this._routerInfo !== null) {
            //
            // If we route, we send everything to the router's client
            // proxy endpoints.
            //
            this._routerInfo
                .getClientEndpoints()
                .then(endpoints => {
                    if (endpoints.length > 0) {
                        this.applyOverrides(endpoints);
                        this.createConnection(endpoints).then(p.resolve, p.reject);
                    } else {
                        this.getConnectionNoRouterInfo(p);
                    }
                })
                .catch(p.reject);
        } else {
            this.getConnectionNoRouterInfo(p);
        }

        p.then(connection => handler.setConnection(connection)).catch(ex => handler.setException(ex));
        return p;
    }

    getConnectionNoRouterInfo(p) {
        if (this._endpoints.length > 0) {
            this.createConnection(this._endpoints).then(p.resolve).catch(p.reject);
            return;
        }

        if (this._locatorInfo !== null) {
            this._locatorInfo
                .getEndpoints(this, null, this._locatorCacheTimeout)
                .then(values => {
                    const [endpoints, cached] = values;
                    if (endpoints.length === 0) {
                        p.reject(new NoEndpointException(this.toString()));
                        return;
                    }

                    this.applyOverrides(endpoints);
                    this.createConnection(endpoints).then(p.resolve, ex => {
                        if (ex instanceof NoEndpointException) {
                            //
                            // No need to retry if there's no endpoints.
                            //
                            p.reject(ex);
                        } else {
                            DEV: console.assert(this._locatorInfo !== null);
                            this.getLocatorInfo().clearCache(this);
                            if (cached) {
                                const traceLevels = this.getInstance().traceLevels();
                                if (traceLevels.retry >= 2) {
                                    this.getInstance()
                                        .initializationData()
                                        .logger.trace(
                                            traceLevels.retryCat,
                                            "connection to cached endpoints failed\n" +
                                                "removing endpoints from cache and trying again\n" +
                                                ex.toString(),
                                        );
                                }
                                this.getConnectionNoRouterInfo(p); // Retry.
                                return;
                            }
                            p.reject(ex);
                        }
                    });
                })
                .catch(p.reject);
        } else {
            p.reject(new NoEndpointException(this.toString()));
        }
    }

    clone() {
        const r = new RoutableReference(
            this.getInstance(),
            this.getCommunicator(),
            this.getIdentity(),
            this.getFacet(),
            this.getMode(),
            this.getSecure(),
            this.getProtocol(),
            this.getEncoding(),
            this._endpoints,
            this._adapterId,
            this._locatorInfo,
            this._routerInfo,
            this._cacheConnection,
            this._preferSecure,
            this._endpointSelection,
            this._locatorCacheTimeout,
            this._invocationTimeout,
        );
        this.copyMembers(r);
        return r;
    }

    copyMembers(rhs) {
        //
        // Copy the members that are not passed to the constructor.
        //
        super.copyMembers(rhs);
        rhs._connectionId = this._connectionId;
    }

    applyOverrides(endpoints) {
        //
        // Apply the endpoint overrides to each endpoint.
        //
        for (let i = 0; i < endpoints.length; ++i) {
            endpoints[i] = endpoints[i].changeConnectionId(this._connectionId);
        }
    }

    filterEndpoints(allEndpoints) {
        //
        // Filter out opaque endpoints or endpoints which can't connect.
        //
        let endpoints = allEndpoints.filter(e => !(e instanceof OpaqueEndpointI) && e.connectable());

        //
        // Filter out endpoints according to the mode of the reference.
        //
        switch (this.getMode()) {
            case ReferenceMode.ModeTwoway:
            case ReferenceMode.ModeOneway:
            case ReferenceMode.ModeBatchOneway: {
                //
                // Filter out datagram endpoints.
                //
                endpoints = endpoints.filter(e => !e.datagram());
                break;
            }

            case ReferenceMode.ModeDatagram:
            case ReferenceMode.ModeBatchDatagram: {
                //
                // Filter out non-datagram endpoints.
                //
                endpoints = endpoints.filter(e => e.datagram());
                break;
            }

            default: {
                DEV: console.assert(false);
                break;
            }
        }

        //
        // Sort the endpoints according to the endpoint selection type.
        //
        switch (this.getEndpointSelection()) {
            case EndpointSelectionType.Random: {
                //
                // Shuffle the endpoints.
                //
                for (let i = endpoints.length; i > 0; --i) {
                    const e = endpoints[i - 1];
                    const rand = Math.floor(Math.random() * i);
                    endpoints[i - 1] = endpoints[rand];
                    endpoints[rand] = e;
                }
                break;
            }
            case EndpointSelectionType.Ordered: {
                // Nothing to do.
                break;
            }
            default: {
                DEV: console.assert(false);
                break;
            }
        }

        //
        // If a secure connection is requested or secure overrides is
        // set, remove all non-secure endpoints. Otherwise if preferSecure is set
        // make secure endpoints preferred. By default make non-secure
        // endpoints preferred over secure endpoints.
        //
        const overrides = this.getInstance().defaultsAndOverrides();
        if (overrides.overrideSecure ? overrides.overrideSecureValue : this.getSecure()) {
            endpoints = endpoints.filter(e => e.secure());
        } else {
            const preferSecure = this.getPreferSecure();
            const compare = (e1, e2) => {
                const ls = e1.secure();
                const rs = e2.secure();
                if ((ls && rs) || (!ls && !rs)) {
                    return 0;
                } else if (!ls && rs) {
                    return preferSecure ? 1 : -1;
                } else {
                    return preferSecure ? -1 : 1;
                }
            };
            endpoints.sort(compare);
        }
        return endpoints;
    }

    createConnection(allEndpoints) {
        const endpoints = this.filterEndpoints(allEndpoints);
        if (endpoints.length === 0) {
            return Promise.reject(new NoEndpointException(this.toString()));
        }

        //
        // Finally, create the connection.
        //
        const promise = new Promise();
        const factory = this.getInstance().outgoingConnectionFactory();
        if (this.getCacheConnection() || endpoints.length == 1) {
            //
            // Get an existing connection or create one if there's no
            // existing connection to one of the given endpoints.
            //
            const cb = new CreateConnectionCallback(this, null, promise);
            factory
                .create(endpoints, false, this.getEndpointSelection())
                .then(connection => cb.setConnection(connection))
                .catch(ex => cb.setException(ex));
        } else {
            //
            // Go through the list of endpoints and try to create the
            // connection until it succeeds. This is different from just
            // calling create() with the given endpoints since this might
            // create a new connection even if there's an existing
            // connection for one of the endpoints.
            //
            const cb = new CreateConnectionCallback(this, endpoints, promise);
            factory
                .create([endpoints[0]], true, this.getEndpointSelection())
                .then(connection => cb.setConnection(connection))
                .catch(ex => cb.setException(ex));
        }
        return promise;
    }
}

class CreateConnectionCallback {
    constructor(r, endpoints, promise) {
        this.ref = r;
        this.endpoints = endpoints;
        this.promise = promise;
        this.i = 0;
        this.exception = null;
    }

    setConnection(connection) {
        //
        // If we have a router, set the object adapter for this router
        // (if any) to the new connection, so that callbacks from the
        // router can be received over this new connection.
        //
        if (this.ref.getRouterInfo() !== null && this.ref.getRouterInfo().getAdapter() !== null) {
            connection.setAdapter(this.ref.getRouterInfo().getAdapter());
        }
        this.promise.resolve(connection);
    }

    setException(ex) {
        if (this.exception === null) {
            this.exception = ex;
        }

        if (this.endpoints === null || ++this.i === this.endpoints.length) {
            this.promise.reject(this.exception);
            return;
        }

        this.ref
            .getInstance()
            .outgoingConnectionFactory()
            .create([this.endpoints[this.i]], this.i != this.endpoints.length - 1, this.ref.getEndpointSelection())
            .then(connection => this.setConnection(connection))
            .catch(ex => this.setException(ex));
    }
}
