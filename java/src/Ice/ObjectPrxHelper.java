// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public class ObjectPrxHelper implements ObjectPrx
{
    public final int
    hashCode()
    {
        return _reference.hashValue;
    }

    public final int
    ice_hash()
    {
        return _reference.hashValue;
    }

    public final boolean
    ice_isA(String __id)
    {
        return ice_isA(__id, null);
    }

    public final boolean
    ice_isA(String __id, java.util.Map __context)
    {
        int __cnt = 0;
        while(true)
        {
            try
            {
                _ObjectDel __del = __getDelegate();
                return __del.ice_isA(__id, __context);
            }
            catch(LocationForward __ex)
            {
                __locationForward(__ex);
            }
            catch(IceInternal.NonRepeatable __ex)
            {
                __cnt = __handleException(__ex.get(), __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__ex, __cnt);
            }
        }
    }

    public final void
    ice_ping()
    {
        ice_ping(null);
    }

    public final void
    ice_ping(java.util.Map __context)
    {
        int __cnt = 0;
        while(true)
        {
            try
            {
                _ObjectDel __del = __getDelegate();
                __del.ice_ping(__context);
                return;
            }
            catch(LocationForward __ex)
            {
                __locationForward(__ex);
            }
            catch(IceInternal.NonRepeatable __ex)
            {
                __cnt = __handleException(__ex.get(), __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__ex, __cnt);
            }
        }
    }

    public String[]
    ice_ids()
    {
        return ice_ids(null);
    }

    public String[]
    ice_ids(java.util.Map __context)
    {
        int __cnt = 0;
        while(true)
        {
            try
            {
                _ObjectDel __del = __getDelegate();
                return __del.ice_ids(__context);
            }
            catch(LocationForward __ex)
            {
                __locationForward(__ex);
            }
            catch(IceInternal.NonRepeatable __ex)
            {
                __cnt = __handleException(__ex.get(), __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__ex, __cnt);
            }
        }
    }

    public String
    ice_id()
    {
        return ice_id(null);
    }

    public String
    ice_id(java.util.Map __context)
    {
        int __cnt = 0;
        while(true)
        {
            try
            {
                _ObjectDel __del = __getDelegate();
                return __del.ice_id(__context);
            }
            catch(LocationForward __ex)
            {
                __locationForward(__ex);
            }
            catch(IceInternal.NonRepeatable __ex)
            {
                __cnt = __handleException(__ex.get(), __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__ex, __cnt);
            }
        }
    }

    public String[]
    ice_facets()
    {
        return ice_facets(null);
    }

    public String[]
    ice_facets(java.util.Map __context)
    {
        int __cnt = 0;
        while(true)
        {
            try
            {
                _ObjectDel __del = __getDelegate();
                return __del.ice_facets(__context);
            }
            catch(LocationForward __ex)
            {
                __locationForward(__ex);
            }
            catch(IceInternal.NonRepeatable __ex)
            {
                __cnt = __handleException(__ex.get(), __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__ex, __cnt);
            }
        }
    }

    public final boolean
    ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams)
    {
        return ice_invoke(operation, mode, inParams, outParams, null);
    }

    public final boolean
    ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams,
               java.util.Map __context)
    {
        int __cnt = 0;
        while(true)
        {
            try
            {
                _ObjectDel __del = __getDelegate();
                return __del.ice_invoke(operation, mode, inParams, outParams, __context);
            }
            catch(LocationForward __ex)
            {
                __locationForward(__ex);
            }
            catch(IceInternal.NonRepeatable __ex)
            {
                if(mode == OperationMode.Nonmutating || mode == OperationMode.Idempotent)
                {
                    __cnt = __handleException(__ex.get(), __cnt);
                }
                else
                {
                    __rethrowException(__ex.get());
                }
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__ex, __cnt);
            }
        }
    }

    public final Identity
    ice_getIdentity()
    {
        return _reference.identity;
    }

    public final ObjectPrx
    ice_newIdentity(Identity newIdentity)
    {
        if(newIdentity.equals(_reference.identity))
        {
            return this;
        }
        else
        {
            ObjectPrxHelper proxy = new ObjectPrxHelper();
            proxy.setup(_reference.changeIdentity(newIdentity));
            return proxy;
        }
    }

    public final String[]
    ice_getFacet()
    {
        return _reference.facet;
    }

    public final ObjectPrx
    ice_newFacet(String[] newFacet)
    {
        if(newFacet.equals(_reference.facet))
        {
            return this;
        }
        else
        {
            ObjectPrxHelper proxy = new ObjectPrxHelper();
            proxy.setup(_reference.changeFacet(newFacet));
            return proxy;
        }
    }

    public final ObjectPrx
    ice_appendFacet(String f)
    {
	String[] newFacet = new String[_reference.facet.length + 1];
	System.arraycopy(_reference.facet, 0, newFacet, 0, _reference.facet.length);
	newFacet[newFacet.length - 1] = f;
	ObjectPrxHelper proxy = new ObjectPrxHelper();
	proxy.setup(_reference.changeFacet(newFacet));
	return proxy;
    }

    public final ObjectPrx
    ice_twoway()
    {
        IceInternal.Reference ref = _reference.changeMode(IceInternal.Reference.ModeTwoway);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelper proxy = new ObjectPrxHelper();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final ObjectPrx
    ice_oneway()
    {
        IceInternal.Reference ref = _reference.changeMode(IceInternal.Reference.ModeOneway);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelper proxy = new ObjectPrxHelper();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final ObjectPrx
    ice_batchOneway()
    {
        IceInternal.Reference ref = _reference.changeMode(IceInternal.Reference.ModeBatchOneway);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelper proxy = new ObjectPrxHelper();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final ObjectPrx
    ice_datagram()
    {
        IceInternal.Reference ref = _reference.changeMode(IceInternal.Reference.ModeDatagram);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelper proxy = new ObjectPrxHelper();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final ObjectPrx
    ice_batchDatagram()
    {
        IceInternal.Reference ref = _reference.changeMode(IceInternal.Reference.ModeBatchDatagram);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelper proxy = new ObjectPrxHelper();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final ObjectPrx
    ice_secure(boolean b)
    {
        IceInternal.Reference ref = _reference.changeSecure(b);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelper proxy = new ObjectPrxHelper();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final ObjectPrx
    ice_timeout(int t)
    {
        IceInternal.Reference ref = _reference.changeTimeout(t);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelper proxy = new ObjectPrxHelper();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final ObjectPrx
    ice_router(Ice.RouterPrx router)
    {
        IceInternal.Reference ref = _reference.changeRouter(router);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelper proxy = new ObjectPrxHelper();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final ObjectPrx
    ice_locator(Ice.LocatorPrx locator)
    {
        IceInternal.Reference ref = _reference.changeLocator(locator);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelper proxy = new ObjectPrxHelper();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final ObjectPrx
    ice_default()
    {
        IceInternal.Reference ref = _reference.changeDefault();
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelper proxy = new ObjectPrxHelper();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final void
    ice_flush()
    {
        _ObjectDel __del = __getDelegate();
        __del.ice_flush();
    }

    public final boolean
    equals(java.lang.Object r)
    {
        ObjectPrxHelper rhs = (ObjectPrxHelper)r;
        return _reference.equals(rhs._reference);
    }

    public final IceInternal.Reference
    __reference()
    {
        return _reference;
    }

    public final void
    __copyFrom(ObjectPrx from)
    {
        ObjectPrxHelper h = (ObjectPrxHelper)from;
        IceInternal.Reference ref = null;
        _ObjectDelM delegateM = null;
        _ObjectDelD delegateD = null;

        synchronized(from)
        {
            ref = h._reference;
            try
            {
                delegateM = (_ObjectDelM)h._delegate;
            }
            catch(ClassCastException ex)
            {
            }
            try
            {
                delegateD = (_ObjectDelD)h._delegate;
            }
            catch(ClassCastException ex)
            {
            }
        }

        //
        // No need to synchronize "*this", as this operation is only
        // called upon initialization.
        //

        _reference = ref;

        if(delegateD != null)
        {
            _ObjectDelD delegate = __createDelegateD();
            delegate.__copyFrom(delegateD);
            _delegate = delegate;
        }
        else if(delegateM != null)
        {
            _ObjectDelM delegate = __createDelegateM();
            delegate.__copyFrom(delegateM);
            _delegate = delegate;
        }
    }

    public final int
    __handleException(LocalException ex, int cnt)
    {
	//
	// Only _delegate needs to be mutex protected here.
	//
	synchronized(this)
	{
	    _delegate = null;
	}

        try
        {
            throw ex;
        }
        catch(CloseConnectionException e)
        {
            //
            // We always retry on a close connection exception, as this
            // indicates graceful server shutdown.
            //
        }
        catch(SocketException e)
        {
            ++cnt;
        }
        catch(DNSException e)
        {
            ++cnt;
        }
        catch(TimeoutException e)
        {
            ++cnt;
        }

        IceInternal.TraceLevels traceLevels = _reference.instance.traceLevels();
        Logger logger = _reference.instance.logger();

	int[] retryIntervals = _reference.instance.proxyFactory().getRetryIntervals();

        if(cnt > retryIntervals.length)
        {
            if(traceLevels.retry >= 1)
            {
                String s = "cannot retry operation call because retry limit has been exceeded\n" + ex.toString();
                logger.trace(traceLevels.retryCat, s);
            }
            throw ex;
        }

        if(traceLevels.retry >= 1)
        {
            String s = "re-trying operation call";
	    if(cnt > 0 && retryIntervals[cnt - 1] > 0)
	    {
		s += " in " + retryIntervals[cnt - 1] + "ms";
	    }
	    s += " because of exception\n" + ex;
            logger.trace(traceLevels.retryCat, s);
        }

	if(cnt > 0)
	{
	    //
	    // Sleep before retrying.
	    //
	    try
	    {
		Thread.currentThread().sleep(retryIntervals[cnt - 1]);
	    }
	    catch(InterruptedException ex1)
	    {
	    }
	}

	return cnt;
    }

    public final synchronized void
    __rethrowException(LocalException ex)
    {
        _delegate = null;

        throw ex;
    }

    public final synchronized void
    __locationForward(LocationForward ex)
    {
        _delegate = null;

        ObjectPrxHelper h = (ObjectPrxHelper)ex._prx;
        if(!_reference.identity.equals(h.__reference().identity))
        {
            throw new LocationForwardIdentityException();
        }

	//
	// TODO: BENOIT: This is not thread-safe. Everywhere else in the
	// code, _reference is considered immutable and is not mutex
	// protected.
	//
        _reference = _reference.changeAdapterId(h.__reference().adapterId);
        _reference = _reference.changeEndpoints(h.__reference().endpoints);

        /*
        IceInternal.TraceLevels traceLevels =
            _reference.instance.traceLevels();
        Logger logger = _reference.instance.logger();

        if(traceLevels.locationForward >= 1)
        {
            String s = "location forward for object with identity `" + _reference.identity + "'";
            logger.trace(traceLevels.locationForwardCat, s);
        }
        */
    }

    public final synchronized _ObjectDel
    __getDelegate()
    {
        if(_delegate == null)
        {
            ObjectAdapter adapter = _reference.instance.objectAdapterFactory().findObjectAdapter(this);
            if(adapter != null)
            {
                _ObjectDelD delegate = __createDelegateD();
                delegate.setup(_reference, adapter);
                _delegate = delegate;
            }
            else
            {
                _ObjectDelM delegate = __createDelegateM();
                delegate.setup(_reference);
                _delegate = delegate;

                //
                // If this proxy is for a non-local object, and we are
                // using a router, then add this proxy to the router info
                // object.
                //
                if(_reference.routerInfo != null)
                {
                    _reference.routerInfo.addProxy(this);
                }
            }
        }

        return _delegate;
    }

    protected _ObjectDelM
    __createDelegateM()
    {
        return new _ObjectDelM();
    }

    protected _ObjectDelD
    __createDelegateD()
    {
        return new _ObjectDelD();
    }

    //
    // Only for use by IceInternal.ProxyFactory
    //
    public final void
    setup(IceInternal.Reference ref)
    {
        //
        // No need to synchronize, as this operation is only called
        // upon initial initialization.
        //
        _reference = ref;
    }

    private IceInternal.Reference _reference;
    private _ObjectDel _delegate;
}
