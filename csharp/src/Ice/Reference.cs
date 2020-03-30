//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using IceUtilInternal;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;

namespace IceInternal
{
    public abstract class Reference : IEquatable<Reference>
    {
        internal interface IGetConnectionCallback
        {
            void SetConnection(Connection connection, bool compress);
            void SetException(System.Exception ex);
        }

        internal static IReadOnlyDictionary<string, string> EmptyContext = new Dictionary<string, string>();

        internal InvocationMode InvocationMode => _invocationMode;
        private protected InvocationMode _invocationMode;

        internal Protocol Protocol => _protocol;
        private protected Protocol _protocol;
        internal Encoding Encoding => _encoding;
        private protected Encoding _encoding;

        internal Identity Identity => _identity;
        private protected Identity _identity;

        internal string Facet => _facet;
        private protected string _facet;

        internal bool IsSecure => _secure;
        private protected bool _secure;

        internal bool? Compress => _compress;
        private protected bool? _compress;

        internal Communicator Communicator { get; }

        internal IReadOnlyDictionary<string, string> Context => _context;
        private protected IReadOnlyDictionary<string, string> _context;

        internal int InvocationTimeout => _invocationTimeout;
        private int _invocationTimeout;

        internal abstract bool IsConnectionCached { get; }

        internal abstract Endpoint[] Endpoints { get; }
        internal abstract string AdapterId { get; }
        internal abstract LocatorInfo? LocatorInfo { get; }
        internal abstract RouterInfo? RouterInfo { get; }
        internal abstract bool IsCollocationOptimized { get; }
        internal abstract bool IsPreferSecure { get; }
        internal abstract EndpointSelectionType EndpointSelection { get; }
        internal abstract int LocatorCacheTimeout { get; }
        internal abstract string ConnectionId { get; }
        internal abstract int? ConnectionTimeout { get; }
        internal abstract ThreadPool ThreadPool { get; }

        internal abstract bool IsIndirect { get; }
        internal abstract bool IsWellKnown { get; }

        public override int GetHashCode()
        {
            // We don't cache the hash code in Reference as Reference is abstract. We cache it in concrete
            // Reference classes.
            var hash = new HashCode();
            hash.Add(InvocationMode);
            hash.Add(IsSecure);
            hash.Add(_identity);
            if (_context != null)
            {
                hash.Add(Collections.GetHashCode(_context));
            }
            hash.Add(_facet);
            if (Compress != null)
            {
                hash.Add(Compress);
            }
            hash.Add(_protocol);
            hash.Add(_encoding);
            hash.Add(_invocationTimeout);
            return hash.ToHashCode();
        }

        public bool GetCompressOverride(out bool compress)
        {
            DefaultsAndOverrides defaultsAndOverrides = Communicator.DefaultsAndOverrides;
            if (defaultsAndOverrides.OverrideCompress)
            {
                compress = defaultsAndOverrides.OverrideCompressValue;
            }
            else if (Compress != null)
            {
                compress = Compress.Value;
            }
            else
            {
                compress = false;
                return false;
            }
            return true;
        }

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

            s.WriteByte((byte)InvocationMode);

            s.WriteBool(IsSecure);

            s.WriteByte((byte)_protocol);
            s.WriteByte(0);
            s.WriteByte(_encoding.Major);
            s.WriteByte(_encoding.Minor);

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
            var s = new System.Text.StringBuilder();

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

            switch (InvocationMode)
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

            if (IsSecure)
            {
                s.Append(" -s");
            }

            s.Append(" -p ");
            if (Communicator.ToStringMode >= ToStringMode.Compat)
            {
                // x.0 style
                s.Append(_protocol.ToString("D"));
                s.Append(".0");
            }
            else
            {
                s.Append(_protocol.ToString().ToLower());
            }

            //
            // Always print the encoding version to ensure a stringified proxy
            // will convert back to a proxy with the same encoding with
            // stringToProxy (and won't use Ice.Default.Encoding).
            //
            s.Append(" -e ");
            s.Append(_encoding.ToString());

            return s.ToString();

            // Derived class writes the remainder of the string.
        }

        public abstract Dictionary<string, string> ToProperty(string prefix);

