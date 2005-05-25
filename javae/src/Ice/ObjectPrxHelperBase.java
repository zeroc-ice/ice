// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public class ObjectPrxHelperBase implements ObjectPrx
{
    public final int
    hashCode()
    {
        return _reference.hashCode();
    }

    public final int
    ice_hash()
    {
        return _reference.hashCode();
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
	        __checkTwowayOnly("ice_isA");
                _ObjectDel __del = __getDelegate();
                return __del.ice_isA(__id, __context);
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
	        __checkTwowayOnly("ice_ping");
                _ObjectDel __del = __getDelegate();
                __del.ice_ping(__context);
                return;
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
	        __checkTwowayOnly("ice_ids");
                _ObjectDel __del = __getDelegate();
                return __del.ice_ids(__context);
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
	        __checkTwowayOnly("ice_id");
                _ObjectDel __del = __getDelegate();
                return __del.ice_id(__context);
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
               java.util.Map context)
    {
        int __cnt = 0;
        while(true)
        {
            try
            {
                _ObjectDel __del = __getDelegate();
                return __del.ice_invoke(operation, mode, inParams, outParams, context);
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
        return _reference.getIdentity();
    }

    public final ObjectPrx
    ice_newIdentity(Identity newIdentity)
    {
        if(newIdentity.equals(_reference.getIdentity()))
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeIdentity(newIdentity));
            return proxy;
        }
    }

    public final java.util.Map
    ice_getContext()
    {
        return _reference.getContext();
    }

    public final ObjectPrx
    ice_newContext(java.util.Map newContext)
    {
        if(_reference.hasContext() && newContext != null && newContext.equals(_reference.getContext()))
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeContext(newContext));
            return proxy;
        }
    }

    public final ObjectPrx
    ice_defaultContext()
    {
        if(!_reference.hasContext())
	{
	    return this;
	}
	else
	{
	    ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
	    proxy.setup(_reference.defaultContext());
	    return proxy;
	}
    }

    public final String
    ice_getFacet()
    {
        return _reference.getFacet();
    }

    public final ObjectPrx
    ice_newFacet(String newFacet)
    {
        if(newFacet == null)
        {
            newFacet = "";
        }

        if(newFacet.equals(_reference.getFacet()))
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeFacet(newFacet));
            return proxy;
        }
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
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final boolean
    ice_isTwoway()
    {
	return _reference.getMode() == IceInternal.Reference.ModeTwoway;
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
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final boolean
    ice_isOneway()
    {
	return _reference.getMode() == IceInternal.Reference.ModeOneway;
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
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final boolean
    ice_isBatchOneway()
    {
	return _reference.getMode() == IceInternal.Reference.ModeBatchOneway;
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
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
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
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
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
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
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
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final Connection
    ice_connection()
    {
        int __cnt = 0;
        while(true)
        {
            try
            {
                _ObjectDel __del = __getDelegate();
                return __del.ice_connection();
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__ex, __cnt);
            }
        }
    }

    public final boolean
    equals(java.lang.Object r)
    {
        ObjectPrxHelperBase rhs = (ObjectPrxHelperBase)r;
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
        ObjectPrxHelperBase h = (ObjectPrxHelperBase)from;
        IceInternal.Reference ref = null;
        _ObjectDel delegate = null;

        synchronized(from)
        {
            ref = h._reference;
            try
            {
                delegate = (_ObjectDel)h._delegate;
            }
            catch(ClassCastException ex)
            {
            }
        }

        //
        // No need to synchronize "*this", as this operation is only
        // called upon initialization.
        //

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_reference == null);
	    IceUtil.Debug.Assert(_delegate == null);
	}

        _reference = ref;

        if(delegate != null)
        {
            _delegate = __createDelegate();
            _delegate.__copyFrom(delegate);
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

	IceInternal.ProxyFactory proxyFactory = _reference.getInstance().proxyFactory();
	if(proxyFactory != null)
	{
	    return proxyFactory.checkRetryAfterException(ex, _reference, cnt);
	}
	else
	{
	    //
            // The communicator is already destroyed, so we cannot
            // retry.
	    //
	    throw ex;
	}
    }

    public final synchronized void
    __rethrowException(LocalException ex)
    {
        _delegate = null;
        throw ex;
    }

    public final void
    __checkTwowayOnly(String name)
    {
	//
	// No mutex lock necessary, there is nothing mutable in this
	// operation.
	//

        if(!ice_isTwoway())
	{
	    TwowayOnlyException ex = new TwowayOnlyException();
	    ex.operation = name;
	    throw ex;
	}
    }

    public final synchronized _ObjectDel
    __getDelegate()
    {
        if(_delegate == null)
        {
            _ObjectDel delegate = __createDelegate();
            delegate.setup(_reference);
            _delegate = delegate;

            //
            // If this proxy is for a non-local object, and we are
            // using a router, then add this proxy to the router info
            // object.
            //
	    try
	    {
	        IceInternal.RoutableReference rr = (IceInternal.RoutableReference)_reference;
	        if(rr != null && rr.getRouterInfo() != null)
	        {
	            rr.getRouterInfo().addProxy(this);
	        }
	    }
	    catch(ClassCastException e)
	    {
	    }
        }

        return _delegate;
    }

    protected _ObjectDel
    __createDelegate()
    {
        return new _ObjectDel();
    }

    protected java.util.Map
    __defaultContext()
    {
        return _reference.getContext();
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

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_reference == null);
	    IceUtil.Debug.Assert(_delegate == null);
	}

        _reference = ref;
    }

    private IceInternal.Reference _reference;
    private _ObjectDel _delegate;
}
