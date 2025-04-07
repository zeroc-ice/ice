// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.time.Duration;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

final class ReferenceFactory {
    public Reference create(Identity ident, String facet, Reference tmpl, EndpointI[] endpoints) {
        if (ident.name.isEmpty() && ident.category.isEmpty()) {
            return null;
        }

        return create(
                ident,
                facet,
                tmpl.getMode(),
                tmpl.getSecure(),
                tmpl.getCompress(),
                tmpl.getProtocol(),
                tmpl.getEncoding(),
                endpoints,
                null,
                null);
    }

    public Reference create(Identity ident, String facet, Reference tmpl, String adapterId) {
        if (ident.name.isEmpty() && ident.category.isEmpty()) {
            return null;
        }

        return create(
                ident,
                facet,
                tmpl.getMode(),
                tmpl.getSecure(),
                tmpl.getCompress(),
                tmpl.getProtocol(),
                tmpl.getEncoding(),
                null,
                adapterId,
                null);
    }

    public Reference create(Identity ident, ConnectionI fixedConnection) {
        if (ident.name.isEmpty() && ident.category.isEmpty()) {
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
                fixedConnection.endpoint().datagram()
                        ? Reference.ModeDatagram
                        : Reference.ModeTwoway,
                fixedConnection.endpoint().secure(),
                Optional.empty(),
                Util.Protocol_1_0,
                _instance.defaultsAndOverrides().defaultEncoding,
                fixedConnection,
                Duration.ofMillis(-1),
                null);
    }

    public Reference copy(Reference r) {
        Identity ident = r.getIdentity();
        if (ident.name.isEmpty() && ident.category.isEmpty()) {
            return null;
        }
        return r.clone();
    }

