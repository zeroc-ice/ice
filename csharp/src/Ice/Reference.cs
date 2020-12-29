// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    /// <summary>Reference is an Ice-internal but publicly visible class. Each Ice proxy has a single Reference.
    /// Reference represents the untyped implementation of a proxy. Multiples proxies that point to the same Ice object
    /// and share the same proxy options can share the same Reference object, even if these proxies have different
    /// types.</summary>
    public sealed class Reference : IEquatable<Reference>
    {
        internal string AdapterId => Location.Count == 0 ? "" : Location[0];
        internal bool CacheConnection { get; } = true;
        internal Communicator Communicator { get; }
        internal IReadOnlyDictionary<string, string> Context { get; }
        internal Encoding Encoding { get; }
        internal IReadOnlyList<Endpoint> Endpoints { get; }
        internal string Facet { get; }
        internal Identity Identity { get; }

        internal IReadOnlyList<InvocationInterceptor> InvocationInterceptors { get; }

        // For ice1 proxies, all the enumerators are meaningful. For other proxies, only the Twoway and Oneway
        // enumerators are used.
        internal InvocationMode InvocationMode { get; }

        internal TimeSpan InvocationTimeout => _invocationTimeout ?? Communicator.DefaultInvocationTimeout;
        internal bool IsFixed { get; }
        internal bool IsIndirect => Endpoints.Count == 0 && !IsFixed;

        internal bool IsOneway => InvocationMode != InvocationMode.Twoway;

        internal bool IsRelative { get; }

        internal bool IsWellKnown => IsIndirect && Location.Count == 0;
        internal object? Label { get; }
        internal IReadOnlyList<string> Location { get; }

        internal TimeSpan LocatorCacheTimeout => _locatorCacheTimeout ?? Communicator.DefaultLocatorCacheTimeout;

        internal LocatorInfo? LocatorInfo { get; }
        internal bool PreferExistingConnection =>
            _preferExistingConnection ?? Communicator.DefaultPreferExistingConnection;
        internal NonSecure PreferNonSecure => _preferNonSecure ?? Communicator.DefaultPreferNonSecure;
        internal Protocol Protocol { get; }

        internal RouterInfo? RouterInfo { get; }

        // Sub-properties for ice1 proxies
        private static readonly string[] _suffixes =
        {
            "CacheConnection",
            "InvocationTimeout",
            "LocatorCacheTimeout",
            "Locator",
            "PreferNonSecure",
            "Relative",
            "Router",
            "Context\\..*"
        };

        private int _hashCode;

        private volatile Connection? _connection; // readonly when IsFixed is true
        private readonly TimeSpan? _invocationTimeout;
        private readonly TimeSpan? _locatorCacheTimeout;
        private readonly bool? _preferExistingConnection;
        private readonly NonSecure? _preferNonSecure;

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

            bool? cacheConnection = null;
            IReadOnlyDictionary<string, string>? context = null;
            Encoding encoding;
            IReadOnlyList<Endpoint> endpoints;
            string facet;
            Identity identity;
            InvocationMode invocationMode = InvocationMode.Twoway;
            TimeSpan? invocationTimeout = null;
            object? label = null;
            IReadOnlyList<string> location;
            bool? preferExistingConnection = null;
            NonSecure? preferNonSecure = null;
            TimeSpan? locatorCacheTimeout = null;
            LocatorInfo? locatorInfo = null;
            Protocol protocol;
            bool? relative = null;
            RouterInfo? routerInfo = null;

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

                (cacheConnection,
                 context,
                 invocationTimeout,
                 label,
                 locatorCacheTimeout,
                 preferExistingConnection,
                 preferNonSecure,
                 relative) = proxyOptions;

                if (locatorCacheTimeout != null && communicator.DefaultLocator == null)
                {
                    throw new FormatException("cannot set locator-cache-timeout without a Locator");
                }
            }
            else
            {
                protocol = Protocol.Ice1;
                string location0;

                (identity, facet, invocationMode, encoding, location0, endpoints) =
                    Ice1Parser.ParseProxy(proxyString, communicator);

                // 0 or 1 segment
                location = location0.Length > 0 ? ImmutableArray.Create(location0) : ImmutableArray<string>.Empty;

                // Override the defaults with the proxy properties if a property prefix is defined.
                if (propertyPrefix != null && propertyPrefix.Length > 0)
                {
                    // Warn about unknown properties.
                    if (communicator.WarnUnknownProperties)
                    {
                        CheckForUnknownProperties(propertyPrefix, communicator);
                    }

                    cacheConnection = communicator.GetPropertyAsBool($"{propertyPrefix}.CacheConnection");

                    string property = $"{propertyPrefix}.Context.";
                    context = communicator.GetProperties(forPrefix: property).
                        ToImmutableDictionary(e => e.Key.Substring(property.Length), e => e.Value);

                    property = $"{propertyPrefix}.InvocationTimeout";
                    invocationTimeout = communicator.GetPropertyAsTimeSpan(property);
                    if (invocationTimeout == TimeSpan.Zero)
                    {
                        throw new InvalidConfigurationException($"{property}: 0 is not a valid value");
                    }

                    label = communicator.GetProperty($"{propertyPrefix}.Label");

                    property = $"{propertyPrefix}.Locator";
                    locatorInfo = communicator.GetLocatorInfo(
                        communicator.GetPropertyAsProxy(property, ILocatorPrx.Factory));

                    if (locatorInfo != null && endpoints.Count > 0)
                    {
                        throw new InvalidConfigurationException($"{property}: cannot set a locator on a direct proxy");
                    }

                    property = $"{propertyPrefix}.LocatorCacheTimeout";
                    locatorCacheTimeout = communicator.GetPropertyAsTimeSpan(property);

                    if (locatorCacheTimeout != null)
                    {
                        if (endpoints.Count > 0)
                        {
                            throw new InvalidConfigurationException($"{property}: proxy has endpoints");
                        }
                        if (locatorInfo == null && communicator.DefaultLocator == null)
                        {
                            throw new InvalidConfigurationException(
                                $"{property}: cannot set locator cache timeout without a Locator");
                        }
                    }

                    preferNonSecure = communicator.GetPropertyAsEnum<NonSecure>($"{propertyPrefix}.PreferNonSecure");
                    relative = communicator.GetPropertyAsBool($"{propertyPrefix}.Relative");

                    if (relative == true && endpoints.Count > 0)
                    {
                        throw new InvalidConfigurationException($"{property}: a direct proxy cannot be relative");
                    }

                    property = $"{propertyPrefix}.Router";
                    if (communicator.GetPropertyAsProxy(property, IRouterPrx.Factory) is IRouterPrx router)
                    {
                        if (protocol != Protocol.Ice1)
                        {
                            throw new InvalidConfigurationException(
                                $"{property}: only an ice1 proxy can have a router");
                        }
                        if (router.Protocol != Protocol.Ice1)
                        {
                            throw new InvalidConfigurationException(@$"{property}={communicator.GetProperty(property)
                                }: a router proxy must use the ice1 protocol");
                        }
                        if (propertyPrefix.EndsWith(".Router", StringComparison.Ordinal))
                        {
                            throw new InvalidConfigurationException($"{property}: cannot set a router on a router");
                        }
                        routerInfo = communicator.GetRouterInfo(router);
                    }
                }
            }

            return new Reference(cacheConnection: cacheConnection ?? true,
                                 communicator: communicator,
                                 context: context ?? communicator.DefaultContext,
                                 encoding: encoding,
                                 endpoints: endpoints,
                                 facet: facet,
                                 identity: identity,
                                 invocationInterceptors: communicator.DefaultInvocationInterceptors,
                                 invocationMode: invocationMode,
                                 invocationTimeout: invocationTimeout,
                                 label: null,
                                 location: location,
                                 locatorCacheTimeout: locatorCacheTimeout,
                                 locatorInfo: locatorInfo ??
                                    (endpoints.Count == 0 ?
                                        communicator.GetLocatorInfo(communicator.DefaultLocator) : null),
                                 preferExistingConnection: preferExistingConnection,
                                 preferNonSecure: preferNonSecure,
                                 protocol: protocol,
                                 relative: relative ?? false,
                                 routerInfo: protocol == Protocol.Ice1 ?
                                    routerInfo ?? communicator.GetRouterInfo(communicator.DefaultRouter) : null);
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
                // Compare properties specific to other kinds of references
                if (CacheConnection != other.CacheConnection)
                {
                    return false;
                }
                if (!Endpoints.SequenceEqual(other.Endpoints))
                {
                    return false;
                }
                if (Label != other.Label)
                {
                    return false;
                }
                if (!Location.SequenceEqual(other.Location))
                {
                    return false;
                }
                if (_locatorCacheTimeout != other._locatorCacheTimeout)
                {
                    return false;
                }
                if (LocatorInfo != other.LocatorInfo)
                {
                    return false;
                }
                if (_preferExistingConnection != other._preferExistingConnection)
                {
                    return false;
                }
                if (_preferNonSecure != other._preferNonSecure)
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
            if (!InvocationInterceptors.SequenceEqual(other.InvocationInterceptors))
            {
                return false;
            }
            if (InvocationMode != other.InvocationMode)
            {
                return false;
            }
            if (_invocationTimeout != other._invocationTimeout)
            {
                return false;
            }
            if (IsFixed != other.IsFixed)
            {
                return false;
            }
            if (IsRelative != other.IsRelative)
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
                hash.Add(InvocationInterceptors.GetSequenceHashCode());
                hash.Add(InvocationMode);
                hash.Add(_invocationTimeout);
                hash.Add(IsFixed);
                hash.Add(IsRelative);
                hash.Add(Protocol);

                if (IsFixed)
                {
                    hash.Add(_connection);
                }
                else
                {
                    hash.Add(CacheConnection);
                    hash.Add(Endpoints.GetSequenceHashCode());
                    hash.Add(Label);
                    hash.Add(Location.GetSequenceHashCode());
                    hash.Add(_locatorCacheTimeout);
                    hash.Add(LocatorInfo);
                    hash.Add(_preferExistingConnection);
                    hash.Add(_preferNonSecure);
                    hash.Add(RouterInfo);
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

                if (!CacheConnection)
                {
                    StartQueryOption(sb, ref firstOption);
                    sb.Append("cache-connection=false");
                }

                if (Context.Count > 0)
                {
                    StartQueryOption(sb, ref firstOption);
                    sb.Append("context=");
                    int index = 0;
                    foreach ((string key, string value) in Context)
                    {
                        sb.Append(Uri.EscapeDataString(key));
                        sb.Append('=');
                        sb.Append(Uri.EscapeDataString(value));
                        if (++index != Context.Count)
                        {
                            sb.Append(',');
                        }
                    }
                }

                if (Encoding != Ice2Definitions.Encoding) // possible but quite unlikely
                {
                    StartQueryOption(sb, ref firstOption);
                    sb.Append("encoding=");
                    sb.Append(Encoding);
                }

                if (IsFixed)
                {
                    StartQueryOption(sb, ref firstOption);
                    sb.Append("fixed=true");
                }

                if (_invocationTimeout is TimeSpan invocationTimeout)
                {
                    StartQueryOption(sb, ref firstOption);
                    sb.Append("invocation-timeout=");
                    sb.Append(TimeSpanExtensions.ToPropertyValue(invocationTimeout));
                }

                if (Label?.ToString() is string label && label.Length > 0)
                {
                    StartQueryOption(sb, ref firstOption);
                    sb.Append("label=");
                    sb.Append(Uri.EscapeDataString(label));
                }

                if (_locatorCacheTimeout is TimeSpan locatorCacheTimeout)
                {
                    StartQueryOption(sb, ref firstOption);
                    sb.Append("locator-cache-timeout=");
                    sb.Append(TimeSpanExtensions.ToPropertyValue(locatorCacheTimeout));
                }

                if (_preferExistingConnection is bool preferExistingConnection)
                {
                    StartQueryOption(sb, ref firstOption);
                    sb.Append("prefer-existing-connection=");
                    sb.Append(preferExistingConnection ? "true" : "false");
                }

                if (_preferNonSecure is NonSecure preferNonSecure)
                {
                    StartQueryOption(sb, ref firstOption);
                    sb.Append("prefer-non-secure=");
                    sb.Append(preferNonSecure.ToString().ToLowerInvariant());
                }

                if (Protocol != Protocol.Ice2) // i.e. > ice2
                {
                    StartQueryOption(sb, ref firstOption);
                    sb.Append("protocol=");
                    sb.Append(Protocol.GetName());
                }

                if (IsRelative)
                {
                    StartQueryOption(sb, ref firstOption);
                    sb.Append("relative=true");
                }

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

        internal static Task<IncomingResponseFrame> InvokeAsync(
            IObjectPrx proxy,
            OutgoingRequestFrame request,
            bool oneway,
            IProgress<bool>? progress = null)
        {
            IReadOnlyList<InvocationInterceptor> invocationInterceptors = proxy.IceReference.InvocationInterceptors;

            switch (proxy.InvocationMode)
            {
                case InvocationMode.BatchOneway:
                case InvocationMode.BatchDatagram:
                    Debug.Assert(false); // not implemented
                    return default;
                case InvocationMode.Datagram when !oneway:
                    throw new InvalidOperationException("cannot make two-way call on a datagram proxy");
                default:
                    return InvokeWithInterceptorsAsync(proxy,
                                                       request,
                                                       oneway,
                                                       0,
                                                       progress,
                                                       request.CancellationToken);
            }

            async Task<IncomingResponseFrame> InvokeWithInterceptorsAsync(
                IObjectPrx proxy,
                OutgoingRequestFrame request,
                bool oneway,
                int i,
                IProgress<bool>? progress,
                CancellationToken cancel)
            {
                cancel.ThrowIfCancellationRequested();

                if (i < invocationInterceptors.Count)
                {
                    // Call the next interceptor in the chain
                    return await invocationInterceptors[i](
                        proxy,
                        request,
                        (target, request, cancel) =>
                            InvokeWithInterceptorsAsync(target, request, oneway, i + 1, progress, cancel),
                        cancel).ConfigureAwait(false);
                }
                else
                {
                    // After we went down the interceptor chain make the invocation.
                    Reference reference = proxy.IceReference;
                    Communicator communicator = reference.Communicator;
                    // If the request size is greater than Ice.RetryRequestSizeMax or the size of the request
                    // would increase the buffer retry size beyond Ice.RetryBufferSizeMax we release the request
                    // after it was sent to avoid holding too much memory and we wont retry in case of a failure.

                    // TODO: this "request size" is now just the payload size. Should we rename the property to
                    // RetryRequestPayloadMaxSize?

                    int requestSize = request.PayloadSize;
                    bool releaseRequestAfterSent =
                        requestSize > communicator.RetryRequestMaxSize ||
                        !communicator.IncRetryBufferSize(requestSize);

                    IInvocationObserver? observer = communicator.Observer?.GetInvocationObserver(proxy,
                                                                                                 request.Operation,
                                                                                                 request.Context);
                    observer?.Attach();
                    try
                    {
                        return await reference.PerformInvokeAsync(request,
                                                                  oneway,
                                                                  progress,
                                                                  releaseRequestAfterSent,
                                                                  observer,
                                                                  cancel).ConfigureAwait(false);
                    }
                    finally
                    {
                        if (!releaseRequestAfterSent)
                        {
                            communicator.DecRetryBufferSize(requestSize);
                        }
                        // TODO release the request memory if not already done after sent.
                        // TODO: Use IDisposable for observers, this will allow using "using".
                        observer?.Detach();
                    }
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

                ProxyKind20 proxyKind = istr.ReadProxyKind20();
                if (proxyKind == ProxyKind20.Null)
                {
                    return null;
                }

                var proxyData = new ProxyData20(istr);

                if (proxyData.Identity.Name.Length == 0)
                {
                    throw new InvalidDataException("received non-null proxy with empty identity name");
                }

                Protocol protocol = proxyData.Protocol ?? Protocol.Ice2;

                if (proxyData.InvocationMode != null && protocol != Protocol.Ice1)
                {
                    throw new InvalidDataException(
                        $"received proxy for protocol {protocol.GetName()} with invocation mode set");
                }

                if (proxyKind == ProxyKind20.IndirectRelative)
                {
                    if (istr.Connection is Connection connection)
                    {
                        if (connection.Protocol != protocol)
                        {
                            throw new InvalidDataException(
                                $"received a relative proxy with invalid protocol {protocol.GetName()}");
                        }

                        // TODO: location is missing
                        return new Reference(context: connection.Communicator.CurrentContext,
                                             encoding: proxyData.Encoding ?? Encoding.V20,
                                             facet: proxyData.Facet ?? "",
                                             fixedConnection: connection,
                                             identity: proxyData.Identity,
                                             invocationInterceptors:
                                                connection.Communicator.DefaultInvocationInterceptors,
                                             invocationMode: InvocationMode.Twoway,
                                             invocationTimeout: null);
                    }
                    else
                    {
                        Reference? source = istr.Reference;

                        if (source == null)
                        {
                            throw new InvalidOperationException(
                                "cannot read a relative proxy from InputStream created without a connection or proxy");
                        }

                        if (source.Protocol != protocol)
                        {
                            throw new InvalidDataException(
                                $"received a relative proxy with invalid protocol {protocol.GetName()}");
                        }

                        if (proxyData.Location?.Length > 1)
                        {
                            throw new InvalidDataException($"received a relative proxy with an invalid location");
                        }

                        IReadOnlyList<string> location = source.Location;
                        if (proxyData.Location?.Length == 1)
                        {
                            // Replace the last segment of location
                            if (location.Count == 0)
                            {
                                location = ImmutableArray.Create(proxyData.Location[0]);
                            }
                            else
                            {
                                ImmutableArray<string>.Builder builder =
                                    ImmutableArray.CreateBuilder<string>(location.Count);
                                builder.AddRange(location.SkipLast(1));
                                builder.Add(proxyData.Location[0]);
                                location = builder.ToImmutable();
                            }
                        }

                        return source.Clone(encoding: proxyData.Encoding ?? Encoding.V20,
                                            facet: proxyData.Facet ?? "",
                                            identity: proxyData.Identity,
                                            location: location);
                    }
                }
                else
                {
                    // The min size for an Endpoint with the 2.0 encoding is: transport (short = 2 bytes) + host name
                    // (min 2 bytes as it cannot be empty) + port number (ushort, 2 bytes) + options (1 byte for empty
                    // sequence), for a total of 7 bytes.
                    IReadOnlyList<Endpoint> endpoints = proxyKind == ProxyKind20.Direct ?
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
        }

        // Helper constructor for non-fixed references. Uses the communicator's defaults.
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
                   context: communicator.DefaultContext,
                   encoding: encoding,
                   endpoints: endpoints,
                   facet: facet,
                   identity: identity,
                   invocationInterceptors: communicator.DefaultInvocationInterceptors,
                   invocationMode: invocationMode,
                   invocationTimeout: null,
                   label: null,
                   location: location,
                   locatorCacheTimeout: null,
                   locatorInfo: communicator.GetLocatorInfo(communicator.DefaultLocator),
                   preferExistingConnection: null,
                   preferNonSecure: null,
                   protocol: protocol,
                   relative: false,
                   routerInfo: protocol == Protocol.Ice1 ?
                    communicator.GetRouterInfo(communicator.DefaultRouter) : null)
        {
        }

        // Helper constructor for fixed references. Uses the communicator's defaults.
        internal Reference(Connection fixedConnection, Identity identity, string facet)
            : this(context: fixedConnection.Communicator.DefaultContext,
                   encoding: fixedConnection.Protocol.GetEncoding(),
                   facet: facet,
                   fixedConnection: fixedConnection,
                   identity: identity,
                   invocationInterceptors: ImmutableList<InvocationInterceptor>.Empty,
                   invocationMode: (fixedConnection.Endpoint?.IsDatagram ?? false) ?
                       InvocationMode.Datagram : InvocationMode.Twoway,
                   invocationTimeout: null)
        {
        }

        private void ClearConnection(Connection connection)
        {
            Debug.Assert(!IsFixed);
            Interlocked.CompareExchange(ref _connection, null, connection);
        }

        internal Reference Clone(
            bool? cacheConnection = null,
            bool clearLabel = false,
            bool clearLocator = false,
            bool clearRouter = false,
            IReadOnlyDictionary<string, string>? context = null, // can be provided by app
            Encoding? encoding = null,
            IEnumerable<Endpoint>? endpoints = null, // from app
            string? facet = null,
            Connection? fixedConnection = null,
            Identity? identity = null,
            string? identityAndFacet = null,
            IEnumerable<InvocationInterceptor>? invocationInterceptors = null, // from app
            InvocationMode? invocationMode = null,
            TimeSpan? invocationTimeout = null,
            object? label = null,
            IEnumerable<string>? location = null, // from app
            ILocatorPrx? locator = null,
            TimeSpan? locatorCacheTimeout = null,
            bool? oneway = null,
            bool? preferExistingConnection = null,
            NonSecure? preferNonSecure = null,
            bool? relative = null,
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
                    throw new ArgumentException(
                        "cannot change the connection caching configuration of a fixed proxy",
                        nameof(cacheConnection));
                }
                if (endpoints != null)
                {
                    throw new ArgumentException("cannot change the endpoints of a fixed proxy", nameof(endpoints));
                }
                if (clearLabel)
                {
                    throw new ArgumentException("cannot change the label of a fixed proxy", nameof(clearLabel));
                }
                else if (label != null)
                {
                    throw new ArgumentException("cannot change the label of a fixed proxy", nameof(label));
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
                    throw new ArgumentException(
                        "cannot set locator cache timeout on a fixed proxy",
                        nameof(locatorCacheTimeout));
                }
                if (preferExistingConnection != null)
                {
                    throw new ArgumentException(
                        "cannot change the prefer-existing-connection configuration of a fixed proxy",
                        nameof(preferExistingConnection));
                }
                if (preferNonSecure != null)
                {
                    throw new ArgumentException(
                        "cannot change the prefer non-secure configuration of a fixed proxy",
                        nameof(preferNonSecure));
                }
                if (relative ?? false)
                {
                    throw new ArgumentException("cannot convert a fixed proxy into a relative proxy", nameof(relative));
                }
                if (router != null)
                {
                    throw new ArgumentException("cannot change the router of a fixed proxy", nameof(router));
                }
                else if (clearRouter)
                {
                    throw new ArgumentException("cannot change the router of a fixed proxy", nameof(clearRouter));
                }

                var clone = new Reference(
                    context?.ToImmutableSortedDictionary() ?? Context,
                    encoding ?? Encoding,
                    facet ?? Facet,
                    (fixedConnection ?? _connection)!,
                    identity ?? Identity,
                    invocationInterceptors?.ToImmutableList() ?? InvocationInterceptors,
                    invocationMode ?? InvocationMode,
                    invocationTimeout ?? _invocationTimeout);
                return clone == this ? this : clone;
            }
            else
            {
                // Non-fixed reference
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

                if (label != null && clearLabel)
                {
                    throw new ArgumentException($"cannot set both {nameof(label)} and {nameof(clearLabel)}");
                }

                if (locator != null && clearLocator)
                {
                    throw new ArgumentException($"cannot set both {nameof(locator)} and {nameof(clearLocator)}");
                }
                if (Protocol != Protocol.Ice1 && router != null)
                {
                    throw new ArgumentException("only an ice1 proxy can have a router", nameof(router));
                }
                if (router != null && clearRouter)
                {
                    throw new ArgumentException($"cannot set both {nameof(router)} and {nameof(clearRouter)}");
                }
                if (router != null && router.Protocol != Protocol.Ice1)
                {
                    throw new ArgumentException($"{nameof(router)} must be an ice1 proxy", nameof(router));
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

                if (relative ?? IsRelative)
                {
                    if (newEndpoints?.Count > 0)
                    {
                        throw new ArgumentException("a relative proxy cannot have endpoints", nameof(relative));
                    }
                    else
                    {
                        newEndpoints = ImmutableArray<Endpoint>.Empty; // make sure the clone's endpoints are empty
                    }
                }

                newEndpoints ??= Endpoints;

                LocatorInfo? locatorInfo = LocatorInfo;
                if (locator != null)
                {
                    if (newEndpoints.Count > 0)
                    {
                        throw new ArgumentException($"cannot set {nameof(locator)} on a direct proxy",
                                                    nameof(locator));
                    }

                    locatorInfo = Communicator.GetLocatorInfo(locator);
                }
                else if (clearLocator || newEndpoints.Count > 0)
                {
                    locatorInfo = null;
                }

                if (locatorCacheTimeout != null)
                {
                    if (newEndpoints.Count > 0)
                    {
                        throw new ArgumentException($"cannot set {nameof(locatorCacheTimeout)} on a direct proxy",
                                                    nameof(locatorCacheTimeout));
                    }
                    if (locatorInfo == null)
                    {
                        throw new ArgumentException($"cannot set {nameof(locatorCacheTimeout)} without a locator",
                                                    nameof(locatorCacheTimeout));
                    }
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

                var clone = new Reference(cacheConnection ?? CacheConnection,
                                          Communicator,
                                          context?.ToImmutableSortedDictionary() ?? Context,
                                          encoding ?? Encoding,
                                          newEndpoints,
                                          facet ?? Facet,
                                          identity ?? Identity,
                                          invocationInterceptors?.ToImmutableList() ?? InvocationInterceptors,
                                          invocationMode ?? InvocationMode,
                                          invocationTimeout ?? _invocationTimeout,
                                          clearLabel ? null : label ?? Label,
                                          newLocation ?? Location,
                                          locatorCacheTimeout ?? (locatorInfo != null ? _locatorCacheTimeout : null),
                                          locatorInfo, // no fallback otherwise breaks clearLocator
                                          preferExistingConnection ?? _preferExistingConnection,
                                          preferNonSecure ?? _preferNonSecure,
                                          Protocol,
                                          relative ?? IsRelative,
                                          routerInfo); // no fallback otherwise breaks clearRouter

                return clone == this ? this : clone;
            }
        }

        private async ValueTask<(List<Endpoint> Endpoints, TimeSpan EndpointsAge)> ComputeEndpointsAsync(
            TimeSpan endpointsMaxAge,
            CancellationToken cancel)
        {
            Debug.Assert(!IsFixed);
            // If the invocation mode is not datagram, we first check if the target is colocated and if that's the
            // case we use the colocated endpoint.
            if (InvocationMode != InvocationMode.Datagram &&
                Communicator.GetColocatedEndpoint(this) is Endpoint colocatedEndpoint)
            {
                return (new List<Endpoint>() { colocatedEndpoint }, TimeSpan.Zero);
            }

            IReadOnlyList<Endpoint>? endpoints = ImmutableArray<Endpoint>.Empty;
            if (RouterInfo != null)
            {
                // Get the router client endpoints if a router is configured
                endpoints = await RouterInfo.GetClientEndpointsAsync(cancel).ConfigureAwait(false);
            }

            TimeSpan endpointsAge = TimeSpan.Zero;
            if (endpoints.Count == 0)
            {
                // Get the proxy's endpoint or query the locator to get endpoints
                if (Endpoints.Count > 0)
                {
                    endpoints = Endpoints.ToList();
                }
                else if (LocatorInfo != null)
                {
                    (endpoints, endpointsAge) =
                        await LocatorInfo.ResolveIndirectReferenceAsync(this,
                                                                        endpointsMaxAge,
                                                                        cancel).ConfigureAwait(false);
                }
            }

            // Apply overrides and filter endpoints
            var filteredEndpoints = endpoints.Where(endpoint =>
            {
                // Filter out opaque and universal endpoints
                if (endpoint is OpaqueEndpoint || endpoint is UniversalEndpoint)
                {
                    return false;
                }

                // With ice1 when secure endpoint is required filter out all non-secure endpoints.
                if (Protocol == Protocol.Ice1 && PreferNonSecure == NonSecure.Never && !endpoint.IsAlwaysSecure)
                {
                    return false;
                }

                // Check if the endpoint is compatible with the proxy invocation mode, Twoway and Oneway invocation
                // modes require a non datagram endpoint, Datagram invocation mode requires a datagram endpoint, the
                // other invocation modes (BatchOneway and BatchDagram) are not supported.
                return InvocationMode switch
                {
                    InvocationMode.Twoway or InvocationMode.Oneway => !endpoint.IsDatagram,
                    InvocationMode.Datagram => endpoint.IsDatagram,
                    _ => false
                };
            }).ToList();

            if (filteredEndpoints.Count == 0)
            {
                throw new NoEndpointException(ToString());
            }

            if (filteredEndpoints.Count > 1)
            {
                filteredEndpoints = Communicator.OrderEndpointsByTransportFailures(filteredEndpoints);
            }
            return (filteredEndpoints, endpointsAge);
        }
        internal Connection? GetCachedConnection() => _connection;

        internal async ValueTask<Connection> GetConnectionAsync(CancellationToken cancel)
        {
            Connection? connection = _connection;
            if (connection != null && connection.IsActive)
            {
                return connection;
            }
            using var linkedCancellationSource = CancellationTokenSource.CreateLinkedTokenSource(
                cancel,
                Communicator.CancellationToken);
            cancel = linkedCancellationSource.Token;
            TimeSpan endpointsAge = TimeSpan.Zero;
            TimeSpan endpointsMaxAge = TimeSpan.MaxValue;
            List<Endpoint>? endpoints = null;
            if ((connection == null || (!IsFixed && !connection.IsActive)) && PreferExistingConnection)
            {
                // No cached connection, so now check if there is an existing connection that we can reuse.
                (endpoints, endpointsAge) = await ComputeEndpointsAsync(endpointsMaxAge, cancel).ConfigureAwait(false);
                connection = Communicator.GetConnection(endpoints, PreferNonSecure, Label);
                if (CacheConnection)
                {
                    _connection = connection;
                }
            }

            while (connection == null)
            {
                if (endpoints == null)
                {
                    (endpoints, endpointsAge) = await ComputeEndpointsAsync(endpointsMaxAge,
                                                                            cancel).ConfigureAwait(false);
                }

                Endpoint last = endpoints[^1];
                foreach (Endpoint endpoint in endpoints)
                {
                    try
                    {
                        connection = await Communicator.ConnectAsync(endpoint,
                                                                     PreferNonSecure,
                                                                     Label,
                                                                     cancel).ConfigureAwait(false);
                        if (CacheConnection)
                        {
                            _connection = connection;
                        }
                        break;
                    }
                    catch
                    {
                        // Ignore the exception unless this is the last endpoint.
                        if (ReferenceEquals(endpoint, last))
                        {
                            if (IsIndirect && endpointsAge != TimeSpan.Zero && endpointsMaxAge == TimeSpan.MaxValue)
                            {
                                // If the first lookup for an indirect reference returns an endpoint from the cache, set
                                // endpointsMaxAge to force a new locator lookup for a fresher endpoint.
                                endpointsMaxAge = endpointsAge;
                                endpoints = null;
                            }
                            else
                            {
                                throw;
                            }
                        }
                    }
                }
            }
            Debug.Assert(connection != null);
            return connection;
        }

        internal Dictionary<string, string> ToProperty(string prefix)
        {
            if (IsFixed)
            {
                throw new NotSupportedException("cannot convert a fixed proxy to a property dictionary");
            }

            var properties = new Dictionary<string, string> { [prefix] = ToString() };

            if (Protocol == Protocol.Ice1)
            {
                if (!CacheConnection)
                {
                    properties[$"{prefix}.CacheConnection"] = "false";
                }

                // We don't output context as this would require hard-to-generate escapes.

                if (_invocationTimeout is TimeSpan invocationTimeout)
                {
                    // For ice2 the invocation timeout is included in the URI
                    properties[$"{prefix}.InvocationTimeout"] = invocationTimeout.ToPropertyValue();
                }
                if (Label?.ToString() is string label && label.Length > 0)
                {
                    properties[$"{prefix}.Label"] = label;
                }
                if (LocatorInfo != null)
                {
                    Dictionary<string, string> locatorProperties = LocatorInfo.Locator.ToProperty(prefix + ".Locator");
                    foreach (KeyValuePair<string, string> entry in locatorProperties)
                    {
                        properties[entry.Key] = entry.Value;
                    }
                }
                if (_locatorCacheTimeout is TimeSpan locatorCacheTimeout)
                {
                    properties[$"{prefix}.LocatorCacheTimeout"] = locatorCacheTimeout.ToPropertyValue();
                }
                if (_preferExistingConnection is bool preferExistingConnection)
                {
                    properties[$"{prefix}.PreferExistingConnection"] = preferExistingConnection ? "true" : "false";
                }
                if (_preferNonSecure is NonSecure preferNonSecure)
                {
                    properties[$"{prefix}.PreferNonSecure"] = preferNonSecure.ToString();
                }
                if (IsRelative)
                {
                    properties[$"{prefix}.Relative"] = "true";
                }
                if (RouterInfo != null)
                {
                    Dictionary<string, string> routerProperties = RouterInfo.Router.ToProperty(prefix + ".Router");
                    foreach (KeyValuePair<string, string> entry in routerProperties)
                    {
                        properties[entry.Key] = entry.Value;
                    }
                }
            }
            // else, only a single property in the dictionary

            return properties;
        }

        // Marshal the non-null reference.
        internal void Write(OutputStream ostr)
        {
            if (IsFixed)
            {
                throw new NotSupportedException("cannot marshal a fixed proxy");
            }

            if (ostr.Encoding == Encoding.V11)
            {
                if (IsRelative)
                {
                    throw new NotSupportedException("cannot marshal a relative proxy with the 1.1 encoding");
                }

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

                ostr.Write(Endpoints.Count > 0 ? ProxyKind20.Direct :
                    IsRelative ? ProxyKind20.IndirectRelative : ProxyKind20.Indirect);

                IReadOnlyList<string> location = Location;
                if (IsRelative && location.Count > 1)
                {
                    // Reduce location to its last segment
                    location = ImmutableArray.Create(location[^1]);
                }

                ostr.WriteProxyData20(Identity, Protocol, Encoding, location, InvocationMode, Facet);

                if (Endpoints.Count > 0)
                {
                    ostr.WriteSequence(Endpoints, (ostr, endpoint) => ostr.WriteEndpoint(endpoint));
                }
            }
        }

        private static void CheckForUnknownProperties(string prefix, Communicator communicator)
        {
            // Do not warn about unknown properties if Ice prefix, i.e. Ice, Glacier2, etc.
            foreach (string name in PropertyNames.ClassPropertyNames)
            {
                if (prefix.StartsWith($"{name}.", StringComparison.Ordinal))
                {
                    return;
                }
            }

            var unknownProps = new List<string>();
            Dictionary<string, string> props = communicator.GetProperties(forPrefix: $"{prefix}.");
            foreach (string prop in props.Keys)
            {
                bool valid = false;
                for (int i = 0; i < _suffixes.Length; ++i)
                {
                    string pattern = "^" + Regex.Escape(prefix + ".") + _suffixes[i] + "$";
                    if (new Regex(pattern).Match(prop).Success)
                    {
                        valid = true;
                        break;
                    }
                }

                if (!valid)
                {
                    unknownProps.Add(prop);
                }
            }

            if (unknownProps.Count != 0)
            {
                var message = new StringBuilder("found unknown properties for proxy '");
                message.Append(prefix);
                message.Append("':");
                foreach (string s in unknownProps)
                {
                    message.Append("\n    ");
                    message.Append(s);
                }
                communicator.Logger.Warning(message.ToString());
            }
        }

        // Constructor for non-fixed references, not bound to a connection
        private Reference(
            bool cacheConnection,
            Communicator communicator,
            IReadOnlyDictionary<string, string> context, // already a copy provided by Ice
            Encoding encoding,
            IReadOnlyList<Endpoint> endpoints, // already a copy provided by Ice
            string facet,
            Identity identity,
            IReadOnlyList<InvocationInterceptor> invocationInterceptors, // already a copy provided by Ice
            InvocationMode invocationMode,
            TimeSpan? invocationTimeout,
            object? label,
            IReadOnlyList<string> location, // already a copy provided by Ice
            TimeSpan? locatorCacheTimeout,
            LocatorInfo? locatorInfo,
            bool? preferExistingConnection,
            NonSecure? preferNonSecure,
            Protocol protocol,
            bool relative,
            RouterInfo? routerInfo)
        {
            CacheConnection = cacheConnection;
            Communicator = communicator;

            Context = context;
            Encoding = encoding;
            Endpoints = endpoints;
            Facet = facet;
            Identity = identity;
            InvocationInterceptors = invocationInterceptors;
            InvocationMode = invocationMode;
            _invocationTimeout = invocationTimeout;
            IsRelative = relative;
            Label = label;
            Location = location;
            _locatorCacheTimeout = locatorCacheTimeout;
            LocatorInfo = locatorInfo;
            _preferExistingConnection = preferExistingConnection;
            _preferNonSecure = preferNonSecure;
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

            Debug.Assert(!IsRelative || Endpoints.Count == 0);
            Debug.Assert(location.Count == 0 || location[0].Length > 0); // first segment cannot be empty
            Debug.Assert(!Endpoints.Any(endpoint => endpoint.Protocol != Protocol));
            Debug.Assert(invocationTimeout != TimeSpan.Zero);
        }

        // Constructor for fixed references.
        private Reference(
            IReadOnlyDictionary<string, string> context, // already a copy provided by Ice
            Encoding encoding,
            string facet,
            Connection fixedConnection,
            Identity identity,
            IReadOnlyList<InvocationInterceptor> invocationInterceptors, // already a copy provided by Ice
            InvocationMode invocationMode,
            TimeSpan? invocationTimeout)
        {
            Communicator = fixedConnection.Communicator;
            Label = null;
            Context = context;
            Encoding = encoding;
            Endpoints = Array.Empty<Endpoint>();
            Facet = facet;
            Identity = identity;
            InvocationInterceptors = invocationInterceptors;
            InvocationMode = invocationMode;
            _invocationTimeout = invocationTimeout;
            IsFixed = true;
            IsRelative = false;
            Location = ImmutableArray<string>.Empty;
            _locatorCacheTimeout = null;
            LocatorInfo = null;
            Protocol = fixedConnection.Protocol;
            RouterInfo = null;

            _connection = fixedConnection;

            if (Protocol == Protocol.Ice1)
            {
                if (InvocationMode == InvocationMode.Datagram)
                {
                    if (!_connection.Endpoint.IsDatagram)
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

        private async Task<IncomingResponseFrame> PerformInvokeAsync(
            OutgoingRequestFrame request,
            bool oneway,
            IProgress<bool>? progress,
            bool releaseRequestAfterSent,
            IInvocationObserver? observer,
            CancellationToken cancel)
        {
            Connection? connection = _connection;
            TimeSpan endpointsMaxAge = TimeSpan.MaxValue;
            TimeSpan endpointsAge = TimeSpan.Zero;
            List<Endpoint>? endpoints = null;

            if ((connection == null || (!IsFixed && !connection.IsActive)) && PreferExistingConnection)
            {
                // No cached connection, so now check if there is an existing connection that we can reuse.
                (endpoints, endpointsAge) = await ComputeEndpointsAsync(endpointsMaxAge, cancel).ConfigureAwait(false);
                connection = Communicator.GetConnection(endpoints, PreferNonSecure, Label);
                if (CacheConnection)
                {
                    _connection = connection;
                }
            }

            int nextEndpoint = 0;
            int attempt = 1;
            bool triedAllEndpoints = false;
            List<Endpoint>? excludedEndpoints = null;
            IncomingResponseFrame? response = null;
            Exception? exception = null;

            bool tryAgain;
            do
            {
                bool sent = false;
                IChildInvocationObserver? childObserver = null;
                SocketStream? stream = null;
                try
                {
                    if (connection == null)
                    {
                        if (endpoints == null)
                        {
                            Debug.Assert(nextEndpoint == 0);
                            // ComputeEndpointsAsync throws if it can't figure out the endpoints
                            (endpoints, endpointsAge) = await ComputeEndpointsAsync(endpointsMaxAge,
                                                                                    cancel).ConfigureAwait(false);
                            if (excludedEndpoints != null)
                            {
                                endpoints = endpoints.Except(excludedEndpoints).ToList();
                                if (endpoints.Count == 0)
                                {
                                    endpoints = null;
                                    throw new NoEndpointException();
                                }
                            }
                        }

                        connection = await Communicator.ConnectAsync(endpoints[nextEndpoint],
                                                                     PreferNonSecure,
                                                                     Label,
                                                                     cancel).ConfigureAwait(false);

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

                        if (CacheConnection)
                        {
                            _connection = connection;
                        }
                    }

                    cancel.ThrowIfCancellationRequested();

                    // Create the outgoing stream.
                    stream = connection.CreateStream(!oneway);

                    childObserver = observer?.GetChildInvocationObserver(connection, request.PayloadSize);
                    childObserver?.Attach();

                    // Send the request and wait for the sending to complete.
                    await stream.SendRequestFrameAsync(request, cancel).ConfigureAwait(false);

                    // The request is sent, notify the progress callback.
                    // TODO: Get rid of the sentSynchronously parameter which is always false now?
                    if (progress != null)
                    {
                        progress.Report(false);
                        progress = null; // Only call the progress callback once (TODO: revisit this?)
                    }
                    if (releaseRequestAfterSent)
                    {
                        // TODO release the request
                    }
                    sent = true;
                    exception = null;
                    response?.Dispose();

                    if (oneway)
                    {
                        return IncomingResponseFrame.WithVoidReturnValue(request.Protocol, request.PayloadEncoding);
                    }

                    // Wait for the reception of the response.
                    response = await stream.ReceiveResponseFrameAsync(cancel).ConfigureAwait(false);

                    childObserver?.Reply(response.PayloadSize);

                    // If success, just return the response!
                    if (response.ResultType == ResultType.Success)
                    {
                        return response;
                    }
                    observer?.RemoteException();
                }
                catch (NoEndpointException ex) when (endpointsAge == TimeSpan.Zero)
                {
                    // If we get NoEndpointException while using non cached endpoints, either all endpoints
                    // have been excluded or the proxy has no endpoints. we cannot retry, return here to
                    // preserve any previous exceptions that might have been throw.
                    childObserver?.Failed(ex.GetType().FullName ?? "System.Exception");
                    observer?.Failed(ex.GetType().FullName ?? "System.Exception"); // TODO cleanup observer logic
                    return response ?? throw exception ?? ex;
                }
                catch (Exception ex)
                {
                    exception = ex;
                    childObserver?.Failed(ex.GetType().FullName ?? "System.Exception");
                }
                finally
                {
                    stream?.TryDispose();
                    childObserver?.Detach();
                }

                // Compute retry policy based on the exception or response retry policy, whether or not the connection
                // is established or the request sent and idempotent
                Debug.Assert(response != null || exception != null);
                RetryPolicy retryPolicy =
                    response?.GetRetryPolicy(this) ?? exception!.GetRetryPolicy(request.IsIdempotent, sent);

                // With the retry-policy OtherReplica we add the current endpoint to the list of excluded
                // endpoints this prevents the endpoints to be tried again during the current retry sequence.
                if (retryPolicy == RetryPolicy.OtherReplica &&
                    (endpoints?[nextEndpoint] ?? connection?.Endpoint) is Endpoint endpoint)
                {
                    excludedEndpoints ??= new();
                    excludedEndpoints.Add(endpoint);
                }

                if (endpoints != null && (connection == null || retryPolicy == RetryPolicy.OtherReplica))
                {
                    // If connection establishment failed or if the endpoint was excluded, try the next endpoint
                    nextEndpoint = ++nextEndpoint % endpoints.Count;
                    if (nextEndpoint == 0)
                    {
                        // nextendpoint == 0 indicates that we already tried all the endpoints.
                        if (endpointsAge != TimeSpan.Zero)
                        {
                            // If we were using cached endpoints, we clear the endpoints, and set endpointsMaxAge to
                            // request a fresher endpoint.
                            endpoints = null;
                            endpointsMaxAge = endpointsAge;
                        }
                        else
                        {
                            // Otherwise we set triedAllEndpoints to true to ensure further connection establishment
                            // failures will now count as attempts (to prevent indefinitely looping if connection
                            // establishment failure results in a retryable exception).
                            triedAllEndpoints = true;
                            if (excludedEndpoints != null)
                            {
                                endpoints = endpoints.Except(excludedEndpoints).ToList();
                            }
                        }
                    }
                }

                // Check if we can retry, we cannot retry if we have consumed all attempts, the current retry
                // policy doesn't allow retries, the request was already released, there are no more endpoints
                // or a fixed reference receives an exception with OtherReplica retry policy.

                if (attempt == Communicator.InvocationMaxAttempts ||
                    retryPolicy == RetryPolicy.NoRetry ||
                    (sent && releaseRequestAfterSent) ||
                    (triedAllEndpoints && endpoints != null && endpoints.Count == 0) ||
                    (IsFixed && retryPolicy == RetryPolicy.OtherReplica))
                {
                    tryAgain = false;
                }
                else
                {
                    tryAgain = true;
                    if (Communicator.TraceLevels.Retry >= 1)
                    {
                        if (connection != null)
                        {
                            TraceRetry("retrying request because of retryable exception",
                                       attempt,
                                       retryPolicy,
                                       exception);
                        }
                        else if (triedAllEndpoints)
                        {
                            TraceRetry("retrying connection establishment because of retryable exception",
                                       attempt,
                                       retryPolicy,
                                       exception);
                        }
                        else
                        {
                            TraceRetry("retrying connection establishment because of retryable exception",
                                       0,
                                       policy: null,
                                       exception);
                        }
                    }

                    if (connection != null || triedAllEndpoints)
                    {
                        // Only count an attempt if the connection was established or if all the endpoints were tried
                        // at least once. This ensures that we don't end up into an infinite loop for connection
                        // establishment failures which don't result in endpoint exclusion.
                        attempt++;
                    }

                    if (retryPolicy.Retryable == Retryable.AfterDelay && retryPolicy.Delay != TimeSpan.Zero)
                    {
                        // The delay task can be canceled either by the user code using the provided cancellation
                        // token or if the communicator is destroyed.
                        await Task.Delay(retryPolicy.Delay, cancel).ConfigureAwait(false);
                    }

                    observer?.Retried();

                    // If an indirect reference is using a endpoint from the cache, set endpointsMaxAge to force
                    // a new locator lookup.
                    if (IsIndirect && endpointsAge != TimeSpan.Zero)
                    {
                        endpointsMaxAge = endpointsAge;
                        endpoints = null;
                    }

                    if (!IsFixed && connection != null)
                    {
                        // Retry with a new connection!
                        connection = null;
                    }
                }
            }
            while (tryAgain);

            // TODO cleanup observer logic we report "System.Exception" for all remote exceptions
            observer?.Failed(exception?.GetType().FullName ?? "System.Exception");
            Debug.Assert(response != null || exception != null);
            Debug.Assert(response == null || response.ResultType == ResultType.Failure);
            return response ?? throw ExceptionUtil.Throw(exception!);

            void TraceRetry(string message, int attempt = 0, RetryPolicy? policy = null, Exception? exception = null)
            {
                Debug.Assert(attempt >= 0 && attempt <= Communicator.InvocationMaxAttempts);
                var sb = new StringBuilder();
                sb.Append(message);
                sb.Append("\nproxy = ");
                sb.Append(this);
                sb.Append("\noperation = ");
                sb.Append(request.Operation);
                if (attempt > 0)
                {
                    sb.Append("\nrequest attempt = ");
                    sb.Append(attempt);
                    sb.Append('/');
                    sb.Append(Communicator.InvocationMaxAttempts);
                }
                if (policy != null)
                {
                    sb.Append("\nretry policy = ");
                    sb.Append(policy);
                }
                sb.Append("\nexception = ");
                sb.Append(exception?.ToString() ?? "\nexception = remote exception");
                Communicator.Logger.Trace(TraceLevels.RetryCategory, sb.ToString());
            }
        }
    }
}
