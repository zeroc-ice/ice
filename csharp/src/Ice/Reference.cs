//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using IceUtilInternal;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;

namespace Ice
{
    /// <summary>Reference is an Ice-internal but publicly visible class. Each Ice proxy has single get-only property,
    /// IceReference, which holds the reference associated with this proxy. Reference represents the untyped
    /// implementation of a proxy. Multiples proxies that point to the same Ice object and share the same proxy
    /// options can share the same Reference object, even if these proxies have different types.</summary>
    public sealed class Reference : IEquatable<Reference>
    {
        internal interface IGetConnectionCallback
        {
            void SetConnection(Connection connection, bool compress);
            void SetException(System.Exception ex);
        }

        internal static readonly IReadOnlyDictionary<string, string> EmptyContext = new Dictionary<string, string>();

        internal string AdapterId { get; }
        internal Communicator Communicator { get; }
        internal bool? Compress { get; }
        internal string ConnectionId { get; }
        internal int? ConnectionTimeout { get; }
        internal IReadOnlyDictionary<string, string> Context { get; }
        internal Encoding Encoding { get; }
        internal EndpointSelectionType EndpointSelection { get; }
        internal Endpoint[] Endpoints { get; }
        internal string Facet { get; }
        internal Identity Identity { get; }
        internal InvocationMode InvocationMode { get; }
        internal int InvocationTimeout { get; }
        internal bool IsCollocationOptimized { get; }
        internal bool IsConnectionCached => IsFixed || _requestHandlerMutex != null;
        internal bool IsFixed => _fixedConnection != null;
        internal bool IsIndirect => !IsFixed && Endpoints.Length == 0;
        internal bool IsWellKnown => !IsFixed && Endpoints.Length == 0 && AdapterId.Length == 0;
        internal int LocatorCacheTimeout { get; }
        internal LocatorInfo? LocatorInfo { get; }
        internal bool PreferNonSecure { get; }
        internal Protocol Protocol { get; }
        internal RouterInfo? RouterInfo { get; }
        internal ThreadPool ThreadPool => IsFixed ? _fixedConnection!.ThreadPool : Communicator.ClientThreadPool();

        private static readonly Random _rand = new Random(unchecked((int)DateTime.Now.Ticks));
        private readonly Connection? _fixedConnection;
        private int _hashCode = 0;
        private IRequestHandler? _requestHandler; // readonly when IsFixed is true
        private readonly object? _requestHandlerMutex;

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

        public override bool Equals(object? obj) => Equals(obj as Reference);