        internal abstract IRequestHandler GetRequestHandler();
        internal abstract Ice.Connection? GetCachedConnection();

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
            if (InvocationMode != other.InvocationMode)
            {
                return false;
            }

            if (IsSecure != other.IsSecure)
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

            if (Compress != other.Compress)
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

        internal virtual Reference Clone(Identity? identity = null,
                                       string? facet = null,
                                       string? adapterId = null,
                                       bool clearLocator = false,
                                       bool clearRouter = false,
                                       bool? collocationOptimized = null,
                                       bool? compress = null,
                                       bool? connectionCached = null,
                                       string? connectionId = null,
                                       int? connectionTimeout = null,
                                       IReadOnlyDictionary<string, string>? context = null,
                                       Encoding? encoding = null,
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
            // This options must always be handled by one of the derived classes.
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
                reference = CloneIfSame(reference);
                reference._identity = identity.Value;
            }

            if (facet != null && !facet.Equals(_facet))
            {
                reference = CloneIfSame(reference);
                reference._facet = facet;
            }

            if (compress is bool compressValue && (Compress == null|| compressValue != Compress))
            {
                reference = CloneIfSame(reference);
                reference._compress = compressValue;
            }

            if (context != null)
            {
                reference = CloneIfSame(reference);
                reference._context = context.Count == 0 ? EmptyContext : new Dictionary<string, string>(context);
            }

            if (encoding is Encoding encodingValue && encodingValue != _encoding)
            {
                reference = CloneIfSame(reference);
                reference._encoding = encodingValue;
            }

            if (invocationMode is InvocationMode invocationModeValue && invocationModeValue != InvocationMode)
            {
                reference = CloneIfSame(reference);
                reference._invocationMode = invocationModeValue;
            }
            else if (oneway is bool onewayValue && ((onewayValue && reference.InvocationMode != InvocationMode.Oneway) ||
                                                    (!onewayValue && reference.InvocationMode != InvocationMode.Twoway)))
            {
                reference = CloneIfSame(reference);
                reference._invocationMode = onewayValue ? InvocationMode.Oneway : InvocationMode.Twoway;
            }

