// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Text.RegularExpressions;

namespace IceInternal
{

    public class ReferenceFactory
    {
        public Reference
        create(Ice.Identity ident, string facet, Reference tmpl, EndpointI[] endpoints)
        {
            if(ident.name.Length == 0 && ident.category.Length == 0)
            {
                return null;
            }

            return create(ident, facet, tmpl.getMode(), tmpl.getSecure(), tmpl.getProtocol(), tmpl.getEncoding(),
                          endpoints, null, null);
        }

        public Reference
        create(Ice.Identity ident, string facet, Reference tmpl, string adapterId)
        {
            if(ident.name.Length == 0 && ident.category.Length == 0)
            {
                return null;
            }

            //
            // Create new reference
            //
            return create(ident, facet, tmpl.getMode(), tmpl.getSecure(), tmpl.getProtocol(), tmpl.getEncoding(),
                          null, adapterId, null);
        }

        public Reference create(Ice.Identity ident, Ice.ConnectionI connection)
        {
            if(ident.name.Length == 0 && ident.category.Length == 0)
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
                _instance.defaultsAndOverrides().defaultEncoding,
                connection);
        }

        public Reference copy(Reference r)
        {
            Ice.Identity ident = r.getIdentity();
            if(ident.name.Length == 0 && ident.category.Length == 0)
            {
                return null;
            }
            return (Reference)r.Clone();
        }

