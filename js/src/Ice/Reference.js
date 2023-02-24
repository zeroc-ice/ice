//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/ArrayUtil");
require("../Ice/BatchRequestQueue");
require("../Ice/BuiltinSequences");
require("../Ice/ConnectionRequestHandler");
require("../Ice/Debug");
require("../Ice/EndpointTypes");
require("../Ice/HashUtil");
require("../Ice/Identity");
require("../Ice/LocalException");
require("../Ice/Locator");
require("../Ice/MapUtil");
require("../Ice/OpaqueEndpointI");
require("../Ice/Promise");
require("../Ice/PropertyNames");
require("../Ice/ReferenceMode");
require("../Ice/Router");
require("../Ice/StringUtil");
require("../Ice/Version");

const ArrayUtil = Ice.ArrayUtil;
const BatchRequestQueue = Ice.BatchRequestQueue;
const ConnectionRequestHandler = Ice.ConnectionRequestHandler;
const Debug = Ice.Debug;
const EndpointSelectionType = Ice.EndpointSelectionType;
const HashUtil = Ice.HashUtil;
const Identity = Ice.Identity;
const LocatorPrx = Ice.LocatorPrx;
const MapUtil = Ice.MapUtil;
const OpaqueEndpointI = Ice.OpaqueEndpointI;
const PropertyNames = Ice.PropertyNames;
const RefMode = Ice.ReferenceMode;
const RouterPrx = Ice.RouterPrx;
const StringSeqHelper = Ice.StringSeqHelper;
const StringUtil = Ice.StringUtil;

const suffixes =
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
class ReferenceFactory
{
    constructor(instance, communicator)
    {
        this._instance = instance;
        this._communicator = communicator;
        this._defaultRouter = null;
        this._defaultLocator = null;
    }

    create(ident, facet, tmpl, endpoints)
    {
        if(ident.name.length === 0 && ident.category.length === 0)
        {
            return null;
        }

        return this.createImpl(ident, facet, tmpl.getMode(), tmpl.getSecure(), tmpl.getProtocol(), tmpl.getEncoding(),
                               endpoints, null, null);
    }

    createWithAdapterId(ident, facet, tmpl, adapterId)
    {
        if(ident.name.length === 0 && ident.category.length === 0)
        {
            return null;
        }

        return this.createImpl(ident, facet, tmpl.getMode(), tmpl.getSecure(), tmpl.getProtocol(), tmpl.getEncoding(),
                               null, adapterId, null);
    }

    createFixed(ident, fixedConnection)
    {
        if(ident.name.length === 0 && ident.category.length === 0)
        {
            return null;
        }

        //
        // Create new reference
        //
        return new FixedReference(
            this._instance,
            this._communicator,
            ident,
            "", // Facet
            fixedConnection.endpoint().datagram() ? RefMode.ModeDatagram : RefMode.ModeTwoway,
            fixedConnection.endpoint().secure(),
            Ice.Protocol_1_0,
            this._instance.defaultsAndOverrides().defaultEncoding,
            fixedConnection,
            -1,
            null);
    }

    copy(r)
    {
        const ident = r.getIdentity();
        if(ident.name.length === 0 && ident.category.length === 0)
        {
            return null;
        }
        return r.clone();
    }

