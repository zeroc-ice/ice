//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/ArrayUtil");
require("../Ice/AsyncResult");
require("../Ice/BuiltinSequences");
require("../Ice/Current");
require("../Ice/Debug");
require("../Ice/Exception");
require("../Ice/FormatType");
require("../Ice/LocalException");
require("../Ice/Object");
require("../Ice/OutgoingAsync");
require("../Ice/ReferenceMode");

const ArrayUtil = Ice.ArrayUtil;
const AsyncResultBase = Ice.AsyncResultBase;
const Debug = Ice.Debug;
const OutgoingAsync = Ice.OutgoingAsync;
const ProxyFlushBatch = Ice.ProxyFlushBatch;
const ProxyGetConnection = Ice.ProxyGetConnection;
const RefMode = Ice.ReferenceMode;
const OperationMode = Ice.OperationMode;

//
// Ice.ObjectPrx
//
class ObjectPrx
{
    constructor()
    {
        this._reference = null;
        this._requestHandler = null;
    }

    hashCode(r)
    {
        return this._reference.hashCode();
    }

    ice_getCommunicator()
    {
        return this._reference.getCommunicator();
    }

    toString()
    {
        return this._reference.toString();
    }

    ice_getIdentity()
    {
        return this._reference.getIdentity().clone();
    }

    ice_identity(newIdentity)
    {
        if(newIdentity === undefined || newIdentity === null || newIdentity.name.length === 0)
        {
            throw new Ice.IllegalIdentityException();
        }
        if(newIdentity.equals(this._reference.getIdentity()))
        {
            return this;
        }
        else
        {
            const proxy = new ObjectPrx();
            proxy._setup(this._reference.changeIdentity(newIdentity));
            return proxy;
        }
    }

    ice_getContext()
    {
        return new Map(this._reference.getContext());
    }

    ice_context(newContext)
    {
        return this._newInstance(this._reference.changeContext(newContext));
    }

    ice_getFacet()
    {
        return this._reference.getFacet();
    }

    ice_facet(newFacet)
    {
        if(newFacet === undefined || newFacet === null)
        {
            newFacet = "";
        }

        if(newFacet === this._reference.getFacet())
        {
            return this;
        }
        else
        {
            const proxy = new ObjectPrx();
            proxy._setup(this._reference.changeFacet(newFacet));
            return proxy;
        }
    }

    ice_getAdapterId()
    {
        return this._reference.getAdapterId();
    }

    ice_adapterId(newAdapterId)
    {
        if(newAdapterId === undefined || newAdapterId === null)
        {
            newAdapterId = "";
        }

        if(newAdapterId === this._reference.getAdapterId())
        {
            return this;
        }
        else
        {
            return this._newInstance(this._reference.changeAdapterId(newAdapterId));
        }
    }

    ice_getEndpoints()
    {
        return ArrayUtil.clone(this._reference.getEndpoints());
    }

    ice_endpoints(newEndpoints)
    {
        if(newEndpoints === undefined || newEndpoints === null)
        {
            newEndpoints = [];
        }

        if(ArrayUtil.equals(newEndpoints, this._reference.getEndpoints()))
        {
            return this;
        }
        else
        {
            return this._newInstance(this._reference.changeEndpoints(newEndpoints));
        }
    }

    ice_getLocatorCacheTimeout()
    {
        return this._reference.getLocatorCacheTimeout();
    }

    ice_locatorCacheTimeout(newTimeout)
    {
        if(newTimeout < -1)
        {
            throw new RangeError("invalid value passed to ice_locatorCacheTimeout: " + newTimeout);
        }
        if(newTimeout === this._reference.getLocatorCacheTimeout())
        {
            return this;
        }
        else
        {
            return this._newInstance(this._reference.changeLocatorCacheTimeout(newTimeout));
        }
    }

    ice_getInvocationTimeout()
    {
        return this._reference.getInvocationTimeout();
    }

    ice_invocationTimeout(newTimeout)
    {
        if(newTimeout < 1 && newTimeout !== -1)
        {
            throw new RangeError("invalid value passed to ice_invocationTimeout: " + newTimeout);
        }
        if(newTimeout === this._reference.getInvocationTimeout())
        {
            return this;
        }
        else
        {
            return this._newInstance(this._reference.changeInvocationTimeout(newTimeout));
        }
    }

