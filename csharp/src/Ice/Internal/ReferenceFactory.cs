// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Text;
using System.Text.RegularExpressions;

namespace Ice.Internal;

internal class ReferenceFactory
{
    internal Reference
    create(Ice.Identity ident, string facet, Reference tmpl, EndpointI[] endpoints)
    {
        if (ident.name.Length == 0 && ident.category.Length == 0)
        {
            return null;
        }

        return create(
            ident,
            facet,
            tmpl.getMode(),
            tmpl.getSecure(),
            tmpl.getProtocol(),
            tmpl.getEncoding(),
            endpoints,
            null,
            null);
    }

    internal Reference
    create(Ice.Identity ident, string facet, Reference tmpl, string adapterId)
    {
        if (ident.name.Length == 0 && ident.category.Length == 0)
        {
            return null;
        }

        //
        // Create new reference
        //
        return create(
            ident,
            facet,
            tmpl.getMode(),
            tmpl.getSecure(),
            tmpl.getProtocol(),
            tmpl.getEncoding(),
            null,
            adapterId,
            null);
    }

    internal Reference create(Ice.Identity ident, Ice.ConnectionI connection)
    {
        if (ident.name.Length == 0 && ident.category.Length == 0)
        {
            return null;
        }

        //
        // Create new reference
        //
        return new FixedReference(
            _instance,
            _communicator,
            ident,
            "", // Facet
            connection.endpoint().datagram() ? Reference.Mode.ModeDatagram : Reference.Mode.ModeTwoway,
            connection.endpoint().secure(),
            compress: null,
            Ice.Util.Protocol_1_0,
            _instance.defaultsAndOverrides().defaultEncoding,
            connection,
            TimeSpan.FromMilliseconds(-1),
            null);
    }

    internal Reference copy(Reference r)
    {
        Ice.Identity ident = r.getIdentity();
        if (ident.name.Length == 0 && ident.category.Length == 0)
        {
            return null;
        }
        return r.Clone();
    }

