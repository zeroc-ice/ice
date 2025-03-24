// Copyright (c) ZeroC, Inc.

import { ObjectPrx } from "./ObjectPrx.js";

import { ArrayUtil } from "./ArrayUtil.js";
import { OutgoingAsync, ProxyFlushBatch, ProxyGetConnection } from "./OutgoingAsync.js";
import { ReferenceMode } from "./ReferenceMode.js";
import { UserException } from "./UserException.js";
import { ParseException, TwowayOnlyException, UnknownUserException } from "./LocalExceptions.js";
import { ConnectionI } from "./ConnectionI.js";
import { TypeRegistry } from "./TypeRegistry.js";
import { Communicator } from "./Communicator.js";
import { Reference } from "./Reference.js";
import { RequestHandlerCache } from "./RequestHandlerCache.js";

ObjectPrx.prototype.hashCode = function () {
    return this._reference.hashCode();
};

ObjectPrx.prototype.ice_getCommunicator = function () {
    return this._reference.getCommunicator();
};

ObjectPrx.prototype.toString = function () {
    return this._reference.toString();
};

ObjectPrx.prototype.ice_getIdentity = function () {
    return this._reference.getIdentity().clone();
};

ObjectPrx.prototype.ice_identity = function (newIdentity) {
    if (newIdentity === undefined || newIdentity === null || newIdentity.name.length === 0) {
        throw new TypeError("The name of an Ice object identity cannot be empty.");
    }
    if (newIdentity.equals(this._reference.getIdentity())) {
        return this;
    } else {
        return new ObjectPrx(this._reference.changeIdentity(newIdentity));
    }
};

ObjectPrx.prototype.ice_getContext = function () {
    return new Map(this._reference.getContext());
};

ObjectPrx.prototype.ice_context = function (newContext) {
    return this._newInstance(this._reference.changeContext(newContext));
};

ObjectPrx.prototype.ice_getFacet = function () {
    return this._reference.getFacet();
};

ObjectPrx.prototype.ice_facet = function (newFacet) {
    if (newFacet === undefined || newFacet === null) {
        newFacet = "";
    }

    if (newFacet === this._reference.getFacet()) {
        return this;
    } else {
        return new ObjectPrx(this._reference.changeFacet(newFacet));
    }
};

ObjectPrx.prototype.ice_getAdapterId = function () {
    return this._reference.getAdapterId();
};

ObjectPrx.prototype.ice_adapterId = function (newAdapterId) {
    if (newAdapterId === undefined || newAdapterId === null) {
        newAdapterId = "";
    }

    if (newAdapterId === this._reference.getAdapterId()) {
        return this;
    } else {
        return this._newInstance(this._reference.changeAdapterId(newAdapterId));
    }
};

ObjectPrx.prototype.ice_getEndpoints = function () {
    return ArrayUtil.clone(this._reference.getEndpoints());
};

ObjectPrx.prototype.ice_endpoints = function (newEndpoints) {
    if (newEndpoints === undefined || newEndpoints === null) {
        newEndpoints = [];
    }

    if (ArrayUtil.equals(newEndpoints, this._reference.getEndpoints())) {
        return this;
    } else {
        return this._newInstance(this._reference.changeEndpoints(newEndpoints));
    }
};

ObjectPrx.prototype.ice_getLocatorCacheTimeout = function () {
    return this._reference.getLocatorCacheTimeout();
};

ObjectPrx.prototype.ice_locatorCacheTimeout = function (newTimeout) {
    if (newTimeout < -1) {
        throw new RangeError("invalid value passed to ice_locatorCacheTimeout: " + newTimeout);
    }
    if (newTimeout === this._reference.getLocatorCacheTimeout()) {
        return this;
    } else {
        return this._newInstance(this._reference.changeLocatorCacheTimeout(newTimeout));
    }
};

ObjectPrx.prototype.ice_getInvocationTimeout = function () {
    return this._reference.getInvocationTimeout();
};

