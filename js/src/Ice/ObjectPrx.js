// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/ArrayUtil",
        "../Ice/AsyncResult",
        "../Ice/Debug",
        "../Ice/FormatType",
        "../Ice/HashMap",
        "../Ice/OutgoingAsync",
        "../Ice/ReferenceMode",
        "../Ice/Current",
        "../Ice/Exception",
        "../Ice/BuiltinSequences",
        "../Ice/LocalException",
        "../Ice/Object"
    ]);

var ArrayUtil = Ice.ArrayUtil;
var AsyncResultBase = Ice.AsyncResultBase;
var AsyncResult = Ice.AsyncResult;
var Debug = Ice.Debug;
var FormatType = Ice.FormatType;
var HashMap = Ice.HashMap;
var OutgoingAsync = Ice.OutgoingAsync;
var ProxyFlushBatch = Ice.ProxyFlushBatch;
var ProxyGetConnection = Ice.ProxyGetConnection;
var RefMode = Ice.ReferenceMode;
var OperationMode = Ice.OperationMode;

//
// Ice.ObjectPrx
//
var ObjectPrx = Ice.Class({
    __init__: function()
    {
        this._reference = null;
        this._requestHandler = null;
    },
    hashCode: function(r)
    {
        return this._reference.hashCode();
    },
    ice_getCommunicator: function()
    {
        return this._reference.getCommunicator();
    },
    toString: function()
    {
        return this._reference.toString();
    },
    ice_getIdentity: function()
    {
        return this._reference.getIdentity().clone();
    },
    ice_identity: function(newIdentity)
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
            var proxy = new ObjectPrx();
            proxy.__setup(this._reference.changeIdentity(newIdentity));
            return proxy;
        }
    },
    ice_getContext: function()
    {
        return new HashMap(this._reference.getContext());
    },
    ice_context: function(newContext)
    {
        return this.__newInstance(this._reference.changeContext(newContext));
    },
    ice_getFacet: function()
    {
        return this._reference.getFacet();
    },
    ice_facet: function(newFacet)
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
            var proxy = new ObjectPrx();
            proxy.__setup(this._reference.changeFacet(newFacet));
            return proxy;
        }
    },
    ice_getAdapterId: function()
    {
        return this._reference.getAdapterId();
    },
    ice_adapterId: function(newAdapterId)
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
    },
    ice_getEndpoints: function()
    {
        return ArrayUtil.clone(this._reference.getEndpoints());
    },
    ice_endpoints: function(newEndpoints)
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
    },
    ice_getLocatorCacheTimeout: function()
    {
        return this._reference.getLocatorCacheTimeout();
    },
    ice_locatorCacheTimeout: function(newTimeout)
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
    },
    ice_getInvocationTimeout: function()
    {
        return this._reference.getInvocationTimeout();
    },
    ice_invocationTimeout: function(newTimeout)
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
    },
    ice_isConnectionCached: function()
    {
        return this._reference.getCacheConnection();
    },
    ice_connectionCached: function(newCache)
    {
        if(newCache === this._reference.getCacheConnection())
        {
            return this;
        }
        else
        {
            return this.__newInstance(this._reference.changeCacheConnection(newCache));
        }
    },
    ice_getEndpointSelection: function()
    {
        return this._reference.getEndpointSelection();
    },
    ice_endpointSelection: function(newType)
    {
        if(newType === this._reference.getEndpointSelection())
        {
            return this;
        }
        else
        {
            return this.__newInstance(this._reference.changeEndpointSelection(newType));
        }
    },
    ice_isSecure: function()
    {
        return this._reference.getSecure();
    },
    ice_secure: function(b)
    {
        if(b === this._reference.getSecure())
        {
            return this;
        }
        else
        {
            return this.__newInstance(this._reference.changeSecure(b));
        }
    },
    ice_getEncodingVersion: function()
    {
        return this._reference.getEncoding().clone();
    },
    ice_encodingVersion: function(e)
    {
        if(e.equals(this._reference.getEncoding()))
        {
            return this;
        }
        else
        {
            return this.__newInstance(this._reference.changeEncoding(e));
        }
    },
    ice_isPreferSecure: function()
    {
        return this._reference.getPreferSecure();
    },
    ice_preferSecure: function(b)
    {
        if(b === this._reference.getPreferSecure())
        {
            return this;
        }
        else
        {
            return this.__newInstance(this._reference.changePreferSecure(b));
        }
    },
    ice_getRouter: function()
    {
        var ri = this._reference.getRouterInfo();
        return ri !== null ? ri.getRouter() : null;
    },
    ice_router: function(router)
    {
        var ref = this._reference.changeRouter(router);
        if(ref.equals(this._reference))
        {
            return this;
        }
        else
        {
            return this.__newInstance(ref);
        }
    },
    ice_getLocator: function()
    {
        var ri = this._reference.getLocatorInfo();
        return ri !== null ? ri.getLocator() : null;
    },
    ice_locator: function(locator)
    {
        var ref = this._reference.changeLocator(locator);
        if(ref.equals(this._reference))
        {
            return this;
        }
        else
        {
            return this.__newInstance(ref);
        }
    },
    ice_isTwoway: function()
    {
        return this._reference.getMode() === RefMode.ModeTwoway;
    },
    ice_twoway: function()
    {
        if(this._reference.getMode() === RefMode.ModeTwoway)
        {
            return this;
        }
        else
        {
            return this.__newInstance(this._reference.changeMode(RefMode.ModeTwoway));
        }
    },
    ice_isOneway: function()
    {
        return this._reference.getMode() === RefMode.ModeOneway;
    },
    ice_oneway: function()
    {
        if(this._reference.getMode() === RefMode.ModeOneway)
        {
            return this;
        }
        else
        {
            return this.__newInstance(this._reference.changeMode(RefMode.ModeOneway));
        }
    },
    ice_isBatchOneway: function()
    {
        return this._reference.getMode() === RefMode.ModeBatchOneway;
    },
    ice_batchOneway: function()
    {
        if(this._reference.getMode() === RefMode.ModeBatchOneway)
        {
            return this;
        }
        else
        {
            return this.__newInstance(this._reference.changeMode(RefMode.ModeBatchOneway));
        }
    },
    ice_isDatagram: function()
    {
        return this._reference.getMode() === RefMode.ModeDatagram;
    },
    ice_datagram: function()
    {
        if(this._reference.getMode() === RefMode.ModeDatagram)
        {
            return this;
        }
        else
        {
            return this.__newInstance(this._reference.changeMode(RefMode.ModeDatagram));
        }
    },
    ice_isBatchDatagram: function()
    {
        return this._reference.getMode() === RefMode.ModeBatchDatagram;
    },
    ice_batchDatagram: function()
    {
        if(this._reference.getMode() === RefMode.ModeBatchDatagram)
        {
            return this;
        }
        else
        {
            return this.__newInstance(this._reference.changeMode(RefMode.ModeBatchDatagram));
        }
    },
    ice_compress: function(co)
    {
        var ref = this._reference.changeCompress(co);
        if(ref.equals(this._reference))
        {
            return this;
        }
        else
        {
            return this.__newInstance(ref);
        }
    },
    ice_timeout: function(t)
    {
        if(t < 1 && t !== -1)
        {
            throw new Error("invalid value passed to ice_timeout: " + t);
        }
        var ref = this._reference.changeTimeout(t);
        if(ref.equals(this._reference))
        {
            return this;
        }
        else
        {
            return this.__newInstance(ref);
        }
    },
    ice_getConnectionId: function()
    {
        return this._reference.getConnectionId();
    },
    ice_connectionId: function(id)
    {
        var ref = this._reference.changeConnectionId(id);
        if(ref.equals(this._reference))
        {
            return this;
        }
        else
        {
            return this.__newInstance(ref);
        }
    },
    ice_getConnection: function()
    {
        var r = new ProxyGetConnection(this, "ice_getConnection");
        try
        {
            r.__invoke();
        }
        catch(ex)
        {
            r.__abort(ex);
        }
        return r;
    },
    ice_getCachedConnection: function()
    {
        return this._requestHandler ? this._requestHandler.getConnection() : null;
    },
    ice_flushBatchRequests: function()
    {
        var r = new ProxyFlushBatch(this, "ice_flushBatchRequests");
        try
        {
            r.__invoke();
        }
        catch(ex)
        {
            r.__abort(ex);
        }
        return r;
    },
    equals: function(r)
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
    },
    __write: function(os)
    {
        this._reference.getIdentity().__write(os);
        this._reference.streamWrite(os);
    },
    __reference: function()
    {
        return this._reference;
    },
    __copyFrom: function(from)
    {
        Debug.assert(this._reference === null);
        Debug.assert(this._requestHandler === null);

        this._reference = from._reference;
        this._requestHandler = from._requestHandler;
    },
    __handleException: function(ex, handler, mode, sent, sleep, cnt)
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
    },
    __checkAsyncTwowayOnly: function(name)
    {
        if(!this.ice_isTwoway())
        {
            throw new Error("`" + name + "' can only be called with a twoway proxy");
        }
    },
    __getRequestHandler: function()
    {
        if(this._reference.getCacheConnection())
        {
            if(this._requestHandler)
            {
                return this._requestHandler;
            }
        }
        return this._reference.getRequestHandler(this);
    },
    __getBatchRequestQueue: function()
    {
        if(!this._batchRequestQueue)
        {
            this._batchRequestQueue = this._reference.getBatchRequestQueue();
        }
        return this._batchRequestQueue;
    },
    __setRequestHandler: function(handler)
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
    },
    __updateRequestHandler: function(previous, handler)
    {
        if(this._reference.getCacheConnection() && previous !== null)
        {
            if(this._requestHandler && this._requestHandler !== handler)
            {
                this._requestHandler = this._requestHandler.update(previous, handler);
            }
        }
    },
    //
    // Only for use by IceInternal.ProxyFactory
    //
    __setup: function(ref)
    {
        Debug.assert(this._reference === null);

        this._reference = ref;
    },
    __newInstance: function(ref)
    {
        var proxy = new this.constructor();
        proxy.__setup(ref);
        return proxy;
    },
    ice_instanceof: function(T)
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
});