        public bool Equals(Reference? other)
        {
            if (other == null)
            {
                return false;
            }
            if (ReferenceEquals(this, other))
            {
                return true;
            }

            if (_hashCode != 0 && other._hashCode != 0 && _hashCode != other._hashCode)
            {
                return false;
            }

            if (IsFixed)
            {
                // Compare properties and fields specific to fixed references
                if (_fixedConnection != other._fixedConnection)
                {
                    return false;
                }
            }
            else
            {
                // Compare properties specific to routable references
                if (AdapterId != other.AdapterId)
                {
                    return false;
                }
                if (ConnectionId != other.ConnectionId)
                {
                    return false;
                }
                if (ConnectionTimeout != other.ConnectionTimeout)
                {
                    return false;
                }
                if (EndpointSelection != other.EndpointSelection)
                {
                    return false;
                }
                if (!Collections.Equals(Endpoints, other.Endpoints))
                {
                    return false;
                }
                if (IsCollocationOptimized != other.IsCollocationOptimized)
                {
                    return false;
                }
                if (IsConnectionCached != other.IsConnectionCached)
                {
                    return false;
                }
                if (LocatorCacheTimeout != other.LocatorCacheTimeout)
                {
                    return false;
                }
                if (LocatorInfo != other.LocatorInfo)
                {
                    return false;
                }
                if (PreferNonSecure != other.PreferNonSecure)
                {
                    return false;
                }
                if (RouterInfo != other.RouterInfo)
                {
                    return false;
                }
            }

            // Compare common properties
            if (Compress != other.Compress)
            {
                return false;
            }
            if (!Collections.Equals(Context, other.Context))
            {
                return false;
            }
            if (Encoding != other.Encoding)
            {
                return false;
            }
            if (Facet != other.Facet)
            {
                return false;
            }
            if (Identity != other.Identity)
            {
                return false;
            }
            if (InvocationMode != other.InvocationMode)
            {
                return false;
            }
            if (InvocationTimeout != other.InvocationTimeout)
            {
                return false;
            }
            if (IsFixed != other.IsFixed)
            {
                return false;
            }
            if (Protocol != other.Protocol)
            {
                return false;
            }

            return true;
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

                // common properties
                if (Compress != null)
                {
                    hash.Add(Compress.Value);
                }
                hash.Add(Collections.GetHashCode(Context));
                hash.Add(Encoding);
                hash.Add(Facet);
                hash.Add(Identity);
                hash.Add(InvocationMode);
                hash.Add(Protocol);

                if (IsFixed)
                {
                    hash.Add(_fixedConnection);
                }
                else
                {
                    hash.Add(AdapterId);
                    hash.Add(ConnectionId);
                    hash.Add(ConnectionTimeout);
                    hash.Add(Collections.GetHashCode(Endpoints));
                    hash.Add(IsCollocationOptimized);
                    hash.Add(IsConnectionCached);
                    hash.Add(EndpointSelection);
                    hash.Add(LocatorCacheTimeout);
                    if (LocatorInfo != null)
                    {
                        hash.Add(LocatorInfo);
                    }
                    hash.Add(PreferNonSecure);
                    if (RouterInfo != null)
                    {
                        hash.Add(RouterInfo);
                    }
                }

                int hashCode = hash.ToHashCode();
                if (hashCode == 0)
                {
                    // 0 means uninitialized so we switch to 1
                    hashCode = 1;
                }
                _hashCode = hashCode;
                return _hashCode;
            }
        }

