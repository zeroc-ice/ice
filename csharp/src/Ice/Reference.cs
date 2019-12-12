//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Globalization;
using Ice;
using System.Linq;
using IceUtilInternal;

namespace IceInternal
{
    public abstract class Reference
    {
        public interface GetConnectionCallback
        {
            void setConnection(ConnectionI connection, bool compress);
            void setException(LocalException ex);
        }

        public InvocationMode getMode()
        {
            return _mode;
        }

        public bool getSecure()
        {
            return secure_;
        }

        public ProtocolVersion getProtocol()
        {
            return _protocol;
        }

        public EncodingVersion getEncoding()
        {
            return _encoding;
        }

        public Identity getIdentity()
        {
            return _identity;
        }

        public string getFacet()
        {
            return _facet;
        }

        public Dictionary<string, string> getContext()
        {
            return _context;
        }

        public int
        getInvocationTimeout()
        {
            return _invocationTimeout;
        }

        public bool?
        getCompress()
        {
            if (overrideCompress_)
            {
                return compress_;
            }
            else
            {
                return null;
            }
        }

        public Communicator getCommunicator()
        {
            return _communicator;
        }

        public abstract EndpointI[] getEndpoints();
        public abstract string getAdapterId();
        public abstract LocatorInfo? getLocatorInfo();
        public abstract RouterInfo? getRouterInfo();
        public abstract bool getCollocationOptimized();
        public abstract bool getCacheConnection();
        public abstract bool getPreferSecure();
        public abstract EndpointSelectionType getEndpointSelection();
        public abstract int getLocatorCacheTimeout();
        public abstract string getConnectionId();
        public abstract int? getTimeout();
        public abstract ThreadPool getThreadPool();

        public override int GetHashCode()
        {
            lock (this)
            {
                if (hashInitialized_)
                {
                    return hashValue_;
                }
                int h = 5381;
                HashUtil.hashAdd(ref h, _mode);
                HashUtil.hashAdd(ref h, secure_);
                HashUtil.hashAdd(ref h, _identity);
                HashUtil.hashAdd(ref h, _context);
                HashUtil.hashAdd(ref h, _facet);
                HashUtil.hashAdd(ref h, overrideCompress_);
                if (overrideCompress_)
                {
                    HashUtil.hashAdd(ref h, compress_);
                }
                HashUtil.hashAdd(ref h, _protocol);
                HashUtil.hashAdd(ref h, _encoding);
                HashUtil.hashAdd(ref h, _invocationTimeout);
                hashValue_ = h;
                hashInitialized_ = true;
                return hashValue_;
            }
        }

        public bool getCompressOverride(out bool compress)
        {
            DefaultsAndOverrides defaultsAndOverrides = _communicator.defaultsAndOverrides();
            if (defaultsAndOverrides.overrideCompress)
            {
                compress = defaultsAndOverrides.overrideCompressValue;
            }
            else if (overrideCompress_)
            {
                compress = compress_;
            }
            else
            {
                compress = false;
                return false;
            }
            return true;
        }

        public abstract bool isIndirect();
        public abstract bool isWellKnown();