//
// Generic invocation for operations that have input parameters.
//
ObjectPrx.__invoke = function(p, name, mode, fmt, ctx, marshalFn, unmarshalFn, userEx, args)
{
    if(unmarshalFn !== null || userEx.length > 0)
    {
        p.__checkAsyncTwowayOnly(name);
    }

    var __r = new OutgoingAsync(p, name,
        function(__res)
        {
            ObjectPrx.__completed(__res, unmarshalFn, userEx);
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
            var __os = __r.__startWriteParams(fmt);
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
};

//
// Handles the completion of an invocation.
//
ObjectPrx.__completed = function(__r, unmarshalFn, userEx)
{
    if(!ObjectPrx.__check(__r, userEx))
    {
        return;
    }

    try
    {
        if(unmarshalFn === null)
        {
            __r.__readEmptyParams();
            __r.succeed(__r);
        }
        else
        {
            var results = unmarshalFn(__r);
            __r.succeed.apply(__r, results);
        }
    }
    catch(ex)
    {
        ObjectPrx.__dispatchLocalException(__r, ex);
        return;
    }
};

//
// Unmarshal callback for operations that return a bool as the only result.
//
ObjectPrx.__returns_bool = function(__is, __results)
{
    __results.push(__is.readBool());
};

//
// Unmarshal callback for operations that return a byte as the only result.
//
ObjectPrx.__returns_byte = function(__is, __results)
{
    __results.push(__is.readByte());
};

//
// Unmarshal callback for operations that return a short as the only result.
//
ObjectPrx.__returns_short = function(__is, __results)
{
    __results.push(__is.readShort());
};

//
// Unmarshal callback for operations that return an int as the only result.
//
ObjectPrx.__returns_int = function(__is, __results)
{
    __results.push(__is.readInt());
};

//
// Unmarshal callback for operations that return a long as the only result.
//
ObjectPrx.__returns_long = function(__is, __results)
{
    __results.push(__is.readLong());
};

//
// Unmarshal callback for operations that return a float as the only result.
//
ObjectPrx.__returns_float = function(__is, __results)
{
    __results.push(__is.readFloat());
};

//
// Unmarshal callback for operations that return a double as the only result.
//
ObjectPrx.__returns_double = function(__is, __results)
{
    __results.push(__is.readDouble());
};

//
// Unmarshal callback for operations that return a string as the only result.
//
ObjectPrx.__returns_string = function(__is, __results)
{
    __results.push(__is.readString());
};

//
// Unmarshal callback for operations that return a proxy as the only result.
//
ObjectPrx.__returns_ObjectPrx = function(__is, __results)
{
    __results.push(__is.readProxy());
};

//
// Unmarshal callback for operations that return an object as the only result.
//
ObjectPrx.__returns_Object = function(__is, __results)
{
    __is.readValue(function(obj){ __results.push(obj); }, Ice.Object);
    __is.readPendingValues();
};

//
// Handles user exceptions.
//
ObjectPrx.__check = function(__r, __uex)
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
                for(var i = 0; i < __uex.length; ++i)
                {
                    if(ex instanceof __uex[i])
                    {
                        __r.fail(ex, __r);
                        return false;
                    }
                }
            }
            __r.fail(new Ice.UnknownUserException(ex.ice_name()), __r);
            return false;
        }
        else
        {
            __r.fail(ex, __r);
            return false;
        }
    }

    return true;
};

