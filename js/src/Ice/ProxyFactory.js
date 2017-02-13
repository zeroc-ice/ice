// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/Debug",
        "../Ice/HashMap",
        "../Ice/ObjectPrx",
        "../Ice/StringUtil",
        "../Ice/Identity",
        "../Ice/Reference",
        "../Ice/LocalException"
    ]);

var Debug = Ice.Debug;
var HashMap = Ice.HashMap;
var ObjectPrx = Ice.ObjectPrx;
var StringUtil = Ice.StringUtil;
var Identity = Ice.Identity;

//
// Only for use by Instance.
//
var ProxyFactory = Ice.Class({
    __init__: function(instance)
    {
        this._instance = instance;

        var arr = this._instance.initializationData().properties.getPropertyAsList("Ice.RetryIntervals");

        if(arr.length > 0)
        {
            this._retryIntervals = [];

            for(var i = 0; i < arr.length; i++)
            {
                var v;

                try
                {
                    v = StringUtil.toInt(arr[i]);
                }
                catch(ex)
                {
                    v = 0;
                }

                //
                // If -1 is the first value, no retry and wait intervals.
                //
                if(i === 0 && v === -1)
                {
                    break;
                }

                this._retryIntervals[i] = v > 0 ? v : 0;
            }
        }
        else
        {
            this._retryIntervals = [ 0 ];
        }
    },
    stringToProxy: function(str)
    {
        var ref = this._instance.referenceFactory().createFromString(str, null);
        return this.referenceToProxy(ref);
    },
    proxyToString: function(proxy)
    {
        if(proxy !== null)
        {
            return proxy.__reference().toString();
        }
        else
        {
            return "";
        }
    },
    propertyToProxy: function(prefix)
    {
        var proxy = this._instance.initializationData().properties.getProperty(prefix);
        var ref = this._instance.referenceFactory().createFromString(proxy, prefix);
        return this.referenceToProxy(ref);
    },
    proxyToProperty: function(proxy, prefix)
    {
        if(proxy !== null)
        {
            return proxy.__reference().toProperty(prefix);
        }
        else
        {
            return new HashMap();
        }
    },
    streamToProxy: function(s, type)
    {
        var ident = new Identity();
        ident.__read(s);

        var ref = this._instance.referenceFactory().createFromStream(ident, s);
        return this.referenceToProxy(ref, type);
    },
    referenceToProxy: function(ref, type)
    {
        if(ref !== null)
        {
            var proxy = type ? new type() : new ObjectPrx();
            proxy.__setup(ref);
            return proxy;
        }
        else
        {
            return null;
        }
    },
    proxyToStream: function(proxy, s)
    {
        if(proxy !== null)
        {
            var ref = proxy.__reference();
            ref.getIdentity().__write(s);
            ref.streamWrite(s);
        }
        else
        {
            var ident = new Identity("", "");
            ident.__write(s);
        }
    },
    checkRetryAfterException: function(ex, ref, sleepInterval, cnt)
    {
        var traceLevels = this._instance.traceLevels();
        var logger = this._instance.initializationData().logger;

        //
        // We don't retry batch requests because the exception might have caused
        // the all the requests batched with the connection to be aborted and we
        // want the application to be notified.
        //
        if(ref.getMode() === Ice.Reference.ModeBatchOneway || ref.getMode() === Ice.Reference.ModeBatchDatagram)
        {
            throw ex;
        }

        if(ex instanceof Ice.ObjectNotExistException)
        {
            var one = ex;

            if(ref.getRouterInfo() !== null && one.operation === "ice_add_proxy")
            {
                //
                // If we have a router, an ObjectNotExistException with an
                // operation name "ice_add_proxy" indicates to the client
                // that the router isn't aware of the proxy (for example,
                // because it was evicted by the router). In this case, we
                // must *always* retry, so that the missing proxy is added
                // to the router.
                //

                ref.getRouterInfo().clearCache(ref);

                if(traceLevels.retry >= 1)
                {
                    logger.trace(traceLevels.retryCat, "retrying operation call to add proxy to router\n" +
                                                    ex.toString());
                }

                if(sleepInterval !== null)
                {
                    sleepInterval.value = 0;
                }
                return cnt; // We must always retry, so we don't look at the retry count.
            }
            else if(ref.isIndirect())
            {
                //
                // We retry ObjectNotExistException if the reference is
                // indirect.
                //

                if(ref.isWellKnown())
                {
                    var li = ref.getLocatorInfo();
                    if(li !== null)
                    {
                        li.clearCache(ref);
                    }
                }
            }
            else
            {
                //
                // For all other cases, we don't retry ObjectNotExistException.
                //
                throw ex;
            }
        }
        else if(ex instanceof Ice.RequestFailedException)
        {
            //
            // For all other cases, we don't retry ObjectNotExistException
            //
            throw ex;
        }

        //
        // There is no point in retrying an operation that resulted in a
        // MarshalException. This must have been raised locally (because
        // if it happened in a server it would result in an
        // UnknownLocalException instead), which means there was a problem
        // in this process that will not change if we try again.
        //
        // The most likely cause for a MarshalException is exceeding the
        // maximum message size, which is represented by the the subclass
        // MemoryLimitException. For example, a client can attempt to send
        // a message that exceeds the maximum memory size, or accumulate
        // enough batch requests without flushing that the maximum size is
        // reached.
        //
        // This latter case is especially problematic, because if we were
        // to retry a batch request after a MarshalException, we would in
        // fact silently discard the accumulated requests and allow new
        // batch requests to accumulate. If the subsequent batched
        // requests do not exceed the maximum message size, it appears to
        // the client that all of the batched requests were accepted, when
        // in reality only the last few are actually sent.
        //
        if(ex instanceof Ice.MarshalException)
        {
            throw ex;
        }

        //
        // Don't retry if the communicator is destroyed or object adapter
        // deactivated.
        //
        if(ex instanceof Ice.CommunicatorDestroyedException || ex instanceof Ice.ObjectAdapterDeactivatedException)
        {
            throw ex;
        }

        //
        // Don't retry invocation timeouts.
        //
        if(ex instanceof Ice.InvocationTimeoutException || ex instanceof Ice.InvocationCanceledException)
        {
            throw ex;
        }

        ++cnt;
        Debug.assert(cnt > 0);

        var interval;
        if(cnt === (this._retryIntervals.length + 1) && ex instanceof Ice.CloseConnectionException)
        {
            //
            // A close connection exception is always retried at least once, even if the retry
            // limit is reached.
            //
            interval = 0;
        }
        else if(cnt > this._retryIntervals.length)
        {
            if(traceLevels.retry >= 1)
            {
                logger.trace(traceLevels.retryCat,
			     "cannot retry operation call because retry limit has been exceeded\n" + ex.toString());
            }
            throw ex;
        }
        else
        {
            interval = this._retryIntervals[cnt - 1];
        }

        if(traceLevels.retry >= 1)
        {
            var msg = "retrying operation call";
            if(interval > 0)
            {
                msg += " in " + interval + "ms";
            }
            msg += " because of exception\n" + ex.toString();
            logger.trace(traceLevels.retryCat, msg);
        }

        Debug.assert(sleepInterval !== null);
        sleepInterval.value = interval;

        return cnt;
    }
});

Ice.ProxyFactory = ProxyFactory;
module.exports.Ice = Ice;
