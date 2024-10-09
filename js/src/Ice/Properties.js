//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { StringUtil } from "./StringUtil.js";
import { PropertyNames } from "./PropertyNames.js";
import { getProcessLogger } from "./ProcessLogger.js";
import { InitializationException } from "./LocalExceptions.js";
import { Debug } from "./Debug.js";

const ParseStateKey = 0;
const ParseStateValue = 1;
//
// Ice.Properties
//
export class Properties {
    constructor(args, defaults) {
        this._properties = new Map();

        if (defaults !== undefined && defaults !== null) {
            // We can't just do a shallow copy of the map as the map values would otherwise be shared between the two
            // instances.
            for (const [key, property] of defaults._properties) {
                this._properties.set(key, {
                    value: property.value,
                    used: false,
                });
            }
        }

        if (args !== undefined && args !== null) {
            const v = this.parseIceCommandLineOptions(args);
            args.length = 0;
            for (let i = 0; i < v.length; ++i) {
                args.push(v[i]);
            }
        }
    }

    getProperty(key) {
        return this.getPropertyWithDefault(key, "");
    }

    getIceProperty(key) {
        return this.getPropertyWithDefault(key, Properties.getDefaultProperty(key));
    }

    getPropertyWithDefault(key, value) {
        const pv = this._properties.get(key);
        if (pv !== undefined) {
            pv.used = true;
            return pv.value;
        } else {
            return value;
        }
    }

    getPropertyAsInt(key) {
        return this.getPropertyAsIntWithDefault(key, 0);
    }

    getIcePropertyAsInt(key) {
        const defaultValueString = Properties.getDefaultProperty(key);
        var defaultValue = 0;
        if (defaultValueString != "") {
            defaultValue = parseInt(defaultValueString);
        }
        return this.getPropertyAsIntWithDefault(key, defaultValue);
    }

    getPropertyAsIntWithDefault(key, value) {
        const pv = this._properties.get(key);
        if (pv !== undefined) {
            pv.used = true;
            return parseInt(pv.value);
        } else {
            return value;
        }
    }

    getPropertyAsList(key) {
        return this.getPropertyAsListWithDefault(key, 0);
    }

    getIcePropertyAsList(key) {
        var defaultPropertyList = StringUtil.splitString(Properties.getDefaultProperty(key), ", \t\r\n");
        return this.getPropertyAsListWithDefault(key, defaultPropertyList);
    }

    getPropertyAsListWithDefault(key, value) {
        if (value === undefined || value === null) {
            value = [];
        }

        const pv = this._properties.get(key);
        if (pv !== undefined) {
            pv.used = true;

            let result = StringUtil.splitString(pv.value, ", \t\r\n");
            if (result === null) {
                getProcessLogger().warning(
                    "mismatched quotes in property " + key + "'s value, returning default value",
                );
                return value;
            }
            if (result.length === 0) {
                result = value;
            }
            return result;
        } else {
            return value;
        }
    }

    getPropertiesForPrefix(prefix = "") {
        const result = new Map();
        for (const [key, property] of this._properties) {
            if (key.startsWith(prefix)) {
                property.used = true;
                result.set(key, property.value);
            }
        }
        return result;
    }

    setProperty(key = "", value = "") {
        //
        // Trim whitespace
        //
        if (key !== null) {
            key = key.trim();
        }

        if (key === null || key.length === 0) {
            throw new InitializationException("Attempt to set property with empty key");
        }

        // Finds the corresponding Ice property if it exists. Also logs warnings for unknown Ice properties and
        // case-insensitive Ice property prefix matches.
        const prop = Properties.findIceProperty(key, true);

        // If the property is deprecated, log a warning
        if (prop !== null && prop.deprecated) {
            getProcessLogger().warning("setting deprecated property: " + key);
        }

        //
        // Set or clear the property.
        //
        if (value !== null && value.length > 0) {
            const pv = this._properties.get(key);
            if (pv !== undefined) {
                pv.value = value;
            } else {
                this._properties.set(key, { value: value, used: false });
            }
        } else {
            this._properties.delete(key);
        }
    }