ObjectPrx.__dispatchLocalException = function(__r, __ex)
{
    __r.fail(__ex, __r);
};

ObjectPrx.ice_staticId = Ice.Object.ice_staticId;

ObjectPrx.checkedCast = function(prx, facet, ctx)
{
    var __r = null;

    if(prx === undefined || prx === null)
    {
        __r = new AsyncResultBase(null, "checkedCast", null, null, null);
        __r.succeed(null, __r);
    }
    else
    {
        if(facet !== undefined)
        {
            prx = prx.ice_facet(facet);
        }

        var self = this;
        __r = new AsyncResultBase(prx.ice_getCommunicator(), "checkedCast", null, prx, null);
        prx.ice_isA(this.ice_staticId(), ctx).then(
            function(__res, __ret)
            {
                if(__ret)
                {
                    var __h = new self();
                    __h.__copyFrom(prx);
                    __r.succeed(__h, __r);
                }
                else
                {
                    __r.succeed(null, __r);
                }
            }).exception(
                function(__ex)
                {
                    if(__ex instanceof Ice.FacetNotExistException)
                    {
                        __r.succeed(null, __r);
                    }
                    else
                    {
                        __r.fail(__ex, __r);
                    }
                });
    }

    return __r;
};

ObjectPrx.uncheckedCast = function(prx, facet)
{
    var r = null;
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
};

