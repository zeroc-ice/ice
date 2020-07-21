//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Globalization;
using System.Threading;
using System.Threading.Tasks;
using System.Linq;

namespace ZeroC.Ice
{
    /// <summary>Reference is an Ice-internal but publicly visible class. Each Ice proxy has single get-only property,
    /// IceReference, which holds the reference associated with this proxy. Reference represents the untyped
    /// implementation of a proxy. Multiples proxies that point to the same Ice object and share the same proxy
    /// options can share the same Reference object, even if these proxies have different types.</summary>
    public sealed class Reference : IEquatable<Reference>
    {
        internal string AdapterId { get; }
        internal Communicator Communicator { get; }
        internal string ConnectionId { get; }
        internal IReadOnlyDictionary<string, string> Context { get; }
        internal Encoding Encoding { get; }
        internal EndpointSelectionType EndpointSelection { get; }
        internal IReadOnlyList<Endpoint> Endpoints { get; }
        internal string Facet { get; }
        internal Identity Identity { get; }
        internal InvocationMode InvocationMode { get; }
        internal int InvocationTimeout { get; }
        internal bool IsConnectionCached;
        internal bool IsFixed => _fixedConnection != null;
        internal bool IsIndirect => !IsFixed && Endpoints.Count == 0;
        internal bool IsWellKnown => !IsFixed && Endpoints.Count == 0 && AdapterId.Length == 0;
        internal TimeSpan LocatorCacheTimeout { get; }
        internal LocatorInfo? LocatorInfo { get; }
        internal bool PreferNonSecure { get; }
        internal Protocol Protocol { get; }
        internal RouterInfo? RouterInfo { get; }
        private readonly Connection? _fixedConnection;
        private int _hashCode = 0;
        private volatile IRequestHandler? _requestHandler; // readonly when IsFixed is true

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

        /// <summary>Creates a reference from a string and a communicator. This an Ice-internal publicly visible static
        /// method.</summary>
        public static Reference Parse(string s, Communicator communicator, string? propertyPrefix = null)
        {
            string proxyString = s.Trim();
            if (proxyString.Length == 0)
            {
                throw new FormatException("empty string is invalid");
            }

            return UriParser.IsUri(proxyString) ? ParseUri(proxyString, communicator, propertyPrefix) :
                ParseOldFormat(proxyString, communicator, propertyPrefix);
        }

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
                if (EndpointSelection != other.EndpointSelection)
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
                hash.Add(Protocol);