ObjectPrx.prototype.ice_invocationTimeout = function (newTimeout) {
    if (newTimeout < 1 && newTimeout !== -1) {
        throw new RangeError("invalid value passed to ice_invocationTimeout: " + newTimeout);
    }
    if (newTimeout === this._reference.getInvocationTimeout()) {
        return this;
    } else {
        return this._newInstance(this._reference.changeInvocationTimeout(newTimeout));
    }
};

ObjectPrx.prototype.ice_isConnectionCached = function () {
    return this._reference.getCacheConnection();
};

ObjectPrx.prototype.ice_connectionCached = function (newCache) {
    if (newCache === this._reference.getCacheConnection()) {
        return this;
    } else {
        return this._newInstance(this._reference.changeCacheConnection(newCache));
    }
};

ObjectPrx.prototype.ice_getEndpointSelection = function () {
    return this._reference.getEndpointSelection();
};

ObjectPrx.prototype.ice_endpointSelection = function (newType) {
    if (newType === this._reference.getEndpointSelection()) {
        return this;
    } else {
        return this._newInstance(this._reference.changeEndpointSelection(newType));
    }
};

ObjectPrx.prototype.ice_isSecure = function () {
    return this._reference.getSecure();
};

ObjectPrx.prototype.ice_secure = function (b) {
    if (b === this._reference.getSecure()) {
        return this;
    } else {
        return this._newInstance(this._reference.changeSecure(b));
    }
};

ObjectPrx.prototype.ice_getEncodingVersion = function () {
    return this._reference.getEncoding().clone();
};

ObjectPrx.prototype.ice_encodingVersion = function (e) {
    if (e.equals(this._reference.getEncoding())) {
        return this;
    } else {
        return this._newInstance(this._reference.changeEncoding(e));
    }
};

ObjectPrx.prototype.ice_isPreferSecure = function () {
    return this._reference.getPreferSecure();
};

ObjectPrx.prototype.ice_preferSecure = function (b) {
    if (b === this._reference.getPreferSecure()) {
        return this;
    } else {
        return this._newInstance(this._reference.changePreferSecure(b));
    }
};

ObjectPrx.prototype.ice_getRouter = function () {
    const ri = this._reference.getRouterInfo();
    return ri !== null ? ri.getRouter() : null;
};

ObjectPrx.prototype.ice_router = function (router) {
    const ref = this._reference.changeRouter(router);
    if (ref.equals(this._reference)) {
        return this;
    } else {
        return this._newInstance(ref);
    }
};

ObjectPrx.prototype.ice_getLocator = function () {
    const ri = this._reference.getLocatorInfo();
    return ri !== null ? ri.getLocator() : null;
};

ObjectPrx.prototype.ice_locator = function (locator) {
    const ref = this._reference.changeLocator(locator);
    if (ref.equals(this._reference)) {
        return this;
    } else {
        return this._newInstance(ref);
    }
};

ObjectPrx.prototype.ice_isTwoway = function () {
    return this._reference.getMode() === ReferenceMode.ModeTwoway;
};

ObjectPrx.prototype.ice_twoway = function () {
    if (this._reference.getMode() === ReferenceMode.ModeTwoway) {
        return this;
    } else {
        return this._newInstance(this._reference.changeMode(ReferenceMode.ModeTwoway));
    }
};

ObjectPrx.prototype.ice_isOneway = function () {
    return this._reference.getMode() === ReferenceMode.ModeOneway;
};

ObjectPrx.prototype.ice_oneway = function () {
    if (this._reference.getMode() === ReferenceMode.ModeOneway) {
        return this;
    } else {
        return this._newInstance(this._reference.changeMode(ReferenceMode.ModeOneway));
    }
};

ObjectPrx.prototype.ice_isBatchOneway = function () {
    return this._reference.getMode() === ReferenceMode.ModeBatchOneway;
};