Object.defineProperty(ObjectPrx, "minWireSize", {
    get: function(){ return 2; }
});

ObjectPrx.write = function(os, v)
{
    os.writeProxy(v);
};

ObjectPrx.read = function(is)
{
    return is.readProxy(this);
};

ObjectPrx.writeOptional = function(os, tag, v)
{
    os.writeOptionalProxy(tag, v);
};

ObjectPrx.readOptional = function(is, tag)
{
    return is.readOptionalProxy(tag, this);
};

ObjectPrx.__instanceof = function(T)
{
    if(T === this)
    {
        return true;
    }

    for(var i in this.__implements)
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
};

var Slice = Ice.Slice;
Slice.defineProxy = function(base, staticId, prxInterfaces)
{
    var prx = function()
    {
        base.call(this);
    };
    prx.__parent = base;
    prx.__implements = prxInterfaces;

    // All generated proxies inherit from ObjectPrx
    prx.prototype = new base();
    prx.prototype.constructor = prx;

    // Static methods
    prx.ice_staticId = staticId;

    // Copy static methods inherited from ObjectPrx
    prx.checkedCast = ObjectPrx.checkedCast;
    prx.uncheckedCast = ObjectPrx.uncheckedCast;
    prx.write = ObjectPrx.write;
    prx.writeOptional = ObjectPrx.writeOptional;
    prx.read = ObjectPrx.read;
    prx.readOptional = ObjectPrx.readOptional;

    prx.__instanceof = ObjectPrx.__instanceof;

    // Static properties
    Object.defineProperty(prx, "minWireSize", {
        get: function(){ return 2; }
    });

    return prx;
};

Ice.ObjectPrx = ObjectPrx;
module.exports.Ice = Ice;
