// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        return ice_isA(__id, null, false);
    }

    public final boolean
    ice_isA(String __id, java.util.Map __context)
    {
        return ice_isA(__id, __context, true);
    }

    private boolean
    ice_isA(String __id, java.util.Map __context, boolean __explicitCtx)
    {
        if(__explicitCtx && __context == null)
        {
            __context = _emptyContext;
        }
        
        int __cnt = 0;
        while(true)
        {
            _ObjectDel __del = null;
            try
            {
                __checkTwowayOnly("ice_isA");
                __del = __getDelegate();
                return __del.ice_isA(__id, __context);
            }
            catch(IceInternal.LocalExceptionWrapper __ex)
            {
                __cnt = __handleExceptionWrapperRelaxed(__del, __ex, __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__del, __ex, __cnt);
            }
        }
    }

    public final void
    ice_ping()
    {
        ice_ping(null, false);
    }

    public final void
    ice_ping(java.util.Map __context)
    {
        ice_ping(__context, true);
    }

    private void
    ice_ping(java.util.Map __context, boolean __explicitCtx)
    {
        if(__explicitCtx && __context == null)
        {
            __context = _emptyContext;
        }

        int __cnt = 0;
        while(true)
        {
            _ObjectDel __del = null;
            try
            {
                __del = __getDelegate();
                __del.ice_ping(__context);
                return;
            }
            catch(IceInternal.LocalExceptionWrapper __ex)
            {
                __cnt = __handleExceptionWrapperRelaxed(__del, __ex, __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__del, __ex, __cnt);
            }
        }
    }

    public final String[]
    ice_ids()
    {
        return ice_ids(null, false);
    }

    public final String[]
    ice_ids(java.util.Map __context)
    {
        return ice_ids(__context, true);
    }

    private String[]
    ice_ids(java.util.Map __context, boolean __explicitCtx)
    {
        if(__explicitCtx && __context == null)
        {
            __context = _emptyContext;
        }

        int __cnt = 0;
        while(true)
        {
            _ObjectDel __del = null;
            try
            {
                __checkTwowayOnly("ice_ids");
                __del = __getDelegate();
                return __del.ice_ids(__context);
            }
            catch(IceInternal.LocalExceptionWrapper __ex)
            {
                __cnt = __handleExceptionWrapperRelaxed(__del, __ex, __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__del, __ex, __cnt);
            }
        }
    }

    public final String
    ice_id()
    {
        return ice_id(null, false);
    }

    public final String
    ice_id(java.util.Map __context)
    {
        return ice_id(__context, true);
    }

    private String
    ice_id(java.util.Map __context, boolean __explicitCtx)
    {
        if(__explicitCtx && __context == null)
        {
            __context = _emptyContext;
        }

        int __cnt = 0;
        while(true)
        {
            _ObjectDel __del = null;
            try
            {
                __checkTwowayOnly("ice_id");
                __del = __getDelegate();
                return __del.ice_id(__context);
            }
            catch(IceInternal.LocalExceptionWrapper __ex)
            {
                __cnt = __handleExceptionWrapperRelaxed(__del, __ex, __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__del, __ex, __cnt);
            }
        }
    }

    public final boolean
    ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams)
    {
        return ice_invoke(operation, mode, inParams, outParams, null, false);
    }

    public final boolean
    ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams,
               java.util.Map context)
    {
        return ice_invoke(operation, mode, inParams, outParams, context, true);
    }
    
    private boolean
    ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams,
               java.util.Map context, boolean explicitCtx)
    {
        if(explicitCtx && context == null)
        {
            context = _emptyContext;
        }

        int __cnt = 0;
        while(true)
        {
            _ObjectDel __del = null;
            try
            {
                __del = __getDelegate();
                return __del.ice_invoke(operation, mode, inParams, outParams, context);
            }
            catch(IceInternal.LocalExceptionWrapper __ex)
            {
                if(mode == OperationMode.Nonmutating || mode == OperationMode.Idempotent)
                {
                    __cnt = __handleExceptionWrapperRelaxed(__del, __ex, __cnt);
                }
                else
                {
                    __handleExceptionWrapper(__del, __ex);
                }
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__del, __ex, __cnt);
            }
        }
    }

    public final void
    ice_invoke_async(AMI_Object_ice_invoke cb, String operation, OperationMode mode, byte[] inParams)
    {
        __checkTwowayOnly("ice_invoke_async");
        cb.__invoke(this, operation, mode, inParams, null);
    }

    public final void
    ice_invoke_async(AMI_Object_ice_invoke cb, String operation, OperationMode mode, byte[] inParams,
                     java.util.Map context)
    {
        if(context == null)
        {
            context = _emptyContext;
        }
        __checkTwowayOnly("ice_invoke_async");
        cb.__invoke(this, operation, mode, inParams, context);
    }
    
    public final Identity
    ice_getIdentity()
    {
        return _reference.getIdentity();
    }

    public final ObjectPrx
    ice_identity(Identity newIdentity)
    {
        if(newIdentity.name.equals(""))
        {
            throw new IllegalIdentityException();
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

    public final java.util.Map
    ice_getContext()
    {
        return _reference.getContext();
    }

    public final ObjectPrx
    ice_context(java.util.Map newContext)
    {
        ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
        proxy.setup(_reference.changeContext(newContext));
        return proxy;
    }

    /**
     * @deprecated This method has been replaced with ice_context.
     **/
    public final ObjectPrx
    ice_newContext(java.util.Map newContext)
    {
        return ice_context(newContext);
    }

    /**
     * @deprecated
     **/
    public final ObjectPrx
    ice_defaultContext()
    {
        ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
        proxy.setup(_reference.defaultContext());
        return proxy;
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

    /**
     * @deprecated This method has been replaced with ice_adapterId.
     **/
    public final ObjectPrx
    ice_newAdapterId(String newAdapterId)
    {
        return ice_adapterId(newAdapterId);
    }

    public final Endpoint[]
    ice_getEndpoints()
    {
        return _reference.getEndpoints();
    }

    public final ObjectPrx
    ice_endpoints(Endpoint[] newEndpoints)
    {
        if(java.util.Arrays.equals(newEndpoints, _reference.getEndpoints()))
        {
            return this;
        }
        else
        {
            IceInternal.EndpointI[] edpts = new IceInternal.EndpointI[newEndpoints.length];
            edpts = (IceInternal.EndpointI[])java.util.Arrays.asList(newEndpoints).toArray(edpts);
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeEndpoints(edpts));
            return proxy;
        }
    }

    /**
     * @deprecated This method has been replaced with ice_endpoints.
     **/
    public final ObjectPrx
    ice_newEndpoints(Endpoint[] newEndpoints)
    {
        return ice_endpoints(newEndpoints);
    }

    public final int
    ice_getLocatorCacheTimeout()
    {
        return _reference.getLocatorCacheTimeout();
    }

    public final ObjectPrx
    ice_locatorCacheTimeout(int newTimeout)
    {
        if(newTimeout == _reference.getLocatorCacheTimeout())
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeLocatorCacheTimeout(newTimeout));
            return proxy;
        }
    }

    public final boolean
    ice_isConnectionCached()
    {
        return _reference.getCacheConnection();
    }

    public final ObjectPrx
    ice_connectionCached(boolean newCache)
    {
        if(newCache == _reference.getCacheConnection())
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeCacheConnection(newCache));
            return proxy;
        }
    }

    public final Ice.EndpointSelectionType
    ice_getEndpointSelection()
    {
        return _reference.getEndpointSelection();
    }

    public final ObjectPrx
    ice_endpointSelection(Ice.EndpointSelectionType newType)
    {
        if(newType == _reference.getEndpointSelection())
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeEndpointSelection(newType));
            return proxy;
        }
    }

    public final boolean
    ice_isSecure()
    {
        return _reference.getSecure();
    }

    public final ObjectPrx
    ice_secure(boolean b)
    {
        if(b == _reference.getSecure())
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeSecure(b));
            return proxy;
        }
    }

    public final boolean
    ice_isPreferSecure()
    {
        return _reference.getPreferSecure();
    }

    public final ObjectPrx
    ice_preferSecure(boolean b)
    {
        if(b == _reference.getPreferSecure())
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changePreferSecure(b));
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

    public final boolean
    ice_isCollocationOptimized()
    {
        return _reference.getCollocationOptimization();
    }

    /**
     * @deprecated This method has been replaced with ice_collocationOptimized.
     **/
    public final ObjectPrx
    ice_collocationOptimization(boolean b)
    {
        return ice_collocationOptimized(b);
    }

    public final ObjectPrx
    ice_collocationOptimized(boolean b)
    {
        if(b == _reference.getCollocationOptimization())
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeCollocationOptimization(b));
            return proxy;
        }
    }

    public final ObjectPrx
    ice_twoway()
    {
        if(_reference.getMode() == IceInternal.Reference.ModeTwoway)
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeMode(IceInternal.Reference.ModeTwoway));
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
        if(_reference.getMode() == IceInternal.Reference.ModeOneway)
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeMode(IceInternal.Reference.ModeOneway));
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
        if(_reference.getMode() == IceInternal.Reference.ModeBatchOneway)
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeMode(IceInternal.Reference.ModeBatchOneway));
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
        if(_reference.getMode() == IceInternal.Reference.ModeDatagram)
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeMode(IceInternal.Reference.ModeDatagram));
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
        if(_reference.getMode() == IceInternal.Reference.ModeBatchDatagram)
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeMode(IceInternal.Reference.ModeBatchDatagram));
            return proxy;
        }
    }

    public final boolean
    ice_isBatchDatagram()
    {
        return _reference.getMode() == IceInternal.Reference.ModeBatchDatagram;
    }

    public final ObjectPrx
    ice_compress(boolean co)
    {
        IceInternal.Reference ref = _reference.changeCompress(co);
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
    ice_connectionId(String id)
    {
        IceInternal.Reference ref = _reference.changeConnectionId(id);
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

    public boolean
    ice_isThreadPerConnection()
    {
        return _reference.getThreadPerConnection();
    }

    public ObjectPrx
    ice_threadPerConnection(boolean tpc)
    {
        IceInternal.Reference ref = _reference.changeThreadPerConnection(tpc);
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
     * @deprecated This method has been replaced with ice_collocationOptimized.
     **/
    public final Connection
    ice_connection()
    {
        return ice_getConnection();
    }

    public final Connection
    ice_getConnection()
    {
        int __cnt = 0;
        while(true)
        {
            _ObjectDel __del = null;
            try
            {
                __del = __getDelegate();
                return __del.__getConnection(new BooleanHolder());
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__del, __ex, __cnt);
            }
        }
    }

    public final Connection
    ice_getCachedConnection()
    {
        _ObjectDel __del = null;
        synchronized(this)
        {
            __del = _delegate;
        }
        
        if(__del != null)
        {
            try
            {
                return __del.__getConnection(new BooleanHolder());
            }
            catch(CollocationOptimizationException ex)
            {
            }
        }
        return null;
    }

    public final boolean
    equals(java.lang.Object r)
    {
        if(this == r)
        {
            return true;
        }

        if(r instanceof ObjectPrxHelperBase)
        {
            return _reference.equals(((ObjectPrxHelperBase)r)._reference);
        }

        return false;
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

        assert(_reference == null);
        assert(_delegate == null);

        _reference = ref;

        if(_reference.getCacheConnection())
        {
            //
            // The _delegate attribute is only used if "cache connection"
            // is enabled. If it's not enabled, we don't keep track of the
            // delegate -- a new delegate is created for each invocations.
            //  
            
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
    }

    public final int
    __handleException(_ObjectDel delegate, LocalException ex, int cnt)
    {
        //
        // Only _delegate needs to be mutex protected here.
        //
        synchronized(this)
        {
            if(delegate == _delegate)
            {
                _delegate = null;
            }
        }

        IceInternal.ProxyFactory proxyFactory;
        try
        {
            proxyFactory = _reference.getInstance().proxyFactory();
        }
        catch(CommunicatorDestroyedException e)
        {
            //
            // The communicator is already destroyed, so we cannot
            // retry.
            //
            throw ex;
        }

        return proxyFactory.checkRetryAfterException(ex, _reference, cnt);
    }

    public final void
    __handleExceptionWrapper(_ObjectDel delegate, IceInternal.LocalExceptionWrapper ex)
    {
        synchronized(this)
        {
            if(delegate == _delegate)
            {
                _delegate = null;
            }
        }

        if(!ex.retry())
        {
            throw ex.get();
        }
    }

    public final int
    __handleExceptionWrapperRelaxed(_ObjectDel delegate, IceInternal.LocalExceptionWrapper ex, int cnt)
    {
        if(!ex.retry())
        {
            return __handleException(delegate, ex.get(), cnt);
        }
        else
        {
            synchronized(this)
            {
                if(delegate == _delegate)
                {
                    _delegate = null;
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

    public final synchronized _ObjectDel
    __getDelegate()
    {
        if(_delegate != null)
        {
            return _delegate;
        }

        _ObjectDel delegate = null;
        if(_reference.getCollocationOptimization())
        {
            ObjectAdapter adapter = _reference.getInstance().objectAdapterFactory().findObjectAdapter(this);
            if(adapter != null)
            {
                _ObjectDelD d = __createDelegateD();
                d.setup(_reference, adapter);
                delegate = d;
            }
        }

        if(delegate == null)
        {
            _ObjectDelM d = __createDelegateM();
            d.setup(_reference);
            delegate = d;

            //
            // If this proxy is for a non-local object, and we are
            // using a router, then add this proxy to the router info
            // object.
            //
            IceInternal.RouterInfo ri = _reference.getRouterInfo();
            if(ri != null)
            {
                ri.addProxy(this);
            }
        }

        if(_reference.getCacheConnection())
        {
            //
            // The _delegate attribute is only used if "cache connection"
            // is enabled. If it's not enabled, we don't keep track of the
            // delegate -- a new delegate is created for each invocations.
            //
            _delegate = delegate;
        }

        return delegate;
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

        assert(_reference == null);
        assert(_delegate == null);

        _reference = ref;
    }

    protected static final java.util.Map _emptyContext = new java.util.HashMap();

    private IceInternal.Reference _reference;
    private _ObjectDel _delegate;
}