ObjectPrx.prototype.ice_batchOneway = function () {
    if (this._reference.getMode() === ReferenceMode.ModeBatchOneway) {
        return this;
    } else {
        return this._newInstance(this._reference.changeMode(ReferenceMode.ModeBatchOneway));
    }
};

ObjectPrx.prototype.ice_isDatagram = function () {
    return this._reference.getMode() === ReferenceMode.ModeDatagram;
};

ObjectPrx.prototype.ice_datagram = function () {
    if (this._reference.getMode() === ReferenceMode.ModeDatagram) {
        return this;
    } else {
        return this._newInstance(this._reference.changeMode(ReferenceMode.ModeDatagram));
    }
};

ObjectPrx.prototype.ice_isBatchDatagram = function () {
    return this._reference.getMode() === ReferenceMode.ModeBatchDatagram;
};

ObjectPrx.prototype.ice_batchDatagram = function () {
    if (this._reference.getMode() === ReferenceMode.ModeBatchDatagram) {
        return this;
    } else {
        return this._newInstance(this._reference.changeMode(ReferenceMode.ModeBatchDatagram));
    }
};

ObjectPrx.prototype.ice_fixed = function (connection) {
    if (connection === null) {
        throw new RangeError("invalid null connection passed to ice_fixed");
    }
    if (!(connection instanceof ConnectionI)) {
        throw new RangeError("invalid connection passed to ice_fixed");
    }
    const ref = this._reference.changeConnection(connection);
    if (ref.equals(this._reference)) {
        return this;
    } else {
        return this._newInstance(ref);
    }
};

ObjectPrx.prototype.ice_isFixed = function () {
    return this._reference.isFixed();
};

ObjectPrx.prototype.ice_getConnectionId = function () {
    return this._reference.getConnectionId();
};

ObjectPrx.prototype.ice_connectionId = function (id) {
    if (id == this._reference.getConnectionId()) {
        return this;
    } else {
        return this._newInstance(this._reference.changeConnectionId(id));
    }
};

ObjectPrx.prototype.ice_getConnection = function () {
    const r = new ProxyGetConnection(this, "ice_getConnection");
    try {
        r.invoke();
    } catch (ex) {
        r.abort(ex);
    }
    return r;
};

ObjectPrx.prototype.ice_getCachedConnection = function () {
    return this._requestHandlerCache.cachedConnection;
};

ObjectPrx.prototype.ice_flushBatchRequests = function () {
    const r = new ProxyFlushBatch(this, "ice_flushBatchRequests");
    try {
        r.invoke();
    } catch (ex) {
        r.abort(ex);
    }
    return r;
};

ObjectPrx.prototype.equals = function (r) {
    if (this === r) {
        return true;
    }

    if (r instanceof ObjectPrx) {
        return this._reference.equals(r._reference);
    }

    return false;
};

ObjectPrx.prototype._write = function (os) {
    this._reference.getIdentity()._write(os);
    this._reference.streamWrite(os);
};

ObjectPrx.prototype._getReference = function () {
    return this._reference;
};

ObjectPrx.prototype._checkAsyncTwowayOnly = function (name) {
    if (!this.ice_isTwoway()) {
        throw new TwowayOnlyException(name);
    }
};

//
// Only for use by ObjectPrx constructor
//
ObjectPrx.prototype._setup = function (arg0, proxyString = "") {
    DEV: console.assert(this._reference === undefined);
    if (arg0 instanceof Communicator) {
        this._reference = arg0.instance.referenceFactory().createFromString(proxyString, "");
        if (this._reference === null) {
            throw new ParseException("Invalid empty proxy string.");
        }
        this._requestHandlerCache = new RequestHandlerCache(this._reference);
    } else if (arg0 instanceof Reference) {
        this._reference = arg0;
        this._requestHandlerCache = new RequestHandlerCache(this._reference);
    } else if (arg0 instanceof ObjectPrx) {
        this._reference = arg0._reference;
        this._requestHandlerCache = arg0._requestHandlerCache;
    } else {
        throw new Error("invalid argument passed to ObjectPrx constructor");
    }
};