        public Reference create(string s, string propertyPrefix)
        {
            if(s.Length == 0)
            {
                return null;
            }

            const string delim = " \t\n\r";

            int beg;
            int end = 0;

            beg = IceUtilInternal.StringUtil.findFirstNotOf(s, delim, end);
            if(beg == -1)
            {
                Ice.ProxyParseException e = new Ice.ProxyParseException();
                e.str = "no non-whitespace characters found in `" + s + "'";
                throw e;
            }

            //
            // Extract the identity, which may be enclosed in single
            // or double quotation marks.
            //
            string idstr = null;
            end = IceUtilInternal.StringUtil.checkQuote(s, beg);
            if(end == -1)
            {
                Ice.ProxyParseException e = new Ice.ProxyParseException();
                e.str = "mismatched quotes around identity in `" + s + "'";
                throw e;
            }
            else if(end == 0)
            {
                end = IceUtilInternal.StringUtil.findFirstOf(s, delim + ":@", beg);
                if(end == -1)
                {
                    end = s.Length;
                }
                idstr = s.Substring(beg, end - beg);
            }
            else
            {
                beg++; // Skip leading quote
                idstr = s.Substring(beg, end - beg);
                end++; // Skip trailing quote
            }

            if(beg == end)
            {
                Ice.ProxyParseException e = new Ice.ProxyParseException();
                e.str = "no identity in `" + s + "'";
                throw e;
            }

            //
            // Parsing the identity may raise IdentityParseException.
            //
            Ice.Identity ident = Ice.Util.stringToIdentity(idstr);

            if(ident.name.Length == 0)
            {
                //
                // An identity with an empty name and a non-empty
                // category is illegal.
                //
                if(ident.category.Length > 0)
                {
                    Ice.IllegalIdentityException e = new Ice.IllegalIdentityException();
                    e.id = ident;
                    throw e;
                }
                //
                // Treat a stringified proxy containing two double
                // quotes ("") the same as an empty string, i.e.,
                // a null proxy, but only if nothing follows the
                // quotes.
                //
                else if(IceUtilInternal.StringUtil.findFirstNotOf(s, delim, end) != -1)
                {
                    Ice.ProxyParseException e = new Ice.ProxyParseException();
                    e.str = "invalid characters after identity in `" + s + "'";
                    throw e;
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
            string adapter = "";

            while(true)
            {
                beg = IceUtilInternal.StringUtil.findFirstNotOf(s, delim, end);
                if(beg == -1)
                {
                    break;
                }

                if(s[beg] == ':' || s[beg] == '@')
                {
                    break;
                }

                end = IceUtilInternal.StringUtil.findFirstOf(s, delim + ":@", beg);
                if(end == -1)
                {
                    end = s.Length;
                }

                if(beg == end)
                {
                    break;
                }

                string option = s.Substring(beg, end - beg);
                if(option.Length != 2 || option[0] != '-')
                {
                    Ice.ProxyParseException e = new Ice.ProxyParseException();
                    e.str = "expected a proxy option but found `" + option + "' in `" + s + "'";
                    throw e;
                }

                //
                // Check for the presence of an option argument. The
                // argument may be enclosed in single or double
                // quotation marks.
                //
                string argument = null;
                int argumentBeg = IceUtilInternal.StringUtil.findFirstNotOf(s, delim, end);
                if(argumentBeg != -1)
                {
                    char ch = s[argumentBeg];
                    if(ch != '@' && ch != ':' && ch != '-')
                    {
                        beg = argumentBeg;
                        end = IceUtilInternal.StringUtil.checkQuote(s, beg);
                        if(end == -1)
                        {
                            Ice.ProxyParseException e = new Ice.ProxyParseException();
                            e.str = "mismatched quotes around value for " + option + " option in `" + s + "'";
                            throw e;
                        }
                        else if(end == 0)
                        {
                            end = IceUtilInternal.StringUtil.findFirstOf(s, delim + ":@", beg);
                            if(end == -1)
                            {
                                end = s.Length;
                            }
                            argument = s.Substring(beg, end - beg);
                        }
                        else
                        {
                            beg++; // Skip leading quote
                            argument = s.Substring(beg, end - beg);
                            end++; // Skip trailing quote
                        }
                    }
                }

                //
                // If any new options are added here,
                // IceInternal::Reference::toString() and its derived classes must be updated as well.
                //
                switch(option[1])
                {
                    case 'f':
                    {
                        if(argument == null)
                        {
                            Ice.ProxyParseException e = new Ice.ProxyParseException();
                            e.str = "no argument provided for -f option in `" + s + "'";
                            throw e;
                        }

                        try
                        {
                            facet = IceUtilInternal.StringUtil.unescapeString(argument, 0, argument.Length, "");
                        }
                        catch(System.ArgumentException argEx)
                        {
                            Ice.ProxyParseException e = new Ice.ProxyParseException();
                            e.str = "invalid facet in `" + s + "': " + argEx.Message;
                            throw e;
                        }
                        break;
                    }

                    case 't':
                    {
                        if(argument != null)
                        {
                            Ice.ProxyParseException e = new Ice.ProxyParseException();
                            e.str = "unexpected argument `" + argument + "' provided for -t option in `" + s + "'";
                            throw e;
                        }
                        mode = Reference.Mode.ModeTwoway;
                        break;
                    }

                    case 'o':
                    {
                        if(argument != null)
                        {
                            Ice.ProxyParseException e = new Ice.ProxyParseException();
                            e.str = "unexpected argument `" + argument + "' provided for -o option in `" + s + "'";
                            throw e;
                        }
                        mode = Reference.Mode.ModeOneway;
                        break;
                    }

                    case 'O':
                    {
                        if(argument != null)
                        {
                            Ice.ProxyParseException e = new Ice.ProxyParseException();
                            e.str = "unexpected argument `" + argument + "' provided for -O option in `" + s + "'";
                            throw e;
                        }
                        mode = Reference.Mode.ModeBatchOneway;
                        break;
                    }

                    case 'd':
                    {
                        if(argument != null)
                        {
                            Ice.ProxyParseException e = new Ice.ProxyParseException();
                            e.str = "unexpected argument `" + argument + "' provided for -d option in `" + s + "'";
                            throw e;
                        }
                        mode = Reference.Mode.ModeDatagram;
                        break;
                    }

                    case 'D':
                    {
                        if(argument != null)
                        {
                            Ice.ProxyParseException e = new Ice.ProxyParseException();
                            e.str = "unexpected argument `" + argument + "' provided for -D option in `" + s + "'";
                            throw e;
                        }
                        mode = Reference.Mode.ModeBatchDatagram;
                        break;
                    }

                    case 's':
                    {
                        if(argument != null)
                        {
                            Ice.ProxyParseException e = new Ice.ProxyParseException();
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
                            throw new Ice.ProxyParseException("no argument provided for -e option `" + s + "'");
                        }

                        try
                        {
                            encoding = Ice.Util.stringToEncodingVersion(argument);
                        }
                        catch(Ice.VersionParseException e)
                        {
                            throw new Ice.ProxyParseException("invalid encoding version `" + argument + "' in `" + s +
                                                              "':\n" + e.str);
                        }
                        break;
                    }

                    case 'p':
                    {
                        if(argument == null)
                        {
                            throw new Ice.ProxyParseException("no argument provided for -p option `" + s + "'");
                        }

                        try
                        {
                            protocol = Ice.Util.stringToProtocolVersion(argument);
                        }
                        catch(Ice.VersionParseException e)
                        {
                            throw new Ice.ProxyParseException("invalid protocol version `" + argument + "' in `" + s +
                                                              "':\n" + e.str);
                        }
                        break;
                    }

                    default:
                    {
                        Ice.ProxyParseException e = new Ice.ProxyParseException();
                        e.str = "unknown option `" + option + "' in `" + s + "'";
                        throw e;
                    }
                }
            }

            if(beg == -1)
            {
                return create(ident, facet, mode, secure, protocol, encoding, null, null, propertyPrefix);
            }

            List<EndpointI> endpoints = new List<EndpointI>();

            if(s[beg] == ':')
            {
                List<string> unknownEndpoints = new List<string>();
                end = beg;

                while(end < s.Length && s[end] == ':')
                {
                    beg = end + 1;

                    end = beg;
                    while(true)
                    {
                        end = s.IndexOf(':', end);
                        if(end == -1)
                        {
                            end = s.Length;
                            break;
                        }
                        else
                        {
                            bool quoted = false;
                            int quote = beg;
                            while(true)
                            {
                                quote = s.IndexOf((System.Char) '\"', quote);
                                if(quote == -1 || end < quote)
                                {
                                    break;
                                }
                                else
                                {
                                    quote = s.IndexOf((System.Char) '\"', ++quote);
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

                    string es = s.Substring(beg, end - beg);
                    EndpointI endp = _instance.endpointFactoryManager().create(es, false);
                    if(endp != null)
                    {
                        endpoints.Add(endp);
                    }
                    else
                    {
                        unknownEndpoints.Add(es);
                    }
                }
                if(endpoints.Count == 0)
                {
                    Debug.Assert(unknownEndpoints.Count > 0);
                    Ice.EndpointParseException e2 = new Ice.EndpointParseException();
                    e2.str = "invalid endpoint `" + unknownEndpoints[0] + "' in `" + s + "'";
                    throw e2;
                }
                else if(unknownEndpoints.Count != 0 &&
                        _instance.initializationData().properties.getPropertyAsIntWithDefault(
                                                                                "Ice.Warn.Endpoints", 1) > 0)
                {
                    StringBuilder msg = new StringBuilder("Proxy contains unknown endpoints:");
                    int sz = unknownEndpoints.Count;
                    for(int idx = 0; idx < sz; ++idx)
                    {
                        msg.Append(" `");
                        msg.Append((string)unknownEndpoints[idx]);
                        msg.Append("'");
                    }
                    _instance.initializationData().logger.warning(msg.ToString());
                }

                EndpointI[] ep = endpoints.ToArray();
                return create(ident, facet, mode, secure, protocol, encoding, ep, null, propertyPrefix);
            }
            else if(s[beg] == '@')
            {
                beg = IceUtilInternal.StringUtil.findFirstNotOf(s, delim, beg + 1);
                if(beg == -1)
                {
                    Ice.ProxyParseException e = new Ice.ProxyParseException();
                    e.str = "missing adapter id in `" + s + "'";
                    throw e;
                }

                string adapterstr = null;
                end = IceUtilInternal.StringUtil.checkQuote(s, beg);
                if(end == -1)
                {
                    Ice.ProxyParseException e = new Ice.ProxyParseException();
                    e.str = "mismatched quotes around adapter id in `" + s + "'";
                    throw e;
                }
                else if(end == 0)
                {
                    end = IceUtilInternal.StringUtil.findFirstOf(s, delim, beg);
                    if(end == -1)
                    {
                        end = s.Length;
                    }
                    adapterstr = s.Substring(beg, end - beg);
                }
                else
                {
                    beg++; // Skip leading quote
                    adapterstr = s.Substring(beg, end - beg);
                    end++; // Skip trailing quote
                }

                if(end != s.Length && IceUtilInternal.StringUtil.findFirstNotOf(s, delim, end) != -1)
                {
                    Ice.ProxyParseException e = new Ice.ProxyParseException();
                    e.str = "invalid trailing characters after `" + s.Substring(0, end + 1) + "' in `" + s + "'";
                    throw e;
                }

                try
                {
                    adapter = IceUtilInternal.StringUtil.unescapeString(adapterstr, 0, adapterstr.Length, "");
                }
                catch(System.ArgumentException argEx)
                {
                    Ice.ProxyParseException e = new Ice.ProxyParseException();
                    e.str = "invalid adapter id in `" + s + "': " + argEx.Message;
                    throw e;
                }
                if(adapter.Length == 0)
                {
                    Ice.ProxyParseException e = new Ice.ProxyParseException();
                    e.str = "empty adapter id in `" + s + "'";
                    throw e;
                }
                return create(ident, facet, mode, secure, protocol, encoding, null, adapter, propertyPrefix);
            }

            Ice.ProxyParseException ex = new Ice.ProxyParseException();
            ex.str = "malformed proxy `" + s + "'";
            throw ex;
        }

        public Reference create(Ice.Identity ident, Ice.InputStream s)
        {
            //
            // Don't read the identity here. Operations calling this
            // constructor read the identity, and pass it as a parameter.
            //

            if(ident.name.Length == 0 && ident.category.Length == 0)
            {
                return null;
            }

            //
            // For compatibility with the old FacetPath.
            //
            string[] facetPath = s.readStringSeq();
            string facet;
            if(facetPath.Length > 0)
            {
                if(facetPath.Length > 1)
                {
                    throw new Ice.ProxyUnmarshalException();
                }
                facet = facetPath[0];
            }
            else
            {
                facet = "";
            }

            int mode = (int)s.readByte();
            if(mode < 0 || mode > (int)Reference.Mode.ModeLast)
            {
                throw new Ice.ProxyUnmarshalException();
            }

            bool secure = s.readBool();

            Ice.ProtocolVersion protocol;
            Ice.EncodingVersion encoding;
            if(!s.getEncoding().Equals(Ice.Util.Encoding_1_0))
            {
                protocol = new Ice.ProtocolVersion();
                protocol.ice_readMembers(s);
                encoding = new Ice.EncodingVersion();
                encoding.ice_readMembers(s);
            }
            else
            {
                protocol = Ice.Util.Protocol_1_0;
                encoding = Ice.Util.Encoding_1_0;
            }

            EndpointI[] endpoints = null;
            string adapterId = "";

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

            return create(ident, facet, (Reference.Mode)mode, secure, protocol, encoding, endpoints, adapterId, null);
        }

        public ReferenceFactory setDefaultRouter(Ice.RouterPrx defaultRouter)
        {
            if(_defaultRouter == null ? defaultRouter == null : _defaultRouter.Equals(defaultRouter))
            {
                return this;
            }

            ReferenceFactory factory = new ReferenceFactory(_instance, _communicator);
            factory._defaultLocator = _defaultLocator;
            factory._defaultRouter = defaultRouter;
            return factory;
        }

        public Ice.RouterPrx getDefaultRouter()
        {
            return _defaultRouter;
        }

        public ReferenceFactory setDefaultLocator(Ice.LocatorPrx defaultLocator)
        {
            if(_defaultLocator == null ? defaultLocator == null : _defaultLocator.Equals(defaultLocator))
            {
                return this;
            }

            ReferenceFactory factory = new ReferenceFactory(_instance, _communicator);
            factory._defaultLocator = defaultLocator;
            factory._defaultRouter = _defaultRouter;
            return factory;
        }

        public Ice.LocatorPrx getDefaultLocator()
        {
            return _defaultLocator;
        }

        //
        // Only for use by Instance
        //
        internal ReferenceFactory(Instance instance, Ice.Communicator communicator)
        {
            _instance = instance;
            _communicator = communicator;
        }

        static private readonly string[] _suffixes =
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
            for(int i = 0; IceInternal.PropertyNames.clPropNames[i] != null; ++i)
            {
                if(prefix.StartsWith(IceInternal.PropertyNames.clPropNames[i] + ".", StringComparison.Ordinal))
                {
                    return;
                }
            }

            List<string> unknownProps = new List<string>();
            Dictionary<string, string> props
                = _instance.initializationData().properties.getPropertiesForPrefix(prefix + ".");
            foreach(String prop in props.Keys)
            {
                bool valid = false;
                for(int i = 0; i < _suffixes.Length; ++i)
                {
                    string pattern = "^" + Regex.Escape(prefix + ".") + _suffixes[i] + "$";
                    if(new Regex(pattern).Match(prop).Success)
                    {
                        valid = true;
                        break;
                    }
                }

                if(!valid)
                {
                    unknownProps.Add(prop);
                }
            }

            if(unknownProps.Count != 0)
            {
                StringBuilder message = new StringBuilder("found unknown properties for proxy '");
                message.Append(prefix);
                message.Append("':");
                foreach(string s in unknownProps)
                {
                    message.Append("\n    ");
                    message.Append(s);
                }
                _instance.initializationData().logger.warning(message.ToString());
            }
        }

        private Reference create(Ice.Identity ident,
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
            if(_defaultLocator != null)
            {
                if(!((Ice.ObjectPrxHelperBase)_defaultLocator).iceReference().getEncoding().Equals(encoding))
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
            int locatorCacheTimeout = defaultsAndOverrides.defaultLocatorCacheTimeout;
            int invocationTimeout = defaultsAndOverrides.defaultInvocationTimeout;
            Dictionary<string, string> context = null;

            //
            // Override the defaults with the proxy properties if a property prefix is defined.
            //
            if(propertyPrefix != null && propertyPrefix.Length > 0)
            {
                Ice.Properties properties = _instance.initializationData().properties;

                //
                // Warn about unknown properties.
                //
                if(properties.getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
                {
                    checkForUnknownProperties(propertyPrefix);
                }

                string property;

                property = propertyPrefix + ".Locator";
                Ice.LocatorPrx locator = Ice.LocatorPrxHelper.uncheckedCast(_communicator.propertyToProxy(property));
                if(locator != null)
                {
                    if(!((Ice.ObjectPrxHelperBase)locator).iceReference().getEncoding().Equals(encoding))
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
                if(router != null)
                {
                    if(propertyPrefix.EndsWith(".Router", StringComparison.Ordinal))
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
                if(properties.getProperty(property).Length > 0)
                {
                    string type = properties.getProperty(property);
                    if(type.Equals("Random"))
                    {
                        endpointSelection = Ice.EndpointSelectionType.Random;
                    }
                    else if(type.Equals("Ordered"))
                    {
                        endpointSelection = Ice.EndpointSelectionType.Ordered;
                    }
                    else
                    {
                        throw new Ice.EndpointSelectionTypeParseException("illegal value `" + type +
                                                                          "'; expected `Random' or `Ordered'");
                    }
                }

                property = propertyPrefix + ".LocatorCacheTimeout";
                string val = properties.getProperty(property);
                if(val.Length > 0)
                {
                    locatorCacheTimeout = properties.getPropertyAsIntWithDefault(property, locatorCacheTimeout);
                    if(locatorCacheTimeout < -1)
                    {
                        locatorCacheTimeout = -1;

                        StringBuilder msg = new StringBuilder("invalid value for ");
                        msg.Append(property);
                        msg.Append(" `");
                        msg.Append(properties.getProperty(property));
                        msg.Append("': defaulting to -1");
                        _instance.initializationData().logger.warning(msg.ToString());
                    }
                }

                property = propertyPrefix + ".InvocationTimeout";
                val = properties.getProperty(property);
                if(val.Length > 0)
                {
                    invocationTimeout = properties.getPropertyAsIntWithDefault(property, invocationTimeout);
                    if(invocationTimeout < 1 && invocationTimeout != -1)
                    {
                        invocationTimeout = -1;

                        StringBuilder msg = new StringBuilder("invalid value for ");
                        msg.Append(property);
                        msg.Append(" `");
                        msg.Append(properties.getProperty(property));
                        msg.Append("': defaulting to -1");
                        _instance.initializationData().logger.warning(msg.ToString());
                    }
                }

                property = propertyPrefix + ".Context.";
                Dictionary<string, string> contexts = properties.getPropertiesForPrefix(property);
                if(contexts.Count != 0)
                {
                    context = new Dictionary<string, string>();
                    foreach(KeyValuePair<string, string> e in contexts)
                    {
                        context.Add(e.Key.Substring(property.Length), e.Value);
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
                                         collocOptimized,
                                         cacheConnection,
                                         preferSecure,
                                         endpointSelection,
                                         locatorCacheTimeout,
                                         invocationTimeout,
                                         context);
        }

        private Instance _instance;
        private Ice.Communicator _communicator;
        private Ice.RouterPrx _defaultRouter;
        private Ice.LocatorPrx _defaultLocator;
    }

}
