//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { ObjectPrx } from "./ObjectPrx.js";

import { ArrayUtil } from "./ArrayUtil.js";
import { AsyncResultBase } from "./AsyncResultBase.js";
import { OutgoingAsync, ProxyFlushBatch, ProxyGetConnection } from "./OutgoingAsync.js";
import { ReferenceMode } from "./ReferenceMode.js";
import { Ice as Ice_OperationMode } from "./OperationMode.js";
const { OperationMode } = Ice_OperationMode;
import { LocalException, UserException } from "./Exception.js";
import {
    CloseConnectionException,
    CommunicatorDestroyedException,
    FacetNotExistException,
    IllegalIdentityException,
    ObjectNotExistException,
    TwowayOnlyException,
    UnknownUserException,
} from "./LocalException.js";
import { ConnectionI } from "./ConnectionI.js";
import { TypeRegistry } from "./TypeRegistry.js";
import { Debug } from "./Debug.js";

ObjectPrx.prototype.hashCode = function (r) {
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
        throw new IllegalIdentityException();
    }
    if (newIdentity.equals(this._reference.getIdentity())) {
        return this;
    } else {
        const proxy = new ObjectPrx();
        proxy._setup(this._reference.changeIdentity(newIdentity));
        return proxy;
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
        const proxy = new ObjectPrx();
        proxy._setup(this._reference.changeFacet(newFacet));
        return proxy;
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

ObjectPrx.prototype.ice_timeout = function (t) {
    if (t < 1 && t !== -1) {
        throw new RangeError("invalid value passed to ice_timeout: " + t);
    }
    const ref = this._reference.changeTimeout(t);
    if (ref.equals(this._reference)) {
        return this;
    } else {
        return this._newInstance(ref);
    }
};

ObjectPrx.prototype.ice_getTimeout = function () {
    return this._reference.getTimeout();
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
    const ref = this._reference.changeConnectionId(id);
    if (ref.equals(this._reference)) {
        return this;
    } else {
        return this._newInstance(ref);
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
    return this._requestHandler ? this._requestHandler.getConnection() : null;
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

ObjectPrx.prototype._copyFrom = function (from) {
    Debug.assert(this._reference === null);
    Debug.assert(this._requestHandler === null);

    this._reference = from._reference;
    this._requestHandler = from._requestHandler;
};

ObjectPrx.prototype._handleException = function (ex, handler, mode, sent, sleep, cnt) {
    this._updateRequestHandler(handler, null); // Clear the request handler

    //
    // We only retry local exception, system exceptions aren't retried.
    //
    // A CloseConnectionException indicates graceful server shutdown, and is therefore
    // always repeatable without violating "at-most-once". That's because by sending a
    // close connection message, the server guarantees that all outstanding requests
    // can safely be repeated.
    //
    // An ObjectNotExistException can always be retried as well without violating
    // "at-most-once" (see the implementation of the checkRetryAfterException method
    //  of the ProxyFactory class for the reasons why it can be useful).
    //
    // If the request didn't get sent or if it's non-mutating or idempotent it can
    // also always be retried if the retry count isn't reached.
    //
    if (
        ex instanceof LocalException &&
        (!sent ||
            mode == OperationMode.Nonmutating ||
            mode == OperationMode.Idempotent ||
            ex instanceof CloseConnectionException ||
            ex instanceof ObjectNotExistException)
    ) {
        try {
            return this._reference
                .getInstance()
                .proxyFactory()
                .checkRetryAfterException(ex, this._reference, sleep, cnt);
        } catch (exc) {
            if (exc instanceof CommunicatorDestroyedException) {
                //
                // The communicator is already destroyed, so we cannot retry.
                //
                throw ex;
            } else {
                throw exc;
            }
        }
    } else {
        throw ex;
    }
};

ObjectPrx.prototype._checkAsyncTwowayOnly = function (name) {
    if (!this.ice_isTwoway()) {
        throw new TwowayOnlyException(name);
    }
};

ObjectPrx.prototype._getRequestHandler = function () {
    if (this._reference.getCacheConnection()) {
        if (this._requestHandler) {
            return this._requestHandler;
        }
    }
    return this._reference.getRequestHandler(this);
};

ObjectPrx.prototype._getBatchRequestQueue = function () {
    if (!this._batchRequestQueue) {
        this._batchRequestQueue = this._reference.getBatchRequestQueue();
    }
    return this._batchRequestQueue;
};

ObjectPrx.prototype._setRequestHandler = function (handler) {
    if (this._reference.getCacheConnection()) {
        if (!this._requestHandler) {
            this._requestHandler = handler;
        }
        return this._requestHandler;
    }
    return handler;
};

ObjectPrx.prototype._updateRequestHandler = function (previous, handler) {
    if (this._reference.getCacheConnection() && previous !== null) {
        if (this._requestHandler && this._requestHandler !== handler) {
            this._requestHandler = this._requestHandler.update(previous, handler);
        }
    }
};

//
// Only for use by IceInternal.ProxyFactory
//
ObjectPrx.prototype._setup = function (ref) {
    Debug.assert(this._reference === null);

    this._reference = ref;
};

ObjectPrx.prototype._newInstance = function (ref) {
    const proxy = new this.constructor();
    proxy._setup(ref);
    return proxy;
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

    const r = new OutgoingAsync(p, name, (res) => {
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

ObjectPrx.checkedCast = function (prx, facet, ctx) {
    let r = null;

    if (prx === undefined || prx === null) {
        r = new AsyncResultBase(null, "checkedCast", null, null, null);
        r.resolve(null);
    } else {
        if (facet !== undefined) {
            prx = prx.ice_facet(facet);
        }

        r = new AsyncResultBase(prx.ice_getCommunicator(), "checkedCast", null, prx, null);
        prx.ice_isA(this.ice_staticId(), ctx)
            .then((ret) => {
                if (ret) {
                    const h = new this();
                    h._copyFrom(prx);
                    r.resolve(h);
                } else {
                    r.resolve(null);
                }
            })
            .catch((ex) => {
                if (ex instanceof FacetNotExistException) {
                    r.resolve(null);
                } else {
                    r.reject(ex);
                }
            });
    }

    return r;
};

ObjectPrx.uncheckedCast = function (prx, facet) {
    let r = null;
    if (prx !== undefined && prx !== null) {
        r = new this();
        if (facet !== undefined) {
            prx = prx.ice_facet(facet);
        }
        r._copyFrom(prx);
    }
    return r;
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
TypeRegistry.declareValueType("Ice.ObjectPrx", ObjectPrx);
