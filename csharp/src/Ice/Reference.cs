// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>Reference is an Ice-internal but publicly visible class. Each Ice proxy has a single Reference.
    /// Reference represents the untyped implementation of a proxy. Multiples proxies that point to the same Ice object
    /// and share the same proxy options can share the same Reference object, even if these proxies have different
    /// types.</summary>
    public sealed class Reference : IEquatable<Reference>
    {
        internal string AdapterId => Location.Count == 0 ? "" : Location[0];
        internal Communicator Communicator { get; }
        internal string ConnectionId { get; }
        internal IReadOnlyDictionary<string, string> Context { get; }
        internal Encoding Encoding { get; }
        internal IReadOnlyList<Endpoint> Endpoints { get; }
        internal string Facet { get; }
        internal Identity Identity { get; }

        // For ice1 proxies, all the enumerators are meaningful. For other proxies, only the Twoway and Oneway
        // enumerators are used.
        internal InvocationMode InvocationMode { get; }

        internal TimeSpan InvocationTimeout { get; }
        internal bool IsConnectionCached { get; }
        internal bool IsFixed { get; }
        internal bool IsIndirect => !IsFixed && Endpoints.Count == 0;
        public bool IsOneway => InvocationMode != InvocationMode.Twoway;
        internal bool IsWellKnown => !IsFixed && Endpoints.Count == 0 && Location.Count == 0;
        internal IReadOnlyList<string> Location { get; }

        internal TimeSpan LocatorCacheTimeout { get; }

        internal LocatorInfo? LocatorInfo { get; }

        internal bool PreferNonSecure { get; }
        internal Protocol Protocol { get; }
        internal RouterInfo? RouterInfo { get; }
        private int _hashCode;
        private volatile Connection? _connection; // readonly when IsFixed is true

        /// <summary>The equality operator == returns true if its operands are equal, false otherwise.</summary>
        /// <param name="lhs">The left hand side operand.</param>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>true</c> if the operands are equal, otherwise <c>false</c>.</returns>
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

        /// <summary>The inequality operator != returns true if its operands are not equal, false otherwise.</summary>
        /// <param name="lhs">The left hand side operand.</param>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>true</c> if the operands are not equal, otherwise <c>false</c>.</returns>
        public static bool operator !=(Reference? lhs, Reference? rhs) => !(lhs == rhs);

        /// <summary>Creates a reference from a string and a communicator. This an Ice-internal publicly visible static
        /// method.</summary>
        public static Reference Parse(string s, Communicator communicator, string? propertyPrefix = null)
        {
            string proxyString = s.Trim();
            if (proxyString.Length == 0)
            {
                throw new FormatException("empty string is invalid");
            }

            Encoding encoding;
            IReadOnlyList<Endpoint> endpoints;
            string facet;
            Identity identity;
            InvocationMode invocationMode = InvocationMode.Twoway;
            TimeSpan? invocationTimeout = null;
            IReadOnlyList<string> location;
            Protocol protocol;

            if (UriParser.IsProxyUri(proxyString))
            {
                List<string> path;
                UriParser.ProxyOptions proxyOptions;
                (endpoints, path, proxyOptions, facet) = UriParser.ParseProxy(proxyString, communicator);

                protocol = proxyOptions.Protocol ?? Protocol.Ice2;
                Debug.Assert(protocol != Protocol.Ice1); // the URI parsing rejects ice1

                encoding = proxyOptions.Encoding ?? Encoding.V20;

                switch (path.Count)
                {
                    case 0:
                        // TODO: should we add a default identity "Default" or "Root" or "Main"?
                        throw new FormatException($"missing identity in proxy `{proxyString}'");
                    case 1:
                        identity = new Identity(category: "", name: path[0]);
                        location = ImmutableArray<string>.Empty;
                        break;
                    case 2:
                        identity = new Identity(category: path[0], name: path[1]);
                        location = ImmutableArray<string>.Empty;
                        break;
                    default:
                        identity = new Identity(category: path[^2], name: path[^1]);
                        path.RemoveRange(path.Count - 2, 2);
                        location = path;
                        break;
                }

                if (identity.Name.Length == 0)
                {
                    throw new FormatException($"invalid identity with empty name in proxy `{proxyString}'");
                }
                if (location.Any(segment => segment.Length == 0))
                {
                    throw new FormatException($"invalid location with empty segment in proxy `{proxyString}'");
                }

                invocationTimeout = proxyOptions.InvocationTimeout;
            }
            else
            {
                protocol = Protocol.Ice1;
                string location0;

                (identity, facet, invocationMode, encoding, location0, endpoints) =
                    Ice1Parser.ParseProxy(proxyString, communicator);

                // 0 or 1 segment
                location = location0.Length > 0 ? ImmutableArray.Create(location0) : ImmutableArray<string>.Empty;
            }

            bool? cacheConnection = null;
            IReadOnlyDictionary<string, string>? context = null;
            TimeSpan? locatorCacheTimeout = null;
            LocatorInfo? locatorInfo = null;
            bool? preferNonSecure = null;
            RouterInfo? routerInfo = null;

            // Override the defaults with the proxy properties if a property prefix is defined.
            if (propertyPrefix != null && propertyPrefix.Length > 0)
            {
                // Warn about unknown properties.
                if (communicator.WarnUnknownProperties)
                {
                    communicator.CheckForUnknownProperties(propertyPrefix);
                }

                cacheConnection = communicator.GetPropertyAsBool($"{propertyPrefix}.ConnectionCached");

                string property = $"{propertyPrefix}.Context.";
                context = communicator.GetProperties(forPrefix: property).
                    ToDictionary(e => e.Key.Substring(property.Length), e => e.Value);
                if (context.Count == 0)
                {
                    context = null;
                }

                if (invocationTimeout == null)
                {
                    property = $"{propertyPrefix}.InvocationTimeout";
                    invocationTimeout = communicator.GetPropertyAsTimeSpan(property);
                    if (invocationTimeout == TimeSpan.Zero)
                    {
                        throw new InvalidConfigurationException($"0 is not a valid value for property `{property}'");
                    }
                }

                locatorInfo = communicator.GetLocatorInfo(
                    communicator.GetPropertyAsProxy($"{propertyPrefix}.Locator", ILocatorPrx.Factory));

                locatorCacheTimeout = communicator.GetPropertyAsTimeSpan($"{propertyPrefix}.LocatorCacheTimeout");
                preferNonSecure = communicator.GetPropertyAsBool($"{propertyPrefix}.PreferNonSecure");

                property = $"{propertyPrefix}.Router";
                if (communicator.GetPropertyAsProxy(property, IRouterPrx.Factory) is IRouterPrx router)
                {
                    if (propertyPrefix.EndsWith(".Router", StringComparison.Ordinal))
                    {
                        throw new InvalidConfigurationException(
                            $"`{property}={communicator.GetProperty(property)}': cannot set a router on a router");
                    }
                    routerInfo = communicator.GetRouterInfo(router);
                }
            }

            return new Reference(cacheConnection: cacheConnection ?? true,
                                 communicator: communicator,
                                 connectionId: "",
                                 context: context ?? communicator.DefaultContext,
                                 encoding: encoding,
                                 endpoints: endpoints,
                                 facet: facet,
                                 identity: identity,
                                 invocationMode: invocationMode,
                                 invocationTimeout: invocationTimeout ?? communicator.DefaultInvocationTimeout,
                                 location: location,
                                 locatorCacheTimeout: locatorCacheTimeout ?? communicator.DefaultLocatorCacheTimeout,
                                 locatorInfo:
                                    locatorInfo ?? communicator.GetLocatorInfo(communicator.DefaultLocator),
                                 preferNonSecure: preferNonSecure ?? communicator.DefaultPreferNonSecure,
                                 protocol: protocol,
                                 routerInfo: routerInfo ?? communicator.GetRouterInfo(communicator.DefaultRouter));
        }

        /// <inheritdoc/>
        public override bool Equals(object? obj) => Equals(obj as Reference);

        /// <inheritdoc/>
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
                if (_connection != other._connection)
                {
                    return false;
                }
            }
            else
            {
                // Compare properties specific to routable references
                if (ConnectionId != other.ConnectionId)
                {
                    return false;
                }
                if (!Endpoints.SequenceEqual(other.Endpoints))
                {
                    return false;
                }
                if (IsConnectionCached != other.IsConnectionCached)
                {
                    return false;
                }
                if (!Location.SequenceEqual(other.Location))
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
            if (!Context.DictionaryEqual(other.Context))
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

        /// <inheritdoc/>
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
                hash.Add(Context.GetDictionaryHashCode());
                hash.Add(Encoding);
                hash.Add(Facet);
                hash.Add(Identity);
                hash.Add(InvocationMode);
                hash.Add(InvocationTimeout);
                hash.Add(Protocol);

                if (IsFixed)
                {
                    hash.Add(_connection);
                }
                else
                {
                    hash.Add(ConnectionId);
                    foreach (Endpoint e in Endpoints)
                    {
                        hash.Add(e);
                    }
                    hash.Add(IsConnectionCached);
                    foreach (string s in Location)
                    {
                        hash.Add(s);
                    }
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

        /// <summary>Converts the reference into a string. The format of this string depends on the protocol: for ice1,
        /// this method uses the ice1 format, which can be customized by Communicator.ToStringMode. For ice2 and
        /// greater, this method uses the URI format.</summary>
        public override string ToString()
        {
            if (Protocol == Protocol.Ice1)
            {
                var sb = new StringBuilder();

                // If the encoded identity string contains characters which the reference parser uses as separators,
                // then we enclose the identity string in quotes.
                string id = Identity.ToString(Communicator.ToStringMode);
                if (StringUtil.FindFirstOf(id, " :@") != -1)
                {
                    sb.Append('"');
                    sb.Append(id);
                    sb.Append('"');
                }
                else
                {
                    sb.Append(id);
                }

                if (Facet.Length > 0)
                {
                    // If the encoded facet string contains characters which the reference parser uses as separators,
                    // then we enclose the facet string in quotes.
                    sb.Append(" -f ");
                    string fs = StringUtil.EscapeString(Facet, Communicator.ToStringMode);
                    if (StringUtil.FindFirstOf(fs, " :@") != -1)
                    {
                        sb.Append('"');
                        sb.Append(fs);
                        sb.Append('"');
                    }
                    else
                    {
                        sb.Append(fs);
                    }
                }

                switch (InvocationMode)
                {
                    case InvocationMode.Twoway:
                        sb.Append(" -t");
                        break;
                    case InvocationMode.Oneway:
                        sb.Append(" -o");
                        break;
                    case InvocationMode.BatchOneway:
                        sb.Append(" -O");
                        break;
                    case InvocationMode.Datagram:
                        sb.Append(" -d");
                        break;
                    case InvocationMode.BatchDatagram:
                        sb.Append(" -D");
                        break;
                }

                // Always print the encoding version to ensure a stringified proxy will convert back to a proxy with the
                // same encoding with StringToProxy. (Only needed for backwards compatibility).
                sb.Append(" -e ");
                sb.Append(Encoding.ToString());

                if (Location.Count > 0)
                {
                    Debug.Assert(Location.Count == 1); // at most 1 segment with ice1

                    sb.Append(" @ ");

                    // If the encoded adapter id string contains characters which the reference parser uses as
                    // separators, then we enclose the adapter id string in quotes.
                    string a = StringUtil.EscapeString(Location[0], Communicator.ToStringMode);
                    if (StringUtil.FindFirstOf(a, " :@") != -1)
                    {
                        sb.Append('"');
                        sb.Append(a);
                        sb.Append('"');
                    }
                    else
                    {
                        sb.Append(a);
                    }
                }
                else
                {
                    foreach (Endpoint e in Endpoints)
                    {
                        sb.Append(':');
                        sb.Append(e);
                    }
                }
                return sb.ToString();
            }
            else // >= ice2, use URI format
            {
                string path;
                if (Location.Count > 0)
                {
                    var pathBuilder = new StringBuilder();
                    foreach (string s in Location)
                    {
                        pathBuilder.Append(Uri.EscapeDataString(s));
                        pathBuilder.Append('/');
                    }
                    if (Identity.Category.Length == 0)
                    {
                        pathBuilder.Append('/');
                    }
                    pathBuilder.Append(Identity); // Identity.ToString() escapes the string
                    path = pathBuilder.ToString();
                }
                else
                {
                    path = Identity.ToString();
                }

                var sb = new StringBuilder();
                bool firstOption = true;

                if (Endpoints.Count > 0)
                {
                    // direct proxy using ice+transport scheme
                    Endpoint mainEndpoint = Endpoints[0];
                    sb.AppendEndpoint(mainEndpoint, path);
                    firstOption = !mainEndpoint.HasOptions;
                }
                else
                {
                    sb.Append("ice:");
                    sb.Append(path);
                }

                if (Protocol != Protocol.Ice2) // i.e. > ice2
                {
                    StartQueryOption(sb, ref firstOption);
                    sb.Append("protocol=");
                    sb.Append(Protocol.GetName());
                }

                if (Encoding != Ice2Definitions.Encoding) // possible but quite unlikely
                {
                    StartQueryOption(sb, ref firstOption);
                    sb.Append("encoding=");
                    sb.Append(Encoding);
                }

                StartQueryOption(sb, ref firstOption);
                sb.Append("invocation-timeout=");
                sb.Append(TimeSpanExtensions.ToPropertyString(InvocationTimeout));

                if (Endpoints.Count > 1)
                {
                    Transport mainTransport = Endpoints[0].Transport;
                    StartQueryOption(sb, ref firstOption);
                    sb.Append("alt-endpoint=");
                    for (int i = 1; i < Endpoints.Count; ++i)
                    {
                        if (i > 1)
                        {
                            sb.Append(',');
                        }
                        sb.AppendEndpoint(Endpoints[i], "", mainTransport != Endpoints[i].Transport, '$');
                    }
                }

                if (Facet.Length > 0)
                {
                    sb.Append('#');
                    sb.Append(Uri.EscapeDataString(Facet));
                }

                return sb.ToString();
            }

            static void StartQueryOption(StringBuilder sb, ref bool firstOption)
            {
                if (firstOption)
                {
                    sb.Append('?');
                    firstOption = false;
                }
                else
                {
                    sb.Append('&');
                }
            }
        }

        /// <summary>Reads a reference from the input stream.</summary>
        /// <param name="istr">The input stream to read from.</param>
        /// <returns>The reference read from the stream (can be null).</returns>
        internal static Reference? Read(InputStream istr)
        {
            if (istr.Encoding == Encoding.V11)
            {
                var identity = new Identity(istr);
                if (identity.Name.Length == 0)
                {
                    return null;
                }

                var proxyData = new ProxyData11(istr);

                if (proxyData.FacetPath.Length > 1)
                {
                    throw new InvalidDataException(
                        $"received proxy with {proxyData.FacetPath.Length} elements in its facet path");
                }

                if ((byte)proxyData.Protocol == 0)
                {
                    throw new InvalidDataException("received proxy with protocol set to 0");
                }

                if (proxyData.Protocol != Protocol.Ice1 && proxyData.InvocationMode != InvocationMode.Twoway)
                {
                    throw new InvalidDataException(
                        $"received proxy for protocol {proxyData.Protocol.GetName()} with invocation mode set");
                }

                if (proxyData.ProtocolMinor != 0)
                {
                    throw new InvalidDataException(
                        $"received proxy with invalid protocolMinor value: {proxyData.ProtocolMinor}");
                }

                // The min size for an Endpoint with the 1.1 encoding is: transport (short = 2 bytes) + encapsulation
                // header (6 bytes), for a total of 8 bytes.
                Endpoint[] endpoints =
                    istr.ReadArray(minElementSize: 8, istr => istr.ReadEndpoint(proxyData.Protocol));

                string location0 = endpoints.Length == 0 ? istr.ReadString() : "";

                return new Reference(istr.Communicator!,
                                     proxyData.Encoding,
                                     endpoints,
                                     proxyData.FacetPath.Length == 1 ? proxyData.FacetPath[0] : "",
                                     identity,
                                     proxyData.InvocationMode,
                                     location: location0.Length > 0 ?
                                        ImmutableArray.Create(location0) : ImmutableArray<string>.Empty,
                                     proxyData.Protocol);
            }
            else
            {
                Debug.Assert(istr.Encoding == Encoding.V20);

                ProxyKind proxyKind = istr.ReadProxyKind();
                if (proxyKind == ProxyKind.Null)
                {
                    return null;
                }

                var proxyData = new ProxyData20(istr);

                if (proxyData.Identity.Name.Length == 0)
                {
                    throw new InvalidDataException(
                        $"received non-null proxy with empty identity name");
                }

                Protocol protocol = proxyData.Protocol ?? Protocol.Ice2;

                if (proxyData.InvocationMode != null && protocol != Protocol.Ice1)
                {
                    throw new InvalidDataException(
                        $"received proxy for protocol {protocol.GetName()} with invocation mode set");
                }

                // The min size for an Endpoint with the 2.0 encoding is: transport (short = 2 bytes) + host name
                // (min 2 bytes as it cannot be empty) + port number (ushort, 2 bytes) + options (1 byte for empty
                // sequence), for a total of 7 bytes.
                IReadOnlyList<Endpoint> endpoints = proxyKind == ProxyKind.Direct ?
                    istr.ReadArray(minElementSize: 7, istr => istr.ReadEndpoint(protocol)) :
                    ImmutableArray<Endpoint>.Empty;

                return new Reference(istr.Communicator!,
                                     proxyData.Encoding ?? Encoding.V20,
                                     endpoints,
                                     proxyData.Facet ?? "",
                                     proxyData.Identity,
                                     proxyData.InvocationMode ?? InvocationMode.Twoway,
                                     (IReadOnlyList<string>?)proxyData.Location ?? ImmutableArray<string>.Empty,
                                     protocol);
            }
        }

        // Helper constructor for routable references, not bound to a connection. Uses the communicator's defaults.
        internal Reference(
            Communicator communicator,
            Encoding encoding,
            IReadOnlyList<Endpoint> endpoints, // already a copy provided by Ice
            string facet,
            Identity identity,
            InvocationMode invocationMode,
            IReadOnlyList<string> location, // already a copy provided by Ice
            Protocol protocol)
            : this(cacheConnection: true,
                   communicator: communicator,
                   connectionId: "",
                   context: communicator.DefaultContext,
                   encoding: encoding,
                   endpoints: endpoints,
                   facet: facet,
                   identity: identity,
                   invocationMode: invocationMode,
                   invocationTimeout: communicator.DefaultInvocationTimeout,
                   location: location,
                   locatorCacheTimeout: communicator.DefaultLocatorCacheTimeout,
                   locatorInfo: communicator.GetLocatorInfo(communicator.DefaultLocator),
                   preferNonSecure: communicator.DefaultPreferNonSecure,
                   protocol: protocol,
                   routerInfo: communicator.GetRouterInfo(communicator.DefaultRouter))
        {
        }

        // Helper constructor for fixed references. Uses the communicator's defaults.
        internal Reference(Communicator communicator, Connection fixedConnection, Identity identity)
            : this(communicator: communicator,
                   context: communicator.DefaultContext,
                   encoding: fixedConnection.Protocol.GetEncoding(),
                   facet: "",
                   fixedConnection: fixedConnection,
                   identity: identity,
                   invocationMode: (fixedConnection.Endpoint?.IsDatagram ?? false) ?
                       InvocationMode.Datagram : InvocationMode.Twoway,
                   invocationTimeout: communicator.DefaultInvocationTimeout)
        {
        }

        internal void ClearConnection(Connection connection)
        {
            Debug.Assert(!IsFixed);
            Interlocked.CompareExchange(ref _connection, null, connection);
        }

        internal Reference Clone(
            bool? cacheConnection = null,
            bool clearLocator = false,
            bool clearRouter = false,
            string? connectionId = null,
            IReadOnlyDictionary<string, string>? context = null, // can be provided by app
            Encoding? encoding = null,
            IEnumerable<Endpoint>? endpoints = null, // from app
            string? facet = null,
            Connection? fixedConnection = null,
            Identity? identity = null,
            string? identityAndFacet = null,
            InvocationMode? invocationMode = null,
            TimeSpan? invocationTimeout = null,
            IEnumerable<string>? location = null, // from app
            ILocatorPrx? locator = null,
            TimeSpan? locatorCacheTimeout = null,
            bool? oneway = null,
            bool? preferNonSecure = null,
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
            if (invocationMode != null)
            {
                if (oneway != null)
                {
                    throw new ArgumentException($"cannot set both {nameof(oneway)} and {nameof(invocationMode)}");
                }
                if (Protocol != Protocol.Ice1)
                {
                    // This way, we won't get an invalid invocationMode when protocol > ice1.
                    throw new ArgumentException(
                        $"{nameof(invocationMode)} applies only to ice1 proxies",
                        nameof(invocationMode));
                }
            }

            if (invocationTimeout != null && invocationTimeout.Value == TimeSpan.Zero)
            {
                throw new ArgumentException("0 is not a valid value for invocationTimeout", nameof(invocationTimeout));
            }

            if (identityAndFacet != null && facet != null)
            {
                throw new ArgumentException($"cannot set both {nameof(facet)} and {nameof(identityAndFacet)}");
            }

            if (identityAndFacet != null && identity != null)
            {
                throw new ArgumentException($"cannot set both {nameof(identity)} and {nameof(identityAndFacet)}");
            }

            if (oneway != null)
            {
                invocationMode = oneway.Value ? InvocationMode.Oneway : InvocationMode.Twoway;
            }
            if (identityAndFacet != null)
            {
                (identity, facet) = UriParser.ParseIdentityAndFacet(identityAndFacet);
            }

            if (IsFixed || fixedConnection != null)
            {
                // Note that Clone does not allow to clear the fixedConnection

                // Make sure that all arguments incompatible with fixed references are null
                if (cacheConnection != null)
                {
                    throw new ArgumentException("cannot change the connection caching configuration of a fixed proxy",
                        nameof(cacheConnection));
                }
                if (connectionId != null)
                {
                    throw new ArgumentException("cannot change the connection ID of a fixed proxy",
                        nameof(connectionId));
                }
                if (endpoints != null)
                {
                    throw new ArgumentException("cannot change the endpoints of a fixed proxy", nameof(endpoints));
                }
                if (location != null)
                {
                    throw new ArgumentException("cannot change the location of a fixed proxy", nameof(location));
                }
                if (locator != null)
                {
                    throw new ArgumentException("cannot change the locator of a fixed proxy", nameof(locator));
                }
                else if (clearLocator)
                {
                    throw new ArgumentException("cannot change the locator of a fixed proxy", nameof(clearLocator));
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
                if (router != null)
                {
                    throw new ArgumentException("cannot change the router of a fixed proxy", nameof(router));
                }
                else if (clearRouter)
                {
                    throw new ArgumentException("cannot change the router of a fixed proxy", nameof(clearRouter));
                }

                var clone = new Reference(Communicator,
                                          context?.ToImmutableDictionary() ?? Context,
                                          encoding ?? Encoding,
                                          facet ?? Facet,
                                          (fixedConnection ?? _connection)!,
                                          identity ?? Identity,
                                          invocationMode ?? InvocationMode,
                                          invocationTimeout ?? InvocationTimeout);
                return clone == this ? this : clone;
            }
            else
            {
                // Routable reference
                if (endpoints?.FirstOrDefault(endpoint => endpoint.Protocol != Protocol) is Endpoint endpoint)
                {
                    throw new ArgumentException($"the protocol of endpoint `{endpoint}' is not {Protocol}",
                                                nameof(endpoints));
                }

                if (location != null && location.Any(segment => segment.Length == 0))
                {
                    throw new ArgumentException($"invalid location `{location}' with an empty segment",
                                                nameof(location));
                }

                if (locator != null && clearLocator)
                {
                    throw new ArgumentException($"cannot set both {nameof(locator)} and {nameof(clearLocator)}");
                }
                if (router != null && clearRouter)
                {
                    throw new ArgumentException($"cannot set both {nameof(router)} and {nameof(clearRouter)}");
                }

                if (locatorCacheTimeout != null &&
                    locatorCacheTimeout < TimeSpan.Zero && locatorCacheTimeout != Timeout.InfiniteTimeSpan)
                {
                    throw new ArgumentException(
                        $"invalid {nameof(locatorCacheTimeout)}: {locatorCacheTimeout}", nameof(locatorCacheTimeout));
                }

                IReadOnlyList<Endpoint>? newEndpoints = endpoints?.ToImmutableArray();
                IReadOnlyList<string>? newLocation = location?.ToImmutableArray();

                if (Protocol == Protocol.Ice1)
                {
                    if (newLocation?.Count > 0 && newEndpoints?.Count > 0)
                    {
                        throw new ArgumentException(
                            @$"cannot set both a non-empty {nameof(location)} and a non-empty {
                                nameof(endpoints)} on an ice1 proxy",
                            nameof(location));
                    }

                    if (newLocation?.Count > 0)
                    {
                        if (newLocation.Count > 1)
                        {
                            throw new ArgumentException(
                                $"{nameof(location)} is limited to a single segment for ice1 proxies",
                                nameof(location));
                        }
                        newEndpoints = ImmutableArray<Endpoint>.Empty; // make sure the clone's endpoints are empty
                    }
                    else if (newEndpoints?.Count > 0)
                    {
                        newLocation = ImmutableArray<string>.Empty; // make sure the clone's location is empty
                    }
                }

                LocatorInfo? locatorInfo = LocatorInfo;
                if (locator != null)
                {
                    locatorInfo = Communicator.GetLocatorInfo(locator);
                }
                else if (clearLocator)
                {
                    locatorInfo = null;
                }

                RouterInfo? routerInfo = RouterInfo;
                if (router != null)
                {
                    routerInfo = Communicator.GetRouterInfo(router);
                }
                else if (clearRouter)
                {
                    routerInfo = null;
                }

                var clone = new Reference(cacheConnection ?? IsConnectionCached,
                                          Communicator,
                                          connectionId ?? ConnectionId,
                                          context?.ToImmutableDictionary() ?? Context,
                                          encoding ?? Encoding,
                                          newEndpoints ?? Endpoints,
                                          facet ?? Facet,
                                          identity ?? Identity,
                                          invocationMode ?? InvocationMode,
                                          invocationTimeout ?? InvocationTimeout,
                                          newLocation ?? Location,
                                          locatorCacheTimeout ?? LocatorCacheTimeout,
                                          locatorInfo, // no fallback otherwise breaks clearLocator
                                          preferNonSecure ?? PreferNonSecure,
                                          Protocol,
                                          routerInfo); // no fallback otherwise breaks clearRouter

                return clone == this ? this : clone;
            }
        }

        internal Connection? GetCachedConnection() => _connection;

        internal async ValueTask<Connection> GetConnectionAsync(
            IReadOnlyList<IConnector> excludedConnectors,
            CancellationToken cancel)
        {
            Connection? connection = _connection;

            // If the cached connection is no longer active, clear it and get a new connection.
            if (!IsFixed && connection != null && !connection.IsActive)
            {
                ClearConnection(connection);
                connection = null;
            }

            if (connection == null)
            {
                Debug.Assert(!IsFixed);

                IReadOnlyList<Endpoint> endpoints = ImmutableArray<Endpoint>.Empty;

                // If the invocation mode is not datagram, we first check if the target is colocated and if that's the
                // case we use the colocated endpoint.
                if (InvocationMode != InvocationMode.Datagram &&
                    Communicator.GetColocatedEndpoint(this) is Endpoint colocatedEndpoint)
                {
                    endpoints = ImmutableArray.Create(colocatedEndpoint);
                }

                if (endpoints.Count == 0 && RouterInfo != null)
                {
                    // Get the router client endpoints if a router is configured
                    endpoints = await RouterInfo.GetClientEndpointsAsync(cancel).ConfigureAwait(false);
                }

                bool cached = false;
                if (endpoints.Count == 0)
                {
                    // Get the proxy's endpoint or query the locator to get endpoints
                    if (Endpoints.Count > 0)
                    {
                        endpoints = Endpoints;
                    }
                    else if (LocatorInfo != null)
                    {
                        // TODO: cache and send the new location with requests
                        (endpoints, _, cached) =
                            await LocatorInfo.ResolveIndirectReferenceAsync(this, cancel).ConfigureAwait(false);
                    }
                }

                if (endpoints.Count == 0)
                {
                    throw new NoEndpointException(ToString());
                }

                // Apply overrides and filter endpoints
                IEnumerable<Endpoint> filteredEndpoints = endpoints.Where(endpoint =>
                {
                    // Filter out opaque endpoints
                    if (endpoint is OpaqueEndpoint || endpoint is UniversalEndpoint)
                    {
                        return false;
                    }

                    // Filter out based on InvocationMode and IsDatagram
                    switch (InvocationMode)
                    {
                        case InvocationMode.Twoway:
                        case InvocationMode.Oneway:
                        case InvocationMode.BatchOneway:
                            if (endpoint.IsDatagram)
                            {
                                return false;
                            }
                            break;

                        case InvocationMode.Datagram:
                        case InvocationMode.BatchDatagram:
                            if (!endpoint.IsDatagram)
                            {
                                return false;
                            }
                            break;

                        default:
                            Debug.Assert(false);
                            return false;
                    }

                    // If PreferNonSecure is false, filter out all non-secure endpoints
                    return PreferNonSecure || endpoint.IsSecure;
                });

                if (PreferNonSecure)
                {
                    // It's just a preference: we can fallback to secure endpoints.
                    filteredEndpoints = filteredEndpoints.OrderBy(endpoint => endpoint.IsSecure);
                }

                endpoints = filteredEndpoints.ToArray();
                if (endpoints.Count == 0)
                {
                    throw new NoEndpointException(ToString());
                }

                // Finally, create the connection.
                try
                {
                    OutgoingConnectionFactory factory = Communicator.OutgoingConnectionFactory;
                    if (IsConnectionCached)
                    {
                        // Get an existing connection or create one if there's no existing connection to one of
                        // the given endpoints.
                        connection = await factory.CreateAsync(endpoints,
                                                               false,
                                                               ConnectionId,
                                                               excludedConnectors,
                                                               cancel).ConfigureAwait(false);
                    }
                    else
                    {
                        // Go through the list of endpoints and try to create the connection until it succeeds. This
                        // is different from just calling create() with all the endpoints since this might create a
                        // new connection even if there's an existing connection for one of the endpoints.
                        Endpoint lastEndpoint = endpoints[endpoints.Count - 1];
                        foreach (Endpoint endpoint in endpoints)
                        {
                            try
                            {
                                connection = await factory.CreateAsync(ImmutableArray.Create(endpoint),
                                                                       endpoint != lastEndpoint,
                                                                       ConnectionId,
                                                                       excludedConnectors,
                                                                       cancel).ConfigureAwait(false);
                                break;
                            }
                            catch (Exception)
                            {
                                if (endpoint == lastEndpoint)
                                {
                                    throw;
                                }
                            }
                        }
                    }
                    Debug.Assert(connection != null);

                    if (RouterInfo != null)
                    {
                        await RouterInfo.AddProxyAsync(IObjectPrx.Factory(this)).ConfigureAwait(false);

                        // Set the object adapter for this router (if any) on the new connection, so that callbacks from
                        // the router can be received over this new connection.
                        if (RouterInfo.Adapter != null)
                        {
                            connection.Adapter = RouterInfo.Adapter;
                        }
                    }
                }
                catch (Exception ex)
                {
                    if (LocatorInfo != null && IsIndirect)
                    {
                        LocatorInfo.ClearCache(this);
                    }

                    if (cached)
                    {
                        TraceLevels traceLevels = Communicator.TraceLevels;
                        if (traceLevels.Retry >= 2)
                        {
                            Communicator.Logger.Trace(TraceLevels.RetryCategory,
                                                      "connection to cached endpoints failed\n" +
                                                      $"removing endpoints from cache and trying again\n{ex}");
                        }
                        return await GetConnectionAsync(excludedConnectors, cancel).ConfigureAwait(false);
                    }
                    throw;
                }

                if (IsConnectionCached)
                {
                    _connection = connection;
                }
            }
            return connection;
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
                [$"{prefix}.ConnectionCached"] = IsConnectionCached ? "1" : "0",
                [$"{prefix}.LocatorCacheTimeout"] = LocatorCacheTimeout.ToPropertyString(),
                [$"{prefix}.PreferNonSecure"] = PreferNonSecure ? "1" : "0"
            };

            if (Protocol == Protocol.Ice1)
            {
                // For Ice2 the invocation timeout is included in the URI
                properties[$"{prefix}.InvocationTimeout"] = InvocationTimeout.ToPropertyString();
            }

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

        // Marshal the non-null reference.
        internal void Write(OutputStream ostr)
        {
            if (IsFixed)
            {
                // TODO: should be true only for the 1.1 encoding once we add Fixed support in the 2.0 encoding
                throw new NotSupportedException("cannot marshal a fixed proxy");
            }

            if (ostr.Encoding == Encoding.V11)
            {
                Identity.IceWrite(ostr);
                ostr.WriteProxyData11(Facet, InvocationMode, Protocol, Encoding);
                ostr.WriteSequence(Endpoints, (ostr, endpoint) => ostr.WriteEndpoint(endpoint));

                if (Endpoints.Count == 0)
                {
                    // If Location holds more than 1 segment, the extra segments are not marshaled.
                    ostr.WriteString(Location.Count == 0 ? "" : Location[0]);
                }
            }
            else
            {
                Debug.Assert(ostr.Encoding == Encoding.V20);

                ostr.Write(Endpoints.Count > 0 ? ProxyKind.Direct : ProxyKind.Indirect);
                ostr.WriteProxyData20(Identity, Protocol, Encoding, Location, InvocationMode, Facet);

                if (Endpoints.Count > 0)
                {
                    ostr.WriteSequence(Endpoints, (ostr, endpoint) => ostr.WriteEndpoint(endpoint));
                }
            }
        }

        // Constructor for routable references, not bound to a connection
        private Reference(
            bool cacheConnection,
            Communicator communicator,
            string connectionId,
            IReadOnlyDictionary<string, string> context, // already a copy provided by Ice
            Encoding encoding,
            IReadOnlyList<Endpoint> endpoints, // already a copy provided by Ice
            string facet,
            Identity identity,
            InvocationMode invocationMode,
            TimeSpan invocationTimeout,
            IReadOnlyList<string> location, // already a copy provided by Ice
            TimeSpan locatorCacheTimeout,
            LocatorInfo? locatorInfo,
            bool preferNonSecure,
            Protocol protocol,
            RouterInfo? routerInfo)
        {
            Communicator = communicator;
            ConnectionId = connectionId;
            Context = context;
            Encoding = encoding;
            Endpoints = endpoints;
            Facet = facet;
            Identity = identity;
            InvocationMode = invocationMode;
            InvocationTimeout = invocationTimeout;
            IsConnectionCached = cacheConnection;
            Location = location;
            LocatorCacheTimeout = locatorCacheTimeout;
            LocatorInfo = locatorInfo;
            PreferNonSecure = preferNonSecure;
            Protocol = protocol;
            RouterInfo = routerInfo;

            if (Protocol == Protocol.Ice1)
            {
                Debug.Assert(location.Count <= 1);
                Debug.Assert(location.Count == 0 || endpoints.Count == 0);
            }
            else
            {
                Debug.Assert((byte)InvocationMode <= (byte)InvocationMode.Oneway);
            }

            Debug.Assert(location.Count == 0 || location[0].Length > 0); // first segment cannot be empty
            Debug.Assert(!Endpoints.Any(endpoint => endpoint.Protocol != Protocol));
            Debug.Assert(invocationTimeout != TimeSpan.Zero);
        }

        // Constructor for fixed references.
        private Reference(
            Communicator communicator,
            IReadOnlyDictionary<string, string> context, // already a copy provided by Ice
            Encoding encoding,
            string facet,
            Connection fixedConnection,
            Identity identity,
            InvocationMode invocationMode,
            TimeSpan invocationTimeout)
        {
            Communicator = communicator;
            ConnectionId = "";
            Context = context;
            Encoding = encoding;
            Endpoints = Array.Empty<Endpoint>();
            Facet = facet;
            Identity = identity;
            InvocationMode = invocationMode;
            InvocationTimeout = invocationTimeout;
            IsConnectionCached = false;
            IsFixed = true;
            Location = ImmutableArray<string>.Empty;
            LocatorCacheTimeout = TimeSpan.Zero;
            LocatorInfo = null;
            PreferNonSecure = false;
            Protocol = fixedConnection.Protocol;
            RouterInfo = null;

            _connection = fixedConnection;

            if (Protocol == Protocol.Ice1)
            {
                if (InvocationMode == InvocationMode.Datagram)
                {
                    if (!(_connection.Endpoint as Endpoint)!.IsDatagram)
                    {
                        throw new ArgumentException(
                            "a fixed datagram proxy requires a datagram connection",
                            nameof(fixedConnection));
                    }
                }
                else if (InvocationMode == InvocationMode.BatchOneway || InvocationMode == InvocationMode.BatchDatagram)
                {
                    throw new NotSupportedException("batch invocation modes are not supported for fixed proxies");
                }
            }
            else
            {
                Debug.Assert((byte)InvocationMode <= (byte)InvocationMode.Oneway);
            }
            Debug.Assert(invocationTimeout != TimeSpan.Zero);
        }
    }
}