            if (invocationTimeout is int invocationTimeoutValue && invocationTimeoutValue != _invocationTimeout)
            {
                if (invocationTimeoutValue < 1 && invocationTimeoutValue != -1)
                {
                    throw new ArgumentException($"invalid value passed to ice_invocationTimeout: {invocationTimeoutValue}",
                                                nameof(invocationTimeout));
                }

                reference = CloneIfSame(reference);
                reference._invocationTimeout = invocationTimeoutValue;
            }
            return reference;
        }

        internal virtual Reference Clone() => (Reference)MemberwiseClone();

        // Creates a new clone of this unless `clone` is already a real clone (meaning "not this")
        // Returns the clone.
        private protected T CloneIfSame<T>(T clone) where T : Reference
        {
            if (ReferenceEquals(this, clone))
            {
                clone = (Clone() as T)!;
            }
            return clone;
        }

        protected Reference(Communicator communicator,
                            Identity identity,
                            string facet,
                            InvocationMode mode,
                            bool secure,
                            Protocol protocol,
                            Encoding encoding,
                            int invocationTimeout,
                            IReadOnlyDictionary<string, string> context)
        {
            //
            // Validate string arguments.
            //
            Debug.Assert(identity.Name != null);
            Debug.Assert(identity.Category != null);
            Debug.Assert(facet != null);

            Communicator = communicator;
            _invocationMode = mode;
            _identity = identity;
            _context = context;
            _facet = facet;
            _protocol = protocol;
            _encoding = encoding;
            _invocationTimeout = invocationTimeout;
            _secure = secure;
            _compress = null;
        }

        protected static Random Rand = new Random(unchecked((int)DateTime.Now.Ticks));
    }

    public class FixedReference : Reference
    {
        // _hashCode is a cached hash code initialized lazily to a value other than 0.
        private int _hashCode = 0;
        private Connection _fixedConnection;
        private ConnectionRequestHandler _requestHandler;

        internal override Reference Clone()
        {
            var clone = (FixedReference)base.Clone();
            clone._hashCode = 0;
            return clone;
        }

        internal override bool IsConnectionCached => true;

        internal FixedReference(Communicator communicator,
                                Identity identity,
                                string facet,
                                InvocationMode mode,
                                bool secure,
                                Protocol protocol,
                                Encoding encoding,
                                Connection connection,
                                int invocationTimeout,
                                IReadOnlyDictionary<string, string> context,
                                bool? compress)
            : base(communicator, identity, facet, mode, secure, protocol, encoding, invocationTimeout, context)
        {
            _fixedConnection = connection;

            if (compress is bool compressValue)
            {
                _compress = compressValue;
            }
            _requestHandler = CreateConnectionRequestHandler(); // must be last
        }

        // Creates the ConnectionRequestHandler after various checks
        private ConnectionRequestHandler CreateConnectionRequestHandler()
        {
            if (InvocationMode == InvocationMode.Datagram)
            {
                if (!(_fixedConnection.Endpoint as Endpoint)!.Datagram())
                {
                    throw new ArgumentException("a fixed datagram proxy requires a datagram connection");
                }
            }
            else if (InvocationMode == InvocationMode.BatchOneway || InvocationMode == InvocationMode.BatchDatagram)
            {
                throw new NotSupportedException("batch invocation modes are not supported for fixed proxies");
            }

            // If a secure connection is requested or secure overrides is set, check if the connection is secure.
            bool secure;
            DefaultsAndOverrides defaultsAndOverrides = Communicator.DefaultsAndOverrides;
            if (defaultsAndOverrides.OverrideSecure)
            {
                secure = defaultsAndOverrides.OverrideSecureValue;
            }
            else
            {
                secure = IsSecure;
            }
            if (secure && !((Endpoint)_fixedConnection.Endpoint).Secure())
            {
                throw new ArgumentException("cannot create secure fixed proxy over non-secure connection");
            }

            _fixedConnection.ThrowException(); // Throw in case our connection is already destroyed.

            bool compress = false;
            if (defaultsAndOverrides.OverrideCompress)
            {
                compress = defaultsAndOverrides.OverrideCompressValue;
            }
            else if (Compress != null)
            {
                compress = Compress.Value;
            }
            return new ConnectionRequestHandler(_fixedConnection, compress);
        }

        internal override Endpoint[] Endpoints => Array.Empty<Endpoint>();

        internal override string AdapterId => "";

        internal override LocatorInfo? LocatorInfo => null;

        internal override RouterInfo? RouterInfo => null;

        internal override bool IsCollocationOptimized => false;

        internal override bool IsPreferSecure => false;

        internal override EndpointSelectionType EndpointSelection => EndpointSelectionType.Random;

        internal override int LocatorCacheTimeout => 0;

        internal override string ConnectionId => "";

        internal override int? ConnectionTimeout => null;

        internal override ThreadPool ThreadPool => _fixedConnection.ThreadPool;

        internal override Reference Clone(Identity? identity = null,
                                        string? facet = null,
                                        string? adapterId = null,
                                        bool clearLocator = false,
                                        bool clearRouter = false,
                                        bool? collocationOptimized = null,
                                        bool? compress = null,
                                        bool? connectionCached = null,
                                        string? connectionId = null,
                                        int? connectionTimeout = null,
                                        IReadOnlyDictionary<string, string>? context = null,
                                        Encoding? encoding = null,
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
                throw new ArgumentException($"cannot set both {nameof(locator)} and {nameof(clearLocator)}");
            }

            if (router != null && clearRouter)
            {
                throw new ArgumentException($"cannot set both {nameof(router)} and {nameof(clearRouter)}");
            }

            if (oneway != null && invocationMode != null)
            {
                throw new ArgumentException($"cannot set both {nameof(oneway)} and {nameof(invocationMode)}");
            }

            if (endpoints != null && adapterId != null)
            {
                throw new ArgumentException($"cannot set both {nameof(endpoints)} and {nameof(adapterId)}");
            }

            // Pass options handled by the base class. Note that base.Clone return this if all the options are null.
            var reference = (FixedReference)base.Clone(identity: identity,
                                                       facet: facet,
                                                       compress: compress,
                                                       context: context,
                                                       encoding: encoding,
                                                       invocationMode: invocationMode,
                                                       invocationTimeout: invocationTimeout,
                                                       oneway: oneway);

            if (fixedConnection != null && fixedConnection != _fixedConnection)
            {
                reference = CloneIfSame(reference);
                reference._fixedConnection = fixedConnection;
                reference._secure = fixedConnection.Endpoint.GetInfo().Secure();
                reference._requestHandler = reference.CreateConnectionRequestHandler();
            }

            if (adapterId != null)
            {
                throw new ArgumentException($"cannot change the adapter ID of a fixed proxy", nameof(adapterId));
            }

            if (collocationOptimized != null)
            {
                throw new ArgumentException($"cannot change the collocation optimization configuration of a fixed proxy", nameof(collocationOptimized));
            }

            if (connectionCached != null)
            {
                throw new ArgumentException($"cannot change the connection caching configuration of a fixed proxy", nameof(connectionCached));
            }

            if (connectionId != null)
            {
                throw new ArgumentException($"cannot change the connection ID of a fixed proxy", nameof(connectionId));
            }

            if (connectionTimeout != null)
            {
                throw new ArgumentException($"cannot change the connection timeout of a fixed proxy", nameof(connectionTimeout));
            }

            if (endpointSelectionType != null)
            {
                throw new ArgumentException($"cannot change the endpoint selection policy of a fixed proxy", nameof(endpointSelectionType));
            }

            if (endpoints != null)
            {
                throw new ArgumentException($"cannot change the endpoints of a fixed proxy", nameof(endpoints));
            }

            if (locator != null)
            {
                throw new ArgumentException($"cannot change the locator of a fixed proxy", nameof(locator));
            }
            else if (clearLocator)
            {
                throw new ArgumentException($"cannot change the locator of a fixed proxy", nameof(clearLocator));
            }

            if (locatorCacheTimeout != null)
            {
                throw new ArgumentException($"cannot change the locator cache timeout of a fixed proxy", nameof(locatorCacheTimeout));
            }

            if (preferSecure != null)
            {
                throw new ArgumentException($"cannot change the prefer sercure configuration of a fixed proxy", nameof(preferSecure));
            }

            if (router != null)
            {
                throw new ArgumentException($"cannot change the router of a fixed proxy", nameof(router));
            }
            else if (clearRouter)
            {
                throw new ArgumentException($"cannot change the router of a fixed proxy", nameof(clearRouter));
            }

            if (secure != null)
            {
                throw new ArgumentException($"cannot change the secure configuration of a fixed proxy", nameof(secure));
            }
            return reference;
        }

        internal override bool IsIndirect => false;

        internal override bool IsWellKnown => false;

        public override void StreamWrite(OutputStream s)
            => throw new NotSupportedException("cannot marshal a fixed proxy");

        public override Dictionary<string, string> ToProperty(string prefix)
            => throw new NotSupportedException("cannot convert a fixed proxy to property dictionary");

        internal override IRequestHandler GetRequestHandler() => _requestHandler;

        internal override Ice.Connection? GetCachedConnection() => _requestHandler.GetConnection();

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

        public override int GetHashCode()
        {
            if (_hashCode != 0)
            {
                return _hashCode;
            }
            else
            {
                // lazy initialization, thread-safe because reading/writing _hashcode (an int) is atomic
                int hashCode = HashCode.Combine(base.GetHashCode(), _fixedConnection);
                if (hashCode == 0)
                {
                    hashCode = 1;
                }
                _hashCode = hashCode;
                return _hashCode;
            }
        }
    }

    public class RoutableReference : Reference
    {
        internal override Endpoint[] Endpoints => _endpoints;

        internal override string AdapterId => _adapterId!;

        internal override LocatorInfo? LocatorInfo => _locatorInfo;

        internal override RouterInfo? RouterInfo => _routerInfo;

        internal override bool IsCollocationOptimized => _collocationOptimized;

        internal override bool IsPreferSecure => _preferSecure;

        internal override EndpointSelectionType EndpointSelection => _endpointSelection;

        internal override int LocatorCacheTimeout => _locatorCacheTimeout;

        internal override string ConnectionId => _connectionId;

        internal override int? ConnectionTimeout
        {
            get
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
        }

        internal override ThreadPool ThreadPool => Communicator.ClientThreadPool();

        internal override bool IsIndirect => _endpoints.Length == 0;

        internal override bool IsWellKnown => _endpoints.Length == 0 && _adapterId!.Length == 0;

        internal override Reference Clone(Identity? identity = null,
                                        string? facet = null,
                                        string? adapterId = null,
                                        bool clearLocator = false,
                                        bool clearRouter = false,
                                        bool? collocationOptimized = null,
                                        bool? compress = null,
                                        bool? connectionCached = null,
                                        string? connectionId = null,
                                        int? connectionTimeout = null,
                                        IReadOnlyDictionary<string, string>? context = null,
                                        Encoding? encoding = null,
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
                throw new ArgumentException($"cannot set both {nameof(locator)} and {nameof(clearLocator)}");
            }

            if (router != null && clearRouter)
            {
                throw new ArgumentException($"cannot set both {nameof(router)} and {nameof(clearRouter)}");
            }

            if (oneway != null && invocationMode != null)
            {
                throw new ArgumentException($"cannot set both {nameof(oneway)} and {nameof(invocationMode)}");
            }

            if (endpoints != null && adapterId != null)
            {
                throw new ArgumentException($"cannot set both {nameof(endpoints)} and {nameof(adapterId)}");
            }

            //
            // Pass down options handle by the base class
            //
            var reference = (RoutableReference)base.Clone(identity: identity,
                                                          facet: facet,
                                                          compress: compress,
                                                          context: context,
                                                          encoding: encoding,
                                                          invocationMode: invocationMode,
                                                          invocationTimeout: invocationTimeout,
                                                          oneway: oneway);

            if (fixedConnection != null)
            {
                var fixedReference = new FixedReference(reference.Communicator,
                                                        reference.Identity,
                                                        reference.Facet,
                                                        reference.InvocationMode,
                                                        fixedConnection.Endpoint.GetInfo().Secure(),
                                                        reference.Protocol,
                                                        reference.Encoding,
                                                        fixedConnection,
                                                        reference.InvocationTimeout,
                                                        reference.Context,
                                                        reference.Compress);

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
                reference = CloneIfSame(reference);
                reference._adapterId = adapterId;
                reference._endpoints = Array.Empty<Endpoint>();
            }

            if (collocationOptimized is bool collocationOptimizedValue && collocationOptimizedValue != _collocationOptimized)
            {
                reference = CloneIfSame(reference);
                reference._collocationOptimized = collocationOptimizedValue;
            }

            if (connectionId != null && !connectionId.Equals(_connectionId))
            {
                reference = CloneIfSame(reference);
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

                reference = CloneIfSame(reference);
                reference._timeout = connectionTimeoutValue;
                reference._endpoints = reference._endpoints.Select(e => e.Timeout(connectionTimeoutValue)).ToArray();
                reference._overrideTimeout = true;
            }

            if (encoding is Encoding encodingValue)
            {
                if (_locatorInfo != null && !_locatorInfo.Locator.Encoding.Equals(encodingValue))
                {
                    reference = CloneIfSame(reference);
                    reference._locatorInfo = Communicator.GetLocatorInfo(
                        _locatorInfo.Locator.Clone(encoding: encodingValue));
                }
            }

            if (endpointSelectionType is EndpointSelectionType endpointSelectionTypeValue && endpointSelectionTypeValue != _endpointSelection)
            {
                reference = CloneIfSame(reference);
                reference._endpointSelection = endpointSelectionTypeValue;
            }

            if (endpoints != null && !endpoints.SequenceEqual(_endpoints))
            {
                reference = CloneIfSame(reference);
                reference._adapterId = "";
                reference._endpoints = reference.ApplyOverrides(endpoints.Select(e => (Endpoint)e).ToArray());
            }

            if (locator != null)
            {
                LocatorInfo locatorInfo = Communicator.GetLocatorInfo(locator);
                if (!locatorInfo.Equals(_locatorInfo))
                {
                    reference = CloneIfSame(reference);
                    reference._locatorInfo = locatorInfo;
                }
            }
            else if (clearLocator && _locatorInfo != null)
            {
                reference = CloneIfSame(reference);
                reference._locatorInfo = null;
            }

            if (locatorCacheTimeout is int locatorCacheTimeoutValue && locatorCacheTimeoutValue != _locatorCacheTimeout)
            {
                if (locatorCacheTimeoutValue < -1)
                {
                    throw new ArgumentException($"invalid value passed to ice_locatorCacheTimeout: {locatorCacheTimeoutValue}",
                                                nameof(locatorCacheTimeout));
                }
                reference = CloneIfSame(reference);
                reference._locatorCacheTimeout = locatorCacheTimeoutValue;
            }

            if (preferSecure is bool preferSecureValue && preferSecureValue != _preferSecure)
            {
                reference = CloneIfSame(reference);
                reference._preferSecure = preferSecureValue;
            }

            if (router != null)
            {
                RouterInfo routerInfo = Communicator.GetRouterInfo(router);
                if (!routerInfo.Equals(_routerInfo))
                {
                    reference = CloneIfSame(reference);
                    reference._routerInfo = routerInfo;
                }
            }
            else if (clearRouter && _routerInfo != null)
            {
                reference = CloneIfSame(reference);
                reference._routerInfo = null;
            }

            if (secure is bool secureValue && secureValue != IsSecure)
            {
                reference = CloneIfSame(reference);
                reference._secure = secureValue;
            }

            if (connectionCached is bool connectionCachedValue)
            {
                // We may be changing this option
                if (reference.IsConnectionCached != connectionCachedValue)
                {
                    reference = CloneIfSame(reference);
                    Debug.Assert(!reference.IsConnectionCached);
                    if (connectionCachedValue)
                    {
                        reference._requestHandlerMutex = new object();
                    }
                }
                // else all good already
            }
            else if (IsConnectionCached != reference.IsConnectionCached)
            {
                Debug.Assert(!reference.IsConnectionCached); // it's a freshly cloned reference
                reference._requestHandlerMutex = new object();
            }
            return reference;
        }

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
            var s = new System.Text.StringBuilder();
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
                [prefix + ".ConnectionCached"] = IsConnectionCached ? "1" : "0",
                [prefix + ".PreferSecure"] = _preferSecure ? "1" : "0",
                [prefix + ".EndpointSelection"] =
                       _endpointSelection == EndpointSelectionType.Random ? "Random" : "Ordered",
                [prefix + ".LocatorCacheTimeout"] = _locatorCacheTimeout.ToString(CultureInfo.InvariantCulture),
                [prefix + ".InvocationTimeout"] = InvocationTimeout.ToString(CultureInfo.InvariantCulture)
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
            if (_hashCode != 0)
            {
                // Already computed, return cached value:
                return _hashCode;
            }
            else
            {
                // Lazy initialization of _hashCode to a value other than 0. Reading/writing _hashCode is atomic.
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
                int hashCode = hash.ToHashCode();
                if (hashCode == 0)
                {
                    hashCode = 1;
                }
                _hashCode = hashCode;
                return _hashCode;
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
            if (IsConnectionCached != rhs.IsConnectionCached)
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

            public void setException(System.Exception ex) => _cb.SetException(ex);

            private readonly RoutableReference _ir;
            private readonly IGetConnectionCallback _cb;
        }

        internal void GetConnection(IGetConnectionCallback callback)
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

            public void SetException(System.Exception ex) => _cb.SetException(ex);

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

            public void SetException(System.Exception exc)
            {
                try
                {
                    throw exc;
                }
                catch (NoEndpointException ex)
                {
                    _cb.SetException(ex); // No need to retry if there's no endpoints.
                }
                catch (System.Exception ex)
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

        internal RoutableReference(Communicator communicator,
                                 Identity identity,
                                 string facet,
                                 InvocationMode mode,
                                 bool secure,
                                 Protocol protocol,
                                 Encoding encoding,
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
                                 IReadOnlyDictionary<string, string> context)
        : base(communicator, identity, facet, mode, secure, protocol, encoding, invocationTimeout, context)
        {
            if (cacheConnection)
            {
                _requestHandlerMutex = new object();
            }
            _endpoints = endpoints;
            _adapterId = adapterId;
            _locatorInfo = locatorInfo;
            _routerInfo = routerInfo;
            _collocationOptimized = collocationOptimized;
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

        internal override Reference Clone()
        {
            var clone = (RoutableReference)base.Clone();
            clone._hashCode = 0;
            clone._requestHandler = null;
            clone._requestHandlerMutex = null;
            return clone;
        }

        protected Endpoint[] ApplyOverrides(Endpoint[] endpts)
        {
            return endpts.Select(endpoint =>
                {
                    endpoint = endpoint.ConnectionId(_connectionId);
                    if (Compress != null)
                    {
                        endpoint = endpoint.Compress(Compress.Value);
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
            switch (InvocationMode)
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
            switch (EndpointSelection)
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
            if (overrides.OverrideSecure ? overrides.OverrideSecureValue : IsSecure)
            {
                endpoints = endpoints.Where(endpoint => endpoint.Secure()).ToList();
            }
            else if (IsPreferSecure)
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

            public void SetException(System.Exception ex)
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
                _rr.Communicator.OutgoingConnectionFactory().Create(endpoint, more, _rr.EndpointSelection, this);
            }

            private readonly RoutableReference _rr;
            private readonly Endpoint[]? _endpoints;
            private readonly IGetConnectionCallback _callback;
            private int _i = 0;
            private System.Exception? _exception = null;
        }

        internal void CreateConnection(Endpoint[] allEndpoints, IGetConnectionCallback callback)
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
            if (IsConnectionCached || endpoints.Length == 1)
            {
                //
                // Get an existing connection or create one if there's no
                // existing connection to one of the given endpoints.
                //
                factory.Create(endpoints, false, EndpointSelection,
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

                factory.Create(new Endpoint[] { endpoints[0] }, true, EndpointSelection,
                               new CreateConnectionCallback(this, endpoints, callback));
            }
        }

        internal override IRequestHandler GetRequestHandler()
        {
            if (IsConnectionCached)
            {
                Debug.Assert(_requestHandlerMutex != null);
                lock (_requestHandlerMutex)
                {
                    if (_requestHandler != null)
                    {
                        return _requestHandler;
                    }
                }
            }
            return Communicator.GetRequestHandler(this);
        }

        internal override Ice.Connection? GetCachedConnection()
        {
            if (IsConnectionCached)
            {
                Debug.Assert(_requestHandlerMutex != null);
                IRequestHandler? handler;
                lock (_requestHandlerMutex)
                {
                    handler = _requestHandler;
                }
                try
                {
                    return handler?.GetConnection();
                }
                catch (System.Exception)
                {
                }
            }
            return null;
        }

        internal IRequestHandler SetRequestHandler(IRequestHandler handler)
        {
            if (IsConnectionCached)
            {
                Debug.Assert(_requestHandlerMutex != null);
                lock (_requestHandlerMutex)
                {
                    if (_requestHandler == null)
                    {
                        _requestHandler = handler;
                    }
                    return _requestHandler;
                }
            }
            return handler;
        }

        internal void UpdateRequestHandler(IRequestHandler? previous, IRequestHandler? handler)
        {
            if (IsConnectionCached && previous != null)
            {
                Debug.Assert(_requestHandlerMutex != null);
                lock (_requestHandlerMutex)
                {
                    if (_requestHandler != null && _requestHandler != handler)
                    {
                        //
                        // Update the request handler only if "previous" is the same
                        // as the current request handler. This is called after
                        // connection binding by the connect request handler. We only
                        // replace the request handler if the current handler is the
                        // connect request handler.
                        //
                        _requestHandler = _requestHandler.Update(previous, handler);
                    }
                }
            }
        }

        internal override bool IsConnectionCached => _requestHandlerMutex != null;

        private IRequestHandler? _requestHandler;
        private object? _requestHandlerMutex;

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

        private int _hashCode = 0; // cached hash code. 0 means not initialized.
        private static readonly Endpoint[] _emptyEndpoints = Array.Empty<Endpoint>();

        private Endpoint[] _endpoints;
        private string? _adapterId;
        private LocatorInfo? _locatorInfo; // Null if no locator is used.
        private RouterInfo? _routerInfo; // Null if no router is used.
        private bool _collocationOptimized;
        private bool _preferSecure;
        private EndpointSelectionType _endpointSelection;
        private int _locatorCacheTimeout;
        private bool _overrideTimeout;
        private int _timeout; // Only used if _overrideTimeout == true
        private string _connectionId = "";
    }
}
