// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using IceUtilInternal;

namespace Ice
{
    public interface ObjectPrx
    {
        int ice_getHash();

        Communicator ice_getCommunicator();

        string ice_toString();

        bool ice_isA(string id__);
        bool ice_isA(string id__, Dictionary<string, string> context__);

        void ice_ping();
        void ice_ping(Dictionary<string, string> context__);

        string[] ice_ids();
        string[] ice_ids(Dictionary<string, string> context__);

        string ice_id();
        string ice_id(Dictionary<string, string> context__);

        // Returns true if ok, false if user exception.
        bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams);
        bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams,
            Dictionary<string, string> context__);

        bool ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inParams);
        bool ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inParams,
            Dictionary<string, string> context);

        Identity ice_getIdentity();
        ObjectPrx ice_identity(Identity newIdentity);
        
        Dictionary<string, string> ice_getContext();
        ObjectPrx ice_context(Dictionary<string, string> newContext);
        [Obsolete("This method is deprecated.")]
        ObjectPrx ice_defaultContext();
        
        string ice_getFacet();
        ObjectPrx ice_facet(string newFacet);

        string ice_getAdapterId();
        ObjectPrx ice_adapterId(string newAdapterId);

        Endpoint[] ice_getEndpoints();
        ObjectPrx ice_endpoints(Endpoint[] newEndpoints);

        int ice_getLocatorCacheTimeout();
        ObjectPrx ice_locatorCacheTimeout(int timeout);
        
        bool ice_isConnectionCached();
        ObjectPrx ice_connectionCached(bool newCache);

        EndpointSelectionType ice_getEndpointSelection();
        ObjectPrx ice_endpointSelection(EndpointSelectionType newType);

        bool ice_isSecure();
        ObjectPrx ice_secure(bool b);

        bool ice_isPreferSecure();
        ObjectPrx ice_preferSecure(bool b);

        Ice.RouterPrx ice_getRouter();
        ObjectPrx ice_router(Ice.RouterPrx router);

        Ice.LocatorPrx ice_getLocator();
        ObjectPrx ice_locator(Ice.LocatorPrx locator);

        bool ice_isCollocationOptimized();
        ObjectPrx ice_collocationOptimized(bool b);

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

        void ice_flushBatchRequests();
        bool ice_flushBatchRequests_async(AMI_Object_ice_flushBatchRequests cb);

        Connection ice_getConnection();
        Connection ice_getCachedConnection();
    }

    public class ObjectPrxHelperBase : ObjectPrx
    {
        public override int GetHashCode()
        {
            return _reference.GetHashCode();
        }

        public int ice_getHash()
        {
            return _reference.GetHashCode();
        }

        public Communicator ice_getCommunicator()
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
            return ice_isA(id__, null, false);
        }

        public bool ice_isA(string id__, Dictionary<string, string> context__)
        {
            return ice_isA(id__, context__, true);
        }

        private bool ice_isA(string id__, Dictionary<string, string> context__, bool explicitContext__)
        {
            if(explicitContext__ && context__ == null)
            {
                context__ = emptyContext_;
            }

            int cnt__ = 0;
            while(true)
            {
                ObjectDel_ del__ = null;
                try
                {
                    checkTwowayOnly__("ice_isA");
                    del__ = getDelegate__(false);
                    return del__.ice_isA(id__, context__);
                }
                catch(IceInternal.LocalExceptionWrapper ex__)
                {
                    handleExceptionWrapperRelaxed__(del__, ex__, null, ref cnt__);
                }
                catch(LocalException ex__)
                {
                    handleException__(del__, ex__, null, ref cnt__);
                }
            }
        }

        public void ice_ping()
        {
            ice_ping(null, false);
        }

        public void ice_ping(Dictionary<string, string> context__)
        {
            ice_ping(context__, true);
        }

        private void ice_ping(Dictionary<string, string> context__, bool explicitContext__)
        {
            if(explicitContext__ && context__ == null)
            {
                context__ = emptyContext_;
            }

            int cnt__ = 0;
            while(true)
            {
                ObjectDel_ del__ = null;
                try
                {
                    del__ = getDelegate__(false);
                    del__.ice_ping(context__);
                    return;
                }
                catch(IceInternal.LocalExceptionWrapper ex__)
                {
                    handleExceptionWrapperRelaxed__(del__, ex__, null, ref cnt__);
                }
                catch(LocalException ex__)
                {
                    handleException__(del__, ex__, null, ref cnt__);
                }
            }
        }

        public string[] ice_ids()
        {
            return ice_ids(null, false);
        }

        public string[] ice_ids(Dictionary<string, string> context__)
        {
            return ice_ids(context__, true);
        }

        private string[] ice_ids(Dictionary<string, string> context__, bool explicitContext__)
        {
            if(explicitContext__ && context__ == null)
            {
                context__ = emptyContext_;
            }
            int cnt__ = 0;
            while(true)
            {
                ObjectDel_ del__ = null;
                try
                {
                    checkTwowayOnly__("ice_ids");
                    del__ = getDelegate__(false);
                    return del__.ice_ids(context__);
                }
                catch(IceInternal.LocalExceptionWrapper ex__)
                {
                    handleExceptionWrapperRelaxed__(del__, ex__, null, ref cnt__);
                }
                catch(LocalException ex__)
                {
                    handleException__(del__, ex__, null, ref cnt__);
                }
            }
        }

        public string ice_id()
        {
            return ice_id(null, false);
        }

        public string ice_id(Dictionary<string, string> context__)
        {
             return ice_id(context__, true);
        }

        private string ice_id(Dictionary<string, string> context__, bool explicitContext__)
        {
            if(explicitContext__ && context__ == null)
            {
                context__ = emptyContext_;
            }
            int cnt__ = 0;
            while(true)
            {
                ObjectDel_ del__ = null;
                try
                {
                    checkTwowayOnly__("ice_id");
                    del__ = getDelegate__(false);
                    return del__.ice_id(context__);
                }
                catch(IceInternal.LocalExceptionWrapper ex__)
                {
                    handleExceptionWrapperRelaxed__(del__, ex__, null, ref cnt__);
                }
                catch(LocalException ex__)
                {
                    handleException__(del__, ex__, null, ref cnt__);
                }
            }
        }

        public bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams)
        {
            return ice_invoke(operation, mode, inParams, out outParams, null, false);
        }

        public bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams,
                               Dictionary<string, string> context)
        {
            return ice_invoke(operation, mode, inParams, out outParams, context, true);
        }
            
        private bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams,
                                Dictionary<string, string> context,  bool explicitContext)
        {
            if(explicitContext && context == null)
            {
                context = emptyContext_;
            }

            int cnt__ = 0;
            while(true)
            {
                ObjectDel_ del__ = null;
                try
                {
                    del__ = getDelegate__(false);
                    return del__.ice_invoke(operation, mode, inParams, out outParams, context);
                }
                catch(IceInternal.LocalExceptionWrapper ex__)
                {
                    if(mode == OperationMode.Nonmutating || mode == OperationMode.Idempotent)
                    {
                        handleExceptionWrapperRelaxed__(del__, ex__, null, ref cnt__);
                    }
                    else
                    {
                        handleExceptionWrapper__(del__, ex__, null);
                    }
                }
                catch(LocalException ex__)
                {
                    handleException__(del__, ex__, null, ref cnt__);
                }
            }
        }

        public bool ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inParams)
        {
            checkTwowayOnly__("ice_invoke_async");
            return cb.invoke__(this, operation, mode, inParams, null);
        }

        public bool ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inParams,
                                     Dictionary<string, string> context)
        {
            if(context == null)
            {
                context = emptyContext_;
            }
            checkTwowayOnly__("ice_invoke_async");
            return cb.invoke__(this, operation, mode, inParams, context);
        }

        public Identity ice_getIdentity()
        {
            return _reference.getIdentity();
        }

        public ObjectPrx ice_identity(Identity newIdentity)
        {
            if(newIdentity.name.Equals(""))
            {
                throw new IllegalIdentityException();
            }
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

        public Dictionary<string, string> ice_getContext()
        {
            return _reference.getContext();
        }

        public ObjectPrx ice_context(Dictionary<string, string> newContext)
        {
            return newInstance(_reference.changeContext(newContext));
        }

        public ObjectPrx ice_defaultContext()
        {
            return newInstance(_reference.defaultContext());
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
                return newInstance(_reference.changeAdapterId(newAdapterId));
            }
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
                ArrayList arr = ArrayList.Adapter(newEndpoints);
                IceInternal.EndpointI[] endpts = (IceInternal.EndpointI[])arr.ToArray(typeof(IceInternal.EndpointI));
                return newInstance(_reference.changeEndpoints(endpts));
            }
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
                return newInstance(_reference.changeLocatorCacheTimeout(newTimeout));
            }
        }

        public bool ice_isConnectionCached()
        {
            return _reference.getCacheConnection();
        }

        public ObjectPrx ice_connectionCached(bool newCache)
        {
            if(newCache == _reference.getCacheConnection())
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changeCacheConnection(newCache));
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
                return newInstance(_reference.changeEndpointSelection(newType));
            }
        }

        public bool ice_isSecure()
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
                return newInstance(_reference.changeSecure(b));
            }
        }

        public bool ice_isPreferSecure()
        {
            return _reference.getPreferSecure();
        }

        public ObjectPrx ice_preferSecure(bool b)
        {
            if(b == _reference.getPreferSecure())
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changePreferSecure(b));
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
                return newInstance(@ref);
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
                return newInstance(@ref);
            }
        }

        public bool ice_isCollocationOptimized()
        {
            return _reference.getCollocationOptimized();
        }

        public ObjectPrx ice_collocationOptimized(bool b)
        {
            if(b == _reference.getCollocationOptimized())
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changeCollocationOptimized(b));
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
                return newInstance(_reference.changeMode(IceInternal.Reference.Mode.ModeTwoway));
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
                return newInstance(_reference.changeMode(IceInternal.Reference.Mode.ModeOneway));
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
                return newInstance(_reference.changeMode(IceInternal.Reference.Mode.ModeBatchOneway));
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
                return newInstance(_reference.changeMode(IceInternal.Reference.Mode.ModeDatagram));
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
                return newInstance(_reference.changeMode(IceInternal.Reference.Mode.ModeBatchDatagram));
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
                return newInstance(@ref);
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
                return newInstance(@ref);
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
                return newInstance(@ref);
            }
        }

        public Connection ice_getConnection()
        {
            int cnt__ = 0;
            while(true)
            {
                ObjectDel_ del__ = null;
                try
                {
                    del__ = getDelegate__(false);
                    // Wait for the connection to be established.
                    return del__.getRequestHandler__().getConnection(true);
                }
                catch(LocalException ex__)
                {
                    handleException__(del__, ex__, null, ref cnt__);
                }
            }
        }

        public Connection ice_getCachedConnection()
        {
            ObjectDel_ del__ = null;
            lock(this)
            {
                del__ = _delegate;
            }

            if(del__ != null)
            {
                try
                {
                    // Wait for the connection to be established.
                    return del__.getRequestHandler__().getConnection(false);
                }
                catch(CollocationOptimizationException)
                {
                }
            }
            return null;
        }

        public void ice_flushBatchRequests()
        {
            //
            // We don't automatically retry if ice_flushBatchRequests fails. Otherwise, if some batch
            // requests were queued with the connection, they would be lost without being noticed.
            //
            ObjectDel_ del__ = null;
            int cnt__ = -1; // Don't retry.
            try
            {
                del__ = getDelegate__(false);
                del__.ice_flushBatchRequests();
                return;
            }
            catch(LocalException ex__)
            {
                handleException__(del__, ex__, null, ref cnt__);
            }
        }

        public bool ice_flushBatchRequests_async(AMI_Object_ice_flushBatchRequests cb)
        {
            return cb.invoke__(this);
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

        public void handleException__(ObjectDel_ @delegate, 
                                      LocalException ex, 
                                      IceInternal.OutgoingAsync outAsync,
                                      ref int cnt)
        {
            //
            // Only _delegate needs to be mutex protected here.
            //
            lock(this)
            {
                if(@delegate == _delegate)
                {
                    _delegate = null;
                }
            }

            if(cnt == -1) // Don't retry if the retry count is -1.
            {
                throw ex;
            }


            try
            {
                _reference.getInstance().proxyFactory().checkRetryAfterException(ex, _reference, outAsync, ref cnt);
            }
            catch(CommunicatorDestroyedException)
            {
                //
                // The communicator is already destroyed, so we cannot
                // retry.
                //
                throw ex;
            }
        }

        public void handleExceptionWrapper__(ObjectDel_ @delegate, IceInternal.LocalExceptionWrapper ex, 
                                             IceInternal.OutgoingAsync outAsync)
        {
            lock(this)
            {
                if(@delegate == _delegate)
                {
                    _delegate = null;
                }
            }

            if(!ex.retry())
            {
                throw ex.get();
            }

            if(outAsync != null)
            {
                outAsync.send__();
            }
        }

        public void handleExceptionWrapperRelaxed__(ObjectDel_ @delegate, IceInternal.LocalExceptionWrapper ex, 
                                                    IceInternal.OutgoingAsync outAsync, ref int cnt)
        {
            if(!ex.retry())
            {
                handleException__(@delegate, ex.get(), outAsync, ref cnt);
            }
            else
            {
                lock(this)
                {
                    if(@delegate == _delegate)
                    {
                        _delegate = null;
                    }
                }

                if(outAsync != null)
                {
                    outAsync.send__();
                }
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

        public ObjectDel_ getDelegate__(bool ami)
        {
            if(_reference.getCacheConnection())
            {
                lock(this)
                {
                    if(_delegate != null)
                    {
                        return _delegate;
                    }
                    // Connect asynchrously to avoid blocking with the proxy mutex locked.
                    _delegate = createDelegate(true);
                    return _delegate;
                }
            }
            else
            {
                IceInternal.Reference.Mode mode = _reference.getMode();
                return createDelegate(ami || 
                                      mode == IceInternal.Reference.Mode.ModeBatchOneway ||
                                      mode == IceInternal.Reference.Mode.ModeBatchDatagram);
            }
        }

        public void setRequestHandler__(ObjectDel_ @delegate, IceInternal.RequestHandler handler)
        {
            lock(this)
            {
                if(@delegate == _delegate)
                {
                    if(_delegate is ObjectDelM_)
                    {
                        _delegate = createDelegateM__();
                        _delegate.setRequestHandler__(handler);
                    }
                    else if(_delegate is ObjectDelD_)
                    {
                        _delegate = createDelegateD__();
                        _delegate.setRequestHandler__(handler);
                    }
                }
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

        private ObjectDel_ createDelegate(bool async)
        {
            if(_reference.getCollocationOptimized())
            {
                ObjectAdapter adapter = _reference.getInstance().objectAdapterFactory().findObjectAdapter(this);
                if(adapter != null)
                {
                    ObjectDelD_ d = createDelegateD__();
                    d.setup(_reference, adapter);
                    return d;
                }
            }

            ObjectDelM_ d2 = createDelegateM__();
            d2.setup(_reference, this, async);
            return d2;
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

        private ObjectPrxHelperBase newInstance(IceInternal.Reference @ref)
        {
            ObjectPrxHelperBase proxy = (ObjectPrxHelperBase)Activator.CreateInstance(GetType());
            proxy.setup(@ref);
            return proxy;
        }

        protected static Dictionary<string, string> emptyContext_ = new Dictionary<string, string>();
        private IceInternal.Reference _reference;
        private ObjectDel_ _delegate;
    }

    public class ObjectPrxHelper : ObjectPrxHelperBase
    {
        public static ObjectPrx checkedCast(Ice.ObjectPrx b)
        {
            return b;
        }

        public static ObjectPrx checkedCast(Ice.ObjectPrx b, Dictionary<string, string> ctx)
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
                    bool ok = bb.ice_isA("::Ice::Object");
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

        public static ObjectPrx checkedCast(Ice.ObjectPrx b, string f, Dictionary<string, string> ctx)
        {
            ObjectPrx d = null;
            if(b != null)
            {
                try
                {
                    Ice.ObjectPrx bb = b.ice_facet(f);
                    bool ok = bb.ice_isA("::Ice::Object", ctx);
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
        bool ice_isA(string id, Dictionary<string, string> context);
        void ice_ping(Dictionary<string, string> context);
        string[] ice_ids(Dictionary<string, string> context);
        string ice_id(Dictionary<string, string> context);
        bool ice_invoke(string operation, Ice.OperationMode mode, byte[] inParams, out byte[] outParams,
                        Dictionary<string, string> context);

        void ice_flushBatchRequests();

        IceInternal.RequestHandler getRequestHandler__();
        void setRequestHandler__(IceInternal.RequestHandler handler);
    }

    public class ObjectDelD_ : ObjectDel_
    {
        public virtual bool ice_isA(string id__, Dictionary<string, string> context__)
        {
            Current current__ = new Current();
            initCurrent__(ref current__, "ice_isA", OperationMode.Nonmutating, context__);

            bool result__ = false;
            IceInternal.Direct.RunDelegate run__ = delegate(Ice.Object servant__)
            {
                result__ = servant__.ice_isA(id__, current__);
                return Ice.DispatchStatus.DispatchOK;
            };

            IceInternal.Direct direct__ = null;
            try
            {
                direct__ = new IceInternal.Direct(current__, run__);
            }
            catch(System.Exception ex__)
            {
                IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
            }

            try
            {
                DispatchStatus status__ = direct__.servant().collocDispatch__(direct__); 
                Debug.Assert(status__ == DispatchStatus.DispatchOK);
                return result__;
            }
            finally
            {
                try
                {
                    direct__.destroy();
                }
                catch(System.Exception ex__)
                {
                    IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
                }
            }
        }
        
        public virtual void ice_ping(Dictionary<string, string> context__)
        {
            Current current__ = new Current();
            initCurrent__(ref current__, "ice_ping", OperationMode.Nonmutating, context__);
         
            IceInternal.Direct.RunDelegate run__ = delegate(Ice.Object servant__)
            {
                servant__.ice_ping(current__);
                return Ice.DispatchStatus.DispatchOK;
            };

            IceInternal.Direct direct__ = null;
            try
            {
                direct__ = new IceInternal.Direct(current__, run__);
            }
            catch(System.Exception ex__)
            {
                IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
            }
            
            try
            {
                DispatchStatus status__ = direct__.servant().collocDispatch__(direct__); 
                Debug.Assert(status__ == DispatchStatus.DispatchOK);
            }
            finally
            {
                try
                {
                    direct__.destroy();
                }
                catch(System.Exception ex__)
                {
                    IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
                }
            }
        }
        
        public virtual string[] ice_ids(Dictionary<string, string> context__)
        {
            Current current__ = new Current();
            initCurrent__(ref current__, "ice_ids", OperationMode.Nonmutating, context__);

            string[] result__ = null;
            IceInternal.Direct.RunDelegate run__ = delegate(Ice.Object servant__)
            {
                result__ = servant__.ice_ids(current__);
                return Ice.DispatchStatus.DispatchOK;
            };

            IceInternal.Direct direct__ = null;
            try
            {
                direct__ = new IceInternal.Direct(current__, run__);
            }
            catch(System.Exception ex__)
            {
                IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
            }
            
            try
            {
                DispatchStatus status__ = direct__.servant().collocDispatch__(direct__); 
                Debug.Assert(status__ == DispatchStatus.DispatchOK);
                return result__;
            }
            finally
            {
                try
                {
                    direct__.destroy();
                }
                catch(System.Exception ex__)
                {
                    IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
                }
            }
        }
        
        public virtual string ice_id(Dictionary<string, string> context__)
        {
            Current current__ = new Current();
            initCurrent__(ref current__, "ice_id", OperationMode.Nonmutating, context__);
            
            string result__ = null;
            IceInternal.Direct.RunDelegate run__ = delegate(Ice.Object servant__)
            {
                result__ = servant__.ice_id(current__);
                return Ice.DispatchStatus.DispatchOK;
            };

            IceInternal.Direct direct__ = null;
            try
            {
                direct__ = new IceInternal.Direct(current__, run__);
            }
            catch(System.Exception ex__)
            {
                IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
            }
            
            try
            {
                DispatchStatus status__ = direct__.servant().collocDispatch__(direct__); 
                Debug.Assert(status__ == DispatchStatus.DispatchOK);
                return result__;
            }
            finally
            {
                try
                {
                    direct__.destroy();
                }
                catch(System.Exception ex__)
                {
                    IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
                }
            }
        }
        
        public virtual bool ice_invoke(string operation, Ice.OperationMode mode, byte[] inParams,
                                       out byte[] outParams, Dictionary<string, string> context)
        {
            throw new CollocationOptimizationException();
        }

        public virtual void ice_flushBatchRequests()
        {
            throw new CollocationOptimizationException();
        }

        public virtual IceInternal.RequestHandler getRequestHandler__()
        {
            throw new CollocationOptimizationException();
        }

        public virtual void setRequestHandler__(IceInternal.RequestHandler handler)
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
                                              Dictionary<string, string> context)
        {
            current.adapter = adapter__;
            current.id = reference__.getIdentity();
            current.facet = reference__.getFacet();
            current.operation = op;
            current.mode = mode;

            if(context != null)
            {
                current.ctx = context;
            }
            else
            {
                //
                // Implicit context
                //
                ImplicitContextI implicitContext =
                    reference__.getInstance().getImplicitContext();
            
                Dictionary<string, string> prxContext = reference__.getContext();
                
                if(implicitContext == null)
                {
                    current.ctx = new Dictionary<string, string>(prxContext);
                }
                else
                {
                    current.ctx = implicitContext.combine(prxContext);
                }
            }
            
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
        public virtual bool ice_isA(string id__, Dictionary<string, string> context__)
        {
            IceInternal.Outgoing og__ = handler__.getOutgoing("ice_isA", OperationMode.Nonmutating, context__);
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
                    if(!ok__)
                    {
                        try
                        {
                            og__.throwUserException();
                        }
                        catch(UserException ex)
                        {
                            throw new UnknownUserException(ex.ice_name(), ex);
                        }
                    }
                    IceInternal.BasicStream is__ = og__.istr();
                    is__.startReadEncaps();
                    bool ret__ = is__.readBool();
                    is__.endReadEncaps();
                    return ret__;
                }
                catch(LocalException ex__)
                {
                    throw new IceInternal.LocalExceptionWrapper(ex__, false);
                }
            }
            finally
            {
                handler__.reclaimOutgoing(og__);
            }
        }
        
        public virtual void ice_ping(Dictionary<string, string> context__)
        {
            IceInternal.Outgoing og__ = handler__.getOutgoing("ice_ping", OperationMode.Nonmutating, context__);
            try
            {
                bool ok__ = og__.invoke();
                if(!og__.istr().isEmpty())
                {
                    try
                    {
                        if(!ok__)
                        {
                            try
                            {
                                og__.throwUserException();
                            }
                            catch(UserException ex)
                            {
                                throw new UnknownUserException(ex.ice_name(), ex);
                            }
                        }
                        og__.istr().skipEmptyEncaps();
                    }
                    catch(LocalException ex__)
                    {
                        throw new IceInternal.LocalExceptionWrapper(ex__, false);
                    }
                }
            }
            finally
            {
                handler__.reclaimOutgoing(og__);
            }
        }
        
        public virtual string[] ice_ids(Dictionary<string, string> context__)
        {
            IceInternal.Outgoing og__ = handler__.getOutgoing("ice_ids", OperationMode.Nonmutating, context__);
            try
            {
                bool ok__ = og__.invoke();
                try
                {
                    if(!ok__)
                    {
                        try
                        {
                            og__.throwUserException();
                        }
                        catch(UserException ex)
                        {
                            throw new UnknownUserException(ex.ice_name(), ex);
                        }
                    }
                    IceInternal.BasicStream is__ = og__.istr();
                    is__.startReadEncaps();
                    string[] ret__ = is__.readStringSeq();
                    is__.endReadEncaps();
                    return ret__;
                }
                catch(LocalException ex__)
                {
                    throw new IceInternal.LocalExceptionWrapper(ex__, false);
                }
            }
            finally
            {
                handler__.reclaimOutgoing(og__);
            }
        }
        
        public virtual string ice_id(Dictionary<string, string> context__)
        {
            IceInternal.Outgoing og__ = handler__.getOutgoing("ice_id", OperationMode.Nonmutating, context__);
            try
            {
                bool ok__ = og__.invoke();
                try
                {
                    if(!ok__)
                    {
                        try
                        {
                            og__.throwUserException();
                        }
                        catch(UserException ex)
                        {
                            throw new UnknownUserException(ex.ice_name(), ex);
                        }
                    }
                    IceInternal.BasicStream is__ = og__.istr();
                    is__.startReadEncaps();
                    string ret__ = is__.readString();
                    is__.endReadEncaps();
                    return ret__;
                }
                catch(LocalException ex__)
                {
                    throw new IceInternal.LocalExceptionWrapper(ex__, false);
                }
            }
            finally
            {
                handler__.reclaimOutgoing(og__);
            }
        }

        public virtual bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams,
                                       Dictionary<string, string> context__)
        {
            IceInternal.Outgoing og__ = handler__.getOutgoing(operation, mode, context__);
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
                if(handler__.getReference().getMode() == IceInternal.Reference.Mode.ModeTwoway)
                {
                    try
                    {
                        IceInternal.BasicStream is__ = og__.istr();
                        is__.startReadEncaps();
                        int sz = is__.getReadEncapsSize();
                        outParams = is__.readBlob(sz);
                        is__.endReadEncaps();
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
                handler__.reclaimOutgoing(og__);
            }
        }
        
        public virtual void ice_flushBatchRequests()
        {
            IceInternal.BatchOutgoing @out = new IceInternal.BatchOutgoing(handler__);
            @out.invoke();
        }

        public virtual IceInternal.RequestHandler getRequestHandler__()
        {
            return handler__;
        }

        public virtual void setRequestHandler__(IceInternal.RequestHandler handler)
        {
            handler__ = handler;
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

            Debug.Assert(handler__ == null);

            handler__ = from.handler__;
        }
        
        protected IceInternal.RequestHandler handler__;

        public virtual void setup(IceInternal.Reference rf, Ice.ObjectPrx proxy, bool async)
        {
            //
            // No need to synchronize, as this operation is only called
            // upon initialization.
            //

            Debug.Assert(handler__ == null);

            if(async)
            {
                IceInternal.ConnectRequestHandler handler = new IceInternal.ConnectRequestHandler(rf, proxy, this);
                handler__ = handler.connect();
            }
            else
            {
                handler__ = new IceInternal.ConnectionRequestHandler(rf, proxy);
            }
        }
    }
}