    createFromString(s, propertyPrefix)
    {
        if(s === undefined || s === null || s.length === 0)
        {
            return null;
        }

        const delim = " \t\n\r";

        let end = 0;
        let beg = StringUtil.findFirstNotOf(s, delim, end);
        if(beg == -1)
        {
            throw new Ice.ProxyParseException("no non-whitespace characters found in `" + s + "'");
        }

        //
        // Extract the identity, which may be enclosed in single
        // or double quotation marks.
        //
        let idstr = null;
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
        const ident = Ice.stringToIdentity(idstr);

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

        let facet = "";
        let mode = RefMode.ModeTwoway;
        let secure = false;
        let encoding = this._instance.defaultsAndOverrides().defaultEncoding;
        let protocol = Ice.Protocol_1_0;
        let adapter = "";

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

            const option = s.substring(beg, end);
            if(option.length != 2 || option.charAt(0) != '-')
            {
                throw new Ice.ProxyParseException("expected a proxy option but found `" + option + "' in `" + s + "'");
            }

            //
            // Check for the presence of an option argument. The
            // argument may be enclosed in single or double
            // quotation marks.
            //
            let argument = null;
            const argumentBeg = StringUtil.findFirstNotOf(s, delim, end);
            if(argumentBeg != -1)
            {
                const ch = s.charAt(argumentBeg);
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

        const endpoints = [];

        if(s.charAt(beg) == ':')
        {
            const unknownEndpoints = [];
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
                        let quoted = false;
                        let quote = beg;
                        while(true)
                        {
                            quote = s.indexOf("\"", quote);
                            if(quote == -1 || end < quote)
                            {
                                break;
                            }
                            else
                            {
                                quote = s.indexOf("\"", ++quote);
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

                const es = s.substring(beg, end);
                const endp = this._instance.endpointFactoryManager().create(es, false);
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
                const msg = [];
                msg.push("Proxy contains unknown endpoints:");
                unknownEndpoints.forEach(unknownEndpoint =>
                    {
                        msg.push(" `");
                        msg.push(unknownEndpoint);
                        msg.push("'");
                    });
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

            let adapterstr = null;
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
    }

    createFromStream(ident, s)
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
        const facetPath = StringSeqHelper.read(s); // String[]
        let facet;
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

        const mode = s.readByte();
        if(mode < 0 || mode > RefMode.ModeLast)
        {
            throw new Ice.ProxyUnmarshalException();
        }

        const secure = s.readBool();

        let protocol = null;
        let encoding = null;
        if(!s.getEncoding().equals(Ice.Encoding_1_0))
        {
            protocol = new Ice.ProtocolVersion();
            protocol._read(s);
            encoding = new Ice.EncodingVersion();
            encoding._read(s);
        }
        else
        {
            protocol = Ice.Protocol_1_0;
            encoding = Ice.Encoding_1_0;
        }

        let endpoints = null; // EndpointI[]
        let adapterId = null;

        const sz = s.readSize();
        if(sz > 0)
        {
            endpoints = [];
            for(let i = 0; i < sz; i++)
            {
                endpoints[i] = this._instance.endpointFactoryManager().read(s);
            }
        }
        else
        {
            adapterId = s.readString();
        }

        return this.createImpl(ident, facet, mode, secure, protocol, encoding, endpoints, adapterId, null);
    }

    setDefaultRouter(defaultRouter)
    {
        if(this._defaultRouter === null ? defaultRouter === null : this._defaultRouter.equals(defaultRouter))
        {
            return this;
        }

        const factory = new ReferenceFactory(this._instance, this._communicator);
        factory._defaultLocator = this._defaultLocator;
        factory._defaultRouter = defaultRouter;
        return factory;
    }

    getDefaultRouter()
    {
        return this._defaultRouter;
    }

    setDefaultLocator(defaultLocator)
    {
        if(this._defaultLocator === null ? defaultLocator === null : this._defaultLocator.equals(defaultLocator))
        {
            return this;
        }

        const factory = new ReferenceFactory(this._instance, this._communicator);
        factory._defaultRouter = this._defaultRouter;
        factory._defaultLocator = defaultLocator;
        return factory;
    }

    getDefaultLocator()
    {
        return this._defaultLocator;
    }

    checkForUnknownProperties(prefix)
    {
        let unknownProps = [];
        //
        // Do not warn about unknown properties for Ice prefixes (Ice, Glacier2, etc.)
        //
        for(let i = 0; i < PropertyNames.clPropNames.length; ++i)
        {
            if(prefix.indexOf(PropertyNames.clPropNames[i] + ".") === 0)
            {
                return;
            }
        }

        const properties = this._instance.initializationData().properties.getPropertiesForPrefix(prefix + ".");
        unknownProps = unknownProps.concat(Array.from(properties.keys()).filter(
            key => !suffixes.some(suffix => key === prefix + "." + suffix)));
        if(unknownProps.length > 0)
        {
            const message = [];
            message.push("found unknown properties for proxy '");
            message.push(prefix);
            message.push("':");
            unknownProps.forEach(unknownProp => message.push("\n    ", unknownProp));
            this._instance.initializationData().logger.warning(message.join(""));
        }
    }

    createImpl(ident, facet, mode, secure, protocol, encoding, endpoints, adapterId, propertyPrefix)
    {
        const defaultsAndOverrides = this._instance.defaultsAndOverrides();

        //
        // Default local proxy options.
        //
        let locatorInfo = null;
        if(this._defaultLocator !== null)
        {
            if(!this._defaultLocator._getReference().getEncoding().equals(encoding))
            {
                locatorInfo = this._instance.locatorManager().find(
                    this._defaultLocator.ice_encodingVersion(encoding));
            }
            else
            {
                locatorInfo = this._instance.locatorManager().find(this._defaultLocator);
            }
        }
        let routerInfo = this._instance.routerManager().find(this._defaultRouter);
        let cacheConnection = true;
        let preferSecure = defaultsAndOverrides.defaultPreferSecure;
        let endpointSelection = defaultsAndOverrides.defaultEndpointSelection;
        let locatorCacheTimeout = defaultsAndOverrides.defaultLocatorCacheTimeout;
        let invocationTimeout = defaultsAndOverrides.defaultInvocationTimeout;

        //
        // Override the defaults with the proxy properties if a property prefix is defined.
        //
        if(propertyPrefix !== null && propertyPrefix.length > 0)
        {
            const properties = this._instance.initializationData().properties;

            //
            // Warn about unknown properties.
            //
            if(properties.getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
            {
                this.checkForUnknownProperties(propertyPrefix);
            }

            let property = propertyPrefix + ".Locator";
            const locator = LocatorPrx.uncheckedCast(this._communicator.propertyToProxy(property));
            if(locator !== null)
            {
                if(!locator._getReference().getEncoding().equals(encoding))
                {
                    locatorInfo = this._instance.locatorManager().find(locator.ice_encodingVersion(encoding));
                }
                else
                {
                    locatorInfo = this._instance.locatorManager().find(locator);
                }
            }

            property = propertyPrefix + ".Router";
            const router = RouterPrx.uncheckedCast(this._communicator.propertyToProxy(property));
            if(router !== null)
            {
                if(propertyPrefix.endsWith("Router"))
                {
                    this._instance.initializationData().logger.warning(
                        "`" + property + "=" + properties.getProperty(property) +
                        "': cannot set a router on a router; setting ignored");
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
                const type = properties.getProperty(property);
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
            let value = properties.getProperty(property);
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
}

Ice.ReferenceFactory = ReferenceFactory;

class Reference
{
    constructor(instance, communicator, identity, facet, mode, secure, protocol, encoding, invocationTimeout, context)
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
        this._context = context === undefined ? Reference._emptyContext : context;
        this._facet = facet;
        this._protocol = protocol;
        this._encoding = encoding;
        this._invocationTimeout = invocationTimeout;
        this._hashInitialized = false;
    }

    getMode()
    {
        return this._mode;
    }

    getSecure()
    {
        return this._secure;
    }

    getProtocol()
    {
        return this._protocol;
    }

    getEncoding()
    {
        return this._encoding;
    }

    getIdentity()
    {
        return this._identity;
    }

    getFacet()
    {
        return this._facet;
    }

    getInstance()
    {
        return this._instance;
    }

    getContext()
    {
        return this._context; // Map
    }

    getInvocationTimeout()
    {
        return this._invocationTimeout;
    }

    getCommunicator()
    {
        return this._communicator;
    }

    getEndpoints()
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    getAdapterId()
    {
        // Abstract
        Debug.assert(false);
        return "";
    }

    getRouterInfo()
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    getLocatorInfo()
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    getCacheConnection()
    {
        // Abstract
        Debug.assert(false);
        return false;
    }

    getPreferSecure()
    {
        // Abstract
        Debug.assert(false);
        return false;
    }

    getEndpointSelection()
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    getLocatorCacheTimeout()
    {
        // Abstract
        Debug.assert(false);
        return 0;
    }

    getConnectionId()
    {
        // Abstract
        Debug.assert(false);
        return "";
    }

    getTimeout()
    {
        // Abstract
        Debug.assert(false);
        return "";
    }

    //
    // The change* methods (here and in derived classes) create
    // a new reference based on the existing one, with the
    // corresponding value changed.
    //
    changeContext(newContext)
    {
        if(newContext === undefined || newContext === null)
        {
            newContext = Reference._emptyContext;
        }
        const r = this._instance.referenceFactory().copy(this);
        if(newContext.size === 0)
        {
            r._context = Reference._emptyContext;
        }
        else
        {
            r._context = new Map(newContext);
        }
        return r;
    }

    changeMode(newMode)
    {
        if(newMode === this._mode)
        {
            return this;
        }
        const r = this._instance.referenceFactory().copy(this);
        r._mode = newMode;
        return r;
    }

    changeSecure(newSecure)
    {
        if(newSecure === this._secure)
        {
            return this;
        }
        const r = this._instance.referenceFactory().copy(this);
        r._secure = newSecure;
        return r;
    }

    changeIdentity(newIdentity)
    {
        if(newIdentity.equals(this._identity))
        {
            return this;
        }
        const r = this._instance.referenceFactory().copy(this);
        r._identity = new Identity(newIdentity.name, newIdentity.category);
        return r;
    }

    changeFacet(newFacet)
    {
        if(newFacet === this._facet)
        {
            return this;
        }
        const r = this._instance.referenceFactory().copy(this);
        r._facet = newFacet;
        return r;
    }

    changeInvocationTimeout(newInvocationTimeout)
    {
        if(newInvocationTimeout === this._invocationTimeout)
        {
            return this;
        }
        const r = this._instance.referenceFactory().copy(this);
        r._invocationTimeout = newInvocationTimeout;
        return r;
    }

    changeEncoding(newEncoding)
    {
        if(newEncoding.equals(this._encoding))
        {
            return this;
        }
        const r = this._instance.referenceFactory().copy(this);
        r._encoding = newEncoding;
        return r;
    }

    changeAdapterId(newAdapterId)
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    changeEndpoints(newEndpoints)
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    changeLocator(newLocator)
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    changeRouter(newRouter)
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    changeCacheConnection(newCache)
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    changePreferSecure(newPreferSecure)
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    changeEndpointSelection(newType)
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    changeLocatorCacheTimeout(newTimeout)
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    changeTimeout(newTimeout)
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    changeConnectionId(connectionId)
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    changeConnection(connection)
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    hashCode()
    {
        if(this._hashInitialized)
        {
            return this._hashValue;
        }

        let h = 5381;
        h = HashUtil.addNumber(h, this._mode);
        h = HashUtil.addBoolean(h, this._secure);
        h = HashUtil.addHashable(h, this._identity);
        if(this._context !== null && this._context !== undefined)
        {
            for(const [key, value] of this._context)
            {
                h = HashUtil.addString(h, key);
                h = HashUtil.addString(h, value);
            }
        }
        h = HashUtil.addString(h, this._facet);
        h = HashUtil.addHashable(h, this._protocol);
        h = HashUtil.addHashable(h, this._encoding);
        h = HashUtil.addNumber(h, this._invocationTimeout);

        this._hashValue = h;
        this._hashInitialized = true;

        return this._hashValue;
    }

    //
    // Utility methods
    //
    isIndirect()
    {
        // Abstract
        Debug.assert(false);
        return false;
    }

    isWellKnown()
    {
        // Abstract
        Debug.assert(false);
        return false;
    }

    //
    // Marshal the reference.
    //
    streamWrite(s)
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
            this._protocol._write(s);
            this._encoding._write(s);
        }

        // Derived class writes the remainder of the reference.
    }

    //
    // Convert the reference to its string form.
    //
    toString()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        const s = [];

        const toStringMode = this._instance.toStringMode();

        //
        // If the encoded identity string contains characters which
        // the reference parser uses as separators, then we enclose
        // the identity string in quotes.
        //

        const id = Ice.identityToString(this._identity, toStringMode);
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
            const fs = StringUtil.escapeString(this._facet, "", toStringMode);
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

            default:
            {
                Debug.assert(false);
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
    }

    //
    // Convert the reference to its property form.
    //
    toProperty(prefix)
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    getRequestHandler(proxy)
    {
        // Abstract
        Debug.assert(false);
    }

    getBatchRequestQueue()
    {
        // Abstract
        Debug.assert(false);
    }

    equals(r)
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

        if(!MapUtil.equals(this._context, r._context))
        {
            return false;
        }

        if(this._facet !== r._facet)
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
    }

    clone()
    {
        // Abstract
        Debug.assert(false);
        return null;
    }

    copyMembers(r)
    {
        //
        // Copy the members that are not passed to the constructor.
        //
        r._context = this._context;
    }
}

Reference._emptyContext = new Map();
Reference._emptyEndpoints = [];

Ice.Reference = Reference;

class FixedReference extends Reference
{
    constructor(instance, communicator, identity, facet, mode, secure, protocol, encoding, connection,
                invocationTimeout, context)
    {
        super(instance, communicator, identity, facet, mode, secure, protocol, encoding, invocationTimeout, context);
        this._fixedConnection = connection;
    }

    getEndpoints()
    {
        return Reference._emptyEndpoints;
    }

    getAdapterId()
    {
        return "";
    }

    getRouterInfo()
    {
        return null;
    }

    getLocatorInfo()
    {
        return null;
    }

    getCacheConnection()
    {
        return true;
    }

    getPreferSecure()
    {
        return false;
    }

    getEndpointSelection()
    {
        return EndpointSelectionType.Random;
    }

    getLocatorCacheTimeout()
    {
        return 0;
    }

    getConnectionId()
    {
        return "";
    }

    getTimeout()
    {
        return undefined;
    }

    changeAdapterId(newAdapterId)
    {
        throw new Ice.FixedProxyException();
    }

    changeEndpoints(newEndpoints)
    {
        throw new Ice.FixedProxyException();
    }

    changeLocato(newLocator)
    {
        throw new Ice.FixedProxyException();
    }

    changeRouter(newRouter)
    {
        throw new Ice.FixedProxyException();
    }

    changeCacheConnection(newCache)
    {
        throw new Ice.FixedProxyException();
    }

    changePreferSecure(prefSec)
    {
        throw new Ice.FixedProxyException();
    }

    changeEndpointSelection(newType)
    {
        throw new Ice.FixedProxyException();
    }

    changeLocatorCacheTimeout(newTimeout)
    {
        throw new Ice.FixedProxyException();
    }

    changeTimeout(newTimeout)
    {
        throw new Ice.FixedProxyException();
    }

    changeConnectionId(connectionId)
    {
        throw new Ice.FixedProxyException();
    }

    changeConnection(newConnection)
    {
        if(newConnection == this._fixedConnection)
        {
            return this;
        }
        const r = this.getInstance().referenceFactory().copy(this);
        r._fixedConnection = newConnection;
        return r;
    }

    isIndirect()
    {
        return false;
    }

    isWellKnown()
    {
        return false;
    }

    streamWrite(s)
    {
        throw new Ice.FixedProxyException();
    }

    toProperty(prefix)
    {
        throw new Ice.FixedProxyException();
    }

    clone()
    {
        const r = new FixedReference(this.getInstance(),
                                     this.getCommunicator(),
                                     this.getIdentity(),
                                     this.getFacet(),
                                     this.getMode(),
                                     this.getSecure(),
                                     this.getProtocol(),
                                     this.getEncoding(),
                                     this._fixedConnection,
                                     this.getInvocationTimeout(),
                                     this.getContext());
        this.copyMembers(r);
        return r;
    }

    getRequestHandler(proxy)
    {
        switch(this.getMode())
        {
            case RefMode.ModeTwoway:
            case RefMode.ModeOneway:
            case RefMode.ModeBatchOneway:
            {
                if(this._fixedConnection.endpoint().datagram())
                {
                    throw new Ice.NoEndpointException(this.toString());
                }
                break;
            }

            case RefMode.ModeDatagram:
            case RefMode.ModeBatchDatagram:
            {
                if(!this._fixedConnection.endpoint().datagram())
                {
                    throw new Ice.NoEndpointException(this.toString());
                }
                break;
            }

            default:
            {
                Debug.assert(false);
                break;
            }
        }

        //
        // If a secure connection is requested or secure overrides is set,
        // check if the connection is secure.
        //
        const defaultsAndOverrides = this.getInstance().defaultsAndOverrides();
        const secure = defaultsAndOverrides.overrideSecure ? defaultsAndOverrides.overrideSecureValue : this.getSecure();
        if(secure && !this._fixedConnection.endpoint().secure())
        {
            throw new Ice.NoEndpointException(this.toString());
        }

        this._fixedConnection.throwException(); // Throw in case our connection is already destroyed.

        return proxy._setRequestHandler(new ConnectionRequestHandler(this, this._fixedConnection));
    }

    getBatchRequestQueue()
    {
        return this._fixedConnection.getBatchRequestQueue();
    }

    equals(rhs)
    {
        if(this === rhs)
        {
            return true;
        }
        if(!(rhs instanceof FixedReference))
        {
            return false;
        }
        if(!super.equals(rhs))
        {
            return false;
        }
        return this._fixedConnection == rhs._fixedConnection;
    }
}

Ice.FixedReference = FixedReference;

class RoutableReference extends Reference
{
    constructor(instance, communicator, identity, facet, mode, secure, protocol, encoding, endpoints,
                adapterId, locatorInfo, routerInfo, cacheConnection, preferSecure, endpointSelection,
                locatorCacheTimeout, invocationTimeout, context)
    {
        super(instance, communicator, identity, facet, mode, secure, protocol, encoding, invocationTimeout, context);
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
    }

    getEndpoints()
    {
        return this._endpoints;
    }

    getAdapterId()
    {
        return this._adapterId;
    }

    getRouterInfo()
    {
        return this._routerInfo;
    }

    getLocatorInfo()
    {
        return this._locatorInfo;
    }

    getCacheConnection()
    {
        return this._cacheConnection;
    }

    getPreferSecure()
    {
        return this._preferSecure;
    }

    getEndpointSelection()
    {
        return this._endpointSelection;
    }

    getLocatorCacheTimeout()
    {
        return this._locatorCacheTimeout;
    }

    getConnectionId()
    {
        return this._connectionId;
    }

    getTimeout()
    {
        return this._overrideTimeout ? this._timeout : undefined;
    }

    changeEncoding(newEncoding)
    {
        const r = super.changeEncoding(newEncoding);
        if(r !== this)
        {
            if(r._locatorInfo !== null && !r._locatorInfo.getLocator().ice_getEncodingVersion().equals(newEncoding))
            {
                r._locatorInfo = this.getInstance().locatorManager().find(
                    r._locatorInfo.getLocator().ice_encodingVersion(newEncoding));
            }
        }
        return r;
    }

    changeAdapterId(newAdapterId)
    {
        if(this._adapterId === newAdapterId)
        {
            return this;
        }
        const r = this.getInstance().referenceFactory().copy(this);
        r._adapterId = newAdapterId;
        r._endpoints = Reference._emptyEndpoints;
        return r;
    }

    changeEndpoints(newEndpoints)
    {
        if(ArrayUtil.equals(newEndpoints, this._endpoints, (e1, e2) => e1.equals(e2)))
        {
            return this;
        }
        const r = this.getInstance().referenceFactory().copy(this);
        r._endpoints = newEndpoints;
        r._adapterId = "";
        r.applyOverrides(r._endpoints);
        return r;
    }

    changeLocator(newLocator)
    {
        const newLocatorInfo = this.getInstance().locatorManager().find(newLocator);
        if(newLocatorInfo !== null && this._locatorInfo !== null && newLocatorInfo.equals(this._locatorInfo))
        {
            return this;
        }
        const r = this.getInstance().referenceFactory().copy(this);
        r._locatorInfo = newLocatorInfo;
        return r;
    }

    changeRouter(newRouter)
    {
        const newRouterInfo = this.getInstance().routerManager().find(newRouter);
        if(newRouterInfo !== null && this._routerInfo !== null && newRouterInfo.equals(this._routerInfo))
        {
            return this;
        }
        const r = this.getInstance().referenceFactory().copy(this);
        r._routerInfo = newRouterInfo;
        return r;
    }

    changeCacheConnection(newCache)
    {
        if(newCache === this._cacheConnection)
        {
            return this;
        }
        const r = this.getInstance().referenceFactory().copy(this);
        r._cacheConnection = newCache;
        return r;
    }

    changePreferSecure(newPreferSecure)
    {
        if(newPreferSecure === this._preferSecure)
        {
            return this;
        }
        const r = this.getInstance().referenceFactory().copy(this);
        r._preferSecure = newPreferSecure;
        return r;
    }

    changeEndpointSelection(newType)
    {
        if(newType === this._endpointSelection)
        {
            return this;
        }
        const r = this.getInstance().referenceFactory().copy(this);
        r._endpointSelection = newType;
        return r;
    }

    changeLocatorCacheTimeout(newTimeout)
    {
        if(this._locatorCacheTimeout === newTimeout)
        {
            return this;
        }
        const r = this.getInstance().referenceFactory().copy(this);
        r._locatorCacheTimeout = newTimeout;
        return r;
    }

    changeTimeout(newTimeout)
    {
        if(this._overrideTimeout && this._timeout === newTimeout)
        {
            return this;
        }
        const r = this.getInstance().referenceFactory().copy(this);
        r._timeout = newTimeout;
        r._overrideTimeout = true;
        r._endpoints = this._endpoints.map(endpoint => endpoint.changeTimeout(newTimeout));
        return r;
    }

    changeConnectionId(id)
    {
        if(this._connectionId === id)
        {
            return this;
        }
        const r = this.getInstance().referenceFactory().copy(this);
        r._connectionId = id;
        r._endpoints = this._endpoints.map(endpoint => endpoint.changeConnectionId(id));
        return r;
    }

    changeConnection(newConnection)
    {
        return new FixedReference(this.getInstance(),
                                  this.getCommunicator(),
                                  this.getIdentity(),
                                  this.getFacet(),
                                  this.getMode(),
                                  this.getSecure(),
                                  this.getProtocol(),
                                  this.getEncoding(),
                                  newConnection,
                                  this.getInvocationTimeout(),
                                  this.getContext());
    }

    isIndirect()
    {
        return this._endpoints.length === 0;
    }

    isWellKnown()
    {
        return this._endpoints.length === 0 && this._adapterId.length === 0;
    }

    streamWrite(s)
    {
        super.streamWrite(s);

        s.writeSize(this._endpoints.length);
        if(this._endpoints.length > 0)
        {
            Debug.assert(this._adapterId.length === 0);
            this._endpoints.forEach(endpoint =>
                {
                    s.writeShort(endpoint.type());
                    endpoint.streamWrite(s);
                });
        }
        else
        {
            s.writeString(this._adapterId); // Adapter id.
        }
    }

    toString()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        const s = [];
        s.push(super.toString());
        if(this._endpoints.length > 0)
        {
            this._endpoints.forEach(endpoint =>
                {
                    const endp = endpoint.toString();
                    if(endp !== null && endp.length > 0)
                    {
                        s.push(':');
                        s.push(endp);
                    }
                });
        }
        else if(this._adapterId.length > 0)
        {
            s.push(" @ ");

            //
            // If the encoded adapter id string contains characters which
            // the reference parser uses as separators, then we enclose
            // the adapter id string in quotes.
            //
            const a = StringUtil.escapeString(this._adapterId, null, this._instance.toStringMode());
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
    }

    toProperty(prefix)
    {
        const properties = new Map();

        properties.set(prefix, this.toString());
        properties.set(prefix + ".CollocationOptimized", "0");
        properties.set(prefix + ".ConnectionCached", this._cacheConnection ? "1" : "0");
        properties.set(prefix + ".PreferSecure", this._preferSecure ? "1" : "0");
        properties.set(prefix + ".EndpointSelection",
                       this._endpointSelection === EndpointSelectionType.Random ? "Random" : "Ordered");

        properties.set(prefix + ".LocatorCacheTimeout", String(this._locatorCacheTimeout));
        properties.set(prefix + ".InvocationTimeout", String(this.getInvocationTimeout()));

        if(this._routerInfo !== null)
        {
            this._routerInfo.getRouter()._getReference().toProperty(prefix + ".Router").forEach(
                (value, key) => properties.set(key, value));
        }

        if(this._locatorInfo !== null)
        {
            this._locatorInfo.getLocator()._getReference().toProperty(prefix + ".Locator").forEach(
                (value, key) => properties.set(key, value));
        }

        return properties;
    }

    hashCode()
    {
        if(!this._hashInitialized)
        {
            super.hashCode(); // Initializes _hashValue.
            this._hashValue = HashUtil.addString(this._hashValue, this._adapterId);
        }
        return this._hashValue;
    }

    equals(rhs)
    {
        if(this === rhs)
        {
            return true;
        }
        if(!(rhs instanceof RoutableReference))
        {
            return false;
        }

        if(!super.equals(rhs))
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
        if(!ArrayUtil.equals(this._endpoints, rhs._endpoints, (e1, e2) => e1.equals(e2)))
        {
            return false;
        }
        if(this._adapterId !== rhs._adapterId)
        {
            return false;
        }
        return true;
    }

    getRequestHandler(proxy)
    {
        return this._instance.requestHandlerFactory().getRequestHandler(this, proxy);
    }

    getBatchRequestQueue()
    {
        return new BatchRequestQueue(this._instance, this._mode === RefMode.ModeBatchDatagram);
    }

    getConnection()
    {
        const p = new Ice.Promise(); // success callback receives (connection)

        if(this._routerInfo !== null)
        {
            //
            // If we route, we send everything to the router's client
            // proxy endpoints.
            //
            this._routerInfo.getClientEndpoints().then(endpts =>
                {
                    if(endpts.length > 0)
                    {
                        this.applyOverrides(endpts);
                        this.createConnection(endpts).then(p.resolve, p.reject);
                    }
                    else
                    {
                        this.getConnectionNoRouterInfo(p);
                    }
                }).catch(p.reject);
        }
        else
        {
            this.getConnectionNoRouterInfo(p);
        }
        return p;
    }

    getConnectionNoRouterInfo(p)
    {
        if(this._endpoints.length > 0)
        {
            this.createConnection(this._endpoints).then(p.resolve).catch(p.reject);
            return;
        }

        if(this._locatorInfo !== null)
        {
            this._locatorInfo.getEndpoints(this, null, this._locatorCacheTimeout).then(
                values =>
                {
                    const [endpoints, cached] = values;
                    if(endpoints.length === 0)
                    {
                        p.reject(new Ice.NoEndpointException(this.toString()));
                        return;
                    }

                    this.applyOverrides(endpoints);
                    this.createConnection(endpoints).then(
                        p.resolve,
                        ex =>
                        {
                            if(ex instanceof Ice.NoEndpointException)
                            {
                                //
                                // No need to retry if there's no endpoints.
                                //
                                p.reject(ex);
                            }
                            else
                            {
                                Debug.assert(this._locatorInfo !== null);
                                this.getLocatorInfo().clearCache(this);
                                if(cached)
                                {
                                    const traceLevels = this.getInstance().traceLevels();
                                    if(traceLevels.retry >= 2)
                                    {
                                        this.getInstance().initializationData().logger.trace(
                                            traceLevels.retryCat,
                                            "connection to cached endpoints failed\n" +
                                            "removing endpoints from cache and trying again\n" +
                                            ex.toString());
                                    }
                                    this.getConnectionNoRouterInfo(p); // Retry.
                                    return;
                                }
                                p.reject(ex);
                            }
                        });
                }).catch(p.reject);
        }
        else
        {
            p.reject(new Ice.NoEndpointException(this.toString()));
        }
    }

    clone()
    {
        const r = new RoutableReference(this.getInstance(),
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
    }

    copyMembers(rhs)
    {
        //
        // Copy the members that are not passed to the constructor.
        //
        super.copyMembers(rhs);
        rhs._overrideTimeout = this._overrideTimeout;
        rhs._timeout = this._timeout;
        rhs._connectionId = this._connectionId;
    }

    applyOverrides(endpts)
    {
        //
        // Apply the endpoint overrides to each endpoint.
        //
        for(let i = 0; i < endpts.length; ++i)
        {
            endpts[i] = endpts[i].changeConnectionId(this._connectionId);
            if(this._overrideTimeout)
            {
                endpts[i] = endpts[i].changeTimeout(this._timeout);
            }
        }
    }

    filterEndpoints(allEndpoints)
    {
        //
        // Filter out opaque endpoints or endpoints which can't connect.
        //
        let endpoints = allEndpoints.filter(e => !(e instanceof OpaqueEndpointI) && e.connectable());

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
                endpoints = endpoints.filter(e => !e.datagram());
                break;
            }

            case RefMode.ModeDatagram:
            case RefMode.ModeBatchDatagram:
            {
                //
                // Filter out non-datagram endpoints.
                //
                endpoints = endpoints.filter(e => e.datagram());
                break;
            }

            default:
            {
                Debug.assert(false);
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
        const overrides = this.getInstance().defaultsAndOverrides();
        if(overrides.overrideSecure ? overrides.overrideSecureValue : this.getSecure())
        {
            endpoints = endpoints.filter(e => e.secure());
        }
        else
        {
            const preferSecure = this.getPreferSecure();
            const compare = (e1, e2) =>
            {
                const ls = e1.secure();
                const rs = e2.secure();
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
    }

    createConnection(allEndpoints)
    {
        const endpoints = this.filterEndpoints(allEndpoints);
        if(endpoints.length === 0)
        {
            return Ice.Promise.reject(new Ice.NoEndpointException(this.toString()));
        }

        //
        // Finally, create the connection.
        //
        const promise = new Ice.Promise();
        const factory = this.getInstance().outgoingConnectionFactory();
        if(this.getCacheConnection() || endpoints.length == 1)
        {
            //
            // Get an existing connection or create one if there's no
            // existing connection to one of the given endpoints.
            //
            const cb = new CreateConnectionCallback(this, null, promise);
            factory.create(endpoints, false, this.getEndpointSelection()).then(
                connection => cb.setConnection(connection)).catch(ex => cb.setException(ex));
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
            const cb = new CreateConnectionCallback(this, endpoints, promise);
            factory.create([endpoints[0]], true, this.getEndpointSelection()).then(
                connection => cb.setConnection(connection)).catch(ex => cb.setException(ex));
        }
        return promise;
    }
}

Ice.RoutableReference = RoutableReference;
module.exports.Ice = Ice;

class CreateConnectionCallback
{
    constructor(r, endpoints, promise)
    {
        this.ref = r;
        this.endpoints = endpoints;
        this.promise = promise;
        this.i = 0;
        this.exception = null;
    }

    setConnection(connection)
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
        this.promise.resolve(connection);
    }

    setException(ex)
    {
        if(this.exception === null)
        {
            this.exception = ex;
        }

        if(this.endpoints === null || ++this.i === this.endpoints.length)
        {
            this.promise.reject(this.exception);
            return;
        }

        this.ref.getInstance().outgoingConnectionFactory().create(
            [this.endpoints[this.i]],
            this.i != this.endpoints.length - 1,
            this.ref.getEndpointSelection()).then(
                connection => this.setConnection(connection)).catch(ex => this.setException(ex));
    }
}