    ice_isConnectionCached()
    {
        return this._reference.getCacheConnection();
    }

    ice_connectionCached(newCache)
    {
        if(newCache === this._reference.getCacheConnection())
        {
            return this;
        }
        else
        {
            return this._newInstance(this._reference.changeCacheConnection(newCache));
        }
    }

    ice_getEndpointSelection()
    {
        return this._reference.getEndpointSelection();
    }

    ice_endpointSelection(newType)
    {
        if(newType === this._reference.getEndpointSelection())
        {
            return this;
        }
        else
        {
            return this._newInstance(this._reference.changeEndpointSelection(newType));
        }
    }

    ice_isSecure()
    {
        return this._reference.getSecure();
    }

    ice_secure(b)
    {
        if(b === this._reference.getSecure())
        {
            return this;
        }
        else
        {
            return this._newInstance(this._reference.changeSecure(b));
        }
    }

    ice_getEncodingVersion()
    {
        return this._reference.getEncoding().clone();
    }

    ice_encodingVersion(e)
    {
        if(e.equals(this._reference.getEncoding()))
        {
            return this;
        }
        else
        {
            return this._newInstance(this._reference.changeEncoding(e));
        }
    }

    ice_isPreferSecure()
    {
        return this._reference.getPreferSecure();
    }

    ice_preferSecure(b)
    {
        if(b === this._reference.getPreferSecure())
        {
            return this;
        }
        else
        {
            return this._newInstance(this._reference.changePreferSecure(b));
        }
    }

    ice_getRouter()
    {
        const ri = this._reference.getRouterInfo();
        return ri !== null ? ri.getRouter() : null;
    }

    ice_router(router)
    {
        const ref = this._reference.changeRouter(router);
        if(ref.equals(this._reference))
        {
            return this;
        }
        else
        {
            return this._newInstance(ref);
        }
    }

    ice_getLocator()
    {
        const ri = this._reference.getLocatorInfo();
        return ri !== null ? ri.getLocator() : null;
    }

    ice_locator(locator)
    {
        const ref = this._reference.changeLocator(locator);
        if(ref.equals(this._reference))
        {
            return this;
        }
        else
        {
            return this._newInstance(ref);
        }
    }

    ice_isTwoway()
    {
        return this._reference.getMode() === RefMode.ModeTwoway;
    }

    ice_twoway()
    {
        if(this._reference.getMode() === RefMode.ModeTwoway)
        {
            return this;
        }
        else
        {
            return this._newInstance(this._reference.changeMode(RefMode.ModeTwoway));
        }
    }

    ice_isOneway()
    {
        return this._reference.getMode() === RefMode.ModeOneway;
    }

    ice_oneway()
    {
        if(this._reference.getMode() === RefMode.ModeOneway)
        {
            return this;
        }
        else
        {
            return this._newInstance(this._reference.changeMode(RefMode.ModeOneway));
        }
    }

    ice_isBatchOneway()
    {
        return this._reference.getMode() === RefMode.ModeBatchOneway;
    }

    ice_batchOneway()
    {
        if(this._reference.getMode() === RefMode.ModeBatchOneway)
        {
            return this;
        }
        else
        {
            return this._newInstance(this._reference.changeMode(RefMode.ModeBatchOneway));
        }
    }

    ice_isDatagram()
    {
        return this._reference.getMode() === RefMode.ModeDatagram;
    }

    ice_datagram()
    {
        if(this._reference.getMode() === RefMode.ModeDatagram)
        {
            return this;
        }
        else
        {
            return this._newInstance(this._reference.changeMode(RefMode.ModeDatagram));
        }
    }

    ice_isBatchDatagram()
    {
        return this._reference.getMode() === RefMode.ModeBatchDatagram;
    }

    ice_batchDatagram()
    {
        if(this._reference.getMode() === RefMode.ModeBatchDatagram)
        {
            return this;
        }
        else
        {
            return this._newInstance(this._reference.changeMode(RefMode.ModeBatchDatagram));
        }
    }

    ice_timeout(t)
    {
        if(t < 1 && t !== -1)
        {
            throw new RangeError("invalid value passed to ice_timeout: " + t);
        }
        const ref = this._reference.changeTimeout(t);
        if(ref.equals(this._reference))
        {
            return this;
        }
        else
        {
            return this._newInstance(ref);
        }
    }