                if (IsFixed)
                {
                    hash.Add(_fixedConnection);
                }
                else
                {
                    hash.Add(AdapterId);
                    hash.Add(ConnectionId);
                    foreach (Endpoint e in Endpoints)
                    {
                        hash.Add(e);
                    }
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
                s.Append(Protocol.GetName());
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

        // TODO: move to private section of the file (not done for ease of review)
        private static Reference ParseOldFormat(string s, Communicator communicator, string? propertyPrefix)
        {
            // TODO: rework this implementation

            int beg = 0;
            int end = 0;

            const string delim = " \t\n\r";

            // Extract the identity, which may be enclosed in single or double quotation marks.
            string identityString;
            end = StringUtil.CheckQuote(s, beg);
            if (end == -1)
            {
                throw new FormatException($"mismatched quotes around identity in `{s}'");
            }
            else if (end == 0)
            {
                end = StringUtil.FindFirstOf(s, delim + ":@", beg);
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
            Protocol protocol = communicator.DefaultProtocol;
            string adapterId;

            while (true)
            {
                beg = StringUtil.FindFirstNotOf(s, delim, end);
                if (beg == -1)
                {
                    break;
                }

                if (s[beg] == ':' || s[beg] == '@')
                {
                    break;
                }

                end = StringUtil.FindFirstOf(s, delim + ":@", beg);
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
                int argumentBeg = StringUtil.FindFirstNotOf(s, delim, end);
                if (argumentBeg != -1)
                {
                    char ch = s[argumentBeg];
                    if (ch != '@' && ch != ':' && ch != '-')
                    {
                        beg = argumentBeg;
                        end = StringUtil.CheckQuote(s, beg);
                        if (end == -1)
                        {
                            throw new FormatException($"mismatched quotes around value for {option} option in `{s}'");
                        }
                        else if (end == 0)
                        {
                            end = StringUtil.FindFirstOf(s, delim + ":@", beg);
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
                        facet = StringUtil.UnescapeString(argument, 0, argument.Length, "");
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
                return Create(adapterId: "", communicator, encoding, endpoints: ImmutableArray<Endpoint>.Empty, facet,
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
                    endpoints.Add(Endpoint.Parse(es, protocol, communicator, false));
                }

                Debug.Assert(endpoints.Count > 0);
                return Create(adapterId: "", communicator, encoding, endpoints, facet, identity, invocationMode,
                    propertyPrefix, protocol);
            }
            else if (s[beg] == '@')
            {
                beg = StringUtil.FindFirstNotOf(s, delim, beg + 1);
                if (beg == -1)
                {
                    throw new FormatException($"missing adapter id in `{s}'");
                }

                string adapterstr;
                end = StringUtil.CheckQuote(s, beg);
                if (end == -1)
                {
                    throw new FormatException($"mismatched quotes around adapter id in `{s}'");
                }
                else if (end == 0)
                {
                    end = StringUtil.FindFirstOf(s, delim, beg);
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

                if (end != s.Length && StringUtil.FindFirstNotOf(s, delim, end) != -1)
                {
                    throw new FormatException(
                        $"invalid trailing characters after `{s.Substring(0, end + 1)}' in `{s}'");
                }

                adapterId = StringUtil.UnescapeString(adapterstr, 0, adapterstr.Length, "");

                if (adapterId.Length == 0)
                {
                    throw new FormatException($"empty adapter id in `{s}'");
                }

                return Create(adapterId, communicator, encoding, endpoints: Array.Empty<Endpoint>(), facet, identity,
                    invocationMode, propertyPrefix, protocol);
            }

            throw new FormatException($"malformed proxy `{s}'");
        }

        private static Reference ParseUri(string uriString, Communicator communicator, string? propertyPrefix)
        {
            (Encoding encoding,
             IReadOnlyList<Endpoint> endpoints,
             string facet,
             InvocationMode invocationMode,
             List<string> path,
             Protocol protocol) = UriParser.Parse(uriString, oaEndpoints: false, communicator);

            string adapterId = "";
            Identity identity;

            switch (path.Count)
            {
                case 0:
                    // TODO: should we add a default identity "Default" or "Root" or "Main"?
                    throw new FormatException($"missing identity in proxy `{uriString}'");
                case 1:
                    identity = new Identity(path[0], "");
                    break;
                case 2:
                    identity = new Identity(path[1], path[0]);
                    break;
                case 3:
                    adapterId = path[0];
                    identity = new Identity(path[2], path[1]);
                    // TODO: temporary, should be only for ice1
                    if (endpoints.Count > 0 && adapterId.Length > 0)
                    {
                        throw new FormatException($"direct proxy `{uriString}' cannot include a location");
                    }
                    break;
                default:
                    // TODO: should we convert adapterId/location into a sequence<string>?
                    throw new FormatException($"too many segments in path of proxy `{uriString}'");
            }

            if (identity.Name.Length == 0)
            {
                throw new FormatException($"invalid identity with empty name in proxy `{uriString}'");
            }

            return Create(adapterId,
                          communicator,
                          encoding,
                          endpoints,
                          facet,
                          identity,
                          invocationMode,
                          propertyPrefix,
                          protocol);
        }

        /// <summary>Reads a reference from the input stream.</summary>
        /// <param name="istr">The input stream to read from.</param>
        /// <param name="communicator">The communicator.</param>
        /// <returns>The reference read from the stream (can be null).</returns>
        internal static Reference? Read(InputStream istr, Communicator communicator)
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
            if (protocol == 0)
            {
                throw new InvalidDataException($"received proxy with protocol set to 0");
            }

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
                    endpoints[i] = istr.ReadEndpoint(protocol, communicator);
                }
            }
            else
            {
                endpoints = Array.Empty<Endpoint>();
                adapterId = istr.ReadString();
            }

            return new Reference(adapterId,
                                 communicator,
                                 encoding,
                                 endpoints,
                                 facet,
                                 identity,
                                 invocationMode: (InvocationMode)mode,
                                 protocol);
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
                   communicator: communicator,
                   connectionId: "",
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

        internal int CheckRetryAfterException(Exception ex, bool sent, bool idempotent, ref int cnt)
        {
            // TODO: revisit retry logic

            //
            // If the request was sent and is not idempotent, the operation might be retried only if the exception
            // is an ObjectNotExistException or ConnectionClosedByPeerException. Otherwise, it can't be retried.
            //
            if (sent && !idempotent && !(ex is ObjectNotExistException) && !(ex is ConnectionClosedByPeerException))
            {
                throw ExceptionUtil.Throw(ex);
            }

            //
            // If it's a fixed proxy, retrying isn't useful as the proxy is tied to
            // the connection and the request will fail with the exception.
            //
            if (IsFixed)
            {
                throw ExceptionUtil.Throw(ex);
            }

            if (ex is ObjectNotExistException one)
            {
                RouterInfo? ri = RouterInfo;
                if (ri != null && one.Operation.Equals("ice_add_proxy"))
                {
                    //
                    // If we have a router, an ObjectNotExistException with an
                    // operation name "ice_add_proxy" indicates to the client
                    // that the router isn't aware of the proxy (for example,
                    // because it was evicted by the router). In this case, we
                    // must *always* retry, so that the missing proxy is added
                    // to the router.
                    //

                    ri.ClearCache(this);

                    if (Communicator.TraceLevels.Retry >= 1)
                    {
                        Communicator.Logger.Trace(Communicator.TraceLevels.RetryCategory,
                            $"retrying operation call to add proxy to router\n {ex}");
                    }
                    return 0; // We must always retry, so we don't look at the retry count.
                }
                else if (IsIndirect)
                {
                    //
                    // We retry ObjectNotExistException if the reference is
                    // indirect.
                    //

                    if (IsWellKnown)
                    {
                        LocatorInfo?.ClearCache(this);
                    }
                }
                else
                {
                    //
                    // For all other cases, we don't retry ObjectNotExistException.
                    //
                    throw ExceptionUtil.Throw(ex);
                }
            }

            //
            // Don't retry if the communicator or object adapter are disposed,
            // or the connection is manually closed.
            //
            if (ex is ObjectDisposedException || ex is ConnectionClosedLocallyException)
            {
                throw ExceptionUtil.Throw(ex);
            }

            ++cnt;
            Debug.Assert(cnt > 0);

            int interval;
            if (cnt == (Communicator.RetryIntervals.Length + 1) && ex is ConnectionClosedByPeerException)
            {
                //
                // A connection closed exception is always retried at least once, even if the retry
                // limit is reached.
                //
                interval = 0;
            }
            else if (cnt > Communicator.RetryIntervals.Length)
            {
                if (Communicator.TraceLevels.Retry >= 1)
                {
                    Communicator.Logger.Trace(Communicator.TraceLevels.RetryCategory,
                        $"cannot retry operation call because retry limit has been exceeded\n{ex}");
                }
                throw ExceptionUtil.Throw(ex);
            }
            else
            {
                interval = Communicator.RetryIntervals[cnt - 1];
            }

            if (Communicator.TraceLevels.Retry >= 1)
            {
                string s = "retrying operation call";
                if (interval > 0)
                {
                    s += " in " + interval + "ms";
                }
                s += $" because of exception\n{ex}";
                Communicator.Logger.Trace(Communicator.TraceLevels.RetryCategory, s);
            }

            return interval;
        }

        internal void ClearRequestHandler(IRequestHandler handler)
        {
            Debug.Assert(!IsFixed);
            Interlocked.CompareExchange(ref _requestHandler, null, handler);
        }

        internal Reference Clone(string? adapterId = null,
                                 bool? cacheConnection = null,
                                 bool clearLocator = false,
                                 bool clearRouter = false,
                                 string? connectionId = null,
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
                                 TimeSpan? locatorCacheTimeout = null,
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
                if (connectionId != null)
                {
                    throw new ArgumentException("cannot change the connection ID of a fixed proxy",
                        nameof(connectionId));
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
                                          context?.ToImmutableDictionary() ?? Context,
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

                if (locatorCacheTimeout != null &&
                    locatorCacheTimeout < TimeSpan.Zero && locatorCacheTimeout != Timeout.InfiniteTimeSpan)
                {
                    throw new ArgumentException(
                        $"invalid {nameof(locatorCacheTimeout)}: {locatorCacheTimeout}", nameof(locatorCacheTimeout));
                }

                if (adapterId != null && endpoints != null)
                {
                    throw new ArgumentException($"cannot set both {nameof(adapterId)} and {nameof(endpoints)}");
                }

                IReadOnlyList<Endpoint>? newEndpoints = null;

                if (adapterId != null)
                {
                    newEndpoints = ImmutableArray<Endpoint>.Empty; // make sure the clone's endpoints are empty
                }
                else if (endpoints != null)
                {
                    adapterId = ""; // make sure the clone's adapterID is empty
                    newEndpoints = endpoints.ToList(); // make a copy
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

                var clone = new Reference(adapterId ?? AdapterId,
                                      cacheConnection ?? IsConnectionCached,
                                      Communicator,
                                      connectionId ?? ConnectionId,
                                      context?.ToImmutableDictionary() ?? Context,
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

        internal IReadOnlyDictionary<string, string> CurrentContext()
        {
            IReadOnlyDictionary<string, string> context;

            if (Context.Count == 0)
            {
                context = Communicator.CurrentContext;
            }
            else if (Communicator.CurrentContext.Count == 0)
            {
                context = Context;
            }
            else
            {
                var combinedContext = new Dictionary<string, string>(Communicator.CurrentContext);
                foreach ((string key, string value) in Context)
                {
                    combinedContext[key] = value;  // the proxy Context entry prevails.
                }
                context = combinedContext;
            }
            return context;
        }

        internal Connection? GetCachedConnection() => _requestHandler?.GetConnection();

        internal async ValueTask<IRequestHandler> GetConnectionRequestHandlerAsync(CancellationToken cancel)
        {
            Debug.Assert(!IsFixed);

            // Get the endpoints
            IReadOnlyList<Endpoint> endpoints = ImmutableArray<Endpoint>.Empty;
            bool cached = false;
            if (RouterInfo != null)
            {
                // Get the router client endpoints if a router is configured
                endpoints = await RouterInfo.GetClientEndpointsAsync(cancel).ConfigureAwait(false);
            }

            if (endpoints.Count == 0)
            {
                // Get the proxy's endpoint or query the locator to get endpoints
                if (Endpoints.Count > 0)
                {
                    endpoints = Endpoints;
                }
                else if (LocatorInfo != null)
                {
                    (endpoints, cached) =
                        await LocatorInfo.GetEndpointsAsync(this, LocatorCacheTimeout, cancel).ConfigureAwait(false);
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
                filteredEndpoints = filteredEndpoints.Shuffle();
            }

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

            //
            // Finally, create the connection.
            //
            try
            {
                OutgoingConnectionFactory factory = Communicator.OutgoingConnectionFactory;
                Connection? connection = null;
                if (IsConnectionCached)
                {
                    // Get an existing connection or create one if there's no existing connection to one of
                    // the given endpoints.
                    connection = await factory.CreateAsync(endpoints,
                                                           false,
                                                           EndpointSelection,
                                                           ConnectionId).ConfigureAwait(false);
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
                            connection = await factory.CreateAsync(new Endpoint[] { endpoint },
                                                                   endpoint != lastEndpoint,
                                                                   EndpointSelection,
                                                                   ConnectionId).ConfigureAwait(false);
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
                    await RouterInfo.AddProxyAsync(IObjectPrx.Factory(this));

                    //
                    // Set the object adapter for this router (if any) on the new connection, so that callbacks from
                    // the router can be received over this new connection.
                    //
                    if (RouterInfo.Adapter != null)
                    {
                        connection.Adapter = RouterInfo.Adapter;
                    }
                }

                // If it's an Ice1 proxy, we check if the connection matches an endpoint with the compression flag
                // enabled. If it's the case, we'll use compression.
                bool compress = false;
                if (Protocol == Protocol.Ice1)
                {
                    // Use compression if the first matching endpoint uses compression. We make sure to compare
                    // endpoints with the same timeout value to ignore the timeout.
                    compress = endpoints.First(endpoint => connection.Endpoints.Exists(connectionEndpoint =>
                        endpoint.NewTimeout(connectionEndpoint.Timeout).Equals(connectionEndpoint))).HasCompressionFlag;
                }
                return new ConnectionRequestHandler(connection, compress);
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
                        Communicator.Logger.Trace(traceLevels.RetryCategory, "connection to cached endpoints failed\n" +
                            $"removing endpoints from cache and trying again\n{ex}");
                    }
                    return await GetConnectionRequestHandlerAsync(cancel);
                }
                throw;
            }
        }

        internal async ValueTask<IRequestHandler> GetRequestHandlerAsync(CancellationToken cancel)
        {
            IRequestHandler? handler = _requestHandler;
            if (handler == null)
            {
                Debug.Assert(!IsFixed);

                if (InvocationMode != InvocationMode.Datagram)
                {
                    // If the invocation mode is not datagram, we first check if the target is colocated.
                    ObjectAdapter? adapter = Communicator.FindObjectAdapter(this);
                    if (adapter != null)
                    {
                        handler = new CollocatedRequestHandler(this, adapter);
                    }
                }

                if (handler == null)
                {
                    handler = await GetConnectionRequestHandlerAsync(cancel).ConfigureAwait(false);
                }

                if (IsConnectionCached)
                {
                    _requestHandler = handler;
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
                [prefix + ".ConnectionCached"] = IsConnectionCached ? "1" : "0",
                [prefix + ".EndpointSelection"] = EndpointSelection.ToString(),
                [prefix + ".InvocationTimeout"] = InvocationTimeout.ToString(CultureInfo.InvariantCulture),
                [prefix + ".LocatorCacheTimeout"] = LocatorCacheTimeout.ToPropertyString(),
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
            IReadOnlyDictionary<string, string>? context = null;
            EndpointSelectionType? endpointSelection = null;
            int? invocationTimeout = null;
            TimeSpan? locatorCacheTimeout = null;
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
                if (invocationTimeout != null && invocationTimeout < 1 && invocationTimeout != -1)
                {
                    throw new InvalidConfigurationException(
                        $"invalid value for property `{property}': `{invocationTimeout}'");
                }

                locatorInfo = communicator.GetLocatorInfo(
                    communicator.GetPropertyAsProxy($"{propertyPrefix}.Locator", ILocatorPrx.Factory), encoding);

                property = $"{propertyPrefix}.LocatorCacheTimeout";
                locatorCacheTimeout = communicator.GetPropertyAsTimeSpan(property);
                if (locatorCacheTimeout != null &&
                    locatorCacheTimeout < TimeSpan.Zero &&
                    locatorCacheTimeout != Timeout.InfiniteTimeSpan)
                {
                    throw new InvalidConfigurationException(
                        $"invalid value for property `{property}': `{locatorCacheTimeout}'");
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
                                 communicator: communicator,
                                 connectionId: "",
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
                          Communicator communicator,
                          string connectionId,
                          IReadOnlyDictionary<string, string> context, // already a copy provided by Ice
                          Encoding encoding,
                          EndpointSelectionType endpointSelection,
                          IReadOnlyList<Endpoint> endpoints, // already a copy provided by Ice
                          string facet,
                          Identity identity,
                          InvocationMode invocationMode,
                          int invocationTimeout,
                          TimeSpan locatorCacheTimeout,
                          LocatorInfo? locatorInfo,
                          bool preferNonSecure,
                          Protocol protocol,
                          RouterInfo? routerInfo)
        {
            AdapterId = adapterId;
            Communicator = communicator;
            ConnectionId = connectionId;
            Context = context;
            Encoding = encoding;
            EndpointSelection = endpointSelection;
            Endpoints = endpoints;
            Facet = facet;
            Identity = identity;
            InvocationMode = invocationMode;
            InvocationTimeout = invocationTimeout;
            IsConnectionCached = cacheConnection;
            LocatorCacheTimeout = locatorCacheTimeout;
            LocatorInfo = locatorInfo;
            PreferNonSecure = preferNonSecure;
            Protocol = protocol;
            RouterInfo = routerInfo;
        }

        // Constructor for fixed references.
        private Reference(Communicator communicator,
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
            ConnectionId = "";
            Context = context;
            Encoding = encoding;
            EndpointSelection = EndpointSelectionType.Random;
            Endpoints = Array.Empty<Endpoint>();
            Facet = facet;
            Identity = identity;
            InvocationMode = invocationMode;
            InvocationTimeout = invocationTimeout;
            IsConnectionCached = false;
            LocatorCacheTimeout = TimeSpan.Zero;
            LocatorInfo = null;
            PreferNonSecure = false;
            Protocol = fixedConnection.Endpoint.Protocol;
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
            _requestHandler = new ConnectionRequestHandler(_fixedConnection, false);
        }
    }
}