    public Reference create(String s, String propertyPrefix) {
        if (s == null || s.isEmpty()) {
            return null;
        }

        final String delim = " \t\n\r";

        int beg;
        int end = 0;

        beg = StringUtil.findFirstNotOf(s, delim, end);
        if (beg == -1) {
            throw new ParseException(
                    "no non-whitespace characters found in proxy string '" + s + "'");
        }

        //
        // Extract the identity, which may be enclosed in single
        // or double quotation marks.
        //
        String idstr = null;
        end = StringUtil.checkQuote(s, beg);
        if (end == -1) {
            throw new ParseException(
                    "mismatched quotes around identity in proxy string '" + s + "'");
        } else if (end == 0) {
            end = StringUtil.findFirstOf(s, delim + ":@", beg);
            if (end == -1) {
                end = s.length();
            }
            idstr = s.substring(beg, end);
        } else {
            beg++; // Skip leading quote
            idstr = s.substring(beg, end);
            end++; // Skip trailing quote
        }

        if (beg == end) {
            throw new ParseException("no identity in proxy string '" + s + "'");
        }

        //
        // Parsing the identity may raise ParseException.
        //
        Identity ident = Util.stringToIdentity(idstr);

        if (ident.name.isEmpty()) {
            //
            // An identity with an empty name and a non-empty
            // category is illegal.
            //
            if (ident.category.length() > 0) {
                throw new ParseException(
                        "The category of a null Ice object identity must be empty.");
            }
            //
            // Treat a stringified proxy containing two double
            // quotes ("") the same as an empty string, i.e.,
            // a null proxy, but only if nothing follows the
            // quotes.
            //
            else if (StringUtil.findFirstNotOf(s, delim, end) != -1) {
                throw new ParseException(
                        "invalid characters after identity in proxy string '" + s + "'");
            } else {
                return null;
            }
        }

        String facet = "";
        int mode = Reference.ModeTwoway;
        boolean secure = false;
        EncodingVersion encoding = _instance.defaultsAndOverrides().defaultEncoding;
        ProtocolVersion protocol = Util.Protocol_1_0;
        String adapter = "";

        while (true) {
            beg = StringUtil.findFirstNotOf(s, delim, end);
            if (beg == -1) {
                break;
            }

            if (s.charAt(beg) == ':' || s.charAt(beg) == '@') {
                break;
            }

            end = StringUtil.findFirstOf(s, delim + ":@", beg);
            if (end == -1) {
                end = s.length();
            }

            if (beg == end) {
                break;
            }

            String option = s.substring(beg, end);
            if (option.length() != 2 || option.charAt(0) != '-') {
                throw new ParseException(
                        "expected a proxy option but found '"
                                + option
                                + "' in proxy string '"
                                + s
                                + "'");
            }

            //
            // Check for the presence of an option argument. The
            // argument may be enclosed in single or double
            // quotation marks.
            //
            String argument = null;
            int argumentBeg = StringUtil.findFirstNotOf(s, delim, end);
            if (argumentBeg != -1) {
                final char ch = s.charAt(argumentBeg);
                if (ch != '@' && ch != ':' && ch != '-') {
                    beg = argumentBeg;
                    end = StringUtil.checkQuote(s, beg);
                    if (end == -1) {
                        throw new ParseException(
                                "mismatched quotes around value for "
                                        + option
                                        + " option in proxy string '"
                                        + s
                                        + "'");
                    } else if (end == 0) {
                        end = StringUtil.findFirstOf(s, delim + ":@", beg);
                        if (end == -1) {
                            end = s.length();
                        }
                        argument = s.substring(beg, end);
                    } else {
                        beg++; // Skip leading quote
                        argument = s.substring(beg, end);
                        end++; // Skip trailing quote
                    }
                }
            }

            //
            // If any new options are added here,
            // com.zeroc.Ice.Reference.toString() and its derived classes must be updated as well.
            //
            switch (option.charAt(1)) {
                case 'f':
                    {
                        if (argument == null) {
                            throw new ParseException(
                                    "no argument provided for -f option in proxy string '"
                                            + s
                                            + "'");
                        }

                        try {
                            facet = StringUtil.unescapeString(argument, 0, argument.length(), "");
                        } catch (IllegalArgumentException ex) {
                            throw new ParseException(
                                    "invalid facet in proxy string '" + s + "'", ex);
                        }

                        break;
                    }

                case 't':
                    {
                        if (argument != null) {
                            throw new ParseException(
                                    "unexpected argument '"
                                            + argument
                                            + "' provided for -t option in proxy string '"
                                            + s
                                            + "'");
                        }
                        mode = Reference.ModeTwoway;
                        break;
                    }

                case 'o':
                    {
                        if (argument != null) {
                            throw new ParseException(
                                    "unexpected argument '"
                                            + argument
                                            + "' provided for -o option in proxy string '"
                                            + s
                                            + "'");
                        }
                        mode = Reference.ModeOneway;
                        break;
                    }

                case 'O':
                    {
                        if (argument != null) {
                            throw new ParseException(
                                    "unexpected argument '"
                                            + argument
                                            + "' provided for -O option in proxy string '"
                                            + s
                                            + "'");
                        }
                        mode = Reference.ModeBatchOneway;
                        break;
                    }

                case 'd':
                    {
                        if (argument != null) {
                            throw new ParseException(
                                    "unexpected argument '"
                                            + argument
                                            + "' provided for -d option in proxy string '"
                                            + s
                                            + "'");
                        }
                        mode = Reference.ModeDatagram;
                        break;
                    }

                case 'D':
                    {
                        if (argument != null) {
                            throw new ParseException(
                                    "unexpected argument '"
                                            + argument
                                            + "' provided for -D option in proxy string '"
                                            + s
                                            + "'");
                        }
                        mode = Reference.ModeBatchDatagram;
                        break;
                    }

                case 's':
                    {
                        if (argument != null) {
                            throw new ParseException(
                                    "unexpected argument '"
                                            + argument
                                            + "' provided for -s option in proxy string '"
                                            + s
                                            + "'");
                        }
                        secure = true;
                        break;
                    }

                case 'e':
                    {
                        if (argument == null) {
                            throw new ParseException(
                                    "no argument provided for -e option in in proxy string '"
                                            + s
                                            + "'");
                        }

                        try {
                            encoding = Util.stringToEncodingVersion(argument);
                        } catch (ParseException ex) {
                            throw new ParseException(
                                    "invalid encoding version '"
                                            + argument
                                            + "' in proxy string '"
                                            + s
                                            + "'",
                                    ex);
                        }
                        break;
                    }

                case 'p':
                    {
                        if (argument == null) {
                            throw new ParseException(
                                    "no argument provided for -p option in proxy string '"
                                            + s
                                            + "'");
                        }

                        try {
                            protocol = Util.stringToProtocolVersion(argument);
                        } catch (ParseException ex) {
                            throw new ParseException(
                                    "invalid protocol version '"
                                            + argument
                                            + "' in proxy string '"
                                            + s
                                            + "'",
                                    ex);
                        }
                        break;
                    }

                default:
                    {
                        throw new ParseException(
                                "unknown option '" + option + "' in proxy string '" + s + "'");
                    }
            }
        }

        if (beg == -1) {
            return create(
                    ident,
                    facet,
                    mode,
                    secure,
                    Optional.empty(),
                    protocol,
                    encoding,
                    null,
                    null,
                    propertyPrefix);
        }

        ArrayList<EndpointI> endpoints = new ArrayList<>();

        if (s.charAt(beg) == ':') {
            ArrayList<String> unknownEndpoints = new ArrayList<>();
            end = beg;

            while (end < s.length() && s.charAt(end) == ':') {
                beg = end + 1;

                end = beg;
                while (true) {
                    end = s.indexOf(':', end);
                    if (end == -1) {
                        end = s.length();
                        break;
                    } else {
                        boolean quoted = false;
                        int quote = beg;
                        while (true) {
                            quote = s.indexOf('\"', quote);
                            if (quote == -1 || end < quote) {
                                break;
                            } else {
                                quote = s.indexOf('\"', ++quote);
                                if (quote == -1) {
                                    break;
                                } else if (end < quote) {
                                    quoted = true;
                                    break;
                                }
                                ++quote;
                            }
                        }
                        if (!quoted) {
                            break;
                        }
                        ++end;
                    }
                }

                String es = s.substring(beg, end);
                EndpointI endp = _instance.endpointFactoryManager().create(es, false);
                if (endp != null) {
                    endpoints.add(endp);
                } else {
                    unknownEndpoints.add(es);
                }
            }
            if (endpoints.isEmpty()) {
                assert (!unknownEndpoints.isEmpty());
                throw new ParseException(
                        "invalid endpoint '" + unknownEndpoints.get(0) + "' in '" + s + "'");
            } else if (!unknownEndpoints.isEmpty()
                    && _instance
                                    .initializationData()
                                    .properties
                                    .getIcePropertyAsInt("Ice.Warn.Endpoints")
                            > 0) {
                StringBuffer msg = new StringBuffer("Proxy contains unknown endpoints:");
                for (String e : unknownEndpoints) {
                    msg.append(" `");
                    msg.append(e);
                    msg.append("'");
                }
                _instance.initializationData().logger.warning(msg.toString());
            }

            EndpointI[] endp = new EndpointI[endpoints.size()];
            endpoints.toArray(endp);
            return create(
                    ident,
                    facet,
                    mode,
                    secure,
                    Optional.empty(),
                    protocol,
                    encoding,
                    endp,
                    null,
                    propertyPrefix);
        } else if (s.charAt(beg) == '@') {
            beg = StringUtil.findFirstNotOf(s, delim, beg + 1);
            if (beg == -1) {
                throw new ParseException("missing adapter ID in proxy string '" + s + "'");
            }

            String adapterstr = null;
            end = StringUtil.checkQuote(s, beg);
            if (end == -1) {
                throw new ParseException(
                        "mismatched quotes around adapter ID in proxy string '" + s + "'");
            } else if (end == 0) {
                end = StringUtil.findFirstOf(s, delim, beg);
                if (end == -1) {
                    end = s.length();
                }
                adapterstr = s.substring(beg, end);
            } else {
                beg++; // Skip leading quote
                adapterstr = s.substring(beg, end);
                end++; // Skip trailing quote
            }

            if (end != s.length() && StringUtil.findFirstNotOf(s, delim, end) != -1) {
                throw new ParseException(
                        "invalid characters after adapter ID in proxy string '" + s + "'");
            }

            try {
                adapter = StringUtil.unescapeString(adapterstr, 0, adapterstr.length(), "");
            } catch (IllegalArgumentException ex) {
                throw new ParseException("invalid adapter ID in proxy string '" + s + "'", ex);
            }
            if (adapter.isEmpty()) {
                throw new ParseException("empty adapter ID in proxy string '" + s + "'");
            }
            return create(
                    ident,
                    facet,
                    mode,
                    secure,
                    Optional.empty(),
                    protocol,
                    encoding,
                    null,
                    adapter,
                    propertyPrefix);
        }

        throw new ParseException("malformed proxy string '" + s + "'");
    }

