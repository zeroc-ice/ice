// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Globalization;
using System.Text;

namespace Ice.Internal;

public abstract class Reference : IEquatable<Reference>
{
    /// <summary>
    /// Represents the invocation mode of a proxy.
    /// </summary>
    public enum Mode
    {
        /// <summary>
        /// A two-way invocation that always expects a response.
        /// </summary>
        ModeTwoway,

        /// <summary>
        /// A one-way invocation that does not expect a response and completes as soon as the request is sent.
        /// </summary>
        ModeOneway,

        /// <summary>
        /// A batch one-way invocation that allows grouping several one-way invocations into a single batch.
        /// </summary>
        ModeBatchOneway,

        /// <summary>
        /// A datagram invocation that does not expect a response and is sent using UDP.
        /// </summary>
        ModeDatagram,

        /// <summary>
        /// A batch datagram invocation that allows grouping several datagram invocations into a single batch.
        /// </summary>
        ModeBatchDatagram
    }

    public interface GetConnectionCallback
    {
        void setConnection(Ice.ConnectionI connection, bool compress);

        void setException(Ice.LocalException ex);
    }

    internal abstract BatchRequestQueue batchRequestQueue { get; }

    internal bool isBatch => _mode is Mode.ModeBatchOneway or Mode.ModeBatchDatagram;

    internal bool isTwoway => _mode is Mode.ModeTwoway;

    public Mode getMode()
    {
        return _mode;
    }

    public bool getSecure()
    {
        return secure_;
    }

    public Ice.ProtocolVersion getProtocol()
    {
        return _protocol;
    }

    public Ice.EncodingVersion getEncoding()
    {
        return _encoding;
    }

    public Ice.Identity getIdentity()
    {
        return _identity;
    }

    public string getFacet()
    {
        return _facet;
    }

    public Instance getInstance()
    {
        return _instance;
    }

    public Dictionary<string, string> getContext()
    {
        return _context;
    }

    public TimeSpan
    getInvocationTimeout()
    {
        return _invocationTimeout;
    }

    public bool?
    getCompress()
    {
        return _compress;
    }

    public Ice.Communicator getCommunicator()
    {
        return _communicator;
    }

    public abstract EndpointI[] getEndpoints();

    public abstract string getAdapterId();

    public abstract LocatorInfo getLocatorInfo();

    public abstract RouterInfo getRouterInfo();

    public abstract bool getCollocationOptimized();

    public abstract bool getCacheConnection();

    public abstract bool getPreferSecure();

    public abstract Ice.EndpointSelectionType getEndpointSelection();

    public abstract TimeSpan getLocatorCacheTimeout();

    public abstract string getConnectionId();

    public abstract ThreadPool getThreadPool();

    public abstract Connection getConnection();

    //
    // The change* methods (here and in derived classes) create
    // a new reference based on the existing one, with the
    // corresponding value changed.
    //
    public Reference changeContext(Dictionary<string, string> newContext)
    {
        if (newContext == null)
        {
            newContext = _emptyContext;
        }
        Reference r = _instance.referenceFactory().copy(this);
        if (newContext.Count == 0)
        {
            r._context = _emptyContext;
        }
        else
        {
            r._context = new Dictionary<string, string>(newContext);
        }
        return r;
    }

    public virtual Reference changeMode(Mode newMode)
    {
        Reference r = _instance.referenceFactory().copy(this);
        r._mode = newMode;
        return r;
    }

    public Reference changeSecure(bool newSecure)
    {
        Reference r = _instance.referenceFactory().copy(this);
        r.secure_ = newSecure;
        return r;
    }

    public Reference changeIdentity(Ice.Identity newIdentity)
    {
        Reference r = _instance.referenceFactory().copy(this);
        // Identity is a reference type, therefore we make a copy of newIdentity.
        r._identity = newIdentity with { };
        return r;
    }

    public Reference changeFacet(string newFacet)
    {
        Reference r = _instance.referenceFactory().copy(this);
        r._facet = newFacet;
        return r;
    }

    public Reference changeInvocationTimeout(TimeSpan newTimeout)
    {
        Reference r = _instance.referenceFactory().copy(this);
        r._invocationTimeout = newTimeout;
        return r;
    }

    public virtual Reference changeEncoding(Ice.EncodingVersion newEncoding)
    {
        Reference r = _instance.referenceFactory().copy(this);
        r._encoding = newEncoding;
        return r;
    }

