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
    public abstract class Reference : IEquatable<Reference>
    {
        public interface IGetConnectionCallback
        {
            void SetConnection(Connection connection, bool compress);
            void SetException(LocalException ex);
        }

        public InvocationMode GetMode() => _mode;

        public bool GetSecure() => Secure;

        public ProtocolVersion GetProtocol() => _protocol;

        public EncodingVersion GetEncoding() => _encoding;

        public Identity GetIdentity() => _identity;

        public string GetFacet() => _facet;

        public Dictionary<string, string> GetContext() => _context;

        public int
        GetInvocationTimeout() => _invocationTimeout;

        public bool?
        GetCompress()
        {
            if (OverrideCompress)
            {
                return Compress;
            }
            else
            {
                return null;
            }
        }

        public Communicator GetCommunicator() => Communicator;

        public abstract Endpoint[] GetEndpoints();
        public abstract string GetAdapterId();
        public abstract LocatorInfo? GetLocatorInfo();
        public abstract RouterInfo? GetRouterInfo();
        public abstract bool GetCollocationOptimized();
        public abstract bool GetCacheConnection();
        public abstract bool GetPreferSecure();
        public abstract EndpointSelectionType GetEndpointSelection();
        public abstract int GetLocatorCacheTimeout();
        public abstract string GetConnectionId();
        public abstract int? GetTimeout();
        public abstract ThreadPool GetThreadPool();

        public override int GetHashCode()
        {
            lock (this)
            {
                if (HashInitialized)
                {
                    return HashValue;
                }
                var hash = new HashCode();
                hash.Add(_mode);
                hash.Add(Secure);
                hash.Add(_identity);
                hash.Add(Collections.GetHashCode(_context));
                hash.Add(_facet);
                hash.Add(OverrideCompress);
                if (OverrideCompress)
                {
                    hash.Add(Compress);
                }
                hash.Add(_protocol);
                hash.Add(_encoding);
                hash.Add(_invocationTimeout);
                HashValue = hash.ToHashCode();
                HashInitialized = true;
                return HashValue;
            }
        }

        public bool GetCompressOverride(out bool compress)
        {
            DefaultsAndOverrides defaultsAndOverrides = Communicator.DefaultsAndOverrides;
            if (defaultsAndOverrides.OverrideCompress)
            {
                compress = defaultsAndOverrides.OverrideCompressValue;
            }
            else if (OverrideCompress)
            {
                compress = Compress;
            }
            else
            {
                compress = false;
                return false;
            }
            return true;
        }

        public abstract bool IsIndirect();
        public abstract bool IsWellKnown();

        //
        // Marshal the reference.
        //
        public virtual void StreamWrite(OutputStream s)
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
                s.WriteStringSeq(Array.Empty<string>());
            }
            else
            {
                string[] facetPath = { _facet };
                s.WriteStringSeq(facetPath);
            }

            s.WriteByte((byte)_mode);

            s.WriteBool(Secure);

            if (!s.Encoding.Equals(Ice.Util.Encoding_1_0))
            {
                s.WriteByte(_protocol.Major);
                s.WriteByte(_protocol.Minor);
                s.WriteByte(_encoding.Major);
                s.WriteByte(_encoding.Minor);
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
            var s = new StringBuilder();

            //
            // If the encoded identity string contains characters which
            // the reference parser uses as separators, then we enclose
            // the identity string in quotes.
            //
            string id = _identity.ToString(Communicator.ToStringMode);
            if (StringUtil.FindFirstOf(id, " :@") != -1)
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
                string fs = StringUtil.EscapeString(_facet, "", Communicator.ToStringMode);
                if (StringUtil.FindFirstOf(fs, " :@") != -1)
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

            if (Secure)
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
                s.Append(Ice.Util.ProtocolVersionToString(_protocol));
            }

            //
            // Always print the encoding version to ensure a stringified proxy
            // will convert back to a proxy with the same encoding with
            // stringToProxy (and won't use Ice.Default.EncodingVersion).
            //
            s.Append(" -e ");
            s.Append(Ice.Util.EncodingVersionToString(_encoding));

            return s.ToString();

            // Derived class writes the remainder of the string.
        }

        public abstract Dictionary<string, string> ToProperty(string prefix);

        public abstract IRequestHandler GetRequestHandler(IObjectPrx proxy);

        public static bool operator ==(Reference? lhs, Reference? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs is null || rhs is null)
            {
                return false;
            }
            return rhs.Equals(lhs);

        }

        public static bool operator !=(Reference? lhs, Reference? rhs) => !(lhs == rhs);

        public override bool Equals(object obj) => Equals(obj as Reference);

        public virtual bool Equals(Reference? other)
        {
            //
            // Note: if(this == obj) and type test are performed by each non-abstract derived class.
            //
            Debug.Assert(other != null);
            if (_mode != other._mode)
            {
                return false;
            }

            if (Secure != other.Secure)
            {
                return false;
            }

            if (!_identity.Equals(other._identity))
            {
                return false;
            }

            if (!Collections.Equals(_context, other._context))
            {
                return false;
            }

            if (!_facet.Equals(other._facet))
            {
                return false;
            }

            if (OverrideCompress != other.OverrideCompress)
            {
                return false;
            }

            if (OverrideCompress && Compress != other.Compress)
            {
                return false;
            }

            if (!_protocol.Equals(other._protocol))
            {
                return false;
            }

            if (!_encoding.Equals(other._encoding))
            {
                return false;
            }

            if (_invocationTimeout != other._invocationTimeout)
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
                                       IEndpoint[]? endpoints = null,
                                       Connection? fixedConnection = null,
                                       InvocationMode? invocationMode = null,
                                       int? invocationTimeout = null,
                                       ILocatorPrx? locator = null,
                                       int? locatorCacheTimeout = null,
                                       bool? oneway = null,
                                       bool? preferSecure = null,
                                       IRouterPrx? router = null,
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

            if (compress is bool compressValue && (!OverrideCompress || compressValue != Compress))
            {
                if (reference == this)
                {
                    reference = Clone();
                }
                reference.Compress = compressValue;
                reference.OverrideCompress = true;
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
            else if (oneway is bool onewayValue && ((onewayValue && reference._mode != InvocationMode.Oneway) ||
                                                    (!onewayValue && reference._mode != InvocationMode.Twoway)))
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

        public Reference Clone() => (Reference)MemberwiseClone();

        protected int HashValue;
        protected bool HashInitialized;
        private static readonly Dictionary<string, string> _emptyContext = new Dictionary<string, string>();

        protected Communicator Communicator;

        private InvocationMode _mode;
        private Identity _identity;
        private Dictionary<string, string> _context;
        private string _facet;
        protected bool Secure;
        private ProtocolVersion _protocol;
        private EncodingVersion _encoding;
        private int _invocationTimeout;

        protected bool OverrideCompress;
        protected bool Compress; // Only used if _overrideCompress == true

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
            Debug.Assert(identity.Name != null);
            Debug.Assert(identity.Category != null);
            Debug.Assert(facet != null);

            Communicator = communicator;
            _mode = mode;
            _identity = identity;
            _context = context != null ? new Dictionary<string, string>(context) : _emptyContext;
            _facet = facet;
            _protocol = protocol;
            _encoding = encoding;
            _invocationTimeout = invocationTimeout;
            Secure = secure;
            HashInitialized = false;
            OverrideCompress = false;
            Compress = false;
        }

        protected static Random Rand = new Random(unchecked((int)DateTime.Now.Ticks));
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
                              Connection connection,
                              int invocationTimeout,
                              Dictionary<string, string>? context,
                              bool? compress)
        : base(communicator, identity, facet, mode, secure, protocol, encoding, invocationTimeout, context)
        {
            _fixedConnection = connection;
            if (compress is bool compressValue)
            {
                OverrideCompress = true;
                Compress = compressValue;
            }
        }

        public override Endpoint[] GetEndpoints() => Array.Empty<Endpoint>();

        public override string GetAdapterId() => "";

        public override LocatorInfo? GetLocatorInfo() => null;

        public override RouterInfo? GetRouterInfo() => null;

        public override bool GetCollocationOptimized() => false;

        public override bool GetCacheConnection() => true;

        public override bool GetPreferSecure() => false;

        public override EndpointSelectionType GetEndpointSelection() => EndpointSelectionType.Random;

        public override int GetLocatorCacheTimeout() => 0;

        public override string GetConnectionId() => "";

        public override int? GetTimeout() => null;

        public override ThreadPool GetThreadPool() => _fixedConnection.ThreadPool;

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
                                        IEndpoint[]? endpoints = null,
                                        Connection? fixedConnection = null,
                                        InvocationMode? invocationMode = null,
                                        int? invocationTimeout = null,
                                        ILocatorPrx? locator = null,
                                        int? locatorCacheTimeout = null,
                                        bool? oneway = null,
                                        bool? preferSecure = null,
                                        IRouterPrx? router = null,
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
                Connection conn = fixedConnection ?? _fixedConnection;
                InvocationMode? mode = invocationMode;

                if (mode == null && oneway != null)
                {
                    mode = oneway == true ? InvocationMode.Twoway : InvocationMode.Oneway;
                }

                if (mode == null)
                {
                    mode = GetMode();
                }
                Debug.Assert(mode != null);

                switch (conn.Type())
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
                reference._fixedConnection = fixedConnection;
                reference.Secure = fixedConnection.Endpoint.GetInfo().Secure();
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

        public override bool IsIndirect() => false;

        public override bool IsWellKnown() => false;

        public override void StreamWrite(OutputStream s) => throw new NotSupportedException("You cannot marshal a fixed proxy");

        public override Dictionary<string, string> ToProperty(string prefix) => throw new NotSupportedException("You cannot convert a fixed proxy to propery dictionary");

        public override IRequestHandler GetRequestHandler(IObjectPrx proxy)
        {
            switch (GetMode())
            {
                case InvocationMode.Twoway:
                case InvocationMode.Oneway:
                case InvocationMode.BatchOneway:
                    {
                        if (((Endpoint)_fixedConnection.Endpoint).Datagram())
                        {
                            throw new NoEndpointException(ToString());
                        }
                        break;
                    }

                case InvocationMode.Datagram:
                case InvocationMode.BatchDatagram:
                    {
                        if (!((Endpoint)_fixedConnection.Endpoint).Datagram())
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
            DefaultsAndOverrides defaultsAndOverrides = Communicator.DefaultsAndOverrides;
            if (defaultsAndOverrides.OverrideSecure)
            {
                secure = defaultsAndOverrides.OverrideSecureValue;
            }
            else
            {
                secure = GetSecure();
            }
            if (secure && !((Endpoint)_fixedConnection.Endpoint).Secure())
            {
                throw new NoEndpointException(ToString());
            }

            _fixedConnection.ThrowException(); // Throw in case our connection is already destroyed.

            bool compress = false;
            if (defaultsAndOverrides.OverrideCompress)
            {
                compress = defaultsAndOverrides.OverrideCompressValue;
            }
            else if (OverrideCompress)
            {
                compress = Compress;
            }

            return proxy.IceSetRequestHandler(new ConnectionRequestHandler(this, _fixedConnection, compress));
        }

        public override bool Equals(Reference? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }

            if (!(other is FixedReference rhs))
            {
                return false;
            }
            if (!base.Equals(rhs))
            {
                return false;
            }
            return _fixedConnection.Equals(rhs._fixedConnection);
        }

        public override int GetHashCode() => HashCode.Combine(base.GetHashCode(), _fixedConnection);

        private Connection _fixedConnection;
    }

    public class RoutableReference : Reference
    {
        public override Endpoint[] GetEndpoints() => _endpoints;

        public override string GetAdapterId() => _adapterId!;

        public override LocatorInfo? GetLocatorInfo() => _locatorInfo;

        public override RouterInfo? GetRouterInfo() => _routerInfo;

        public override bool GetCollocationOptimized() => _collocationOptimized;

        public override bool GetCacheConnection() => _cacheConnection;

        public override bool GetPreferSecure() => _preferSecure;

        public override EndpointSelectionType GetEndpointSelection() => _endpointSelection;

        public override int GetLocatorCacheTimeout() => _locatorCacheTimeout;

        public override string GetConnectionId() => _connectionId;

        public override int? GetTimeout()
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

        public override ThreadPool GetThreadPool() => Communicator.ClientThreadPool();

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
                                        IEndpoint[]? endpoints = null,
                                        Connection? fixedConnection = null,
                                        InvocationMode? invocationMode = null,
                                        int? invocationTimeout = null,
                                        ILocatorPrx? locator = null,
                                        int? locatorCacheTimeout = null,
                                        bool? oneway = null,
                                        bool? preferSecure = null,
                                        IRouterPrx? router = null,
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
                var fixedReference = new FixedReference(reference.GetCommunicator(),
                                                        reference.GetIdentity(),
                                                        reference.GetFacet(),
                                                        reference.GetMode(),
                                                        fixedConnection.Endpoint.GetInfo().Secure(),
                                                        reference.GetProtocol(),
                                                        reference.GetEncoding(),
                                                        fixedConnection,
                                                        reference.GetInvocationTimeout(),
                                                        reference.GetContext(),
                                                        reference.GetCompress());

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
                reference._endpoints = Array.Empty<Endpoint>();
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
                reference._endpoints = reference._endpoints.Select(e => e.ConnectionId(connectionId)).ToArray();
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
                reference._endpoints = reference._endpoints.Select(e => e.Timeout(connectionTimeoutValue)).ToArray();
                reference._overrideTimeout = true;
            }

            if (encodingVersion is EncodingVersion encodingVersionValue)
            {
                if (_locatorInfo != null && !_locatorInfo.Locator.EncodingVersion.Equals(encodingVersionValue))
                {
                    if (reference == this)
                    {
                        reference = (RoutableReference)Clone();
                    }
                    reference._locatorInfo = Communicator.GetLocatorInfo(
                        _locatorInfo.Locator.Clone(encodingVersion: encodingVersionValue));
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
                reference._endpoints = reference.ApplyOverrides(endpoints.Select(e => (Endpoint)e).ToArray());
            }

            if (locator != null)
            {
                LocatorInfo locatorInfo = Communicator.GetLocatorInfo(locator);
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
                RouterInfo routerInfo = Communicator.GetRouterInfo(router);
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

            if (secure is bool secureValue && secureValue != Secure)
            {
                if (reference == this)
                {
                    reference = (RoutableReference)Clone();
                }
                reference.Secure = secureValue;
            }
            return reference;
        }

        public override bool IsIndirect() => _endpoints.Length == 0;

        public override bool IsWellKnown() => _endpoints.Length == 0 && _adapterId!.Length == 0;

        public override void StreamWrite(OutputStream s)
        {
            base.StreamWrite(s);

            s.WriteSize(_endpoints.Length);
            if (_endpoints.Length > 0)
            {
                Debug.Assert(_adapterId!.Length == 0);
                foreach (Endpoint endpoint in _endpoints)
                {
                    s.WriteShort(endpoint.Type());
                    endpoint.StreamWrite(s);
                }
            }
            else
            {
                s.WriteString(_adapterId ?? ""); // Adapter id.
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
            var s = new StringBuilder();
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
                string a = StringUtil.EscapeString(_adapterId, null, Communicator.ToStringMode);
                if (StringUtil.FindFirstOf(a, " :@") != -1)
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

        public override Dictionary<string, string> ToProperty(string prefix)
        {
            var properties = new Dictionary<string, string>
            {
                [prefix] = ToString(),
                [prefix + ".CollocationOptimized"] = _collocationOptimized ? "1" : "0",
                [prefix + ".ConnectionCached"] = _cacheConnection ? "1" : "0",
                [prefix + ".PreferSecure"] = _preferSecure ? "1" : "0",
                [prefix + ".EndpointSelection"] =
                       _endpointSelection == EndpointSelectionType.Random ? "Random" : "Ordered",
                [prefix + ".LocatorCacheTimeout"] = _locatorCacheTimeout.ToString(CultureInfo.InvariantCulture),
                [prefix + ".InvocationTimeout"] = GetInvocationTimeout().ToString(CultureInfo.InvariantCulture)
            };

            if (_routerInfo != null)
            {
                Dictionary<string, string> routerProperties = _routerInfo.Router.IceReference.ToProperty(prefix + ".Router");
                foreach (KeyValuePair<string, string> entry in routerProperties)
                {
                    properties[entry.Key] = entry.Value;
                }
            }

            if (_locatorInfo != null)
            {
                Dictionary<string, string> locatorProperties = _locatorInfo.Locator.IceReference.ToProperty(prefix + ".Locator");
                foreach (KeyValuePair<string, string> entry in locatorProperties)
                {
                    properties[entry.Key] = entry.Value;
                }
            }

            return properties;
        }

        public override int GetHashCode()
        {
            lock (this)
            {
                if (!HashInitialized)
                {
                    var hash = new HashCode();
                    hash.Add(base.GetHashCode());
                    if (_locatorInfo != null)
                    {
                        hash.Add(_locatorInfo);
                    }
                    if (_routerInfo != null)
                    {
                        hash.Add(_routerInfo);
                    }
                    hash.Add(_collocationOptimized);
                    hash.Add(_collocationOptimized);
                    hash.Add(_preferSecure);
                    hash.Add(_endpointSelection);
                    hash.Add(_locatorCacheTimeout);
                    hash.Add(_overrideTimeout);
                    hash.Add(_connectionId);
                    hash.Add(_adapterId);
                    hash.Add(Collections.GetHashCode(_endpoints));
                    HashValue = hash.ToHashCode();
                }
                return HashValue;
            }
        }

        public override bool Equals(Reference? obj)
        {
            if (ReferenceEquals(this, obj))
            {
                return true;
            }

            if (!(obj is RoutableReference rhs))
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
            internal RouterEndpointsCallback(RoutableReference ir, IGetConnectionCallback cb)
            {
                _ir = ir;
                _cb = cb;
            }

            public void setEndpoints(Endpoint[] endpts)
            {
                if (endpts.Length > 0)
                {
                    _ir.CreateConnection(_ir.ApplyOverrides(endpts), _cb);
                }
                else
                {
                    _ir.GetConnectionNoRouterInfo(_cb);
                }
            }

            public void setException(LocalException ex) => _cb.SetException(ex);

            private readonly RoutableReference _ir;
            private readonly IGetConnectionCallback _cb;
        }

        public override IRequestHandler GetRequestHandler(IObjectPrx proxy) => Communicator.GetRequestHandler(this, proxy);

        public void GetConnection(IGetConnectionCallback callback)
        {
            if (_routerInfo != null)
            {
                //
                // If we route, we send everything to the router's client
                // proxy endpoints.
                //
                _routerInfo.GetClientEndpoints(new RouterEndpointsCallback(this, callback));
            }
            else
            {
                GetConnectionNoRouterInfo(callback);
            }
        }

        private sealed class LocatorEndpointsCallback : LocatorInfo.IGetEndpointsCallback
        {
            internal LocatorEndpointsCallback(RoutableReference ir, IGetConnectionCallback cb)
            {
                _ir = ir;
                _cb = cb;
            }

            public void SetEndpoints(Endpoint[] endpoints, bool cached)
            {
                if (endpoints.Length == 0)
                {
                    _cb.SetException(new NoEndpointException(_ir.ToString()));
                    return;
                }

                _ir.CreateConnection(_ir.ApplyOverrides(endpoints), new ConnectionCallback(_ir, _cb, cached));
            }

            public void SetException(LocalException ex) => _cb.SetException(ex);

            private readonly RoutableReference _ir;
            private readonly IGetConnectionCallback _cb;
        }

        private sealed class ConnectionCallback : IGetConnectionCallback
        {
            internal ConnectionCallback(RoutableReference ir, IGetConnectionCallback cb, bool cached)
            {
                _ir = ir;
                _cb = cb;
                _cached = cached;
            }

            public void SetConnection(Connection connection, bool compress) => _cb.SetConnection(connection, compress);

            public void SetException(LocalException exc)
            {
                try
                {
                    throw exc;
                }
                catch (NoEndpointException ex)
                {
                    _cb.SetException(ex); // No need to retry if there's no endpoints.
                }
                catch (LocalException ex)
                {
                    Debug.Assert(_ir._locatorInfo != null);
                    _ir._locatorInfo.ClearCache(_ir);
                    if (_cached)
                    {
                        TraceLevels traceLevels = _ir.Communicator.TraceLevels;
                        if (traceLevels.Retry >= 2)
                        {
                            _ir.Communicator.Logger.Trace(traceLevels.RetryCat,
                                $"connection to cached endpoints failed\nremoving endpoints from cache and trying again\n{ex}");
                        }
                        _ir.GetConnectionNoRouterInfo(_cb); // Retry.
                        return;
                    }
                    _cb.SetException(ex);
                }
            }

            private readonly RoutableReference _ir;
            private readonly IGetConnectionCallback _cb;
            private readonly bool _cached;
        }

        private void GetConnectionNoRouterInfo(IGetConnectionCallback callback)
        {
            if (_endpoints.Length > 0)
            {
                CreateConnection(_endpoints, callback);
                return;
            }

            if (_locatorInfo != null)
            {
                _locatorInfo.GetEndpoints(this, _locatorCacheTimeout, new LocatorEndpointsCallback(this, callback));
            }
            else
            {
                callback.SetException(new NoEndpointException(ToString()));
            }
        }

        public RoutableReference(Communicator communicator,
                                 Identity identity,
                                 string facet,
                                 InvocationMode mode,
                                 bool secure,
                                 ProtocolVersion protocol,
                                 EncodingVersion encoding,
                                 Endpoint[] endpoints,
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

        protected Endpoint[] ApplyOverrides(Endpoint[] endpts)
        {
            return endpts.Select(endpoint =>
                {
                    endpoint = endpoint.ConnectionId(_connectionId);
                    if (OverrideCompress)
                    {
                        endpoint = endpoint.Compress(Compress);
                    }

                    if (_overrideTimeout)
                    {
                        endpoint.Timeout(_timeout);
                    }
                    return endpoint;
                }).ToArray();
        }

        private Endpoint[] FilterEndpoints(Endpoint[] allEndpoints)
        {
            var endpoints = new List<Endpoint>();

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
            switch (GetMode())
            {
                case InvocationMode.Twoway:
                case InvocationMode.Oneway:
                case InvocationMode.BatchOneway:
                    {
                    //
                    // Filter out datagram endpoints.
                    //
                        var tmp = new List<Endpoint>();
                        foreach (Endpoint endpoint in endpoints)
                        {
                            if (!endpoint.Datagram())
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
                        var tmp = new List<Endpoint>();
                        foreach (Endpoint endpoint in endpoints)
                        {
                            if (endpoint.Datagram())
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
            switch (GetEndpointSelection())
            {
                case EndpointSelectionType.Random:
                    {
                        lock (Rand)
                        {
                            for (int i = 0; i < endpoints.Count - 1; ++i)
                            {
                                int r = Rand.Next(endpoints.Count - i) + i;
                                Debug.Assert(r >= i && r < endpoints.Count);
                                if (r != i)
                                {
                                    Endpoint tmp = endpoints[i];
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
            DefaultsAndOverrides overrides = Communicator.DefaultsAndOverrides;
            if (overrides.OverrideSecure ? overrides.OverrideSecureValue : GetSecure())
            {
                endpoints = endpoints.Where(endpoint => endpoint.Secure()).ToList();
            }
            else if (GetPreferSecure())
            {
                endpoints = endpoints.OrderByDescending(endpoint => endpoint.Secure()).ToList();
            }
            else
            {
                endpoints = endpoints.OrderBy(endpoint => endpoint.Secure()).ToList();
            }

            return endpoints.Select(e => e).ToArray();
        }

        private sealed class CreateConnectionCallback : OutgoingConnectionFactory.ICreateConnectionCallback
        {
            internal CreateConnectionCallback(RoutableReference rr, Endpoint[]? endpoints, IGetConnectionCallback cb)
            {
                _rr = rr;
                _endpoints = endpoints;
                _callback = cb;
            }

            public void SetConnection(Connection connection, bool compress)
            {
                //
                // If we have a router, set the object adapter for this router
                // (if any) to the new connection, so that callbacks from the
                // router can be received over this new connection.
                //
                if (_rr._routerInfo != null && _rr._routerInfo.Adapter != null)
                {
                    connection.SetAdapter(_rr._routerInfo.Adapter);
                }
                _callback.SetConnection(connection, compress);
            }

            public void SetException(LocalException ex)
            {
                if (_exception == null)
                {
                    _exception = ex;
                }

                if (_endpoints == null || ++_i == _endpoints.Length)
                {
                    _callback.SetException(_exception);
                    return;
                }

                bool more = _i != _endpoints.Length - 1;
                var endpoint = new Endpoint[] { _endpoints[_i] };
                _rr.Communicator.OutgoingConnectionFactory().Create(endpoint, more, _rr.GetEndpointSelection(), this);
            }

            private readonly RoutableReference _rr;
            private readonly Endpoint[]? _endpoints;
            private readonly IGetConnectionCallback _callback;
            private int _i = 0;
            private LocalException? _exception = null;
        }

        protected void CreateConnection(Endpoint[] allEndpoints, IGetConnectionCallback callback)
        {
            Endpoint[] endpoints = FilterEndpoints(allEndpoints);
            if (endpoints.Length == 0)
            {
                callback.SetException(new NoEndpointException(ToString()));
                return;
            }

            //
            // Finally, create the connection.
            //
            OutgoingConnectionFactory factory = Communicator.OutgoingConnectionFactory();
            if (GetCacheConnection() || endpoints.Length == 1)
            {
                //
                // Get an existing connection or create one if there's no
                // existing connection to one of the given endpoints.
                //
                factory.Create(endpoints, false, GetEndpointSelection(),
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

                factory.Create(new Endpoint[] { endpoints[0] }, true, GetEndpointSelection(),
                               new CreateConnectionCallback(this, endpoints, callback));
            }
        }

        private class EndpointComparator : IComparer<Endpoint>
        {
            public EndpointComparator(bool preferSecure) => _preferSecure = preferSecure;

            public int Compare(Endpoint le, Endpoint re)
            {
                bool ls = le.Secure();
                bool rs = re.Secure();
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

            private readonly bool _preferSecure;
        }

        private static readonly Endpoint[] _emptyEndpoints = Array.Empty<Endpoint>();

        private Endpoint[] _endpoints;
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
