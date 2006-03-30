// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Diagnostics;
using IceUtil;

namespace Ice
{
    public interface ObjectPrx
    {
        int ice_hash();

	Communicator ice_communicator();

	string ice_toString();

        bool ice_isA(string id__);
        bool ice_isA(string id__, Context context__);

        void ice_ping();
        void ice_ping(Context context__);

        string[] ice_ids();
        string[] ice_ids(Context context__);

        string ice_id();
        string ice_id(Context context__);

        // Returns true if ok, false if user exception.
        bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams);
        bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams,
            Context context__);

        void ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inParams);
        void ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inParams,
            Context context);

        Identity ice_getIdentity();
	[Obsolete("This method has been deprecated, use ice_identity instead.")]
        ObjectPrx ice_newIdentity(Identity newIdentity);
        ObjectPrx ice_identity(Identity newIdentity);
	
        Context ice_getContext();
	[Obsolete("This method has been deprecated, use ice_context instead.")]
        ObjectPrx ice_newContext(Context newContext);
        ObjectPrx ice_context(Context newContext);
        ObjectPrx ice_defaultContext();
	
        string ice_getFacet();
	[Obsolete("This method has been deprecated, use ice_facet instead.")]
        ObjectPrx ice_newFacet(string newFacet);
        ObjectPrx ice_facet(string newFacet);

        string ice_getAdapterId();
	[Obsolete("This method has been deprecated, use ice_adapterId instead.")]
        ObjectPrx ice_newAdapterId(string newAdapterId);
        ObjectPrx ice_adapterId(string newAdapterId);

	Endpoint[] ice_getEndpoints();
	[Obsolete("This method has been deprecated, use ice_endpoints instead.")]
        ObjectPrx ice_newEndpoints(Endpoint[] newEndpoints);
        ObjectPrx ice_endpoints(Endpoint[] newEndpoints);

	int ice_getLocatorCacheTimeout();
	ObjectPrx ice_locatorCacheTimeout(int timeout);
	
	bool ice_getCacheConnection();
	ObjectPrx ice_cacheConnection(bool newCache);

	EndpointSelectionType ice_getEndpointSelection();
	ObjectPrx ice_endpointSelection(EndpointSelectionType newType);

	bool ice_getSecure();
        ObjectPrx ice_secure(bool b);

	Ice.RouterPrx ice_getRouter();
        ObjectPrx ice_router(Ice.RouterPrx router);

	Ice.LocatorPrx ice_getLocator();
        ObjectPrx ice_locator(Ice.LocatorPrx locator);

	bool ice_getCollocationOptimization();
        ObjectPrx ice_collocationOptimization(bool b);

        ObjectPrx ice_twoway();
        bool ice_isTwoway();
        ObjectPrx ice_oneway();
        bool ice_isOneway();
        ObjectPrx ice_batchOneway();
        bool ice_isBatchOneway();
        ObjectPrx ice_datagram();
        bool ice_isDatagram();
        ObjectPrx ice_batchDatagram();
        bool ice_isBatchDatagram();

        ObjectPrx ice_compress(bool co);
        ObjectPrx ice_timeout(int t);
        ObjectPrx ice_connectionId(string connectionId);

        Connection ice_connection();
    }

    public class ObjectPrxHelperBase : ObjectPrx
    {
        public override int GetHashCode()
        {
            return _reference.GetHashCode();
        }

        public int ice_hash()
        {
            return _reference.GetHashCode();
        }

        public Communicator ice_communicator()
	{
	    return _reference.getCommunicator();
	}

	public override string ToString()
	{
	    return _reference.ToString();
	}

	public string ice_toString()
	{
	    return ToString();
	}

        public bool ice_isA(string id__)
        {
            return ice_isA(id__, _reference.getContext());
        }

        public bool ice_isA(string id__, Context context__)
        {
            int cnt__ = 0;
            while(true)
            {
                try
                {
                    checkTwowayOnly__("ice_isA");
                    ObjectDel_ del__ = getDelegate__();
                    return del__.ice_isA(id__, context__);
                }
                catch(IceInternal.LocalExceptionWrapper ex__)
                {
                    cnt__ = handleExceptionWrapperRelaxed__(ex__, cnt__);
                }
                catch(LocalException ex__)
                {
                    cnt__ = handleException__(ex__, cnt__);
                }
            }
        }

        public void ice_ping()
        {
            ice_ping(_reference.getContext());
        }

        public void ice_ping(Context context__)
        {
            int cnt__ = 0;
            while(true)
            {
                try
                {
                    checkTwowayOnly__("ice_ping");
                    ObjectDel_ del__ = getDelegate__();
                    del__.ice_ping(context__);
                    return;
                }
                catch(IceInternal.LocalExceptionWrapper ex__)
                {
		    cnt__ = handleExceptionWrapperRelaxed__(ex__, cnt__);
                }
                catch(LocalException ex__)
                {
                    cnt__ = handleException__(ex__, cnt__);
                }
            }
        }

        public string[] ice_ids()
        {
            return ice_ids(_reference.getContext());
        }

        public string[] ice_ids(Context context__)
        {
            int cnt__ = 0;
            while(true)
            {
                try
                {
                    checkTwowayOnly__("ice_ids");
                    ObjectDel_ del__ = getDelegate__();
                    return del__.ice_ids(context__);
                }
                catch(IceInternal.LocalExceptionWrapper ex__)
                {
		    cnt__ = handleExceptionWrapperRelaxed__(ex__, cnt__);
                }
                catch(LocalException ex__)
                {
                    cnt__ = handleException__(ex__, cnt__);
                }
            }
        }

        public string ice_id()
        {
            return ice_id(_reference.getContext());
        }

        public string ice_id(Context context__)
        {
            int cnt__ = 0;
            while(true)
            {
                try
                {
                    checkTwowayOnly__("ice_id");
                    ObjectDel_ del__ = getDelegate__();
                    return del__.ice_id(context__);
                }
                catch(IceInternal.LocalExceptionWrapper ex__)
                {
		    cnt__ = handleExceptionWrapperRelaxed__(ex__, cnt__);
                }
                catch(LocalException ex__)
                {
                    cnt__ = handleException__(ex__, cnt__);
                }
            }
        }

        public bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams)
        {
            return ice_invoke(operation, mode, inParams, out outParams, _reference.getContext());
        }

        public bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams,
            Context context)
        {
            int cnt__ = 0;
            while(true)
            {
                try
                {
                    ObjectDel_ del__ = getDelegate__();
                    return del__.ice_invoke(operation, mode, inParams, out outParams, context);
                }
                catch(IceInternal.LocalExceptionWrapper ex__)
                {
                    if(mode == OperationMode.Nonmutating || mode == OperationMode.Idempotent)
                    {
                        cnt__ = handleExceptionWrapperRelaxed__(ex__, cnt__);
                    }
                    else
                    {
                        handleExceptionWrapper__(ex__);
                    }
                }
                catch(LocalException ex__)
                {
                    cnt__ = handleException__(ex__, cnt__);
                }
            }
        }

        public void ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inParams)
        {
            ice_invoke_async(cb, operation, mode, inParams, _reference.getContext());
        }

        public void ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inParams,
				     Context context)
        {
            checkTwowayOnly__("ice_invoke_async");
            cb.invoke__(this, operation, mode, inParams, context);
        }

        public Identity ice_getIdentity()
        {
            return _reference.getIdentity();
        }

        public ObjectPrx ice_identity(Identity newIdentity)
        {
            if(newIdentity.Equals(_reference.getIdentity()))
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

        public ObjectPrx ice_newIdentity(Identity newIdentity)
        {
	    return ice_identity(newIdentity);
	}

        public Context ice_getContext()
        {
            return _reference.getContext();
        }

        public ObjectPrx ice_context(Context newContext)
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeContext(newContext));
            return proxy;
        }

        public ObjectPrx ice_newContext(Context newContext)
        {
	    return ice_context(newContext);
	}

	public ObjectPrx ice_defaultContext()
	{
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.defaultContext());
            return proxy;
	}

        public string ice_getFacet()
        {
            return _reference.getFacet();
        }

        public ObjectPrx ice_facet(string newFacet)
        {
            if(newFacet == null)
            {
                newFacet = "";
            }

            if(newFacet.Equals(_reference.getFacet()))
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

        public ObjectPrx ice_newFacet(string newFacet)
        {
	    return ice_facet(newFacet);
	}

        public string ice_getAdapterId()
        {
            return _reference.getAdapterId();
        }

        public ObjectPrx ice_adapterId(string newAdapterId)
        {
            if(newAdapterId == null)
            {
                newAdapterId = "";
            }

            if(newAdapterId.Equals(_reference.getAdapterId()))
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

        public ObjectPrx ice_newAdapterId(string newAdapterId)
        {
	    return ice_adapterId(newAdapterId);
	}

        public Endpoint[] ice_getEndpoints()
        {
            return _reference.getEndpoints();
        }

        public ObjectPrx ice_endpoints(Endpoint[] newEndpoints)
        {
            if(Arrays.Equals(newEndpoints, _reference.getEndpoints()))
            {
                return this;
            }
            else
            {
                ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
		ArrayList arr = ArrayList.Adapter(newEndpoints);
		IceInternal.EndpointI[] endpts = (IceInternal.EndpointI[])arr.ToArray(typeof(IceInternal.EndpointI));
                proxy.setup(_reference.changeEndpoints(endpts));
                return proxy;
            }
        }

        public ObjectPrx ice_newEndpoints(Endpoint[] newEndpoints)
        {
	    return ice_endpoints(newEndpoints);
	}

        public int ice_getLocatorCacheTimeout()
        {
            return _reference.getLocatorCacheTimeout();
        }

        public ObjectPrx ice_locatorCacheTimeout(int newTimeout)
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

        public bool ice_getCacheConnection()
        {
            return _reference.getCacheConnection();
        }

        public ObjectPrx ice_cacheConnection(bool newCache)
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

        public EndpointSelectionType ice_getEndpointSelection()
        {
            return _reference.getEndpointSelection();
        }

        public ObjectPrx ice_endpointSelection(EndpointSelectionType newType)
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

        public bool ice_getSecure()
        {
            return _reference.getSecure();
        }

        public ObjectPrx ice_secure(bool b)
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

        public Ice.RouterPrx ice_getRouter()
        {
	    IceInternal.RouterInfo ri = _reference.getRouterInfo();
	    return ri != null ? ri.getRouter() : null;
        }

        public ObjectPrx ice_router(RouterPrx router)
        {
            IceInternal.Reference @ref = _reference.changeRouter(router);
            if(@ref.Equals(_reference))
            {
                return this;
            }
            else
            {
                ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
                proxy.setup(@ref);
                return proxy;
            }
        }

        public Ice.LocatorPrx ice_getLocator()
        {
	    IceInternal.LocatorInfo li = _reference.getLocatorInfo();
	    return li != null ? li.getLocator() : null;
        }

        public ObjectPrx ice_locator(LocatorPrx locator)
        {
            IceInternal.Reference @ref = _reference.changeLocator(locator);
            if(@ref.Equals(_reference))
            {
                return this;
            }
            else
            {
                ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
                proxy.setup(@ref);
                return proxy;
            }
        }

        public bool ice_getCollocationOptimization()
        {
            return _reference.getCollocationOptimization();
        }

        public ObjectPrx ice_collocationOptimization(bool b)
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

        public ObjectPrx ice_twoway()
        {
            if(_reference.getMode() == IceInternal.Reference.Mode.ModeTwoway)
            {
                return this;
            }
            else
            {
                ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
                proxy.setup(_reference.changeMode(IceInternal.Reference.Mode.ModeTwoway));
                return proxy;
            }
        }

        public bool ice_isTwoway()
        {
            return _reference.getMode() == IceInternal.Reference.Mode.ModeTwoway;
        }

        public ObjectPrx ice_oneway()
        {
            if(_reference.getMode() == IceInternal.Reference.Mode.ModeOneway)
            {
                return this;
            }
            else
            {
                ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
                proxy.setup(_reference.changeMode(IceInternal.Reference.Mode.ModeOneway));
                return proxy;
            }
        }

        public bool ice_isOneway()
        {
            return _reference.getMode() == IceInternal.Reference.Mode.ModeOneway;
        }

        public ObjectPrx ice_batchOneway()
        {
            if(_reference.getMode() == IceInternal.Reference.Mode.ModeBatchOneway)
            {
                return this;
            }
            else
            {
                ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
                proxy.setup(_reference.changeMode(IceInternal.Reference.Mode.ModeBatchOneway));
                return proxy;
            }
        }

        public bool ice_isBatchOneway()
        {
            return _reference.getMode() == IceInternal.Reference.Mode.ModeBatchOneway;
        }

        public ObjectPrx ice_datagram()
        {
            if(_reference.getMode() == IceInternal.Reference.Mode.ModeDatagram)
            {
                return this;
            }
            else
            {
                ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
                proxy.setup(_reference.changeMode(IceInternal.Reference.Mode.ModeDatagram));
                return proxy;
            }
        }

        public bool ice_isDatagram()
        {
            return _reference.getMode() == IceInternal.Reference.Mode.ModeDatagram;
        }

        public ObjectPrx ice_batchDatagram()
        {
            if(_reference.getMode() == IceInternal.Reference.Mode.ModeBatchDatagram)
            {
                return this;
            }
            else
            {
                ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
                proxy.setup(_reference.changeMode(IceInternal.Reference.Mode.ModeBatchDatagram));
                return proxy;
            }
        }

        public bool ice_isBatchDatagram()
        {
            return _reference.getMode() == IceInternal.Reference.Mode.ModeBatchDatagram;
        }

        public ObjectPrx ice_compress(bool co)
        {
            IceInternal.Reference @ref = _reference.changeCompress(co);
            if(@ref.Equals(_reference))
            {
                return this;
            }
            else
            {
                ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
                proxy.setup(@ref);
                return proxy;
            }
        }

        public ObjectPrx ice_timeout(int t)
        {
            IceInternal.Reference @ref = _reference.changeTimeout(t);
            if(@ref.Equals(_reference))
            {
                return this;
            }
            else
            {
                ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
                proxy.setup(@ref);
                return proxy;
            }
        }

	public ObjectPrx ice_connectionId(string connectionId)
        {
            IceInternal.Reference @ref = _reference.changeConnectionId(connectionId);
            if(@ref.Equals(_reference))
            {
                return this;
            }
            else
            {
                ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
                proxy.setup(@ref);
                return proxy;
            }
        }

        public Connection ice_connection()
        {
            int cnt__ = 0;
            while(true)
            {
                try
                {
                    ObjectDel_ del__ = getDelegate__();
		    bool comp;
                    return del__.getConnection__(out comp);
                }
                catch(LocalException ex__)
                {
                    cnt__ = handleException__(ex__, cnt__);
                }
            }
        }

        public override bool Equals(object r)
        {
            ObjectPrxHelperBase rhs = r as ObjectPrxHelperBase;
            return object.ReferenceEquals(rhs, null) ? false : _reference.Equals(rhs._reference);
        }

        public static bool Equals(Ice.ObjectPrxHelperBase lhs, Ice.ObjectPrxHelperBase rhs)
        {
            return object.ReferenceEquals(lhs, null) ? object.ReferenceEquals(rhs, null) : lhs.Equals(rhs);
        }

        public static bool operator==(ObjectPrxHelperBase lhs, ObjectPrxHelperBase rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator!=(ObjectPrxHelperBase lhs, ObjectPrxHelperBase rhs)
        {
            return !Equals(lhs, rhs);
        }

        public IceInternal.Reference reference__()
        {
            return _reference;
        }

        public void copyFrom__(ObjectPrx from)
        {
            ObjectPrxHelperBase h = (ObjectPrxHelperBase)from;
            IceInternal.Reference @ref = null;
            ObjectDelM_ delegateM = null;
            ObjectDelD_ delegateD = null;

            lock(from)
            {
                @ref = h._reference;
                delegateM = h._delegate as ObjectDelM_;
                delegateD = h._delegate as ObjectDelD_;
            }

            //
            // No need to synchronize "*this", as this operation is only
            // called upon initialization.
            //

            Debug.Assert(_reference == null);
            Debug.Assert(_delegate == null);

            _reference = @ref;

	    if(_reference.getCacheConnection())
	    {
		//
		// The _delegate attribute is only used if "cache connection"
		// is enabled. If it's not enabled, we don't keep track of the
		// delegate -- a new delegate is created for each invocations.
		//	

		if(delegateD != null)
		{
		    ObjectDelD_ @delegate = createDelegateD__();
		    @delegate.copyFrom__(delegateD);
		    _delegate = @delegate;
		}
		else if(delegateM != null)
		{
		    ObjectDelM_ @delegate = createDelegateM__();
		    @delegate.copyFrom__(delegateM);
		    _delegate = @delegate;
		}
	    }
        }

        public int handleException__(LocalException ex, int cnt)
        {
            //
            // Only _delegate needs to be mutex protected here.
            //
            lock(this)
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

        public void handleExceptionWrapper__(IceInternal.LocalExceptionWrapper ex)
        {
            lock(this)
            {
                _delegate = null;
            }

            if(!ex.retry())
            {
                throw ex.get();
            }
        }

        public int handleExceptionWrapperRelaxed__(IceInternal.LocalExceptionWrapper ex, int cnt)
        {
            if(!ex.retry())
            {
                return handleException__(ex.get(), cnt);
            }
            else
            {
                lock(this)
                {
                    _delegate = null;
                }
                return cnt;
            }
        }

        public void checkTwowayOnly__(string name)
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

        public ObjectDel_ getDelegate__()
        {
            lock(this)
            {
                if(_delegate != null)
                {
		    return _delegate;
		}

		ObjectDel_ @delegate = null;
		if(_reference.getCollocationOptimization())
		{
		    ObjectAdapter adapter = _reference.getInstance().objectAdapterFactory().findObjectAdapter(this);
		    if(adapter != null)
		    {
			ObjectDelD_ d = createDelegateD__();
			d.setup(_reference, adapter);
			@delegate = d;
		    }
		}
		
		if(@delegate == null)
		{
		    ObjectDelM_ d = createDelegateM__();
		    d.setup(_reference);
		    @delegate = d;
		    
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
		    _delegate = @delegate;
		}
		
		return @delegate;
	    }
        }

        protected virtual ObjectDelM_ createDelegateM__()
        {
            return new ObjectDelM_();
        }

        protected virtual ObjectDelD_ createDelegateD__()
        {
            return new ObjectDelD_();
        }

        protected Context defaultContext__()
        {
            return _reference.getContext();
        }

        //
        // Only for use by IceInternal.ProxyFactory
        //
        public void setup(IceInternal.Reference @ref)
        {
            //
            // No need to synchronize, as this operation is only called
            // upon initial initialization.
            //

            Debug.Assert(_reference == null);
            Debug.Assert(_delegate == null);

            _reference = @ref;
        }

        private IceInternal.Reference _reference;
        private ObjectDel_ _delegate;
    }

    public class ObjectPrxHelper : ObjectPrxHelperBase
    {
        public static ObjectPrx checkedCast(Ice.ObjectPrx b)
        {
            return b;
        }

        public static ObjectPrx checkedCast(Ice.ObjectPrx b, Ice.Context ctx)
        {
            return b;
        }

        public static ObjectPrx checkedCast(Ice.ObjectPrx b, string f)
        {
            ObjectPrx d = null;
            if(b != null)
            {
                try
                {
                    Ice.ObjectPrx bb = b.ice_facet(f);
                    bool ok = bb.ice_isA("::Object");
                    Debug.Assert(ok);
                    ObjectPrxHelper h = new ObjectPrxHelper();
                    h.copyFrom__(bb);
                    d = h;
                }
                catch(Ice.FacetNotExistException)
                {
                }
            }
            return d;
        }

        public static ObjectPrx checkedCast(Ice.ObjectPrx b, string f, Ice.Context ctx)
        {
            ObjectPrx d = null;
            if(b != null)
            {
                try
                {
                    Ice.ObjectPrx bb = b.ice_facet(f);
                    bool ok = bb.ice_isA("::Object", ctx);
                    Debug.Assert(ok);
                    ObjectPrxHelper h = new ObjectPrxHelper();
                    h.copyFrom__(bb);
                    d = h;
                }
                catch(Ice.FacetNotExistException)
                {
                }
            }
            return d;
        }

        public static ObjectPrx uncheckedCast(Ice.ObjectPrx b)
        {
            return b;
        }

        public static ObjectPrx uncheckedCast(Ice.ObjectPrx b, string f)
        {
            ObjectPrx d = null;
            if(b != null)
            {
                Ice.ObjectPrx bb = b.ice_facet(f);
                ObjectPrxHelper h = new ObjectPrxHelper();
                h.copyFrom__(bb);
                d = h;
            }
            return d;
        }
    }

    public interface ObjectDel_
    {
	bool ice_isA(string id, Ice.Context context);
	void ice_ping(Ice.Context context);
	string[] ice_ids(Ice.Context context);
	string ice_id(Ice.Context context);
	bool ice_invoke(string operation, Ice.OperationMode mode, byte[] inParams, out byte[] outParams,
			Ice.Context context);

        ConnectionI getConnection__(out bool compress);
    }

    public class ObjectDelD_ : ObjectDel_
    {
        public virtual bool ice_isA(string id__, Ice.Context context__)
        {
            Current current__ = new Current();
            initCurrent__(ref current__, "ice_isA", OperationMode.Nonmutating, context__);
            while(true)
            {
                IceInternal.Direct direct__ = new IceInternal.Direct(current__);
                try
                {
                    return direct__.servant().ice_isA(id__, current__);
                }
                finally
                {
                    direct__.destroy();
                }
            }
        }
	
        public virtual void ice_ping(Ice.Context context__)
        {
            Current current__ = new Current();
            initCurrent__(ref current__, "ice_ping", OperationMode.Nonmutating, context__);
            while(true)
            {
                IceInternal.Direct direct__ = new IceInternal.Direct(current__);
                try
                {
                    direct__.servant().ice_ping(current__);
                    return;
                }
                finally
                {
                    direct__.destroy();
                }
            }
        }
	
        public virtual string[] ice_ids(Ice.Context context__)
        {
            Current current__ = new Current();
            initCurrent__(ref current__, "ice_ids", OperationMode.Nonmutating, context__);
            while(true)
            {
                IceInternal.Direct direct__ = new IceInternal.Direct(current__);
                try
                {
                    return direct__.servant().ice_ids(current__);
                }
                finally
                {
                    direct__.destroy();
                }
            }
        }
	
        public virtual string ice_id(Ice.Context context__)
        {
            Current current__ = new Current();
            initCurrent__(ref current__, "ice_id", OperationMode.Nonmutating, context__);
            while(true)
            {
                IceInternal.Direct direct__ = new IceInternal.Direct(current__);
                try
                {
                    return direct__.servant().ice_id(current__);
                }
                finally
                {
                    direct__.destroy();
                }
            }
        }
	
        public virtual bool ice_invoke(string operation, Ice.OperationMode mode, byte[] inParams,
            out byte[] outParams, Ice.Context context)
        {
            throw new CollocationOptimizationException();
        }
	
        public virtual ConnectionI getConnection__(out bool compress)
        {
            throw new CollocationOptimizationException();
        }

        //
        // Only for use by ObjectPrx.
        //
        internal void copyFrom__(ObjectDelD_ from)
        {
            //
            // No need to synchronize "from", as the delegate is immutable
            // after creation.
            //
	    
            //
            // No need to synchronize, as this operation is only called
            // upon initialization.
            //
	    
            Debug.Assert(reference__ == null);
            Debug.Assert(adapter__ == null);
	    
            reference__ = from.reference__;
            adapter__ = from.adapter__;
        }
	
        protected internal IceInternal.Reference reference__;
        protected internal Ice.ObjectAdapter adapter__;
	
        protected internal void initCurrent__(ref Current current, string op, Ice.OperationMode mode,
					      Ice.Context context)
        {
            current.adapter = adapter__;
            current.id = reference__.getIdentity();
            current.facet = reference__.getFacet();
            current.operation = op;
            current.mode = mode;
            current.ctx = context;
	    current.requestId = -1;
        }
	
        public virtual void setup(IceInternal.Reference rf, Ice.ObjectAdapter adapter)
        {
            //
            // No need to synchronize, as this operation is only called
            // upon initialization.
            //
	    
            Debug.Assert(reference__ == null);
            Debug.Assert(adapter__ == null);
	    
            reference__ = rf;
            adapter__ = adapter;
        }
    }
	    
    public class ObjectDelM_ : ObjectDel_
    {
        public virtual bool ice_isA(string id__, Ice.Context context__)
        {
            IceInternal.Outgoing og__ = getOutgoing("ice_isA", OperationMode.Nonmutating, context__);
            try
            {
		try
		{
		    IceInternal.BasicStream os__ = og__.ostr();
		    os__.writeString(id__);
		}
		catch(LocalException ex__)
		{
		    og__.abort(ex__);
		}
		bool ok__ = og__.invoke();
		try
		{
		    IceInternal.BasicStream is__ = og__.istr();
		    if(!ok__)
		    {
			try
			{
			    is__.throwException();
			}
			catch(UserException)
			{
			    throw new UnknownUserException();
			}
		    }
		    return is__.readBool();
		}
		catch(LocalException ex__)
		{
		    throw new IceInternal.LocalExceptionWrapper(ex__, false);
		}
            }
            finally
            {
                reclaimOutgoing(og__);
            }
        }
	
        public virtual void ice_ping(Ice.Context context__)
        {
            IceInternal.Outgoing og__ = getOutgoing("ice_ping", OperationMode.Nonmutating, context__);
            try
            {
		bool ok__ = og__.invoke();
		try
		{
		    IceInternal.BasicStream is__ = og__.istr();
		    if(!ok__)
		    {
			try
			{
			    is__.throwException();
			}
			catch(UserException)
			{
			    throw new UnknownUserException();
			}
		    }
		}
		catch(LocalException ex__)
		{
		    throw new IceInternal.LocalExceptionWrapper(ex__, false);
		}
            }
            finally
            {
                reclaimOutgoing(og__);
            }
        }
	
        public virtual string[] ice_ids(Ice.Context context__)
        {
            IceInternal.Outgoing og__ = getOutgoing("ice_ids", OperationMode.Nonmutating, context__);
            try
            {
		bool ok__ = og__.invoke();
		try
		{
		    IceInternal.BasicStream is__ = og__.istr();
		    if(!ok__)
		    {
			try
			{
			    is__.throwException();
			}
			catch(UserException)
			{
			    throw new UnknownUserException();
			}
		    }
		    return is__.readStringSeq();
		}
		catch(LocalException ex__)
		{
		    throw new IceInternal.LocalExceptionWrapper(ex__, false);
		}
            }
            finally
            {
                reclaimOutgoing(og__);
            }
        }
	
        public virtual string ice_id(Ice.Context context__)
        {
            IceInternal.Outgoing og__ = getOutgoing("ice_id", OperationMode.Nonmutating, context__);
            try
            {
		bool ok__ = og__.invoke();
		try
		{
		    IceInternal.BasicStream is__ = og__.istr();
		    if(!ok__)
		    {
			try
			{
			    is__.throwException();
			}
			catch(UserException)
			{
			    throw new UnknownUserException();
			}
		    }
		    return is__.readString();
		}
		catch(LocalException ex__)
		{
		    throw new IceInternal.LocalExceptionWrapper(ex__, false);
		}
            }
            finally
            {
                reclaimOutgoing(og__);
            }
        }
	
        public virtual bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams,
                                       Ice.Context context__)
        {
            IceInternal.Outgoing og__ = getOutgoing(operation, mode, context__);
            try
            {
		try
		{
		    IceInternal.BasicStream os__ = og__.ostr();
		    os__.writeBlob(inParams);
		}
		catch(LocalException ex__)
		{
		    og__.abort(ex__);
		}
                bool ok = og__.invoke();
                outParams = null;
                if(reference__.getMode() == IceInternal.Reference.Mode.ModeTwoway)
                {
                    try
                    {
                        IceInternal.BasicStream is__ = og__.istr();
                        int sz = is__.getReadEncapsSize();
                        outParams = is__.readBlob(sz);
                    }
                    catch(LocalException ex__)
                    {
                        throw new IceInternal.LocalExceptionWrapper(ex__, false);
                    }
                }
                return ok;
            }
            finally
            {
                reclaimOutgoing(og__);
            }
        }
	
        public virtual ConnectionI getConnection__(out bool compress)
        {
	    compress = compress__;
            return connection__;
        }
	
        //
        // Only for use by ObjectPrx
        //
        internal void copyFrom__(ObjectDelM_ from)
        {
            //
            // No need to synchronize "from", as the delegate is immutable
            // after creation.
            //
	    
            //
            // No need to synchronize, as this operation is only called
            // upon initialization.
            //
	    
            Debug.Assert(reference__ == null);
            Debug.Assert(connection__ == null);
	    
            reference__ = from.reference__;
            connection__ = from.connection__;
	    compress__ = from.compress__;
        }
	
        protected IceInternal.Reference reference__;
        protected ConnectionI connection__;
        protected bool compress__;

        public virtual void setup(IceInternal.Reference rf)
        {
            //
            // No need to synchronize, as this operation is only called
            // upon initialization.
            //
	    
            Debug.Assert(reference__ == null);
            Debug.Assert(connection__ == null);
	    
            reference__ = rf;
            connection__ = reference__.getConnection(out compress__);
        }
	
        protected IceInternal.Outgoing getOutgoing(string operation, OperationMode mode, Ice.Context context)
        {
	    return connection__.getOutgoing(reference__, operation, mode, context, compress__);
        }

        protected void reclaimOutgoing(IceInternal.Outgoing outg)
        {
	    connection__.reclaimOutgoing(outg);
        }
    }
}