    public virtual Reference changeCompress(bool newCompress)
    {
        Reference r = _instance.referenceFactory().copy(this);
        r._compress = newCompress;
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

    public abstract Reference changeLocatorCacheTimeout(TimeSpan newTimeout);

    public abstract Reference changeConnectionId(string connectionId);

    public abstract Reference changeConnection(Ice.ConnectionI connection);

    // Gets the effective compression setting, taking into account the override.
    public bool? getCompressOverride()
    {
        DefaultsAndOverrides defaultsAndOverrides = getInstance().defaultsAndOverrides();
        return defaultsAndOverrides.overrideCompress ?? _compress;
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
        if (_facet.Length == 0)
        {
            s.writeStringSeq(null);
        }
        else
        {
            string[] facetPath = { _facet };
            s.writeStringSeq(facetPath);
        }

        s.writeByte((byte)_mode);

        s.writeBool(secure_);

        if (!s.getEncoding().Equals(Ice.Util.Encoding_1_0))
        {
            ProtocolVersion.ice_write(s, _protocol);
            EncodingVersion.ice_write(s, _encoding);
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

        Ice.ToStringMode toStringMode = _instance.toStringMode();

        //
        // If the encoded identity string contains characters which
        // the reference parser uses as separators, then we enclose
        // the identity string in quotes.
        //
        string id = Ice.Util.identityToString(_identity, toStringMode);
        if (Ice.UtilInternal.StringUtil.findFirstOf(id, " :@") != -1)
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
            string fs = Ice.UtilInternal.StringUtil.escapeString(_facet, "", toStringMode);
            if (Ice.UtilInternal.StringUtil.findFirstOf(fs, " :@") != -1)
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
            case Mode.ModeTwoway:
            {
                // Don't print the default mode.
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

        if (secure_)
        {
            s.Append(" -s");
        }

        if (_protocol != Ice.Util.Protocol_1_0)
        {
            // We print the protocol unless it's 1.0.
            s.Append(" -p ");
            s.Append(Ice.Util.protocolVersionToString(_protocol));
        }

        // We print the encoding if it's not 1.1 or if Ice.Default.EncodingVersion is set to something other than 1.1.
        if (_encoding != Ice.Util.Encoding_1_1 ||
            _instance.defaultsAndOverrides().defaultEncoding != Ice.Util.Encoding_1_1)
        {
            s.Append(" -e ");
            s.Append(Ice.Util.encodingVersionToString(_encoding));
        }

        return s.ToString();

        // Derived class writes the remainder of the string.
    }

    public abstract Dictionary<string, string> toProperty(string prefix);

    internal abstract RequestHandler getRequestHandler();

    public static bool operator ==(Reference lhs, Reference rhs) => lhs is null ? rhs is null : lhs.Equals(rhs);

    public static bool operator !=(Reference lhs, Reference rhs) => !(lhs == rhs);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(_mode);
        hash.Add(secure_);
        hash.Add(_identity);
        hash.Add(_context.Count); // we only hash the count, not the contents
        hash.Add(_facet);
        hash.Add(_compress);
        // We don't hash protocol and encoding; they are usually "1.0" and "1.1" respectively.
        hash.Add(_invocationTimeout);
        return hash.ToHashCode();
    }

    public virtual bool Equals(Reference other)
    {
        // The derived class checks ReferenceEquals and guarantees other is not null.
        Debug.Assert(other is not null);

        return _mode == other._mode &&
            secure_ == other.secure_ &&
            _identity == other._identity &&
            _context.DictionaryEqual(other._context) &&
            _facet == other._facet &&
            _compress == other._compress &&
            _protocol == other._protocol &&
            _encoding == other._encoding &&
            _invocationTimeout == other._invocationTimeout;
    }

    public override bool Equals(object obj) => Equals(obj as Reference);

    public virtual Reference Clone() => (Reference)MemberwiseClone();

    private static Dictionary<string, string> _emptyContext = new Dictionary<string, string>();

    private protected Instance _instance;
    private Ice.Communicator _communicator;

    private Mode _mode;
    private Ice.Identity _identity;
    private Dictionary<string, string> _context;
    private string _facet;
    protected bool secure_;
    private Ice.ProtocolVersion _protocol;
    private Ice.EncodingVersion _encoding;
    private TimeSpan _invocationTimeout;
    private bool? _compress;

    protected Reference(
        Instance instance,
        Ice.Communicator communicator,
        Ice.Identity identity,
        string facet,
        Mode mode,
        bool secure,
        bool? compress,
        Ice.ProtocolVersion protocol,
        Ice.EncodingVersion encoding,
        TimeSpan invocationTimeout,
        Dictionary<string, string> context)
    {
        // Validate string arguments.
        Debug.Assert(facet != null);

        _instance = instance;
        _communicator = communicator;
        _mode = mode;
        _identity = identity;
        _context = context != null ? new Dictionary<string, string>(context) : _emptyContext;
        _facet = facet;
        _protocol = protocol;
        _encoding = encoding;
        _invocationTimeout = invocationTimeout;
        secure_ = secure;
        _compress = compress;
    }

    protected static Random rand_ = new Random(unchecked((int)DateTime.Now.Ticks));
}

public class FixedReference : Reference
{
    internal override BatchRequestQueue batchRequestQueue => _fixedConnection.getBatchRequestQueue();

    public FixedReference(
        Instance instance,
        Ice.Communicator communicator,
        Ice.Identity identity,
        string facet,
        Mode mode,
        bool secure,
        bool? compress,
        Ice.ProtocolVersion protocol,
        Ice.EncodingVersion encoding,
        Ice.ConnectionI connection,
        TimeSpan invocationTimeout,
        Dictionary<string, string> context)
    : base(instance, communicator, identity, facet, mode, secure, compress, protocol, encoding, invocationTimeout, context)
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

    public override TimeSpan getLocatorCacheTimeout()
    {
        return TimeSpan.Zero;
    }

    public override string getConnectionId()
    {
        return "";
    }

    public override ThreadPool getThreadPool()
    {
        return _fixedConnection.getThreadPool();
    }

    public override Connection getConnection()
    {
        return _fixedConnection;
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

    public override Reference changePreferSecure(bool newPreferSecure)
    {
        throw new Ice.FixedProxyException();
    }

    public override Reference changeEndpointSelection(Ice.EndpointSelectionType newType)
    {
        throw new Ice.FixedProxyException();
    }

    public override Reference changeLocatorCacheTimeout(TimeSpan newTimeout)
    {
        throw new Ice.FixedProxyException();
    }

    public override Reference changeConnectionId(string connectionId)
    {
        throw new Ice.FixedProxyException();
    }

    public override Reference changeConnection(Ice.ConnectionI connection)
    {
        if (_fixedConnection == connection)
        {
            return this;
        }
        FixedReference r = (FixedReference)getInstance().referenceFactory().copy(this);
        r._fixedConnection = connection;
        return r;
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

    public override Dictionary<string, string> toProperty(string prefix)
    {
        throw new Ice.FixedProxyException();
    }

    internal override RequestHandler getRequestHandler()
    {
        // We need to perform all these checks here and not in the constructor because changeConnection() clones then
        // sets the connection.

        switch (getMode())
        {
            case Mode.ModeTwoway:
            case Mode.ModeOneway:
            case Mode.ModeBatchOneway:
            {
                if (_fixedConnection.endpoint().datagram())
                {
                    throw new NoEndpointException(new ObjectPrxHelper(this));
                }
                break;
            }

            case Mode.ModeDatagram:
            case Mode.ModeBatchDatagram:
            {
                if (!_fixedConnection.endpoint().datagram())
                {
                    throw new NoEndpointException(new ObjectPrxHelper(this));
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
        if (defaultsAndOverrides.overrideSecure is not null)
        {
            secure = defaultsAndOverrides.overrideSecure.Value;
        }
        else
        {
            secure = getSecure();
        }
        if (secure && !_fixedConnection.endpoint().secure())
        {
            throw new NoEndpointException(new ObjectPrxHelper(this));
        }

        _fixedConnection.throwException(); // Throw in case our connection is already destroyed.

        bool compress = defaultsAndOverrides.overrideCompress ?? getCompress() ?? false;
        return new FixedRequestHandler(this, _fixedConnection, compress);
    }

    public override bool Equals(Reference other)
    {
        if (ReferenceEquals(this, other))
        {
            return true;
        }
        FixedReference rhs = other as FixedReference;
        return rhs is not null && base.Equals(rhs) && _fixedConnection.Equals(rhs._fixedConnection);
    }

    private static EndpointI[] _emptyEndpoints = [];
    private Ice.ConnectionI _fixedConnection;
}

public class RoutableReference : Reference
{
    internal override BatchRequestQueue batchRequestQueue => _batchRequestQueue;

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

    public override TimeSpan getLocatorCacheTimeout()
    {
        return _locatorCacheTimeout;
    }

    public override string getConnectionId()
    {
        return _connectionId;
    }

    public override Reference changeMode(Mode newMode)
    {
        Reference r = base.changeMode(newMode);
        ((RoutableReference)r).setBatchRequestQueue();
        return r;
    }

    public override ThreadPool getThreadPool()
    {
        return getInstance().clientThreadPool();
    }

    public override Connection getConnection()
    {
        return null;
    }

    public override Reference changeEncoding(Ice.EncodingVersion newEncoding)
    {
        RoutableReference r = (RoutableReference)base.changeEncoding(newEncoding);
        if (r != this)
        {
            LocatorInfo locInfo = r._locatorInfo;
            if (locInfo != null && !locInfo.getLocator().ice_getEncodingVersion().Equals(newEncoding))
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
        if (r != this && _endpoints.Length > 0) // Also override the compress flag on the endpoints if it was updated
        {
            EndpointI[] newEndpoints = new EndpointI[_endpoints.Length];
            for (int i = 0; i < _endpoints.Length; i++)
            {
                newEndpoints[i] = _endpoints[i].compress(newCompress);
            }
            r._endpoints = newEndpoints;
        }
        return r;
    }

    public override Reference changeEndpoints(EndpointI[] newEndpoints)
    {
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._endpoints = newEndpoints;
        r._adapterId = "";
        r.applyOverrides(ref r._endpoints);
        return r;
    }

    public override Reference changeAdapterId(string newAdapterId)
    {
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._adapterId = newAdapterId;
        r._endpoints = _emptyEndpoints;
        return r;
    }

    public override Reference changeLocator(Ice.LocatorPrx newLocator)
    {
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._locatorInfo = getInstance().locatorManager().get(newLocator);
        return r;
    }

    public override Reference changeRouter(Ice.RouterPrx newRouter)
    {
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._routerInfo = getInstance().routerManager().get(newRouter);
        return r;
    }

    public override Reference changeCollocationOptimized(bool newCollocationOptimized)
    {
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._collocationOptimized = newCollocationOptimized;
        return r;
    }

    public override Reference changeCacheConnection(bool newCache)
    {
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._cacheConnection = newCache;
        return r;
    }

    public override Reference changePreferSecure(bool newPreferSecure)
    {
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._preferSecure = newPreferSecure;
        return r;
    }

    public override Reference changeEndpointSelection(Ice.EndpointSelectionType newType)
    {
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._endpointSelection = newType;
        return r;
    }

    public override Reference changeLocatorCacheTimeout(TimeSpan newTimeout)
    {
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._locatorCacheTimeout = newTimeout;
        return r;
    }

    public override Reference changeConnectionId(string connectionId)
    {
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._connectionId = connectionId;
        if (_endpoints.Length > 0)
        {
            EndpointI[] newEndpoints = new EndpointI[_endpoints.Length];
            for (int i = 0; i < _endpoints.Length; i++)
            {
                newEndpoints[i] = _endpoints[i].connectionId(connectionId);
            }
            r._endpoints = newEndpoints;
        }
        return r;
    }

    public override Reference changeConnection(Ice.ConnectionI connection)
    {
        return new FixedReference(
            getInstance(),
            getCommunicator(),
            getIdentity(),
            getFacet(),
            getMode(),
            getSecure(),
            getCompress(),
            getProtocol(),
            getEncoding(),
            connection,
            getInvocationTimeout(),
            getContext());
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
        if (_endpoints.Length > 0)
        {
            Debug.Assert(_adapterId.Length == 0);
            foreach (EndpointI endpoint in _endpoints)
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
        else if (_adapterId.Length > 0)
        {
            s.Append(" @ ");

            //
            // If the encoded adapter id string contains characters which
            // the reference parser uses as separators, then we enclose
            // the adapter id string in quotes.
            //
            string a = Ice.UtilInternal.StringUtil.escapeString(_adapterId, null, getInstance().toStringMode());
            if (Ice.UtilInternal.StringUtil.findFirstOf(a, " :@") != -1)
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
        Dictionary<string, string> properties = new Dictionary<string, string>
        {
            [prefix] = ToString(),
            [prefix + ".CollocationOptimized"] = _collocationOptimized ? "1" : "0",
            [prefix + ".ConnectionCached"] = _cacheConnection ? "1" : "0",
            [prefix + ".PreferSecure"] = _preferSecure ? "1" : "0",
            [prefix + ".EndpointSelection"] =
                   _endpointSelection == Ice.EndpointSelectionType.Random ? "Random" : "Ordered",
            [prefix + ".LocatorCacheTimeout"] = _locatorCacheTimeout.TotalSeconds.ToString(CultureInfo.InvariantCulture),
            [prefix + ".InvocationTimeout"] = getInvocationTimeout().TotalMilliseconds.ToString(CultureInfo.InvariantCulture)
        };

        if (_routerInfo != null)
        {
            Ice.ObjectPrxHelperBase h = (Ice.ObjectPrxHelperBase)_routerInfo.getRouter();
            Dictionary<string, string> routerProperties = h.iceReference().toProperty(prefix + ".Router");
            foreach (KeyValuePair<string, string> entry in routerProperties)
            {
                properties[entry.Key] = entry.Value;
            }
        }

        if (_locatorInfo != null)
        {
            Ice.ObjectPrxHelperBase h = (Ice.ObjectPrxHelperBase)_locatorInfo.getLocator();
            Dictionary<string, string> locatorProperties = h.iceReference().toProperty(prefix + ".Locator");
            foreach (KeyValuePair<string, string> entry in locatorProperties)
            {
                properties[entry.Key] = entry.Value;
            }
        }

        return properties;
    }

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(base.GetHashCode());
        hash.Add(_adapterId);
        foreach (var endpoint in _endpoints)
        {
            hash.Add(endpoint);
        }
        return hash.ToHashCode();
    }

    public override bool Equals(Reference other)
    {
        if (ReferenceEquals(this, other))
        {
            return true;
        }
        var rhs = other as RoutableReference;

        return rhs is not null &&
            base.Equals(rhs) &&
            _locatorInfo == rhs._locatorInfo &&
            _routerInfo == rhs._routerInfo &&
            _collocationOptimized == rhs._collocationOptimized &&
            _cacheConnection == rhs._cacheConnection &&
            _preferSecure == rhs._preferSecure &&
            _endpointSelection == rhs._endpointSelection &&
            _locatorCacheTimeout == rhs._locatorCacheTimeout &&
            _connectionId == rhs._connectionId &&
            _adapterId == rhs._adapterId &&
            _endpoints.SequenceEqual(rhs._endpoints);
    }

    public override Reference Clone()
    {
        var clone = (RoutableReference)MemberwiseClone();
        // Each reference gets its own batch request queue.
        clone.setBatchRequestQueue();
        return clone;
    }

    private sealed class RouterEndpointsCallback : RouterInfo.GetClientEndpointsCallback
    {
        internal RouterEndpointsCallback(RoutableReference ir, GetConnectionCallback cb)
        {
            _ir = ir;
            _cb = cb;
        }

        public void setEndpoints(EndpointI[] endpoints)
        {
            if (endpoints.Length > 0)
            {
                _ir.applyOverrides(ref endpoints);
                _ir.createConnection(endpoints, _cb);
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

    internal override RequestHandler getRequestHandler()
    {
        if (_collocationOptimized)
        {
            if (_instance.objectAdapterFactory().findObjectAdapter(this) is ObjectAdapter adapter)
            {
                return new CollocatedRequestHandler(this, adapter);
            }
        }

        var handler = new ConnectRequestHandler(this);
        getConnection(handler);
        return handler;
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
                _cb.setException(new NoEndpointException(new ObjectPrxHelper(_ir)));
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
            catch (NoEndpointException ex)
            {
                _cb.setException(ex); // No need to retry if there's no endpoints.
            }
            catch (Ice.LocalException ex)
            {
                Debug.Assert(_ir._locatorInfo != null);
                _ir._locatorInfo.clearCache(_ir);
                if (_cached)
                {
                    TraceLevels traceLevels = _ir.getInstance().traceLevels();
                    if (traceLevels.retry >= 2)
                    {
                        string s = "connection to cached endpoints failed\n" +
                                   "removing endpoints from cache and trying again\n" + ex;
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
            callback.setException(new NoEndpointException(new ObjectPrxHelper(this)));
        }
    }

    public RoutableReference(
        Instance instance,
        Ice.Communicator communicator,
        Ice.Identity identity,
        string facet,
        Mode mode,
        bool secure,
        bool? compress,
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
        TimeSpan locatorCacheTimeout,
        TimeSpan invocationTimeout,
        Dictionary<string, string> context)
    : base(
        instance,
        communicator,
        identity,
        facet,
        mode,
        secure,
        compress,
        protocol,
        encoding,
        invocationTimeout,
        context)
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

        if (_endpoints == null)
        {
            _endpoints = _emptyEndpoints;
        }

        if (_adapterId == null)
        {
            _adapterId = "";
        }
        setBatchRequestQueue();

        Debug.Assert(_adapterId.Length == 0 || _endpoints.Length == 0);
    }

    protected void applyOverrides(ref EndpointI[] endpoints)
    {
        for (int i = 0; i < endpoints.Length; ++i)
        {
            endpoints[i] = endpoints[i].connectionId(_connectionId);
            bool? compress = getCompress();
            if (compress is not null)
            {
                endpoints[i] = endpoints[i].compress(compress.Value);
            }
        }
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
            case Mode.ModeTwoway:
            case Mode.ModeOneway:
            case Mode.ModeBatchOneway:
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

            case Mode.ModeDatagram:
            case Mode.ModeBatchDatagram:
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
            case Ice.EndpointSelectionType.Random:
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
        if (overrides.overrideSecure ?? getSecure())
        {
            List<EndpointI> tmp = new List<EndpointI>();
            foreach (EndpointI endpoint in endpoints)
            {
                if (endpoint.secure())
                {
                    tmp.Add(endpoint);
                }
            }
            endpoints = tmp;
        }
        else if (getPreferSecure())
        {
            UtilInternal.Collections.Sort(ref endpoints, _preferSecureEndpointComparator);
        }
        else
        {
            UtilInternal.Collections.Sort(ref endpoints, _preferNonSecureEndpointComparator);
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
            if (_rr._routerInfo != null && _rr._routerInfo.getAdapter() != null)
            {
                connection.setAdapter(_rr._routerInfo.getAdapter());
            }
            _callback.setConnection(connection, compress);
        }

        public void setException(Ice.LocalException ex)
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
            var endpointList = new List<EndpointI> { _endpoints[_i] };
            _rr.getInstance().outgoingConnectionFactory().create(endpointList, more, this);
        }

        private RoutableReference _rr;
        private EndpointI[] _endpoints;
        private GetConnectionCallback _callback;
        private int _i;
        private Ice.LocalException _exception;
    }

    protected void createConnection(EndpointI[] allEndpoints, GetConnectionCallback callback)
    {
        EndpointI[] endpoints = filterEndpoints(allEndpoints);
        if (endpoints.Length == 0)
        {
            callback.setException(new NoEndpointException(new ObjectPrxHelper(this)));
            return;
        }

        //
        // Finally, create the connection.
        //
        OutgoingConnectionFactory factory = getInstance().outgoingConnectionFactory();
        if (getCacheConnection() || endpoints.Length == 1)
        {
            //
            // Get an existing connection or create one if there's no
            // existing connection to one of the given endpoints.
            //
            factory.create(endpoints.ToList(), false, new CreateConnectionCallback(this, null, callback));
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

            factory.create(
                new List<EndpointI> { endpoints[0] },
                true,
                new CreateConnectionCallback(this, endpoints, callback));
        }
    }

    // Sets or resets _batchRequestQueue based on _mode.
    private void setBatchRequestQueue()
    {
        _batchRequestQueue =
            isBatch ? new BatchRequestQueue(getInstance(), getMode() == Mode.ModeBatchDatagram) : null;
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
    private static EndpointI[] _emptyEndpoints = [];

    private BatchRequestQueue _batchRequestQueue;

    private EndpointI[] _endpoints;
    private string _adapterId;
    private LocatorInfo _locatorInfo; // Null if no locator is used.
    private RouterInfo _routerInfo; // Null if no router is used.
    private bool _collocationOptimized;
    private bool _cacheConnection;
    private bool _preferSecure;
    private Ice.EndpointSelectionType _endpointSelection;
    private TimeSpan _locatorCacheTimeout;

    private string _connectionId = "";
}
