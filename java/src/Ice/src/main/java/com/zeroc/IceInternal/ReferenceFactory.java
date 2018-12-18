// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import com.zeroc.Ice.ProxyParseException;
import com.zeroc.IceUtilInternal.StringUtil;

public final class ReferenceFactory
{
    public Reference
    create(com.zeroc.Ice.Identity ident, String facet, Reference tmpl, EndpointI[] endpoints)
    {
        if(ident.name.length() == 0 && ident.category.length() == 0)
        {
            return null;
        }

        return create(ident, facet, tmpl.getMode(), tmpl.getSecure(), tmpl.getProtocol(), tmpl.getEncoding(),
                      endpoints, null, null);
    }

    public Reference
    create(com.zeroc.Ice.Identity ident, String facet, Reference tmpl, String adapterId)
    {
        if(ident.name.length() == 0 && ident.category.length() == 0)
        {
            return null;
        }

        return create(ident, facet, tmpl.getMode(), tmpl.getSecure(), tmpl.getProtocol(), tmpl.getEncoding(), null,
                      adapterId, null);
    }

    public Reference
    create(com.zeroc.Ice.Identity ident, com.zeroc.Ice.ConnectionI fixedConnection)
    {
        if(ident.name.length() == 0 && ident.category.length() == 0)
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
            fixedConnection.endpoint().datagram() ? Reference.ModeDatagram : Reference.ModeTwoway,
            fixedConnection.endpoint().secure(),
            com.zeroc.Ice.Util.Protocol_1_0,
            _instance.defaultsAndOverrides().defaultEncoding,
            fixedConnection,
            -1,
            null,
            java.util.Optional.empty());
    }

    public Reference
    copy(Reference r)
    {
        com.zeroc.Ice.Identity ident = r.getIdentity();
        if(ident.name.length() == 0 && ident.category.length() == 0)
        {
            return null;
        }
        return r.clone();
    }

    public Reference
    create(String s, String propertyPrefix)
    {
        if(s == null || s.length() == 0)
        {
            return null;
        }

        final String delim = " \t\n\r";

        int beg;
        int end = 0;

        beg = StringUtil.findFirstNotOf(s, delim, end);
        if(beg == -1)
        {
            ProxyParseException e = new ProxyParseException();
            e.str = "no non-whitespace characters found in `" + s + "'";
            throw e;
        }

        //
        // Extract the identity, which may be enclosed in single
        // or double quotation marks.
        //
        String idstr = null;
        end = StringUtil.checkQuote(s, beg);
        if(end == -1)
        {
            ProxyParseException e = new ProxyParseException();
            e.str = "mismatched quotes around identity in `" + s + "'";
            throw e;
        }
        else if(end == 0)
        {
            end = StringUtil.findFirstOf(s, delim + ":@", beg);
            if(end == -1)
            {
                end = s.length();
            }
            idstr = s.substring(beg, end);
        }
        else
        {
            beg++; // Skip leading quote
            idstr = s.substring(beg, end);
            end++; // Skip trailing quote
        }

        if(beg == end)
        {
            ProxyParseException e = new ProxyParseException();
            e.str = "no identity in `" + s + "'";
            throw e;
        }

        //
        // Parsing the identity may raise IdentityParseException.
        //
        com.zeroc.Ice.Identity ident = com.zeroc.Ice.Util.stringToIdentity(idstr);

        if(ident.name.length() == 0)
        {
            //
            // An identity with an empty name and a non-empty
            // category is illegal.
            //
            if(ident.category.length() > 0)
            {
                com.zeroc.Ice.IllegalIdentityException e = new com.zeroc.Ice.IllegalIdentityException();
                e.id = ident;
                throw e;
            }
            //
            // Treat a stringified proxy containing two double
            // quotes ("") the same as an empty string, i.e.,
            // a null proxy, but only if nothing follows the
            // quotes.
            //
            else if(StringUtil.findFirstNotOf(s, delim, end) != -1)
            {
                ProxyParseException e = new ProxyParseException();
                e.str = "invalid characters after identity in `" + s + "'";
                throw e;
            }
            else
            {
                return null;
            }
        }

        String facet = "";
        int mode = Reference.ModeTwoway;
        boolean secure = false;
        com.zeroc.Ice.EncodingVersion encoding = _instance.defaultsAndOverrides().defaultEncoding;
        com.zeroc.Ice.ProtocolVersion protocol = com.zeroc.Ice.Util.Protocol_1_0;
        String adapter = "";

        while(true)
        {
            beg = StringUtil.findFirstNotOf(s, delim, end);
            if(beg == -1)
            {
                break;
            }

            if(s.charAt(beg) == ':' || s.charAt(beg) == '@')
            {
                break;
            }

            end = StringUtil.findFirstOf(s, delim + ":@", beg);
            if(end == -1)
            {
                end = s.length();
            }

            if(beg == end)
            {
                break;
            }

            String option = s.substring(beg, end);
            if(option.length() != 2 || option.charAt(0) != '-')
            {
                ProxyParseException e = new ProxyParseException();
                e.str = "expected a proxy option but found `" + option + "' in `" + s + "'";
                throw e;
            }

            //
            // Check for the presence of an option argument. The
            // argument may be enclosed in single or double
            // quotation marks.
            //
            String argument = null;
            int argumentBeg = StringUtil.findFirstNotOf(s, delim, end);
            if(argumentBeg != -1)
            {
                final char ch = s.charAt(argumentBeg);
                if(ch != '@' && ch != ':' && ch != '-')
                {
                    beg = argumentBeg;
                    end = StringUtil.checkQuote(s, beg);
                    if(end == -1)
                    {
                        ProxyParseException e = new ProxyParseException();
                        e.str = "mismatched quotes around value for " + option + " option in `" + s + "'";
                        throw e;
                    }
                    else if(end == 0)
                    {
                        end = StringUtil.findFirstOf(s, delim + ":@", beg);
                        if(end == -1)
                        {
                            end = s.length();
                        }
                        argument = s.substring(beg, end);
                    }
                    else
                    {
                        beg++; // Skip leading quote
                        argument = s.substring(beg, end);
                        end++; // Skip trailing quote
                    }
                }
            }

            //
            // If any new options are added here,
            // IceInternal::Reference::toString() and its derived classes must be updated as well.
            //
            switch(option.charAt(1))
            {
                case 'f':
                {
                    if(argument == null)
                    {
                        ProxyParseException e = new ProxyParseException();
                        e.str = "no argument provided for -f option in `" + s + "'";
                        throw e;
                    }

                    try
                    {
                        facet = StringUtil.unescapeString(argument, 0, argument.length(), "");
                    }
                    catch(IllegalArgumentException ex)
                    {
                        ProxyParseException e = new ProxyParseException();
                        e.str = "invalid facet in `" + s + "': " + ex.getMessage();
                        throw e;
                    }

                    break;
                }

                case 't':
                {
                    if(argument != null)
                    {
                        ProxyParseException e = new ProxyParseException();
                        e.str = "unexpected argument `" + argument + "' provided for -t option in `" + s + "'";
                        throw e;
                    }
                    mode = Reference.ModeTwoway;
                    break;
                }

                case 'o':
                {
                    if(argument != null)
                    {
                        ProxyParseException e = new ProxyParseException();
                        e.str = "unexpected argument `" + argument + "' provided for -o option in `" + s + "'";
                        throw e;
                    }
                    mode = Reference.ModeOneway;
                    break;
                }

                case 'O':
                {
                    if(argument != null)
                    {
                        ProxyParseException e = new ProxyParseException();
                        e.str = "unexpected argument `" + argument + "' provided for -O option in `" + s + "'";
                        throw e;
                    }
                    mode = Reference.ModeBatchOneway;
                    break;
                }

                case 'd':
                {
                    if(argument != null)
                    {
                        ProxyParseException e = new ProxyParseException();
                        e.str = "unexpected argument `" + argument + "' provided for -d option in `" + s + "'";
                        throw e;
                    }
                    mode = Reference.ModeDatagram;
                    break;
                }

                case 'D':
                {
                    if(argument != null)
                    {
                        ProxyParseException e = new ProxyParseException();
                        e.str = "unexpected argument `" + argument + "' provided for -D option in `" + s + "'";
                        throw e;
                    }
                    mode = Reference.ModeBatchDatagram;
                    break;
                }

                case 's':
                {
                    if(argument != null)
                    {
                        ProxyParseException e = new ProxyParseException();
                        e.str = "unexpected argument `" + argument + "' provided for -s option in `" + s + "'";
                        throw e;
                    }
                    secure = true;
                    break;
                }

                case 'e':
                {
                    if(argument == null)
                    {
                        throw new ProxyParseException("no argument provided for -e option in `" + s + "'");
                    }

                    try
                    {
                        encoding = com.zeroc.Ice.Util.stringToEncodingVersion(argument);
                    }
                    catch(com.zeroc.Ice.VersionParseException e)
                    {
                        throw new ProxyParseException("invalid encoding version `" + argument + "' in `" + s +
                                                      "':\n" + e.str);
                    }
                    break;
                }

                case 'p':
                {
                    if(argument == null)
                    {
                        throw new ProxyParseException("no argument provided for -p option in `" + s + "'");
                    }

                    try
                    {
                        protocol = com.zeroc.Ice.Util.stringToProtocolVersion(argument);
                    }
                    catch(com.zeroc.Ice.VersionParseException e)
                    {
                        throw new ProxyParseException("invalid protocol version `" + argument + "' in `" + s +
                                                      "':\n" + e.str);
                    }
                    break;
                }

                default:
                {
                    ProxyParseException e = new ProxyParseException();
                    e.str = "unknown option `" + option + "' in `" + s + "'";
                    throw e;
                }
            }
        }

        if(beg == -1)
        {
            return create(ident, facet, mode, secure, protocol, encoding, null, null, propertyPrefix);
        }

        java.util.ArrayList<EndpointI> endpoints = new java.util.ArrayList<>();

        if(s.charAt(beg) == ':')
        {
            java.util.ArrayList<String> unknownEndpoints = new java.util.ArrayList<>();
            end = beg;

            while(end < s.length() && s.charAt(end) == ':')
            {
                beg = end + 1;

                end = beg;
                while(true)
                {
                    end = s.indexOf(':', end);
                    if(end == -1)
                    {
                        end = s.length();
                        break;
                    }
                    else
                    {
                        boolean quoted = false;
                        int quote = beg;
                        while(true)
                        {
                            quote = s.indexOf('\"', quote);
                            if(quote == -1 || end < quote)
                            {
                                break;
                            }
                            else
                            {
                                quote = s.indexOf('\"', ++quote);
                                if(quote == -1)
                                {
                                    break;
                                }
                                else if(end < quote)
                                {
                                    quoted = true;
                                    break;
                                }
                                ++quote;
                            }
                        }
                        if(!quoted)
                        {
                            break;
                        }
                        ++end;
                    }
                }

                String es = s.substring(beg, end);
                EndpointI endp = _instance.endpointFactoryManager().create(es, false);
                if(endp != null)
                {
                    endpoints.add(endp);
                }
                else
                {
                    unknownEndpoints.add(es);
                }
            }
            if(endpoints.size() == 0)
            {
                assert(!unknownEndpoints.isEmpty());
                com.zeroc.Ice.EndpointParseException e = new com.zeroc.Ice.EndpointParseException();
                e.str = "invalid endpoint `" + unknownEndpoints.get(0) + "' in `" + s + "'";
                throw e;
            }
            else if(unknownEndpoints.size() != 0 &&
                   _instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Endpoints", 1) > 0)
            {
                StringBuffer msg = new StringBuffer("Proxy contains unknown endpoints:");
                for(String e : unknownEndpoints)
                {
                    msg.append(" `");
                    msg.append(e);
                    msg.append("'");
                }
                _instance.initializationData().logger.warning(msg.toString());
            }

            EndpointI[] endp = new EndpointI[endpoints.size()];
            endpoints.toArray(endp);
            return create(ident, facet, mode, secure, protocol, encoding, endp, null, propertyPrefix);
        }
        else if(s.charAt(beg) == '@')
        {
            beg = StringUtil.findFirstNotOf(s, delim, beg + 1);
            if(beg == -1)
            {
                ProxyParseException e = new ProxyParseException();
                e.str = "missing adapter id in `" + s + "'";
                throw e;
            }

            String adapterstr = null;
            end = StringUtil.checkQuote(s, beg);
            if(end == -1)
            {
                ProxyParseException e = new ProxyParseException();
                e.str = "mismatched quotes around adapter id in `" + s + "'";
                throw e;
            }
            else if(end == 0)
            {
                end = StringUtil.findFirstOf(s, delim, beg);
                if(end == -1)
                {
                    end = s.length();
                }
                adapterstr = s.substring(beg, end);
            }
            else
            {
                beg++; // Skip leading quote
                adapterstr = s.substring(beg, end);
                end++; // Skip trailing quote
            }

            if(end != s.length() && StringUtil.findFirstNotOf(s, delim, end) != -1)
            {
                ProxyParseException e = new ProxyParseException();
                e.str = "invalid trailing characters after `" + s.substring(0, end + 1) + "' in `" + s + "'";
                throw e;
            }

            try
            {
                adapter = StringUtil.unescapeString(adapterstr, 0, adapterstr.length(), "");
            }
            catch(IllegalArgumentException ex)
            {
                ProxyParseException e = new ProxyParseException();
                e.str = "invalid adapter id in `" + s + "': " + ex.getMessage();
                throw e;
            }
            if(adapter.length() == 0)
            {
                ProxyParseException e = new ProxyParseException();
                e.str = "empty adapter id in `" + s + "'";
                throw e;
            }
            return create(ident, facet, mode, secure, protocol, encoding, null, adapter, propertyPrefix);
        }

        ProxyParseException ex = new ProxyParseException();
        ex.str = "malformed proxy `" + s + "'";
        throw ex;
    }

    public Reference
    create(com.zeroc.Ice.Identity ident, com.zeroc.Ice.InputStream s)
    {
        //
        // Don't read the identity here. Operations calling this
        // constructor read the identity, and pass it as a parameter.
        //

        if(ident.name.length() == 0 && ident.category.length() == 0)
        {
            return null;
        }

        //
        // For compatibility with the old FacetPath.
        //
        String[] facetPath = s.readStringSeq();
        String facet;
        if(facetPath.length > 0)
        {
            if(facetPath.length > 1)
            {
                throw new com.zeroc.Ice.ProxyUnmarshalException();
            }
            facet = facetPath[0];
        }
        else
        {
            facet = "";
        }

        int mode = s.readByte();
        if(mode < 0 || mode > Reference.ModeLast)
        {
            throw new com.zeroc.Ice.ProxyUnmarshalException();
        }

        boolean secure = s.readBool();

        com.zeroc.Ice.ProtocolVersion protocol;
        com.zeroc.Ice.EncodingVersion encoding;
        if(!s.getEncoding().equals(com.zeroc.Ice.Util.Encoding_1_0))
        {
            protocol = com.zeroc.Ice.ProtocolVersion.ice_read(s);
            encoding = com.zeroc.Ice.EncodingVersion.ice_read(s);
        }
        else
        {
            protocol = com.zeroc.Ice.Util.Protocol_1_0;
            encoding = com.zeroc.Ice.Util.Encoding_1_0;
        }

        EndpointI[] endpoints = null;
        String adapterId = null;

        int sz = s.readSize();
        if(sz > 0)
        {
            endpoints = new EndpointI[sz];
            for(int i = 0; i < sz; i++)
            {
                endpoints[i] = _instance.endpointFactoryManager().read(s);
            }
        }
        else
        {
            adapterId = s.readString();
        }

        return create(ident, facet, mode, secure, protocol, encoding, endpoints, adapterId, null);
    }

    public ReferenceFactory
    setDefaultRouter(com.zeroc.Ice.RouterPrx defaultRouter)
    {
        if(_defaultRouter == null ? defaultRouter == null : _defaultRouter.equals(defaultRouter))
        {
            return this;
        }

        ReferenceFactory factory = new ReferenceFactory(_instance, _communicator);
        factory._defaultLocator = _defaultLocator;
        factory._defaultRouter = defaultRouter;
        return factory;
    }

    public com.zeroc.Ice.RouterPrx
    getDefaultRouter()
    {
        return _defaultRouter;
    }

    public ReferenceFactory
    setDefaultLocator(com.zeroc.Ice.LocatorPrx defaultLocator)
    {
        if(_defaultLocator == null ? defaultLocator == null : _defaultLocator.equals(defaultLocator))
        {
            return this;
        }

        ReferenceFactory factory = new ReferenceFactory(_instance, _communicator);
        factory._defaultRouter = _defaultRouter;
        factory._defaultLocator = defaultLocator;
        return factory;
    }

    public com.zeroc.Ice.LocatorPrx
    getDefaultLocator()
    {
        return _defaultLocator;
    }

    //
    // Only for use by Instance
    //
    ReferenceFactory(Instance instance, com.zeroc.Ice.Communicator communicator)
    {
        _instance = instance;
        _communicator = communicator;
    }

    static private String[] _suffixes =
    {
        "EndpointSelection",
        "ConnectionCached",
        "PreferSecure",
        "LocatorCacheTimeout",
        "InvocationTimeout",
        "Locator",
        "Router",
        "CollocationOptimized",
        "Context\\..*"
    };

    private void
    checkForUnknownProperties(String prefix)
    {
        //
        // Do not warn about unknown properties if Ice prefix, ie Ice, Glacier2, etc
        //
        for(int i = 0; PropertyNames.clPropNames[i] != null; ++i)
        {
            if(prefix.startsWith(PropertyNames.clPropNames[i] + "."))
            {
                return;
            }
        }

        java.util.ArrayList<String> unknownProps = new java.util.ArrayList<>();
        java.util.Map<String, String> props =
            _instance.initializationData().properties.getPropertiesForPrefix(prefix + ".");
        for(java.util.Map.Entry<String, String> p : props.entrySet())
        {
            String prop = p.getKey();

            boolean valid = false;
            for(String suffix : _suffixes)
            {
                String pattern = java.util.regex.Pattern.quote(prefix + ".") + suffix;
                if(java.util.regex.Pattern.compile(pattern).matcher(prop).matches())
                {
                    valid = true;
                    break;
                }
            }

            if(!valid)
            {
                unknownProps.add(prop);
            }
        }

        if(unknownProps.size() != 0)
        {
            StringBuffer message = new StringBuffer("found unknown properties for proxy '");
            message.append(prefix);
            message.append("':");
            for(String s : unknownProps)
            {
                message.append("\n    ");
                message.append(s);
            }
            _instance.initializationData().logger.warning(message.toString());
        }
    }

    private Reference
    create(com.zeroc.Ice.Identity ident, String facet, int mode, boolean secure, com.zeroc.Ice.ProtocolVersion protocol,
           com.zeroc.Ice.EncodingVersion encoding, EndpointI[] endpoints, String adapterId, String propertyPrefix)
    {
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();

        //
        // Default local proxy options.
        //
        LocatorInfo locatorInfo = null;
        if(_defaultLocator != null)
        {
            if(!((com.zeroc.Ice._ObjectPrxI)_defaultLocator)._getReference().getEncoding().equals(encoding))
            {
                locatorInfo = _instance.locatorManager().get(_defaultLocator.ice_encodingVersion(encoding));
            }
            else
            {
                locatorInfo = _instance.locatorManager().get(_defaultLocator);
            }
        }
        RouterInfo routerInfo = _instance.routerManager().get(_defaultRouter);
        boolean collocationOptimized = defaultsAndOverrides.defaultCollocationOptimization;
        boolean cacheConnection = true;
        boolean preferSecure = defaultsAndOverrides.defaultPreferSecure;
        com.zeroc.Ice.EndpointSelectionType endpointSelection = defaultsAndOverrides.defaultEndpointSelection;
        int locatorCacheTimeout = defaultsAndOverrides.defaultLocatorCacheTimeout;
        int invocationTimeout = defaultsAndOverrides.defaultInvocationTimeout;
        java.util.Map<String, String> context = null;

        //
        // Override the defaults with the proxy properties if a property prefix is defined.
        //
        if(propertyPrefix != null && propertyPrefix.length() > 0)
        {
            com.zeroc.Ice.Properties properties = _instance.initializationData().properties;

            //
            // Warn about unknown properties.
            //
            if(properties.getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
            {
                checkForUnknownProperties(propertyPrefix);
            }

            String property;

            property = propertyPrefix + ".Locator";
            com.zeroc.Ice.LocatorPrx locator =
                com.zeroc.Ice.LocatorPrx.uncheckedCast(_communicator.propertyToProxy(property));
            if(locator != null)
            {
                if(!((com.zeroc.Ice._ObjectPrxI)locator)._getReference().getEncoding().equals(encoding))
                {
                    locatorInfo = _instance.locatorManager().get(locator.ice_encodingVersion(encoding));
                }
                else
                {
                    locatorInfo = _instance.locatorManager().get(locator);
                }
            }

            property = propertyPrefix + ".Router";
            com.zeroc.Ice.RouterPrx router =
                com.zeroc.Ice.RouterPrx.uncheckedCast(_communicator.propertyToProxy(property));
            if(router != null)
            {
                if(propertyPrefix.endsWith(".Router"))
                {
                    String s = "`" + property + "=" + properties.getProperty(property) +
                        "': cannot set a router on a router; setting ignored";
                    _instance.initializationData().logger.warning(s);
                }
                else
                {
                    routerInfo = _instance.routerManager().get(router);
                }
            }

            property = propertyPrefix + ".CollocationOptimized";
            collocationOptimized = properties.getPropertyAsIntWithDefault(property, collocationOptimized ? 1 : 0) > 0;

            property = propertyPrefix + ".ConnectionCached";
            cacheConnection = properties.getPropertyAsIntWithDefault(property, cacheConnection ? 1 : 0) > 0;

            property = propertyPrefix + ".PreferSecure";
            preferSecure = properties.getPropertyAsIntWithDefault(property, preferSecure ? 1 : 0) > 0;

            property = propertyPrefix + ".EndpointSelection";
            if(properties.getProperty(property).length() > 0)
            {
                String type = properties.getProperty(property);
                if(type.equals("Random"))
                {
                    endpointSelection = com.zeroc.Ice.EndpointSelectionType.Random;
                }
                else if(type.equals("Ordered"))
                {
                    endpointSelection = com.zeroc.Ice.EndpointSelectionType.Ordered;
                }
                else
                {
                    throw new com.zeroc.Ice.EndpointSelectionTypeParseException("illegal value `" + type +
                                                                      "'; expected `Random' or `Ordered'");
                }
            }

            property = propertyPrefix + ".LocatorCacheTimeout";
            String value = properties.getProperty(property);
            if(!value.isEmpty())
            {
                locatorCacheTimeout = properties.getPropertyAsIntWithDefault(property, locatorCacheTimeout);
                if(locatorCacheTimeout < -1)
                {
                    locatorCacheTimeout = -1;

                    StringBuffer msg = new StringBuffer("invalid value for ");
                    msg.append(property);
                    msg.append(" '");
                    msg.append(properties.getProperty(property));
                    msg.append("': defaulting to -1");
                    _instance.initializationData().logger.warning(msg.toString());
                }
            }

            property = propertyPrefix + ".InvocationTimeout";
            value = properties.getProperty(property);
            if(!value.isEmpty())
            {
                invocationTimeout = properties.getPropertyAsIntWithDefault(property, locatorCacheTimeout);
                if(invocationTimeout < 1 && invocationTimeout != -1)
                {
                    invocationTimeout = -1;

                    StringBuffer msg = new StringBuffer("invalid value for ");
                    msg.append(property);
                    msg.append(" '");
                    msg.append(properties.getProperty(property));
                    msg.append("': defaulting to -1");
                    _instance.initializationData().logger.warning(msg.toString());
                }
            }

            property = propertyPrefix + ".Context.";
            java.util.Map<String, String> contexts = properties.getPropertiesForPrefix(property);
            if(!contexts.isEmpty())
            {
                context = new java.util.HashMap<>();
                for(java.util.Map.Entry<String, String> e : contexts.entrySet())
                {
                    context.put(e.getKey().substring(property.length()), e.getValue());
                }
            }
        }

        //
        // Create new reference
        //
        return new RoutableReference(_instance,
                                     _communicator,
                                     ident,
                                     facet,
                                     mode,
                                     secure,
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

    final private Instance _instance;
    final private com.zeroc.Ice.Communicator _communicator;
    private com.zeroc.Ice.RouterPrx _defaultRouter;
    private com.zeroc.Ice.LocatorPrx _defaultLocator;
}
