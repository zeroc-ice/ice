//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/Debug");
require("../Ice/LocalException");
require("../Ice/ProcessLogger");
require("../Ice/ProcessLogger");
require("../Ice/PropertyNames");
require("../Ice/StringUtil");

const StringUtil = Ice.StringUtil;
const PropertyNames = Ice.PropertyNames;
const Debug = Ice.Debug;
const getProcessLogger = Ice.getProcessLogger;
const InitializationException = Ice.InitializationException;

const ParseStateKey = 0;
const ParseStateValue = 1;
//
// Ice.Properties
//
class Properties
{
    constructor(args, defaults)
    {
        this._properties = new Map();

        if(defaults !== undefined && defaults !== null)
        {
            //
            // NOTE: we can't just do a shallow copy of the map as the map values
            // would otherwise be shared between the two PropertiesI object.
            //
            for(const [key, property] of defaults._properties)
            {
                this._properties.set(key, {value: property.value, used: false});
            }
        }

        if(args !== undefined && args !== null)
        {
            const v = this.parseIceCommandLineOptions(args);
            args.length = 0;
            for(let i = 0; i < v.length; ++i)
            {
                args.push(v[i]);
            }
        }
    }

    getProperty(key)
    {
        return this.getPropertyWithDefault(key, "");
    }

    getPropertyWithDefault(key, value)
    {
        const pv = this._properties.get(key);
        if(pv !== undefined)
        {
            pv.used = true;
            return pv.value;
        }
        else
        {
            return value;
        }
    }

    getPropertyAsInt(key)
    {
        return this.getPropertyAsIntWithDefault(key, 0);
    }

    getPropertyAsIntWithDefault(key, value)
    {
        const pv = this._properties.get(key);
        if(pv !== undefined)
        {
            pv.used = true;
            return parseInt(pv.value);
        }
        else
        {
            return value;
        }
    }

    getPropertyAsList(key)
    {
        return this.getPropertyAsListWithDefault(key, 0);
    }

    getPropertyAsListWithDefault(key, value)
    {
        if(value === undefined || value === null)
        {
            value = [];
        }

        const pv = this._properties.get(key);
        if(pv !== undefined)
        {
            pv.used = true;

            let result = StringUtil.splitString(pv.value, ", \t\r\n");
            if(result === null)
            {
                getProcessLogger().warning("mismatched quotes in property " + key + "'s value, returning default value");
                return value;
            }
            if(result.length === 0)
            {
                result = value;
            }
            return result;
        }
        else
        {
            return value;
        }
    }

    getPropertiesForPrefix(prefix = "")
    {
        const result = new Map();
        this._properties.forEach((property, key) =>
            {
                if(key.indexOf(prefix) === 0)
                {
                    property.used = true;
                    result.set(key, property.value);
                }
            });
        return result;
    }

    setProperty(key = "", value = "")
    {
        //
        // Trim whitespace
        //
        if(key !== null)
        {
            key = key.trim();
        }

        //
        // Check if the property is legal.
        //
        const logger = getProcessLogger();
        if(key === null || key.length === 0)
        {
            throw new InitializationException("Attempt to set property with empty key");
        }

        let dotPos = key.indexOf(".");
        if(dotPos !== -1)
        {
            const prefix = key.substr(0, dotPos);
            for(let i = 0; i < PropertyNames.validProps.length; ++i)
            {
                let pattern = PropertyNames.validProps[i][0].pattern;
                dotPos = pattern.indexOf(".");
                //
                // Each top level prefix describes a non-empty namespace. Having a string without a
                // prefix followed by a dot is an error.
                //
                Debug.assert(dotPos != -1);
                if(pattern.substring(0, dotPos - 1) != prefix)
                {
                    continue;
                }

                let found = false;
                let mismatchCase = false;
                let otherKey;
                for(let j = 0; j < PropertyNames.validProps[i][j].length && !found; ++j)
                {
                    pattern = PropertyNames.validProps[i][j].pattern();
                    let pComp = new RegExp(pattern);
                    found = pComp.test(key);

                    if(found && PropertyNames.validProps[i][j].deprecated)
                    {
                        logger.warning("deprecated property: " + key);
                        if(PropertyNames.validProps[i][j].deprecatedBy !== null)
                        {
                            key = PropertyNames.validProps[i][j].deprecatedBy;
                        }
                    }

                    if(found)
                    {
                        break;
                    }
                    else
                    {
                        pComp = new RegExp(pattern.toUpperCase());
                        found = pComp.test(key.toUpperCase());
                        if(found)
                        {
                            mismatchCase = true;
                            otherKey = pattern.substr(2);
                            otherKey = otherKey.substr(0, otherKey.length - 1);
                            otherKey = otherKey.replace(/\\/g, "");
                            break;
                        }
                    }
                }

                if(!found)
                {
                    logger.warning("unknown property: " + key);
                }
                else if(mismatchCase)
                {
                    logger.warning("unknown property: `" + key + "'; did you mean `" + otherKey + "'");
                }
            }
        }

        //
        // Set or clear the property.
        //
        if(value !== null && value.length > 0)
        {
            const pv = this._properties.get(key);
            if(pv !== undefined)
            {
                pv.value = value;
            }
            else
            {
                this._properties.set(key, {value: value, used: false});
            }
        }
        else
        {
            this._properties.delete(key);
        }
    }