    public Reference create(Identity ident, InputStream s) {
        //
        // Don't read the identity here. Operations calling this
        // constructor read the identity, and pass it as a parameter.
        //

        if (ident.name.isEmpty() && ident.category.isEmpty()) {
            return null;
        }

        //
        // For compatibility with the old FacetPath.
        //
        String[] facetPath = s.readStringSeq();
        String facet;
        if (facetPath.length > 0) {
            if (facetPath.length > 1) {
                throw new MarshalException(
                        "Received invalid facet path with " + facetPath.length + " elements.");
            }
            facet = facetPath[0];
        } else {
            facet = "";
        }

        int mode = s.readByte();
        if (mode < 0 || mode > Reference.ModeLast) {
            throw new MarshalException("Received invalid proxy mode " + mode);
        }

        boolean secure = s.readBool();

        ProtocolVersion protocol;
        EncodingVersion encoding;
        if (!s.getEncoding().equals(Util.Encoding_1_0)) {
            protocol = ProtocolVersion.ice_read(s);
            encoding = EncodingVersion.ice_read(s);
        } else {
            protocol = Util.Protocol_1_0;
            encoding = Util.Encoding_1_0;
        }

        EndpointI[] endpoints = null;
        String adapterId = null;

        int sz = s.readSize();
        if (sz > 0) {
            endpoints = new EndpointI[sz];
            for (int i = 0; i < sz; i++) {
                endpoints[i] = _instance.endpointFactoryManager().read(s);
            }
        } else {
            adapterId = s.readString();
        }

        return create(
                ident,
                facet,
                mode,
                secure,
                Optional.empty(),
                protocol,
                encoding,
                endpoints,
                adapterId,
                null);
    }

