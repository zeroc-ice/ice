// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/ArrayUtil",
        "../Ice/BatchRequestQueue",
        "../Ice/Debug",
        "../Ice/HashMap",
        "../Ice/HashUtil",
        "../Ice/OpaqueEndpointI",
        "../Ice/Promise",
        "../Ice/Protocol",
        "../Ice/ReferenceMode",
        "../Ice/StringUtil",
        "../Ice/BuiltinSequences",
        "../Ice/EndpointTypes",
        "../Ice/Identity",
        "../Ice/Router",
        "../Ice/Locator",
        "../Ice/LocalException",
        "../Ice/Version",
        "../Ice/PropertyNames",
        "../Ice/ConnectionRequestHandler",
    ]);

var ArrayUtil = Ice.ArrayUtil;
var Debug = Ice.Debug;
var BatchRequestQueue = Ice.BatchRequestQueue;
var HashMap = Ice.HashMap;
var HashUtil = Ice.HashUtil;
var OpaqueEndpointI = Ice.OpaqueEndpointI;
var Promise = Ice.Promise;
var Protocol = Ice.Protocol;
var RefMode = Ice.ReferenceMode;
var StringUtil = Ice.StringUtil;
var StringSeqHelper = Ice.StringSeqHelper;
var EndpointSelectionType = Ice.EndpointSelectionType;
var Identity = Ice.Identity;
var RouterPrx = Ice.RouterPrx;
var LocatorPrx = Ice.LocatorPrx;
var PropertyNames = Ice.PropertyNames;
var ConnectionRequestHandler = Ice.ConnectionRequestHandler;

var Class = Ice.Class;

var suffixes =
[
    "EndpointSelection",
    "ConnectionCached",
    "PreferSecure",
    "EncodingVersion",
    "LocatorCacheTimeout",
    "InvocationTimeout",
    "Locator",
    "Router",
    "CollocationOptimized"
];