    getCommandLineOptions() {
        const result = [];
        this._properties.forEach((property, key) => {
            result.push("--" + key + "=" + property.value);
        });
        return result;
    }

    parseCommandLineOptions(pfx, options) {
        if (pfx.length > 0 && pfx.charAt(pfx.length - 1) != ".") {
            pfx += ".";
        }
        pfx = "--" + pfx;

        const result = [];

        options.forEach(opt => {
            if (opt.indexOf(pfx) === 0) {
                if (opt.indexOf("=") === -1) {
                    opt += "=1";
                }

                this.parseLine(opt.substring(2));
            } else {
                result.push(opt);
            }
        });
        return result;
    }

    parseIceCommandLineOptions(options) {
        let args = options.slice();
        for (const prefix of PropertyNames.validProps.keys()) {
            args = this.parseCommandLineOptions(prefix, args);
        }
        return args;
    }

    parse(data) {
        data.match(/[^\r\n]+/g).forEach(line => this.parseLine(line));
    }

    parseLine(line) {
        let key = "";
        let value = "";

        let state = ParseStateKey;

        let whitespace = "";
        let escapedSpace = "";
        let finished = false;

        for (let i = 0; i < line.length; ++i) {
            let c = line.charAt(i);
            switch (state) {
                case ParseStateKey: {
                    switch (c) {
                        case "\\":
                            if (i < line.length - 1) {
                                c = line.charAt(++i);
                                switch (c) {
                                    case "\\":
                                    case "#":
                                    case "=":
                                        key += whitespace;
                                        whitespace = "";
                                        key += c;
                                        break;

                                    case " ":
                                        if (key.length !== 0) {
                                            whitespace += c;
                                        }
                                        break;

                                    default:
                                        key += whitespace;
                                        whitespace = "";
                                        key += "\\";
                                        key += c;
                                        break;
                                }
                            } else {
                                key += whitespace;
                                key += c;
                            }
                            break;

                        case " ":
                        case "\t":
                        case "\r":
                        case "\n":
                            if (key.length !== 0) {
                                whitespace += c;
                            }
                            break;

                        case "=":
                            whitespace = "";
                            state = ParseStateValue;
                            break;

                        case "#":
                            finished = true;
                            break;

                        default:
                            key += whitespace;
                            whitespace = "";
                            key += c;
                            break;
                    }
                    break;
                }

                case ParseStateValue: {
                    switch (c) {
                        case "\\":
                            if (i < line.length - 1) {
                                c = line.charAt(++i);
                                switch (c) {
                                    case "\\":
                                    case "#":
                                    case "=":
                                        value += value.length === 0 ? escapedSpace : whitespace;
                                        whitespace = "";
                                        escapedSpace = "";
                                        value += c;
                                        break;

                                    case " ":
                                        whitespace += c;
                                        escapedSpace += c;
                                        break;

                                    default:
                                        value += value.length === 0 ? escapedSpace : whitespace;
                                        whitespace = "";
                                        escapedSpace = "";
                                        value += "\\";
                                        value += c;
                                        break;
                                }
                            } else {
                                value += value.length === 0 ? escapedSpace : whitespace;
                                value += c;
                            }
                            break;

                        case " ":
                        case "\t":
                        case "\r":
                        case "\n":
                            if (value.length !== 0) {
                                whitespace += c;
                            }
                            break;

                        case "#":
                            finished = true;
                            break;

                        default:
                            value += value.length === 0 ? escapedSpace : whitespace;
                            whitespace = "";
                            escapedSpace = "";
                            value += c;
                            break;
                    }
                    break;
                }

                default: {
                    Debug.assert(false);
                    break;
                }
            }
            if (finished) {
                break;
            }
        }
        value += escapedSpace;

        if ((state === ParseStateKey && key.length !== 0) || (state == ParseStateValue && key.length === 0)) {
            getProcessLogger().warning('invalid config file entry: "' + line + '"');
            return;
        } else if (key.length === 0) {
            return;
        }

        this.setProperty(key, value);
    }

