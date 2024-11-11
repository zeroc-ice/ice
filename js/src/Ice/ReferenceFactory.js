//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { EndpointSelectionType } from "./EndpointSelectionType.js";
import { Ice as Ice_Locator } from "./Locator.js";
const { LocatorPrx } = Ice_Locator;
import { PropertyNames } from "./PropertyNames.js";
import { ReferenceMode } from "./ReferenceMode.js";
import { Ice as Ice_Router } from "./Router.js";
const { RouterPrx } = Ice_Router;
import { Ice as Ice_BuiltinSequences } from "./BuiltinSequences.js";
const { StringSeqHelper } = Ice_BuiltinSequences;
import { StringUtil } from "./StringUtil.js";
import { Encoding_1_0, Protocol_1_0, stringToEncodingVersion, stringToProtocolVersion } from "./Protocol.js";
import { ParseException, MarshalException } from "./LocalExceptions.js";
import { Ice as Ice_Version } from "./Version.js";
const { ProtocolVersion, EncodingVersion } = Ice_Version;
import { Debug } from "./Debug.js";
import { stringToIdentity } from "./StringToIdentity.js";
import { RoutableReference, FixedReference } from "./Reference.js";
import { Properties } from "./Properties.js";

//
// Only for use by Instance
//
export class ReferenceFactory {
    constructor(instance, communicator) {
        this._instance = instance;
        this._communicator = communicator;
        this._defaultRouter = null;
        this._defaultLocator = null;
    }

    create(ident, facet, template, endpoints) {
        if (ident.name.length === 0 && ident.category.length === 0) {
            return null;
        }

        return this.createImpl(
            ident,
            facet,
            template.getMode(),
            template.getSecure(),
            template.getProtocol(),
            template.getEncoding(),
            endpoints,
            null,
            null,
        );
    }

    createWithAdapterId(ident, facet, template, adapterId) {
        if (ident.name.length === 0 && ident.category.length === 0) {
            return null;
        }

        return this.createImpl(
            ident,
            facet,
            template.getMode(),
            template.getSecure(),
            template.getProtocol(),
            template.getEncoding(),
            null,
            adapterId,
            null,
        );
    }

    createFixed(ident, fixedConnection) {
        if (ident.name.length === 0 && ident.category.length === 0) {
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
            fixedConnection.endpoint().datagram() ? ReferenceMode.ModeDatagram : ReferenceMode.ModeTwoway,
            fixedConnection.endpoint().secure(),
            Protocol_1_0,
            this._instance.defaultsAndOverrides().defaultEncoding,
            fixedConnection,
            -1,
            null,
        );
    }

    copy(r) {
        const ident = r.getIdentity();
        if (ident.name.length === 0 && ident.category.length === 0) {
            return null;
        }
        return r.clone();
    }