    public ReferenceFactory setDefaultRouter(RouterPrx defaultRouter) {
        if (_defaultRouter == null ? defaultRouter == null : _defaultRouter.equals(defaultRouter)) {
            return this;
        }

        ReferenceFactory factory = new ReferenceFactory(_instance, _communicator);
        factory._defaultLocator = _defaultLocator;
        factory._defaultRouter = defaultRouter;
        return factory;
    }

    public RouterPrx getDefaultRouter() {
        return _defaultRouter;
    }

    public ReferenceFactory setDefaultLocator(LocatorPrx defaultLocator) {
        if (_defaultLocator == null
                ? defaultLocator == null
                : _defaultLocator.equals(defaultLocator)) {
            return this;
        }

        ReferenceFactory factory = new ReferenceFactory(_instance, _communicator);
        factory._defaultRouter = _defaultRouter;
        factory._defaultLocator = defaultLocator;
        return factory;
    }

    public LocatorPrx getDefaultLocator() {
        return _defaultLocator;
    }

    //
    // Only for use by Instance
    //
    ReferenceFactory(Instance instance, Communicator communicator) {
        _instance = instance;
        _communicator = communicator;
    }

    private Reference create(
            Identity ident,
            String facet,
            int mode,
            boolean secure,
            Optional<Boolean> compress,
            ProtocolVersion protocol,
            EncodingVersion encoding,
            EndpointI[] endpoints,
            String adapterId,
            String propertyPrefix) {
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();

        //
        // Default local proxy options.
        //
        LocatorInfo locatorInfo = null;
        if (_defaultLocator != null) {
            if (!((_ObjectPrxI) _defaultLocator)._getReference().getEncoding().equals(encoding)) {
                locatorInfo =
                        _instance
                                .locatorManager()
                                .get(_defaultLocator.ice_encodingVersion(encoding));
            } else {
                locatorInfo = _instance.locatorManager().get(_defaultLocator);
            }
        }
        RouterInfo routerInfo = _instance.routerManager().get(_defaultRouter);
        boolean collocationOptimized = defaultsAndOverrides.defaultCollocationOptimization;
        boolean cacheConnection = true;
        boolean preferSecure = defaultsAndOverrides.defaultPreferSecure;
        EndpointSelectionType endpointSelection = defaultsAndOverrides.defaultEndpointSelection;
        Duration locatorCacheTimeout = defaultsAndOverrides.defaultLocatorCacheTimeout;
        Duration invocationTimeout = defaultsAndOverrides.defaultInvocationTimeout;
        Map<String, String> context = null;

        //
        // Override the defaults with the proxy properties if a property prefix is defined.
        //
        if (propertyPrefix != null && !propertyPrefix.isEmpty()) {
            Properties properties = _instance.initializationData().properties;

            Properties.validatePropertiesWithPrefix(
                    propertyPrefix, properties, PropertyNames.ProxyProps);

            String property;

            property = propertyPrefix + ".Locator";
            LocatorPrx locator = LocatorPrx.uncheckedCast(_communicator.propertyToProxy(property));
            if (locator != null) {
                if (!((_ObjectPrxI) locator)._getReference().getEncoding().equals(encoding)) {
                    locatorInfo =
                            _instance.locatorManager().get(locator.ice_encodingVersion(encoding));
                } else {
                    locatorInfo = _instance.locatorManager().get(locator);
                }
            }

            property = propertyPrefix + ".Router";
            RouterPrx router = RouterPrx.uncheckedCast(_communicator.propertyToProxy(property));
            if (router != null) {
                if (propertyPrefix.endsWith(".Router")) {
                    String s =
                            "`"
                                    + property
                                    + "="
                                    + properties.getProperty(property)
                                    + "': cannot set a router on a router; setting ignored";
                    _instance.initializationData().logger.warning(s);
                } else {
                    routerInfo = _instance.routerManager().get(router);
                }
            }

            property = propertyPrefix + ".CollocationOptimized";
            collocationOptimized =
                    properties.getPropertyAsIntWithDefault(property, collocationOptimized ? 1 : 0)
                            > 0;

            property = propertyPrefix + ".ConnectionCached";
            cacheConnection =
                    properties.getPropertyAsIntWithDefault(property, cacheConnection ? 1 : 0) > 0;

            property = propertyPrefix + ".PreferSecure";
            preferSecure =
                    properties.getPropertyAsIntWithDefault(property, preferSecure ? 1 : 0) > 0;

            property = propertyPrefix + ".EndpointSelection";
            if (properties.getProperty(property).length() > 0) {
                String type = properties.getProperty(property);
                if ("Random".equals(type)) {
                    endpointSelection = EndpointSelectionType.Random;
                } else if ("Ordered".equals(type)) {
                    endpointSelection = EndpointSelectionType.Ordered;
                } else {
                    throw new ParseException(
                            "illegal value '"
                                    + type
                                    + "' in property '"
                                    + property
                                    + "'; expected 'Random' or 'Ordered'");
                }
            }

            property = propertyPrefix + ".LocatorCacheTimeout";
            locatorCacheTimeout =
                    Duration.ofSeconds(
                            properties.getPropertyAsIntWithDefault(
                                    property, (int) locatorCacheTimeout.toSeconds()));

            property = propertyPrefix + ".InvocationTimeout";
            invocationTimeout =
                    Duration.ofMillis(
                            properties.getPropertyAsIntWithDefault(
                                    property, (int) invocationTimeout.toMillis()));

            property = propertyPrefix + ".Context.";
            Map<String, String> contexts = properties.getPropertiesForPrefix(property);
            if (!contexts.isEmpty()) {
                context = new HashMap<>();
                for (Map.Entry<String, String> e : contexts.entrySet()) {
                    context.put(e.getKey().substring(property.length()), e.getValue());
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
                compress,
                protocol,
                encoding,
                endpoints,
                adapterId,
                locatorInfo,
                routerInfo,
                collocationOptimized,
                cacheConnection,
                preferSecure,
                endpointSelection,
                locatorCacheTimeout,
                invocationTimeout,
                context);
    }

    private final Instance _instance;
    private final Communicator _communicator;
    private RouterPrx _defaultRouter;
    private LocatorPrx _defaultLocator;
}
