// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

        bool ice_isA(string __id);
        bool ice_isA(string __id, Context __context);

        void ice_ping();
        void ice_ping(Context __context);

        string[] ice_ids();
        string[] ice_ids(Context __context);

        string ice_id();
        string ice_id(Context __context);

        // Returns true if ok, false if user exception.
        bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams);
        bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams,
            Context __context);

        void ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inParams);
        void ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inParams,
            Context context);

        Identity ice_getIdentity();
        ObjectPrx ice_newIdentity(Identity newIdentity);

        Context ice_getContext();
        ObjectPrx ice_newContext(Context newContext);
        ObjectPrx ice_defaultContext();

        string ice_getFacet();
        ObjectPrx ice_newFacet(string newFacet);

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

        ObjectPrx ice_secure(bool b);
        ObjectPrx ice_compress(bool co);
        ObjectPrx ice_timeout(int t);
        ObjectPrx ice_router(Ice.RouterPrx router);
        ObjectPrx ice_locator(Ice.LocatorPrx locator);
        ObjectPrx ice_collocationOptimization(bool b);
        ObjectPrx ice_default();

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

        public bool ice_isA(string __id)
        {
            return ice_isA(__id, _reference.getContext());
        }

        public bool ice_isA(string __id, Context __context)
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

        public void ice_ping()
        {
            ice_ping(_reference.getContext());
        }

        public void ice_ping(Context __context)
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

        public string[] ice_ids()
        {
            return ice_ids(_reference.getContext());
        }

        public string[] ice_ids(Context __context)
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

        public string ice_id()
        {
            return ice_id(_reference.getContext());
        }

        public string ice_id(Context __context)
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

        public bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams)
        {
            return ice_invoke(operation, mode, inParams, out outParams, _reference.getContext());
        }

        public bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams,
            Context context)
        {
            int __cnt = 0;
            while(true)
            {
                try
                {
                    _ObjectDel __del = __getDelegate();
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

        public void ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inParams)
        {
            ice_invoke_async(cb, operation, mode, inParams, _reference.getContext());
        }

        public void ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inParams, Context context)
        {
            __checkTwowayOnly("ice_invoke_async");
            cb.__invoke(this, operation, mode, inParams, context);
        }

        public Identity ice_getIdentity()
        {
            return _reference.getIdentity();
        }

        public ObjectPrx ice_newIdentity(Identity newIdentity)
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

        public Context ice_getContext()
        {
            return _reference.getContext();
        }

        public ObjectPrx ice_newContext(Context newContext)
        {
            if(_reference.hasContext() && newContext != null && newContext.Equals(_reference.getContext()))
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

	public ObjectPrx ice_defaultContext()
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

        public string ice_getFacet()
        {
            return _reference.getFacet();
        }

        public ObjectPrx ice_newFacet(string newFacet)
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

        public ObjectPrx ice_twoway()
        {
            IceInternal.Reference @ref = _reference.changeMode(IceInternal.Reference.Mode.ModeTwoway);
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

        public bool ice_isTwoway()
        {
            return _reference.getMode() == IceInternal.Reference.Mode.ModeTwoway;
        }

        public ObjectPrx ice_oneway()
        {
            IceInternal.Reference @ref = _reference.changeMode(IceInternal.Reference.Mode.ModeOneway);
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

        public bool ice_isOneway()
        {
            return _reference.getMode() == IceInternal.Reference.Mode.ModeOneway;
        }

        public ObjectPrx ice_batchOneway()
        {
            IceInternal.Reference @ref = _reference.changeMode(IceInternal.Reference.Mode.ModeBatchOneway);
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

        public bool ice_isBatchOneway()
        {
            return _reference.getMode() == IceInternal.Reference.Mode.ModeBatchOneway;
        }

        public ObjectPrx ice_datagram()
        {
            IceInternal.Reference @ref = _reference.changeMode(IceInternal.Reference.Mode.ModeDatagram);
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

        public bool ice_isDatagram()
        {
            return _reference.getMode() == IceInternal.Reference.Mode.ModeDatagram;
        }

        public ObjectPrx ice_batchDatagram()
        {
            IceInternal.Reference @ref = _reference.changeMode(IceInternal.Reference.Mode.ModeBatchDatagram);
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

        public bool ice_isBatchDatagram()
        {
            return _reference.getMode() == IceInternal.Reference.Mode.ModeBatchDatagram;
        }

        public ObjectPrx ice_secure(bool b)
        {
            IceInternal.Reference @ref = _reference.changeSecure(b);
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

        public ObjectPrx ice_collocationOptimization(bool b)
        {
            IceInternal.Reference @ref = _reference.changeCollocationOptimization(b);
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

        public ObjectPrx ice_default()
        {
            IceInternal.Reference @ref = _reference.changeDefault();
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

        public override bool Equals(object r)
        {
            ObjectPrxHelperBase rhs = r as ObjectPrxHelperBase;
            return rhs == null ? false : _reference.Equals(rhs._reference);
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

        public IceInternal.Reference __reference()
        {
            return _reference;
        }

        public void __copyFrom(ObjectPrx from)
        {
            ObjectPrxHelperBase h = (ObjectPrxHelperBase)from;
            IceInternal.Reference @ref = null;
            _ObjectDelM delegateM = null;
            _ObjectDelD delegateD = null;

            lock(from)
            {
                @ref = h._reference;
                delegateM = h._delegate as _ObjectDelM;
                delegateD = h._delegate as _ObjectDelD;
            }

            //
            // No need to synchronize "*this", as this operation is only
            // called upon initialization.
            //

            Debug.Assert(_reference == null);
            Debug.Assert(_delegate == null);

            _reference = @ref;

            if(delegateD != null)
            {
                _ObjectDelD @delegate = __createDelegateD();
                @delegate.__copyFrom(delegateD);
                _delegate = @delegate;
            }
            else if(delegateM != null)
            {
                _ObjectDelM @delegate = __createDelegateM();
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

	    if(ice_isOneway() || ice_isBatchOneway())
	    {
		//
		// We do not retry oneway or batch oneway requests
		// (except for problems during connection
		// establishment, which are not handled here anyway).
		// If we retry a oneway or batch oneway, previous
		// oneways from the same batch, or previous oneways
		// that are buffered by the IP stack implementation,
		// are silently thrown away. This can lead to a
		// situation where the latest oneway succeeds due to
		// retry, but former oneways are discarded.
		//
		throw ex;
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

        public void __rethrowException(LocalException ex)
        {
            lock(this)
            {
                _delegate = null;
                throw ex;
            }
        }

        public void __checkTwowayOnly(string name)
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

        public _ObjectDel __getDelegate()
        {
            lock(this)
            {
                if(_delegate == null)
                {
                    if(_reference.getCollocationOptimization())
                    {
                        ObjectAdapter adapter = _reference.getInstance().objectAdapterFactory().findObjectAdapter(this);
                        if(adapter != null)
                        {
                            _ObjectDelD @delegate = __createDelegateD();
                            @delegate.setup(_reference, adapter);
                            _delegate = @delegate;
                        }
                    }

                    if(_delegate == null)
                    {
                        _ObjectDelM @delegate = __createDelegateM();
                        @delegate.setup(_reference);
                        _delegate = @delegate;

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
                        catch(InvalidCastException)
                        {
                        }
                    }
                }

                return _delegate;
            }
        }

        protected virtual _ObjectDelM __createDelegateM()
        {
            return new _ObjectDelM();
        }

        protected virtual _ObjectDelD __createDelegateD()
        {
            return new _ObjectDelD();
        }

        protected Context __defaultContext()
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
        private _ObjectDel _delegate;
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
                    Ice.ObjectPrx bb = b.ice_newFacet(f);
                    bool ok = bb.ice_isA("::Object");
                    Debug.Assert(ok);
                    ObjectPrxHelper h = new ObjectPrxHelper();
                    h.__copyFrom(bb);
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
                    Ice.ObjectPrx bb = b.ice_newFacet(f);
                    bool ok = bb.ice_isA("::Object", ctx);
                    Debug.Assert(ok);
                    ObjectPrxHelper h = new ObjectPrxHelper();
                    h.__copyFrom(bb);
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
                Ice.ObjectPrx bb = b.ice_newFacet(f);
                ObjectPrxHelper h = new ObjectPrxHelper();
                h.__copyFrom(bb);
                d = h;
            }
            return d;
        }
    }

    public interface _ObjectDel
    {
	bool ice_isA(string id, Ice.Context context);
	void ice_ping(Ice.Context context);
	string[] ice_ids(Ice.Context context);
	string ice_id(Ice.Context context);
	bool ice_invoke(string operation, Ice.OperationMode mode, byte[] inParams, out byte[] outParams,
			Ice.Context context);
        Connection ice_connection();
    }

    public class _ObjectDelD : _ObjectDel
    {
        public virtual bool ice_isA(string __id, Ice.Context __context)
        {
            Current __current = new Current();
            __initCurrent(ref __current, "ice_isA", OperationMode.Nonmutating, __context);
            while(true)
            {
                IceInternal.Direct __direct = new IceInternal.Direct(__current);
                try
                {
                    return __direct.servant().ice_isA(__id, __current);
                }
                finally
                {
                    __direct.destroy();
                }
            }
        }
	
        public virtual void ice_ping(Ice.Context __context)
        {
            Current __current = new Current();
            __initCurrent(ref __current, "ice_ping", OperationMode.Nonmutating, __context);
            while(true)
            {
                IceInternal.Direct __direct = new IceInternal.Direct(__current);
                try
                {
                    __direct.servant().ice_ping(__current);
                    return;
                }
                finally
                {
                    __direct.destroy();
                }
            }
        }
	
        public virtual string[] ice_ids(Ice.Context __context)
        {
            Current __current = new Current();
            __initCurrent(ref __current, "ice_ids", OperationMode.Nonmutating, __context);
            while(true)
            {
                IceInternal.Direct __direct = new IceInternal.Direct(__current);
                try
                {
                    return __direct.servant().ice_ids(__current);
                }
                finally
                {
                    __direct.destroy();
                }
            }
        }
	
        public virtual string ice_id(Ice.Context __context)
        {
            Current __current = new Current();
            __initCurrent(ref __current, "ice_id", OperationMode.Nonmutating, __context);
            while(true)
            {
                IceInternal.Direct __direct = new IceInternal.Direct(__current);
                try
                {
                    return __direct.servant().ice_id(__current);
                }
                finally
                {
                    __direct.destroy();
                }
            }
        }
	
        public virtual bool ice_invoke(string operation, Ice.OperationMode mode, byte[] inParams,
            out byte[] outParams, Ice.Context context)
        {
            throw new CollocationOptimizationException();
        }
	
        public virtual Connection ice_connection()
        {
            throw new CollocationOptimizationException();
        }

        //
        // Only for use by ObjectPrx.
        //
        internal void __copyFrom(_ObjectDelD from)
        {
            //
            // No need to synchronize "from", as the delegate is immutable
            // after creation.
            //
	    
            //
            // No need to synchronize, as this operation is only called
            // upon initialization.
            //
	    
            Debug.Assert(__reference == null);
            Debug.Assert(__adapter == null);
	    
            __reference = from.__reference;
            __adapter = from.__adapter;
        }
	
        protected internal IceInternal.Reference __reference;
        protected internal Ice.ObjectAdapter __adapter;
	
        protected internal void __initCurrent(ref Current current, string op, Ice.OperationMode mode, Ice.Context context)
        {
            current.adapter = __adapter;
            current.id = __reference.getIdentity();
            current.facet = __reference.getFacet();
            current.operation = op;
            current.mode = mode;
            current.ctx = context;
        }
	
        public virtual void setup(IceInternal.Reference rf, Ice.ObjectAdapter adapter)
        {
            //
            // No need to synchronize, as this operation is only called
            // upon initialization.
            //
	    
            Debug.Assert(__reference == null);
            Debug.Assert(__adapter == null);
	    
            __reference = rf;
            __adapter = adapter;
        }
    }
	    
    public class _ObjectDelM : _ObjectDel
    {
        public _ObjectDelM()
        {
            __outgoingMutex = new System.Object();
        }

        public virtual bool ice_isA(string __id, Ice.Context __context)
        {
            IceInternal.Outgoing __out = getOutgoing("ice_isA", OperationMode.Nonmutating, __context, __compress);
            try
            {
                IceInternal.BasicStream __is = __out.istr();
                IceInternal.BasicStream __os = __out.ostr();
                __os.writeString(__id);
                if(!__out.invoke())
                {
                    throw new UnknownUserException();
                }
                try
                {
                    return __is.readBool();
                }
                catch(LocalException __ex)
                {
                    throw new IceInternal.NonRepeatable(__ex);
                }
            }
            finally
            {
                reclaimOutgoing(__out);
            }
        }
	
        public virtual void ice_ping(Ice.Context __context)
        {
            IceInternal.Outgoing __out = getOutgoing("ice_ping", OperationMode.Nonmutating, __context, __compress);
            try
            {
                if(!__out.invoke())
                {
                    throw new UnknownUserException();
                }
            }
            finally
            {
                reclaimOutgoing(__out);
            }
        }
	
        public virtual string[] ice_ids(Ice.Context __context)
        {
            IceInternal.Outgoing __out = getOutgoing("ice_ids", OperationMode.Nonmutating, __context, __compress);
            try
            {
                IceInternal.BasicStream __is = __out.istr();
                if(!__out.invoke())
                {
                    throw new UnknownUserException();
                }
                try
                {
                    return __is.readStringSeq();
                }
                catch(LocalException __ex)
                {
                    throw new IceInternal.NonRepeatable(__ex);
                }
            }
            finally
            {
                reclaimOutgoing(__out);
            }
        }
	
        public virtual string ice_id(Ice.Context __context)
        {
            IceInternal.Outgoing __out = getOutgoing("ice_id", OperationMode.Nonmutating, __context, __compress);
            try
            {
                IceInternal.BasicStream __is = __out.istr();
                if(!__out.invoke())
                {
                    throw new UnknownUserException();
                }
                try
                {
                    return __is.readString();
                }
                catch(LocalException __ex)
                {
                    throw new IceInternal.NonRepeatable(__ex);
                }
            }
            finally
            {
                reclaimOutgoing(__out);
            }
        }
	
        public virtual bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams,
                                       Ice.Context __context)
        {
            IceInternal.Outgoing __out = getOutgoing(operation, mode, __context, __compress);
            try
            {
                IceInternal.BasicStream __os = __out.ostr();
                __os.writeBlob(inParams);
                bool ok = __out.invoke();
                outParams = null;
                if(__reference.getMode() == IceInternal.Reference.Mode.ModeTwoway)
                {
                    try
                    {
                        IceInternal.BasicStream __is = __out.istr();
                        int sz = __is.getReadEncapsSize();
                        outParams = __is.readBlob(sz);
                    }
                    catch(LocalException __ex)
                    {
                        throw new IceInternal.NonRepeatable(__ex);
                    }
                }
                return ok;
            }
            finally
            {
                reclaimOutgoing(__out);
            }
        }
	
        public virtual Connection ice_connection()
        {
            return __connection;
        }
	
        //
        // Only for use by ObjectPrx
        //
        internal void __copyFrom(_ObjectDelM from)
        {
            //
            // No need to synchronize "from", as the delegate is immutable
            // after creation.
            //
	    
            //
            // No need to synchronize, as this operation is only called
            // upon initialization.
            //
	    
            Debug.Assert(__reference == null);
            Debug.Assert(__connection == null);
	    
            __reference = from.__reference;
            __connection = from.__connection;
	    __compress = from.__compress;
        }
	
        protected internal IceInternal.Reference __reference;
        protected internal ConnectionI __connection;
	
        public virtual void setup(IceInternal.Reference rf)
        {
            //
            // No need to synchronize, as this operation is only called
            // upon initialization.
            //
	    
            Debug.Assert(__reference == null);
            Debug.Assert(__connection == null);
	    
            __reference = rf;
            __connection = __reference.getConnection(out __compress);
        }
	
        protected internal virtual IceInternal.Outgoing getOutgoing(string operation, OperationMode mode,
            Ice.Context context, bool compress)
        {
            IceInternal.Outgoing outg;
	    
            lock(__outgoingMutex)
            {
                if(__outgoingCache == null)
                {
                    outg = new IceInternal.Outgoing(__connection, __reference, operation, mode, context, compress);
                }
                else
                {
                    outg = __outgoingCache;
                    __outgoingCache = __outgoingCache.next;
                    outg.reset(operation, mode, context);
                }
            }
	    
            return outg;
        }
	
        protected internal virtual void reclaimOutgoing(IceInternal.Outgoing outg)
        {
            lock(__outgoingMutex)
            {
                outg.next = __outgoingCache;
                __outgoingCache = outg;
            }
        }
	
        ~_ObjectDelM()
        {
            while(__outgoingCache != null)
            {
                IceInternal.Outgoing next = __outgoingCache.next;
                __outgoingCache.destroy();
                __outgoingCache.next = null;
                __outgoingCache = next;
            }
        }
	
        private IceInternal.Outgoing __outgoingCache;
        private System.Object __outgoingMutex;
        protected bool __compress;
    }

}