    clone() {
        return new Properties(null, this);
    }

    getUnusedProperties() {
        const unused = [];
        this._properties.forEach((property, key) => {
            if (!property.used) {
                unused.push(key);
            }
        });
        return unused;
    }

    static createProperties(args, defaults) {
        return new Properties(args, defaults);
    }

    static findProperty(key, propertyArray) {
        for (const prop of propertyArray.properties) {
            // If the key is an exact match, return the property unless it has a property class which is prefix only.
            // If the key is a regex match, return the property. A property cannot have a property class and use regex.
            if (key == prop.pattern) {
                if (prop.propertyArray !== null && prop.propertyArray.prefixOnly) {
                    return null;
                }
                return prop;
            } else if (prop.usesRegex && key.match(prop.pattern)) {
                return prop;
            }

            // If the property has a property array, check if the key is a prefix of the property.
            if (prop.propertyArray) {
                // Check if the key is a prefix of the property.
                // The key must be:
                // - shorter than the property pattern
                // - the property pattern must start with the key
                // - the pattern character after the key must be a dot
                if (key.length > prop.pattern.length && key.startsWith(`${prop.pattern}.`)) {
                    // Plus one to skip the dot.
                    let substring = key.substring(prop.pattern.length + 1);

                    // Check if the suffix is a valid property. If so, return it. If it's not, continue searching
                    // the current property array.
                    const subProp = Properties.findProperty(substring, prop.propertyArray);
                    if (subProp !== null) {
                        return subProp;
                    }
                }
            }
        }
        return null;
    }

    static validatePropertiesWithPrefix(prefix, properties, propertyArray) {
        // Do not check for unknown properties if Ice prefix, ie Ice, Glacier2, etc
        if (PropertyNames.validProps.keys().some(name => prefix.startsWith(`${name}.`))) {
            return;
        }

        var unknownProps = [];
        let props = properties.getPropertiesForPrefix(`${prefix}.`);

        for (const key of props.keys()) {
            if (Properties.findProperty(key.substring(prefix.length + 1), propertyArray) === null) {
                unknownProps.push(key);
            }
        }

        if (unknownProps.length > 0) {
            `found unknown properties for ${propertyArray.name}: '${prefix}'\n${unknownProps.join(",\n")}`;
        }
    }

    static findIceProperty(key, logWarnings) {
        // Check if the property is a known Ice property and log warnings if necessary
        const logger = getProcessLogger();

        let dotPos = key.indexOf(".");

        // If the key doesn't contain a dot, it's not a valid Ice property.
        if (dotPos === -1) {
            return null;
        }

        const prefix = key.substr(0, dotPos);
        var propertyArray = null;

        // Search for the property prefix
        for (const [validPropsPrefix, validPropsValue] of PropertyNames.validProps) {
            if (validPropsPrefix === prefix) {
                propertyArray = validPropsValue;
                break;
            }

            if (logWarnings && validPropsPrefix.toUpperCase() === prefix.toUpperCase()) {
                logger.warning("unknown property prefix: `" + prefix + "'; did you mean `" + validPropsPrefix + "'?");
                return null;
            }
        }

        if (propertyArray === null) {
            // The prefix is not a valid Ice property.
            return null;
        }

        let prop = Properties.findProperty(key.substr(dotPos + 1), propertyArray);
        if (prop !== null) {
            return prop;
        }

        // If we get here, the prefix is valid but the property is unknown
        if (logWarnings) {
            logger.warning("unknown property: " + key);
        }
        return null;
    }

    static getDefaultProperty(key) {
        // Find the property, don't log any warnings.
        const prop = Properties.findIceProperty(key, false);
        if (prop === null) {
            throw new Error("unknown Ice property: " + key);
        }
        return prop.defaultValue;
    }
}