ObjectPrx.prototype._newInstance = function (ref) {
    return new this.constructor(ref);
};

ObjectPrx.prototype.ice_instanceof = function (T) {
    if (T) {
        if (this instanceof T) {
            return true;
        }
        return this.constructor._instanceof(T);
    }
    return false;
};

//
// Generic invocation for operations that have input parameters.
//
ObjectPrx._invoke = function (p, name, mode, fmt, ctx, marshalFn, unmarshalFn, userEx, args) {
    if (unmarshalFn !== null || userEx.length > 0) {
        p._checkAsyncTwowayOnly(name);
    }

    const r = new OutgoingAsync(p, name, res => {
        this._completed(res, unmarshalFn, userEx);
    });

    try {
        r.prepare(name, mode, ctx);
        if (marshalFn === null) {
            r.writeEmptyParams();
        } else {
            const ostr = r.startWriteParams(fmt);
            marshalFn(ostr, args);
            r.endWriteParams();
        }
        r.invoke();
    } catch (ex) {
        r.abort(ex);
    }
    return r;
};

//
// Handles the completion of an invocation.
//
ObjectPrx._completed = function (r, unmarshalFn, userEx) {
    if (!this._check(r, userEx)) {
        return;
    }

    try {
        if (unmarshalFn === null) {
            r.readEmptyParams();
            r.resolve();
        } else {
            r.resolve(unmarshalFn(r));
        }
    } catch (ex) {
        this.dispatchLocalException(r, ex);
    }
};

//
// Handles user exceptions.
//
ObjectPrx._check = function (r, uex) {
    //
    // If uex is non-null, it must be an array of exception types.
    //
    try {
        r.throwUserException();
    } catch (ex) {
        if (ex instanceof UserException) {
            if (uex !== null) {
                for (let i = 0; i < uex.length; ++i) {
                    if (ex instanceof uex[i]) {
                        r.reject(ex);
                        return false;
                    }
                }
            }
            r.reject(new UnknownUserException(ex.ice_id()));
            return false;
        } else {
            r.reject(ex);
            return false;
        }
    }

    return true;
};

ObjectPrx.dispatchLocalException = function (r, ex) {
    r.reject(ex);
};

ObjectPrx.checkedCast = async function (prx, facet, ctx) {
    let r = null;
    if (prx !== undefined && prx !== null) {
        if (facet !== undefined) {
            prx = prx.ice_facet(facet);
        }

        if (await prx.ice_isA(this.ice_staticId(), ctx)) {
            r = new this(prx);
        }
    }
    return r;
};

ObjectPrx.uncheckedCast = function (prx, facet) {
    return prx === null ? null : new this(facet === undefined ? prx : prx.ice_facet(facet));
};

Object.defineProperty(ObjectPrx, "minWireSize", { get: () => 2 });

ObjectPrx.write = function (os, v) {
    os.writeProxy(v);
};

ObjectPrx.read = function (is) {
    return is.readProxy(this);
};

ObjectPrx.writeOptional = function (os, tag, v) {
    os.writeOptionalProxy(tag, v);
};

ObjectPrx.readOptional = function (is, tag) {
    return is.readOptionalProxy(tag, this);
};

ObjectPrx._instanceof = function (T) {
    if (T === this) {
        return true;
    }

    for (const i in this._implements) {
        if (this._implements[i]._instanceof(T)) {
            return true;
        }
    }

    return false;
};

ObjectPrx.ice_staticId = function () {
    return this._id;
};

Object.defineProperty(ObjectPrx, "_implements", { get: () => [] });
TypeRegistry.declareProxyType("Ice.ObjectPrx", ObjectPrx);