    ice_getTimeout()
    {
        return this._reference.getTimeout();
    }

    ice_fixed(connection)
    {
        if(connection === null)
        {
            throw new RangeError("invalid null connection passed to ice_fixed");
        }
        if(!(connection instanceof Ice.ConnectionI))
        {
            throw new RangeError("invalid connection passed to ice_fixed");
        }
        const ref = this._reference.changeConnection(connection);
        if(ref.equals(this._reference))
        {
            return this;
        }
        else
        {
            return this._newInstance(ref);
        }
    }

    ice_isFixed()
    {
        return this._reference instanceof Ice.FixedReference;
    }

    ice_getConnectionId()
    {
        return this._reference.getConnectionId();
    }

    ice_connectionId(id)
    {
        const ref = this._reference.changeConnectionId(id);
        if(ref.equals(this._reference))
        {
            return this;
        }
        else
        {
            return this._newInstance(ref);
        }
    }

    ice_getConnection()
    {
        const r = new ProxyGetConnection(this, "ice_getConnection");
        try
        {
            r.invoke();
        }
        catch(ex)
        {
            r.abort(ex);
        }
        return r;
    }

    ice_getCachedConnection()
    {
        return this._requestHandler ? this._requestHandler.getConnection() : null;
    }

    ice_flushBatchRequests()
    {
        const r = new ProxyFlushBatch(this, "ice_flushBatchRequests");
        try
        {
            r.invoke();
        }
        catch(ex)
        {
            r.abort(ex);
        }
        return r;
    }

    equals(r)
    {
        if(this === r)
        {
            return true;
        }

        if(r instanceof ObjectPrx)
        {
            return this._reference.equals(r._reference);
        }

        return false;
    }

    _write(os)
    {
        this._reference.getIdentity()._write(os);
        this._reference.streamWrite(os);
    }

    _getReference()
    {
        return this._reference;
    }

    _copyFrom(from)
    {
        Debug.assert(this._reference === null);
        Debug.assert(this._requestHandler === null);

        this._reference = from._reference;
        this._requestHandler = from._requestHandler;
    }

    _handleException(ex, handler, mode, sent, sleep, cnt)
    {
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
        if(ex instanceof Ice.LocalException &&
           (!sent ||
            mode == OperationMode.Nonmutating || mode == OperationMode.Idempotent ||
            ex instanceof Ice.CloseConnectionException || ex instanceof Ice.ObjectNotExistException))
        {
            try
            {
                return this._reference.getInstance().proxyFactory().checkRetryAfterException(ex,
                                                                                             this._reference,
                                                                                             sleep,
                                                                                             cnt);
            }
            catch(exc)
            {
                if(exc instanceof Ice.CommunicatorDestroyedException)
                {
                    //
                    // The communicator is already destroyed, so we cannot retry.
                    //
                    throw ex;
                }
                else
                {
                    throw exc;
                }
            }
        }
        else
        {
            throw ex;
        }
    }

    _checkAsyncTwowayOnly(name)
    {
        if(!this.ice_isTwoway())
        {
            throw new Ice.TwowayOnlyException(name);
        }
    }

    _getRequestHandler()
    {
        if(this._reference.getCacheConnection())
        {
            if(this._requestHandler)
            {
                return this._requestHandler;
            }
        }
        return this._reference.getRequestHandler(this);
    }

    _getBatchRequestQueue()
    {
        if(!this._batchRequestQueue)
        {
            this._batchRequestQueue = this._reference.getBatchRequestQueue();
        }
        return this._batchRequestQueue;
    }

    _setRequestHandler(handler)
    {
        if(this._reference.getCacheConnection())
        {
            if(!this._requestHandler)
            {
                this._requestHandler = handler;
            }
            return this._requestHandler;
        }
        return handler;
    }

    _updateRequestHandler(previous, handler)
    {
        if(this._reference.getCacheConnection() && previous !== null)
        {
            if(this._requestHandler && this._requestHandler !== handler)
            {
                this._requestHandler = this._requestHandler.update(previous, handler);
            }
        }
    }

    //
    // Only for use by IceInternal.ProxyFactory
    //
    _setup(ref)
    {
        Debug.assert(this._reference === null);

        this._reference = ref;
    }

    _newInstance(ref)
    {
        const proxy = new this.constructor();
        proxy._setup(ref);
        return proxy;
    }