//
// Only for use by Instance
//
var ReferenceFactory = Class({
    __init__: function(instance, communicator)
    {
        this._instance = instance;
        this._communicator = communicator;
        this._defaultRouter = null;
        this._defaultLocator = null;
    },
    create: function(ident, facet, tmpl, endpoints)
    {
        if(ident.name.length === 0 && ident.category.length === 0)
        {
            return null;
        }

        return this.createImpl(ident, facet, tmpl.getMode(), tmpl.getSecure(), tmpl.getProtocol(), tmpl.getEncoding(),
                            endpoints, null, null);
    },
    createWithAdapterId: function(ident, facet, tmpl, adapterId)
    {
        if(ident.name.length === 0 && ident.category.length === 0)
        {
            return null;
        }

        return this.createImpl(ident, facet, tmpl.getMode(), tmpl.getSecure(), tmpl.getProtocol(), tmpl.getEncoding(),
                            null, adapterId, null);
    },
    createFixed: function(ident, fixedConnection)
    {
        if(ident.name.length === 0 && ident.category.length === 0)
        {
            return null;
        }

        //
        // Create new reference
        //
        var ref = new FixedReference(
            this._instance,
            this._communicator,
            ident,
            "", // Facet
            fixedConnection.endpoint().datagram() ? RefMode.ModeDatagram : RefMode.ModeTwoway,
            fixedConnection.endpoint().secure(),
            this._instance.defaultsAndOverrides().defaultEncoding,
            fixedConnection);
        return ref;
    },
    copy: function(r)
    {
        var ident = r.getIdentity();
        if(ident.name.length === 0 && ident.category.length === 0)
        {
            return null;
        }
        return r.clone();
    },
    createFromString: function(s, propertyPrefix)
    {
        if(s === undefined || s === null || s.length === 0)
        {
            return null;
        }

        var delim = " \t\n\r";

        var beg;
        var end = 0;

        beg = StringUtil.findFirstNotOf(s, delim, end);
        if(beg == -1)
        {
            throw new Ice.ProxyParseException("no non-whitespace characters found in `" + s + "'");
        }

        //
        // Extract the identity, which may be enclosed in single
        // or double quotation marks.
        //
        var idstr = null;
        end = StringUtil.checkQuote(s, beg);
        if(end === -1)
        {
            throw new Ice.ProxyParseException("mismatched quotes around identity in `" + s + "'");
        }
        else if(end === 0)
        {
            end = StringUtil.findFirstOf(s, delim + ":@", beg);
            if(end === -1)
            {
                end = s.length;
            }
            idstr = s.substring(beg, end);
        }
        else
        {
            beg++; // Skip leading quote
            idstr = s.substring(beg, end);
            end++; // Skip trailing quote
        }

        if(beg === end)
        {
            throw new Ice.ProxyParseException("no identity in `" + s + "'");
        }

        //
        // Parsing the identity may raise IdentityParseException.
        //
        var ident = Ice.stringToIdentity(idstr);

        if(ident.name.length === 0)
        {
            //
            // An identity with an empty name and a non-empty
            // category is illegal.
            //
            if(ident.category.length > 0)
            {
                throw new Ice.IllegalIdentityException(ident);
            }
            //
            // Treat a stringified proxy containing two double
            // quotes ("") the same as an empty string, i.e.,
            // a null proxy, but only if nothing follows the
            // quotes.
            //
            else if(StringUtil.findFirstNotOf(s, delim, end) != -1)
            {
                throw new Ice.ProxyParseException("invalid characters after identity in `" + s + "'");
            }
            else
            {
                return null;
            }
        }

        var facet = "";
        var mode = RefMode.ModeTwoway;
        var secure = false;
        var encoding = this._instance.defaultsAndOverrides().defaultEncoding;
        var protocol = Ice.Protocol_1_0;
        var adapter = "";

        while(true)
        {
            beg = StringUtil.findFirstNotOf(s, delim, end);
            if(beg === -1)
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
                end = s.length;
            }

            if(beg == end)
            {
                break;
            }

            var option = s.substring(beg, end);
            if(option.length != 2 || option.charAt(0) != '-')
            {
                throw new Ice.ProxyParseException("expected a proxy option but found `" + option + "' in `" + s + "'");
            }

            //
            // Check for the presence of an option argument. The
            // argument may be enclosed in single or double
            // quotation marks.
            //
            var argument = null;
            var argumentBeg = StringUtil.findFirstNotOf(s, delim, end);
            if(argumentBeg != -1)
            {
                var ch = s.charAt(argumentBeg);
                if(ch != "@" && ch != ":" && ch != "-")
                {
                    beg = argumentBeg;
                    end = StringUtil.checkQuote(s, beg);
                    if(end == -1)
                    {
                        throw new Ice.ProxyParseException("mismatched quotes around value for " + option +
                                                        " option in `" + s + "'");
                    }
                    else if(end === 0)
                    {
                        end = StringUtil.findFirstOf(s, delim + ":@", beg);
                        if(end === -1)
                        {
                            end = s.length;
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
                    if(argument === null)
                    {
                        throw new Ice.ProxyParseException("no argument provided for -f option in `" + s + "'");
                    }

                    try
                    {
                        facet = StringUtil.unescapeString(argument, 0, argument.length);
                    }
                    catch(ex)
                    {
                        throw new Ice.ProxyParseException("invalid facet in `" + s + "': " + ex.message);
                    }

                    break;
                }

                case 't':
                {
                    if(argument !== null)
                    {
                        throw new Ice.ProxyParseException("unexpected argument `" + argument +
                                                            "' provided for -t option in `" + s + "'");
                    }
                    mode = RefMode.ModeTwoway;
                    break;
                }

                case 'o':
                {
                    if(argument !== null)
                    {
                        throw new Ice.ProxyParseException("unexpected argument `" + argument +
                                                            "' provided for -o option in `" + s + "'");
                    }
                    mode = RefMode.ModeOneway;
                    break;
                }

                case 'O':
                {
                    if(argument !== null)
                    {
                        throw new Ice.ProxyParseException("unexpected argument `" + argument +
                                                            "' provided for -O option in `" + s + "'");
                    }
                    mode = RefMode.ModeBatchOneway;
                    break;
                }

                case 'd':
                {
                    if(argument !== null)
                    {
                        throw new Ice.ProxyParseException("unexpected argument `" + argument +
                                                            "' provided for -d option in `" + s + "'");
                    }
                    mode = RefMode.ModeDatagram;
                    break;
                }

                case 'D':
                {
                    if(argument !== null)
                    {
                        throw new Ice.ProxyParseException("unexpected argument `" + argument +
                                                            "' provided for -D option in `" + s + "'");
                    }
                    mode = RefMode.ModeBatchDatagram;
                    break;
                }

                case 's':
                {
                    if(argument !== null)
                    {
                        throw new Ice.ProxyParseException("unexpected argument `" + argument +
                                                            "' provided for -s option in `" + s + "'");
                    }
                    secure = true;
                    break;
                }

                case 'e':
                {
                    if(argument === null)
                    {
                        throw new Ice.ProxyParseException("no argument provided for -e option in `" + s + "'");
                    }

                    try
                    {
                        encoding = Ice.stringToEncodingVersion(argument);
                    }
                    catch(e) // VersionParseException
                    {
                        throw new Ice.ProxyParseException("invalid encoding version `" + argument + "' in `" + s +
                                                            "':\n" + e.str);
                    }
                    break;
                }

                case 'p':
                {
                    if(argument === null)
                    {
                        throw new Ice.ProxyParseException("no argument provided for -p option in `" + s + "'");
                    }

                    try
                    {
                        protocol = Ice.stringToProtocolVersion(argument);
                    }
                    catch(e) // VersionParseException
                    {
                        throw new Ice.ProxyParseException("invalid protocol version `" + argument + "' in `" + s +
                                                            "':\n" + e.str);
                    }
                    break;
                }

                default:
                {
                    throw new Ice.ProxyParseException("unknown option `" + option + "' in `" + s + "'");
                }
            }
        }

        if(beg === -1)
        {
            return this.createImpl(ident, facet, mode, secure, protocol, encoding, null, null, propertyPrefix);
        }

        var endpoints = [];

        if(s.charAt(beg) == ':')
        {
            var unknownEndpoints = [];
            end = beg;

            while(end < s.length && s.charAt(end) == ':')
            {
                beg = end + 1;

                end = beg;
                while(true)
                {
                    end = s.indexOf(':', end);
                    if(end == -1)
                    {
                        end = s.length;
                        break;
                    }
                    else
                    {
                        var quoted = false;
                        var quote = beg;
                        while(true)
                        {
                            quote = s.indexOf("\"", quote);
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

                var es = s.substring(beg, end);
                var endp = this._instance.endpointFactoryManager().create(es, false);
                if(endp !== null)
                {
                    endpoints.push(endp);
                }
                else
                {
                    unknownEndpoints.push(es);
                }
            }
            if(endpoints.length === 0)
            {
                Debug.assert(unknownEndpoints.length > 0);
                throw new Ice.EndpointParseException("invalid endpoint `" + unknownEndpoints[0] + "' in `" + s + "'");
            }
            else if(unknownEndpoints.length !== 0 &&
                this._instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Endpoints", 1) > 0)
            {
                var msg = [];
                msg.push("Proxy contains unknown endpoints:");
                for(var i = 0; i < unknownEndpoints.length; ++i)
                {
                    msg.push(" `");
                    msg.push(unknownEndpoints[i]);
                    msg.push("'");
                }
                this._instance.initializationData().logger.warning(msg.join(""));
            }

            return this.createImpl(ident, facet, mode, secure, protocol, encoding, endpoints, null, propertyPrefix);
        }
        else if(s.charAt(beg) == '@')
        {
            beg = StringUtil.findFirstNotOf(s, delim, beg + 1);
            if(beg == -1)
            {
                throw new Ice.ProxyParseException("missing adapter id in `" + s + "'");
            }

            var adapterstr = null;
            end = StringUtil.checkQuote(s, beg);
            if(end === -1)
            {
                throw new Ice.ProxyParseException("mismatched quotes around adapter id in `" + s + "'");
            }
            else if(end === 0)
            {
                end = StringUtil.findFirstOf(s, delim, beg);
                if(end === -1)
                {
                    end = s.length;
                }
                adapterstr = s.substring(beg, end);
            }
            else
            {
                beg++; // Skip leading quote
                adapterstr = s.substring(beg, end);
                end++; // Skip trailing quote
            }

            if(end !== s.length && StringUtil.findFirstNotOf(s, delim, end) !== -1)
            {
                throw new Ice.ProxyParseException("invalid trailing characters after `" + s.substring(0, end + 1) +
                                                    "' in `" + s + "'");
            }

            try
            {
                adapter = StringUtil.unescapeString(adapterstr, 0, adapterstr.length);
            }
            catch(ex)
            {
                throw new Ice.ProxyParseException("invalid adapter id in `" + s + "': " + ex.message);
            }
            if(adapter.length === 0)
            {
                throw new Ice.ProxyParseException("empty adapter id in `" + s + "'");
            }
            return this.createImpl(ident, facet, mode, secure, protocol, encoding, null, adapter, propertyPrefix);
        }

        throw new Ice.ProxyParseException("malformed proxy `" + s + "'");
    },
    createFromStream: function(ident, s)
    {
        //
        // Don't read the identity here. Operations calling this
        // constructor read the identity, and pass it as a parameter.
        //

        if(ident.name.length === 0 && ident.category.length === 0)
        {
            return null;
        }

        //
        // For compatibility with the old FacetPath.
        //
        var facetPath = StringSeqHelper.read(s); // String[]
        var facet;
        if(facetPath.length > 0)
        {
            if(facetPath.length > 1)
            {
                throw new Ice.ProxyUnmarshalException();
            }
            facet = facetPath[0];
        }
        else
        {
            facet = "";
        }

        var mode = s.readByte();
        if(mode < 0 || mode > RefMode.ModeLast)
        {
            throw new Ice.ProxyUnmarshalException();
        }

        var secure = s.readBool();

        var protocol = null;
        var encoding = null;
        if(!s.getEncoding().equals(Ice.Encoding_1_0))
        {
            protocol = new Ice.ProtocolVersion();
            protocol.__read(s);
            encoding = new Ice.EncodingVersion();
            encoding.__read(s);
        }
        else
        {
            protocol = Ice.Protocol_1_0;
            encoding = Ice.Encoding_1_0;
        }

        var endpoints = null; // EndpointI[]
        var adapterId = null;

        var sz = s.readSize();
        if(sz > 0)
        {
            endpoints = [];
            for(var i = 0; i < sz; i++)
            {
                endpoints[i] = this._instance.endpointFactoryManager().read(s);
            }
        }
        else
        {
            adapterId = s.readString();
        }

        return this.createImpl(ident, facet, mode, secure, protocol, encoding, endpoints, adapterId, null);
    },
    setDefaultRouter: function(defaultRouter)
    {
        if(this._defaultRouter === null ? defaultRouter === null : this._defaultRouter.equals(defaultRouter))
        {
            return this;
        }

        var factory = new ReferenceFactory(this._instance, this._communicator);
        factory._defaultLocator = this._defaultLocator;
        factory._defaultRouter = defaultRouter;
        return factory;
    },
    getDefaultRouter: function()
    {
        return this._defaultRouter;
    },
    setDefaultLocator: function(defaultLocator)
    {
        if(this._defaultLocator === null ? defaultLocator === null : this._defaultLocator.equals(defaultLocator))
        {
            return this;
        }

        var factory = new ReferenceFactory(this._instance, this._communicator);
        factory._defaultRouter = this._defaultRouter;
        factory._defaultLocator = defaultLocator;
        return factory;
    },
    getDefaultLocator: function()
    {
        return this._defaultLocator;
    },
    checkForUnknownProperties: function(prefix)
    {
        var unknownProps = [], i, length;
        //
        // Do not warn about unknown properties for Ice prefixes (Ice, Glacier2, etc.)
        //
        for(i = 0; i < PropertyNames.clPropNames.length; ++i)
        {
            if(prefix.indexOf(PropertyNames.clPropNames[i] + ".") === 0)
            {
                return;
            }
        }

        var props = this._instance.initializationData().properties.getPropertiesForPrefix(prefix + ".");
        for(var e = props.entries; e !== null; e = e.next)
        {
            var valid = false;
            for(i = 0, length = suffixes.length; i < length; ++i)
            {
                if(e.key === prefix + "." + suffixes[i])
                {
                    valid = true;
                    break;
                }
            }

            if(!valid)
            {
                unknownProps.push(e.key);
            }
        }

        if(unknownProps.length > 0)
        {
            var message = [];
            message.push("found unknown properties for proxy '");
            message.push(prefix);
            message.push("':");
            for(i = 0, length = unknownProps.length; i < length; ++i)
            {
                message.push("\n    ");
                message.push(unknownProps[i]);
            }
            this._instance.initializationData().logger.warning(message.join(""));
        }
    },
    createImpl: function(ident, facet, mode, secure, protocol, encoding, endpoints, adapterId, propertyPrefix)
    {
        var defaultsAndOverrides = this._instance.defaultsAndOverrides();

        //
        // Default local proxy options.
        //
        var locatorInfo = null;
        if(this._defaultLocator !== null)
        {
            if(!this._defaultLocator.__reference().getEncoding().equals(encoding))
            {
                locatorInfo = this._instance.locatorManager().find(
                    this._defaultLocator.ice_encodingVersion(encoding));
            }
            else
            {
                locatorInfo = this._instance.locatorManager().find(this._defaultLocator);
            }
        }
        var routerInfo = this._instance.routerManager().find(this._defaultRouter);
        var cacheConnection = true;
        var preferSecure = defaultsAndOverrides.defaultPreferSecure;
        var endpointSelection = defaultsAndOverrides.defaultEndpointSelection;
        var locatorCacheTimeout = defaultsAndOverrides.defaultLocatorCacheTimeout;
        var invocationTimeout = defaultsAndOverrides.defaultInvocationTimeout;

        //
        // Override the defaults with the proxy properties if a property prefix is defined.
        //
        if(propertyPrefix !== null && propertyPrefix.length > 0)
        {
            var properties = this._instance.initializationData().properties;

            //
            // Warn about unknown properties.
            //
            if(properties.getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
            {
                this.checkForUnknownProperties(propertyPrefix);
            }

            var property;

            property = propertyPrefix + ".Locator";
            var locator = LocatorPrx.uncheckedCast(this._communicator.propertyToProxy(property));
            if(locator !== null)
            {
                if(!locator.__reference().getEncoding().equals(encoding))
                {
                    locatorInfo = this._instance.locatorManager().find(locator.ice_encodingVersion(encoding));
                }
                else
                {
                    locatorInfo = this._instance.locatorManager().find(locator);
                }
            }

            property = propertyPrefix + ".Router";
            var router = RouterPrx.uncheckedCast(this._communicator.propertyToProxy(property));
            if(router !== null)
            {
                var match = ".Router";
                if(propertyPrefix.lastIndexOf(match) == propertyPrefix.length - match.length)
                {
                    var s = "`" + property + "=" + properties.getProperty(property) +
                        "': cannot set a router on a router; setting ignored";
                    this._instance.initializationData().logger.warning(s);
                }
                else
                {
                    routerInfo = this._instance.routerManager().find(router);
                }
            }

            property = propertyPrefix + ".ConnectionCached";
            cacheConnection = properties.getPropertyAsIntWithDefault(property, cacheConnection ? 1 : 0) > 0;

            property = propertyPrefix + ".PreferSecure";
            preferSecure = properties.getPropertyAsIntWithDefault(property, preferSecure ? 1 : 0) > 0;

            property = propertyPrefix + ".EndpointSelection";
            if(properties.getProperty(property).length > 0)
            {
                var type = properties.getProperty(property);
                if(type == "Random")
                {
                    endpointSelection = EndpointSelectionType.Random;
                }
                else if(type == "Ordered")
                {
                    endpointSelection = EndpointSelectionType.Ordered;
                }
                else
                {
                    throw new Ice.EndpointSelectionTypeParseException("illegal value `" + type +
                                                                        "'; expected `Random' or `Ordered'");
                }
            }

            property = propertyPrefix + ".LocatorCacheTimeout";
            var value = properties.getProperty(property);
            if(value.length !== 0)
            {
                locatorCacheTimeout = properties.getPropertyAsIntWithDefault(property, locatorCacheTimeout);
                if(locatorCacheTimeout < -1)
                {
                    locatorCacheTimeout = -1;
                    this._instance.initializationData().logger.warning(
                        "invalid value for" + property + "`" + properties.getProperty(property) +
                        "': defaulting to -1");
                }
            }

            property = propertyPrefix + ".InvocationTimeout";
            value = properties.getProperty(property);
            if(value.length !== 0)
            {
                invocationTimeout = properties.getPropertyAsIntWithDefault(property, invocationTimeout);
                if(invocationTimeout < 1 && invocationTimeout !== -1)
                {
                    invocationTimeout = -1;
                    this._instance.initializationData().logger.warning(
                        "invalid value for" + property + "`" + properties.getProperty(property) +
                        "': defaulting to -1");
                }
            }
        }

        //
        // Create new reference
        //
        return new RoutableReference(this._instance,
                                     this._communicator,
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
                                     cacheConnection,
                                     preferSecure,
                                     endpointSelection,
                                     locatorCacheTimeout,
                                     invocationTimeout);
    }
});

Ice.ReferenceFactory = ReferenceFactory;

var Reference = Class({
    __init__: function(instance, communicator, identity, facet, mode, secure, protocol, encoding, invocationTimeout)
    {
        //
        // Validate string arguments.
        //
        Debug.assert(identity === undefined || identity.name !== null);
        Debug.assert(identity === undefined || identity.category !== null);
        Debug.assert(facet === undefined || facet !== null);

        this._instance = instance;
        this._communicator = communicator;
        this._mode = mode;
        this._secure = secure;
        this._identity = identity;
        this._context = Reference._emptyContext;
        this._facet = facet;
        this._protocol = protocol;
        this._encoding = encoding;
        this._invocationTimeout = invocationTimeout;
        this._hashInitialized = false;
        this._overrideCompress = false;
        this._compress = false; // Only used if _overrideCompress == true
    },
    getMode: function()
    {
        return this._mode;
    },
    getSecure: function()
    {
        return this._secure;
    },
    getProtocol: function()
    {
        return this._protocol;
    },
    getEncoding: function()
    {
        return this._encoding;
    },
    getIdentity: function()
    {
        return this._identity;
    },
    getFacet: function()
    {
        return this._facet;
    },
    getInstance: function()
    {
        return this._instance;
    },
    getContext: function()
    {
        return this._context; // HashMap
    },
    getInvocationTimeout: function()
    {
        return this._invocationTimeout;
    },
    getCommunicator: function()
    {
        return this._communicator;
    },
    getEndpoints: function()
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    getAdapterId: function()
    {
        // Abstract
        Debug.assert(false);
        return "";
    },
    getRouterInfo: function()
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    getLocatorInfo: function()
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    getCacheConnection: function()
    {
        // Abstract
        Debug.assert(false);
        return false;
    },
    getPreferSecure: function()
    {
        // Abstract
        Debug.assert(false);
        return false;
    },
    getEndpointSelection: function()
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    getLocatorCacheTimeout: function()
    {
        // Abstract
        Debug.assert(false);
        return 0;
    },
    getConnectionId: function()
    {
        // Abstract
        Debug.assert(false);
        return "";
    },
    //
    // The change* methods (here and in derived classes) create
    // a new reference based on the existing one, with the
    // corresponding value changed.
    //
    changeContext: function(newContext)
    {
        if(newContext === undefined || newContext === null)
        {
            newContext = Reference._emptyContext;
        }
        var r = this._instance.referenceFactory().copy(this);
        if(newContext.size === 0)
        {
            r._context = Reference._emptyContext;
        }
        else
        {
            r._context = new HashMap(newContext);
        }
        return r;
    },
    changeMode: function(newMode)
    {
        if(newMode === this._mode)
        {
            return this;
        }
        var r = this._instance.referenceFactory().copy(this);
        r._mode = newMode;
        return r;
    },
    changeSecure: function(newSecure)
    {
        if(newSecure === this._secure)
        {
            return this;
        }
        var r = this._instance.referenceFactory().copy(this);
        r._secure = newSecure;
        return r;
    },
    changeIdentity: function(newIdentity)
    {
        if(newIdentity.equals(this._identity))
        {
            return this;
        }
        var r = this._instance.referenceFactory().copy(this);
        r._identity = new Identity(newIdentity.name, newIdentity.category);
        return r;
    },
    changeFacet: function(newFacet)
    {
        if(newFacet === this._facet)
        {
            return this;
        }
        var r = this._instance.referenceFactory().copy(this);
        r._facet = newFacet;
        return r;
    },
    changeInvocationTimeout: function(newInvocationTimeout)
    {
        if(newInvocationTimeout === this._invocationTimeout)
        {
            return this;
        }
        var r = this._instance.referenceFactory().copy(this);
        r._invocationTimeout = newInvocationTimeout;
        return r;
    },
    changeEncoding: function(newEncoding)
    {
        if(newEncoding.equals(this._encoding))
        {
            return this;
        }
        var r = this._instance.referenceFactory().copy(this);
        r._encoding = newEncoding;
        return r;
    },
    changeCompress: function(newCompress)
    {
        if(this._overrideCompress && this._compress === newCompress)
        {
            return this;
        }
        var r = this._instance.referenceFactory().copy(this);
        r._compress = newCompress;
        r._overrideCompress = true;
        return r;
    },
    changeAdapterId: function(newAdapterId)
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    changeEndpoints: function(newEndpoints)
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    changeLocator: function(newLocator)
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    changeRouter: function(newRouter)
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    changeCacheConnection: function(newCache)
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    changePreferSecure: function(newPreferSecure)
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    changeEndpointSelection: function(newType)
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    changeLocatorCacheTimeout: function(newTimeout)
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    changeTimeout: function(newTimeout)
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    changeConnectionId: function(connectionId)
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    hashCode: function()
    {
        if(this._hashInitialized)
        {
            return this._hashValue;
        }

        var h = 5381;
        h = HashUtil.addNumber(h, this._mode);
        h = HashUtil.addBoolean(h, this._secure);
        h = HashUtil.addHashable(h, this._identity);
        if(this._context !== null && this._context !== undefined)
        {
            for(var e = this._context.entries; e !== null; e = e.next)
            {
                h = HashUtil.addString(h, e.key);
                h = HashUtil.addString(h, e.value);
            }
        }
        h = HashUtil.addString(h, this._facet);
        h = HashUtil.addBoolean(h, this._overrideCompress);
        if(this._overrideCompress)
        {
            h = HashUtil.addBoolean(h, this._compress);
        }
        h = HashUtil.addHashable(h, this._protocol);
        h = HashUtil.addHashable(h, this._encoding);
        h = HashUtil.addNumber(h, this._invocationTimeout);

        this._hashValue = h;
        this._hashInitialized = true;

        return this._hashValue;
    },
    //
    // Utility methods
    //
    isIndirect: function()
    {
        // Abstract
        Debug.assert(false);
        return false;
    },
    isWellKnown: function()
    {
        // Abstract
        Debug.assert(false);
        return false;
    },
    //
    // Marshal the reference.
    //
    streamWrite: function(s)
    {
        //
        // Don't write the identity here. Operations calling streamWrite
        // write the identity.
        //

        //
        // For compatibility with the old FacetPath.
        //
        if(this._facet.length === 0)
        {
            s.writeSize(0); // Empty string sequence
        }
        else
        {
            s.writeSize(1); // String sequence with one element
            s.writeString(this._facet);
        }

        s.writeByte(this._mode);

        s.writeBool(this._secure);

        if(!s.getEncoding().equals(Ice.Encoding_1_0))
        {
            this._protocol.__write(s);
            this._encoding.__write(s);
        }

        // Derived class writes the remainder of the reference.
    },
    //
    // Convert the reference to its string form.
    //
    toString: function()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        var s = [];

        //
        // If the encoded identity string contains characters which
        // the reference parser uses as separators, then we enclose
        // the identity string in quotes.
        //
        var id = Ice.identityToString(this._identity);
        if(id.search(/[ :@]/) != -1)
        {
            s.push('"');
            s.push(id);
            s.push('"');
        }
        else
        {
            s.push(id);
        }

        if(this._facet.length > 0)
        {
            //
            // If the encoded facet string contains characters which
            // the reference parser uses as separators, then we enclose
            // the facet string in quotes.
            //
            s.push(" -f ");
            var fs = StringUtil.escapeString(this._facet, "");
            if(fs.search(/[ :@]/) != -1)
            {
                s.push('"');
                s.push(fs);
                s.push('"');
            }
            else
            {
                s.push(fs);
            }
        }

        switch(this._mode)
        {
            case RefMode.ModeTwoway:
            {
                s.push(" -t");
                break;
            }

            case RefMode.ModeOneway:
            {
                s.push(" -o");
                break;
            }

            case RefMode.ModeBatchOneway:
            {
                s.push(" -O");
                break;
            }

            case RefMode.ModeDatagram:
            {
                s.push(" -d");
                break;
            }

            case RefMode.ModeBatchDatagram:
            {
                s.push(" -D");
                break;
            }
        }

        if(this._secure)
        {
            s.push(" -s");
        }

        if(!this._protocol.equals(Ice.Protocol_1_0))
        {
            //
            // We only print the protocol if it's not 1.0. It's fine as
            // long as we don't add Ice.Default.ProtocolVersion, a
            // stringified proxy will convert back to the same proxy with
            // stringToProxy.
            //
            s.push(" -p ");
            s.push(Ice.protocolVersionToString(this._protocol));
        }

        //
        // Always print the encoding version to ensure a stringified proxy
        // will convert back to a proxy with the same encoding with
        // stringToProxy (and won't use Ice.Default.EncodingVersion).
        //
        s.push(" -e ");
        s.push(Ice.encodingVersionToString(this._encoding));

        return s.join("");

        // Derived class writes the remainder of the string.
    },
    //
    // Convert the reference to its property form.
    //
    toProperty: function(prefix)
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    getRequestHandler: function(proxy)
    {
        // Abstract
        Debug.assert(false);
    },
    getBatchRequestQueue: function()
    {
        // Abstract
        Debug.assert(false);
    },
    equals: function(r)
    {
        //
        // Note: if(this === r) and type test are performed by each non-abstract derived class.
        //

        if(this._mode !== r._mode)
        {
            return false;
        }

        if(this._secure !== r._secure)
        {
            return false;
        }

        if(!this._identity.equals(r._identity))
        {
            return false;
        }

        if(!this._context.equals(r._context))
        {
            return false;
        }

        if(this._facet !== r._facet)
        {
            return false;
        }

        if(this._overrideCompress !== r._overrideCompress)
        {
        return false;
        }
        if(this._overrideCompress && this._compress !== r._compress)
        {
            return false;
        }

        if(!this._protocol.equals(r._protocol))
        {
            return false;
        }

        if(!this._encoding.equals(r._encoding))
        {
            return false;
        }

        if(this._invocationTimeout !== r._invocationTimeout)
        {
            return false;
        }

        return true;
    },
    clone: function()
    {
        // Abstract
        Debug.assert(false);
        return null;
    },
    copyMembers: function(r)
    {
        //
        // Copy the members that are not passed to the constructor.
        //
        r._context = this._context;
        r._overrideCompress = this._overrideCompress;
        r._compress = this._compress;
    }
});

Reference._emptyContext = new HashMap();
Reference._emptyEndpoints = [];

Ice.Reference = Reference;

var FixedReference = Class(Reference, {
    __init__: function(instance, communicator, identity, facet, mode, secure, encoding, connection)
    {
        Reference.call(this, instance, communicator, identity, facet, mode, secure, Ice.Protocol_1_0, encoding);
        this._fixedConnection = connection;
    },
    getEndpoints: function()
    {
        return Reference._emptyEndpoints;
    },
    getAdapterId: function()
    {
        return "";
    },
    getRouterInfo: function()
    {
        return null;
    },
    getLocatorInfo: function()
    {
        return null;
    },
    getCacheConnection: function()
    {
        return true;
    },
    getPreferSecure: function()
    {
        return false;
    },
    getEndpointSelection: function()
    {
        return EndpointSelectionType.Random;
    },
    getLocatorCacheTimeout: function()
    {
        return 0;
    },
    getConnectionId: function()
    {
        return "";
    },
    changeAdapterId: function(newAdapterId)
    {
        throw new Ice.FixedProxyException();
    },
    changeEndpoints: function(newEndpoints)
    {
        throw new Ice.FixedProxyException();
    },
    changeLocator: function(newLocator)
    {
        throw new Ice.FixedProxyException();
    },
    changeRouter: function(newRouter)
    {
        throw new Ice.FixedProxyException();
    },
    changeCacheConnection: function(newCache)
    {
        throw new Ice.FixedProxyException();
    },
    changePreferSecure: function(prefSec)
    {
        throw new Ice.FixedProxyException();
    },
    changeEndpointSelection: function(newType)
    {
        throw new Ice.FixedProxyException();
    },
    changeLocatorCacheTimeout: function(newTimeout)
    {
        throw new Ice.FixedProxyException();
    },
    changeTimeout: function(newTimeout)
    {
        throw new Ice.FixedProxyException();
    },
    changeConnectionId: function(connectionId)
    {
        throw new Ice.FixedProxyException();
    },
    isIndirect: function()
    {
        return false;
    },
    isWellKnown: function()
    {
        return false;
    },
    streamWrite: function(s)
    {
        throw new Ice.FixedProxyException();
    },
    toString: function()
    {
        throw new Ice.FixedProxyException();
    },
    toProperty: function(prefix)
    {
        throw new Ice.FixedProxyException();
    },
    clone: function()
    {
        var r = new FixedReference(this.getInstance(), this.getCommunicator(), this.getIdentity(), this.getFacet(),
                                    this.getMode(), this.getSecure(), this.getEncoding(), this._fixedConnection);
        this.copyMembers(r);
        return r;
    },
    getRequestHandler: function(proxy)
    {
        switch(this.getMode())
        {
            case RefMode.ModeTwoway:
            case RefMode.ModeOneway:
            case RefMode.ModeBatchOneway:
            {
                if(this._fixedConnection.endpoint().datagram())
                {
                    throw new Ice.NoEndpointException("");
                }
                break;
            }

            case RefMode.ModeDatagram:
            case RefMode.ModeBatchDatagram:
            {
                if(!this._fixedConnection.endpoint().datagram())
                {
                    throw new Ice.NoEndpointException("");
                }
                break;
            }
        }

        //
        // If a secure connection is requested or secure overrides is set,
        // check if the connection is secure.
        //
        var secure;
        var defaultsAndOverrides = this.getInstance().defaultsAndOverrides();
        if(defaultsAndOverrides.overrideSecure)
        {
            secure = defaultsAndOverrides.overrideSecureValue;
        }
        else
        {
            secure = this.getSecure();
        }
        if(secure && !this._fixedConnection.endpoint().secure())
        {
            throw new Ice.NoEndpointException("");
        }

        this._fixedConnection.throwException(); // Throw in case our connection is already destroyed.

        var compress;
        if(defaultsAndOverrides.overrideCompress)
        {
            compress = defaultsAndOverrides.overrideCompressValue;
        }
        else if(this._overrideCompress)
        {
            compress = this._compress;
        }
        else
        {
            compress = this._fixedConnection.endpoint().compress();
        }

        return proxy.__setRequestHandler(new ConnectionRequestHandler(this, this._fixedConnection, compress));
    },
    getBatchRequestQueue: function()
    {
        return this._fixedConnection.getBatchRequestQueue();
    },
    equals: function(rhs)
    {
        if(this === rhs)
        {
            return true;
        }
        if(!(rhs instanceof FixedReference))
        {
            return false;
        }
        if(!Reference.prototype.equals.call(this, rhs))
        {
            return false;
        }
        return this._fixedConnection.equals(rhs._fixedConnection);
    }
});

Ice.FixedReference = FixedReference;

var RoutableReference = Class(Reference, {
    __init__: function(instance, communicator, identity, facet, mode, secure, protocol, encoding, endpoints,
                        adapterId, locatorInfo, routerInfo, cacheConnection, preferSecure, endpointSelection,
                        locatorCacheTimeout, invocationTimeout)
    {
        Reference.call(this, instance, communicator, identity, facet, mode, secure, protocol, encoding,
                        invocationTimeout);
        this._endpoints = endpoints;
        this._adapterId = adapterId;
        this._locatorInfo = locatorInfo;
        this._routerInfo = routerInfo;
        this._cacheConnection = cacheConnection;
        this._preferSecure = preferSecure;
        this._endpointSelection = endpointSelection;
        this._locatorCacheTimeout = locatorCacheTimeout;
        this._overrideTimeout = false;
        this._timeout = -1;

        if(this._endpoints === null)
        {
            this._endpoints = Reference._emptyEndpoints;
        }
        if(this._adapterId === null)
        {
            this._adapterId = "";
        }
        this._connectionId = "";
        Debug.assert(this._adapterId.length === 0 || this._endpoints.length === 0);
    },
    getEndpoints: function()
    {
        return this._endpoints;
    },
    getAdapterId: function()
    {
        return this._adapterId;
    },
    getRouterInfo: function()
    {
        return this._routerInfo;
    },
    getLocatorInfo: function()
    {
        return this._locatorInfo;
    },
    getCacheConnection: function()
    {
        return this._cacheConnection;
    },
    getPreferSecure: function()
    {
        return this._preferSecure;
    },
    getEndpointSelection: function()
    {
        return this._endpointSelection;
    },
    getLocatorCacheTimeout: function()
    {
        return this._locatorCacheTimeout;
    },
    getConnectionId: function()
    {
        return this._connectionId;
    },
    changeEncoding: function(newEncoding)
    {
        var r = Reference.prototype.changeEncoding.call(this, newEncoding);
        if(r !== this)
        {
            var locInfo = r._locatorInfo;
            if(locInfo !== null && !locInfo.getLocator().ice_getEncodingVersion().equals(newEncoding))
            {
                r._locatorInfo = this.getInstance().locatorManager().find(
                    locInfo.getLocator().ice_encodingVersion(newEncoding));
            }
        }
        return r;
    },
    changeCompress: function(newCompress)
    {
        var r = Reference.prototype.changeCompress.call(this, newCompress);
        if(r !== this && this._endpoints.length > 0) // Also override the compress flag on the endpoints if it was updated.
        {
            var newEndpoints = [];
            for(var i = 0; i < this._endpoints.length; i++)
            {
                newEndpoints[i] = this._endpoints[i].changeCompress(newCompress);
            }
            r._endpoints = newEndpoints;
        }
        return r;
    },
    changeAdapterId: function(newAdapterId)
    {
        if(this._adapterId === newAdapterId)
        {
            return this;
        }
        var r = this.getInstance().referenceFactory().copy(this);
        r._adapterId = newAdapterId;
        r._endpoints = Reference._emptyEndpoints;
        return r;
    },
    changeEndpoints: function(newEndpoints)
    {
        if(ArrayUtil.equals(newEndpoints, this._endpoints, function(e1, e2) { return e1.equals(e2); }))
        {
            return this;
        }
        var r = this.getInstance().referenceFactory().copy(this);
        r._endpoints = newEndpoints;
        r._adapterId = "";
        r.applyOverrides(r._endpoints);
        return r;
    },
    changeLocator: function(newLocator)
    {
        var newLocatorInfo = this.getInstance().locatorManager().find(newLocator);
        if(newLocatorInfo !== null && this._locatorInfo !== null && newLocatorInfo.equals(this._locatorInfo))
        {
            return this;
        }
        var r = this.getInstance().referenceFactory().copy(this);
        r._locatorInfo = newLocatorInfo;
        return r;
    },
    changeRouter: function(newRouter)
    {
        var newRouterInfo = this.getInstance().routerManager().find(newRouter);
        if(newRouterInfo !== null && this._routerInfo !== null && newRouterInfo.equals(this._routerInfo))
        {
            return this;
        }
        var r = this.getInstance().referenceFactory().copy(this);
        r._routerInfo = newRouterInfo;
        return r;
    },
    changeCacheConnection: function(newCache)
    {
        if(newCache === this._cacheConnection)
        {
            return this;
        }
        var r = this.getInstance().referenceFactory().copy(this);
        r._cacheConnection = newCache;
        return r;
    },
    changePreferSecure: function(newPreferSecure)
    {
        if(newPreferSecure === this._preferSecure)
        {
            return this;
        }
        var r = this.getInstance().referenceFactory().copy(this);
        r._preferSecure = newPreferSecure;
        return r;
    },
    changeEndpointSelection: function(newType)
    {
        if(newType === this._endpointSelection)
        {
            return this;
        }
        var r = this.getInstance().referenceFactory().copy(this);
        r._endpointSelection = newType;
        return r;
    },
    changeLocatorCacheTimeout: function(newTimeout)
    {
        if(this._locatorCacheTimeout === newTimeout)
        {
            return this;
        }
        var r = this.getInstance().referenceFactory().copy(this);
        r._locatorCacheTimeout = newTimeout;
        return r;
    },
    changeTimeout: function(newTimeout)
    {
        if(this._overrideTimeout && this._timeout === newTimeout)
        {
            return this;
        }
        var r = this.getInstance().referenceFactory().copy(this);
        r._timeout = newTimeout;
        r._overrideTimeout = true;
        if(this._endpoints.length > 0)
        {
            var newEndpoints = [];
            for(var i = 0; i < this._endpoints.length; i++)
            {
                newEndpoints[i] = this._endpoints[i].changeTimeout(newTimeout);
            }
            r._endpoints = newEndpoints;
        }
        return r;
    },
    changeConnectionId: function(id)
    {
        if(this._connectionId === id)
        {
            return this;
        }
        var r = this.getInstance().referenceFactory().copy(this);
        r._connectionId = id;
        if(this._endpoints.length > 0)
        {
            var newEndpoints = [];
            for(var i = 0; i < this._endpoints.length; i++)
            {
                newEndpoints[i] = this._endpoints[i].changeConnectionId(id);
            }
            r._endpoints = newEndpoints;
        }
        return r;
    },
    isIndirect: function()
    {
        return this._endpoints.length === 0;
    },
    isWellKnown: function()
    {
        return this._endpoints.length === 0 && this._adapterId.length === 0;
    },
    streamWrite: function(s)
    {
        Reference.prototype.streamWrite.call(this, s);

        s.writeSize(this._endpoints.length);
        if(this._endpoints.length > 0)
        {
            Debug.assert(this._adapterId.length === 0);
            for(var i = 0; i < this._endpoints.length; ++i)
            {
                s.writeShort(this._endpoints[i].type());
                this._endpoints[i].streamWrite(s);
            }
        }
        else
        {
            s.writeString(this._adapterId); // Adapter id.
        }
    },
    toString: function()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        var s = [];
        s.push(Reference.prototype.toString.call(this));
        if(this._endpoints.length > 0)
        {
            for(var i = 0; i < this._endpoints.length; ++i)
            {
                var endp = this._endpoints[i].toString();
                if(endp !== null && endp.length > 0)
                {
                    s.push(':');
                    s.push(endp);
                }
            }
        }
        else if(this._adapterId.length > 0)
        {
            s.push(" @ ");

            //
            // If the encoded adapter id string contains characters which
            // the reference parser uses as separators, then we enclose
            // the adapter id string in quotes.
            //
            var a = StringUtil.escapeString(this._adapterId, null);
            if(a.search(/[ :@]/) != -1)
            {
                s.push('"');
                s.push(a);
                s.push('"');
            }
            else
            {
                s.push(a);
            }
        }
        return s.join("");
    },
    toProperty: function(prefix)
    {
        var properties = new HashMap(), e;

        properties.set(prefix, this.toString());
        properties.set(prefix + ".CollocationOptimized", "0");
        properties.set(prefix + ".ConnectionCached", this._cacheConnection ? "1" : "0");
        properties.set(prefix + ".PreferSecure", this._preferSecure ? "1" : "0");
        properties.set(prefix + ".EndpointSelection",
                    this._endpointSelection === EndpointSelectionType.Random ? "Random" : "Ordered");

        properties.set(prefix + ".LocatorCacheTimeout", "" + this._locatorCacheTimeout);
        properties.set(prefix + ".InvocationTimeout", "" + this.getInvocationTimeout());

        if(this._routerInfo !== null)
        {
            var h = this._routerInfo.getRouter();
            var routerProperties = h.__reference().toProperty(prefix + ".Router");
            for(e = routerProperties.entries; e !== null; e = e.next)
            {
                properties.set(e.key, e.value);
            }
        }

        if(this._locatorInfo !== null)
        {
            var p = this._locatorInfo.getLocator();
            var locatorProperties = p.__reference().toProperty(prefix + ".Locator");
            for(e = locatorProperties.entries; e !== null; e = e.next)
            {
                properties.set(e.key, e.value);
            }
        }

        return properties;
    },
    hashCode: function()
    {
        if(!this._hashInitialized)
        {
            Reference.prototype.hashCode.call(this); // Initializes _hashValue.
            this._hashValue = HashUtil.addString(this._hashValue, this._adapterId);
        }
        return this._hashValue;
    },
    equals: function(rhs)
    {
        if(this === rhs)
        {
            return true;
        }
        if(!(rhs instanceof RoutableReference))
        {
            return false;
        }

        if(!Reference.prototype.equals.call(this, rhs))
        {
            return false;
        }

        if(this._locatorInfo === null ? rhs._locatorInfo !== null : !this._locatorInfo.equals(rhs._locatorInfo))
        {
            return false;
        }
        if(this._routerInfo === null ? rhs._routerInfo !== null : !this._routerInfo.equals(rhs._routerInfo))
        {
            return false;
        }
        if(this._cacheConnection !== rhs._cacheConnection)
        {
            return false;
        }
        if(this._preferSecure !== rhs._preferSecure)
        {
            return false;
        }
        if(this._endpointSelection !== rhs._endpointSelection)
        {
            return false;
        }
        if(this._locatorCacheTimeout !== rhs._locatorCacheTimeout)
        {
            return false;
        }
        if(this._connectionId !== rhs._connectionId)
        {
            return false;
        }
        if(this._overrideTimeout !== rhs._overrideTimeout)
        {
            return false;
        }
        if(this._overrideTimeout && this._timeout !== rhs._timeout)
        {
            return false;
        }
        if(!ArrayUtil.equals(this._endpoints, rhs._endpoints, function(e1, e2) { return e1.equals(e2); }))
        {
            return false;
        }
        if(this._adapterId !== rhs._adapterId)
        {
            return false;
        }
        return true;
    },
    getRequestHandler: function(proxy)
    {
        return this._instance.requestHandlerFactory().getRequestHandler(this, proxy);
    },
    getBatchRequestQueue: function()
    {
        return new BatchRequestQueue(this._instance, this._mode === RefMode.ModeBatchDatagram);
    },
    getConnection: function()
    {
        var promise = new Promise(); // success callback receives (connection, compress)

        if(this._routerInfo !== null)
        {
            //
            // If we route, we send everything to the router's client
            // proxy endpoints.
            //
            var self = this;
            this._routerInfo.getClientEndpoints().then(
                function(endpts)
                {
                    if(endpts.length > 0)
                    {
                        self.applyOverrides(endpts);
                        self.createConnection(endpts).then(
                            function(connection, compress)
                            {
                                promise.succeed(connection, compress);
                            },
                            function(ex)
                            {
                                promise.fail(ex);
                            });
                    }
                    else
                    {
                        self.getConnectionNoRouterInfo(promise);
                    }
                }).exception(
                    function(ex)
                    {
                        promise.fail(ex);
                    });
        }
        else
        {
            this.getConnectionNoRouterInfo(promise);
        }

        return promise;
    },
    getConnectionNoRouterInfo: function(promise)
    {
        if(this._endpoints.length > 0)
        {
            this.createConnection(this._endpoints).then(
                function(connection, compress)
                {
                    promise.succeed(connection, compress);
                }).exception(
                    function(ex)
                    {
                        promise.fail(ex);
                    });
            return;
        }

        var self = this;
        if(this._locatorInfo !== null)
        {
            this._locatorInfo.getEndpoints(this, null, this._locatorCacheTimeout).then(
                function(endpoints, cached)
                {
                    if(endpoints.length === 0)
                    {
                        promise.fail(new Ice.NoEndpointException(self.toString()));
                        return;
                    }

                    self.applyOverrides(endpoints);
                    self.createConnection(endpoints).then(
                        function(connection, compress)
                        {
                            promise.succeed(connection, compress);
                        },
                        function(ex)
                        {
                            if(ex instanceof Ice.NoEndpointException)
                            {
                                //
                                // No need to retry if there's no endpoints.
                                //
                                promise.fail(ex);
                            }
                            else
                            {
                                Debug.assert(self._locatorInfo !== null);
                                self.getLocatorInfo().clearCache(self);
                                if(cached)
                                {
                                    var traceLevels = self.getInstance().traceLevels();
                                    if(traceLevels.retry >= 2)
                                    {
                                        var s = "connection to cached endpoints failed\n" +
                                                "removing endpoints from cache and trying one more time\n" +
                                                ex.toString();
                                        self.getInstance().initializationData().logger.trace(traceLevels.retryCat, s);
                                    }
                                    self.getConnectionNoRouterInfo(promise); // Retry.
                                    return;
                                }
                                promise.fail(ex);
                            }
                        });
                }).exception(
                    function(ex)
                    {
                        promise.fail(ex);
                    });
        }
        else
        {
            promise.fail(new Ice.NoEndpointException(this.toString()));
        }
    },
    clone: function()
    {
        var r = new RoutableReference(this.getInstance(),
                                        this.getCommunicator(),
                                        this.getIdentity(),
                                        this.getFacet(),
                                        this.getMode(),
                                        this.getSecure(),
                                        this.getProtocol(),
                                        this.getEncoding(),
                                        this._endpoints,
                                        this._adapterId,
                                        this._locatorInfo,
                                        this._routerInfo,
                                        this._cacheConnection,
                                        this._preferSecure,
                                        this._endpointSelection,
                                        this._locatorCacheTimeout,
                                        this._invocationTimeout);
        this.copyMembers(r);
        return r;
    },
    copyMembers: function(rhs)
    {
        //
        // Copy the members that are not passed to the constructor.
        //
        Reference.prototype.copyMembers.call(this, rhs);
        rhs._overrideTimeout = this._overrideTimeout;
        rhs._timeout = this._timeout;
        rhs._connectionId = this._connectionId;
    },
    applyOverrides: function(endpts)
    {
        //
        // Apply the endpoint overrides to each endpoint.
        //
        for(var i = 0; i < endpts.length; ++i)
        {
            endpts[i] = endpts[i].changeConnectionId(this._connectionId);
            if(this._overrideCompress)
            {
                endpts[i] = endpts[i].changeCompress(this._compress);
            }
            if(this._overrideTimeout)
            {
                endpts[i] = endpts[i].changeTimeout(this._timeout);
            }
        }
    },
    filterEndpoints: function(allEndpoints)
    {
        var endpoints = [];

        //
        // Filter out opaque endpoints or endpoints which can't connect.
        //
        for(var i = 0; i < allEndpoints.length; ++i)
        {
            if(!(allEndpoints[i] instanceof OpaqueEndpointI) && allEndpoints[i].connectable())
            {
                endpoints.push(allEndpoints[i]);
            }
        }

        //
        // Filter out endpoints according to the mode of the reference.
        //
        switch(this.getMode())
        {
            case RefMode.ModeTwoway:
            case RefMode.ModeOneway:
            case RefMode.ModeBatchOneway:
            {
                //
                // Filter out datagram endpoints.
                //
                endpoints = ArrayUtil.filter(endpoints, function(e, index, arr) { return !e.datagram(); });
                break;
            }

            case RefMode.ModeDatagram:
            case RefMode.ModeBatchDatagram:
            {
                //
                // Filter out non-datagram endpoints.
                //
                endpoints = ArrayUtil.filter(endpoints, function(e, index, arr) { return e.datagram(); });
                break;
            }
        }

        //
        // Sort the endpoints according to the endpoint selection type.
        //
        switch(this.getEndpointSelection())
        {
            case EndpointSelectionType.Random:
            {
                //
                // Shuffle the endpoints.
                //
                ArrayUtil.shuffle(endpoints);
                break;
            }
            case EndpointSelectionType.Ordered:
            {
                // Nothing to do.
                break;
            }
            default:
            {
                Debug.assert(false);
                break;
            }
        }

        //
        // If a secure connection is requested or secure overrides is
        // set, remove all non-secure endpoints. Otherwise if preferSecure is set
        // make secure endpoints prefered. By default make non-secure
        // endpoints preferred over secure endpoints.
        //
        var overrides = this.getInstance().defaultsAndOverrides();
        if(overrides.overrideSecure ? overrides.overrideSecureValue : this.getSecure())
        {
            endpoints = ArrayUtil.filter(endpoints, function(e, index, arr) { return e.secure(); });
        }
        else
        {
            var preferSecure = this.getPreferSecure();
            var compare = function(e1, e2)
            {
                var ls = e1.secure();
                var rs = e2.secure();
                if((ls && rs) || (!ls && !rs))
                {
                    return 0;
                }
                else if(!ls && rs)
                {
                    return preferSecure ? 1 : -1;
                }
                else
                {
                    return preferSecure ? -1 : 1;
                }
            };
            endpoints.sort(compare);
        }
        return endpoints;
    },
    createConnection: function(allEndpoints)
    {
        var endpoints = this.filterEndpoints(allEndpoints);
        if(endpoints.length === 0)
        {
            return new Promise().fail(new Ice.NoEndpointException(this.toString()));
        }

        //
        // Finally, create the connection.
        //
        var promise = new Promise();
        var factory = this.getInstance().outgoingConnectionFactory();
        var cb;
        if(this.getCacheConnection() || endpoints.length == 1)
        {
            //
            // Get an existing connection or create one if there's no
            // existing connection to one of the given endpoints.
            //
            cb = new CreateConnectionCallback(this, null, promise);
            factory.create(endpoints, false, this.getEndpointSelection()).then(
                function(connection, compress)
                {
                    cb.setConnection(connection, compress);
                }).exception(
                    function(ex)
                    {
                        cb.setException(ex);
                    });
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
            var v = [ endpoints[0] ];
            cb = new CreateConnectionCallback(this, endpoints, promise);
            factory.create(v, true, this.getEndpointSelection()).then(
                function(connection, compress)
                {
                    cb.setConnection(connection, compress);
                }).exception(
                    function(ex)
                    {
                        cb.setException(ex);
                    });
        }

        return promise;
    }
});

Ice.RoutableReference = RoutableReference;
module.exports.Ice = Ice;

var CreateConnectionCallback = Class({
    __init__: function(r, endpoints, promise)
    {
        this.ref = r;
        this.endpoints = endpoints;
        this.promise = promise;
        this.i = 0;
        this.exception = null;
    },
    setConnection: function(connection, compress)
    {
        //
        // If we have a router, set the object adapter for this router
        // (if any) to the new connection, so that callbacks from the
        // router can be received over this new connection.
        //
        if(this.ref.getRouterInfo() !== null && this.ref.getRouterInfo().getAdapter() !== null)
        {
            connection.setAdapter(this.ref.getRouterInfo().getAdapter());
        }
        this.promise.succeed(connection, compress);
    },
    setException: function(ex)
    {
        if(this.exception === null)
        {
            this.exception = ex;
        }

        if(this.endpoints === null || ++this.i === this.endpoints.length)
        {
            this.promise.fail(this.exception);
            return;
        }

        var more = this.i != this.endpoints.length - 1;
        var arr = [ this.endpoints[this.i] ];
        var self = this;
        this.ref.getInstance().outgoingConnectionFactory().create(arr, more, this.ref.getEndpointSelection()).then(
            function(connection, compress)
            {
                self.setConnection(connection, compress);
            }).exception(
                function(ex)
                {
                    self.setException(ex);
                });
    }
});