        // Convert the reference to its string form.
        public override string ToString()
        {
            // WARNING: Certain features, such as proxy validation in Glacier2, depend on the format of proxy strings.
            // Changes to ToString() and methods called to generate parts of the reference string could break
            // these features.

            var s = new System.Text.StringBuilder();

            // If the encoded identity string contains characters which the reference parser uses as separators, then
            // we enclose the identity string in quotes.
            string id = Identity.ToString(Communicator.ToStringMode);
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

            if (Facet.Length > 0)
            {
                // If the encoded facet string contains characters which the reference parser uses as separators, then
                // we enclose the facet string in quotes.
                s.Append(" -f ");
                string fs = StringUtil.EscapeString(Facet, "", Communicator.ToStringMode);
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

            s.Append(" -p ");
            if (Communicator.ToStringMode >= ToStringMode.Compat)
            {
                // x.0 style
                s.Append(Protocol.ToString("D"));
                s.Append(".0");
            }
            else
            {
                s.Append(Protocol.ToString().ToLower());
            }

            // Always print the encoding version to ensure a stringified proxy will convert back to a proxy with the
            // same encoding with StringToProxy (and won't use Ice.Default.Encoding).
            s.Append(" -e ");
            s.Append(Encoding.ToString());

            if (Endpoints.Length > 0)
            {
                for (int i = 0; i < Endpoints.Length; i++)
                {
                    string endp = Endpoints[i].ToString();
                    if (endp != null && endp.Length > 0)
                    {
                        s.Append(':');
                        s.Append(endp);
                    }
                }
            }
            else if (AdapterId.Length > 0)
            {
                s.Append(" @ ");

                // If the encoded adapter id string contains characters which the reference parser uses as separators,
                // then we enclose the adapter id string in quotes.
                string a = StringUtil.EscapeString(AdapterId, null, Communicator.ToStringMode);
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

        // Constructor for routable references, not bound to a connection
        internal Reference(string adapterId,
                           bool cacheConnection,
                           bool collocationOptimized,
                           Communicator communicator,
                           bool? compress,
                           string connectionId,
                           int? connectionTimeout,
                           IReadOnlyDictionary<string, string> context, // already a copy provided by Ice
                           Encoding encoding,
                           EndpointSelectionType endpointSelection,
                           Endpoint[] endpoints, // already a copy provided by Ice
                           string facet,
                           Identity identity,
                           InvocationMode invocationMode,
                           int invocationTimeout,
                           int locatorCacheTimeout,
                           LocatorInfo? locatorInfo,
                           bool preferNonSecure,
                           Protocol protocol,
                           RouterInfo? routerInfo)
        {
            AdapterId = adapterId;
            Communicator = communicator;
            Compress = compress;
            ConnectionId = connectionId;
            ConnectionTimeout = connectionTimeout;
            Context = context;
            Encoding = encoding;
            EndpointSelection = endpointSelection;
            Endpoints = endpoints;
            Facet = facet;
            Identity = identity;
            InvocationMode = invocationMode;
            InvocationTimeout = invocationTimeout;
            IsCollocationOptimized = collocationOptimized;
            LocatorCacheTimeout = locatorCacheTimeout;
            LocatorInfo = locatorInfo;
            PreferNonSecure = preferNonSecure;
            Protocol = protocol;
            RouterInfo = routerInfo;

            if (cacheConnection)
            {
                _requestHandlerMutex = new object();
            }
        }

        // Constructor for fixed references
        internal Reference(Communicator communicator,
                           bool? compress,
                           IReadOnlyDictionary<string, string> context, // already a copy provided by Ice
                           Encoding encoding,
                           string facet,
                           Connection fixedConnection,
                           Identity identity,
                           InvocationMode invocationMode,
                           int invocationTimeout)
        {
            AdapterId = "";
            Communicator = communicator;
            Compress = compress;
            ConnectionId = "";
            ConnectionTimeout = null;
            Context = context;
            Encoding = encoding;
            EndpointSelection = EndpointSelectionType.Random;
            Endpoints = Array.Empty<Endpoint>();
            Facet = facet;
            Identity = identity;
            InvocationMode = invocationMode;
            InvocationTimeout = invocationTimeout;
            IsCollocationOptimized = false;
            LocatorCacheTimeout = 0;
            LocatorInfo = null;
            PreferNonSecure = false;
            Protocol = Protocol.Ice1; // it's really the connection's protocol
            RouterInfo = null;

            _fixedConnection = fixedConnection;

            if (InvocationMode == InvocationMode.Datagram)
            {
                if (!(_fixedConnection.Endpoint as Endpoint)!.IsDatagram)
                {
                    throw new ArgumentException("a fixed datagram proxy requires a datagram connection",
                        nameof(fixedConnection));
                }
            }
            else if (InvocationMode == InvocationMode.BatchOneway || InvocationMode == InvocationMode.BatchDatagram)
            {
                throw new NotSupportedException("batch invocation modes are not supported for fixed proxies");
            }

            if (Communicator.DefaultsAndOverrides.OverrideCompress)
            {
                compress = Communicator.DefaultsAndOverrides.OverrideCompressValue;
            }

            _fixedConnection.ThrowException(); // Throw in case our connection is already destroyed.
            _requestHandler = new ConnectionRequestHandler(_fixedConnection, compress ?? false);
        }

        internal Reference Clone(string? adapterId = null,
                                 bool? cacheConnection = null,
                                 bool clearLocator = false,
                                 bool clearRouter = false,
                                 bool? collocationOptimized = null,
                                 bool? compress = null,
                                 string? connectionId = null,
                                 int? connectionTimeout = null,
                                 IReadOnlyDictionary<string, string>? context = null, // can be provided by app
                                 Encoding? encoding = null,
                                 EndpointSelectionType? endpointSelection = null,
                                 Endpoint[]? endpoints = null, // can be provided by app
                                 string? facet = null,
                                 Connection? fixedConnection = null,
                                 Identity? identity = null,
                                 InvocationMode? invocationMode = null,
                                 int? invocationTimeout = null,
                                 ILocatorPrx? locator = null,
                                 int? locatorCacheTimeout = null,
                                 bool? oneway = null,
                                 bool? preferNonSecure = null,
                                 Protocol? protocol = null,
                                 IRouterPrx? router = null)
        {
            // Check for incompatible arguments
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
            if (oneway != null)
            {
                invocationMode = oneway.Value ? InvocationMode.Oneway : InvocationMode.Twoway;
            }
            if (endpoints != null && adapterId != null)
            {
                throw new ArgumentException($"cannot set both {nameof(endpoints)} and {nameof(adapterId)}");
            }

            if (invocationTimeout != null && invocationTimeout.Value < 1 && invocationTimeout.Value != -1)
            {
                throw new ArgumentException($"invalid invocation timeout: {invocationTimeout.Value}",
                    nameof(invocationTimeout));
            }

            if (context != null)
            {
                // We never want to hold onto the context provided by the application.
                if (context.Count == 0)
                {
                    context = EmptyContext;
                }
                else
                {
                    context = new Dictionary<string, string>(context);
                }
            }

            if (IsFixed || fixedConnection != null)
            {
                // Note that Clone does not allow to clear the fixedConnection

                // Make sure that all arguments incompatible with fixed references are null
                if (adapterId != null)
                {
                    throw new ArgumentException("cannot change the adapter ID of a fixed proxy", nameof(adapterId));
                }
                if (cacheConnection != null)
                {
                    throw new ArgumentException("cannot change the connection caching configuration of a fixed proxy",
                        nameof(cacheConnection));
                }
                if (collocationOptimized != null)
                {
                    throw new ArgumentException(
                        "cannot change the collocation optimization configuration of a fixed proxy",
                        nameof(collocationOptimized));
                }
                if (connectionId != null)
                {
                    throw new ArgumentException("cannot change the connection ID of a fixed proxy",
                        nameof(connectionId));
                }
                if (connectionTimeout != null)
                {
                    throw new ArgumentException("cannot change the connection timeout of a fixed proxy",
                        nameof(connectionTimeout));
                }
                if (endpointSelection != null)
                {
                    throw new ArgumentException("cannot change the endpoint selection policy of a fixed proxy",
                        nameof(endpointSelection));
                }
                if (endpoints != null)
                {
                    throw new ArgumentException("cannot change the endpoints of a fixed proxy",
                        nameof(endpoints));
                }
                if (locator != null)
                {
                    throw new ArgumentException("cannot change the locator of a fixed proxy",
                        nameof(locator));
                }
                else if (clearLocator)
                {
                    throw new ArgumentException("cannot change the locator of a fixed proxy",
                        nameof(clearLocator));
                }
                if (locatorCacheTimeout != null)
                {
                    throw new ArgumentException("cannot change the locator cache timeout of a fixed proxy",
                        nameof(locatorCacheTimeout));
                }
                if (preferNonSecure != null)
                {
                    throw new ArgumentException("cannot change the prefer non-secure configuration of a fixed proxy",
                        nameof(preferNonSecure));
                }
                if (protocol != null)
                {
                    throw new ArgumentException("cannot change the protocol of a fixed proxy", nameof(protocol));
                }
                if (router != null)
                {
                    throw new ArgumentException("cannot change the router of a fixed proxy", nameof(router));
                }
                else if (clearRouter)
                {
                    throw new ArgumentException("cannot change the router of a fixed proxy", nameof(clearRouter));
                }

                var clone = new Reference(Communicator,
                                          compress ?? Compress,
                                          context ?? Context,
                                          encoding ?? Encoding,
                                          facet ?? Facet,
                                          (fixedConnection ?? _fixedConnection)!,
                                          identity ?? Identity,
                                          invocationMode ?? InvocationMode,
                                          invocationTimeout ?? InvocationTimeout);
                return clone == this ? this : clone;
            }
            else
            {
                // Routable reference
                if (locator != null && clearLocator)
                {
                    throw new ArgumentException($"cannot set both {nameof(locator)} and {nameof(clearLocator)}");
                }
                if (router != null && clearRouter)
                {
                    throw new ArgumentException($"cannot set both {nameof(router)} and {nameof(clearRouter)}");
                }

                if (connectionTimeout != null && connectionTimeout.Value < 1 && connectionTimeout.Value != -1)
                {
                    throw new ArgumentException($"invalid {nameof(connectionTimeout)}: {connectionTimeout.Value}",
                        nameof(connectionTimeout));
                }
                if (locatorCacheTimeout != null && locatorCacheTimeout.Value < -1)
                {
                    throw new ArgumentException(
                        $"invalid {nameof(locatorCacheTimeout)}: {locatorCacheTimeout.Value}",
                        nameof(locatorCacheTimeout));
                }

                if (adapterId != null && endpoints != null)
                {
                    throw new ArgumentException($"cannot set both {nameof(adapterId)} and {nameof(endpoints)}");
                }
                if (adapterId != null)
                {
                    endpoints = Array.Empty<Endpoint>(); // make sure the clone's endpoints are empty
                }
                else if (endpoints != null)
                {
                    adapterId = ""; // make sure the clone's adapterID is empty
                    endpoints = (Endpoint[])endpoints.Clone(); // make a copy
                }

                var locatorInfo = LocatorInfo;
                if (locator != null)
                {
                    locatorInfo = Communicator.GetLocatorInfo(locator);
                }
                else if (clearLocator)
                {
                    locatorInfo = null;
                }
                if (encoding != null && locatorInfo != null && locatorInfo.Locator.Encoding != encoding)
                {
                    // Need to adjust the encoding used by the locator until we update the locator API
                    locatorInfo = Communicator.GetLocatorInfo(locatorInfo.Locator.Clone(encoding: encoding));
                }

                var routerInfo = RouterInfo;
                if (router != null)
                {
                    routerInfo = Communicator.GetRouterInfo(router);
                }
                else if (clearRouter)
                {
                    routerInfo = null;
                }

                // Update the endpoints if needed
                if (compress != null || connectionId != null || connectionTimeout != null || endpoints != null)
                {
                    endpoints ??= Endpoints;
                    if (endpoints.Length > 0)
                    {
                        compress ??= Compress;
                        connectionId ??= ConnectionId;
                        connectionTimeout ??= ConnectionTimeout;

                        endpoints = endpoints.Select(endpoint =>
                        {
                            if (compress != null)
                            {
                                endpoint = endpoint.NewCompressionFlag(compress.Value);
                            }
                            endpoint = endpoint.NewConnectionId(connectionId);
                            if (connectionTimeout != null)
                            {
                                endpoint = endpoint.NewTimeout(connectionTimeout.Value);
                            }
                            return endpoint;
                        }).ToArray();
                    }
                }

                var clone = new Reference(adapterId ?? AdapterId,
                                      cacheConnection ?? IsConnectionCached,
                                      collocationOptimized ?? IsCollocationOptimized,
                                      Communicator,
                                      compress ?? Compress,
                                      connectionId ?? ConnectionId,
                                      connectionTimeout ?? ConnectionTimeout,
                                      context ?? Context,
                                      encoding ?? Encoding,
                                      endpointSelection ?? EndpointSelection,
                                      endpoints ?? Endpoints,
                                      facet ?? Facet,
                                      identity ?? Identity,
                                      invocationMode ?? InvocationMode,
                                      invocationTimeout ?? InvocationTimeout,
                                      locatorCacheTimeout ?? LocatorCacheTimeout,
                                      locatorInfo, // no fallback otherwise breaks clearLocator
                                      preferNonSecure ?? PreferNonSecure,
                                      protocol ?? Protocol,
                                      routerInfo); // no fallback otherwise breaks clearRouter

                return clone == this ? this : clone;
            }
        }

        // Marshal the reference.
        internal void Write(OutputStream ostr)
        {
            if (IsFixed)
            {
                throw new NotSupportedException("cannot marshal a fixed proxy");
            }

            Identity.IceWrite(ostr);

            // For compatibility with the old FacetPath
            if (Facet.Length == 0)
            {
                ostr.WriteStringSeq(Array.Empty<string>());
            }
            else
            {
                ostr.WriteStringSeq(new string[] { Facet });
            }

            ostr.WriteByte((byte)InvocationMode);
            ostr.WriteBool(false); // secure option, always false (not used)
            ostr.WriteByte((byte)Protocol);
            ostr.WriteByte(0);
            ostr.WriteByte(Encoding.Major);
            ostr.WriteByte(Encoding.Minor);

            ostr.WriteSize(Endpoints.Length);
            if (Endpoints.Length > 0)
            {
                Debug.Assert(AdapterId.Length == 0);
                foreach (Endpoint endpoint in Endpoints)
                {
                    ostr.WriteShort((short)endpoint.Type);
                    endpoint.IceWrite(ostr);
                }
            }
            else
            {
                ostr.WriteString(AdapterId);
            }
        }

        internal Dictionary<string, string> ToProperty(string prefix)
        {
            if (IsFixed)
            {
                throw new NotSupportedException("cannot convert a fixed proxy to a property dictionary");
            }

            var properties = new Dictionary<string, string>
            {
                [prefix] = ToString(),
                [prefix + ".CollocationOptimized"] = IsCollocationOptimized ? "1" : "0",
                [prefix + ".ConnectionCached"] = IsConnectionCached ? "1" : "0",
                [prefix + ".EndpointSelection"] = EndpointSelection.ToString(),
                [prefix + ".InvocationTimeout"] = InvocationTimeout.ToString(CultureInfo.InvariantCulture),
                [prefix + ".LocatorCacheTimeout"] = LocatorCacheTimeout.ToString(CultureInfo.InvariantCulture),
                [prefix + ".PreferNonSecure"] = PreferNonSecure ? "1" : "0"
            };

            if (RouterInfo != null)
            {
                Dictionary<string, string> routerProperties =
                    RouterInfo.Router.ToProperty(prefix + ".Router");
                foreach (KeyValuePair<string, string> entry in routerProperties)
                {
                    properties[entry.Key] = entry.Value;
                }
            }
            if (LocatorInfo != null)
            {
                Dictionary<string, string> locatorProperties =
                    LocatorInfo.Locator.ToProperty(prefix + ".Locator");
                foreach (KeyValuePair<string, string> entry in locatorProperties)
                {
                    properties[entry.Key] = entry.Value;
                }
            }

            return properties;
        }

        internal IRequestHandler GetRequestHandler()
        {
            if (IsFixed)
            {
                return _requestHandler!;
            }
            else
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
        }

        internal Ice.Connection? GetCachedConnection()
        {
            if (IsFixed)
            {
                return _requestHandler!.GetConnection();
            }
            else
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
        }

        // The remaining methods are used only for routable references.

        internal void GetConnection(IGetConnectionCallback callback)
        {
            Debug.Assert(!IsFixed);
            if (RouterInfo != null)
            {
                //
                // If we route, we send everything to the router's client
                // proxy endpoints.
                //
                RouterInfo.GetClientEndpoints(new RouterEndpointsCallback(this, callback));
            }
            else
            {
                GetConnectionNoRouterInfo(callback);
            }
        }

        internal void CreateConnection(Endpoint[] allEndpoints, IGetConnectionCallback callback)
        {
            Debug.Assert(!IsFixed);
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

        internal IRequestHandler SetRequestHandler(IRequestHandler handler)
        {
            Debug.Assert(!IsFixed);
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
            Debug.Assert(!IsFixed);
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

        private void GetConnectionNoRouterInfo(IGetConnectionCallback callback)
        {
            Debug.Assert(!IsFixed);
            if (Endpoints.Length > 0)
            {
                CreateConnection(Endpoints, callback);
                return;
            }

            if (LocatorInfo != null)
            {
                LocatorInfo.GetEndpoints(this, LocatorCacheTimeout, new LocatorEndpointsCallback(this, callback));
            }
            else
            {
                callback.SetException(new NoEndpointException(ToString()));
            }
        }

        private Endpoint[] ApplyOverrides(Endpoint[] endpts)
        {
            Debug.Assert(!IsFixed);
            return endpts.Select(endpoint =>
                {
                    endpoint = endpoint.NewConnectionId(ConnectionId);
                    if (Compress != null)
                    {
                        endpoint = endpoint.NewCompressionFlag(Compress.Value);
                    }
                    if (ConnectionTimeout != null)
                    {
                        endpoint = endpoint.NewTimeout(ConnectionTimeout.Value);
                    }
                    return endpoint;
                }).ToArray();
        }

        private Endpoint[] FilterEndpoints(Endpoint[] allEndpoints)
        {
            Debug.Assert(!IsFixed);
            var endpoints = new List<Endpoint>();

            //
            // Filter out unknown endpoints.
            //
            for (int i = 0; i < allEndpoints.Length; i++)
            {
                if (!(allEndpoints[i] is OpaqueEndpoint))
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
                            if (!endpoint.IsDatagram)
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
                            if (endpoint.IsDatagram)
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
                        lock (_rand)
                        {
                            for (int i = 0; i < endpoints.Count - 1; ++i)
                            {
                                int r = _rand.Next(endpoints.Count - i) + i;
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

            if (PreferNonSecure)
            {
                // It's just a preference: we can fallback to secure endpoints.
                endpoints = endpoints.OrderBy(endpoint => endpoint.IsSecure).ToList();
            }
            else
            {
                // Filter-out non-secure endpoints. This can eliminate all endpoints.
                endpoints = endpoints.Where(endpoint => endpoint.IsSecure).ToList();
            }

            return endpoints.ToArray();
        }

        // TODO: refactor this class
        private sealed class RouterEndpointsCallback : RouterInfo.GetClientEndpointsCallback
        {
            internal RouterEndpointsCallback(Reference ir, IGetConnectionCallback cb)
            {
                Debug.Assert(!ir.IsFixed);
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

            private readonly Reference _ir;
            private readonly IGetConnectionCallback _cb;
        }

        // TODO: refactor this class
        private sealed class LocatorEndpointsCallback : LocatorInfo.IGetEndpointsCallback
        {
            internal LocatorEndpointsCallback(Reference ir, IGetConnectionCallback cb)
            {
                Debug.Assert(!ir.IsFixed);
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

            private readonly Reference _ir;
            private readonly IGetConnectionCallback _cb;
        }

         // TODO: refactor this class
        private sealed class ConnectionCallback : IGetConnectionCallback
        {
            internal ConnectionCallback(Reference ir, IGetConnectionCallback cb, bool cached)
            {
                Debug.Assert(!ir.IsFixed);
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
                    Debug.Assert(_ir.LocatorInfo != null);
                    _ir.LocatorInfo.ClearCache(_ir);
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

            private readonly Reference _ir;
            private readonly IGetConnectionCallback _cb;
            private readonly bool _cached;
        }

        // TODO: refactor this class
        private sealed class CreateConnectionCallback : OutgoingConnectionFactory.ICreateConnectionCallback
        {
            internal CreateConnectionCallback(Reference rr, Endpoint[]? endpoints, IGetConnectionCallback cb)
            {
                Debug.Assert(!rr.IsFixed);
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
                if (_rr.RouterInfo != null && _rr.RouterInfo.Adapter != null)
                {
                    connection.SetAdapter(_rr.RouterInfo.Adapter);
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

            private readonly Reference _rr;
            private readonly Endpoint[]? _endpoints;
            private readonly IGetConnectionCallback _callback;
            private int _i = 0;
            private System.Exception? _exception = null;
        }
    }
}
