// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************
namespace Ice
{
	
    using System.Collections;
    using System.Diagnostics;

    public class ObjectPrxHelper : ObjectPrx
    {
	public override int GetHashCode()
	{
	    return _reference.hashValue;
	}
	
	public int ice_hash()
	{
	    return _reference.hashValue;
	}
	
	public bool ice_isA(string __id)
	{
	    return ice_isA(__id, null);
	}
	
	public bool ice_isA(string __id, Context __context)
	{
	    int __cnt = 0;
	    while(true)
	    {
		try
		{
		    __checkTwowayOnly("ice_isA");
		    Object_Del __del = __getDelegate();
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
	
	public void ice_ping()
	{
	    ice_ping(null);
	}
	
	public void ice_ping(Context __context)
	{
	    int __cnt = 0;
	    while(true)
	    {
		try
		{
		    __checkTwowayOnly("ice_ping");
		    Object_Del __del = __getDelegate();
		    __del.ice_ping(__context);
		    return ;
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
	
	public virtual StringSeq ice_ids()
	{
	    return ice_ids(null);
	}
	
	public virtual StringSeq ice_ids(Context __context)
	{
	    int __cnt = 0;
	    while(true)
	    {
		try
		{
		    __checkTwowayOnly("ice_ids");
		    Object_Del __del = __getDelegate();
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
	
	public virtual string ice_id()
	{
	    return ice_id(null);
	}
	
	public virtual string ice_id(Context __context)
	{
	    int __cnt = 0;
	    while(true)
	    {
		try
		{
		    __checkTwowayOnly("ice_id");
		    Object_Del __del = __getDelegate();
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
	
	public virtual FacetPath ice_facets()
	{
	    return ice_facets(null);
	}
	
	public virtual FacetPath ice_facets(Context __context)
	{
	    int __cnt = 0;
	    while(true)
	    {
		try
		{
		    __checkTwowayOnly("ice_facets");
		    Object_Del __del = __getDelegate();
		    return __del.ice_facets(__context);
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
	
	public bool ice_invoke(string operation, OperationMode mode, ByteSeq inParams, out ByteSeq outParams)
	{
	    return ice_invoke(operation, mode, inParams, out outParams, null);
	}
	
	public bool ice_invoke(string operation, OperationMode mode, ByteSeq inParams, out ByteSeq outParams,
	                       Context context)
	{
	    int __cnt = 0;
	    while(true)
	    {
		try
		{
		    Object_Del __del = __getDelegate();
		    return __del.ice_invoke(operation, mode, inParams, out outParams, context);
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
	
	public void ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, ByteSeq inParams)
	{
	    ice_invoke_async(cb, operation, mode, inParams, null);
	}
	
	public void ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, ByteSeq inParams,
			             Context context)
	{
	    int __cnt = 0;
	    while(true)
	    {
		try
		{
		    Object_Del __del = __getDelegate();
		    __del.ice_invoke_async(cb, operation, mode, inParams, context);
		    return ;
		}
		catch(LocalException __ex)
		{
		    __cnt = __handleException(__ex, __cnt);
		}
	    }
	}
	
	public Identity ice_getIdentity()
	{
	    return _reference.identity;
	}
	
	public ObjectPrx ice_newIdentity(Identity newIdentity)
	{
	    if(newIdentity.Equals(_reference.identity))
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
	
	public Context ice_getContext()
	{
	    return _reference.context;
	}
	
	public ObjectPrx ice_newContext(Context newContext)
	{
	    if(newContext.Equals(_reference.context))
	    {
		return this;
	    }
	    else
	    {
		ObjectPrxHelper proxy = new ObjectPrxHelper();
		proxy.setup(_reference.changeContext(newContext));
		return proxy;
	    }
	}
	
	public FacetPath ice_getFacet()
	{
	    return _reference.facet;
	}
	
	public ObjectPrx ice_newFacet(FacetPath newFacet)
	{
	    if(newFacet.Equals(_reference.facet))
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
	
	public ObjectPrx ice_appendFacet(string f)
	{
	    FacetPath newFacet = (FacetPath)_reference.facet.Clone();
	    newFacet.Add(f);
	    ObjectPrxHelper proxy = new ObjectPrxHelper();
	    proxy.setup(_reference.changeFacet(newFacet));
	    return proxy;
	}
	
	public ObjectPrx ice_twoway()
	{
	    IceInternal.Reference rf = _reference.changeMode(IceInternal.Reference.ModeTwoway);
	    if(rf.Equals(_reference))
	    {
		return this;
	    }
	    else
	    {
		ObjectPrxHelper proxy = new ObjectPrxHelper();
		proxy.setup(rf);
		return proxy;
	    }
	}
	
	public bool ice_isTwoway()
	{
	    return _reference.mode == IceInternal.Reference.ModeTwoway;
	}
	
	public ObjectPrx ice_oneway()
	{
	    IceInternal.Reference rf = _reference.changeMode(IceInternal.Reference.ModeOneway);
	    if(rf.Equals(_reference))
	    {
		return this;
	    }
	    else
	    {
		ObjectPrxHelper proxy = new ObjectPrxHelper();
		proxy.setup(rf);
		return proxy;
	    }
	}
	
	public bool ice_isOneway()
	{
	    return _reference.mode == IceInternal.Reference.ModeOneway;
	}
	
	public ObjectPrx ice_batchOneway()
	{
	    IceInternal.Reference rf = _reference.changeMode(IceInternal.Reference.ModeBatchOneway);
	    if(rf.Equals(_reference))
	    {
		return this;
	    }
	    else
	    {
		ObjectPrxHelper proxy = new ObjectPrxHelper();
		proxy.setup(rf);
		return proxy;
	    }
	}
	
	public bool ice_isBatchOneway()
	{
	    return _reference.mode == IceInternal.Reference.ModeBatchOneway;
	}
	
	public ObjectPrx ice_datagram()
	{
	    IceInternal.Reference rf = _reference.changeMode(IceInternal.Reference.ModeDatagram);
	    if(rf.Equals(_reference))
	    {
		return this;
	    }
	    else
	    {
		ObjectPrxHelper proxy = new ObjectPrxHelper();
		proxy.setup(rf);
		return proxy;
	    }
	}
	
	public bool ice_isDatagram()
	{
	    return _reference.mode == IceInternal.Reference.ModeDatagram;
	}
	
	public ObjectPrx ice_batchDatagram()
	{
	    IceInternal.Reference rf = _reference.changeMode(IceInternal.Reference.ModeBatchDatagram);
	    if(rf.Equals(_reference))
	    {
		return this;
	    }
	    else
	    {
		ObjectPrxHelper proxy = new ObjectPrxHelper();
		proxy.setup(rf);
		return proxy;
	    }
	}
	
	public bool ice_isBatchDatagram()
	{
	    return _reference.mode == IceInternal.Reference.ModeBatchDatagram;
	}
	
	public ObjectPrx ice_secure(bool b)
	{
	    IceInternal.Reference rf = _reference.changeSecure(b);
	    if(rf.Equals(_reference))
	    {
		return this;
	    }
	    else
	    {
		ObjectPrxHelper proxy = new ObjectPrxHelper();
		proxy.setup(rf);
		return proxy;
	    }
	}
	
	public ObjectPrx ice_compress(bool co)
	{
	    IceInternal.Reference rf = _reference.changeCompress(co);
	    if(rf.Equals(_reference))
	    {
		return this;
	    }
	    else
	    {
		ObjectPrxHelper proxy = new ObjectPrxHelper();
		proxy.setup(rf);
		return proxy;
	    }
	}
	
	public ObjectPrx ice_timeout(int t)
	{
	    IceInternal.Reference rf = _reference.changeTimeout(t);
	    if(rf.Equals(_reference))
	    {
		return this;
	    }
	    else
	    {
		ObjectPrxHelper proxy = new ObjectPrxHelper();
		proxy.setup(rf);
		return proxy;
	    }
	}
	
	public ObjectPrx ice_router(Ice.RouterPrx router)
	{
	    IceInternal.Reference rf = _reference.changeRouter(router);
	    if(rf.Equals(_reference))
	    {
		return this;
	    }
	    else
	    {
		ObjectPrxHelper proxy = new ObjectPrxHelper();
		proxy.setup(rf);
		return proxy;
	    }
	}
	
	public ObjectPrx ice_locator(Ice.LocatorPrx locator)
	{
	    IceInternal.Reference rf = _reference.changeLocator(locator);
	    if(rf.Equals(_reference))
	    {
		return this;
	    }
	    else
	    {
		ObjectPrxHelper proxy = new ObjectPrxHelper();
		proxy.setup(rf);
		return proxy;
	    }
	}
	
	public ObjectPrx ice_collocationOptimization(bool b)
	{
	    IceInternal.Reference rf = _reference.changeCollocationOptimization(b);
	    if(rf.Equals(_reference))
	    {
		return this;
	    }
	    else
	    {
		ObjectPrxHelper proxy = new ObjectPrxHelper();
		proxy.setup(rf);
		return proxy;
	    }
	}
	
	public ObjectPrx ice_default()
	{
	    IceInternal.Reference rf = _reference.changeDefault();
	    if(rf.Equals(_reference))
	    {
		return this;
	    }
	    else
	    {
		ObjectPrxHelper proxy = new ObjectPrxHelper();
		proxy.setup(rf);
		return proxy;
	    }
	}
	
	public void __checkTwowayOnly(string name)
	{
	    lock(this)
	    {
		if(!ice_isTwoway())
		{
		    TwowayOnlyException ex = new TwowayOnlyException();
		    ex.operation = name;
		    throw ex;
		}
	    }
	}
	
	public override bool Equals(object r)
	{
	    ObjectPrxHelper rhs = (ObjectPrxHelper)r;
	    return _reference.Equals(rhs._reference);
	}
	
	public IceInternal.Reference __reference()
	{
	    return _reference;
	}
	
	public void __copyFrom(ObjectPrx from)
	{
	    ObjectPrxHelper h = (ObjectPrxHelper) from;
	    IceInternal.Reference rf = null;
	    Object_DelM delegateM = null;
	    Object_DelD delegateD = null;
	    
	    lock(from)
	    {
		rf = h._reference;
		try
		{
		    delegateM = (Object_DelM)h._delegate;
		}
		catch(System.InvalidCastException)
		{
		}
		try
		{
		    delegateD = (Object_DelD)h._delegate;
		}
		catch(System.InvalidCastException)
		{
		}
	    }
	    
	    //
	    // No need to synchronize "*this", as this operation is only
	    // called upon initialization.
	    //
	    
	    Debug.Assert(_reference == null);
	    Debug.Assert(_delegate == null);
	    
	    _reference = rf;
	    
	    if(delegateD != null)
	    {
		Object_DelD @delegate = __createDelegateD();
		@delegate.__copyFrom(delegateD);
		_delegate = @delegate;
	    }
	    else if(delegateM != null)
	    {
		Object_DelM @delegate = __createDelegateM();
		@delegate.__copyFrom(delegateM);
		_delegate = @delegate;
	    }
	}
	
	public int __handleException(LocalException ex, int cnt)
	{
	    //
	    // Only _delegate needs to be mutex protected here.
	    //
	    lock(this)
	    {
		_delegate = null;
	    }
	    
	    if(_reference.locatorInfo != null)
	    {
		_reference.locatorInfo.clearObjectCache(_reference);
	    }
	    
	    ++cnt;
	    
	    IceInternal.TraceLevels traceLevels = _reference.instance.traceLevels();
	    Logger logger = _reference.instance.logger();
	    IceInternal.ProxyFactory proxyFactory = _reference.instance.proxyFactory();
	    
	    //
	    // Instance components may be null if Communicator has been destroyed.
	    //
	    if(traceLevels != null && logger != null && proxyFactory != null)
	    {
		int[] retryIntervals = proxyFactory.getRetryIntervals();
		
		if(cnt > retryIntervals.Length)
		{
		    if(traceLevels.retry >= 1)
		    {
			//UPGRADE_TODO: The equivalent in .NET for method 'java.lang.Throwable.toString' may return a different value. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1043"' // TODO
			string s = "cannot retry operation call because retry limit has been exceeded\n" + ex.ToString();
			logger.trace(traceLevels.retryCat, s);
		}
		    throw ex;
		}
		
		if(traceLevels.retry >= 1)
		{
		    string s = "re-trying operation call";
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
			SupportClass.ThreadClass.Current();
			System.Threading.Thread.Sleep(new System.TimeSpan(10000 * retryIntervals[cnt - 1]));
		    }
		    catch(System.Threading.ThreadInterruptedException)
		    {
		    }
		}
		
		return cnt;
	    }
	    else
	    {
		//
		// Impossible to retry after Communicator has been destroyed.
		//
		throw ex;
	    }
	}
	
	public void __rethrowException(LocalException ex)
	{
	    lock(this)
	    {
		_delegate = null;
		
		if(_reference.locatorInfo != null)
		{
		    _reference.locatorInfo.clearObjectCache(_reference);
		}
		
		throw ex;
	    }
	}
	
	public Object_Del __getDelegate()
	{
	    lock(this)
	    {
		if(_delegate == null)
		{
		    if(_reference.collocationOptimization)
		    {
			ObjectAdapter adapter = _reference.instance.objectAdapterFactory().findObjectAdapter(this);
			if(adapter != null)
			{
			    Object_DelD @delegate = __createDelegateD();
			    @delegate.setup(_reference, adapter);
			    _delegate = @delegate;
			}
		    }
		    
		    if(_delegate == null)
		    {
			Object_DelM @delegate = __createDelegateM();
			@delegate.setup(_reference);
			_delegate = @delegate;
			
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
	}
	
	protected virtual Object_DelM __createDelegateM()
	{
	    return new Object_DelM();
	}
	
	protected virtual Object_DelD __createDelegateD()
	{
	    return new Object_DelD();
	}
	
	protected internal virtual Context __defaultContext()
	{
	    return _reference.context;
	}
	
	//
	// Only for use by IceInternal.ProxyFactory
	//
	public void setup(IceInternal.Reference rf)
	{
	    //
	    // No need to synchronize, as this operation is only called
	    // upon initial initialization.
	    //
	    
	    Debug.Assert(_reference == null);
	    Debug.Assert(_delegate == null);
	    
	    _reference = rf;
	}
	
	private IceInternal.Reference _reference;
	private Object_Del _delegate;
    }

}
