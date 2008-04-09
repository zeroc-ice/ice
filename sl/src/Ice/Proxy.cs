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
using IceUtil;

namespace Ice
{
    public interface ObjectPrx
    {
        [Obsolete("This method is deprecated, use ice_getHash instead.")]
        int ice_hash();
        int ice_getHash();

        [Obsolete("This method is deprecated, use ice_getCommunicator instead.")]
        Communicator ice_communicator();
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

        Identity ice_getIdentity();
        [Obsolete("This method is deprecated, use ice_identity instead.")]
        ObjectPrx ice_newIdentity(Identity newIdentity);
        ObjectPrx ice_identity(Identity newIdentity);
        
        Dictionary<string, string> ice_getContext();
        [Obsolete("This method is deprecated, use ice_context instead.")]
        ObjectPrx ice_newContext(Dictionary<string, string> newContext);
        ObjectPrx ice_context(Dictionary<string, string> newContext);
        [Obsolete("This method is deprecated.")]
        ObjectPrx ice_defaultContext();
        
        string ice_getFacet();
        [Obsolete("This method is deprecated, use ice_facet instead.")]
        ObjectPrx ice_newFacet(string newFacet);
        ObjectPrx ice_facet(string newFacet);

        string ice_getAdapterId();
        [Obsolete("This method is deprecated, use ice_adapterId instead.")]
        ObjectPrx ice_newAdapterId(string newAdapterId);
        ObjectPrx ice_adapterId(string newAdapterId);

        Endpoint[] ice_getEndpoints();
        [Obsolete("This method is deprecated, use ice_endpoints instead.")]
        ObjectPrx ice_newEndpoints(Endpoint[] newEndpoints);
        ObjectPrx ice_endpoints(Endpoint[] newEndpoints);

        bool ice_isSecure();
        ObjectPrx ice_secure(bool b);

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
    }

    public class ObjectPrxHelperBase : ObjectPrx
    {
        public override int GetHashCode()
        {
            return _reference.GetHashCode();
        }

        public int ice_hash()
        {
            return ice_getHash();
        }

        public int ice_getHash()
        {
            return _reference.GetHashCode();
        }

        public Communicator ice_communicator()
        {
            return ice_getCommunicator();
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

        private bool ice_isA(string id__, Dictionary<string, string> context__,
                             bool explicitContext__)
        {
            if(explicitContext__ && context__ == null)
            {
                context__ = emptyContext_;
            }

            int cnt__ = 0;
            while(true)
            {
                try
                {
                    IceInternal.Outgoing og__ = getOutgoing("ice_isA", OperationMode.Nonmutating, context__);
                    IceInternal.BasicStream os__ = og__.ostr();
                    os__.writeString(id__);
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
                try
                {
                    IceInternal.Outgoing og__ = getOutgoing("ice_ping", OperationMode.Nonmutating, context__);
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
                        return;
                    }
                    catch(LocalException ex__)
                    {
                        throw new IceInternal.LocalExceptionWrapper(ex__, false);
                    }
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
                try
                {
                    IceInternal.Outgoing og__ = getOutgoing("ice_ids", OperationMode.Nonmutating, context__);
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
                try
                {
                    IceInternal.Outgoing og__ = getOutgoing("ice_id", OperationMode.Nonmutating, context__);
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

        public ObjectPrx ice_newIdentity(Identity newIdentity)
        {
            return ice_identity(newIdentity);
        }

        public Dictionary<string, string> ice_getContext()
        {
            return _reference.getContext();
        }

        public ObjectPrx ice_context(Dictionary<string, string> newContext)
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeContext(newContext));
            return proxy;
        }

        public ObjectPrx ice_newContext(Dictionary<string, string> newContext)
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
                IceInternal.EndpointI[] endpts = new IceInternal.EndpointI[newEndpoints.Length];
                for(int i = 0; i < newEndpoints.Length; ++i)
                {
                    endpts[i] = (IceInternal.EndpointI)newEndpoints[i];
                }
                proxy.setup(_reference.changeEndpoints(endpts));
                return proxy;
            }
        }

        public ObjectPrx ice_newEndpoints(Endpoint[] newEndpoints)
        {
            return ice_endpoints(newEndpoints);
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
                ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
                proxy.setup(_reference.changeSecure(b));
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

            lock(from)
            {
                @ref = h._reference;
            }

            //
            // No need to synchronize "*this", as this operation is only
            // called upon initialization.
            //

            Debug.Assert(_reference == null);

            _reference = @ref;
        }

        public int handleException__(LocalException ex, int cnt)
        {
            IceInternal.ProxyFactory proxyFactory;
            try
            {
                proxyFactory = _reference.getInstance().proxyFactory();
            }
            catch(CommunicatorDestroyedException)
            {
                //
                // The communicator is already destroyed, so we cannot
                // retry.
                //
                throw ex;
            }

            return proxyFactory.checkRetryAfterException(ex, _reference, cnt);
        }

        public void handleExceptionWrapper__(IceInternal.LocalExceptionWrapper ex)
        {
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

            _reference = @ref;
        }

        protected IceInternal.Outgoing getOutgoing(string operation, OperationMode mode,
                                                   Dictionary<string, string> context)
        {
            return new IceInternal.Outgoing(_reference, operation, mode, context);
        }

        protected static Dictionary<string, string> emptyContext_ = new Dictionary<string, string>();
        private IceInternal.Reference _reference;
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
}
