// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/ArrayUtil",
        "../Ice/AsyncResult",
        "../Ice/Debug",
        "../Ice/FormatType",
        "../Ice/OutgoingAsync",
        "../Ice/ReferenceMode",
        "../Ice/Current",
        "../Ice/Exception",
        "../Ice/BuiltinSequences",
        "../Ice/LocalException",
        "../Ice/Object"
    ]);

const ArrayUtil = Ice.ArrayUtil;
const AsyncResultBase = Ice.AsyncResultBase;
const AsyncResult = Ice.AsyncResult;
const Debug = Ice.Debug;
const FormatType = Ice.FormatType;
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
            proxy.__setup(this._reference.changeIdentity(newIdentity));
            return proxy;
        }
    }

    ice_getContext()
    {
        return new Map(this._reference.getContext());
    }

    ice_context(newContext)
    {
        return this.__newInstance(this._reference.changeContext(newContext));
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
            proxy.__setup(this._reference.changeFacet(newFacet));
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
            return this.__newInstance(this._reference.changeAdapterId(newAdapterId));
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
            return this.__newInstance(this._reference.changeEndpoints(newEndpoints));
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
            throw new Error("invalid value passed to ice_locatorCacheTimeout: " + newTimeout);
        }
        if(newTimeout === this._reference.getLocatorCacheTimeout())
        {
            return this;
        }
        else
        {
            return this.__newInstance(this._reference.changeLocatorCacheTimeout(newTimeout));
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
            throw new Error("invalid value passed to ice_invocationTimeout: " + newTimeout);
        }
        if(newTimeout === this._reference.getInvocationTimeout())
        {
            return this;
        }
        else
        {
            return this.__newInstance(this._reference.changeInvocationTimeout(newTimeout));
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
            return this.__newInstance(this._reference.changeCacheConnection(newCache));
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
            return this.__newInstance(this._reference.changeEndpointSelection(newType));
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
            return this.__newInstance(this._reference.changeSecure(b));
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
            return this.__newInstance(this._reference.changeEncoding(e));
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
            return this.__newInstance(this._reference.changePreferSecure(b));
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
            return this.__newInstance(ref);
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
            return this.__newInstance(ref);
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
            return this.__newInstance(this._reference.changeMode(RefMode.ModeTwoway));
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
            return this.__newInstance(this._reference.changeMode(RefMode.ModeOneway));
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
            return this.__newInstance(this._reference.changeMode(RefMode.ModeBatchOneway));
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
            return this.__newInstance(this._reference.changeMode(RefMode.ModeDatagram));
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
            return this.__newInstance(this._reference.changeMode(RefMode.ModeBatchDatagram));
        }
    }

    ice_compress(co)
    {
        const ref = this._reference.changeCompress(co);
        if(ref.equals(this._reference))
        {
            return this;
        }
        else
        {
            return this.__newInstance(ref);
        }
    }

    ice_timeout(t)
    {
        if(t < 1 && t !== -1)
        {
            throw new Error("invalid value passed to ice_timeout: " + t);
        }
        const ref = this._reference.changeTimeout(t);
        if(ref.equals(this._reference))
        {
            return this;
        }
        else
        {
            return this.__newInstance(ref);
        }
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
            return this.__newInstance(ref);
        }
    }

    ice_getConnection()
    {
        const r = new ProxyGetConnection(this, "ice_getConnection");
        try
        {
            r.__invoke();
        }
        catch(ex)
        {
            r.__abort(ex);
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
            r.__invoke();
        }
        catch(ex)
        {
            r.__abort(ex);
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

    __write(os)
    {
        this._reference.getIdentity().__write(os);
        this._reference.streamWrite(os);
    }

    __reference()
    {
        return this._reference;
    }

    __copyFrom(from)
    {
        Debug.assert(this._reference === null);
        Debug.assert(this._requestHandler === null);

        this._reference = from._reference;
        this._requestHandler = from._requestHandler;
    }

    __handleException(ex, handler, mode, sent, sleep, cnt)
    {
        this.__updateRequestHandler(handler, null); // Clear the request handler

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

    __checkAsyncTwowayOnly(name)
    {
        if(!this.ice_isTwoway())
        {
            throw new Error("`" + name + "' can only be called with a twoway proxy");
        }
    }

    __getRequestHandler()
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

    __getBatchRequestQueue()
    {
        if(!this._batchRequestQueue)
        {
            this._batchRequestQueue = this._reference.getBatchRequestQueue();
        }
        return this._batchRequestQueue;
    }

    __setRequestHandler(handler)
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

    __updateRequestHandler(previous, handler)
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
    __setup(ref)
    {
        Debug.assert(this._reference === null);

        this._reference = ref;
    }

    __newInstance(ref)
    {
        const proxy = new this.constructor();
        proxy.__setup(ref);
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
            return this.constructor.__instanceof(T);
        }
        return false;
    }
    
    //
    // Generic invocation for operations that have input parameters.
    //
    static __invoke(p, name, mode, fmt, ctx, marshalFn, unmarshalFn, userEx, args)
    {
        if(unmarshalFn !== null || userEx.length > 0)
        {
            p.__checkAsyncTwowayOnly(name);
        }

        const __r = new OutgoingAsync(p, name,
            __res =>
            {
                this.__completed(__res, unmarshalFn, userEx);
            });

        try
        {
            __r.__prepare(name, mode, ctx);
            if(marshalFn === null)
            {
                __r.__writeEmptyParams();
            }
            else
            {
                const __os = __r.__startWriteParams(fmt);
                marshalFn.call(null, __os, args);
                __r.__endWriteParams();
            }
            __r.__invoke();
        }
        catch(ex)
        {
            __r.__abort(ex);
        }
        return __r;
    }

    //
    // Handles the completion of an invocation.
    //
    static __completed(__r, unmarshalFn, userEx)
    {
        if(!this.__check(__r, userEx))
        {
            return;
        }

        try
        {
            if(unmarshalFn === null)
            {
                __r.__readEmptyParams();
                __r.resolve();
            }
            else
            {
                __r.resolve(unmarshalFn(__r));
            }
        }
        catch(ex)
        {
            this.__dispatchLocalException(__r, ex);
            return;
        }
    }

    //
    // Unmarshal callback for operations that return a bool as the only result.
    //
    static __returns_bool(__is, __results)
    {
        __results.push(__is.readBool());
    }

    //
    // Unmarshal callback for operations that return a byte as the only result.
    //
    static __returns_byte(__is, __results)
    {
        __results.push(__is.readByte());
    }

    //
    // Unmarshal callback for operations that return a short as the only result.
    //
    static __returns_short(__is, __results)
    {
        __results.push(__is.readShort());
    }

    //
    // Unmarshal callback for operations that return an int as the only result.
    //
    static __returns_int(__is, __results)
    {
        __results.push(__is.readInt());
    }

    //
    // Unmarshal callback for operations that return a long as the only result.
    //
    static __returns_long(__is, __results)
    {
        __results.push(__is.readLong());
    }

    //
    // Unmarshal callback for operations that return a float as the only result.
    //
    static __returns_float(__is, __results)
    {
        __results.push(__is.readFloat());
    }

    //
    // Unmarshal callback for operations that return a double as the only result.
    //
    static __returns_double(__is, __results)
    {
        __results.push(__is.readDouble());
    }

    //
    // Unmarshal callback for operations that return a string as the only result.
    //
    static __returns_string(__is, __results)
    {
        __results.push(__is.readString());
    }

    //
    // Unmarshal callback for operations that return a proxy as the only result.
    //
    static __returns_ObjectPrx(__is, __results)
    {
        __results.push(__is.readProxy());
    }

    //
    // Unmarshal callback for operations that return an object as the only result.
    //
    static __returns_Object(__is, __results)
    {
        __is.readValue(obj => __results.push(obj), Ice.Object);
        __is.readPendingValues();
    }

    //
    // Handles user exceptions.
    //
    static __check(__r, __uex)
    {
        //
        // If __uex is non-null, it must be an array of exception types.
        //
        try
        {
            __r.__throwUserException();
        }
        catch(ex)
        {
            if(ex instanceof Ice.UserException)
            {
                if(__uex !== null)
                {
                    for(let i = 0; i < __uex.length; ++i)
                    {
                        if(ex instanceof __uex[i])
                        {
                            __r.reject(ex);
                            return false;
                        }
                    }
                }
                __r.reject(new Ice.UnknownUserException(ex.ice_name()));
                return false;
            }
            else
            {
                __r.reject(ex);
                return false;
            }
        }

        return true;
    }

    static __dispatchLocalException(__r, __ex)
    {
        __r.reject(__ex);
    }

    static checkedCast(prx, facet, ctx)
    {
        let __r = null;

        if(prx === undefined || prx === null)
        {
            __r = new AsyncResultBase(null, "checkedCast", null, null, null);
            __r.resolve(null);
        }
        else
        {
            if(facet !== undefined)
            {
                prx = prx.ice_facet(facet);
            }
            __r = new AsyncResultBase(prx.ice_getCommunicator(), "checkedCast", null, prx, null);
            prx.ice_isA(this.ice_staticId(), ctx).then(
                __ret =>
                {
                    if(__ret)
                    {
                        const __h = new this();
                        __h.__copyFrom(prx);
                        __r.resolve(__h);
                    }
                    else
                    {
                        __r.resolve(null);
                    }
                }).catch(
                    __ex =>
                    {
                        if(__ex instanceof Ice.FacetNotExistException)
                        {
                            __r.resolve(null);
                        }
                        else
                        {
                            __r.reject(__ex);
                        }
                    });
        }

        return __r;
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
            r.__copyFrom(prx);
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

    static __instanceof(T)
    {
        if(T === this)
        {
            return true;
        }

        for(let i in this.__implements)
        {
            if(this.__implements[i].__instanceof(T))
            {
                return true;
            }
        }

        if(this.__parent)
        {
            return this.__parent.__instanceof(T);
        }
        return false;
    }
    
    static ice_staticId()
    {
        return "::Ice::Object";
    }
}

Ice.ObjectPrx = ObjectPrx;
module.exports.Ice = Ice;