        //
        // Marshal the reference.
        //
        public virtual void streamWrite(OutputStream s)
        {
            //
            // Don't write the identity here. Operations calling streamWrite
            // write the identity.
            //

            //
            // For compatibility with the old FacetPath.
            //
            if (_facet.Length == 0)
            {
                s.WriteStringSeq(null);
            }
            else
            {
                string[] facetPath = { _facet };
                s.WriteStringSeq(facetPath);
            }

            s.WriteByte((byte)_mode);

            s.WriteBool(secure_);

            if (!s.GetEncoding().Equals(Ice.Util.Encoding_1_0))
            {
                _protocol.ice_writeMembers(s);
                _encoding.ice_writeMembers(s);
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
            string id = _identity.ToString(_communicator.ToStringMode);
            if (StringUtil.findFirstOf(id, " :@") != -1)
            {
                s.Append('"');
                s.Append(id);
                s.Append('"');
            }
            else
            {
                s.Append(id);
            }

            if (_facet.Length > 0)
            {
                //
                // If the encoded facet string contains characters which
                // the reference parser uses as separators, then we enclose
                // the facet string in quotes.
                //
                s.Append(" -f ");
                string fs = StringUtil.escapeString(_facet, "", _communicator.ToStringMode);
                if (StringUtil.findFirstOf(fs, " :@") != -1)
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

            switch (_mode)
            {
                case InvocationMode.Twoway:
                    {
                        s.Append(" -t");
                        break;
                    }

                case InvocationMode.Oneway:
                    {
                        s.Append(" -o");
                        break;
                    }

                case InvocationMode.BatchOneway:
                    {
                        s.Append(" -O");
                        break;
                    }

                case InvocationMode.Datagram:
                    {
                        s.Append(" -d");
                        break;
                    }

                case InvocationMode.BatchDatagram:
                    {
                        s.Append(" -D");
                        break;
                    }
            }

            if (secure_)
            {
                s.Append(" -s");
            }

            if (!_protocol.Equals(Ice.Util.Protocol_1_0))
            {
                //
                // We only print the protocol if it's not 1.0. It's fine as
                // long as we don't add Ice.Default.ProtocolVersion, a
                // stringified proxy will convert back to the same proxy with
                // stringToProxy.
                //
                s.Append(" -p ");
                s.Append(Ice.Util.protocolVersionToString(_protocol));
            }

            //
            // Always print the encoding version to ensure a stringified proxy
            // will convert back to a proxy with the same encoding with
            // stringToProxy (and won't use Ice.Default.EncodingVersion).
            //
            s.Append(" -e ");
            s.Append(Ice.Util.encodingVersionToString(_encoding));

            return s.ToString();

            // Derived class writes the remainder of the string.
        }

        public abstract Dictionary<string, string> toProperty(string prefix);

        public abstract RequestHandler getRequestHandler(IObjectPrx proxy);

        public override bool Equals(object obj)
        {
            //
            // Note: if(this == obj) and type test are performed by each non-abstract derived class.
            //

            Reference r = (Reference)obj; // Guaranteed to succeed.

            if (_mode != r._mode)
            {
                return false;
            }

            if (secure_ != r.secure_)
            {
                return false;
            }

            if (!_identity.Equals(r._identity))
            {
                return false;
            }

            if (!Collections.Equals(_context, r._context))
            {
                return false;
            }

            if (!_facet.Equals(r._facet))
            {
                return false;
            }

            if (overrideCompress_ != r.overrideCompress_)
            {
                return false;
            }
            if (overrideCompress_ && compress_ != r.compress_)
            {
                return false;
            }

            if (!_protocol.Equals(r._protocol))
            {
                return false;
            }

            if (!_encoding.Equals(r._encoding))
            {
                return false;
            }

            if (_invocationTimeout != r._invocationTimeout)
            {
                return false;
            }

            return true;
        }

        public virtual Reference Clone(Identity? identity = null,
                                       string? facet = null,
                                       string? adapterId = null,
                                       bool clearLocator = false,
                                       bool clearRouter = false,
                                       bool? collocationOptimized = null,
                                       bool? compress = null,
                                       bool? connectionCached = null,
                                       string? connectionId = null,
                                       int? connectionTimeout = null,
                                       Dictionary<string, string>? context = null,
                                       EncodingVersion? encodingVersion = null,
                                       EndpointSelectionType? endpointSelectionType = null,
                                       Endpoint[]? endpoints = null,
                                       Connection? fixedConnection = null,
                                       InvocationMode? invocationMode = null,
                                       int? invocationTimeout = null,
                                       LocatorPrx? locator = null,
                                       int? locatorCacheTimeout = null,
                                       bool? oneway = null,
                                       bool? preferSecure = null,
                                       RouterPrx? router = null,
                                       bool? secure = null)
        {
            Reference reference = this;

            //
            // This options must always be handled by one of the super classe.
            //
            Debug.Assert(adapterId == null);
            Debug.Assert(clearLocator == false);
            Debug.Assert(clearRouter == false);
            Debug.Assert(collocationOptimized == null);
            Debug.Assert(connectionCached == null);
            Debug.Assert(connectionId == null);
            Debug.Assert(connectionTimeout == null);
            Debug.Assert(endpointSelectionType == null);
            Debug.Assert(endpoints == null);
            Debug.Assert(locator == null);
            Debug.Assert(locatorCacheTimeout == null);
            Debug.Assert(preferSecure == null);
            Debug.Assert(router == null);
            Debug.Assert(secure == null);

            if (identity != null && !identity.Equals(_identity))
            {
                if (reference == this)
                {
                    reference = Clone();
                }
                reference._identity = identity.Value;
            }

            if (facet != null && !facet.Equals(_facet))
            {
                if (reference == this)
                {
                    reference = Clone();
                }
                reference._facet = facet;
            }

            if (compress is bool compressValue && (!overrideCompress_ || compressValue != compress_))
            {
                if (reference == this)
                {
                    reference = Clone();
                }
                reference.compress_ = compressValue;
                reference.overrideCompress_ = true;
            }

            if (context != null)
            {
                if (reference == this)
                {
                    reference = Clone();
                }
                reference._context = context.Count == 0 ? _emptyContext : new Dictionary<string, string>(context);
            }

            if (encodingVersion is EncodingVersion encodingVersionValue && encodingVersionValue != _encoding)
            {
                if (reference == this)
                {
                    reference = Clone();
                }
                reference._encoding = encodingVersionValue;
            }

            if (invocationMode is InvocationMode invocationModeValue && invocationModeValue != _mode)
            {
                if (reference == this)
                {
                    reference = Clone();
                }
                reference._mode = invocationModeValue;
            }
            else if (oneway is bool onewayValue && (onewayValue && reference._mode != InvocationMode.Oneway ||
                                                    !onewayValue && reference._mode != InvocationMode.Twoway))
            {
                if (reference == this)
                {
                    reference = Clone();
                }
                reference._mode = onewayValue ? InvocationMode.Oneway : InvocationMode.Twoway;
            }

            if (invocationTimeout is int invocationTimeoutValue && invocationTimeoutValue != _invocationTimeout)
            {
                if (invocationTimeoutValue < 1 && invocationTimeoutValue != -1 && invocationTimeoutValue != -2)
                {
                    throw new ArgumentException($"invalid value passed to ice_invocationTimeout: {invocationTimeoutValue}",
                                                nameof(invocationTimeout));
                }

                if (reference == this)
                {
                    reference = Clone();
                }
                reference._invocationTimeout = invocationTimeoutValue;
            }
            return reference;
        }

        public Reference Clone()
        {
            return (Reference)MemberwiseClone();
        }

        protected int hashValue_;
        protected bool hashInitialized_;
        private static Dictionary<string, string> _emptyContext = new Dictionary<string, string>();

        protected Communicator _communicator;

        private InvocationMode _mode;
        private Identity _identity;
        private Dictionary<string, string> _context;
        private string _facet;
        protected bool secure_;
        private ProtocolVersion _protocol;
        private EncodingVersion _encoding;
        private int _invocationTimeout;

        protected bool overrideCompress_;
        protected bool compress_; // Only used if _overrideCompress == true

        protected Reference(Communicator communicator,
                            Identity identity,
                            string facet,
                            InvocationMode mode,
                            bool secure,
                            ProtocolVersion protocol,
                            EncodingVersion encoding,
                            int invocationTimeout,
                            Dictionary<string, string>? context)
        {
            //
            // Validate string arguments.
            //
            Debug.Assert(identity.name != null);
            Debug.Assert(identity.category != null);
            Debug.Assert(facet != null);

            _communicator = communicator;
            _mode = mode;
            _identity = identity;
            _context = context != null ? new Dictionary<string, string>(context) : _emptyContext;
            _facet = facet;
            _protocol = protocol;
            _encoding = encoding;
            _invocationTimeout = invocationTimeout;
            secure_ = secure;
            hashInitialized_ = false;
            overrideCompress_ = false;
            compress_ = false;
        }

        protected static Random rand_ = new Random(unchecked((int)DateTime.Now.Ticks));
    }

    public class FixedReference : Reference
    {
        public FixedReference(Communicator communicator,
                              Identity identity,
                              string facet,
                              InvocationMode mode,
                              bool secure,
                              ProtocolVersion protocol,
                              EncodingVersion encoding,
                              ConnectionI connection,
                              int invocationTimeout,
                              Dictionary<string, string>? context,
                              bool? compress)
        : base(communicator, identity, facet, mode, secure, protocol, encoding, invocationTimeout, context)
        {
            _fixedConnection = connection;
            if (compress is bool compressValue)
            {
                overrideCompress_ = true;
                compress_ = compressValue;
            }
        }

        public override EndpointI[] getEndpoints()
        {
            return Array.Empty<EndpointI>();
        }

        public override string getAdapterId()
        {
            return "";
        }

        public override LocatorInfo? getLocatorInfo()
        {
            return null;
        }

        public override RouterInfo? getRouterInfo()
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

        public override EndpointSelectionType getEndpointSelection()
        {
            return EndpointSelectionType.Random;
        }

        public override int getLocatorCacheTimeout()
        {
            return 0;
        }

        public override string getConnectionId()
        {
            return "";
        }

        public override int? getTimeout()
        {
            return null;
        }

        public override ThreadPool getThreadPool()
        {
            return _fixedConnection.getThreadPool();
        }

        public override Reference Clone(Identity? identity = null,
                                        string? facet = null,
                                        string? adapterId = null,
                                        bool clearLocator = false,
                                        bool clearRouter = false,
                                        bool? collocationOptimized = null,
                                        bool? compress = null,
                                        bool? connectionCached = null,
                                        string? connectionId = null,
                                        int? connectionTimeout = null,
                                        Dictionary<string, string>? context = null,
                                        EncodingVersion? encodingVersion = null,
                                        EndpointSelectionType? endpointSelectionType = null,
                                        Endpoint[]? endpoints = null,
                                        Connection? fixedConnection = null,
                                        InvocationMode? invocationMode = null,
                                        int? invocationTimeout = null,
                                        LocatorPrx? locator = null,
                                        int? locatorCacheTimeout = null,
                                        bool? oneway = null,
                                        bool? preferSecure = null,
                                        RouterPrx? router = null,
                                        bool? secure = null)
        {
            if (locator != null && clearLocator)
            {
                throw new ArgumentException($"You cannot set both {nameof(locator)} and {nameof(clearLocator)}");
            }

            if (router != null && clearRouter)
            {
                throw new ArgumentException($"You cannot set both {nameof(router)} and {nameof(clearRouter)}");
            }

            if (oneway != null && invocationMode != null)
            {
                throw new ArgumentException($"You cannot set both {nameof(oneway)} and {nameof(invocationMode)}");
            }

            if (endpoints != null && adapterId != null)
            {
                throw new ArgumentException($"You cannot set both {nameof(endpoints)} and {nameof(adapterId)}");
            }

            //
            // Check that invocation mode and connection are compatible
            //
            if (invocationMode != null || oneway != null || fixedConnection != null)
            {
                var conn = fixedConnection ?? _fixedConnection;
                var mode = invocationMode;

                if (mode == null && oneway != null)
                {
                    mode = oneway == true ? InvocationMode.Twoway : InvocationMode.Oneway;
                }

                if (mode == null)
                {
                    mode = getMode();
                }
                Debug.Assert(mode != null);

                switch (conn.type())
                {
                    case "udp":
                        {
                            if (mode != InvocationMode.BatchDatagram && mode != InvocationMode.Datagram)
                            {
                                throw new ArgumentException($"InvocationMode {mode} requires a TCP connection");
                            }
                            break;
                        }
                    default:
                        {
                            if (mode == InvocationMode.BatchDatagram || mode == InvocationMode.Datagram)
                            {
                                throw new ArgumentException($"InvocationMode {mode} requires an UDP connection");
                            }
                            break;
                        }
                }
            }

            //
            // Pass down options handle by the base class
            //
            var reference = (FixedReference)base.Clone(identity: identity,
                                                       facet: facet,
                                                       compress: compress,
                                                       context: context,
                                                       encodingVersion: encodingVersion,
                                                       invocationMode: invocationMode,
                                                       invocationTimeout: invocationTimeout,
                                                       oneway: oneway);

            if (fixedConnection != null && fixedConnection != _fixedConnection)
            {
                if (reference == this)
                {
                    reference = (FixedReference)Clone();
                }
                reference._fixedConnection = (ConnectionI)fixedConnection;
                reference.secure_ = fixedConnection.getEndpoint().getInfo().secure();
            }

            if (adapterId != null)
            {
                throw new ArgumentException($"Cannot change the adapter ID of a fixed proxy", nameof(adapterId));
            }

            if (collocationOptimized != null)
            {
                throw new ArgumentException($"Cannot change the collocation optimization configuration of a fixed proxy", nameof(collocationOptimized));
            }

            if (connectionCached != null)
            {
                throw new ArgumentException($"Cannot change the connection caching configuration of a fixed proxy", nameof(connectionCached));
            }

            if (connectionId != null)
            {
                throw new ArgumentException($"Cannot change the connection ID of a fixed proxy", nameof(connectionId));
            }

            if (connectionTimeout != null)
            {
                throw new ArgumentException($"Cannot change the connection timeout of a fixed proxy", nameof(connectionTimeout));
            }

            if (endpointSelectionType != null)
            {
                throw new ArgumentException($"Cannot change the endpoint selection policy of a fixed proxy", nameof(endpointSelectionType));
            }

            if (endpoints != null)
            {
                throw new ArgumentException($"Cannot change the endpoints of a fixed proxy", nameof(endpoints));
            }

            if (locator != null)
            {
                throw new ArgumentException($"Cannot change the locator of a fixed proxy", nameof(locator));
            }
            else if (clearLocator)
            {
                throw new ArgumentException($"Cannot change the locator of a fixed proxy", nameof(clearLocator));
            }

            if (locatorCacheTimeout != null)
            {
                throw new ArgumentException($"Cannot change the locator cache timeout of a fixed proxy", nameof(locatorCacheTimeout));
            }

            if (preferSecure != null)
            {
                throw new ArgumentException($"Cannot change the prefer sercure configuration of a fixed proxy", nameof(preferSecure));
            }

            if (router != null)
            {
                throw new ArgumentException($"Cannot change the router of a fixed proxy", nameof(router));
            }
            else if (clearRouter)
            {
                throw new ArgumentException($"Cannot change the router of a fixed proxy", nameof(clearRouter));
            }

            if (secure != null)
            {
                throw new ArgumentException($"Cannot change the secure configuration of a fixed proxy", nameof(secure));
            }
            return reference;
        }

        public override bool isIndirect()
        {
            return false;
        }

        public override bool isWellKnown()
        {
            return false;
        }

        public override void streamWrite(OutputStream s)
        {
            throw new NotSupportedException("You cannot marshal a fixed proxy");
        }

        public override Dictionary<string, string> toProperty(string prefix)
        {
            throw new NotSupportedException("You cannot convert a fixed proxy to propery dictionary");
        }

        public override RequestHandler getRequestHandler(IObjectPrx proxy)
        {
            switch (getMode())
            {
                case InvocationMode.Twoway:
                case InvocationMode.Oneway:
                case InvocationMode.BatchOneway:
                    {
                        if (_fixedConnection.endpoint().datagram())
                        {
                            throw new NoEndpointException(ToString());
                        }
                        break;
                    }

                case InvocationMode.Datagram:
                case InvocationMode.BatchDatagram:
                    {
                        if (!_fixedConnection.endpoint().datagram())
                        {
                            throw new NoEndpointException(ToString());
                        }
                        break;
                    }
            }

            //
            // If a secure connection is requested or secure overrides is set,
            // check if the connection is secure.
            //
            bool secure;
            DefaultsAndOverrides defaultsAndOverrides = _communicator.defaultsAndOverrides();
            if (defaultsAndOverrides.overrideSecure)
            {
                secure = defaultsAndOverrides.overrideSecureValue;
            }
            else
            {
                secure = getSecure();
            }
            if (secure && !_fixedConnection.endpoint().secure())
            {
                throw new NoEndpointException(ToString());
            }

            _fixedConnection.throwException(); // Throw in case our connection is already destroyed.

            bool compress = false;
            if (defaultsAndOverrides.overrideCompress)
            {
                compress = defaultsAndOverrides.overrideCompressValue;
            }
            else if (overrideCompress_)
            {
                compress = compress_;
            }

            return proxy.IceSetRequestHandler(new ConnectionRequestHandler(this, _fixedConnection, compress));
        }

        public override bool Equals(object obj)
        {
            if (ReferenceEquals(this, obj))
            {
                return true;
            }
            FixedReference? rhs = obj as FixedReference;
            if (rhs == null)
            {
                return false;
            }
            if (!base.Equals(rhs))
            {
                return false;
            }
            return _fixedConnection.Equals(rhs._fixedConnection);
        }

        //
        // If we override Equals, we must also override GetHashCode.
        //
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        private ConnectionI _fixedConnection;
    }

    public class RoutableReference : Reference
    {
        public override EndpointI[] getEndpoints()
        {
            return _endpoints;
        }

        public override string getAdapterId()
        {
            return _adapterId!;
        }

        public override LocatorInfo? getLocatorInfo()
        {
            return _locatorInfo;
        }

        public override RouterInfo? getRouterInfo()
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

        public override EndpointSelectionType getEndpointSelection()
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

        public override int? getTimeout()
        {
            if (_overrideTimeout)
            {
                return _timeout;
            }
            else
            {
                return null;
            }
        }

        public override ThreadPool getThreadPool()
        {
            return _communicator.clientThreadPool();
        }

        public override Reference Clone(Identity? identity = null,
                                        string? facet = null,
                                        string? adapterId = null,
                                        bool clearLocator = false,
                                        bool clearRouter = false,
                                        bool? collocationOptimized = null,
                                        bool? compress = null,
                                        bool? connectionCached = null,
                                        string? connectionId = null,
                                        int? connectionTimeout = null,
                                        Dictionary<string, string>? context = null,
                                        EncodingVersion? encodingVersion = null,
                                        EndpointSelectionType? endpointSelectionType = null,
                                        Endpoint[]? endpoints = null,
                                        Connection? fixedConnection = null,
                                        InvocationMode? invocationMode = null,
                                        int? invocationTimeout = null,
                                        LocatorPrx? locator = null,
                                        int? locatorCacheTimeout = null,
                                        bool? oneway = null,
                                        bool? preferSecure = null,
                                        RouterPrx? router = null,
                                        bool? secure = null)
        {
            if (locator != null && clearLocator)
            {
                throw new ArgumentException($"You cannot set both {nameof(locator)} and {nameof(clearLocator)}");
            }

            if (router != null && clearRouter)
            {
                throw new ArgumentException($"You cannot set both {nameof(router)} and {nameof(clearRouter)}");
            }

            if (oneway != null && invocationMode != null)
            {
                throw new ArgumentException($"You cannot set both {nameof(oneway)} and {nameof(invocationMode)}");
            }

            if (endpoints != null && adapterId != null)
            {
                throw new ArgumentException($"You cannot set both {nameof(endpoints)} and {nameof(adapterId)}");
            }

            //
            // Pass down options handle by the base class
            //
            var reference = (RoutableReference)base.Clone(identity: identity,
                                                          facet: facet,
                                                          compress: compress,
                                                          context: context,
                                                          encodingVersion: encodingVersion,
                                                          invocationMode: invocationMode,
                                                          invocationTimeout: invocationTimeout,
                                                          oneway: oneway);

            if (fixedConnection != null)
            {
                FixedReference fixedReference = new FixedReference(reference.getCommunicator(),
                                                                   reference.getIdentity(),
                                                                   reference.getFacet(),
                                                                   reference.getMode(),
                                                                   fixedConnection.getEndpoint().getInfo().secure(),
                                                                   reference.getProtocol(),
                                                                   reference.getEncoding(),
                                                                   (ConnectionI)fixedConnection,
                                                                   reference.getInvocationTimeout(),
                                                                   reference.getContext(),
                                                                   reference.getCompress());

                return fixedReference.Clone(adapterId: adapterId,
                                            clearLocator: clearLocator,
                                            clearRouter: clearRouter,
                                            collocationOptimized: collocationOptimized,
                                            connectionCached: connectionCached,
                                            connectionId: connectionId,
                                            connectionTimeout: connectionTimeout,
                                            endpointSelectionType: endpointSelectionType,
                                            endpoints: endpoints,
                                            fixedConnection: fixedConnection,
                                            locator: locator,
                                            locatorCacheTimeout: locatorCacheTimeout,
                                            preferSecure: preferSecure,
                                            router: router,
                                            secure: secure);
            }

            if (adapterId != null && !adapterId.Equals(_adapterId))
            {
                if (reference == this)
                {
                    reference = (RoutableReference)Clone();
                }
                reference._adapterId = adapterId;
                reference._endpoints = Array.Empty<EndpointI>();
            }

            if (collocationOptimized is bool collocationOptimizedValue && collocationOptimizedValue != _collocationOptimized)
            {
                if (reference == this)
                {
                    reference = (RoutableReference)Clone();
                }
                reference._collocationOptimized = collocationOptimizedValue;
            }

            if (connectionCached is bool connectionCachedValue && connectionCachedValue != _cacheConnection)
            {
                if (reference == this)
                {
                    reference = (RoutableReference)Clone();
                }
                reference._cacheConnection = connectionCachedValue;
            }

            if (connectionId != null && !connectionId.Equals(_connectionId))
            {
                if (reference == this)
                {
                    reference = (RoutableReference)Clone();
                }
                reference._connectionId = connectionId;
                reference._endpoints = reference._endpoints.Select(e => e.connectionId(connectionId)).ToArray();
            }

            if (connectionTimeout is int connectionTimeoutValue && (!_overrideTimeout || connectionTimeoutValue != _timeout))
            {
                if (connectionTimeoutValue < 1 && connectionTimeoutValue != -1)
                {
                    throw new ArgumentException($"invalid connectionTimeout value: {connectionTimeoutValue}",
                                                nameof(connectionTimeout));
                }

                if (reference == this)
                {
                    reference = (RoutableReference)Clone();
                }
                reference._timeout = connectionTimeoutValue;
                reference._endpoints = reference._endpoints.Select(e => e.timeout(connectionTimeoutValue)).ToArray();
                reference._overrideTimeout = true;
            }

            if (encodingVersion is EncodingVersion encodingVersionValue)
            {
                if (_locatorInfo != null && !_locatorInfo.getLocator().EncodingVersion.Equals(encodingVersionValue))
                {
                    if (reference == this)
                    {
                        reference = (RoutableReference)Clone();
                    }
                    reference._locatorInfo = _communicator.locatorManager().get(
                        _locatorInfo.getLocator().Clone(encodingVersion: encodingVersionValue));
                }
            }

            if (endpointSelectionType is EndpointSelectionType endpointSelectionTypeValue && endpointSelectionTypeValue != _endpointSelection)
            {
                if (reference == this)
                {
                    reference = (RoutableReference)Clone();
                }
                reference._endpointSelection = endpointSelectionTypeValue;
            }

            if (endpoints != null && !endpoints.SequenceEqual(_endpoints))
            {
                if (reference == this)
                {
                    reference = (RoutableReference)Clone();
                }
                reference._adapterId = "";
                reference._endpoints = reference.applyOverrides(endpoints.Select(e => (EndpointI)e).ToArray());
            }

            if (locator != null)
            {
                LocatorInfo locatorInfo = _communicator.locatorManager().get(locator);
                if (!locatorInfo.Equals(_locatorInfo))
                {
                    if (reference == this)
                    {
                        reference = (RoutableReference)Clone();
                    }
                    reference._locatorInfo = locatorInfo;
                }
            }
            else if (clearLocator && _locatorInfo != null)
            {
                if (reference == this)
                {
                    reference = (RoutableReference)Clone();
                }
                reference._locatorInfo = null;
            }

            if (locatorCacheTimeout is int locatorCacheTimeoutValue && locatorCacheTimeoutValue != _locatorCacheTimeout)
            {
                if (locatorCacheTimeoutValue < -1)
                {
                    throw new ArgumentException($"invalid value passed to ice_locatorCacheTimeout: {locatorCacheTimeoutValue}",
                                                nameof(locatorCacheTimeout));
                }
                if (reference == this)
                {
                    reference = (RoutableReference)Clone();
                }
                reference._locatorCacheTimeout = locatorCacheTimeoutValue;
            }

            if (preferSecure is bool preferSecureValue && preferSecureValue != _preferSecure)
            {
                if (reference == this)
                {
                    reference = (RoutableReference)Clone();
                }
                reference._preferSecure = preferSecureValue;
            }

            if (router != null)
            {
                RouterInfo routerInfo = _communicator.routerManager().get(router);
                if (!routerInfo.Equals(_routerInfo))
                {
                    if (reference == this)
                    {
                        reference = (RoutableReference)Clone();
                    }
                    reference._routerInfo = routerInfo;
                }
            }
            else if (clearRouter && _routerInfo != null)
            {
                if (reference == this)
                {
                    reference = (RoutableReference)Clone();
                }
                reference._routerInfo = null;
            }

            if (secure is bool secureValue && secureValue != secure_)
            {
                if (reference == this)
                {
                    reference = (RoutableReference)Clone();
                }
                reference.secure_ = secureValue;
            }
            return reference;
        }

        public override bool isIndirect()
        {
            return _endpoints.Length == 0;
        }

        public override bool isWellKnown()
        {
            return _endpoints.Length == 0 && _adapterId!.Length == 0;
        }

        public override void streamWrite(OutputStream s)
        {
            base.streamWrite(s);

            s.WriteSize(_endpoints.Length);
            if (_endpoints.Length > 0)
            {
                Debug.Assert(_adapterId!.Length == 0);
                foreach (EndpointI endpoint in _endpoints)
                {
                    s.WriteShort(endpoint.type());
                    endpoint.streamWrite(s);
                }
            }
            else
            {
                s.WriteString(_adapterId); // Adapter id.
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

            if (_endpoints.Length > 0)
            {
                for (int i = 0; i < _endpoints.Length; i++)
                {
                    string endp = _endpoints[i].ToString();
                    if (endp != null && endp.Length > 0)
                    {
                        s.Append(':');
                        s.Append(endp);
                    }
                }
            }
            else if (_adapterId!.Length > 0)
            {
                s.Append(" @ ");

                //
                // If the encoded adapter id string contains characters which
                // the reference parser uses as separators, then we enclose
                // the adapter id string in quotes.
                //
                string a = StringUtil.escapeString(_adapterId, null, _communicator.ToStringMode);
                if (StringUtil.findFirstOf(a, " :@") != -1)
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
                       _endpointSelection == EndpointSelectionType.Random ? "Random" : "Ordered";
            properties[prefix + ".LocatorCacheTimeout"] = _locatorCacheTimeout.ToString(CultureInfo.InvariantCulture);
            properties[prefix + ".InvocationTimeout"] = getInvocationTimeout().ToString(CultureInfo.InvariantCulture);

            if (_routerInfo != null)
            {
                var h = _routerInfo.getRouter();
                Dictionary<string, string> routerProperties = h.IceReference.toProperty(prefix + ".Router");
                foreach (KeyValuePair<string, string> entry in routerProperties)
                {
                    properties[entry.Key] = entry.Value;
                }
            }

            if (_locatorInfo != null)
            {
                var h = _locatorInfo.getLocator();
                Dictionary<string, string> locatorProperties = h.IceReference.toProperty(prefix + ".Locator");
                foreach (KeyValuePair<string, string> entry in locatorProperties)
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
            lock (this)
            {
                if (!hashInitialized_)
                {
                    int h = base.GetHashCode(); // Initializes hashValue_.
                    HashUtil.hashAdd(ref h, _adapterId);
                    hashValue_ = h;
                }
                return hashValue_;
            }
        }

        public override bool Equals(object obj)
        {
            if (ReferenceEquals(this, obj))
            {
                return true;
            }

            RoutableReference? rhs = obj as RoutableReference;
            if (rhs == null)
            {
                return false;
            }

            if (!base.Equals(obj))
            {
                return false;
            }

            if (_locatorInfo == null ? rhs._locatorInfo != null : !_locatorInfo.Equals(rhs._locatorInfo))
            {
                return false;
            }
            if (_routerInfo == null ? rhs._routerInfo != null : !_routerInfo.Equals(rhs._routerInfo))
            {
                return false;
            }
            if (_collocationOptimized != rhs._collocationOptimized)
            {
                return false;
            }
            if (_cacheConnection != rhs._cacheConnection)
            {
                return false;
            }
            if (_preferSecure != rhs._preferSecure)
            {
                return false;
            }
            if (_endpointSelection != rhs._endpointSelection)
            {
                return false;
            }
            if (_locatorCacheTimeout != rhs._locatorCacheTimeout)
            {
                return false;
            }
            if (_overrideTimeout != rhs._overrideTimeout)
            {
                return false;
            }
            if (_overrideTimeout && _timeout != rhs._timeout)
            {
                return false;
            }
            if (!_connectionId.Equals(rhs._connectionId))
            {
                return false;
            }
            if (!_adapterId!.Equals(rhs._adapterId))
            {
                return false;
            }
            if (!Collections.Equals(_endpoints, rhs._endpoints))
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
                if (endpts.Length > 0)
                {
                    _ir.createConnection(_ir.applyOverrides(endpts), _cb);
                }
                else
                {
                    _ir.getConnectionNoRouterInfo(_cb);
                }
            }

            public void setException(LocalException ex)
            {
                _cb.setException(ex);
            }

            private RoutableReference _ir;
            private GetConnectionCallback _cb;
        }

        public override RequestHandler getRequestHandler(IObjectPrx proxy)
        {
            return _communicator.requestHandlerFactory().getRequestHandler(this, proxy);
        }

        public void getConnection(GetConnectionCallback callback)
        {
            if (_routerInfo != null)
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
                if (endpoints.Length == 0)
                {
                    _cb.setException(new NoEndpointException(_ir.ToString()));
                    return;
                }

                _ir.createConnection(_ir.applyOverrides(endpoints), new ConnectionCallback(_ir, _cb, cached));
            }

            public void setException(LocalException ex)
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

            public void setConnection(ConnectionI connection, bool compress)
            {
                _cb.setConnection(connection, compress);
            }

            public void setException(LocalException exc)
            {
                try
                {
                    throw exc;
                }
                catch (NoEndpointException ex)
                {
                    _cb.setException(ex); // No need to retry if there's no endpoints.
                }
                catch (LocalException ex)
                {
                    Debug.Assert(_ir._locatorInfo != null);
                    _ir._locatorInfo.clearCache(_ir);
                    if (_cached)
                    {
                        TraceLevels traceLevels = _ir._communicator.traceLevels();
                        if (traceLevels.retry >= 2)
                        {
                            string s = "connection to cached endpoints failed\n" +
                                       "removing endpoints from cache and trying again\n" + ex;
                            _ir._communicator.Logger.trace(traceLevels.retryCat, s);
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
            if (_endpoints.Length > 0)
            {
                createConnection(_endpoints, callback);
                return;
            }

            if (_locatorInfo != null)
            {
                _locatorInfo.getEndpoints(this, _locatorCacheTimeout, new LocatorEndpointsCallback(this, callback));
            }
            else
            {
                callback.setException(new NoEndpointException(ToString()));
            }
        }

        public RoutableReference(Communicator communicator,
                                 Identity identity,
                                 string facet,
                                 InvocationMode mode,
                                 bool secure,
                                 ProtocolVersion protocol,
                                 EncodingVersion encoding,
                                 EndpointI[] endpoints,
                                 string? adapterId,
                                 LocatorInfo? locatorInfo,
                                 RouterInfo? routerInfo,
                                 bool collocationOptimized,
                                 bool cacheConnection,
                                 bool preferSecure,
                                 EndpointSelectionType endpointSelection,
                                 int locatorCacheTimeout,
                                 int invocationTimeout,
                                 Dictionary<string, string>? context)
        : base(communicator, identity, facet, mode, secure, protocol, encoding, invocationTimeout, context)
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

            if (_endpoints == null)
            {
                _endpoints = _emptyEndpoints;
            }

            if (_adapterId == null)
            {
                _adapterId = "";
            }

            Debug.Assert(_adapterId.Length == 0 || _endpoints.Length == 0);
        }

        protected EndpointI[] applyOverrides(EndpointI[] endpts)
        {
            return endpts.Select(endpoint =>
                {
                    endpoint = endpoint.connectionId(_connectionId);
                    if (overrideCompress_)
                    {
                        endpoint = endpoint.compress(compress_);
                    }

                    if (_overrideTimeout)
                    {
                        endpoint.timeout(_timeout);
                    }
                    return endpoint;
                }).ToArray();
        }

        private EndpointI[] filterEndpoints(EndpointI[] allEndpoints)
        {
            List<EndpointI> endpoints = new List<EndpointI>();

            //
            // Filter out unknown endpoints.
            //
            for (int i = 0; i < allEndpoints.Length; i++)
            {
                if (!(allEndpoints[i] is OpaqueEndpointI))
                {
                    endpoints.Add(allEndpoints[i]);
                }
            }

            //
            // Filter out endpoints according to the mode of the reference.
            //
            switch (getMode())
            {
                case InvocationMode.Twoway:
                case InvocationMode.Oneway:
                case InvocationMode.BatchOneway:
                    {
                        //
                        // Filter out datagram endpoints.
                        //
                        List<EndpointI> tmp = new List<EndpointI>();
                        foreach (EndpointI endpoint in endpoints)
                        {
                            if (!endpoint.datagram())
                            {
                                tmp.Add(endpoint);
                            }
                        }
                        endpoints = tmp;
                        break;
                    }

                case InvocationMode.Datagram:
                case InvocationMode.BatchDatagram:
                    {
                        //
                        // Filter out non-datagram endpoints.
                        //
                        List<EndpointI> tmp = new List<EndpointI>();
                        foreach (EndpointI endpoint in endpoints)
                        {
                            if (endpoint.datagram())
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
            switch (getEndpointSelection())
            {
                case EndpointSelectionType.Random:
                    {
                        lock (rand_)
                        {
                            for (int i = 0; i < endpoints.Count - 1; ++i)
                            {
                                int r = rand_.Next(endpoints.Count - i) + i;
                                Debug.Assert(r >= i && r < endpoints.Count);
                                if (r != i)
                                {
                                    EndpointI tmp = endpoints[i];
                                    endpoints[i] = endpoints[r];
                                    endpoints[r] = tmp;
                                }
                            }
                        }
                        break;
                    }
                case EndpointSelectionType.Ordered:
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
            DefaultsAndOverrides overrides = _communicator.defaultsAndOverrides();
            if (overrides.overrideSecure ? overrides.overrideSecureValue : getSecure())
            {
                endpoints = endpoints.Where(endpoint => endpoint.secure()).ToList();
            }
            else if (getPreferSecure())
            {
                endpoints = endpoints.OrderByDescending(endpoint => endpoint.secure()).ToList();
            }
            else
            {
                endpoints = endpoints.OrderBy(endpoint => endpoint.secure()).ToList();
            }

            return endpoints.Select(e => e).ToArray();
        }

        private sealed class CreateConnectionCallback : OutgoingConnectionFactory.CreateConnectionCallback
        {
            internal CreateConnectionCallback(RoutableReference rr, EndpointI[]? endpoints, GetConnectionCallback cb)
            {
                _rr = rr;
                _endpoints = endpoints;
                _callback = cb;
            }

            public void setConnection(ConnectionI connection, bool compress)
            {
                //
                // If we have a router, set the object adapter for this router
                // (if any) to the new connection, so that callbacks from the
                // router can be received over this new connection.
                //
                if (_rr._routerInfo != null && _rr._routerInfo.getAdapter() != null)
                {
                    connection.setAdapter(_rr._routerInfo.getAdapter());
                }
                _callback.setConnection(connection, compress);
            }

            public void setException(LocalException ex)
            {
                if (_exception == null)
                {
                    _exception = ex;
                }

                if (_endpoints == null || ++_i == _endpoints.Length)
                {
                    _callback.setException(_exception);
                    return;
                }

                bool more = _i != _endpoints.Length - 1;
                EndpointI[] endpoint = new EndpointI[] { _endpoints[_i] };
                _rr._communicator.outgoingConnectionFactory().create(endpoint, more, _rr.getEndpointSelection(), this);
            }

            private RoutableReference _rr;
            private EndpointI[]? _endpoints;
            private GetConnectionCallback _callback;
            private int _i = 0;
            private LocalException? _exception = null;
        }

        protected void createConnection(EndpointI[] allEndpoints, GetConnectionCallback callback)
        {
            EndpointI[] endpoints = filterEndpoints(allEndpoints);
            if (endpoints.Length == 0)
            {
                callback.setException(new NoEndpointException(ToString()));
                return;
            }

            //
            // Finally, create the connection.
            //
            OutgoingConnectionFactory factory = _communicator.outgoingConnectionFactory();
            if (getCacheConnection() || endpoints.Length == 1)
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

                factory.create(new EndpointI[] { endpoints[0] }, true, getEndpointSelection(),
                               new CreateConnectionCallback(this, endpoints, callback));
            }
        }

        private class EndpointComparator : IComparer<EndpointI>
        {
            public EndpointComparator(bool preferSecure)
            {
                _preferSecure = preferSecure;
            }

            public int Compare(EndpointI le, EndpointI re)
            {
                bool ls = le.secure();
                bool rs = re.secure();
                if ((ls && rs) || (!ls && !rs))
                {
                    return 0;
                }
                else if (!ls && rs)
                {
                    if (_preferSecure)
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
                    if (_preferSecure)
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
        private static EndpointI[] _emptyEndpoints = Array.Empty<EndpointI>();

        private EndpointI[] _endpoints;
        private string? _adapterId;
        private LocatorInfo? _locatorInfo; // Null if no locator is used.
        private RouterInfo? _routerInfo; // Null if no router is used.
        private bool _collocationOptimized;
        private bool _cacheConnection;
        private bool _preferSecure;
        private EndpointSelectionType _endpointSelection;
        private int _locatorCacheTimeout;

        private bool _overrideTimeout;
        private int _timeout; // Only used if _overrideTimeout == true
        private string _connectionId = "";
    }
}
