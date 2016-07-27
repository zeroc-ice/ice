// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Globalization;

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

        public bool getSecure()
        {
            return secure_;
        }

        public Ice.ProtocolVersion getProtocol()
        {
            return protocol_;
        }

        public Ice.EncodingVersion getEncoding()
        {
            return encoding_;
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

        public int
        getInvocationTimeout()
        {
            return invocationTimeout_;
        }

        public Ice.Communicator getCommunicator()
        {
            return communicator_;
        }

        public abstract EndpointI[] getEndpoints();
        public abstract string getAdapterId();
        public abstract LocatorInfo getLocatorInfo();
        public abstract RouterInfo getRouterInfo();
        public abstract bool getCollocationOptimized();
        public abstract bool getCacheConnection();
        public abstract bool getPreferSecure();
        public abstract Ice.EndpointSelectionType getEndpointSelection();
        public abstract int getLocatorCacheTimeout();
        public abstract String getConnectionId();

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

        public Reference changeSecure(bool newSecure)
        {
            if(newSecure == secure_)
            {
                return this;
            }
            Reference r = instance_.referenceFactory().copy(this);
            r.secure_ = newSecure;
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

        public Reference changeInvocationTimeout(int newTimeout)
        {
            if(newTimeout == invocationTimeout_)
            {
                return this;
            }
            Reference r = instance_.referenceFactory().copy(this);
            r.invocationTimeout_ = newTimeout;
            return r;
        }

        public virtual Reference changeEncoding(Ice.EncodingVersion newEncoding)
        {
            if(newEncoding.Equals(encoding_))
            {
                return this;
            }
            Reference r = instance_.referenceFactory().copy(this);
            r.encoding_ = newEncoding;
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

        public abstract Reference changeEndpoints(EndpointI[] newEndpoints);
        public abstract Reference changeAdapterId(string newAdapterId);
        public abstract Reference changeLocator(Ice.LocatorPrx newLocator);
        public abstract Reference changeRouter(Ice.RouterPrx newRouter);
        public abstract Reference changeCollocationOptimized(bool newCollocationOptimized);
        public abstract Reference changeCacheConnection(bool newCache);
        public abstract Reference changePreferSecure(bool newPreferSecure);
        public abstract Reference changeEndpointSelection(Ice.EndpointSelectionType newType);
        public abstract Reference changeLocatorCacheTimeout(int newTimeout);

        public abstract Reference changeTimeout(int newTimeout);
        public abstract Reference changeConnectionId(string connectionId);

        public override int GetHashCode()
        {
            lock(this)
            {
                if(hashInitialized_)
                {
                    return hashValue_;
                }
                int h = 5381;
                IceInternal.HashUtil.hashAdd(ref h, mode_);
                IceInternal.HashUtil.hashAdd(ref h, secure_);
                IceInternal.HashUtil.hashAdd(ref h, identity_);
                IceInternal.HashUtil.hashAdd(ref h, context_);
                IceInternal.HashUtil.hashAdd(ref h, facet_);
                IceInternal.HashUtil.hashAdd(ref h, overrideCompress_);
                if(overrideCompress_)
                {
                    IceInternal.HashUtil.hashAdd(ref h, compress_);
                }
                IceInternal.HashUtil.hashAdd(ref h, protocol_);
                IceInternal.HashUtil.hashAdd(ref h, encoding_);
                IceInternal.HashUtil.hashAdd(ref h, invocationTimeout_);
                hashValue_ = h;
                hashInitialized_ = true;
                return hashValue_;
            }
        }

        public abstract bool isIndirect();
        public abstract bool isWellKnown();

        //
        // Marshal the reference.
        //
        public virtual void streamWrite(Ice.OutputStream s)
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

            s.writeBool(secure_);

            if(!s.getEncoding().Equals(Ice.Util.Encoding_1_0))
            {
                protocol_.write__(s);
                encoding_.write__(s);
            }

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
            string id = Ice.Util.identityToString(identity_);
            if(IceUtilInternal.StringUtil.findFirstOf(id, " :@") != -1)
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
                if(IceUtilInternal.StringUtil.findFirstOf(fs, " :@") != -1)
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

            if(secure_)
            {
                s.Append(" -s");
            }

            if(!protocol_.Equals(Ice.Util.Protocol_1_0))
            {
                //
                // We only print the protocol if it's not 1.0. It's fine as
                // long as we don't add Ice.Default.ProtocolVersion, a
                // stringified proxy will convert back to the same proxy with
                // stringToProxy.
                //
                s.Append(" -p ");
                s.Append(Ice.Util.protocolVersionToString(protocol_));
            }

            //
            // Always print the encoding version to ensure a stringified proxy
            // will convert back to a proxy with the same encoding with
            // stringToProxy (and won't use Ice.Default.EncodingVersion).
            //
            s.Append(" -e ");
            s.Append(Ice.Util.encodingVersionToString(encoding_));

            return s.ToString();

            // Derived class writes the remainder of the string.
        }

        public abstract Dictionary<string, string> toProperty(string prefix);

        public abstract RequestHandler getRequestHandler(Ice.ObjectPrxHelperBase proxy);

        public abstract BatchRequestQueue getBatchRequestQueue();

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

            if(secure_ != r.secure_)
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

            if(!protocol_.Equals(r.protocol_))
            {
                return false;
            }

            if(!encoding_.Equals(r.encoding_))
            {
                return false;
            }

            if(invocationTimeout_ != r.invocationTimeout_)
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

        protected int hashValue_;
        protected bool hashInitialized_;
        private static Dictionary<string, string> _emptyContext = new Dictionary<string, string>();

        private Instance instance_;
        private Ice.Communicator communicator_;

        private Mode mode_;
        private Ice.Identity identity_;
        private Dictionary<string, string> context_;
        private string facet_;
        protected bool secure_;
        private Ice.ProtocolVersion protocol_;
        private Ice.EncodingVersion encoding_;
        private int invocationTimeout_;

        protected bool overrideCompress_;
        protected bool compress_; // Only used if _overrideCompress == true

        protected Reference(Instance instance,
                            Ice.Communicator communicator,
                            Ice.Identity identity,
                            string facet,
                            Mode mode,
                            bool secure,
                            Ice.ProtocolVersion protocol,
                            Ice.EncodingVersion encoding,
                            int invocationTimeout,
                            Dictionary<string, string> context)
        {
            //
            // Validate string arguments.
            //
            Debug.Assert(identity.name != null);
            Debug.Assert(identity.category != null);
            Debug.Assert(facet != null);

            instance_ = instance;
            communicator_ = communicator;
            mode_ = mode;
            identity_ = identity;
            context_ = context != null ? new Dictionary<string, string>(context) : _emptyContext;
            facet_ = facet;
            protocol_ = protocol;
            encoding_ = encoding;
            invocationTimeout_ = invocationTimeout;
            secure_ = secure;
            hashInitialized_ = false;
            overrideCompress_ = false;
            compress_ = false;
        }

        protected static System.Random rand_ = new System.Random(unchecked((int)System.DateTime.Now.Ticks));
    }

    public class FixedReference : Reference
    {
        public FixedReference(Instance instance,
                              Ice.Communicator communicator,
                              Ice.Identity identity,
                              string facet,
                              Reference.Mode mode,
                              bool secure,
                              Ice.EncodingVersion encoding,
                              Ice.ConnectionI connection)
        : base(instance, communicator, identity, facet, mode, secure, Ice.Util.Protocol_1_0, encoding, -1, null)
        {
            _fixedConnection = connection;
        }

        public override EndpointI[] getEndpoints()
        {
            return _emptyEndpoints;
        }

        public override string getAdapterId()
        {
            return "";
        }

        public override LocatorInfo getLocatorInfo()
        {
            return null;
        }

        public override RouterInfo getRouterInfo()
        {
            return null;
        }

        public override bool getCollocationOptimized()
        {
            return false;
        }

        public override bool getCacheConnection()
        {
            return true;
        }

        public override bool getPreferSecure()
        {
            return false;
        }

        public override Ice.EndpointSelectionType getEndpointSelection()
        {
            return Ice.EndpointSelectionType.Random;
        }

        public override int getLocatorCacheTimeout()
        {
            return 0;
        }

        public override string getConnectionId()
        {
            return "";
        }

        public override Reference changeEndpoints(EndpointI[] newEndpoints)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeAdapterId(string newAdapterId)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeLocator(Ice.LocatorPrx newLocator)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeRouter(Ice.RouterPrx newRouter)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeCollocationOptimized(bool newCollocationOptimized)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeCacheConnection(bool newCache)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changePreferSecure(bool prefSec)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeEndpointSelection(Ice.EndpointSelectionType newType)
        {
            throw new Ice.FixedProxyException();
        }

        public override Reference changeLocatorCacheTimeout(int newTimeout)
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

        public override bool isIndirect()
        {
            return false;
        }

        public override bool isWellKnown()
        {
            return false;
        }

        public override void streamWrite(Ice.OutputStream s)
        {
            throw new Ice.FixedProxyException();
        }

        public override string ToString()
        {
            throw new Ice.FixedProxyException();
        }

        public override Dictionary<string, string> toProperty(string prefix)
        {
            throw new Ice.FixedProxyException();
        }

        public override RequestHandler getRequestHandler(Ice.ObjectPrxHelperBase proxy)
        {
            switch(getMode())
            {
            case Reference.Mode.ModeTwoway:
            case Reference.Mode.ModeOneway:
            case Reference.Mode.ModeBatchOneway:
            {
                if(_fixedConnection.endpoint().datagram())
                {
                    throw new Ice.NoEndpointException("");
                }
                break;
            }

            case Reference.Mode.ModeDatagram:
            case Reference.Mode.ModeBatchDatagram:
            {
                if(!_fixedConnection.endpoint().datagram())
                {
                    throw new Ice.NoEndpointException("");
                }
                break;
            }
            }

            //
            // If a secure connection is requested or secure overrides is set,
            // check if the connection is secure.
            //
            bool secure;
            DefaultsAndOverrides defaultsAndOverrides = getInstance().defaultsAndOverrides();
            if(defaultsAndOverrides.overrideSecure)
            {
                secure = defaultsAndOverrides.overrideSecureValue;
            }
            else
            {
                secure = getSecure();
            }
            if(secure && !_fixedConnection.endpoint().secure())
            {
                throw new Ice.NoEndpointException("");
            }

            _fixedConnection.throwException(); // Throw in case our connection is already destroyed.

            bool compress;
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
                compress = _fixedConnection.endpoint().compress();
            }

            return ((Ice.ObjectPrxHelperBase)proxy).setRequestHandler__(new ConnectionRequestHandler(this,
                                                                                                     _fixedConnection,
                                                                                                     compress));
        }

        public override BatchRequestQueue getBatchRequestQueue()
        {
            return _fixedConnection.getBatchRequestQueue();
        }

        public override bool Equals(object obj)
        {
            if(object.ReferenceEquals(this, obj))
            {
                return true;
            }
            FixedReference rhs = obj as FixedReference;
            if(rhs == null)
            {
                return false;
            }
            if(!base.Equals(rhs))
            {
                return false;
            }
            return _fixedConnection.Equals(_fixedConnection);
        }

        //
        // If we override Equals, we must also override GetHashCode.
        //
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        private Ice.ConnectionI _fixedConnection;
        private static EndpointI[] _emptyEndpoints = new EndpointI[0];
    }

    public class RoutableReference : Reference
    {
        public override EndpointI[] getEndpoints()
        {
            return _endpoints;
        }

        public override string getAdapterId()
        {
            return _adapterId;
        }

        public override LocatorInfo getLocatorInfo()
        {
            return _locatorInfo;
        }

        public override RouterInfo getRouterInfo()
        {
            return _routerInfo;
        }

        public override bool getCollocationOptimized()
        {
            return _collocationOptimized;
        }

        public override bool getCacheConnection()
        {
            return _cacheConnection;
        }

        public override bool getPreferSecure()
        {
            return _preferSecure;
        }

        public override Ice.EndpointSelectionType getEndpointSelection()
        {
            return _endpointSelection;
        }

        public override int getLocatorCacheTimeout()
        {
            return _locatorCacheTimeout;
        }

        public override string getConnectionId()
        {
            return _connectionId;
        }

        public override Reference changeEncoding(Ice.EncodingVersion newEncoding)
        {
            RoutableReference r = (RoutableReference)base.changeEncoding(newEncoding);
            if(r != this)
            {
                LocatorInfo locInfo = r._locatorInfo;
                if(locInfo != null && !locInfo.getLocator().ice_getEncodingVersion().Equals(newEncoding))
                {
                    r._locatorInfo = getInstance().locatorManager().get(
                        (Ice.LocatorPrx)locInfo.getLocator().ice_encodingVersion(newEncoding));
                }
            }
            return r;
        }

        public override Reference changeCompress(bool newCompress)
        {
            RoutableReference r = (RoutableReference)base.changeCompress(newCompress);
            if(r != this && _endpoints.Length > 0) // Also override the compress flag on the endpoints if it was updated
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

        public override Reference changeEndpoints(EndpointI[] newEndpoints)
        {
            if(Array.Equals(newEndpoints, _endpoints))
            {
                return this;
            }
            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r._endpoints = newEndpoints;
            r._adapterId = "";
            r.applyOverrides(ref r._endpoints);
            return r;
        }

        public override Reference changeAdapterId(string newAdapterId)
        {
            if(_adapterId.Equals(newAdapterId))
            {
                return this;
            }
            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r._adapterId = newAdapterId;
            r._endpoints = _emptyEndpoints;
            return r;
        }

        public override Reference changeLocator(Ice.LocatorPrx newLocator)
        {
            LocatorInfo newLocatorInfo = getInstance().locatorManager().get(newLocator);
            if(newLocatorInfo != null && _locatorInfo != null && newLocatorInfo.Equals(_locatorInfo))
            {
                return this;
            }
            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r._locatorInfo = newLocatorInfo;
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

        public override Reference changeCollocationOptimized(bool newCollocationOptimized)
        {
            if(newCollocationOptimized == _collocationOptimized)
            {
                return this;
            }
            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r._collocationOptimized = newCollocationOptimized;
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

        public override Reference changeLocatorCacheTimeout(int newTimeout)
        {
            if(newTimeout == _locatorCacheTimeout)
            {
                return this;
            }
            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r._locatorCacheTimeout = newTimeout;
            return r;
        }

        public override Reference changeTimeout(int newTimeout)
        {
            if(_overrideTimeout && _timeout == newTimeout)
            {
                return this;
            }

            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r._timeout = newTimeout;
            r._overrideTimeout = true;
            if(_endpoints.Length > 0)
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

        public override Reference changeConnectionId(string id)
        {
            if(_connectionId.Equals(id))
            {
                return this;
            }
            RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
            r._connectionId = id;
            if(_endpoints.Length > 0)
            {
                EndpointI[] newEndpoints = new EndpointI[_endpoints.Length];
                for(int i = 0; i < _endpoints.Length; i++)
                {
                    newEndpoints[i] = _endpoints[i].connectionId(id);
                }
                r._endpoints = newEndpoints;
            }
            return r;
        }

        public override bool isIndirect()
        {
            return _endpoints.Length == 0;
        }

        public override bool isWellKnown()
        {
            return _endpoints.Length == 0 && _adapterId.Length == 0;
        }

        public override void streamWrite(Ice.OutputStream s)
        {
            base.streamWrite(s);

            s.writeSize(_endpoints.Length);
            if(_endpoints.Length > 0)
            {
                Debug.Assert(_adapterId.Length == 0);
                foreach(EndpointI endpoint in _endpoints)
                {
                    s.writeShort(endpoint.type());
                    endpoint.streamWrite(s);
                }
            }
            else
            {
                s.writeString(_adapterId); // Adapter id.
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

            if(_endpoints.Length > 0)
            {
                for(int i = 0; i < _endpoints.Length; i++)
                {
                    string endp = _endpoints[i].ToString();
                    if(endp != null && endp.Length > 0)
                    {
                        s.Append(':');
                        s.Append(endp);
                    }
                }
            }
            else if(_adapterId.Length > 0)
            {
                s.Append(" @ ");

                //
                // If the encoded adapter id string contains characters which
                // the reference parser uses as separators, then we enclose
                // the adapter id string in quotes.
                //
                string a = IceUtilInternal.StringUtil.escapeString(_adapterId, null);
                if(IceUtilInternal.StringUtil.findFirstOf(a, " :@") != -1)
                {
                    s.Append('"');
                    s.Append(a);
                    s.Append('"');
                }
                else
                {
                    s.Append(a);
                 }
            }
            return s.ToString();
        }

        public override Dictionary<string, string> toProperty(string prefix)
        {
            Dictionary<string, string> properties = new Dictionary<string, string>();

            properties[prefix] = ToString();
            properties[prefix + ".CollocationOptimized"] = _collocationOptimized ? "1" : "0";
            properties[prefix + ".ConnectionCached"] = _cacheConnection ? "1" : "0";
            properties[prefix + ".PreferSecure"] = _preferSecure ? "1" : "0";
            properties[prefix + ".EndpointSelection"] =
                       _endpointSelection == Ice.EndpointSelectionType.Random ? "Random" : "Ordered";
            properties[prefix + ".LocatorCacheTimeout"] = _locatorCacheTimeout.ToString(CultureInfo.InvariantCulture);
            properties[prefix + ".InvocationTimeout"] = getInvocationTimeout().ToString(CultureInfo.InvariantCulture);

            if(_routerInfo != null)
            {
                Ice.ObjectPrxHelperBase h = (Ice.ObjectPrxHelperBase)_routerInfo.getRouter();
                Dictionary<String, String> routerProperties = h.reference__().toProperty(prefix + ".Router");
                foreach(KeyValuePair<string, string> entry in routerProperties)
                {
                    properties[entry.Key] = entry.Value;
                }
            }

            if(_locatorInfo != null)
            {
                Ice.ObjectPrxHelperBase h = (Ice.ObjectPrxHelperBase)_locatorInfo.getLocator();
                Dictionary<String, String> locatorProperties = h.reference__().toProperty(prefix + ".Locator");
                foreach(KeyValuePair<string, string> entry in locatorProperties)
                {
                    properties[entry.Key] = entry.Value;
                }
            }

            return properties;
        }

        //
        // If we override Equals, we must also override GetHashCode.
        //
        public override int GetHashCode()
        {
            lock(this)
            {
                if(!hashInitialized_)
                {
                    int h = base.GetHashCode(); // Initializes hashValue_.
                    IceInternal.HashUtil.hashAdd(ref h, _adapterId);
                    hashValue_ = h;
                }
                return hashValue_;
            }
        }

        public override bool Equals(object obj)
        {
            if(Object.ReferenceEquals(this, obj))
            {
                return true;
            }

            RoutableReference rhs = obj as RoutableReference;
            if(rhs == null)
            {
                return false;
            }

            if(!base.Equals(obj))
            {
                return false;
            }

            if(_locatorInfo == null ? rhs._locatorInfo != null : !_locatorInfo.Equals(rhs._locatorInfo))
            {
                return false;
            }
            if(_routerInfo == null ? rhs._routerInfo != null : !_routerInfo.Equals(rhs._routerInfo))
            {
                return false;
            }
            if(_collocationOptimized != rhs._collocationOptimized)
            {
                return false;
            }
            if(_cacheConnection != rhs._cacheConnection)
            {
                return false;
            }
            if(_preferSecure != rhs._preferSecure)
            {
                return false;
            }
            if(_endpointSelection != rhs._endpointSelection)
            {
                return false;
            }
            if(_locatorCacheTimeout != rhs._locatorCacheTimeout)
            {
                return false;
            }
            if(_overrideTimeout != rhs._overrideTimeout)
            {
                return false;
            }
            if(_overrideTimeout && _timeout != rhs._timeout)
            {
                return false;
            }
            if(!_connectionId.Equals(rhs._connectionId))
            {
                return false;
            }
            if(!_adapterId.Equals(rhs._adapterId))
            {
                return false;
            }
            if(!IceUtilInternal.Arrays.Equals(_endpoints, rhs._endpoints))
            {
                return false;
            }
            return true;
        }


        private sealed class RouterEndpointsCallback : RouterInfo.GetClientEndpointsCallback
        {
            internal RouterEndpointsCallback(RoutableReference ir, GetConnectionCallback cb)
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

            private RoutableReference _ir;
            private GetConnectionCallback _cb;
        }

        public override RequestHandler getRequestHandler(Ice.ObjectPrxHelperBase proxy)
        {
            return getInstance().requestHandlerFactory().getRequestHandler(this, proxy);
        }

        public override BatchRequestQueue getBatchRequestQueue()
        {
            return new BatchRequestQueue(getInstance(), getMode() == Reference.Mode.ModeBatchDatagram);
        }

        public void getConnection(GetConnectionCallback callback)
        {
            if(_routerInfo != null)
            {
                //
                // If we route, we send everything to the router's client
                // proxy endpoints.
                //
                _routerInfo.getClientEndpoints(new RouterEndpointsCallback(this, callback));
            }
            else
            {
                getConnectionNoRouterInfo(callback);
            }
        }

        private sealed class LocatorEndpointsCallback : LocatorInfo.GetEndpointsCallback
        {
            internal LocatorEndpointsCallback(RoutableReference ir, GetConnectionCallback cb)
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

            private RoutableReference _ir;
            private GetConnectionCallback _cb;
        }

        private sealed class ConnectionCallback : GetConnectionCallback
        {
            internal ConnectionCallback(RoutableReference ir, GetConnectionCallback cb, bool cached)
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
                    Debug.Assert(_ir._locatorInfo != null);
                    _ir._locatorInfo.clearCache(_ir);
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

            private RoutableReference _ir;
            private GetConnectionCallback _cb;
            private bool _cached;
        }

        private void getConnectionNoRouterInfo(GetConnectionCallback callback)
        {
            if(_endpoints.Length > 0)
            {
                createConnection(_endpoints, callback);
                return;
            }

            if(_locatorInfo != null)
            {
                _locatorInfo.getEndpoints(this, _locatorCacheTimeout, new LocatorEndpointsCallback(this, callback));
            }
            else
            {
                callback.setException(new Ice.NoEndpointException(ToString()));
            }
        }

        public RoutableReference(Instance instance,
                                 Ice.Communicator communicator,
                                 Ice.Identity identity,
                                 string facet,
                                 Reference.Mode mode,
                                 bool secure,
                                 Ice.ProtocolVersion protocol,
                                 Ice.EncodingVersion encoding,
                                 EndpointI[] endpoints,
                                 string adapterId,
                                 LocatorInfo locatorInfo,
                                 RouterInfo routerInfo,
                                 bool collocationOptimized,
                                 bool cacheConnection,
                                 bool preferSecure,
                                 Ice.EndpointSelectionType endpointSelection,
                                 int locatorCacheTimeout,
                                 int invocationTimeout,
                                 Dictionary<string, string> context)
        : base(instance, communicator, identity, facet, mode, secure, protocol, encoding, invocationTimeout, context)
        {
            _endpoints = endpoints;
            _adapterId = adapterId;
            _locatorInfo = locatorInfo;
            _routerInfo = routerInfo;
            _collocationOptimized = collocationOptimized;
            _cacheConnection = cacheConnection;
            _preferSecure = preferSecure;
            _endpointSelection = endpointSelection;
            _locatorCacheTimeout = locatorCacheTimeout;
            _overrideTimeout = false;
            _timeout = -1;

            if(_endpoints == null)
            {
                _endpoints = _emptyEndpoints;
            }

            if(_adapterId == null)
            {
                _adapterId = "";
            }

            Debug.Assert(_adapterId.Length == 0 || _endpoints.Length == 0);
        }

        protected void applyOverrides(ref EndpointI[] endpts)
        {
            for(int i = 0; i < endpts.Length; ++i)
            {
                endpts[i] = endpts[i].connectionId(_connectionId);
                if(overrideCompress_)
                {
                    endpts[i] = endpts[i].compress(compress_);
                }
                if(_overrideTimeout)
                {
                    endpts[i] = endpts[i].timeout(_timeout);
                }
            }
        }

        private EndpointI[] filterEndpoints(EndpointI[] allEndpoints)
        {
            List<EndpointI> endpoints = new List<EndpointI>();

            //
            // Filter out unknown endpoints.
            //
            for(int i = 0; i < allEndpoints.Length; i++)
            {
                if(!(allEndpoints[i] is IceInternal.OpaqueEndpointI))
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
                    List<EndpointI> tmp = new List<EndpointI>();
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
                    List<EndpointI> tmp = new List<EndpointI>();
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
                    lock(rand_)
                    {
                        for(int i = 0; i < endpoints.Count - 1; ++i)
                        {
                            int r = rand_.Next(endpoints.Count - i) + i;
                            Debug.Assert(r >= i && r < endpoints.Count);
                            if(r != i)
                            {
                                EndpointI tmp = endpoints[i];
                                endpoints[i] = endpoints[r];
                                endpoints[r] = tmp;
                            }
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
                List<EndpointI> tmp = new List<EndpointI>();
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
                IceUtilInternal.Collections.Sort(ref endpoints, _preferSecureEndpointComparator);
            }
            else
            {
                IceUtilInternal.Collections.Sort(ref endpoints, _preferNonSecureEndpointComparator);
            }

            EndpointI[] arr = new EndpointI[endpoints.Count];
            endpoints.CopyTo(arr);
            return arr;
        }

        private sealed class CreateConnectionCallback : OutgoingConnectionFactory.CreateConnectionCallback
        {
            internal CreateConnectionCallback(RoutableReference rr, EndpointI[] endpoints, GetConnectionCallback cb)
            {
                _rr = rr;
                _endpoints = endpoints;
                _callback = cb;
            }

            public void setConnection(Ice.ConnectionI connection, bool compress)
            {
                //
                // If we have a router, set the object adapter for this router
                // (if any) to the new connection, so that callbacks from the
                // router can be received over this new connection.
                //
                if(_rr._routerInfo != null && _rr._routerInfo.getAdapter() != null)
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
                _rr.getInstance().outgoingConnectionFactory().create(endpoint, more, _rr.getEndpointSelection(), this);
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
                factory.create(endpoints, false, getEndpointSelection(),
                               new CreateConnectionCallback(this, null, callback));
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

                factory.create(new EndpointI[]{ endpoints[0] }, true, getEndpointSelection(),
                               new CreateConnectionCallback(this, endpoints, callback));
            }
        }

        private class EndpointComparator : IComparer<IceInternal.EndpointI>
        {
            public EndpointComparator(bool preferSecure)
            {
                _preferSecure = preferSecure;
            }

            public int Compare(IceInternal.EndpointI le, IceInternal.EndpointI re)
            {
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
        private static EndpointI[] _emptyEndpoints = new EndpointI[0];

        private EndpointI[] _endpoints;
        private string _adapterId;
        private LocatorInfo _locatorInfo; // Null if no locator is used.
        private RouterInfo _routerInfo; // Null if no router is used.
        private bool _collocationOptimized;
        private bool _cacheConnection;
        private bool _preferSecure;
        private Ice.EndpointSelectionType _endpointSelection;
        private int _locatorCacheTimeout;

        private bool _overrideTimeout;
        private int _timeout; // Only used if _overrideTimeout == true
        private string _connectionId = "";
    }
}
