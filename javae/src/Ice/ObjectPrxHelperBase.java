// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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

    /**
     * @deprecated This method has been replaced with ice_getHash.
     **/
    public final int
    ice_hash()
    {
        return ice_getHash();
    }

    public final int
    ice_getHash()
    {
        return _reference.hashCode();
    }

    /**
     * @deprecated This method has been replaced with ice_getCommunicator.
     **/
    public final Communicator ice_communicator()
    {
        return ice_getCommunicator();
    }

    public final Communicator ice_getCommunicator()
    {
        return _reference.getCommunicator();
    }

    public final String toString()
    {
        return _reference.toString();
    }

    public final String ice_toString()
    {
        return toString();
    }

    public final boolean
    ice_isA(String __id)
    {
        return ice_isA(__id, _reference.getContext());
    }

    public final boolean
    ice_isA(String __id, java.util.Hashtable __context)
    {
        int __cnt = 0;
        while(true)
        {
            Connection __connection = null;
            try
            {
	        __checkTwowayOnly("ice_isA");
                __connection = ice_getConnection();
                IceInternal.Outgoing __og = __connection.getOutgoing(_reference, "ice_isA", OperationMode.Nonmutating,
                                                                     __context);
                try
                {
                    try
                    {
                        IceInternal.BasicStream __os = __og.stream();
                        __os.writeString(__id);
                    }
                    catch(Ice.LocalException __ex)
                    {
                        __og.abort(__ex);
                    }
                    boolean __ok = __og.invoke();
                    try
                    {
                        IceInternal.BasicStream __is = __og.stream();
                        if(!__ok)
                        {
			    try
			    {
                                __is.throwException();
			    }
			    catch(UserException __ex)
			    {
			        throw new Ice.UnknownUserException(__ex.ice_name());
			    }
                        }
                        return __is.readBool();
                    }
                    catch(LocalException __ex)
                    {
                        throw new IceInternal.LocalExceptionWrapper(__ex, false);
                    }
                }
                finally
                {
                    __connection.reclaimOutgoing(__og);
                }
            }
            catch(IceInternal.LocalExceptionWrapper __ex)
            {
                __cnt = __handleExceptionWrapperRelaxed(__connection, __ex, __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__connection, __ex, __cnt);
            }
        }
    }

    public final void
    ice_ping()
    {
        ice_ping(_reference.getContext());
    }

    public final void
    ice_ping(java.util.Hashtable __context)
    {
        int __cnt = 0;
        while(true)
        {
            Connection __connection = null;
            try
            {
		__connection = ice_getConnection();
                IceInternal.Outgoing __og = __connection.getOutgoing(_reference, "ice_ping", OperationMode.Nonmutating,
                                                                    __context);
                try
                {
                    boolean __ok = __og.invoke();
                    try
                    {
                        IceInternal.BasicStream __is = __og.stream();
                        if(!__ok)
                        {
			    try
			    {
                                __is.throwException();
			    }
			    catch(UserException __ex)
			    {
			        throw new Ice.UnknownUserException(__ex.ice_name());
			    }
                        }
                    }
                    catch(LocalException __ex)
                    {
                        throw new IceInternal.LocalExceptionWrapper(__ex, false);
                    }
                }
                finally
                {
                   __connection.reclaimOutgoing(__og);
                }
                return;
            }
            catch(IceInternal.LocalExceptionWrapper __ex)
            {
                __cnt = __handleExceptionWrapperRelaxed(__connection, __ex, __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__connection, __ex, __cnt);
            }
        }
    }

    public String[]
    ice_ids()
    {
        return ice_ids(_reference.getContext());
    }

    public String[]
    ice_ids(java.util.Hashtable __context)
    {
        int __cnt = 0;
        while(true)
        {
            Connection __connection = null;
            try
            {
	        __checkTwowayOnly("ice_ids");
	        __connection = ice_getConnection();
                IceInternal.Outgoing __og = __connection.getOutgoing(_reference, "ice_ids", OperationMode.Nonmutating,
                                                                    __context);
                try
                {
                    boolean __ok = __og.invoke();
                    try
                    {
                        IceInternal.BasicStream __is = __og.stream();
                        if(!__ok)
                        {
			    try
			    {
                            __is.throwException();
			    }
			    catch(UserException __ex)
			    {
			        throw new Ice.UnknownUserException(__ex.ice_name());
			    }
                        }
                        return __is.readStringSeq();
                    }
                    catch(LocalException __ex)
                    {
                        throw new IceInternal.LocalExceptionWrapper(__ex, false);
                    }
                }
                finally
                {
                    __connection.reclaimOutgoing(__og);
                }
            }
            catch(IceInternal.LocalExceptionWrapper __ex)
            {
                __cnt = __handleExceptionWrapperRelaxed(__connection, __ex, __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__connection, __ex, __cnt);
            }
        }
    }

    public String
    ice_id()
    {
        return ice_id(_reference.getContext());
    }

    public String
    ice_id(java.util.Hashtable __context)
    {
        int __cnt = 0;
        while(true)
        {
            Connection __connection = null;
            try
            {
	        __checkTwowayOnly("ice_id");
	        __connection = ice_getConnection();
                IceInternal.Outgoing __og = __connection.getOutgoing(_reference, "ice_id", OperationMode.Nonmutating,
                                                                    __context);
                try
                {
                    boolean __ok = __og.invoke();
                    try
                    {
                        IceInternal.BasicStream __is = __og.stream();
                        if(!__ok)
                        {
			    try
			    {
                                __is.throwException();
			    }
			    catch(UserException __ex)
			    {
			        throw new Ice.UnknownUserException(__ex.ice_name());
			    }
                        }
                        return __is.readString();
                    }
                    catch(LocalException __ex)
                    {
                        throw new IceInternal.LocalExceptionWrapper(__ex, false);
                    }
                }
                finally
                {
                    __connection.reclaimOutgoing(__og);
                }
            }
            catch(IceInternal.LocalExceptionWrapper __ex)
            {
                __cnt = __handleExceptionWrapperRelaxed(__connection, __ex, __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__connection, __ex, __cnt);
            }
        }
    }

    public final Identity
    ice_getIdentity()
    {
        return _reference.getIdentity();
    }

    public final ObjectPrx
    ice_identity(Identity newIdentity)
    {
	if(newIdentity.equals(""))
	{
	    throw new Ice.IllegalIdentityException();
	}
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

    /**
     * @deprecated This method has been replaced with ice_identity.
     **/
    public final ObjectPrx
    ice_newIdentity(Identity newIdentity)
    {
	return ice_identity(newIdentity);
    }

    public final java.util.Hashtable
    ice_getContext()
    {
        return _reference.getContext();
    }

    public final ObjectPrx
    ice_context(java.util.Hashtable newContext)
    {
	ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
	proxy.setup(_reference.changeContext(newContext));
	return proxy;
    }

    /**
     * @deprecated This method has been replaced with ice_context.
     **/
    public final ObjectPrx
    ice_newContext(java.util.Hashtable newContext)
    {
	return ice_context(newContext);
    }

    public final String
    ice_getFacet()
    {
        return _reference.getFacet();
    }

    public final ObjectPrx
    ice_facet(String newFacet)
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

    /**
     * @deprecated This method has been replaced with ice_facet.
     **/
    public final ObjectPrx
    ice_newFacet(String newFacet)
    {
	return ice_facet(newFacet);
    }

    public final String
    ice_getAdapterId()
    {
        return _reference.getAdapterId();
    }

    public final ObjectPrx
    ice_adapterId(String newAdapterId)
    {
        if(newAdapterId == null)
        {
            newAdapterId = "";
        }

        if(newAdapterId.equals(_reference.getAdapterId()))
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeAdapterId(newAdapterId));
            return proxy;
        }
    }

    public final boolean
    ice_isSecure()
    {
        return _reference.getSecure();
    }

    public final ObjectPrx
    ice_secure(boolean newSecure)
    {
        if(newSecure == _reference.getSecure())
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeSecure(newSecure));
            return proxy;
        }
    }

    public final Ice.RouterPrx
    ice_getRouter()
    {
	IceInternal.RouterInfo ri = _reference.getRouterInfo();
	return ri != null ? ri.getRouter() : null;
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

    public final Ice.LocatorPrx
    ice_getLocator()
    {
	IceInternal.LocatorInfo ri = _reference.getLocatorInfo();
	return ri != null ? ri.getLocator() : null;
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
    ice_datagram()
    {
        IceInternal.Reference ref = _reference.changeMode(IceInternal.Reference.ModeDatagram);
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
    ice_isDatagram()
    {
	return _reference.getMode() == IceInternal.Reference.ModeDatagram;
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
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final boolean
    ice_isBatchDatagram()
    {
	return _reference.getMode() == IceInternal.Reference.ModeBatchDatagram;
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

    /**
     * @deprecated This method has been replaced with ice_getConnection.
     **/
    public synchronized final Connection
    ice_connection()
    {
        return ice_getConnection();
    }

    public synchronized final Connection
    ice_getConnection()
    {
	if(_connection == null)
	{
	    _connection = _reference.getConnection();

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
	return _connection;
    }

    public synchronized final Connection
    ice_getCachedConnection()
    {
        return _connection;
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
        Connection con = null;

        synchronized(from)
        {
            ref = h._reference;
            con = h._connection;
        }

        //
        // No need to synchronize "*this", as this operation is only
        // called upon initialization.
        //

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_reference == null);
	    IceUtil.Debug.Assert(_connection == null);
	}

        _reference = ref;
	_connection = con;
    }

    public final int
    __handleException(Connection connection, LocalException ex, int cnt)
    {
	//
	// Only _connection needs to be mutex protected here.
	//
	synchronized(this)
	{
            if(connection == _connection)
            {
                _connection = null;
            }
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

    public final void
    __handleExceptionWrapper(Connection connection, IceInternal.LocalExceptionWrapper ex)
    {
        synchronized(this)
	{
            if(connection == _connection)
            {
                _connection = null;
            }
	}

	if(!ex.retry())
	{
	    throw ex.get();
	}
    }

    public final int
    __handleExceptionWrapperRelaxed(Connection connection, IceInternal.LocalExceptionWrapper ex, int cnt)
    {
        if(!ex.retry())
	{
	    return __handleException(connection, ex.get(), cnt);
	}
	else
	{
	    synchronized(this)
	    {
                if(connection == _connection)
                {
                    _connection = null;
                }
	    }
	    return cnt;
	}
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
	    IceUtil.Debug.Assert(_connection == null);
	}

        _reference = ref;
    }

    protected static final java.util.Hashtable _emptyContext = new java.util.Hashtable();

    protected IceInternal.Reference _reference;
    private Connection _connection;

}
