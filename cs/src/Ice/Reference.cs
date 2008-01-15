// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace IceInternal
{
    public abstract class Reference : ICloneable
    {
        public enum Mode {
                            ModeTwoway,
                            ModeOneway,
                            ModeBatchOneway,
                            ModeDatagram,
                            ModeBatchDatagram,
                            ModeLast=ModeBatchDatagram
                         };

        public interface GetConnectionCallback
        {
            void setConnection(Ice.ConnectionI connection, bool compress);
            void setException(Ice.LocalException ex);
        }

        public Mode getMode()
        {
            return mode_;
        }

        public Ice.Identity getIdentity()
        {
            return identity_;
        }

        public string getFacet()
        {
            return facet_;
        }

        public Instance getInstance()
        {
            return instance_;
        }

        public Dictionary<string, string> getContext()
        {
            return context_;
        }

        public Reference defaultContext()
        {
            Reference r = instance_.referenceFactory().copy(this);
            r.context_ = instance_.getDefaultContext();
            return r;
        }

        public Ice.Communicator getCommunicator()
        {
            return communicator_;
        }

        public virtual RouterInfo getRouterInfo()
        {
            return null;
        }

        public virtual LocatorInfo getLocatorInfo()
        {
            return null;
        }

        public abstract bool getSecure();
        public abstract bool getPreferSecure();
        public abstract string getAdapterId();
        public abstract EndpointI[] getEndpoints();
        public abstract bool getCollocationOptimization();
        public abstract int getLocatorCacheTimeout();
        public abstract bool getCacheConnection();
        public abstract Ice.EndpointSelectionType getEndpointSelection();
        public abstract bool getThreadPerConnection();

        //
        // The change* methods (here and in derived classes) create
        // a new reference based on the existing one, with the
        // corresponding value changed.
        //
        public Reference changeContext(Dictionary<string, string> newContext)
        {
            if(newContext == null)
            {
                newContext = _emptyContext;
            }
            Reference r = instance_.referenceFactory().copy(this);
            if(newContext.Count == 0)
            {
                r.context_ = _emptyContext;
            }
            else
            {
                r.context_ = new Dictionary<string, string>(newContext);
            }
            return r;
        }

        public Reference changeMode(Mode newMode)
        {
            if(newMode == mode_)
            {
                return this;
            }
            Reference r = instance_.referenceFactory().copy(this);
            r.mode_ = newMode;
            return r;
        }

        public Reference changeIdentity(Ice.Identity newIdentity)
        {
            if(newIdentity.Equals(identity_))
            {
                return this;
            }
            Reference r = instance_.referenceFactory().copy(this);
            r.identity_ = newIdentity; // Identity is a value type, therefore a copy of newIdentity is made.
            return r;
        }

        public Reference changeFacet(string newFacet)
        {
            if(newFacet.Equals(facet_))
            {
                return this;
            }
            Reference r = instance_.referenceFactory().copy(this);
            r.facet_ = newFacet;
            return r;
        }

        public virtual Reference changeCompress(bool newCompress)
        {
            if(overrideCompress_ && compress_ == newCompress)
            {
                return this;
            }

            Reference r = instance_.referenceFactory().copy(this);
            r.compress_ = newCompress;
            r.overrideCompress_ = true;
            return r;
        }

        public abstract Reference changeSecure(bool newSecure);
        public abstract Reference changePreferSecure(bool newPreferSecure);
        public abstract Reference changeRouter(Ice.RouterPrx newRouter);
        public abstract Reference changeLocator(Ice.LocatorPrx newLocator);
        public abstract Reference changeTimeout(int newTimeout);
        public abstract Reference changeConnectionId(string connectionId);
        public abstract Reference changeCollocationOptimization(bool newCollocationOptimization);
        public abstract Reference changeAdapterId(string newAdapterId);
        public abstract Reference changeEndpoints(EndpointI[] newEndpoints);
        public abstract Reference changeLocatorCacheTimeout(int newTimeout);
        public abstract Reference changeCacheConnection(bool newCache);
        public abstract Reference changeEndpointSelection(Ice.EndpointSelectionType newType);
        public abstract Reference changeThreadPerConnection(bool newTpc);

        public override int GetHashCode()
        {
            lock(this)
            {
                if(hashInitialized_)
                {
                    return hashValue_;
                }

                int h = (int)mode_;

                int sz = identity_.name.Length;
                for(int i = 0; i < sz; i++)
                {   
                    h = 5 * h + (int)identity_.name[i];
                }

                sz = identity_.category.Length;
                for(int i = 0; i < sz; i++)
                {   
                    h = 5 * h + (int)identity_.category[i];
                }

                h = 5 * h + context_.GetHashCode();

                sz = facet_.Length;
                for(int i = 0; i < sz; i++)
                {   
                    h = 5 * h + (int)facet_[i];
                }

                h = 5 * h + (getSecure() ? 1 : 0);

                hashValue_ = h;
                hashInitialized_ = true;

                return h;
            }
        }

        //
        // Marshal the reference.
        //
        public virtual void streamWrite(BasicStream s)
        {
            //
            // Don't write the identity here. Operations calling streamWrite
            // write the identity.
            //

            //
            // For compatibility with the old FacetPath.
            //
            if(facet_.Length == 0)
            {
                s.writeStringSeq(null);
            }
            else
            {
                string[] facetPath = { facet_ };
                s.writeStringSeq(facetPath);
            }

            s.writeByte((byte)mode_);

            s.writeBool(getSecure());

            // Derived class writes the remainder of the reference.
        }

        //
        // Convert the reference to its string form.
        //
        public override string ToString()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
            StringBuilder s = new StringBuilder();

            //
            // If the encoded identity string contains characters which
            // the reference parser uses as separators, then we enclose
            // the identity string in quotes.
            //
            string id = instance_.identityToString(identity_);
            if(IceUtilInternal.StringUtil.findFirstOf(id, " \t\n\r:@") != -1)
            {
                s.Append('"');
                s.Append(id);
                s.Append('"');
            }
            else
            {
                s.Append(id);
            }

            if(facet_.Length > 0)
            {
                //
                // If the encoded facet string contains characters which
                // the reference parser uses as separators, then we enclose
                // the facet string in quotes.
                //
                s.Append(" -f ");
                string fs = IceUtilInternal.StringUtil.escapeString(facet_, "");
                if(IceUtilInternal.StringUtil.findFirstOf(fs, " \t\n\r:@") != -1)
                {
                    s.Append('"');
                    s.Append(fs);
                    s.Append('"');
                }
                else
                {
                    s.Append(fs);
                }
            }

            switch(mode_)
            {
            case Mode.ModeTwoway:
                {
                    s.Append(" -t");
                    break;
                }

            case Mode.ModeOneway:
                {
                    s.Append(" -o");
                    break;
                }

            case Mode.ModeBatchOneway:
                {
                    s.Append(" -O");
                    break;
                }

            case Mode.ModeDatagram:
                {
                    s.Append(" -d");
                    break;
                }

            case Mode.ModeBatchDatagram:
                {
                    s.Append(" -D");
                    break;
                }
            }

            if(getSecure())
            {
                s.Append(" -s");
            }

            return s.ToString();

            // Derived class writes the remainder of the string.
        }

        public abstract Ice.ConnectionI getConnection(out bool comp);
        public abstract void getConnection(GetConnectionCallback callback);

        public override bool Equals(object obj)
        {
            //
            // Note: if(this == obj) and type test are performed by each non-abstract derived class.
            //

            Reference r = (Reference)obj; // Guaranteed to succeed.

            if(mode_ != r.mode_)
            {
                return false;
            }

            if(!identity_.Equals(r.identity_))
            {
                return false;
            }

            if(!Ice.CollectionComparer.Equals(context_, r.context_))
            {
                return false;
            }

            if(!facet_.Equals(r.facet_))
            {
                return false;
            }

            if(overrideCompress_ != r.overrideCompress_)
            {
                return false;
            }
            if(overrideCompress_ && compress_ != r.compress_)
            {
                return false;
            }

            return true;
        }

        public Object Clone()
        {
            //
            // A member-wise copy is safe because the members are immutable.
            //
            return MemberwiseClone();
        }

        private Instance instance_;
        private Ice.Communicator communicator_;
        private Mode mode_;
        private Ice.Identity identity_;
        private Dictionary<string, string> context_;
        private static Dictionary<string, string> _emptyContext = new Dictionary<string, string>();
        private string facet_;

        protected int hashValue_;
        protected bool hashInitialized_;
        protected bool overrideCompress_;
        protected bool compress_; // Only used if _overrideCompress == true

        protected Reference(Instance inst,
                            Ice.Communicator com,
                            Ice.Identity ident,
                            Dictionary<string, string> ctx,
                            string fac,
                            Mode md)
        {
            //
            // Validate string arguments.
            //
            Debug.Assert(ident.name != null);
            Debug.Assert(ident.category != null);
            Debug.Assert(fac != null);

            instance_ = inst;
            communicator_ = com;
            mode_ = md;
            identity_ = ident;
            context_ = ctx == null ? _emptyContext : ctx;
            facet_ = fac;
            hashInitialized_ = false;
            overrideCompress_ = false;
            compress_ = false;
        }

        protected static System.Random rand_ = new System.Random(unchecked((int)System.DateTime.Now.Ticks));
    }

    public class FixedReference : Reference
    {
        public FixedReference(Instance inst,
                              Ice.Communicator com,
                              Ice.Identity ident,
                              Dictionary<string, string> ctx,
                              string fs,
                              Reference.Mode md,
                              Ice.ConnectionI[] fixedConns)
            : base(inst, com, ident, ctx, fs, md)
        {
            _fixedConnections = fixedConns;
        }

        public override bool getSecure()
        {
            return false;
        }

        public override bool getPreferSecure()
        {
            return false;
        }

        public override string getAdapterId()
        {
            return "";
        }

        public override EndpointI[] getEndpoints()
        {
            return new EndpointI[0];
        }

        public override bool getCollocationOptimization()
        {
            return false;
        }

        public override int getLocatorCacheTimeout()
        {
            return 0;
        }

        public override bool getCacheConnection()
        {
            return false;
        }

        public override Ice.EndpointSelectionType getEndpointSelection()
        {
            return Ice.EndpointSelectionType.Random;
        }

        public override bool getThreadPerConnection()
        {
            return false;
        }

        public override Reference changeSecure(bool sec)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changePreferSecure(bool prefSec)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeRouter(Ice.RouterPrx newRouter)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeLocator(Ice.LocatorPrx newLocator)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeTimeout(int newTimeout)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeConnectionId(string connectionId)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeCollocationOptimization(bool newCollocationOptimization)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeAdapterId(string newAdapterId)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeEndpoints(EndpointI[] newEndpoints)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeLocatorCacheTimeout(int newTimeout)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeCacheConnection(bool newCache)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeEndpointSelection(Ice.EndpointSelectionType newType)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeThreadPerConnection(bool newTpc)
        {
            throw new Ice.FixedProxyException();
        }

        public override void streamWrite(BasicStream s)
        {
            throw new Ice.FixedProxyException();
        }

        public override string ToString()
        {
            throw new Ice.FixedProxyException();
        }

        public override Ice.ConnectionI getConnection(out bool compress)
        {
            Ice.ConnectionI[] filteredConns = filterConnections(_fixedConnections);
            if(filteredConns.Length == 0)
            {
                Ice.NoEndpointException ex = new Ice.NoEndpointException();
                ex.proxy = ""; // No stringified representation for fixed proxies.
                throw ex;
            }

            Ice.ConnectionI connection = filteredConns[0];
            Debug.Assert(connection != null);
            connection.throwException(); // Throw in case our connection is already destroyed.
            
            DefaultsAndOverrides defaultsAndOverrides = getInstance().defaultsAndOverrides();
            if(defaultsAndOverrides.overrideCompress)
            {
                compress = defaultsAndOverrides.overrideCompressValue;
            }
            else if(overrideCompress_)
            {
                compress = compress_;
            }
            else
            {
                compress = connection.endpoint().compress();
            }
            return connection;
        }

        public override void getConnection(GetConnectionCallback callback)
        {
            try
            {
                bool compress;
                Ice.ConnectionI connection = getConnection(out compress);
                callback.setConnection(connection, compress);
            }
            catch(Ice.LocalException ex)
            {
                callback.setException(ex);
            }
        }

        public override bool Equals(object obj)
        {
            if(object.ReferenceEquals(this, obj))
            {
                return true;
            }
            if(!(obj is FixedReference))
            {
                return false;
            }
            FixedReference rhs = (FixedReference)obj;
            if(!base.Equals(rhs))
            {
                return false;
            }
            return IceUtilInternal.Arrays.Equals(_fixedConnections, rhs._fixedConnections);
        }

        //
        // If we override Equals, we must also override GetHashCode.
        //
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        //
        // Filter connections based on criteria from this reference.
        //
        public Ice.ConnectionI[]
        filterConnections(Ice.ConnectionI[] allConnections)
        {
            ArrayList connections = new ArrayList();

            switch(getMode())
            {
                case Reference.Mode.ModeTwoway:
                case Reference.Mode.ModeOneway:
                case Reference.Mode.ModeBatchOneway:
                {
                    //
                    // Filter out datagram endpoints.
                    //
                    for(int i = 0; i < allConnections.Length; ++i)
                    {
                        if(!allConnections[i].endpoint().datagram())
                        {
                            connections.Add(allConnections[i]);
                        }
                    }

                    break;
                }

                case Reference.Mode.ModeDatagram:
                case Reference.Mode.ModeBatchDatagram:
                {
                    //
                    // Filter out non-datagram endpoints.
                    //
                    for(int i = 0; i < allConnections.Length; ++i)
                    {
                        if(allConnections[i].endpoint().datagram())
                        {
                            connections.Add(allConnections[i]);
                        }
                    }

                    break;
                }
            }

            //
            // Randomize the order of connections.
            //
            for(int i = 0; i < connections.Count - 2; ++i)
            {
                int r = rand_.Next(connections.Count - i) + i;
                Debug.Assert(r >= i && r < connections.Count);
                if(r != i)
                {
                    object tmp = connections[i];
                    connections[i] = connections[r];
                    connections[r] = tmp;
                }
            }

            //
            // If a secure connection is requested or secure overrides is set, 
            // remove all non-secure endpoints. Otherwise if preferSecure is set
            // make secure endpoints prefered. By default make non-secure
            // endpoints preferred over secure endpoints.
            //
            DefaultsAndOverrides overrides = getInstance().defaultsAndOverrides();
            if(overrides.overrideSecure ? overrides.overrideSecureValue : getSecure())
            {
                ArrayList tmp = new ArrayList();
                foreach(Ice.ConnectionI connection in connections)
                {
                    if(connection.endpoint().secure())
                    {
                        tmp.Add(connection);
                    }
                }
                connections = tmp;
            }
            else if(getPreferSecure())
            {
                IceUtilInternal.Arrays.Sort(ref connections, _preferSecureConnectionComparator);
            }
            else
            {
                IceUtilInternal.Arrays.Sort(ref connections, _preferNonSecureConnectionComparator);
            }
            
            Ice.ConnectionI[] arr = new Ice.ConnectionI[connections.Count];
            if(arr.Length != 0)
            {
                connections.CopyTo(arr);
            }
            return arr;
        }

        private class ConnectionComparator : IComparer
        {
            public ConnectionComparator(bool preferSecure)
            {
                _preferSecure = preferSecure;
            }

            public int Compare(object l, object r)
            {
                Ice.ConnectionI lc = (Ice.ConnectionI)l;
                Ice.ConnectionI rc = (Ice.ConnectionI)r;
                bool ls = lc.endpoint().secure();
                bool rs = rc.endpoint().secure();
                if((ls && rs) || (!ls && !rs))
                {
                    return 0;
                }
                else if(!ls && rs)
                {
                    if(_preferSecure)
                    {
                        return 1;
                    }
                    else
                    {
                        return -1;
                    }
                }
                else
                {
                    if(_preferSecure)
                    {
                        return -1;
                    }
                    else
                    {
                        return 1;
                    }
                }
            }

            private bool _preferSecure;
        }
        
        private static ConnectionComparator _preferNonSecureConnectionComparator = new ConnectionComparator(false);
        private static ConnectionComparator _preferSecureConnectionComparator = new ConnectionComparator(true);
        private Ice.ConnectionI[] _fixedConnections;
    }

    public abstract class RoutableReference : Reference
    {
        public override RouterInfo getRouterInfo()
        {
            return _routerInfo;
        }

        public override bool getSecure()
        {
            return _secure;
        }

        public override bool getPreferSecure()
        {
            return _preferSecure;
        }

        public override bool getCollocationOptimization()
        {
            return _collocationOptimization;
        }

        public override bool getCacheConnection()
        {
            return _cacheConnection;
        }

        public override Ice.EndpointSelectionType getEndpointSelection()
        {
            return _endpointSelection;
        }

        public override bool getThreadPerConnection()
        {
            return _threadPerConnection;
        }

        public override Reference changeSecure(bool newSecure)
        {
            if(newSecure == _secure)
            {
                return this;
            }
            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r._secure = newSecure;
            return r;
        }

        public override Reference changePreferSecure(bool newPreferSecure)
        {
            if(newPreferSecure == _preferSecure)
            {
                return this;
            }
            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r._preferSecure = newPreferSecure;
            return r;
        }

        public override Reference changeRouter(Ice.RouterPrx newRouter)
        {
            RouterInfo newRouterInfo = getInstance().routerManager().get(newRouter);
            if(newRouterInfo != null && _routerInfo != null && newRouterInfo.Equals(_routerInfo))
            {
                return this;
            }
            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r._routerInfo = newRouterInfo;
            return r;
        }

        public override Reference changeCollocationOptimization(bool newCollocationOptimization)
        {
            if(newCollocationOptimization == _collocationOptimization)
            {
                return this;
            }
            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r._collocationOptimization = newCollocationOptimization;
            return r;
        }

        public override Reference changeTimeout(int newTimeout)
        {
            if(overrideTimeout_ && timeout_ == newTimeout)
            {
                return this;
            }

            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r.timeout_ = newTimeout;
            r.overrideTimeout_ = true;
            return r;
        }

        public override Reference changeConnectionId(string id)
        {
            if(connectionId_.Equals(id))
            {
                return this;
            }
            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r.connectionId_ = id;
            return r;
        }

        public override Reference changeCacheConnection(bool newCache)
        {
            if(newCache == _cacheConnection)
            {
                return this;
            }
            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r._cacheConnection = newCache;
            return r;
        }

        public override Reference changeEndpointSelection(Ice.EndpointSelectionType newType)
        {
            if(newType == _endpointSelection)
            {
                return this;
            }
            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r._endpointSelection = newType;
            return r;
        }       

        public override Reference changeThreadPerConnection(bool newTpc)
        {
            if(newTpc == _threadPerConnection)
            {
                return this;
            }
            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r._threadPerConnection = newTpc;
            return r;
        }

        public override bool Equals(object obj)
        {
            //
            // Note: if(this == obj) and type test are performed by each non-abstract derived class.
            //

            if(!base.Equals(obj))
            {
                return false;
            }
            RoutableReference rhs = (RoutableReference)obj; // Guaranteed to succeed.
            if(_secure != rhs._secure)
            {
                return false;
            }
            if(_preferSecure != rhs._preferSecure)
            {
                return false;
            }
            if(_collocationOptimization != rhs._collocationOptimization)
            {
                return false;
            }
            if(_cacheConnection != rhs._cacheConnection)
            {
                return false;
            }
            if(_endpointSelection != rhs._endpointSelection)
            {
                return false;
            }
            if(overrideTimeout_ != rhs.overrideTimeout_)
            {
                return false;
            }
            if(overrideTimeout_ && timeout_ != rhs.timeout_)
            {
                return false;
            }
            if(!connectionId_.Equals(rhs.connectionId_))
            {
                return false;
            }
            if(_threadPerConnection != rhs._threadPerConnection)
            {
                return false;
            }
            return _routerInfo == null ? rhs._routerInfo == null : _routerInfo.Equals(rhs._routerInfo);
        }

        //
        // If we override Equals, we must also override GetHashCode.
        //
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        protected RoutableReference(Instance inst,
                                    Ice.Communicator com,
                                    Ice.Identity ident,
                                    Dictionary<string, string> ctx,
                                    string fac,
                                    Reference.Mode md,
                                    bool sec,
                                    bool prefSec,
                                    RouterInfo rtrInfo,
                                    bool collocationOpt,
                                    bool cacheConnection,
                                    Ice.EndpointSelectionType endpointSelection,
                                    bool threadPerConnection)
            : base(inst, com, ident, ctx, fac, md)
        {
            _secure = sec;
            _preferSecure = prefSec;
            _routerInfo = rtrInfo;
            _collocationOptimization = collocationOpt;
            _cacheConnection = cacheConnection;
            _endpointSelection = endpointSelection;
            overrideTimeout_ = false;
            timeout_ = -1;
            _threadPerConnection = threadPerConnection;
        }

        protected void applyOverrides(ref EndpointI[] endpts)
        {
            for(int i = 0; i < endpts.Length; ++i)
            {
                endpts[i] = endpts[i].connectionId(connectionId_);
                if(overrideCompress_)
                {
                    endpts[i] = endpts[i].compress(compress_);
                }
                if(overrideTimeout_)
                {
                    endpts[i] = endpts[i].timeout(timeout_);
                }
            }
        }

        private EndpointI[] filterEndpoints(EndpointI[] allEndpoints)
        {
            ArrayList endpoints = new ArrayList();

            //
            // Filter out unknown endpoints.
            //
            for(int i = 0; i < allEndpoints.Length; i++)
            {
                if(!allEndpoints[i].unknown())
                {
                    endpoints.Add(allEndpoints[i]);
                }
            }
            
            //
            // Filter out endpoints according to the mode of the reference.
            //
            switch(getMode())
            {
                case Reference.Mode.ModeTwoway:
                case Reference.Mode.ModeOneway:
                case Reference.Mode.ModeBatchOneway:
                {
                    //
                    // Filter out datagram endpoints.
                    //
                    ArrayList tmp = new ArrayList();
                    foreach(EndpointI endpoint in endpoints)
                    {
                        if(!endpoint.datagram())
                        {
                            tmp.Add(endpoint);
                        }
                    }
                    endpoints = tmp;
                    break;
                }

                case Reference.Mode.ModeDatagram:
                case Reference.Mode.ModeBatchDatagram:
                {
                    //
                    // Filter out non-datagram endpoints.
                    //
                    ArrayList tmp = new ArrayList();
                    foreach(EndpointI endpoint in endpoints)
                    {
                        if(endpoint.datagram())
                        {
                            tmp.Add(endpoint);
                        }
                    }
                    endpoints = tmp;
                    break;
                }
            }

            //
            // Sort the endpoints according to the endpoint selection type.
            //
            switch(getEndpointSelection())
            {
                case Ice.EndpointSelectionType.Random:
                {
                    for(int i = 0; i < endpoints.Count - 2; ++i)
                    {
                        int r = rand_.Next(endpoints.Count - i) + i;
                        Debug.Assert(r >= i && r < endpoints.Count);
                        if(r != i)
                        {
                            object tmp = endpoints[i];
                            endpoints[i] = endpoints[r];
                            endpoints[r] = tmp;
                        }
                    }
                    break;
                }
                case Ice.EndpointSelectionType.Ordered:
                {
                    // Nothing to do.
                    break;
                }
                default:
                {
                    Debug.Assert(false);
                    break;
                }
            }

            //
            // If a secure connection is requested or secure overrides
            // is set, remove all non-secure endpoints. Otherwise make
            // non-secure endpoints preferred over secure endpoints by
            // partitioning the endpoint vector, so that non-secure
            // endpoints come first.
            //
            DefaultsAndOverrides overrides = getInstance().defaultsAndOverrides();
            if(overrides.overrideSecure ? overrides.overrideSecureValue : getSecure())
            {
                ArrayList tmp = new ArrayList();
                foreach(EndpointI endpoint in endpoints)
                {
                    if(endpoint.secure())
                    {
                        tmp.Add(endpoint);
                    }
                }
                endpoints = tmp;
            }
            else if(getPreferSecure())
            {
                IceUtilInternal.Arrays.Sort(ref endpoints, _preferSecureEndpointComparator);
            }
            else
            {
                IceUtilInternal.Arrays.Sort(ref endpoints, _preferNonSecureEndpointComparator);
            }

            EndpointI[] arr = new EndpointI[endpoints.Count];
            endpoints.CopyTo(arr);
            return arr;
        }

        protected Ice.ConnectionI createConnection(EndpointI[] allEndpoints, out bool compress)
        {
            compress = false; // Satisfy the compiler

            EndpointI[] endpoints = filterEndpoints(allEndpoints);
            if(endpoints.Length == 0)
            {
                throw new Ice.NoEndpointException(ToString());
            }

            //
            // Finally, create the connection.
            //
            OutgoingConnectionFactory factory = getInstance().outgoingConnectionFactory();
            Ice.ConnectionI connection = null;
            if(getCacheConnection() || endpoints.Length == 1)
            {
                //
                // Get an existing connection or create one if there's no
                // existing connection to one of the given endpoints.
                //
                connection = factory.create(endpoints, false, _threadPerConnection, getEndpointSelection(),
                                            out compress);
            }
            else
            {
                //
                // Go through the list of endpoints and try to create the
                // connection until it succeeds. This is different from just
                // calling create() with the given endpoints since this might
                // create a new connection even if there's an existing
                // connection for one of the endpoints.
                //

                Ice.LocalException exception = null;
                EndpointI[] endpoint = new EndpointI[1];
                for(int i = 0; i < endpoints.Length; ++i)
                {
                    try
                    {
                        endpoint[0] = endpoints[i];
                        bool more = i != endpoints.Length - 1;
                        connection = factory.create(endpoint, more, _threadPerConnection, getEndpointSelection(),
                                                    out compress);
                        break;
                    }
                    catch(Ice.LocalException ex)
                    {
                        exception = ex;
                    }
                }

                if(connection == null)
                {
                    Debug.Assert(exception != null);
                    throw exception;
                }
            }

            Debug.Assert(connection != null);

            //
            // If we have a router, set the object adapter for this router
            // (if any) to the new connection, so that callbacks from the
            // router can be received over this new connection.
            //
            if(_routerInfo != null)
            {
                connection.setAdapter(_routerInfo.getAdapter());
            }

            return connection;
        }

        private sealed class ConnectionCallback : OutgoingConnectionFactory.CreateConnectionCallback
        {
            internal ConnectionCallback(RoutableReference rr, EndpointI[] endpoints, GetConnectionCallback callback)
            {
                _rr = rr;
                _endpoints = endpoints;
                _callback = callback;
            }

            public void setConnection(Ice.ConnectionI connection, bool compress)
            {
                //
                // If we have a router, set the object adapter for this router
                // (if any) to the new connection, so that callbacks from the
                // router can be received over this new connection.
                //
                if(_rr._routerInfo != null)
                {
                    connection.setAdapter(_rr._routerInfo.getAdapter());
                }
                _callback.setConnection(connection, compress);
            }

            public void setException(Ice.LocalException ex)
            {
                if(_exception == null)
                {
                    _exception = ex;
                }

                if(_endpoints == null || ++_i == _endpoints.Length)
                {
                    _callback.setException(_exception);
                    return;
                }

                bool more = _i != _endpoints.Length - 1;
                EndpointI[] endpoint = new EndpointI[]{ _endpoints[_i] };
                _rr.getInstance().outgoingConnectionFactory().create(endpoint, more, _rr._threadPerConnection,
                                                                     _rr.getEndpointSelection(), this);
            }

            private RoutableReference _rr;
            private EndpointI[] _endpoints;
            private GetConnectionCallback _callback;
            private int _i = 0;
            private Ice.LocalException _exception = null;
        }

        protected void createConnection(EndpointI[] allEndpoints, GetConnectionCallback callback)
        {
            EndpointI[] endpoints = filterEndpoints(allEndpoints);
            if(endpoints.Length == 0)
            {
                callback.setException(new Ice.NoEndpointException(ToString()));
                return;
            }

            //
            // Finally, create the connection.
            //
            OutgoingConnectionFactory factory = getInstance().outgoingConnectionFactory();
            if(getCacheConnection() || endpoints.Length == 1)
            {
                //
                // Get an existing connection or create one if there's no
                // existing connection to one of the given endpoints.
                //
                factory.create(endpoints, false, _threadPerConnection, getEndpointSelection(),
                               new ConnectionCallback(this, null, callback));
            }
            else
            {
                //
                // Go through the list of endpoints and try to create the
                // connection until it succeeds. This is different from just
                // calling create() with the given endpoints since this might
                // create a new connection even if there's an existing
                // connection for one of the endpoints.
                //

                factory.create(new EndpointI[]{ endpoints[0] }, true, _threadPerConnection, getEndpointSelection(),
                               new ConnectionCallback(this, endpoints, callback));
            }
        }

        private class EndpointComparator : IComparer
        {
            public EndpointComparator(bool preferSecure)
            {
                _preferSecure = preferSecure;
            }

            public int Compare(object l, object r)
            {
                IceInternal.EndpointI le = (IceInternal.EndpointI)l;
                IceInternal.EndpointI re = (IceInternal.EndpointI)r;
                bool ls = le.secure();
                bool rs = re.secure();
                if((ls && rs) || (!ls && !rs))
                {
                    return 0;
                }
                else if(!ls && rs)
                {
                    if(_preferSecure)
                    {
                        return 1;
                    }
                    else
                    {
                        return -1;
                    }
                }
                else
                {
                    if(_preferSecure)
                    {
                        return -1;
                    }
                    else
                    {
                        return 1;
                    }
                }
            }

            private bool _preferSecure;
        }
        
        private static EndpointComparator _preferNonSecureEndpointComparator = new EndpointComparator(false);
        private static EndpointComparator _preferSecureEndpointComparator = new EndpointComparator(true);

        private bool _secure;
        private bool _preferSecure;
        private RouterInfo _routerInfo; // Null if no router is used.
        private bool _collocationOptimization;
        private bool _cacheConnection;
        private Ice.EndpointSelectionType _endpointSelection;
        private string connectionId_ = "";
        private bool overrideTimeout_;
        private int timeout_;
        private bool _threadPerConnection;
    }

    public class DirectReference : RoutableReference
    {
        public DirectReference(Instance inst,
                               Ice.Communicator com,
                               Ice.Identity ident,
                               Dictionary<string, string> ctx,
                               string fs,
                               Reference.Mode md,
                               bool sec,
                               bool prefSec,
                               EndpointI[] endpts,
                               RouterInfo rtrInfo,
                               bool collocationOpt,
                               bool cacheConnection,
                               Ice.EndpointSelectionType endpointSelection,
                               bool threadPerConnection)
            : base(inst, com, ident, ctx, fs, md, sec, prefSec, rtrInfo, collocationOpt, cacheConnection,
                   endpointSelection, threadPerConnection)
        {
            _endpoints = endpts;
        }

        public override string getAdapterId()
        {
            return "";
        }

        public override EndpointI[] getEndpoints()
        {
            return _endpoints;
        }

        public override int getLocatorCacheTimeout()
        {
            return 0;
        }

        public override Reference changeLocator(Ice.LocatorPrx newLocator)
        {
            return this;
        }

        public override Reference changeCompress(bool newCompress)
        {
            DirectReference r = (DirectReference)base.changeCompress(newCompress);
            if(r != this) // Also override the compress flag on the endpoints if it was updated.
            {
                EndpointI[] newEndpoints = new EndpointI[_endpoints.Length];
                for(int i = 0; i < _endpoints.Length; i++)
                {
                    newEndpoints[i] = _endpoints[i].compress(newCompress);
                }
                r._endpoints = newEndpoints;
            }
            return r;
        }

        public override Reference changeTimeout(int newTimeout)
        {
            DirectReference r = (DirectReference)base.changeTimeout(newTimeout);
            if(r != this) // Also override the timeout on the endpoints if it was updated.
            {
                EndpointI[] newEndpoints = new EndpointI[_endpoints.Length];
                for(int i = 0; i < _endpoints.Length; i++)
                {
                    newEndpoints[i] = _endpoints[i].timeout(newTimeout);
                }
                r._endpoints = newEndpoints;
            }
            return r;
        }

        public override Reference changeConnectionId(string connectionId)
        {
            DirectReference r = (DirectReference)base.changeConnectionId(connectionId);
            if(r != this) // Also override the connection id on the endpoints if it was updated.
            {
                EndpointI[] newEndpoints = new EndpointI[_endpoints.Length];
                for(int i = 0; i < _endpoints.Length; i++)
                {
                    newEndpoints[i] = _endpoints[i].connectionId(connectionId);
                }
                r._endpoints = newEndpoints;
            }
            return r;
        }

        public override Reference changeAdapterId(string newAdapterId)
        {
            if(newAdapterId == null || newAdapterId.Length == 0)
            {
                return this;
            }
            LocatorInfo locatorInfo = 
                getInstance().locatorManager().get(getInstance().referenceFactory().getDefaultLocator());
            return getInstance().referenceFactory().create(
                getIdentity(), getContext(), getFacet(), getMode(), getSecure(), getPreferSecure(), newAdapterId,
                getRouterInfo(), locatorInfo, getCollocationOptimization(), getCacheConnection(),
                getEndpointSelection(), getThreadPerConnection(), getLocatorCacheTimeout());
        }

        public override Reference changeEndpoints(EndpointI[] newEndpoints)
        {
            if(Array.Equals(newEndpoints, _endpoints))
            {
                return this;
            }
            DirectReference r = (DirectReference)getInstance().referenceFactory().copy(this);
            r._endpoints = newEndpoints;
            r.applyOverrides(ref r._endpoints);
            return r;
        }

        public override Reference changeLocatorCacheTimeout(int newTimeout)
        {
            return this;
        }

        public override void streamWrite(BasicStream s)
        {
            base.streamWrite(s);

            s.writeSize(_endpoints.Length);
            if(_endpoints.Length > 0)
            {
                for(int i = 0; i < _endpoints.Length; i++)
                {
                    _endpoints[i].streamWrite(s);
                }
            }
            else
            {
                s.writeString(""); // Adapter id.
            }
        }

        public override string ToString()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
            StringBuilder s = new StringBuilder();
            s.Append(base.ToString());

            for(int i = 0; i < _endpoints.Length; i++)
            {
                string endp = _endpoints[i].ToString();
                if(endp != null && endp.Length > 0)
                {
                    s.Append(':');
                    s.Append(endp);
                }
            }
            return s.ToString();
        }

        public override Ice.ConnectionI getConnection(out bool comp)
        {
            if(getRouterInfo() != null)
            {
                //
                // If we route, we send everything to the router's client
                // proxy endpoints.
                //
                EndpointI[] endpts = getRouterInfo().getClientEndpoints();
                if(endpts.Length > 0)
                {
                    applyOverrides(ref endpts);
                    return createConnection(endpts, out comp);
                }
            }

            return createConnection(_endpoints, out comp);
        }

        private sealed class RouterEndpointsCallback : RouterInfo.GetClientEndpointsCallback
        {
            internal RouterEndpointsCallback(DirectReference dr, GetConnectionCallback cb)
            {
                _dr = dr;
                _cb = cb;
            }

            public void setEndpoints(EndpointI[] endpts)
            {
                if(endpts.Length > 0)
                {
                    _dr.applyOverrides(ref endpts);
                    _dr.createConnection(endpts, _cb);
                    return;
                }

                _dr.createConnection(_dr._endpoints, _cb);
            }

            public void setException(Ice.LocalException ex)
            {
                _cb.setException(ex);
            }

            private DirectReference _dr;
            private GetConnectionCallback _cb;
        }

        public override void getConnection(GetConnectionCallback callback)
        {
            if(getRouterInfo() != null)
            {
                //
                // If we route, we send everything to the router's client
                // proxy endpoints.
                //
                getRouterInfo().getClientEndpoints(new RouterEndpointsCallback(this, callback));
                return;
            }

            createConnection(_endpoints, callback);
        }

        public override bool Equals(object obj)
        {
            if(Object.ReferenceEquals(this, obj))
            {
                return true;
            }
            if(!(obj is DirectReference))
            {
                return false;
            }
            DirectReference rhs = (DirectReference)obj;
            if(!base.Equals(rhs))
            {
                return false;
            }
            return IceUtilInternal.Arrays.Equals(_endpoints, rhs._endpoints);
        }

        //
        // If we override Equals, we must also override GetHashCode.
        //
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        private EndpointI[] _endpoints;
    }

    public class IndirectReference : RoutableReference
    {
        public IndirectReference(Instance inst,
                                 Ice.Communicator com,
                                 Ice.Identity ident,
                                 Dictionary<string, string> ctx,
                                 string fs,
                                 Reference.Mode md,
                                 bool sec,
                                 bool prefSec,
                                 string adptid,
                                 RouterInfo rtrInfo,
                                 LocatorInfo locInfo,
                                 bool collocationOpt,
                                 bool cacheConnection,
                                 Ice.EndpointSelectionType endpointSelection,
                                 bool threadPerConnection,
                                 int locatorCacheTimeout)
            : base(inst, com, ident, ctx, fs, md, sec, prefSec, rtrInfo, collocationOpt, cacheConnection,
                   endpointSelection, threadPerConnection)
        {
            adapterId_ = adptid;
            locatorInfo_ = locInfo;
            locatorCacheTimeout_ = locatorCacheTimeout;
        }

        public override LocatorInfo getLocatorInfo()
        {
            return locatorInfo_;
        }

        public override string getAdapterId()
        {
            return adapterId_;
        }

        public override EndpointI[] getEndpoints()
        {
            return new EndpointI[0];
        }

        public override int getLocatorCacheTimeout()
        {
            return locatorCacheTimeout_;
        }

        public override Reference changeLocator(Ice.LocatorPrx newLocator)
        {
            LocatorInfo newLocatorInfo = getInstance().locatorManager().get(newLocator);
            if(locatorInfo_ != null && newLocatorInfo != null && newLocatorInfo.Equals(locatorInfo_))
            {
                return this;
            }
            IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
            r.locatorInfo_ = newLocatorInfo;
            return r;
        }

        public override Reference changeAdapterId(string newAdapterId)
        {
            if(adapterId_.Equals(newAdapterId))
            {
                return this;
            }
            IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
            r.adapterId_ = newAdapterId;
            return r;
        }

        public override Reference changeEndpoints(EndpointI[] newEndpoints)
        {
            if(newEndpoints == null || newEndpoints.Length == 0)
            {
                return this;
            }
            return getInstance().referenceFactory().create(
                getIdentity(), getContext(), getFacet(), getMode(), getSecure(), getPreferSecure(), newEndpoints, 
                getRouterInfo(), getCollocationOptimization(), getCacheConnection(), getEndpointSelection(),
                getThreadPerConnection());
        }

        public override Reference changeLocatorCacheTimeout(int newTimeout)
        {
            if(newTimeout == locatorCacheTimeout_)
            {
                return this;
            }
            IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
            r.locatorCacheTimeout_ = newTimeout;
            return r;
        }

        public override void streamWrite(BasicStream s)
        {
            base.streamWrite(s);

            s.writeSize(0);
            s.writeString(adapterId_);
        }

        public override string ToString()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
            string result = base.ToString();

            if(adapterId_.Length == 0)
            {
                return result;
            }

            StringBuilder s = new StringBuilder();
            s.Append(result);
            s.Append(" @ ");

            //
            // If the encoded adapter id string contains characters which
            // the reference parser uses as separators, then we enclose
            // the adapter id string in quotes.
            //
            string a = IceUtilInternal.StringUtil.escapeString(adapterId_, null);
            if(IceUtilInternal.StringUtil.findFirstOf(a, " \t\n\r") != -1)
            {
                s.Append('"');
                s.Append(a);
                s.Append('"');
            }
            else
            {
                s.Append(a);
            }
            return s.ToString();
        }

        public override Ice.ConnectionI getConnection(out bool comp)
        {
            if(getRouterInfo() != null)
            {
                //
                // If we route, we send everything to the router's client
                // proxy endpoints.
                //
                EndpointI[] endpts = getRouterInfo().getClientEndpoints();
                if(endpts.Length > 0)
                {
                    applyOverrides(ref endpts);
                    return createConnection(endpts, out comp);
                }
            }

            while(true)
            {
                bool cached = false;
                EndpointI[] endpts = null;
                if(locatorInfo_ != null)
                {
                    endpts = locatorInfo_.getEndpoints(this, locatorCacheTimeout_, out cached);
                    applyOverrides(ref endpts);
                }

                if(endpts == null || endpts.Length == 0)
                {
                    throw new Ice.NoEndpointException(ToString());
                }

                try
                {
                    return createConnection(endpts, out comp);
                }
                catch(Ice.NoEndpointException ex)
                {
                    throw ex; // No need to retry if there's no endpoints.
                }
                catch(Ice.LocalException ex)
                {
                    Debug.Assert(locatorInfo_ != null);
                    locatorInfo_.clearCache(this);
                    if(cached)
                    {
                        TraceLevels traceLevels = getInstance().traceLevels();
                        if(traceLevels.retry >= 2)
                        {
                            String s = "connection to cached endpoints failed\n" +
                                       "removing endpoints from cache and trying one more time\n" + ex;
                            getInstance().initializationData().logger.trace(traceLevels.retryCat, s);
                        }
                        continue; // Try again if the endpoints were cached.
                    }
                    throw;
                }
            }
        }

        private sealed class RouterEndpointsCallback : RouterInfo.GetClientEndpointsCallback
        {
            internal RouterEndpointsCallback(IndirectReference ir, GetConnectionCallback cb)
            {
                _ir = ir;
                _cb = cb;
            }

            public void setEndpoints(EndpointI[] endpts)
            {
                if(endpts.Length > 0)
                {
                    _ir.applyOverrides(ref endpts);
                    _ir.createConnection(endpts, _cb);
                }
                else
                {
                    _ir.getConnectionNoRouterInfo(_cb);
                }
            }

            public void setException(Ice.LocalException ex)
            {
                _cb.setException(ex);
            }

            private IndirectReference _ir;
            private GetConnectionCallback _cb;
        }

        public override void getConnection(GetConnectionCallback callback)
        {
            if(getRouterInfo() != null)
            {
                //
                // If we route, we send everything to the router's client
                // proxy endpoints.
                //
                getRouterInfo().getClientEndpoints(new RouterEndpointsCallback(this, callback));
            }
            else
            {
                getConnectionNoRouterInfo(callback);
            }
        }

        private sealed class LocatorEndpointsCallback : LocatorInfo.GetEndpointsCallback
        {
            internal LocatorEndpointsCallback(IndirectReference ir, GetConnectionCallback cb)
            {
                _ir = ir;
                _cb = cb;
            }

            public void setEndpoints(EndpointI[] endpoints, bool cached)
            {
                if(endpoints.Length == 0)
                {
                    _cb.setException(new Ice.NoEndpointException(_ir.ToString()));
                    return;
                }

                _ir.applyOverrides(ref endpoints);
                _ir.createConnection(endpoints, new ConnectionCallback(_ir, _cb, cached));
            }

            public void setException(Ice.LocalException ex)
            {
                _cb.setException(ex);
            }

            private IndirectReference _ir;
            private GetConnectionCallback _cb;
        }

        private sealed class ConnectionCallback : GetConnectionCallback
        {
            internal ConnectionCallback(IndirectReference ir, GetConnectionCallback cb, bool cached)
            {
                _ir = ir;
                _cb = cb;
                _cached = cached;
            }

            public void setConnection(Ice.ConnectionI connection, bool compress)
            {
                _cb.setConnection(connection, compress);
            }

            public void setException(Ice.LocalException exc)
            {
                try
                {
                    throw exc;
                }
                catch(Ice.NoEndpointException ex)
                {
                    _cb.setException(ex); // No need to retry if there's no endpoints.
                }
                catch(Ice.LocalException ex)
                {
                    Debug.Assert(_ir.locatorInfo_ != null);
                    _ir.locatorInfo_.clearCache(_ir);
                    if(_cached)
                    {
                        TraceLevels traceLevels = _ir.getInstance().traceLevels();
                        if(traceLevels.retry >= 2)
                        {
                            String s = "connection to cached endpoints failed\n" +
                                       "removing endpoints from cache and trying one more time\n" + ex;
                            _ir.getInstance().initializationData().logger.trace(traceLevels.retryCat, s);
                        }
                        _ir.getConnectionNoRouterInfo(_cb); // Retry.
                        return;
                    }
                    _cb.setException(ex);
                }
            }

            private IndirectReference _ir;
            private GetConnectionCallback _cb;
            private bool _cached;
        }

        private void getConnectionNoRouterInfo(GetConnectionCallback callback)
        {
            if(locatorInfo_ != null)
            {
                locatorInfo_.getEndpoints(this, locatorCacheTimeout_, new LocatorEndpointsCallback(this, callback));
            }
            else
            {
                callback.setException(new Ice.NoEndpointException(ToString()));
            }
        }

        public override bool Equals(object obj)
        {
            if(object.ReferenceEquals(this, obj))
            {
                return true;
            }
            if(!(obj is IndirectReference))
            {
                return false;
            }
            IndirectReference rhs = (IndirectReference)obj;
            if(!base.Equals(rhs))
            {
                return false;
            }
            if(!adapterId_.Equals(rhs.adapterId_))
            {
                return false;
            }
            if(locatorInfo_ == null ? rhs.locatorInfo_ != null : !locatorInfo_.Equals(rhs.locatorInfo_))
            {
                return false;
            }
            return locatorCacheTimeout_ == rhs.locatorCacheTimeout_;
        }

        //
        // If we override Equals, we must also override GetHashCode.
        //
        public override int GetHashCode()
        {
            lock(this)
            {
                if(base.hashInitialized_)
                {
                    return hashValue_;
                }
                base.GetHashCode();         // Initializes hashValue_.
                int sz = adapterId_.Length; // Add hash of adapter ID to base hash.
                for(int i = 0; i < sz; i++)
                {   
                    hashValue_ = 5 * hashValue_ + (int)adapterId_[i];
                }
                return hashValue_;
            }
        }

        private string adapterId_;
        private LocatorInfo locatorInfo_;
        private int locatorCacheTimeout_;
    }
}
