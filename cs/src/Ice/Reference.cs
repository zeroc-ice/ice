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

        public Ice.Context getContext()
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
        public Reference changeContext(Ice.Context newContext)
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
                r.context_ = (Ice.Context)newContext.Clone();
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

        public abstract Reference changeSecure(bool newSecure);
        public abstract Reference changePreferSecure(bool newPreferSecure);
        public abstract Reference changeRouter(Ice.RouterPrx newRouter);
        public abstract Reference changeLocator(Ice.LocatorPrx newLocator);
        public abstract Reference changeCompress(bool newCompress);
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
            if(IceUtil.StringUtil.findFirstOf(id, " \t\n\r:@") != -1)
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
                string fs = IceUtil.StringUtil.escapeString(facet_, "");
                if(IceUtil.StringUtil.findFirstOf(fs, " \t\n\r:@") != -1)
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

            if(!context_.Equals(r.context_))
            {
                return false;
            }

            if(!facet_.Equals(r.facet_))
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
        private Ice.Context context_;
        private static Ice.Context _emptyContext = new Ice.Context();
        private string facet_;

        protected int hashValue_;
        protected bool hashInitialized_;

        protected Reference(Instance inst,
                            Ice.Communicator com,
                            Ice.Identity ident,
                            Ice.Context ctx,
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
        }

        protected static System.Random rand_ = new System.Random(unchecked((int)System.DateTime.Now.Ticks));
    }

    public class FixedReference : Reference
    {
        public FixedReference(Instance inst,
                              Ice.Communicator com,
                              Ice.Identity ident,
                              Ice.Context ctx,
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

        public override Reference changeCompress(bool newCompress)
        {
            // TODO: FixedReferences should probably have a _compress flag,
            // that gets its default from the fixed connection this reference
            // refers to. This should be changable with changeCompress().
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
            compress = connection.endpoint().compress();

            return connection;
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
            return IceUtil.Arrays.Equals(_fixedConnections, rhs._fixedConnections);
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
                IceUtil.Arrays.Sort(ref connections, _preferSecureConnectionComparator);
            }
            else
            {
                IceUtil.Arrays.Sort(ref connections, _preferNonSecureConnectionComparator);
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

        public EndpointI[] getRoutedEndpoints()
        {
            if(_routerInfo != null)
            {
                //
                // If we route, we send everything to the router's client
                // proxy endpoints.
                //
                return _routerInfo.getClientEndpoints();
            }
            return new EndpointI[0];
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

        public override Reference changeCompress(bool newCompress)
        {
            if(overrideCompress_ && compress_ == newCompress)
            {
                return this;
            }

            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r.compress_ = newCompress;
            r.overrideCompress_ = true;
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
            if(overrideCompress_ != rhs.overrideCompress_)
            {
                return false;
            }
            if(overrideCompress_ && compress_ != rhs.compress_)
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
                                    Ice.Context ctx,
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
            overrideCompress_ = false;
            compress_ = false;
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

        protected Ice.ConnectionI createConnection(EndpointI[] allEndpoints, out bool comp)
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
                IceUtil.Arrays.Sort(ref endpoints, _preferSecureEndpointComparator);
            }
            else
            {
                IceUtil.Arrays.Sort(ref endpoints, _preferNonSecureEndpointComparator);
            }
            
            if(endpoints.Count == 0)
            {
                Ice.NoEndpointException ex = new Ice.NoEndpointException();
                ex.proxy = ToString();
                throw ex;
            }
            
            //
            // Finally, create the connection.
            //
            OutgoingConnectionFactory factory = getInstance().outgoingConnectionFactory();
            if(getCacheConnection() || endpoints.Count == 1)
            {
                //
                // Get an existing connection or create one if there's no
                // existing connection to one of the given endpoints.
                //
                EndpointI[] arr = new EndpointI[endpoints.Count];
                endpoints.CopyTo(arr);
                return factory.create(arr, false, _threadPerConnection, getEndpointSelection(), out comp);
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

                foreach(EndpointI e in endpoints)
                {
                    try
                    {
                        endpoint[0] = e;
                        return factory.create(endpoint, e != endpoints[endpoints.Count - 1], _threadPerConnection,
                                              getEndpointSelection(), out comp);
                    }
                    catch(Ice.LocalException ex)
                    {
                        exception = ex;
                    }
                }

                Debug.Assert(exception != null);
                throw exception;
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
        private bool overrideCompress_;
        private bool compress_;
        private bool overrideTimeout_;
        private int timeout_;
        private bool _threadPerConnection;
    }

    public class DirectReference : RoutableReference
    {
        public DirectReference(Instance inst,
                               Ice.Communicator com,
                               Ice.Identity ident,
                               Ice.Context ctx,
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
            EndpointI[] endpts = base.getRoutedEndpoints();
            applyOverrides(ref endpts);

            if(endpts.Length == 0)
            {
                endpts = _endpoints; // Endpoint overrides are already applied on these endpoints.
            }

            Ice.ConnectionI connection = createConnection(endpts, out comp);

            //
            // If we have a router, set the object adapter for this router
            // (if any) to the new connection, so that callbacks from the
            // router can be received over this new connection.
            //
            if(getRouterInfo() != null)
            {
                connection.setAdapter(getRouterInfo().getAdapter());
            }

            Debug.Assert(connection != null);
            return connection;
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
            return IceUtil.Arrays.Equals(_endpoints, rhs._endpoints);
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
                                 Ice.Context ctx,
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
            string a = IceUtil.StringUtil.escapeString(adapterId_, null);
            if(IceUtil.StringUtil.findFirstOf(a, " \t\n\r") != -1)
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
            Ice.ConnectionI connection;

            while(true)
            {
                EndpointI[] endpts = base.getRoutedEndpoints();
                bool cached = false;
                if(endpts.Length == 0 && locatorInfo_ != null)
                {
                    endpts = locatorInfo_.getEndpoints(this, locatorCacheTimeout_, out cached);
                }

                applyOverrides(ref endpts);

                try
                {
                    connection = createConnection(endpts, out comp);
                    Debug.Assert(connection != null);
                }
                catch(Ice.NoEndpointException ex)
                {
                    throw ex; // No need to retry if there's no endpoints.
                }
                catch(Ice.LocalException ex)
                {
                    if(getRouterInfo() == null)
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
                            
                            continue;
                        }
                    }

                    throw;
                }

                break;
            }

            //
            // If we have a router, set the object adapter for this router
            // (if any) to the new connection, so that callbacks from the
            // router can be received over this new connection.
            //
            if(getRouterInfo() != null)
            {
                connection.setAdapter(getRouterInfo().getAdapter());
            }

            Debug.Assert(connection != null);
            return connection;
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