    getCommandLineOptions()
    {
        const result = [];
        this._properties.forEach((property, key) =>
            {
                result.push("--" + key + "=" + property.value);
            });
        return result;
    }

    parseCommandLineOptions(pfx, options)
    {
        if(pfx.length > 0 && pfx.charAt(pfx.length - 1) != ".")
        {
            pfx += ".";
        }
        pfx = "--" + pfx;

        const result = [];

        options.forEach(opt =>
            {
                if(opt.indexOf(pfx) === 0)
                {
                    if(opt.indexOf('=') === -1)
                    {
                        opt += "=1";
                    }

                    this.parseLine(opt.substring(2));
                }
                else
                {
                    result.push(opt);
                }
            });
        return result;
    }

    parseIceCommandLineOptions(options)
    {
        let args = options.slice();
        for(let i = 0; i < PropertyNames.clPropNames.length; ++i)
        {
            args = this.parseCommandLineOptions(PropertyNames.clPropNames[i], args);
        }
        return args;
    }

    parse(data)
    {
        data.match(/[^\r\n]+/g).forEach(line => this.parseLine(line));
    }

    parseLine(line)
    {
        let key = "";
        let value = "";

        let state = ParseStateKey;

        let whitespace = "";
        let escapedspace = "";
        let finished = false;

        for(let i = 0; i < line.length; ++i)
        {
            let c = line.charAt(i);
            switch(state)
            {
                case ParseStateKey:
                {
                    switch(c)
                    {
                        case '\\':
                            if(i < line.length - 1)
                            {
                                c = line.charAt(++i);
                                switch(c)
                                {
                                    case '\\':
                                    case '#':
                                    case '=':
                                        key += whitespace;
                                        whitespace = "";
                                        key += c;
                                        break;

                                    case ' ':
                                        if(key.length !== 0)
                                        {
                                            whitespace += c;
                                        }
                                        break;

                                    default:
                                        key += whitespace;
                                        whitespace = "";
                                        key += '\\';
                                        key += c;
                                        break;
                                }
                            }
                            else
                            {
                                key += whitespace;
                                key += c;
                            }
                            break;

                        case ' ':
                        case '\t':
                        case '\r':
                        case '\n':
                            if(key.length !== 0)
                            {
                                whitespace += c;
                            }
                            break;

                        case '=':
                            whitespace = "";
                            state = ParseStateValue;
                            break;

                        case '#':
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

                case ParseStateValue:
                {
                    switch(c)
                    {
                        case '\\':
                            if(i < line.length - 1)
                            {
                                c = line.charAt(++i);
                                switch(c)
                                {
                                    case '\\':
                                    case '#':
                                    case '=':
                                        value += value.length === 0 ? escapedspace : whitespace;
                                        whitespace = "";
                                        escapedspace = "";
                                        value += c;
                                        break;

                                    case ' ':
                                        whitespace += c;
                                        escapedspace += c;
                                        break;

                                    default:
                                        value += value.length === 0 ? escapedspace : whitespace;
                                        whitespace = "";
                                        escapedspace = "";
                                        value += '\\';
                                        value += c;
                                        break;
                                }
                            }
                            else
                            {
                                value += value.length === 0 ? escapedspace : whitespace;
                                value += c;
                            }
                            break;

                        case ' ':
                        case '\t':
                        case '\r':
                        case '\n':
                            if(value.length !== 0)
                            {
                                whitespace += c;
                            }
                            break;

                        case '#':
                            finished = true;
                            break;

                        default:
                            value += value.length === 0 ? escapedspace : whitespace;
                            whitespace = "";
                            escapedspace = "";
                            value += c;
                            break;
                    }
                    break;
                }

                default:
                {
                    Debug.assert(false);
                    break;
                }
            }
            if(finished)
            {
                break;
            }
        }
        value += escapedspace;

        if((state === ParseStateKey && key.length !== 0) ||
           (state == ParseStateValue && key.length === 0))
        {
            getProcessLogger().warning("invalid config file entry: \"" + line + "\"");
            return;
        }
        else if(key.length === 0)
        {
            return;
        }

        this.setProperty(key, value);
    }

    clone()
    {
        return new Properties(null, this);
    }

    getUnusedProperties()
    {
        const unused = [];
        this._properties.forEach((property, key) =>
            {
                if(!property.used)
                {
                    unused.push(key);
                }
            });
        return unused;
    }

    static createProperties(args, defaults)
    {
        return new Properties(args, defaults);
    }
}

Ice.Properties = Properties;
module.exports.Ice = Ice;