    createFromString(s, propertyPrefix) {
        if (s === undefined || s === null || s.length === 0) {
            return null;
        }

        const delim = " \t\n\r";

        let end = 0;
        let beg = StringUtil.findFirstNotOf(s, delim, end);
        if (beg == -1) {
            throw new ParseException(`no non-whitespace characters found in '${s}'`);
        }

        //
        // Extract the identity, which may be enclosed in single
        // or double quotation marks.
        //
        let idstr = null;
        end = StringUtil.checkQuote(s, beg);
        if (end === -1) {
            throw new ParseException(`mismatched quotes around identity in '${s}'`);
        } else if (end === 0) {
            end = StringUtil.findFirstOf(s, delim + ":@", beg);
            if (end === -1) {
                end = s.length;
            }
            idstr = s.substring(beg, end);
        } else {
            beg++; // Skip leading quote
            idstr = s.substring(beg, end);
            end++; // Skip trailing quote
        }

        if (beg === end) {
            throw new ParseException(`no identity in '${s}'`);
        }

        //
        // Parsing the identity may raise ParseException.
        //
        const ident = stringToIdentity(idstr);

        if (ident.name.length === 0) {
            //
            // An identity with an empty name and a non-empty
            // category is illegal.
            //
            if (ident.category.length > 0) {
                throw new ParseException("The category of a null Ice object identity must be empty.");
            }
            //
            // Treat a stringified proxy containing two double
            // quotes ("") the same as an empty string, i.e.,
            // a null proxy, but only if nothing follows the
            // quotes.
            //
            else if (StringUtil.findFirstNotOf(s, delim, end) != -1) {
                throw new ParseException(`invalid characters after identity in '${s}'`);
            } else {
                return null;
            }
        }

        let facet = "";
        let mode = ReferenceMode.ModeTwoway;
        let secure = false;
        let encoding = this._instance.defaultsAndOverrides().defaultEncoding;
        let protocol = Protocol_1_0;
        let adapter = "";

        while (true) {
            beg = StringUtil.findFirstNotOf(s, delim, end);
            if (beg === -1) {
                break;
            }

            if (s.charAt(beg) == ":" || s.charAt(beg) == "@") {
                break;
            }

            end = StringUtil.findFirstOf(s, delim + ":@", beg);
            if (end == -1) {
                end = s.length;
            }

            if (beg == end) {
                break;
            }

            const option = s.substring(beg, end);
            if (option.length != 2 || option.charAt(0) != "-") {
                throw new ParseException(`expected a proxy option but found '${option}' in '${s}'`);
            }

            //
            // Check for the presence of an option argument. The
            // argument may be enclosed in single or double
            // quotation marks.
            //
            let argument = null;
            const argumentBeg = StringUtil.findFirstNotOf(s, delim, end);
            if (argumentBeg != -1) {
                const ch = s.charAt(argumentBeg);
                if (ch != "@" && ch != ":" && ch != "-") {
                    beg = argumentBeg;
                    end = StringUtil.checkQuote(s, beg);
                    if (end == -1) {
                        throw new ParseException(`mismatched quotes around value for ${option} option in '${s}'`);
                    } else if (end === 0) {
                        end = StringUtil.findFirstOf(s, delim + ":@", beg);
                        if (end === -1) {
                            end = s.length;
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
            // IceInternal::Reference::toString() and its derived classes must be updated as well.
            //
            switch (option.charAt(1)) {
                case "f": {
                    if (argument === null) {
                        throw new ParseException(`no argument provided for -f option in '${s}'`);
                    }

                    try {
                        facet = StringUtil.unescapeString(argument, 0, argument.length);
                    } catch (ex) {
                        throw new ParseException(`invalid facet in ${s}': `, ex);
                    }

                    break;
                }

                case "t": {
                    if (argument !== null) {
                        throw new ParseException(`unexpected argument '${argument}' provided for -t option in '${s}'`);
                    }
                    mode = ReferenceMode.ModeTwoway;
                    break;
                }

                case "o": {
                    if (argument !== null) {
                        throw new ParseException(`unexpected argument '${argument}' provided for -o option in '${s}'`);
                    }
                    mode = ReferenceMode.ModeOneway;
                    break;
                }

                case "O": {
                    if (argument !== null) {
                        throw new ParseException(`unexpected argument '${argument}' provided for -O option in '${s}'`);
                    }
                    mode = ReferenceMode.ModeBatchOneway;
                    break;
                }

                case "d": {
                    if (argument !== null) {
                        throw new ParseException(`unexpected argument '${argument}' provided for -d option in '${s}'`);
                    }
                    mode = ReferenceMode.ModeDatagram;
                    break;
                }

                case "D": {
                    if (argument !== null) {
                        throw new ParseException(`unexpected argument '${argument}' provided for -D option in '${s}'`);
                    }
                    mode = ReferenceMode.ModeBatchDatagram;
                    break;
                }

                case "s": {
                    if (argument !== null) {
                        throw new ParseException(`unexpected argument '${argument}' provided for -s option in '${s}'`);
                    }
                    secure = true;
                    break;
                }

                case "e": {
                    if (argument === null) {
                        throw new ParseException(`no argument provided for -e option in '${s}'`);
                    }

                    try {
                        encoding = stringToEncodingVersion(argument);
                    } catch (e) {
                        throw new ParseException(`invalid encoding version '${argument}' in '${s}'`, e);
                    }
                    break;
                }

                case "p": {
                    if (argument === null) {
                        throw new ParseException(`no argument provided for -p option in '${s}'`);
                    }

                    try {
                        protocol = stringToProtocolVersion(argument);
                    } catch (
                        e // ParseException
                    ) {
                        throw new ParseException(`invalid protocol version '${argument}' in '${s}'`, e);
                    }
                    break;
                }

                default: {
                    throw new ParseException(`unknown option '${option}' in '${s}'`);
                }
            }
        }

        if (beg === -1) {
            return this.createImpl(ident, facet, mode, secure, protocol, encoding, null, null, propertyPrefix);
        }

        const endpoints = [];

        if (s.charAt(beg) == ":") {
            const unknownEndpoints = [];
            end = beg;

            while (end < s.length && s.charAt(end) == ":") {
                beg = end + 1;

                end = beg;
                while (true) {
                    end = s.indexOf(":", end);
                    if (end == -1) {
                        end = s.length;
                        break;
                    } else {
                        let quoted = false;
                        let quote = beg;
                        while (true) {
                            quote = s.indexOf('"', quote);
                            if (quote == -1 || end < quote) {
                                break;
                            } else {
                                quote = s.indexOf('"', ++quote);
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

                const es = s.substring(beg, end);
                const endp = this._instance.endpointFactoryManager().create(es, false);
                if (endp !== null) {
                    endpoints.push(endp);
                } else {
                    unknownEndpoints.push(es);
                }
            }
            if (endpoints.length === 0) {
                DEV: console.assert(unknownEndpoints.length > 0);
                throw new ParseException(`invalid endpoint '${unknownEndpoints[0]}' in '${s}'`);
            } else if (
                unknownEndpoints.length !== 0 &&
                this._instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Endpoints", 1) > 0
            ) {
                const msg = [];
                msg.push("Proxy contains unknown endpoints:");
                unknownEndpoints.forEach(unknownEndpoint => {
                    msg.push(" `");
                    msg.push(unknownEndpoint);
                    msg.push("'");
                });
                this._instance.initializationData().logger.warning(msg.join(""));
            }

            return this.createImpl(ident, facet, mode, secure, protocol, encoding, endpoints, null, propertyPrefix);
        } else if (s.charAt(beg) == "@") {
            beg = StringUtil.findFirstNotOf(s, delim, beg + 1);
            if (beg == -1) {
                throw new ParseException(`missing adapter id in '${s}'`);
            }

            let adapterstr = null;
            end = StringUtil.checkQuote(s, beg);
            if (end === -1) {
                throw new ParseException(`mismatched quotes around adapter id in '${s}'`);
            } else if (end === 0) {
                end = StringUtil.findFirstOf(s, delim, beg);
                if (end === -1) {
                    end = s.length;
                }
                adapterstr = s.substring(beg, end);
            } else {
                beg++; // Skip leading quote
                adapterstr = s.substring(beg, end);
                end++; // Skip trailing quote
            }

            if (end !== s.length && StringUtil.findFirstNotOf(s, delim, end) !== -1) {
                throw new ParseException(`invalid trailing characters after ${s.substring(0, end + 1)}' in '${s}'`);
            }

            try {
                adapter = StringUtil.unescapeString(adapterstr, 0, adapterstr.length);
            } catch (ex) {
                throw new ParseException(`invalid adapter id in '${s}'`, ex);
            }
            if (adapter.length === 0) {
                throw new ParseException(`empty adapter id in '${s}'`);
            }
            return this.createImpl(ident, facet, mode, secure, protocol, encoding, null, adapter, propertyPrefix);
        }

        throw new ParseException(`malformed proxy '${s}'`);
    }

    createFromStream(ident, s) {
        //
        // Don't read the identity here. Operations calling this
        // constructor read the identity, and pass it as a parameter.
        //

        if (ident.name.length === 0 && ident.category.length === 0) {
            return null;
        }

        //
        // For compatibility with the old FacetPath.
        //
        const facetPath = StringSeqHelper.read(s); // String[]
        let facet;
        if (facetPath.length > 0) {
            if (facetPath.length > 1) {
                throw new MarshalException(`Received invalid facet path with ${facetPath.length} elements.`);
            }
            facet = facetPath[0];
        } else {
            facet = "";
        }

        const mode = s.readByte();
        if (mode < 0 || mode > ReferenceMode.ModeLast) {
            throw new MarshalException(`Received invalid proxy mode ${mode}`);
        }

        const secure = s.readBool();

        let protocol = null;
        let encoding = null;
        if (!s.getEncoding().equals(Encoding_1_0)) {
            protocol = new ProtocolVersion();
            protocol._read(s);
            encoding = new EncodingVersion();
            encoding._read(s);
        } else {
            protocol = Protocol_1_0;
            encoding = Encoding_1_0;
        }

        let endpoints = null; // EndpointI[]
        let adapterId = null;

        const sz = s.readSize();
        if (sz > 0) {
            endpoints = [];
            for (let i = 0; i < sz; i++) {
                endpoints[i] = this._instance.endpointFactoryManager().read(s);
            }
        } else {
            adapterId = s.readString();
        }

        return this.createImpl(ident, facet, mode, secure, protocol, encoding, endpoints, adapterId, null);
    }

    setDefaultRouter(defaultRouter) {
        if (this._defaultRouter === null ? defaultRouter === null : this._defaultRouter.equals(defaultRouter)) {
            return this;
        }

        const factory = new ReferenceFactory(this._instance, this._communicator);
        factory._defaultLocator = this._defaultLocator;
        factory._defaultRouter = defaultRouter;
        return factory;
    }

    getDefaultRouter() {
        return this._defaultRouter;
    }

    setDefaultLocator(defaultLocator) {
        if (this._defaultLocator === null ? defaultLocator === null : this._defaultLocator.equals(defaultLocator)) {
            return this;
        }

        const factory = new ReferenceFactory(this._instance, this._communicator);
        factory._defaultRouter = this._defaultRouter;
        factory._defaultLocator = defaultLocator;
        return factory;
    }

    getDefaultLocator() {
        return this._defaultLocator;
    }

    createImpl(ident, facet, mode, secure, protocol, encoding, endpoints, adapterId, propertyPrefix) {
        const defaultsAndOverrides = this._instance.defaultsAndOverrides();

        //
        // Default local proxy options.
        //
        let locatorInfo = null;
        if (this._defaultLocator !== null) {
            if (!this._defaultLocator._getReference().getEncoding().equals(encoding)) {
                locatorInfo = this._instance.locatorManager().find(this._defaultLocator.ice_encodingVersion(encoding));
            } else {
                locatorInfo = this._instance.locatorManager().find(this._defaultLocator);
            }
        }
        let routerInfo = this._instance.routerManager().find(this._defaultRouter);
        let cacheConnection = true;
        let preferSecure = defaultsAndOverrides.defaultPreferSecure;
        let endpointSelection = defaultsAndOverrides.defaultEndpointSelection;
        let locatorCacheTimeout = defaultsAndOverrides.defaultLocatorCacheTimeout;
        let invocationTimeout = defaultsAndOverrides.defaultInvocationTimeout;
        let context = null;

        //
        // Override the defaults with the proxy properties if a property prefix is defined.
        //
        if (propertyPrefix !== null && propertyPrefix.length > 0) {
            const properties = this._instance.initializationData().properties;

            Properties.validatePropertiesWithPrefix(propertyPrefix, properties, PropertyNames.ProxyProps);

            let property = propertyPrefix + ".Locator";
            const locator = LocatorPrx.uncheckedCast(this._communicator.propertyToProxy(property));
            if (locator !== null) {
                if (!locator._getReference().getEncoding().equals(encoding)) {
                    locatorInfo = this._instance.locatorManager().find(locator.ice_encodingVersion(encoding));
                } else {
                    locatorInfo = this._instance.locatorManager().find(locator);
                }
            }

            property = propertyPrefix + ".Router";
            const router = RouterPrx.uncheckedCast(this._communicator.propertyToProxy(property));
            if (router !== null) {
                if (propertyPrefix.endsWith("Router")) {
                    this._instance
                        .initializationData()
                        .logger.warning(
                            "`" +
                                property +
                                "=" +
                                properties.getProperty(property) +
                                "': cannot set a router on a router; setting ignored",
                        );
                } else {
                    routerInfo = this._instance.routerManager().find(router);
                }
            }

            property = propertyPrefix + ".ConnectionCached";
            cacheConnection = properties.getPropertyAsIntWithDefault(property, cacheConnection ? 1 : 0) > 0;

            property = propertyPrefix + ".PreferSecure";
            preferSecure = properties.getPropertyAsIntWithDefault(property, preferSecure ? 1 : 0) > 0;

            property = propertyPrefix + ".EndpointSelection";
            if (properties.getProperty(property).length > 0) {
                const type = properties.getProperty(property);
                if (type == "Random") {
                    endpointSelection = EndpointSelectionType.Random;
                } else if (type == "Ordered") {
                    endpointSelection = EndpointSelectionType.Ordered;
                } else {
                    throw new ParseException(`illegal value '${type}'; expected 'Random' or 'Ordered'`);
                }
            }

            property = propertyPrefix + ".LocatorCacheTimeout";
            let value = properties.getProperty(property);
            if (value.length !== 0) {
                locatorCacheTimeout = properties.getPropertyAsIntWithDefault(property, locatorCacheTimeout);
                if (locatorCacheTimeout < -1) {
                    locatorCacheTimeout = -1;
                    this._instance
                        .initializationData()
                        .logger.warning(
                            "invalid value for" +
                                property +
                                "`" +
                                properties.getProperty(property) +
                                "': defaulting to -1",
                        );
                }
            }

            property = propertyPrefix + ".InvocationTimeout";
            value = properties.getProperty(property);
            if (value.length !== 0) {
                invocationTimeout = properties.getPropertyAsIntWithDefault(property, invocationTimeout);
                if (invocationTimeout < 1 && invocationTimeout !== -1) {
                    invocationTimeout = -1;
                    this._instance
                        .initializationData()
                        .logger.warning(
                            "invalid value for" +
                                property +
                                "`" +
                                properties.getProperty(property) +
                                "': defaulting to -1",
                        );
                }
            }

            property = propertyPrefix + ".Context.";
            const contexts = properties.getPropertiesForPrefix(property);
            if (contexts.size > 0) {
                context = new Map();
                for (const [key, value] of contexts) {
                    context.set(key.substring(property.length), value);
                }
            }
        }

        //
        // Create new reference
        //
        return new RoutableReference(
            this._instance,
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
            invocationTimeout,
            context,
        );
    }
}
