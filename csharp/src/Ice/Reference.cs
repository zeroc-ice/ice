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
using System.Text;

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
        internal IReadOnlyList<Endpoint> Endpoints { get; }
        internal string Facet { get; }
        internal Identity Identity { get; }
        internal InvocationMode InvocationMode { get; }
        internal int InvocationTimeout { get; }
        internal bool IsCollocationOptimized { get; }
        internal bool IsConnectionCached => IsFixed || _requestHandlerMutex != null;
        internal bool IsFixed => _fixedConnection != null;
        internal bool IsIndirect => !IsFixed && Endpoints.Count == 0;
        internal bool IsWellKnown => !IsFixed && Endpoints.Count == 0 && AdapterId.Length == 0;
        internal int LocatorCacheTimeout { get; }
        internal LocatorInfo? LocatorInfo { get; }
        internal bool PreferNonSecure { get; }
        internal Protocol Protocol { get; }
        internal RouterInfo? RouterInfo { get; }
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

        /// <summary>Creates a reference from a string and a communucator. This an Ice-internal publicly visible static
        /// method.</summary>
        public static Reference Parse(string s, Communicator communicator) => Parse(s, null, communicator);

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
                if (!Endpoints.SequenceEqual(other.Endpoints))
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
                hash.Add(Context.GetDictionaryHashCode());
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
                    foreach (Endpoint e in Endpoints)
                    {
                        hash.Add(e);
                    }
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
                    s.Append(" -t");
                    break;
                case InvocationMode.Oneway:
                    s.Append(" -o");
                    break;
                case InvocationMode.BatchOneway:
                    s.Append(" -O");
                    break;
                case InvocationMode.Datagram:
                    s.Append(" -d");
                    break;
                case InvocationMode.BatchDatagram:
                    s.Append(" -D");
                    break;
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

            if (AdapterId.Length > 0)
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
            else
            {
                foreach (Endpoint e in Endpoints)
                {
                    s.Append(":");
                    s.Append(e);
                }
            }
            return s.ToString();
        }

        internal static Reference Parse(string s, string? propertyPrefix, Communicator communicator)
        {
            const string delim = " \t\n\r";

            int beg;
            int end = 0;

            beg = IceUtilInternal.StringUtil.FindFirstNotOf(s, delim, end);
            if (beg == -1)
            {
                throw new FormatException($"no non-whitespace characters found in `{s}'");
            }

            // Extract the identity, which may be enclosed in single or double quotation marks.
            string identityString;
            end = IceUtilInternal.StringUtil.CheckQuote(s, beg);
            if (end == -1)
            {
                throw new FormatException($"mismatched quotes around identity in `{s} '");
            }
            else if (end == 0)
            {
                end = IceUtilInternal.StringUtil.FindFirstOf(s, delim + ":@", beg);
                if (end == -1)
                {
                    end = s.Length;
                }
                identityString = s[beg..end];
            }
            else
            {
                beg++; // Skip leading quote
                identityString = s[beg..end];
                end++; // Skip trailing quote
            }

            if (beg == end)
            {
                throw new FormatException($"no identity in `{s}'");
            }

            // Parsing the identity may raise FormatException.
            var identity = Identity.Parse(identityString);

            string facet = "";
            InvocationMode invocationMode = InvocationMode.Twoway;
            Encoding encoding = communicator.DefaultEncoding;
            Protocol protocol = Protocol.Ice1;
            string adapterId;

            while (true)
            {
                beg = IceUtilInternal.StringUtil.FindFirstNotOf(s, delim, end);
                if (beg == -1)
                {
                    break;
                }

                if (s[beg] == ':' || s[beg] == '@')
                {
                    break;
                }

                end = IceUtilInternal.StringUtil.FindFirstOf(s, delim + ":@", beg);
                if (end == -1)
                {
                    end = s.Length;
                }

                if (beg == end)
                {
                    break;
                }

                string option = s[beg..end];
                if (option.Length != 2 || option[0] != '-')
                {
                    throw new FormatException("expected a proxy option but found `{option}' in `{s}'");
                }

                // Check for the presence of an option argument. The argument may be enclosed in single or double
                // quotation marks.
                string? argument = null;
                int argumentBeg = IceUtilInternal.StringUtil.FindFirstNotOf(s, delim, end);
                if (argumentBeg != -1)
                {
                    char ch = s[argumentBeg];
                    if (ch != '@' && ch != ':' && ch != '-')
                    {
                        beg = argumentBeg;
                        end = IceUtilInternal.StringUtil.CheckQuote(s, beg);
                        if (end == -1)
                        {
                            throw new FormatException($"mismatched quotes around value for {option} option in `{s}'");
                        }
                        else if (end == 0)
                        {
                            end = IceUtilInternal.StringUtil.FindFirstOf(s, delim + ":@", beg);
                            if (end == -1)
                            {
                                end = s.Length;
                            }
                            argument = s[beg..end];
                        }
                        else
                        {
                            beg++; // Skip leading quote
                            argument = s[beg..end];
                            end++; // Skip trailing quote
                        }
                    }
                }

                switch (option[1])
                {
                    case 'f':
                        if (argument == null)
                        {
                            throw new FormatException($"no argument provided for -f option in `{s}'");
                        }
                        facet = IceUtilInternal.StringUtil.UnescapeString(argument, 0, argument.Length, "");
                        break;

                    case 't':
                        if (argument != null)
                        {
                            throw new FormatException(
                                $"unexpected argument `{argument}' provided for -t option in `{s}'");
                        }
                        invocationMode = InvocationMode.Twoway;
                        break;

                    case 'o':
                        if (argument != null)
                        {
                            throw new FormatException(
                                $"unexpected argument `{argument}' provided for -o option in `{s}'");
                        }
                        invocationMode = InvocationMode.Oneway;
                        break;

                    case 'O':
                        if (argument != null)
                        {
                            throw new FormatException(
                                $"unexpected argument `{argument}' provided for -O option in `{s}'");
                        }
                        invocationMode = InvocationMode.BatchOneway;
                        break;

                    case 'd':
                        if (argument != null)
                        {
                            throw new FormatException(
                                $"unexpected argument `{argument}' provided for -d option in `{s}'");
                        }
                        invocationMode = InvocationMode.Datagram;
                        break;

                    case 'D':
                        if (argument != null)
                        {
                            throw new FormatException(
                                $"unexpected argument `{argument}' provided for -D option in `{s}'");
                        }
                        invocationMode = InvocationMode.BatchDatagram;
                        break;

                    case 's':
                        if (argument != null)
                        {
                            throw new FormatException(
                                $"unexpected argument `{argument}' provided for -s option in `{s}'");
                        }
                        communicator.Logger.Warning(
                            $"while parsing `{s}': the `-s' proxy option no longer has any effect");
                        break;

                    case 'e':
                        if (argument == null)
                        {
                            throw new FormatException($"no argument provided for -e option in `{s}'");
                        }
                        encoding = Encoding.Parse(argument);
                        break;

                    case 'p':
                        if (argument == null)
                        {
                            throw new FormatException($"no argument provided for -p option `{s}'");
                        }
                        protocol = ProtocolExtensions.Parse(argument);
                        break;

                    default:
                        throw new FormatException("unknown option `{option}' in `{s}'");
                }
            }

            if (beg == -1)
            {
                return Create(adapterId: "", communicator, encoding, endpoints: Array.Empty<Endpoint>(), facet,
                    identity, invocationMode, propertyPrefix, protocol);
            }

            var endpoints = new List<Endpoint>();

            if (s[beg] == ':')
            {
                end = beg;

                while (end < s.Length && s[end] == ':')
                {
                    beg = end + 1;

                    end = beg;
                    while (true)
                    {
                        end = s.IndexOf(':', end);
                        if (end == -1)
                        {
                            end = s.Length;
                            break;
                        }
                        else
                        {
                            bool quoted = false;
                            int quote = beg;
                            while (true)
                            {
                                quote = s.IndexOf('\"', quote);
                                if (quote == -1 || end < quote)
                                {
                                    break;
                                }
                                else
                                {
                                    quote = s.IndexOf('\"', ++quote);
                                    if (quote == -1)
                                    {
                                        break;
                                    }
                                    else if (end < quote)
                                    {
                                        quoted = true;
                                        break;
                                    }
                                    ++quote;
                                }
                            }
                            if (!quoted)
                            {
                                break;
                            }
                            ++end;
                        }
                    }

                    string es = s[beg..end];
                    endpoints.Add(Endpoint.Parse(es, communicator, false));
                }

                Debug.Assert(endpoints.Count > 0);
                return Create(adapterId: "", communicator, encoding, endpoints, facet, identity, invocationMode,
                    propertyPrefix, protocol);
            }
            else if (s[beg] == '@')
            {
                beg = IceUtilInternal.StringUtil.FindFirstNotOf(s, delim, beg + 1);
                if (beg == -1)
                {
                    throw new FormatException($"missing adapter id in `{s}'");
                }

                string adapterstr;
                end = IceUtilInternal.StringUtil.CheckQuote(s, beg);
                if (end == -1)
                {
                    throw new FormatException($"mismatched quotes around adapter id in `{s}'");
                }
                else if (end == 0)
                {
                    end = IceUtilInternal.StringUtil.FindFirstOf(s, delim, beg);
                    if (end == -1)
                    {
                        end = s.Length;
                    }
                    adapterstr = s[beg..end];
                }
                else
                {
                    beg++; // Skip leading quote
                    adapterstr = s[beg..end];
                    end++; // Skip trailing quote
                }

                if (end != s.Length && IceUtilInternal.StringUtil.FindFirstNotOf(s, delim, end) != -1)
                {
                    throw new FormatException(
                        $"invalid trailing characters after `{s.Substring(0, end + 1)}' in `{s}'");
                }

                adapterId = IceUtilInternal.StringUtil.UnescapeString(adapterstr, 0, adapterstr.Length, "");

                if (adapterId.Length == 0)
                {
                    throw new FormatException($"empty adapter id in `{s}'");
                }

                return Create(adapterId, communicator, encoding, endpoints: Array.Empty<Endpoint>(), facet, identity,
                    invocationMode, propertyPrefix, protocol);
            }

            throw new FormatException($"malformed proxy `{s}'");
        }

        /// <summary>Reads a reference from the input stream.</summary>
        /// <param name="istr">The input stream to read from.</param>
        /// <returns>The reference read from the stream (can be null).</returns>
        internal static Reference? Read(InputStream istr)
        {
            var identity = new Identity(istr);
            if (identity.Name.Length == 0)
            {
                return null;
            }

            string facet = istr.ReadFacet();
            int mode = istr.ReadByte();
            if (mode < 0 || mode > (int)InvocationMode.Last)
            {
                throw new InvalidDataException($"invalid invocation mode: {mode}");
            }

            istr.ReadBool(); // secure option, ignored

            byte major = istr.ReadByte();
            byte minor = istr.ReadByte();
            if (minor != 0)
            {
                throw new InvalidDataException($"received proxy with protocol set to {major}.{minor}");
            }
            var protocol = (Protocol)major;

            major = istr.ReadByte();
            minor = istr.ReadByte();
            var encoding = new Encoding(major, minor);

            Endpoint[] endpoints;
            string adapterId = "";

            int sz = istr.ReadSize();
            if (sz > 0)
            {
                endpoints = new Endpoint[sz];
                for (int i = 0; i < sz; i++)
                {
                    endpoints[i] = istr.ReadEndpoint();
                }
            }
            else
            {
                endpoints = Array.Empty<Endpoint>();
                adapterId = istr.ReadString();
            }

            return new Reference(adapterId: adapterId, communicator: istr.Communicator, encoding: encoding,
                endpoints: endpoints, facet: facet, identity: identity, invocationMode: (InvocationMode)mode,
                protocol: protocol);
        }

        // Helper constructor for routable references, not bound to a connection. Uses the communicator's defaults.
        internal Reference(string adapterId,
                           Communicator communicator,
                           Encoding encoding,
                           IReadOnlyList<Endpoint> endpoints, // already a copy provided by Ice
                           string facet,
                           Identity identity,
                           InvocationMode invocationMode,
                           Protocol protocol)
            : this(adapterId: adapterId,
                   cacheConnection: true,
                   collocationOptimized: communicator.DefaultCollocationOptimized,
                   communicator: communicator,
                   compress: null,
                   connectionId: "",
                   connectionTimeout: null,
                   context: communicator.DefaultContext,
                   encoding: encoding,
                   endpointSelection: communicator.DefaultEndpointSelection,
                   endpoints: endpoints,
                   facet: facet,
                   identity: identity,
                   invocationMode: invocationMode,
                   invocationTimeout: communicator.DefaultInvocationTimeout,
                   locatorCacheTimeout: communicator.DefaultLocatorCacheTimeout,
                   locatorInfo: communicator.GetLocatorInfo(communicator.DefaultLocator, encoding),
                   preferNonSecure: communicator.DefaultPreferNonSecure,
                   protocol: protocol,
                   routerInfo: communicator.GetRouterInfo(communicator.DefaultRouter))
        {
        }

        // Helper constructor for fixed references. Uses the communicator's defaults.
        internal Reference(Communicator communicator, Connection fixedConnection, Identity identity)
            : this(communicator: communicator,
                   compress: null,
                   context: communicator.DefaultContext,
                   encoding: communicator.DefaultEncoding,
                   facet: "",
                   fixedConnection: fixedConnection,
                   identity: identity,
                   invocationMode: fixedConnection.Endpoint.IsDatagram ?
                      InvocationMode.Datagram : InvocationMode.Twoway,
                   invocationTimeout: -1)
        {
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
                                 IEnumerable<Endpoint>? endpoints = null, // from app
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

                IReadOnlyList<Endpoint>? newEndpoints = null;

                if (adapterId != null)
                {
                    newEndpoints = Array.Empty<Endpoint>(); // make sure the clone's endpoints are empty
                }
                else if (endpoints != null)
                {
                    adapterId = ""; // make sure the clone's adapterID is empty
                    newEndpoints = endpoints.ToArray(); // make a copy
                }

                LocatorInfo? locatorInfo = LocatorInfo;
                if (locator != null)
                {
                    locatorInfo = Communicator.GetLocatorInfo(locator, encoding ?? locator.Encoding);
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

                // Update/create the newEndpoints if needed
                if (compress != null || connectionId != null || connectionTimeout != null || newEndpoints != null)
                {
                    newEndpoints ??= Endpoints;
                    if (newEndpoints.Count > 0)
                    {
                        compress ??= Compress;
                        connectionId ??= ConnectionId;
                        connectionTimeout ??= ConnectionTimeout;

                        newEndpoints = newEndpoints.Select(endpoint =>
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
                                      newEndpoints ?? Endpoints,
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

        internal void CreateConnection(IReadOnlyList<Endpoint> allEndpoints, IGetConnectionCallback callback)
        {
            Debug.Assert(!IsFixed);
            IReadOnlyList<Endpoint> endpoints = FilterEndpoints(allEndpoints);
            if (endpoints.Count == 0)
            {
                callback.SetException(new NoEndpointException(ToString()));
                return;
            }

            //
            // Finally, create the connection.
            //
            OutgoingConnectionFactory factory = Communicator.OutgoingConnectionFactory();
            if (IsConnectionCached || endpoints.Count == 1)
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

        // Marshal the reference.
        internal void Write(OutputStream ostr)
        {
            if (IsFixed)
            {
                throw new NotSupportedException("cannot marshal a fixed proxy");
            }

            Identity.IceWrite(ostr);
            ostr.WriteFacet(Facet);
            ostr.WriteByte((byte)InvocationMode);
            ostr.WriteBool(false); // secure option, always false (not used)
            ostr.WriteByte((byte)Protocol);
            ostr.WriteByte(0);
            ostr.WriteByte(Encoding.Major);
            ostr.WriteByte(Encoding.Minor);

            ostr.WriteSize(Endpoints.Count);
            if (Endpoints.Count > 0)
            {
                Debug.Assert(AdapterId.Length == 0);
                foreach (Endpoint endpoint in Endpoints)
                {
                    ostr.WriteEndpoint(endpoint);
                }
            }
            else
            {
                ostr.WriteString(AdapterId);
            }
        }

        // Helper factory method used by Parse.
        private static Reference Create(string adapterId,
                                        Communicator communicator,
                                        Encoding encoding,
                                        IReadOnlyList<Endpoint> endpoints,
                                        string facet,
                                        Identity identity,
                                        InvocationMode invocationMode,
                                        string? propertyPrefix,
                                        Protocol protocol)
        {
            bool? cacheConnection = null;
            bool? collocOptimized = null;
            IReadOnlyDictionary<string, string>? context = null;
            EndpointSelectionType? endpointSelection = null;
            int? invocationTimeout = null;
            int? locatorCacheTimeout = null;
            LocatorInfo? locatorInfo = null;
            bool? preferNonSecure = null;
            RouterInfo? routerInfo = null;

            // Override the defaults with the proxy properties if a property prefix is defined.
            if (propertyPrefix != null && propertyPrefix.Length > 0)
            {
                // Warn about unknown properties.
                if (communicator.GetPropertyAsBool("Ice.Warn.UnknownProperties") ?? true)
                {
                    communicator.CheckForUnknownProperties(propertyPrefix);
                }

                cacheConnection = communicator.GetPropertyAsBool($"{propertyPrefix}.ConnectionCached");
                collocOptimized = communicator.GetPropertyAsBool($"{propertyPrefix}.CollocationOptimized");

                string property = $"{propertyPrefix}.Context.";
                context = communicator.GetProperties(forPrefix: property).
                    ToDictionary(e => e.Key.Substring(property.Length), e => e.Value);
                if (context.Count == 0)
                {
                    context = null;
                }

                property = $"{propertyPrefix}.EndpointSelection";
                try
                {
                    endpointSelection =
                        communicator.GetProperty(property) is string endpointSelectionStr ?
                            Enum.Parse<EndpointSelectionType>(endpointSelectionStr) : (EndpointSelectionType?)null;
                }
                catch (FormatException ex)
                {
                    throw new InvalidConfigurationException($"cannot parse property `{property}'", ex);
                }

                property = $"{propertyPrefix}.InvocationTimeout";
                invocationTimeout = communicator.GetPropertyAsInt(property);
                if (invocationTimeout is int invocationTimeoutValue)
                {
                    if (invocationTimeoutValue < 1 && invocationTimeoutValue != -1)
                    {
                        throw new InvalidConfigurationException(
                            $"invalid value for property `{property}': `{invocationTimeoutValue}'");
                    }
                }

                locatorInfo = communicator.GetLocatorInfo(
                    communicator.GetPropertyAsProxy($"{propertyPrefix}.Locator", ILocatorPrx.Factory), encoding);

                property = $"{propertyPrefix}.LocatorCacheTimeout";
                locatorCacheTimeout = communicator.GetPropertyAsInt(property);
                if (locatorCacheTimeout is int locatorCacheTimeoutValue)
                {
                    if (locatorCacheTimeoutValue < -1)
                    {
                        throw new InvalidConfigurationException(
                            $"invalid value for property `{property}': `{locatorCacheTimeoutValue}'");
                    }
                }

                preferNonSecure = communicator.GetPropertyAsBool($"{propertyPrefix}.PreferNonSecure");

                property = $"{propertyPrefix}.Router";
                IRouterPrx? router = communicator.GetPropertyAsProxy(property, IRouterPrx.Factory);
                if (router != null)
                {
                    if (propertyPrefix.EndsWith(".Router", StringComparison.Ordinal))
                    {
                        throw new InvalidConfigurationException(
                            $"`{property}={communicator.GetProperty(property)}': cannot set a router on a router");
                    }
                    routerInfo = communicator.GetRouterInfo(router);
                }
            }

            return new Reference(adapterId: adapterId,
                                 cacheConnection: cacheConnection ?? true,
                                 collocationOptimized: collocOptimized ?? communicator.DefaultCollocationOptimized,
                                 communicator: communicator,
                                 compress: null,
                                 connectionId: "",
                                 connectionTimeout: null,
                                 context: context ?? communicator.DefaultContext,
                                 encoding: encoding,
                                 endpointSelection: endpointSelection ?? communicator.DefaultEndpointSelection,
                                 endpoints: endpoints,
                                 facet: facet,
                                 identity: identity,
                                 invocationMode: invocationMode,
                                 invocationTimeout: invocationTimeout ?? communicator.DefaultInvocationTimeout,
                                 locatorCacheTimeout: locatorCacheTimeout ?? communicator.DefaultLocatorCacheTimeout,
                                 locatorInfo:
                                    locatorInfo ?? communicator.GetLocatorInfo(communicator.DefaultLocator, encoding),
                                 preferNonSecure: preferNonSecure ?? communicator.DefaultPreferNonSecure,
                                 protocol: protocol,
                                 routerInfo: routerInfo ?? communicator.GetRouterInfo(communicator.DefaultRouter));
        }

        // Constructor for routable references, not bound to a connection
        private Reference(string adapterId,
                          bool cacheConnection,
                          bool collocationOptimized,
                          Communicator communicator,
                          bool? compress,
                          string connectionId,
                          int? connectionTimeout,
                          IReadOnlyDictionary<string, string> context, // already a copy provided by Ice
                          Encoding encoding,
                          EndpointSelectionType endpointSelection,
                          IReadOnlyList<Endpoint> endpoints, // already a copy provided by Ice
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

        // Constructor for fixed references.
        private Reference(Communicator communicator,
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

            _fixedConnection.ThrowException(); // Throw in case our connection is already destroyed.
            _requestHandler = new ConnectionRequestHandler(_fixedConnection,
                                                           Communicator.OverrideCompress ?? compress ?? false);
        }

        private IEnumerable<Endpoint> ApplyOverrides(IReadOnlyList<Endpoint> endpoints)
        {
            Debug.Assert(!IsFixed);
            return endpoints.Select(endpoint =>
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
                });
        }

        private IReadOnlyList<Endpoint> FilterEndpoints(IReadOnlyList<Endpoint> allEndpoints)
        {
            Debug.Assert(!IsFixed);

            IEnumerable<Endpoint> filteredEndpoints = allEndpoints.Where(endpoint =>
            {
                // Filter out opaque endpoints
                if (endpoint is OpaqueEndpoint)
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

            if (EndpointSelection == EndpointSelectionType.Random)
            {
                // Shuffle the filtered endpoints using _rand
                Endpoint[] array = filteredEndpoints.ToArray();
                lock (_rand)
                {
                    for (int i = 0; i < array.Length - 1; ++i)
                    {
                        int r = _rand.Next(array.Length - i) + i;
                        Debug.Assert(r >= i && r < array.Length);
                        if (r != i)
                        {
                            Endpoint tmp = array[i];
                            array[i] = array[r];
                            array[r] = tmp;
                        }
                    }
                }

                if (!PreferNonSecure)
                {
                    // We're done
                    return array;
                }
                else
                {
                    filteredEndpoints = array;
                }
            }

            if (PreferNonSecure)
            {
                // It's just a preference: we can fallback to secure endpoints.
                filteredEndpoints = filteredEndpoints.OrderBy(endpoint => endpoint.IsSecure);
            }
            // else, already filtered out

            return filteredEndpoints.ToArray();
        }

        private void GetConnectionNoRouterInfo(IGetConnectionCallback callback)
        {
            Debug.Assert(!IsFixed);
            if (Endpoints.Count > 0)
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

        // TODO: refactor this class
        private sealed class RouterEndpointsCallback : RouterInfo.IGetClientEndpointsCallback
        {
            internal RouterEndpointsCallback(Reference ir, IGetConnectionCallback cb)
            {
                Debug.Assert(!ir.IsFixed);
                _ir = ir;
                _cb = cb;
            }

            public void SetEndpoints(IReadOnlyList<Endpoint> endpts)
            {
                if (endpts.Count > 0)
                {
                    _ir.CreateConnection(_ir.ApplyOverrides(endpts).ToArray(), _cb);
                }
                else
                {
                    _ir.GetConnectionNoRouterInfo(_cb);
                }
            }

            public void SetException(System.Exception ex) => _cb.SetException(ex);

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

            public void SetEndpoints(IReadOnlyList<Endpoint> endpoints, bool cached)
            {
                if (endpoints.Count == 0)
                {
                    _cb.SetException(new NoEndpointException(_ir.ToString()));
                    return;
                }

                _ir.CreateConnection(_ir.ApplyOverrides(endpoints).ToArray(), new ConnectionCallback(_ir, _cb, cached));
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
            internal CreateConnectionCallback(Reference rr, IReadOnlyList<Endpoint>? endpoints,
                 IGetConnectionCallback cb)
            {
                Debug.Assert(!rr.IsFixed);
                _rr = rr;
                _endpoints = endpoints;
                if (_endpoints != null)
                {
                    Debug.Assert(_endpoints.Count > 1); // at least 2 endpoints, and we always skip the first one
                    _endpointEnumerator = _endpoints.GetEnumerator();
                    _hasMoreEndpoints = _endpointEnumerator.MoveNext();
                    Debug.Assert(_hasMoreEndpoints);
                    _hasMoreEndpoints = _endpointEnumerator.MoveNext();
                    Debug.Assert(_hasMoreEndpoints);
                }
                else
                {
                    _hasMoreEndpoints = false;
                }
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
                    connection.Adapter = _rr.RouterInfo.Adapter;
                }
                _callback.SetConnection(connection, compress);
            }

            public void SetException(System.Exception ex)
            {
                if (_exception == null)
                {
                    _exception = ex;
                }

                if (!_hasMoreEndpoints)
                {
                    _callback.SetException(_exception);
                    return;
                }
                Debug.Assert(_endpointEnumerator != null);

                var endpoint = new Endpoint[] { _endpointEnumerator.Current };
                _hasMoreEndpoints = _endpointEnumerator.MoveNext();

                _rr.Communicator.OutgoingConnectionFactory().Create(endpoint, _hasMoreEndpoints,
                    _rr.EndpointSelection, this);
            }

            private readonly Reference _rr;
            private readonly IReadOnlyList<Endpoint>? _endpoints;
            private readonly IEnumerator<Endpoint>? _endpointEnumerator;
            private bool _hasMoreEndpoints;
            private readonly IGetConnectionCallback _callback;
            private System.Exception? _exception = null;
        }
    }
}