    ice_instanceof(T)
    {
        if(T)
        {
            if(this instanceof T)
            {
                return true;
            }
            return this.constructor._instanceof(T);
        }
        return false;
    }

    //
    // Generic invocation for operations that have input parameters.
    //
    static _invoke(p, name, mode, fmt, ctx, marshalFn, unmarshalFn, userEx, args)
    {
        if(unmarshalFn !== null || userEx.length > 0)
        {
            p._checkAsyncTwowayOnly(name);
        }

        const r = new OutgoingAsync(p, name,
            res =>
            {
                this._completed(res, unmarshalFn, userEx);
            });

        try
        {
            r.prepare(name, mode, ctx);
            if(marshalFn === null)
            {
                r.writeEmptyParams();
            }
            else
            {
                const ostr = r.startWriteParams(fmt);
                marshalFn(ostr, args);
                r.endWriteParams();
            }
            r.invoke();
        }
        catch(ex)
        {
            r.abort(ex);
        }
        return r;
    }

    //
    // Handles the completion of an invocation.
    //
    static _completed(r, unmarshalFn, userEx)
    {
        if(!this._check(r, userEx))
        {
            return;
        }

        try
        {
            if(unmarshalFn === null)
            {
                r.readEmptyParams();
                r.resolve();
            }
            else
            {
                r.resolve(unmarshalFn(r));
            }
        }
        catch(ex)
        {
            this.dispatchLocalException(r, ex);
        }
    }

    //
    // Handles user exceptions.
    //
    static _check(r, uex)
    {
        //
        // If uex is non-null, it must be an array of exception types.
        //
        try
        {
            r.throwUserException();
        }
        catch(ex)
        {
            if(ex instanceof Ice.UserException)
            {
                if(uex !== null)
                {
                    for(let i = 0; i < uex.length; ++i)
                    {
                        if(ex instanceof uex[i])
                        {
                            r.reject(ex);
                            return false;
                        }
                    }
                }
                r.reject(new Ice.UnknownUserException(ex.ice_id()));
                return false;
            }
            else
            {
                r.reject(ex);
                return false;
            }
        }

        return true;
    }

    static dispatchLocalException(r, ex)
    {
        r.reject(ex);
    }

    static checkedCast(prx, facet, ctx)
    {
        let r = null;

        if(prx === undefined || prx === null)
        {
            r = new AsyncResultBase(null, "checkedCast", null, null, null);
            r.resolve(null);
        }
        else
        {
            if(facet !== undefined)
            {
                prx = prx.ice_facet(facet);
            }

            r = new AsyncResultBase(prx.ice_getCommunicator(), "checkedCast", null, prx, null);
            prx.ice_isA(this.ice_staticId(), ctx).then(
                ret =>
                {
                    if(ret)
                    {
                        const h = new this();
                        h._copyFrom(prx);
                        r.resolve(h);
                    }
                    else
                    {
                        r.resolve(null);
                    }
                }).catch(
                    ex =>
                    {
                        if(ex instanceof Ice.FacetNotExistException)
                        {
                            r.resolve(null);
                        }
                        else
                        {
                            r.reject(ex);
                        }
                    });
        }

        return r;
    }

    static uncheckedCast(prx, facet)
    {
        let r = null;
        if(prx !== undefined && prx !== null)
        {
            r = new this();
            if(facet !== undefined)
            {
                prx = prx.ice_facet(facet);
            }
            r._copyFrom(prx);
        }
        return r;
    }

    static get minWireSize()
    {
        return 2;
    }

    static write(os, v)
    {
        os.writeProxy(v);
    }

    static read(is)
    {
        return is.readProxy(this);
    }

    static writeOptional(os, tag, v)
    {
        os.writeOptionalProxy(tag, v);
    }

    static readOptional(is, tag)
    {
        return is.readOptionalProxy(tag, this);
    }

    static _instanceof(T)
    {
        if(T === this)
        {
            return true;
        }

        for(const i in this._implements)
        {
            if(this._implements[i]._instanceof(T))
            {
                return true;
            }
        }

        return false;
    }

    static ice_staticId()
    {
        return this._id;
    }

    static get _implements()
    {
        return [];
    }
}

Ice.ObjectPrx = ObjectPrx;
module.exports.Ice = Ice;