    internal Reference create(string s, string propertyPrefix)
    {
        if (s.Length == 0)
        {
            return null;
        }

        const string delim = " \t\n\r";

        int beg;
        int end = 0;

        beg = Ice.UtilInternal.StringUtil.findFirstNotOf(s, delim, end);
        if (beg == -1)
        {
            throw new ParseException($"no non-whitespace characters found in proxy string '{s}'");
        }

        end = Ice.UtilInternal.StringUtil.checkQuote(s, beg);

        //
        // Extract the identity, which may be enclosed in single
        // or double quotation marks.
        //
        string idstr;
        if (end == -1)
        {
            throw new ParseException($"mismatched quotes around identity in proxy string '{s}'");
        }
        else if (end == 0)
        {
            end = Ice.UtilInternal.StringUtil.findFirstOf(s, delim + ":@", beg);
            if (end == -1)
            {
                end = s.Length;
            }
            idstr = s[beg..end];
        }
        else
        {
            beg++; // Skip leading quote
            idstr = s[beg..end];
            end++; // Skip trailing quote
        }

        if (beg == end)
        {
            throw new ParseException($"no identity in proxy string '{s}'");
        }

        //
        // Parsing the identity may raise ParseException.
        //
        Ice.Identity ident = Ice.Util.stringToIdentity(idstr);

        if (ident.name.Length == 0)
        {
            //
            // An identity with an empty name and a non-empty
            // category is illegal.
            //
            if (ident.category.Length > 0)
            {
                throw new ParseException("The category of a null Ice object identity must be empty.");
            }
            //
            // Treat a stringified proxy containing two double
            // quotes ("") the same as an empty string, i.e.,
            // a null proxy, but only if nothing follows the
            // quotes.
            //
            else if (Ice.UtilInternal.StringUtil.findFirstNotOf(s, delim, end) != -1)
            {
                throw new ParseException($"invalid characters after identity in proxy string '{s}'");
            }
            else
            {
                return null;
            }
        }

        string facet = "";
        Reference.Mode mode = Reference.Mode.ModeTwoway;
        bool secure = false;
        Ice.EncodingVersion encoding = _instance.defaultsAndOverrides().defaultEncoding;
        Ice.ProtocolVersion protocol = Ice.Util.Protocol_1_0;
        while (true)
        {
            beg = Ice.UtilInternal.StringUtil.findFirstNotOf(s, delim, end);
            if (beg == -1)
            {
                break;
            }

            if (s[beg] == ':' || s[beg] == '@')
            {
                break;
            }

            end = Ice.UtilInternal.StringUtil.findFirstOf(s, delim + ":@", beg);
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
                throw new ParseException($"expected a proxy option but found '{option}' in proxy string '{s}'");
            }

            //
            // Check for the presence of an option argument. The
            // argument may be enclosed in single or double
            // quotation marks.
            //
            string argument = null;
            int argumentBeg = Ice.UtilInternal.StringUtil.findFirstNotOf(s, delim, end);
            if (argumentBeg != -1)
            {
                char ch = s[argumentBeg];
                if (ch != '@' && ch != ':' && ch != '-')
                {
                    beg = argumentBeg;
                    end = Ice.UtilInternal.StringUtil.checkQuote(s, beg);
                    if (end == -1)
                    {
                        throw new ParseException(
                            $"mismatched quotes around value for {option} option in proxy string '{s}'");
                    }
                    else if (end == 0)
                    {
                        end = Ice.UtilInternal.StringUtil.findFirstOf(s, delim + ":@", beg);
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

            //
            // If any new options are added here,
            // Ice.Internal::Reference::toString() and its derived classes must be updated as well.
            //
            switch (option[1])
            {
                case 'f':
                {
                    if (argument == null)
                    {
                        throw new ParseException($"no argument provided for -f option in proxy string '{s}'");
                    }

                    try
                    {
                        facet = Ice.UtilInternal.StringUtil.unescapeString(argument, 0, argument.Length, "");
                    }
                    catch (ArgumentException argEx)
                    {
                        throw new ParseException($"invalid facet in proxy string '{s}'", argEx);
                    }
                    break;
                }

                case 't':
                {
                    if (argument != null)
                    {
                        throw new ParseException(
                            $"unexpected argument '{argument}' provided for -t option in proxy string '{s}'");
                    }
                    mode = Reference.Mode.ModeTwoway;
                    break;
                }

                case 'o':
                {
                    if (argument != null)
                    {
                        throw new ParseException(
                            $"unexpected argument '{argument}' provided for -o option in proxy string '{s}'");
                    }
                    mode = Reference.Mode.ModeOneway;
                    break;
                }

                case 'O':
                {
                    if (argument != null)
                    {
                        throw new ParseException(
                            $"unexpected argument '{argument}' provided for -O option in proxy string '{s}'");
                    }
                    mode = Reference.Mode.ModeBatchOneway;
                    break;
                }

                case 'd':
                {
                    if (argument != null)
                    {
                        throw new ParseException(
                            $"unexpected argument '{argument}' provided for -d option in proxy string '{s}'");
                    }
                    mode = Reference.Mode.ModeDatagram;
                    break;
                }

                case 'D':
                {
                    if (argument != null)
                    {
                        throw new ParseException(
                            $"unexpected argument '{argument}' provided for -D option in proxy string '{s}'");
                    }
                    mode = Reference.Mode.ModeBatchDatagram;
                    break;
                }

                case 's':
                {
                    if (argument != null)
                    {
                        throw new ParseException(
                            $"unexpected argument '{argument}' provided for -s option in proxy string '{s}'");
                    }
                    secure = true;
                    break;
                }

                case 'e':
                {
                    if (argument == null)
                    {
                        throw new ParseException($"no argument provided for -e option in proxy string '{s}'");
                    }

                    try
                    {
                        encoding = Ice.Util.stringToEncodingVersion(argument);
                    }
                    catch (ParseException e)
                    {
                        throw new ParseException($"invalid encoding version '{argument}' in proxy string '{s}'", e);
                    }
                    break;
                }

                case 'p':
                {
                    if (argument == null)
                    {
                        throw new ParseException($"no argument provided for -p option in proxy string '{s}'");
                    }

                    try
                    {
                        protocol = Ice.Util.stringToProtocolVersion(argument);
                    }
                    catch (ParseException e)
                    {
                        throw new ParseException($"invalid protocol version '{argument}' in proxy string '{s}'", e);
                    }
                    break;
                }

                default:
                {
                    throw new ParseException($"unknown option '{option}' in proxy string '{s}'");
                }
            }
        }

        if (beg == -1)
        {
            return create(ident, facet, mode, secure, protocol, encoding, null, null, propertyPrefix);
        }

        var endpoints = new List<EndpointI>();

        if (s[beg] == ':')
        {
            var unknownEndpoints = new List<string>();
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
                EndpointI endp = _instance.endpointFactoryManager().create(es, false);
                if (endp != null)
                {
                    endpoints.Add(endp);
                }
                else
                {
                    unknownEndpoints.Add(es);
                }
            }
            if (endpoints.Count == 0)
            {
                Debug.Assert(unknownEndpoints.Count > 0);
                throw new ParseException($"invalid endpoint '{unknownEndpoints[0]}' in '{s}'");
            }
            else if (unknownEndpoints.Count != 0 &&
                    _instance.initializationData().properties.getIcePropertyAsInt("Ice.Warn.Endpoints") > 0)
            {
                var msg = new StringBuilder("Proxy contains unknown endpoints:");
                int sz = unknownEndpoints.Count;
                for (int idx = 0; idx < sz; ++idx)
                {
                    msg.Append(" `");
                    msg.Append(unknownEndpoints[idx]);
                    msg.Append('\'');
                }
                _instance.initializationData().logger.warning(msg.ToString());
            }

            EndpointI[] ep = endpoints.ToArray();
            return create(ident, facet, mode, secure, protocol, encoding, ep, null, propertyPrefix);
        }
        else if (s[beg] == '@')
        {
            beg = Ice.UtilInternal.StringUtil.findFirstNotOf(s, delim, beg + 1);
            if (beg == -1)
            {
                throw new ParseException($"missing adapter ID in proxy string '{s}'");
            }

            end = Ice.UtilInternal.StringUtil.checkQuote(s, beg);

            string adapterstr;
            if (end == -1)
            {
                throw new ParseException($"mismatched quotes around adapter ID in proxy string '{s}'");
            }
            else if (end == 0)
            {
                end = Ice.UtilInternal.StringUtil.findFirstOf(s, delim, beg);
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

            if (end != s.Length && Ice.UtilInternal.StringUtil.findFirstNotOf(s, delim, end) != -1)
            {
                throw new ParseException($"invalid characters after adapter ID in proxy string '{s}'");
            }

            string adapter;
            try
            {
                adapter = Ice.UtilInternal.StringUtil.unescapeString(adapterstr, 0, adapterstr.Length, "");
            }
            catch (ArgumentException argEx)
            {
                throw new ParseException($"invalid adapter ID in proxy string '{s}'", argEx);
            }
            if (adapter.Length == 0)
            {
                throw new ParseException($"empty adapter ID in proxy string '{s}'");
            }
            return create(ident, facet, mode, secure, protocol, encoding, null, adapter, propertyPrefix);
        }

        throw new ParseException($"malformed proxy string '{s}'");
    }

    internal Reference create(Ice.Identity ident, Ice.InputStream s)
    {
        //
        // Don't read the identity here. Operations calling this
        // constructor read the identity, and pass it as a parameter.
        //

        if (ident.name.Length == 0 && ident.category.Length == 0)
        {
            return null;
        }

        //
        // For compatibility with the old FacetPath.
        //
        string[] facetPath = s.readStringSeq();
        string facet;
        if (facetPath.Length > 0)
        {
            if (facetPath.Length > 1)
            {
                throw new MarshalException($"Received invalid facet path with {facetPath.Length} elements.");
            }
            facet = facetPath[0];
        }
        else
        {
            facet = "";
        }

        int mode = s.readByte();
        if (mode < 0 || mode > (int)Reference.Mode.ModeBatchDatagram)
        {
            throw new MarshalException($"Received invalid proxy mode {mode}");
        }

        bool secure = s.readBool();

        Ice.ProtocolVersion protocol;
        Ice.EncodingVersion encoding;
        if (!s.getEncoding().Equals(Ice.Util.Encoding_1_0))
        {
            protocol = new Ice.ProtocolVersion(s);
            encoding = new Ice.EncodingVersion(s);
        }
        else
        {
            protocol = Ice.Util.Protocol_1_0;
            encoding = Ice.Util.Encoding_1_0;
        }

        EndpointI[] endpoints = null;
        string adapterId = "";

        int sz = s.readSize();
        if (sz > 0)
        {
            endpoints = new EndpointI[sz];
            for (int i = 0; i < sz; i++)
            {
                endpoints[i] = _instance.endpointFactoryManager().read(s);
            }
        }
        else
        {
            adapterId = s.readString();
        }

        return create(ident, facet, (Reference.Mode)mode, secure, protocol, encoding, endpoints, adapterId, null);
    }

    internal ReferenceFactory setDefaultRouter(Ice.RouterPrx defaultRouter)
    {
        if (_defaultRouter == null ? defaultRouter == null : _defaultRouter.Equals(defaultRouter))
        {
            return this;
        }

        var factory = new ReferenceFactory(_instance, _communicator);
        factory._defaultLocator = _defaultLocator;
        factory._defaultRouter = defaultRouter;
        return factory;
    }

    internal Ice.RouterPrx getDefaultRouter() => _defaultRouter;

    internal ReferenceFactory setDefaultLocator(Ice.LocatorPrx defaultLocator)
    {
        if (_defaultLocator == null ? defaultLocator == null : _defaultLocator.Equals(defaultLocator))
        {
            return this;
        }

        var factory = new ReferenceFactory(_instance, _communicator);
        factory._defaultLocator = defaultLocator;
        factory._defaultRouter = _defaultRouter;
        return factory;
    }

    internal Ice.LocatorPrx getDefaultLocator() => _defaultLocator;

    //
    // Only for use by Instance
    //
    internal ReferenceFactory(Instance instance, Ice.Communicator communicator)
    {
        _instance = instance;
        _communicator = communicator;
    }

    private Reference create(
        Ice.Identity ident,
        string facet,
        Reference.Mode mode,
        bool secure,
        Ice.ProtocolVersion protocol,
        Ice.EncodingVersion encoding,
        EndpointI[] endpoints,
        string adapterId,
        string propertyPrefix)
    {
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();

        //
        // Default local proxy options.
        //
        LocatorInfo locatorInfo = null;
        if (_defaultLocator != null)
        {
            if (!((Ice.ObjectPrxHelperBase)_defaultLocator).iceReference().getEncoding().Equals(encoding))
            {
                locatorInfo = _instance.locatorManager().get(
                    (Ice.LocatorPrx)_defaultLocator.ice_encodingVersion(encoding));
            }
            else
            {
                locatorInfo = _instance.locatorManager().get(_defaultLocator);
            }
        }
        RouterInfo routerInfo = _instance.routerManager().get(_defaultRouter);
        bool collocOptimized = defaultsAndOverrides.defaultCollocationOptimization;
        bool cacheConnection = true;
        bool preferSecure = defaultsAndOverrides.defaultPreferSecure;
        Ice.EndpointSelectionType endpointSelection = defaultsAndOverrides.defaultEndpointSelection;
        TimeSpan locatorCacheTimeout = defaultsAndOverrides.defaultLocatorCacheTimeout;
        TimeSpan invocationTimeout = defaultsAndOverrides.defaultInvocationTimeout;
        Dictionary<string, string> context = null;

        //
        // Override the defaults with the proxy properties if a property prefix is defined.
        //
        if (propertyPrefix != null && propertyPrefix.Length > 0)
        {
            Properties properties = _instance.initializationData().properties;

            Properties.validatePropertiesWithPrefix(propertyPrefix, properties, PropertyNames.ProxyProps);

            string property;

            property = propertyPrefix + ".Locator";
            Ice.LocatorPrx locator = Ice.LocatorPrxHelper.uncheckedCast(_communicator.propertyToProxy(property));
            if (locator != null)
            {
                if (!((Ice.ObjectPrxHelperBase)locator).iceReference().getEncoding().Equals(encoding))
                {
                    locatorInfo = _instance.locatorManager().get(
                        (Ice.LocatorPrx)locator.ice_encodingVersion(encoding));
                }
                else
                {
                    locatorInfo = _instance.locatorManager().get(locator);
                }
            }

            property = propertyPrefix + ".Router";
            Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(_communicator.propertyToProxy(property));
            if (router != null)
            {
                if (propertyPrefix.EndsWith(".Router", StringComparison.Ordinal))
                {
                    string s = "`" + property + "=" + properties.getProperty(property) +
                        "': cannot set a router on a router; setting ignored";
                    _instance.initializationData().logger.warning(s);
                }
                else
                {
                    routerInfo = _instance.routerManager().get(router);
                }
            }

            property = propertyPrefix + ".CollocationOptimized";
            collocOptimized = properties.getPropertyAsIntWithDefault(property, collocOptimized ? 1 : 0) > 0;

            property = propertyPrefix + ".ConnectionCached";
            cacheConnection = properties.getPropertyAsIntWithDefault(property, cacheConnection ? 1 : 0) > 0;

            property = propertyPrefix + ".PreferSecure";
            preferSecure = properties.getPropertyAsIntWithDefault(property, preferSecure ? 1 : 0) > 0;

            property = propertyPrefix + ".EndpointSelection";
            if (properties.getProperty(property).Length > 0)
            {
                string type = properties.getProperty(property);
                if (type == "Random")
                {
                    endpointSelection = Ice.EndpointSelectionType.Random;
                }
                else if (type == "Ordered")
                {
                    endpointSelection = Ice.EndpointSelectionType.Ordered;
                }
                else
                {
                    throw new ParseException(
                        $"illegal value '{type}' in property '{property}'; expected 'Random' or 'Ordered'");
                }
            }

            property = propertyPrefix + ".LocatorCacheTimeout";
            locatorCacheTimeout = TimeSpan.FromSeconds(
                properties.getPropertyAsIntWithDefault(property, (int)locatorCacheTimeout.TotalSeconds));

            property = propertyPrefix + ".InvocationTimeout";
            invocationTimeout = TimeSpan.FromMilliseconds(
                properties.getPropertyAsIntWithDefault(property, (int)invocationTimeout.TotalMilliseconds));

            property = propertyPrefix + ".Context.";
            Dictionary<string, string> contexts = properties.getPropertiesForPrefix(property);
            if (contexts.Count != 0)
            {
                context = new Dictionary<string, string>();
                foreach (KeyValuePair<string, string> e in contexts)
                {
                    context.Add(e.Key[property.Length..], e.Value);
                }
            }
        }

        //
        // Create new reference
        //
        return new RoutableReference(
            _instance,
            _communicator,
            ident,
            facet,
            mode,
            secure,
            compress: null,
            protocol,
            encoding,
            endpoints,
            adapterId,
            locatorInfo,
            routerInfo,
            collocOptimized,
            cacheConnection,
            preferSecure,
            endpointSelection,
            locatorCacheTimeout,
            invocationTimeout,
            context);
    }

    private readonly Instance _instance;
    private readonly Ice.Communicator _communicator;
    private Ice.RouterPrx _defaultRouter;
    private Ice.LocatorPrx _defaultLocator;
}
